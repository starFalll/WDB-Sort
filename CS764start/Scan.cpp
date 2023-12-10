#include "Scan.h"
#include <cstdlib>
#include <ctime>

ScanPlan::ScanPlan (RowCount const count, RowSize const row_size) : Plan(row_size), _count (count)
{
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan ()
{
} // ScanPlan::~ScanPlan

Iterator * ScanPlan::init () const
{
	////TRACE (TRACE_SWITCH);
	return new ScanIterator (this);
} // ScanPlan::init

ScanIterator::ScanIterator (ScanPlan const * const plan) : Iterator(plan->GetSize()), 
	_plan (plan), _count (0)
{
	////TRACE (TRACE_SWITCH);
	std::srand (static_cast <unsigned int> (std::time (NULL)));

} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator ()
{
	//TRACE (TRACE_SWITCH);
	printf ("||---------------------Generate & Scan Data---------------------||\n"
		"||%-25s| %12lu of%12lu Records||\n",
		"Records scanned",(unsigned long) (_count),(unsigned long) (_plan->_count));
} // ScanIterator::~ScanIterator

bool ScanIterator::next ()
{
	if (_count >= _plan->_count)
		return false;
	Item item = GenerateOneRecord();
	// save generated record in input hdd
	HDD_INPUT->write(item.fields[0], _row_size / 3);
	HDD_INPUT->write(item.fields[1], _row_size / 3);
	HDD_INPUT->write(item.fields[2], _row_size - 2 * (_row_size / 3));
	std::vector<Item> * records;
	uint32_t * index;
	GetRecords(&records, &index);
	*index = (++ (*index)) % records->size ();
	records->at (*index) = item;
	//TRACE_ITEM (TRACE_SWITCH, item.fields[INCL], item.fields[MEM], item.fields[MGMT]);
	++ _count;
	return true;
} // ScanIterator::next


Item ScanIterator::GenerateOneRecord ()
{
	int element_size = _row_size / 3;
	FieldType incl = GeneratRandomStr(element_size);
	FieldType mem = GeneratRandomStr(element_size);
	FieldType mgmt = GeneratRandomStr(_row_size - element_size*2);

	return Item (incl, mem, mgmt);
}

//generate random string, whose lenth is equal to the column's size
char* ScanIterator::GeneratRandomStr(int count){
	char* str = new char[count];
	char c;
	for(int idx = 0; idx < count-1 ; idx++){
		c = '0' + std::rand()%10;
		str[idx] = c;
	}
	str[count-1] = '\0';
	return str;
}