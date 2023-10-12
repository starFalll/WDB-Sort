#include "Iterator.h"
#include "LoserTree.h"
#include <queue>

class SortPlan : public Plan
{
	friend class SortIterator;
public:
	SortPlan (Plan * const input);
	~SortPlan ();
	Iterator * init () const;
	
private:
	Plan * const _input;
}; // class SortPlan

class SortIterator : public Iterator
{
public:
	SortIterator (SortPlan const * const plan);
	~SortIterator ();
	bool next () override;
	void GetRecords(std::vector<Item> ** records, uint32_t ** index) override;
	
	template <typename RandomIt, typename Compare>
	void QuickSort (RandomIt start, RandomIt end, Compare comp);

	void MultiwayMerge ();
private:
	uint32_t _sort_index;
	std::vector<Item> _sort_records;
	SortPlan const * const _plan;
	Iterator * const _input;
	RowCount _consumed, _produced;
	// merge cache run
	uint32_t _cache_run_limit;
	std::vector<std::vector<Item>> _cache_run_list;
}; // class SortIterator
