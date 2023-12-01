#pragma once

#include "defs.h"
#include "Item.h"

class Plan
{
	friend class Iterator;
public:
	Plan (ElementSize eSize);
	virtual ~Plan ();
	virtual class Iterator * init () const = 0;
	virtual ElementSize GetSize() const;
protected:
	ElementSize _eSize;	
private:
}; // class Plan

class Iterator
{
public:
	Iterator (ElementSize eSize);
	virtual ~Iterator ();
	void run ();
	virtual bool next () = 0;
	virtual void GetRecords(std::vector<Item> ** records, uint32_t ** index);
	virtual ElementSize GetSize() const;

protected:
	ElementSize _eSize;
private:
	// ring queue
	std::vector<Item> _records;
	uint32_t _index;
	RowCount _count;
}; // class Iterator