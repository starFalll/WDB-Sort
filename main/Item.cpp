#include "Item.h"

Item::Item (const FieldType incl, const FieldType mem, const FieldType mgmt)
{
	fields[INCL] = incl;
	fields[MEM] = mem;
	fields[MGMT] = mgmt;    
}

Item::Item (const Item& other){
	if (this == &other) return;
	// copy constructor
	for (int32_t i = 0; i < sizeof(other.fields) / sizeof(other.fields[0]); i++){
		char* cur = other.fields[i];
		int size = 0;	
		while (*cur != '\0') {
			size++;
			cur++;
		}
		fields[i] = new char[size+1];
		memset(fields[i], 0, size+1);
		memcpy(fields[i], other.fields[i], size);
	}
}

Item& Item::operator=(const Item& other) {
	if (this == &other) return *this;
	// the copy-and-swap idiom
	Item tmp(other);
	std::swap(tmp.fields[INCL], fields[INCL]);
	std::swap(tmp.fields[MEM], fields[MEM]);
	std::swap(tmp.fields[MGMT], fields[MGMT]);
	return *this;
}

Item::~Item() {
	if (fields[INCL]) {
		delete [] fields[INCL];
		fields[INCL] = nullptr;
	}
	if (fields[MEM]) {
		delete [] fields[MEM];
		fields[INCL] = nullptr;
	}
	if (fields[MGMT]) {
		delete [] fields[MGMT];
		fields[INCL] = nullptr;
	}
}

Item::Item (){
	fields[INCL] = nullptr;
	fields[MEM] = nullptr;
	fields[MGMT] = nullptr;
}

// set eSize length  value to 99999...
Item::Item (RowSize row_size, char init_char){
	int element_size = int(row_size) / 3;
	fields[INCL] = new char[element_size];
	fields[MEM] = new char[element_size];
	fields[MGMT] = new char[row_size - element_size*2];
	memset(fields[INCL], init_char, element_size);
	fields[INCL][element_size-1] = '\0';	
	memset(fields[MEM], init_char, element_size);
	fields[MEM][element_size-1] = '\0';	
	memset(fields[MGMT], init_char, row_size - element_size*2);
	fields[MGMT][row_size - element_size*2-1] = '\0';	
}

bool Item::operator < (const Item & other) const
{
	return strcmp(fields[COMPARE_FIELD], other.fields[COMPARE_FIELD]) < 0;
}

bool Item::operator > (const Item & other) const
{
	return strcmp(fields[COMPARE_FIELD], other.fields[COMPARE_FIELD]) > 0;
}

// using first value to compare
char* Item::GetItemString()
{
	return fields[0];
}