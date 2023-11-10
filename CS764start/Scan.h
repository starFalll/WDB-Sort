#include "Iterator.h"

class ScanPlan : public Plan
{
	friend class ScanIterator;
public:
	ScanPlan (RowCount const count, ElementSize const eSize);
	~ScanPlan ();
	Iterator * init () const;
private:
	RowCount const _count;
	ElementSize _eSize;
}; // class ScanPlan

class ScanIterator : public Iterator
{
public:
	ScanIterator (ScanPlan const * const plan);
	~ScanIterator ();
	bool next ();
private:
	Item GenerateOneRecord ();

private:
	ScanPlan const * const _plan;
	RowCount _count;
	ElementSize _eSize;
}; // class ScanIterator
