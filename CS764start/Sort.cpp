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
	_consumed (0), _produced (0)
{
	TRACE (TRACE_SWITCH);

	// allocate 90MB to sort
	_sort_records.resize (MAX_DRAM * 9 / 10 / sizeof(Item));
	_sort_index = 0;
	// while (_input->next ())  ++ _consumed;
	// delete _input;

	// traceprintf ("consumed %lu rows\n",
	// 		(unsigned long) (_consumed));
} // SortIterator::SortIterator

SortIterator::~SortIterator ()
{
	TRACE (TRACE_SWITCH);
	delete _input;
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

	// _sort_records is fulled
	if ((!ret && _consumed > 0) || 0 == _sort_index) {
		// because quicksort's sequential and localized memory references work well with a cache
		std::sort(_sort_records.begin(), _sort_records.end(), [] (const Item & a, const Item & b) {
			// TODO supporting group by
			// temporarily sorting by first field
			return a.fields[INCL] < b.fields[INCL];
		});
		_produced += _sort_index == 0 ? (RowCount)_sort_records.size() : _sort_index;
		TRACE (TRACE_SWITCH);
		for (int i = 0; i < _sort_index; i++) {
			const auto& item = _sort_records[i];
			traceprintf ("test %d\n",item.fields[INCL]);
		}
		// TODO asynchronously writing buffer into SSD or HDD

	}
	// ++ _produced;
	return ret;
} // SortIterator::next


void SortIterator::GetRecords(std::vector<Item> ** records, uint32_t ** index)
{
	*records = &_sort_records;
	*index = &_sort_index;
}