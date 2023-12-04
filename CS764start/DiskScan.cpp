#include "DiskScan.h"

DiskScan::DiskScan(GroupCount const ssd_group_count, GroupCount const hdd_group_count, RowSize const row_size, RowCount const each_group_row_count, BatchSize const batch_size):
    _ssd_group_count(ssd_group_count), _hdd_group_count(hdd_group_count) , _row_size(row_size) , _each_group_row_count(each_group_row_count) , _batch_size(batch_size),
    _disk_run_list_row(ssd_group_count + hdd_group_count), _disk_run_list_col(batch_size)
{
    TRACE (TRACE_SWITCH);
    SSD = new File(SSD_PATH_TEMP);
	HDD = new File(HDD_PATH_TEMP);
    RES_HDD = new File(RES_HDD_PATH, __LONG_LONG_MAX__, HDD_BLOCK);

    _shared_buffer = new SharedBuffer(OUTPUT_BUFFER, _row_size);
    // initialize current run index
	_current_run_index = 0;
	// initialize run list
	_disk_run_list = new Item**[_disk_run_list_row];
	for(uint32_t i=0;i<_disk_run_list_row;i++){
		_disk_run_list[i] = new Item*[_disk_run_list_col];
	}

	_each_group_col = new uint32_t[_ssd_group_count + _hdd_group_count];
	//initialize group offset
	_group_offset = new uint32_t[_ssd_group_count + _hdd_group_count];

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
    traceprintf ("diskscanssd %lu,scanhdd %lu",
        (unsigned long) (_ssd_group_count),
        (unsigned long) (_hdd_group_count));
	TRACE (TRACE_SWITCH);

    for(uint32_t i=0;i<_disk_run_list_row;i++){
        for(uint32_t j=0;j<_each_group_col[i];j++){
            std::cout<<std::endl;
		    if(_disk_run_list[i][j]!=nullptr){
                Item temp = *_disk_run_list[i][j];
                std::cout<<std::string(temp.fields[INCL])+" ";
                std::cout<<std::string(temp.fields[MEM])+" ";
                std::cout<<std::string(temp.fields[MGMT])+" ";
            }
	    }
		std::cout<<std::endl;
	}

    // release resource
	for(uint32_t i=0;i<_disk_run_list_row;i++){
		delete _disk_run_list[i];
	}
	delete [] _disk_run_list;

	delete [] _each_group_col;
	delete [] _group_offset;

    delete SSD;
	delete HDD;
	delete RES_HDD;
} // ScanPlan::~ScanPlan

void DiskScan::ReadFromDisk(){
    //construct _disk_run_list
    //ssd part
    for(uint32_t group_num =0; group_num < _ssd_group_count; group_num ++){
		_each_group_col[group_num] = _batch_size;
        char* buffer = SSD->read(group_num , _row_size, _each_group_row_count, _batch_size, _group_offset[group_num]);
		_group_offset[group_num] += _batch_size; //记录每个组读到哪里了
        Bytes2DiskRecord(buffer , group_num);
		_current_run_index++;
    }
    //hdd part
    for(uint32_t group_num = 0; group_num < _hdd_group_count ; group_num ++){
		_each_group_col[group_num] = _batch_size;
        char* buffer = HDD->read(group_num , _row_size, _each_group_row_count, _batch_size, _group_offset[group_num]);
		_group_offset[group_num] += _batch_size; //记录每个组读到哪里了
        Bytes2DiskRecord(buffer , group_num);
        _current_run_index++;
    }
}

void DiskScan::RefillRow(uint32_t group_num){
    //construct _disk_run_list
    //ssd part
    if (group_num < _ssd_group_count){
		if (_each_group_row_count - _group_offset[group_num] >= _batch_size){
			_each_group_col[group_num] = _batch_size;
			char* buffer = SSD->read(group_num , _row_size, _each_group_row_count, _batch_size, _group_offset[group_num]);
			Bytes2DiskRecord(buffer , group_num);
			_group_offset[group_num] += _batch_size; //记录每个组读到哪里了
		} else{
			_each_group_col[group_num] = _each_group_row_count - _group_offset[group_num];
			char* buffer = SSD->read(group_num , _row_size, _each_group_row_count, _each_group_row_count - _group_offset[group_num], _group_offset[group_num]);
			Bytes2DiskRecord(buffer , group_num);
			_group_offset[group_num] = _each_group_row_count;
		}
    } else{
		if (_each_group_row_count - _group_offset[group_num] >= _batch_size){
			_each_group_col[group_num] = _batch_size;
			char* buffer = HDD->read(group_num , _row_size, _each_group_row_count, _batch_size, _group_offset[group_num]);
			Bytes2DiskRecord(buffer , group_num);
			_group_offset[group_num] += _batch_size; //记录每个组读到哪里了
		}else{
			_each_group_col[group_num] = _each_group_row_count - _group_offset[group_num];
			char* buffer = HDD->read(group_num , _row_size, _each_group_row_count, _each_group_row_count - _group_offset[group_num], _group_offset[group_num]);
			Bytes2DiskRecord(buffer , group_num);
			_group_offset[group_num] = _each_group_row_count;
		}
    }
}

void DiskScan::MultiwayMerge(){
	// check full or finish and get the column number in the last row
	uint32_t last_row_col =  _disk_run_list_col;
	// reset loser tree
	_loser_tree->reset(_current_run_index, _loser_tree->getMinItem());

	// 初始基准字符串为空
	std::string ovc_negative_infinity(_row_size/3, '0');
	std::string* base_str_ptr = &ovc_negative_infinity; 

	// Initialize with the first element of each sorted sequence
	for (uint32_t i = 0; i < _disk_run_list_row; i++) {	
		_loser_tree->push(_disk_run_list[i][0], i, 0, base_str_ptr);
	}
	// for (uint32_t i = 0; i < _disk_run_list_row * 2; i++) {	
	// 	std::cout<< _loser_tree->getvalue(i) <<std::endl;
	// }

	bool isFinish = false;
	_shared_buffer->reset();
	std::thread resConsumeThread(&SharedBuffer::resConsume, _shared_buffer, RES_HDD);
	while (!_loser_tree->empty()) {
		// get smallest element
		TreeNode* cur = _loser_tree->top();

		// get the string of current data
		std::string tmp_base_str(cur->_value->GetItemString());
		base_str_ptr = &tmp_base_str;

		// calculate the index of next data item 
		uint32_t run_index = cur->_run_index;
		uint32_t element_index = cur->_element_index + 1;

		// calculate the last index in the target run
		//uint32_t target_element_index = _disk_run_list_col;
		uint32_t target_element_index = _each_group_col[run_index];

		// push next data into the tree
		if (element_index < target_element_index) {
			std::cout<<_disk_run_list[run_index][element_index]->fields[0]<<std::endl;
			_loser_tree->push(_disk_run_list[run_index][element_index], run_index, element_index, base_str_ptr);
		}else if (_group_offset[run_index] < _each_group_row_count) {
			RefillRow(run_index);
			element_index = 0;
            std::cout<<_disk_run_list[run_index][element_index]->fields[0]<<std::endl;
            _loser_tree->push(_disk_run_list[run_index][element_index], run_index, element_index, base_str_ptr);
		} else{
			Item temp = Item(_row_size,'9');
			_loser_tree->push(&temp, run_index, -1, base_str_ptr);
			//_loser_tree->push(&ITEM_MAX, -1, -1);
		}

		if(_loser_tree->empty()){
			// for (uint32_t i = 0; i < _disk_run_list_row * 2; i++) {	
			// 	std::cout<< _loser_tree->getvalue(i) <<std::endl;
			// }
			isFinish = true;
		}
		// save in results
		_shared_buffer->produce(*(cur->_value), isFinish);
	}
	resConsumeThread.join();
}

void DiskScan::Bytes2DiskRecord(char* buffer, uint32_t group_num){
    int element_size = _row_size /3;
	//重新分配该行的长度
	// delete _disk_run_list[group_num];
	// _disk_run_list[group_num] = new Item*[_each_group_col[group_num]];
    for (int col = 0; col< _each_group_col[group_num]; col++){
		char* incl = new char[element_size+1];
		memset(incl, 0, element_size+1);
		char* mem = new char[element_size+1];
		memset(mem, 0, element_size+1);
		char* mgmt = new char[_row_size - element_size *2+1];
		memset(mgmt, 0, _row_size - element_size *2+1);
		for(int i  = 0 ; i< element_size; i ++){
			incl[i] = buffer[i];
		}
		for(int i  = 0 ; i< element_size; i ++){
			mem[i] = buffer[i + element_size];
		}
		for(int i  = 0 ; i< _row_size - element_size *2; i ++){
			mgmt[i] = buffer[i+ element_size*2];
		}
        // std::string incl(buffer, element_size);
        // std::string mem (buffer + element_size , element_size);
        // std::string mgmt(buffer + element_size*2 , _row_size - element_size*2);
        buffer += _row_size;
        Item * temp = new Item(incl,mem,mgmt);
        
		_disk_run_list[group_num][col] = temp;
    }
}