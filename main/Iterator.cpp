#include "Iterator.h"

Plan::Plan (RowSize row_size)
{
	_row_size = row_size;
} // Plan::Plan

Plan::~Plan ()
{
} // Plan::~Plan

RowSize Plan::GetSize() const
{
	return _row_size;
}

Iterator::Iterator (RowSize row_size) : _row_size(row_size), _count (0) 
{
	// allocate 2MB to _records
	_records.resize (MAX_CPU_CACHE * 2 / sizeof (Item), Item(row_size, '0'));
	_index = 0;

	SSD_INPUT = new File(SSD_PATH_INPUT, MAX_SSD, SSD_BLOCK, SSD, _row_size);
	HDD_INPUT = new File(HDD_PATH_INPUT, __LONG_LONG_MAX__, HDD_BLOCK, HDD, _row_size);
	SSD_TEMP = new File(SSD_PATH_TEMP, MAX_SSD, SSD_BLOCK, SSD, _row_size);
	HDD_TEMP = new File(HDD_PATH_TEMP, __LONG_LONG_MAX__, HDD_BLOCK, HDD, _row_size);
} // Iterator::Iterator

Iterator::~Iterator ()
{
	delete SSD_INPUT;
	delete HDD_INPUT;
	if (SSD_TEMP->getCurByte()!= 0){
		printf(
		"||--------------------[CPU & Memory Status]---------------------||\n"
		"||%-25s|%33lu MB||\n"
		"||%-25s|%33lu MB||\n",
		"CPU Cache used size", (unsigned long)MAX_CPU_CACHE /1024 /1024, "DRAM used size", (unsigned long)MAX_DRAM /1024 /1024);
		printf (
		"||------------------------[Disk Status]-------------------------||\n"
		"||%-25s| %15lu Bytes ≈%9lu MB||\n"
		"||%-25s|                           %6.2f ms||\n"
		"||%-25s|                      %9lu MB/s||\n"
		"||                         |                                    ||\n"
		"||%-25s| %15lu Bytes ≈%9lu MB||\n"
		"||%-25s|                           %6.2f ms||\n"
		"||%-25s|                      %9lu MB/s||\n",
		"Data written into SSD",(unsigned long) (SSD_TEMP->getCurByte()),(unsigned long) (SSD_TEMP->getCurByte()/(double)1000/1000),
		"SSD write latency",SSD_LATENCY,
		"SSD write bandwidth",SSD_BANDWIDTH/1024/1024,
		"Data written into HDD",(unsigned long) (HDD_TEMP->getCurByte()),(unsigned long) (HDD_TEMP->getCurByte()/(double)1000/1000),
		"HDD write latency",HDD_LATENCY,
		"HDD write bandwidth",HDD_BANDWIDTH/1024/1024);
	}
	delete SSD_TEMP;
	delete HDD_TEMP;
	////TRACE (TRACE_SWITCH);
} // Iterator::~Iterator

void Iterator::run ()
{
	////TRACE (TRACE_SWITCH);

	while (next ())  ++ _count;

	// printf ("\n|%-35s|%15lu lines|\n"
	// "Entire plan produced",(unsigned long) _count);
} // Iterator::run

void Iterator::GetRecords(std::vector<Item> ** records, uint32_t ** index)
{
	*records = &_records;
	*index = &_index;
}

RowSize Iterator::GetSize() const
{
	return _row_size;
}
