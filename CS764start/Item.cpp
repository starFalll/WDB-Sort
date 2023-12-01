#include "Item.h"

Item::Item (const FieldType incl, const FieldType mem, const FieldType mgmt)
{
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

Item::Item ()
{
	// fields.resize(MAX_ITEM);
	fields[INCL] = '0';
	fields[MEM] = '0';
	fields[MGMT] = '0';
}

// set eSize length  value to 99999...
Item::Item (ElementSize eSize){

	fields[INCL].assign(eSize, '9');
	fields[MEM].assign(eSize, '9');
	fields[MGMT].assign(eSize, '9');
}

bool Item::operator < (const Item & other) const
{
	return fields[COMPARE_FIELD] < other.fields[COMPARE_FIELD];
}

// using first value to compare
const FieldType* Item::GetItemString() const
{
	return &fields[0];
}