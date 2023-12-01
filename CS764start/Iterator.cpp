#include "Iterator.h"

Plan::Plan (ElementSize eSize)
{
	_eSize = eSize;
	TRACE (TRACE_SWITCH);
} // Plan::Plan

Plan::~Plan ()
{
	TRACE (TRACE_SWITCH);
} // Plan::~Plan

ElementSize Plan::GetSize() const
{
	return _eSize;
}

Iterator::Iterator (ElementSize eSize) : _eSize(eSize), _count (0) 
{
	TRACE (TRACE_SWITCH);
	// allocate 2MB to _records
	_records.resize (MAX_CPU_CACHE * 2 / sizeof (Item));
	_index = 0;
} // Iterator::Iterator

Iterator::~Iterator ()
{
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

ElementSize Iterator::GetSize() const
{
	return _eSize;
}
