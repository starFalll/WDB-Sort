#pragma once

#include "defs.h"
#include <vector>

typedef uint64_t RowCount;
typedef uint32_t FieldType;

enum ItemField
{
	INCL = 1,
	MEM,
	MGMT,
	MAX_ITEM,
};

static const std::vector<ItemField> ItemFields = {INCL, MEM, MGMT};

// Define class for data records
struct Item
{
	std::vector<FieldType> fields; 
	Item (FieldType incl, FieldType mem, FieldType mgmt);
	Item ();
	virtual ~Item() = default;
};

class Plan
{
	friend class Iterator;
public:
	Plan ();
	virtual ~Plan ();
	virtual class Iterator * init () const = 0;
private:
}; // class Plan

class Iterator
{
public:
	Iterator ();
	virtual ~Iterator ();
	void run ();
	virtual bool next () = 0;
	virtual void GetRecords(std::vector<Item> ** records, uint32_t ** index);
private:
	// ring queue
	std::vector<Item> _records;
	uint32_t _index;
	RowCount _count;
}; // class Iterator