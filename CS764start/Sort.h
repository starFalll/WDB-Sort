#include "Iterator.h"
#include <queue>

struct TreeNode{
	Item* _value;
	size_t _run_index;
	size_t _element_index;
	TreeNode(Item item, uint32_t run_index, uint32_t element_index);
	TreeNode();
	bool operator < (const TreeNode & other) const;
	virtual ~TreeNode() = default;
};

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
