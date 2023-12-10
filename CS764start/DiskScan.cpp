#include "DiskScan.h"

DiskScan::DiskScan(std::vector<int>& ssd_each_group_row, std::vector<int>& hdd_each_group_row, RowSize const row_size, BatchSize const batch_size):
    _row_size(row_size) , _batch_size(batch_size),
	_disk_run_list_row(ssd_each_group_row.size() + hdd_each_group_row.size()),
	_disk_run_list_col(batch_size),
	_ssd_each_group_row(ssd_each_group_row),
    _hdd_each_group_row(hdd_each_group_row)
{
    ////TRACE (TRACE_SWITCH);
    SSD = new File(SSD_PATH_TEMP, FileType::SSD, _row_size, SSD_BLOCK);
	HDD = new File(HDD_PATH_TEMP, FileType::HDD, _row_size, HDD_BLOCK);
    RES_HDD = new File(RES_HDD_PATH, __LONG_LONG_MAX__, HDD_BLOCK, FileType::HDD, _row_size);

    _shared_buffer = new SharedBuffer(OUTPUT_BUFFER, _row_size);
    // initialize current run index
	_current_run_index = 0;
	// initialize run list
	_disk_run_list = new Item**[_disk_run_list_row];
	for(uint32_t i=0;i<_disk_run_list_row;i++){
		_disk_run_list[i] = new Item*[_disk_run_list_col];
		memset(_disk_run_list[i], 0, sizeof(Item*) * _disk_run_list_col);
	}

	_each_group_col = new uint32_t[_ssd_each_group_row.size() + _hdd_each_group_row.size()];
	memset(_each_group_col, 0, sizeof(uint32_t) * (_ssd_each_group_row.size() + _hdd_each_group_row.size()));
	//initialize group offset
	_group_offset = new uint32_t[_ssd_each_group_row.size() + _hdd_each_group_row.size()];
	memset(_group_offset, 0, sizeof(uint32_t) * (_ssd_each_group_row.size() + _hdd_each_group_row.size()));	
    // initialize loser of tree
	_loser_tree = new LoserTree(_disk_run_list_row, _row_size);

    // merge
	if((_current_run_index >= _disk_run_list_row)){
		_loser_tree->reset(_disk_run_list_row, _loser_tree->getMaxItem());
		MultiwayMerge();
		_current_run_index = 0;
	}
}

DiskScan::~DiskScan ()
{
	unsigned long ssd_size = 0;
	for(int i =0 ;i <_ssd_each_group_row.size();i++){
		ssd_size += (unsigned long)_ssd_each_group_row[i]*_row_size;
	}
	unsigned long hdd_size = 0;
	for(int i =0 ;i <_hdd_each_group_row.size();i++){
		hdd_size += (unsigned long)_hdd_each_group_row[i]*_row_size;
	}

	printf (
	"||-----------------------[Memory Status]------------------------||\n"
	"||%-25s|%33lu MB||\n"
	"||----------------------[Read Disk Status]----------------------||\n"
	"||%-25s| %15lu Bytes ≈%9lu MB||\n"
	"||%-25s|                           %6.2f ms||\n"
	"||%-25s|                      %9lu MB/s||\n"
	"||                         |                                    ||\n"
	"||%-25s| %15lu Bytes ≈%9lu MB||\n"
	"||%-25s|                           %6.2f ms||\n"
	"||%-25s|                      %9lu MB/s||\n"
	"||---------------------[Write Disk Status]----------------------||\n"
	"||%-25s| %15lu Bytes ≈%9lu MB||\n"
	"||%-25s|                           %6.2f ms||\n"
	"||%-25s|                      %9lu MB/s||\n",
	"DRAM allocated size", (unsigned long)MAX_DRAM /1024 /1024,
	"Data read from SSD",ssd_size,(unsigned long) (ssd_size/(double)1000/1000),
	"SSD read latency",SSD_LATENCY,
	"SSD read bandwidth",SSD_BANDWIDTH/1024/1024,
	"Data read from HDD",(unsigned long) (hdd_size),(unsigned long) (hdd_size/(double)1000/1000),
	"HDD read latency",HDD_LATENCY,
	"HDD read bandwidth",HDD_BANDWIDTH/1024/1024,
	"Data written into HDD",(unsigned long) (RES_HDD->getCurByte()),(unsigned long) (RES_HDD->getCurByte()/(double)1000/1000),
	"HDD write latency",HDD_LATENCY,
	"HDD write bandwidth",HDD_BANDWIDTH/1024/1024);

    // release resource
	for(uint32_t i=0;i<_disk_run_list_row;i++){
		for (uint32_t j = 0; j < _disk_run_list_col; j++) {
			if (_disk_run_list[i][j]) {
				delete _disk_run_list[i][j];
				_disk_run_list[i][j] = nullptr;
			}
		}
		delete [] _disk_run_list[i];
	}
	delete [] _disk_run_list;

	delete _loser_tree;

	delete [] _each_group_col;
	delete [] _group_offset;

	delete _shared_buffer;

    delete SSD;
	delete HDD;
	delete RES_HDD;
} // ScanPlan::~ScanPlan

void DiskScan::ReadFromDisk(){
    //construct _disk_run_list
    //ssd part
    for(uint32_t group_num =0; group_num < _ssd_each_group_row.size(); group_num ++){
		
		BatchSize read_size;
        char* buffer = SSD->read(group_num , _row_size, _ssd_each_group_row, _batch_size, _group_offset[group_num], &read_size);
		_each_group_col[group_num] = read_size;
		_group_offset[group_num] += read_size; //记录每个组读到哪里了
        Bytes2DiskRecord(buffer , group_num);
		_current_run_index++;
    }
    //hdd part
    for(uint32_t group_num = 0; group_num < _hdd_each_group_row.size() ; group_num ++){
		
		BatchSize read_size;
        char* buffer = HDD->read(group_num , _row_size, _hdd_each_group_row, _batch_size, _group_offset[group_num + _ssd_each_group_row.size()], &read_size);
		_each_group_col[group_num + _ssd_each_group_row.size()] = read_size;
		_group_offset[group_num + _ssd_each_group_row.size()] += read_size; //记录每个组读到哪里了
        Bytes2DiskRecord(buffer , group_num + _ssd_each_group_row.size());
        _current_run_index++;
    }
}

void DiskScan::RefillRow(uint32_t group_num){
    //construct _disk_run_list
    //ssd part
    if (group_num < _ssd_each_group_row.size()){
		// 可以读下一整个batch
		if (_ssd_each_group_row[group_num] - _group_offset[group_num] >= _batch_size){
			BatchSize read_size;
			char* buffer = SSD->read(group_num , _row_size, _ssd_each_group_row, _batch_size, _group_offset[group_num], &read_size);
			_each_group_col[group_num] = read_size;
			Bytes2DiskRecord(buffer , group_num);
			_group_offset[group_num] += read_size; //记录每个组读到哪里了
		} else{
			BatchSize read_size;
			char* buffer = SSD->read(group_num , _row_size, _ssd_each_group_row, _ssd_each_group_row[group_num] - _group_offset[group_num], 
				_group_offset[group_num], &read_size);
			_each_group_col[group_num] = read_size;
			Bytes2DiskRecord(buffer , group_num);
			_group_offset[group_num] += read_size;
		}
    } else{
		if (_hdd_each_group_row[group_num-_ssd_each_group_row.size()] - _group_offset[group_num] >= _batch_size){
			BatchSize read_size;
			char* buffer = HDD->read(group_num-_ssd_each_group_row.size(), _row_size, _hdd_each_group_row, _batch_size, _group_offset[group_num], &read_size);
			_each_group_col[group_num] = read_size;
			Bytes2DiskRecord(buffer , group_num);
			_group_offset[group_num] += read_size; //记录每个组读到哪里了
		}else{
			BatchSize read_size;
			
			char* buffer = HDD->read(group_num-_ssd_each_group_row.size(), _row_size, _hdd_each_group_row,_hdd_each_group_row[group_num-_ssd_each_group_row.size()] - _group_offset[group_num], _group_offset[group_num], &read_size);
			_each_group_col[group_num] = read_size;
			Bytes2DiskRecord(buffer , group_num);
			_group_offset[group_num] += read_size;
		}
    }
}

void DiskScan::MultiwayMerge(){
	// check full or finish and get the column number in the last row
	// reset loser tree
	_loser_tree->reset(_current_run_index, _loser_tree->getMinItem());

	// 初始基准字符串为空
	std::string* base_str_ptr = nullptr; 

	// Initialize with the first element of each sorted sequence
	for (uint32_t i = 0; i < _disk_run_list_row; i++) {	
		_loser_tree->push(_disk_run_list[i][0], i, 0, base_str_ptr);
	}
	uint64_t count = 0;
	bool isFinish = false;
	_shared_buffer->reset();
	std::thread resConsumeThread(&SharedBuffer::resConsume, _shared_buffer, RES_HDD);
	while (!_loser_tree->empty()) {
		// get smallest element
		TreeNode* cur = _loser_tree->top();
		Item cur_item = *(cur->_value);

		// get the string of current data
		std::string tmp_base_str(cur->_value->GetItemString());
		base_str_ptr = &tmp_base_str;

		// calculate the index of next data item 
		uint32_t run_index = cur->_run_index;
		uint32_t element_index = cur->_element_index + 1;

		// calculate the last index in the target run
		uint32_t target_element_index = _each_group_col[run_index];
		// push next data into the tree
		if (element_index < target_element_index) {
			_loser_tree->push(_disk_run_list[run_index][element_index], run_index, element_index, base_str_ptr);
		}else if (run_index <_ssd_each_group_row.size()){
			if (_group_offset[run_index] < _ssd_each_group_row[run_index]){
			RefillRow(run_index);
			element_index = 0;
            _loser_tree->push(_disk_run_list[run_index][element_index], run_index, element_index, base_str_ptr);
			}else{
			_loser_tree->push(_loser_tree->getMaxItem(), run_index, -1, base_str_ptr);
			}
		}else if (run_index >= _ssd_each_group_row.size()) {
			if (_group_offset[run_index] < _hdd_each_group_row[run_index-_ssd_each_group_row.size()]){
			RefillRow(run_index);
			element_index = 0;
            _loser_tree->push(_disk_run_list[run_index][element_index], run_index, element_index, base_str_ptr);
			}else{
			_loser_tree->push(_loser_tree->getMaxItem(), run_index, -1, base_str_ptr);
			}
		}
		if(_loser_tree->empty()){
			isFinish = true;
		}
		// save in results
		count++;
		_shared_buffer->produce(cur_item, isFinish);
	}
	resConsumeThread.join();
}

// 把buffer里的数据填进disk_run_list
void DiskScan::Bytes2DiskRecord(char* buffer, uint32_t group_num){
    int element_size = _row_size /3;
	char* block = buffer;
    for (int col = 0; col< _each_group_col[group_num]; col++){
		char* incl = new char[element_size];
		memset(incl, 0, element_size);
		char* mem = new char[element_size];
		memset(mem, 0, element_size);
		char* mgmt = new char[_row_size - element_size *2];
		memset(mgmt, 0, _row_size - element_size *2);
		for(int i  = 0 ; i< element_size-1; i ++){
			incl[i] = buffer[i];
		}
		for(int i  = 0 ; i< element_size-1; i ++){
			mem[i] = buffer[i + element_size];
		}
		for(int i  = 0 ; i< _row_size - element_size *2-1; i ++){
			mgmt[i] = buffer[i+ element_size*2];
		}
        buffer += _row_size;
        Item * temp = new Item(incl,mem,mgmt);
		if (_disk_run_list[group_num][col])
			delete _disk_run_list[group_num][col];
		_disk_run_list[group_num][col] = temp;
    }
	delete [] block;
	block = nullptr;
}