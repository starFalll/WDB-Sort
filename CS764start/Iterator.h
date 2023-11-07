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

static const ItemField COMPARE_FIELD = INCL;

static const std::vector<ItemField> ITEM_FIELDS = {INCL, MEM, MGMT};

// Define class for data records
// 24 bytes
struct Item
{
	FieldType fields[3]; 
	Item (FieldType incl, FieldType mem, FieldType mgmt);
	Item (const Item& other);
	Item ();
	bool operator < (const Item & other) const;
	~Item() = default;
};

// the minimal value of object Item
static const Item ITEM_MIN = Item(0, 0, 0);
// the maximal value of object Item
static const Item ITEM_MAX = Item(UINT32_MAX, UINT32_MAX, UINT32_MAX);

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