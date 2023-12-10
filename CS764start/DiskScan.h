#include "LoserTree.h"
#include "SharedBuffer.h"
#include <vector>

class DiskScan
{
public:
	DiskScan(std::vector<int>& ssd_each_group_row, std::vector<int>& hdd_each_group_row, RowSize const row_size, BatchSize const batch_size);
	~DiskScan();
    void ReadFromDisk();
    void MultiwayMerge();
    void RefillRow(uint32_t group_num);
    void Bytes2DiskRecord(char* buffer, uint32_t group_num);
private:
    RowSize const _row_size;
    BatchSize const _batch_size;

    File* SSD;
	File* HDD;
    File* RES_HDD;

    //2d array row&col
    const uint32_t _disk_run_list_row;
	const uint32_t _disk_run_list_col;
    uint32_t _current_run_index;
    Item*** _disk_run_list;

    //each group's rows
    std::vector<int>& _ssd_each_group_row;
    std::vector<int>& _hdd_each_group_row;

    uint32_t* _each_group_col;

    //group offset
    uint32_t* _group_offset;

    // loser tree
	LoserTree* _loser_tree;

    SharedBuffer* _shared_buffer;

}; // class ScanPlan