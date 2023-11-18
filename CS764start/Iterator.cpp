#include "Iterator.h"

/*
Item::Item (FieldType incl, FieldType mem, FieldType mgmt)
{
	// fields.resize(MAX_ITEM);
	fields [INCL] = incl;
	fields [MEM] = mem;
	fields [MGMT] = mgmt;
}
*/

Item::Item (StringFieldType incl, StringFieldType mem, StringFieldType mgmt)
{
	// fields.resize(MAX_ITEM);
	fields [INCL] = incl;
	fields [MEM] = mem;
	fields [MGMT] = mgmt;
}

Item::Item (const Item& other){
	// copy constructor
	for (int32_t i = 0; i < sizeof(other.fields) / sizeof(other.fields[0]); i++){
		fields[i] = other.fields[i];
	}
}

/*
Item::Item ()
{
	// fields.resize(MAX_ITEM);
	fields [INCL] = INT_MAX;
	fields [MEM] = INT_MAX;
	fields [MGMT] = INT_MAX;
}
*/
Item::Item ()
{
	// fields.resize(MAX_ITEM);
	fields [INCL] = std::to_string(INT_MAX);
	fields [MEM] = std::to_string(INT_MAX);
	fields [MGMT] = std::to_string(INT_MAX);
}

Item::Item (ElementSize eSize){
	char inclTemp[eSize];
	memset(inclTemp , '0' , sizeof(inclTemp));
	char memTemp[eSize];
	memset(memTemp , '0' , sizeof(memTemp));
	char mgmtTemp[eSize];
	memset(mgmtTemp , '0' , sizeof(mgmtTemp));
	StringFieldType incl = std::to_string(INT_MAX);
	StringFieldType mem = std::to_string(INT_MAX);
	StringFieldType mgmt = std::to_string(INT_MAX);
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
	fields [INCL] = (std::string)inclTemp;
	fields [MEM] = (std::string)memTemp;
	fields [MGMT] = (std::string)mgmtTemp;
}

bool Item::operator < (const Item & other) const
{
	return fields[COMPARE_FIELD] < other.fields[COMPARE_FIELD];
}

// using first value to compare
const StringFieldType* Item::GetItemString() const
{
	return &fields[0];
}

Plan::Plan ()
{
	TRACE (TRACE_SWITCH);
} // Plan::Plan

Plan::~Plan ()
{
	TRACE (TRACE_SWITCH);
} // Plan::~Plan

Iterator::Iterator () : _count (0)
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
