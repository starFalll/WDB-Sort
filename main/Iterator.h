#pragma once

#include "defs.h"
#include "Item.h"
#include "File.h"

class Plan
{
	friend class Iterator;
public:
	Plan (RowSize row_size);
	virtual ~Plan ();
	virtual class Iterator * init () const = 0;
	virtual RowSize GetSize() const;
protected:
	RowSize _row_size;	
private:
}; // class Plan

class Iterator
{
public:
	Iterator (RowSize row_size);
	virtual ~Iterator ();
	void run ();
	virtual bool next () = 0;
	virtual void GetRecords(std::vector<Item> ** records, uint32_t ** index);
	virtual RowSize GetSize() const;

	File* SSD_TEMP;
	File* HDD_TEMP;
protected:
	RowSize _row_size;
	File* SSD_INPUT;
	File* HDD_INPUT;
	
private:
	// ring queue
	std::vector<Item> _records;
	uint32_t _index;
	RowCount _count;
}; // class Iterator