#include "Iterator.h"
#include "File.h"
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
	ElementSize _eSize;
}; // class SortPlan

class SortIterator : public Iterator
{
public:
	SortIterator (SortPlan const * const plan);
	SortIterator (int _eSize, int batch_size, int group_row);

	~SortIterator ();
	bool next () override;
	void GetRecords(std::vector<Item> ** records, uint32_t ** index) override;
	
	template <typename RandomIt, typename Compare>
	void QuickSort (RandomIt start, RandomIt end, Compare comp);

	void MultiwayMerge ();
	void MultiwayMergeFromDisk();
private:
	// loser tree
	LoserTree* _loser_tree;

	uint32_t _sort_index;
	std::vector<Item> _sort_records;
	SortPlan const * const _plan;
	Iterator * const _input;
	RowCount _consumed, _produced;
	ElementSize _eSize;
	// merge cache run
	const uint32_t _cache_run_list_row;
	const uint32_t _cache_run_list_col;
	uint32_t _current_run_index;
	Item*** _cache_run_list;
	// result array
	const Item** _result;

	//merge from disk parameter
	uint32_t _disk_run_list_row;
	uint32_t _disk_run_list_col;
	Item*** _disk_run_list;
	const Item** _disk_result;

	BatchSize _batch_size;
	GroupRow _group_row;
	uint32_t _ssd_group_num;
	uint32_t _hdd_group_num;
	SSDRowCount _ssd_offset;
	HDDRowCount _hdd_offset;
	File* SSD;
	File* HDD;
	File* ResultHDD;
}; // class SortIterator
