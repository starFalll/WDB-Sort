#include "Scan.h"
#include <cstdlib>
#include <ctime>

ScanPlan::ScanPlan (RowCount const count, RowSize const row_size) : Plan(row_size), _count (count)
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

ScanIterator::ScanIterator (ScanPlan const * const plan) : Iterator(plan->GetSize()), 
	_plan (plan), _count (0)
{
	TRACE (TRACE_SWITCH);
	std::srand (static_cast <unsigned int> (std::time (NULL)));

	HDD = new File(HDD_PATH_INPUT, __LONG_LONG_MAX__, HDD_BLOCK);
} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator ()
{
	delete HDD;
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
	// save generated record in input hdd
	// std::string str1 = item.fields[0];
	// std::string str2 = item.fields[1];
	// std::string str3 = item.fields[2];
	// const char tmp[3][16] = {*str1.c_str(), *str2.c_str(), *str3.c_str()};
	// char tmp[3][5] = {"1234", "1234", "1234"};
	
	// char tmp[3][16];
	// for(int i=0;i<3;i++){
	// 	for(int j=0;j<item.fields[i].size();j++){
	// 		tmp[i][j] = item.fields[i][j];
	// 	}
	// }
	// HDD->write(tmp[0], 3*_eSize);
	HDD->write((char*)&item, sizeof(item));

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
	int element_size = _row_size / 3;
	FieldType incl = GeneratRandomStr(element_size);
	FieldType mem = GeneratRandomStr(element_size);
	FieldType mgmt = GeneratRandomStr(_row_size - element_size*2);

	return Item (incl, mem, mgmt);
}

//generate random string, whose lenth is equal to the column's size
std::string ScanIterator::GeneratRandomStr(int count){
	std::string str;
	char c;
	for(int idx = 0; idx < count ; idx++){
		c = '0' + std::rand()%10;
		str.push_back(c);
	}
	return str;
}