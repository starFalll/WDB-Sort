#include "Sort.h"
#include <algorithm>

SortPlan::SortPlan (Plan * const input) : _input (input)
{
	TRACE (TRACE_SWITCH);
} // SortPlan::SortPlan

SortPlan::~SortPlan ()
{
	TRACE (TRACE_SWITCH);
	delete _input;
} // SortPlan::~SortPlan

Iterator * SortPlan::init () const
{
	TRACE (TRACE_SWITCH);
	return new SortIterator (this);
} // SortPlan::init


SortIterator::SortIterator (SortPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0),
	_cache_run_list_row((MAX_DRAM * 8 / 10) / (MAX_CPU_CACHE * 5 / 10)),
	_cache_run_list_col(MAX_CPU_CACHE * 5 / 10 / sizeof(Item))
{
	TRACE (TRACE_SWITCH);

	// init producer consumer
	_pc = new ProducerConsumer(OUTPUT_BUFFER / sizeof(Item));
	SSD = new File(SSD_PATH, MAX_SSD);
	HDD = new File(HDD_PATH, __LONG_LONG_MAX__);

	// allocate 90MB to sort
	// _sort_records.resize (MAX_DRAM * 9 / 10 / sizeof(Item));
	// allocate 0.5MB to sort (use CPU Cache)
	_sort_records.resize (MAX_CPU_CACHE * 5 / 10 / sizeof(Item));

	// initialize current run index
	_current_run_index = 0;
	// initialize run list
	_cache_run_list = new Item**[_cache_run_list_row];
	for(uint32_t i=0;i<_cache_run_list_row;i++){
		_cache_run_list[i] = new Item*[_cache_run_list_col];
	}

	_sort_index = 0;

	// initialize loser of tree
	_loser_tree = new LoserTree(_cache_run_list_row);
} // SortIterator::SortIterator

SortIterator::~SortIterator ()
{
	TRACE (TRACE_SWITCH);

	// release resource
	for(uint32_t i=0;i<_cache_run_list_row;i++){
		delete _cache_run_list[i];
	}
	delete [] _cache_run_list;

	delete _input;

	delete _pc;
	delete SSD;
	delete HDD;

	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_produced),
			(unsigned long) (_consumed));
} // SortIterator::~SortIterator

bool SortIterator::next ()
{
	TRACE (TRACE_SWITCH);

	bool ret = false;
	// if (_produced >= _consumed)  return false;
	if ((ret = _input->next ())) {
		std::vector<Item> * filter_records;
		uint32_t * filter_index;
		_input->GetRecords(&filter_records, &filter_index);
		_sort_index = (++ _sort_index) % _sort_records.size ();
		_sort_records [_sort_index] = filter_records->at (*filter_index);
		TRACE_ITEM (TRACE_SWITCH, _sort_records [_sort_index].fields[INCL], _sort_records [_sort_index].fields[MEM], _sort_records [_sort_index].fields[MGMT]);
		++ _consumed;
	}
	TRACE (TRACE_SWITCH);
	// _sort_records is fulled
	if ((!ret && _consumed > 0) || 0 == _sort_index) {
		uint32_t add_num = _sort_index == 0 ? (RowCount)_sort_records.size() : _sort_index;
		// because quicksort's sequential and localized memory references work well with a cache
		QuickSort(_sort_records.begin(), _sort_records.begin() + add_num, [] (const Item & a, const Item & b) {
			// TODO supporting group by
			// temporarily sorting by first field
			return a.fields[COMPARE_FIELD] < b.fields[COMPARE_FIELD];
		});
		
		// save sorted 0.5M data in memory
		for(uint32_t i=0;i<add_num;i++){
			_cache_run_list[_current_run_index][i] = new Item(_sort_records[i]);
		}
		// count current run index
		_current_run_index++;

		_produced += add_num;
		TRACE (TRACE_SWITCH);
		// printf("test index:%u\n", _sort_index);
		// for (int i = 0; i < add_num; i++) {
		// 	const auto& item = _sort_records[i];
		// 	traceprintf ("test result: %d\n",item.fields[INCL]);
		// }
		// TODO asynchronously writing buffer into SSD or HDD

	}

	// run list is full or finish, start to merge
	if((_current_run_index >= _cache_run_list_row) || (!ret && _consumed > 0)){
		_loser_tree->reset(_cache_run_list_row, &ITEM_MAX);
		MultiwayMerge();
		_current_run_index = 0;
	}

	// ++ _produced;
	return ret;
} // SortIterator::next


void SortIterator::GetRecords(std::vector<Item> ** records, uint32_t ** index)
{
	*records = &_sort_records;
	*index = &_sort_index;
}

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
	_loser_tree->reset(_current_run_index, &ITEM_MIN);

	// Initialize with the first element of each sorted sequence
	for (uint32_t i = 0; i < _current_run_index; i++) {	
		_loser_tree->push(_cache_run_list[i][0], i, 0);
	}

	// reset result index
	int32_t res_index = 0;

	std::thread cyclicalConsumeThread(&ProducerConsumer::cyclicalConsume, _pc, SSD, HDD);
	while (!_loser_tree->empty()) {
		// get smallest element
		TreeNode* cur = _loser_tree->top();

		// save in results
		// _results[res_index] = *(cur->_value);
		_pc->produce(*(cur->_value));
		res_index++;

		// calculate the index of next data item 
		uint32_t run_index = cur->_run_index;
		uint32_t element_index = cur->_element_index + 1;

		// calculate the last index in the target run
		uint32_t target_element_index = (run_index == _current_run_index-1) ? last_row_col : _cache_run_list_col;
		// push next data into the tree
		if (element_index < target_element_index) {
			_loser_tree->push(_cache_run_list[run_index][element_index], run_index, element_index);
		}else{
			_loser_tree->push(&ITEM_MAX, run_index, -1);
		}
	}
	cyclicalConsumeThread.join();
}



