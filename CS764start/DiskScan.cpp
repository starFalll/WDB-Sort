#include "DiskScan.h"

DiskScan::DiskScan(GroupCount const ssd_group_count, GroupCount const hdd_group_count, RowSize const row_size, RowCount const each_group_row_count, BatchSize const batch_size):
    _ssd_group_count(ssd_group_count), _hdd_group_count(hdd_group_count) , _row_size(row_size) , _each_group_row_count(each_group_row_count) , _batch_size(batch_size),
    _disk_run_list_row(ssd_group_count + hdd_group_count), _disk_run_list_col(batch_size)
{
    TRACE (TRACE_SWITCH);
    SSD = new File(SSD_PATH, MAX_SSD, SSD_BLOCK);
	HDD = new File(HDD_PATH, __LONG_LONG_MAX__, HDD_BLOCK);
    RES_HDD = new File(RES_HDD_PATH, __LONG_LONG_MAX__, HDD_BLOCK);

    _shared_buffer = new SharedBuffer(OUTPUT_BUFFER / sizeof(Item));
    // initialize current run index
	_current_run_index = 0;
	// initialize run list
	_disk_run_list = new Item**[_disk_run_list_row];
	for(uint32_t i=0;i<_disk_run_list_row;i++){
		_disk_run_list[i] = new Item*[_disk_run_list_col];
	}

    // initialize loser of tree
	_loser_tree = new LoserTree(_disk_run_list_row);

    // merge
	if((_current_run_index >= _disk_run_list_row)){
		_loser_tree->reset(_disk_run_list_row, &ITEM_MAX);
		MultiwayMerge();
		_current_run_index = 0;
	}
}

DiskScan::~DiskScan ()
{
    traceprintf ("diskscanssd %lu,scanhdd %lu",
        (unsigned long) (_ssd_group_count),
        (unsigned long) (_hdd_group_count));
	TRACE (TRACE_SWITCH);

    for(uint32_t i=0;i<_disk_run_list_row;i++){
        for(uint32_t j=0;j<_disk_run_list_col;j++){
            std::cout<<std::endl;
		    if(_disk_run_list[i][j]!=nullptr){
                Item temp = *_disk_run_list[i][j];
                std::cout<<temp.fields[INCL]+" ";
                std::cout<<temp.fields[MEM]+" ";
                std::cout<<temp.fields[MGMT]+" ";
            }
	    }
	}

    // release resource
	for(uint32_t i=0;i<_disk_run_list_row;i++){
		delete _disk_run_list[i];
	}
	delete [] _disk_run_list;

    delete SSD;
	delete HDD;
} // ScanPlan::~ScanPlan

void DiskScan::ReadFromDisk(){
    //construct _disk_run_list
    //ssd part
    for(uint32_t group_num =0; group_num < _ssd_group_count; group_num ++){
        char* buffer = SSD->read(group_num , _each_group_row_count, _batch_size);
        Bytes2DiskRecord(buffer , group_num);
		_current_run_index++;
    }
    //hdd part
    for(uint32_t group_num = 0; group_num < _hdd_group_count ; group_num ++){
        char* buffer = HDD->read(group_num , _each_group_row_count, _batch_size);
        Bytes2DiskRecord(buffer , group_num);
        _current_run_index++;
    }
}

void DiskScan::RefillRow(uint32_t group_num){
    //construct _disk_run_list
    //ssd part
    if (group_num < _ssd_group_count){
        char* buffer = SSD->read(group_num , _each_group_row_count, _batch_size);
        Bytes2DiskRecord(buffer , group_num);
    } else{
        char* buffer = HDD->read(group_num , _each_group_row_count, _batch_size);
        Bytes2DiskRecord(buffer , group_num);
    }
}

void DiskScan::MultiwayMerge(){
	// check full or finish and get the column number in the last row
	uint32_t last_row_col =  _disk_run_list_col;
	// reset loser tree
	_loser_tree->reset(_disk_run_list_row, &ITEM_MIN);

	// 初始基准字符串为空
	const StringFieldType* base_str_ptr = nullptr; 

	// Initialize with the first element of each sorted sequence
	for (uint32_t i = 0; i < _disk_run_list_row; i++) {	
		_loser_tree->push(_disk_run_list[i][0], i, 0, base_str_ptr);
	}

	// reset result index
	int32_t res_index = 0;
	bool isFinish = false;
	_shared_buffer->reset();
	std::thread resConsumeThread(&SharedBuffer::resConsume, _shared_buffer, RES_HDD);
	while (!_loser_tree->empty()) {
		// get smallest element
		TreeNode* cur = _loser_tree->top();

		// get the string of current data
		base_str_ptr = cur->_value->GetItemString();

		// calculate the index of next data item 
		uint32_t run_index = cur->_run_index;
		uint32_t element_index = cur->_element_index + 1;

		// calculate the last index in the target run
		uint32_t target_element_index = _disk_run_list_col;
		// push next data into the tree
		if (element_index < target_element_index) {
			_loser_tree->push(_disk_run_list[run_index][element_index], run_index, element_index, base_str_ptr);
		}else{
			Item temp = Item(_row_size);
			_loser_tree->push(&temp, run_index, -1, base_str_ptr);
			//_loser_tree->push(&ITEM_MAX, -1, -1);
		}

		if(_loser_tree->empty()){
			isFinish = true;
		}
		// save in results
		_shared_buffer->produce(*(cur->_value), isFinish);
		res_index++;
	}
	resConsumeThread.join();
}

void DiskScan::Bytes2DiskRecord(char* buffer, uint32_t group_num){
    int element_size = _row_size /3;
    for (int col = 0; col<_batch_size; col++){
        // 转换为三个对应长度的std::string
        Item * temp = (Item*)buffer;
        // std::string incl(buffer, element_size);
        // std::string mem (buffer + element_size , element_size);
        // std::string mgmt(buffer + element_size*2 , _row_size - element_size*2);
        buffer += sizeof(Item);
        //Item * temp = new Item(incl,mem,mgmt);
        if(group_num<_ssd_group_count){
            _disk_run_list[group_num][col] = temp;
        } else{
            _disk_run_list[group_num + _ssd_group_count][col] = temp;
        }
    }
}