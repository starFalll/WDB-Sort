#include "Scan.h"
#include <cstdlib>
#include <ctime>

ScanPlan::ScanPlan (RowCount const count) : _count (count)
{
	TRACE (TRACE_SWITCH);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan ()
{
	TRACE (TRACE_SWITCH);
} // ScanPlan::~ScanPlan

Iterator * ScanPlan::init () const
{
	TRACE (TRACE_SWITCH);
	return new ScanIterator (this);
} // ScanPlan::init

ScanIterator::ScanIterator (ScanPlan const * const plan) :
	_plan (plan), _count (0)
{
	TRACE (TRACE_SWITCH);
	std::srand (static_cast <unsigned int> (std::time (NULL)));
} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator ()
{
	TRACE (TRACE_SWITCH);
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_count),
			(unsigned long) (_plan->_count));
} // ScanIterator::~ScanIterator

bool ScanIterator::next ()
{
	if (_count >= _plan->_count)
		return false;
	Item item = GenerateOneRecord();

	// // scan too quickly, wait writing
	// if (_index < _records.size() && !_records[_index].write) {

	// }
	std::vector<Item> * records;
	uint32_t * index;
	GetRecords(&records, &index);
	*index = (++ (*index)) % records->size ();
	records->at (*index) = item;
	TRACE_ITEM (TRACE_SWITCH, item.fields[INCL], item.fields[MEM], item.fields[MGMT]);
	++ _count;
	return true;
} // ScanIterator::next


Item ScanIterator::GenerateOneRecord ()
{
	//FieldType incl = std::rand();
	//FieldType mem = std::rand();
	//FieldType mgmt = std::rand();
	StringFieldType incl = std::to_string(std::rand());
	StringFieldType mem = std::to_string(std::rand());
	StringFieldType mgmt = std::to_string(std::rand());
	return Item (incl, mem, mgmt);
}
