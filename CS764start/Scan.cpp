#include "Scan.h"
#include <cstdlib>
#include <ctime>
#include <fstream>

ScanPlan::ScanPlan (RowCount const count,ElementSize const eSize) : _count (count), _eSize(eSize)
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
	_plan (plan), _count (0), _eSize(plan->_eSize)
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
	char inclTemp[_eSize] = {0};
	memset(inclTemp , '0' , sizeof(inclTemp));
	char memTemp[_eSize] = {0};
	memset(memTemp , '0' , sizeof(memTemp));
	char mgmtTemp[_eSize] = {0};
	memset(mgmtTemp , '0' , sizeof(mgmtTemp));
	StringFieldType incl = GeneratRandomStr();
	StringFieldType mem = GeneratRandomStr();
	StringFieldType mgmt = GeneratRandomStr();
	int index = 0;
	for(auto s : incl){
		inclTemp[index++] = s;
	}
	index = 0;
	for(auto s : mem){
		memTemp[index++] = s;
	}
	index = 0;
	for(auto s : mgmt){
		mgmtTemp[index++] = s;
	}
	incl = (std::string)inclTemp;
	mem  = (std::string)memTemp;
	mgmt = (std::string)mgmtTemp;

	return Item (incl, mem, mgmt);
}

//generate random string, whose lenth is equal to the column's size
std::string ScanIterator::GeneratRandomStr(){
	std::string str;
	char c;
	for(int idx = 0; idx < _eSize ; idx++){
		c = '0' + std::rand()%10;
		str.push_back(c);
	}
	return str;
}

Item ScanIterator::ReadRecordsFromSSD()
{
	std::ifstream File("ssd.txt");
	std::string file_text;
	//分组读取SSD里的内容

	char WordBuffer[_eSize*3];
    //以二进制模式打开 ssd.txt 文件
    std::ifstream inFile("ssd.txt", std::ios::in | std::ios::binary);
	
	if (!inFile) {
		return Item("0","0","0");
	}
	//跳到_dRow*n行，表示从第n个小组读一条
	inFile.seekg(_ssdRow*_eSize*3,std::ios::beg);
	//从 in.txt 文件中读取一行字符串，最多不超过 ElementSize*3 个字节
	inFile.getline(WordBuffer, _eSize*3);
	inFile.close();

	StringFieldType incl;
	StringFieldType mem;
	StringFieldType mgmt;
	for(int tempIndex = 0;tempIndex<_eSize;tempIndex++){
		incl.push_back(WordBuffer[tempIndex]);
	}
	for(int tempIndex = _eSize ;tempIndex<_eSize*2;tempIndex++){
		mem.push_back(WordBuffer[tempIndex]);
	}
	for(int tempIndex = _eSize*2 ;tempIndex<_eSize*3;tempIndex++){
		mgmt.push_back(WordBuffer[tempIndex]);
	}
	//每个小组的行数，此处暂定为170行
	_ssdRow += 170;
	return Item (incl, mem, mgmt);
}

Item ScanIterator::ReadRecordsFromHDD()
{
	std::ifstream File("hdd.txt");
	std::string file_text;
	//分组读取SSD里的内容

	char WordBuffer[_eSize*3];
    //以二进制模式打开 ssd.txt 文件
    std::ifstream inFile("hdd.txt", std::ios::in | std::ios::binary);
	
	if (!inFile) {
		return Item("0","0","0");
	}
	//跳到_dRow*n行，表示从第n个小组读一条
	inFile.seekg(_hddRow *_eSize*3,std::ios::beg);
	//从 in.txt 文件中读取一行字符串，最多不超过 ElementSize*3 个字节
	inFile.getline(WordBuffer, _eSize*3);
	inFile.close();

	StringFieldType incl;
	StringFieldType mem;
	StringFieldType mgmt;
	for(int tempIndex = 0;tempIndex<_eSize;tempIndex++){
		incl.push_back(WordBuffer[tempIndex]);
	}
	for(int tempIndex = _eSize ;tempIndex<_eSize*2;tempIndex++){
		mem.push_back(WordBuffer[tempIndex]);
	}
	for(int tempIndex = _eSize*2 ;tempIndex<_eSize*3;tempIndex++){
		mgmt.push_back(WordBuffer[tempIndex]);
	}
	//每个小组的行数，此处暂定为21000行
	_hddRow += 21000;
	return Item (incl, mem, mgmt);
}