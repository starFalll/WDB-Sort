#include "Sort.h"
#include <algorithm>

SortPlan::SortPlan (Plan * const input) : Plan(input->GetSize()), _input (input)
{
} // SortPlan::SortPlan

SortPlan::~SortPlan ()
{
	delete _input;
} // SortPlan::~SortPlan

Iterator * SortPlan::init () const
{
	//TRACE (TRACE_SWITCH);
	return new SortIterator (this);
} // SortPlan::init


SortIterator::SortIterator (SortPlan const * const plan) :
	Iterator(plan->GetSize()), _plan (plan), 
	_input (plan->_input->init ()), _consumed (0), _produced (0), 
	_cache_run_list_row((MAX_DRAM * 5 / 10) / (MAX_CPU_CACHE * 5 / 10)),
	_cache_run_list_col(MAX_CPU_CACHE * 5 / 10 / _row_size)
{
	//TRACE (TRACE_SWITCH);

	// init producer consumer
	_shared_buffer = new SharedBuffer(OUTPUT_BUFFER, _row_size);

	// allocate 0.5MB to sort (use CPU Cache)
	_sort_records.resize (MAX_CPU_CACHE * 5 / 10 / _row_size, Item(plan->_input->GetSize(), '0'));

	// initialize current run index
	_current_run_index = 0;
	// initialize run list
	_cache_run_list = new Item**[_cache_run_list_row];
	for(uint32_t i=0;i<_cache_run_list_row;i++){
		_cache_run_list[i] = new Item*[_cache_run_list_col];
		memset(_cache_run_list[i], 0, sizeof(Item*) * _cache_run_list_col);
	}

	_sort_index = 0;
	_last_consumed = 0;

	// initialize loser of tree
	_loser_tree = new LoserTree(_cache_run_list_row, _row_size);
} // SortIterator::SortIterator

SortIterator::~SortIterator ()
{
	//TRACE (TRACE_SWITCH);

	// release resource
	for(uint32_t i=0;i<_cache_run_list_row;i++){
		for (uint32_t j = 0; j < _cache_run_list_col; j++) {
			delete _cache_run_list[i][j];
		}
		delete [] _cache_run_list[i];
	}
	delete [] _cache_run_list;

	delete _input;

	delete _shared_buffer;

	delete _loser_tree;

	printf ("||------------------------In-memory Sort------------------------||\n"
		"||%-25s| %12lu of%12lu Records||\n",
		"Records sorted",(unsigned long) (_produced),(unsigned long) (_consumed));
} // SortIterator::~SortIterator

bool SortIterator::next ()
{
	//TRACE (TRACE_SWITCH);
	
	bool ret = false;
	if ((ret = _input->next ())) {
		std::vector<Item> * filter_records;
		uint32_t * filter_index;
		_input->GetRecords(&filter_records, &filter_index);
		_sort_index = (++ _sort_index) % _sort_records.size ();
		_sort_records [_sort_index] = filter_records->at (*filter_index);
		++ _consumed;
	}
	//TRACE (TRACE_SWITCH);
	// _sort_records is fulled
	if (_consumed>_last_consumed && (!ret || (0 == _sort_index))) {
		uint32_t add_num = _sort_index == 0 ? (RowCount)_sort_records.size() : _sort_index+1;
		// when not full, valid value from index 1
		uint32_t begin_num = _sort_index == 0 ? 0 : 1;
		// because quicksort's sequential and localized memory references work well with a cache
		QuickSort(_sort_records.begin() + begin_num, _sort_records.begin() + add_num, [] (const Item & a, const Item & b) {
			// TODO supporting group by
			// temporarily sorting by first field
			return a < b;
		});
		
		// save sorted 0.5M data in memory
		for(uint32_t i=0, j = begin_num;j<add_num;i++, j++){
			if (_cache_run_list[_current_run_index][i]) {
				delete _cache_run_list[_current_run_index][i];
				_cache_run_list[_current_run_index][i] = nullptr;
			}
			_cache_run_list[_current_run_index][i] = new Item(_sort_records[j]);
		}
		_current_run_index++;

		_produced += add_num - begin_num;
		_last_consumed = _consumed;
		//TRACE (TRACE_SWITCH);
	}

	// run list is full or finish, start to merge
	if((_current_run_index >= _cache_run_list_row) || (_consumed > 0 && !ret)){
		MultiwayMerge();
		_current_run_index = 0;
	}
	// ++ _produced;
	return ret;
} // SortIterator::next


template <typename RandomIt, typename Compare>
void SortIterator::QuickSort (RandomIt start, RandomIt end, Compare comp)
{
	if (start >= end - 1) return;
	RandomIt pivot = getmid (start, end - 1, start + (end - start) / 2);
	swap (* pivot, * (end-1));
	pivot = end - 1;
	auto left = start, right = end - 2;
	while (left < right) {
		while (left < right && !comp ( * right, * pivot)) { // * right >= * pivot
			right--;
		}
		while (left < right && comp ( * left, * pivot)) {
			left++;
		}
		swap( * left, * right);
	}
	if (comp( * left, * pivot)) {
		left++;
	}
	swap(*left, *pivot);
	
	QuickSort (start, left, comp);
	QuickSort (left+1, end, comp);
}

void SortIterator::MultiwayMerge (){
	// check full or finish and get the column number in the last row
	uint32_t last_row_col = (_sort_index == 0) ? _cache_run_list_col : _sort_index;
	// reset loser tree
	_loser_tree->reset(_current_run_index, _loser_tree->getMinItem());

	// 初始基准字符串为空
	std::string* base_str_ptr = nullptr; 

	// Initialize with the first element of each sorted sequence
	for (uint32_t i = 0; i < _current_run_index; i++) {	
		_loser_tree->push(_cache_run_list[i][0], i, 0, base_str_ptr);
	}


	// merge finish symbol
	bool isFinish = false;
	// reset shared buffer
	_shared_buffer->reset();
	// create consume thread
	std::thread cyclicalConsumeThread(&SharedBuffer::cyclicalConsume, _shared_buffer, SSD_TEMP, HDD_TEMP);
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
		uint32_t target_element_index = (run_index == _current_run_index-1) ? last_row_col : _cache_run_list_col;
		// push next data into the tree
		if (element_index < target_element_index) {
			_loser_tree->push(_cache_run_list[run_index][element_index], run_index, element_index, base_str_ptr);
		}else{
			// push infinity
			_loser_tree->push(_loser_tree->getMaxItem(), run_index, -1, base_str_ptr);
		}

		if(_loser_tree->empty()){
			isFinish = true;
		}
		// save in results
		_shared_buffer->produce(*(cur->_value), isFinish);
	}
	cyclicalConsumeThread.join();
}