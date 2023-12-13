#include "Filter.h"

FilterPlan::FilterPlan (Plan * const input) : Plan(input->GetSize()), _input (input)
{
	////TRACE (TRACE_SWITCH);
} // FilterPlan::FilterPlan

FilterPlan::~FilterPlan ()
{
	////TRACE (TRACE_SWITCH);
	delete _input;
} // FilterPlan::~FilterPlan

Iterator * FilterPlan::init () const
{
	////TRACE (TRACE_SWITCH);
	return new FilterIterator (this);
} // FilterPlan::init

/*
bool FilterPlan::SetPredicate (ItemField field, PredicateEnum predicate, FieldType value) 
{
	//TRACE (TRACE_SWITCH);
	Predicate new_condition (field, predicate, value);
	_predicates [field].push_back (new_condition);
	return true;
}
*/

bool FilterPlan::SetPredicate (ItemField field, PredicateEnum predicate, FieldType value) 
{
	////TRACE (TRACE_SWITCH);
	Predicate new_condition (field, predicate, value);
	_predicates [field].push_back (new_condition);
	return true;
}

bool FilterPlan::ApplyPredicate (Item & item) const
{
	// if do not have conditions, don't do filter
	if (_predicates.empty()) return true;

	// for (const auto & field : ITEM_FIELDS) {
	// 	auto it = _predicates.find (field);
	// 	if (it != _predicates.end ()) {
	// 		for (const auto & predicate : it->second) {
	// 			// if ((predicate.predicate == EQ && stoull(item.fields [field]) == stoull(predicate.value))
	// 			// 	|| (predicate.predicate == GT && stoull(item.fields [field]) > stoull(predicate.value))
	// 			// 	|| (predicate.predicate == LT && stoull(item.fields [field]) < stoull(predicate.value))
	// 			// 	|| (predicate.predicate == GE && stoull(item.fields [field]) >= stoull(predicate.value))
	// 			// 	|| (predicate.predicate == LE && stoull(item.fields [field]) <= stoull(predicate.value))) {
	// 			// 	return true;
	// 			// }
	// 			return true;
	// 		}
	// 	}
		
	// }
	return true;
}

FilterIterator::FilterIterator (FilterPlan const * const plan) :
	Iterator(plan->GetSize()), _plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0)
{
	////TRACE (TRACE_SWITCH);

	// allocate 1MB to filter
	_filter_records.resize(MAX_CPU_CACHE * 1 / _row_size, Item(plan->_input->GetSize(), '0'));
	_filter_index = 0;
} // FilterIterator::FilterIterator

FilterIterator::~FilterIterator ()
{
	////TRACE (TRACE_SWITCH);

	delete _input;

	printf ("||-------------------------Filter Data--------------------------||\n"
		"||%-25s| %12lu of%12lu Records||\n",
		"Records filtered",(unsigned long) (_produced),(unsigned long) (_consumed));
	// traceprintf ("\nproduced %lu of %lu rows\n\n",
	// 		(unsigned long) (_produced),
	// 		(unsigned long) (_consumed));
} // FilterIterator::~FilterIterator

bool FilterIterator::next ()
{
	////TRACE (TRACE_SWITCH);

	do
	{
		if ( ! _input->next ())  return false;
		++ _consumed;
	} while (!ApplyCondition());
	// read scan cur record
	std::vector<Item> * scan_records;
	uint32_t * scan_index;
	_input->GetRecords(&scan_records, &scan_index);
	auto& item = scan_records->at (*scan_index);
	//TRACE_ITEM (TRACE_SWITCH, item.fields[INCL], item.fields[MEM], item.fields[MGMT]);
	// write into filter record
	_filter_index = (++ _filter_index) % _filter_records.size ();
	_filter_records [_filter_index] = item;
	++ _produced;
	return true;
} // FilterIterator::next

void FilterIterator::GetRecords(std::vector<Item> ** records, uint32_t ** index)
{
	*records = &_filter_records;
	*index = &_filter_index;
}

bool FilterIterator::ApplyCondition()
{
	////TRACE (TRACE_SWITCH);
	std::vector<Item> * records;
	uint32_t * index;
	_input->GetRecords(&records, &index);
	return _plan->ApplyPredicate (records->at (*index));
}
