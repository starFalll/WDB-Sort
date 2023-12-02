#include "Iterator.h"
#include "File.h"

class ScanPlan : public Plan
{
	friend class ScanIterator;
public:
	ScanPlan (RowCount const count, RowSize const row_size);
	~ScanPlan ();
	Iterator * init () const;
private:
	RowCount const _count;
}; // class ScanPlan

class ScanIterator : public Iterator
{
public:
	ScanIterator (ScanPlan const * const plan);
	~ScanIterator ();
	bool next ();
private:
	Item GenerateOneRecord ();
	char* GeneratRandomStr(int count);

	ScanPlan const * const _plan;
	RowCount _count;
	File* HDD;
}; // class ScanIterator
