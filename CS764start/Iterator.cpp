#include "Iterator.h"

Plan::Plan (RowSize row_size)
{
	_row_size = row_size;
	TRACE (TRACE_SWITCH);
} // Plan::Plan

Plan::~Plan ()
{
	TRACE (TRACE_SWITCH);
} // Plan::~Plan

RowSize Plan::GetSize() const
{
	return _row_size;
}

Iterator::Iterator (RowSize row_size) : _row_size(row_size), _count (0) 
{
	TRACE (TRACE_SWITCH);
	// allocate 2MB to _records
	_records.resize (MAX_CPU_CACHE * 2 / sizeof (Item), Item(row_size, '0'));
	_index = 0;

	SSD_INPUT = new File(SSD_PATH_INPUT, MAX_SSD, SSD_BLOCK);
	HDD_INPUT = new File(HDD_PATH_INPUT, __LONG_LONG_MAX__, HDD_BLOCK);
	SSD_TEMP = new File(SSD_PATH_TEMP, MAX_SSD, SSD_BLOCK);
	HDD_TEMP = new File(HDD_PATH_TEMP, __LONG_LONG_MAX__, HDD_BLOCK);
} // Iterator::Iterator

Iterator::~Iterator ()
{
	delete SSD_INPUT;
	delete HDD_INPUT;
	delete SSD_TEMP;
	delete HDD_TEMP;
	TRACE (TRACE_SWITCH);
} // Iterator::~Iterator

void Iterator::run ()
{
	TRACE (TRACE_SWITCH);

	while (next ())  ++ _count;

	traceprintf ("entire plan produced %lu rows\n",
			(unsigned long) _count);
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
