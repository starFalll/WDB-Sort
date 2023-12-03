#include "Item.h"

Item::Item (const FieldType incl, const FieldType mem, const FieldType mgmt)
{
	fields[INCL] = incl;
	fields[MEM] = mem;
	fields[MGMT] = mgmt;    
}

Item::Item (const Item& other){
	// copy constructor
	for (int32_t i = 0; i < sizeof(other.fields) / sizeof(other.fields[0]); i++){
		fields[i] = other.fields[i];
	}
}

Item::Item (){}

// set eSize length  value to 99999...
Item::Item (RowSize row_size, char init_char){
	int element_size = int(row_size) / 3;
	fields[INCL] = new char[element_size];
	fields[MEM] = new char[element_size];
	fields[MGMT] = new char[row_size - element_size*2];
	memset(fields[INCL], init_char, element_size);
	memset(fields[MEM], init_char, element_size);
	memset(fields[MGMT], init_char, row_size - element_size*2);
}

bool Item::operator < (const Item & other) const
{
	return strcmp(fields[COMPARE_FIELD], other.fields[COMPARE_FIELD]) < 0;
}

// using first value to compare
char* Item::GetItemString()
{
	return fields[0];
}