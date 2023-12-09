#include "Iterator.h"
#include "LoserTree.h"
#include "SharedBuffer.h"
#include <queue>
// #include <thread>

class SortPlan : public Plan
{
	friend class SortIterator;
public:
	SortPlan (Plan * const input);
	virtual ~SortPlan ();
	Iterator * init () const;
	
private:
	Plan * const _input;
}; // class SortPlan

class SortIterator : public Iterator
{
public:
	SortIterator (SortPlan const * const plan);
	virtual ~SortIterator ();
	bool next () override;
	
	template <typename RandomIt, typename Compare>
	void QuickSort (RandomIt start, RandomIt end, Compare comp);

	void MultiwayMerge ();
private:
	// loser tree
	LoserTree* _loser_tree;

	uint32_t _sort_index;
	std::vector<Item> _sort_records;
	SortPlan const * const _plan;
	Iterator * const _input;
	RowCount _consumed, _produced, _last_consumed;
	// merge cache run
	const uint32_t _cache_run_list_row;
	const uint32_t _cache_run_list_col;
	uint32_t _current_run_index;
	Item*** _cache_run_list;
	// result buffer
	SharedBuffer* _shared_buffer;

}; // class SortIterator
