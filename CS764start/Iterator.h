#pragma once

#include "defs.h"
#include <vector>
#include <string>
#include <string.h>

typedef uint64_t RowCount;
typedef uint32_t FieldType;

typedef uint32_t ElementSize;
typedef uint64_t SSDRowCount;
typedef uint64_t HDDRowCount;

enum ItemField
{
	INCL = 0,
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
	//FieldType fields[3]; 
	//Item (FieldType incl, FieldType mem, FieldType mgmt);
	Item (StringFieldType incl, StringFieldType mem, StringFieldType mgmt);
	StringFieldType fields[3]; 
	Item (const Item& other);
	Item ();
	Item (ElementSize eSize);
	bool operator < (const Item & other) const;
	const StringFieldType* GetItemString() const;
	~Item() = default;
};

// the minimal value of object Item
//static const Item ITEM_MIN = Item(0, 0, 0);
static const Item ITEM_MIN = Item("0", "0", "0");
// the maximal value of object Item
//static const Item ITEM_MAX = Item(UINT32_MAX, UINT32_MAX, UINT32_MAX);
static const Item ITEM_MAX = Item(std::to_string(UINT32_MAX), std::to_string(UINT32_MAX), std::to_string(UINT32_MAX));

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
	ElementSize _eSize;
}; // class Iterator