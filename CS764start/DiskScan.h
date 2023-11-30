#include "LoserTree.h"
#include "SharedBuffer.h"

class DiskScan
{
public:
	DiskScan(GroupCount const ssd_group_count, GroupCount const hdd_group_count, RowSize const row_size, RowCount const each_group_row_count, BatchSize const batch_size);
	~DiskScan();
    void ReadFromDisk();
    void MultiwayMerge();
    void RefillRow(uint32_t group_num);
    void Bytes2DiskRecord(char* buffer, uint32_t group_num);
private:
	GroupCount const _ssd_group_count;
    GroupCount const _hdd_group_count;
    RowSize const _row_size;
    RowCount const _each_group_row_count;
    BatchSize const _batch_size;

    File* SSD;
	File* HDD;
    File* RES_HDD;

    //2d array row&col
    const uint32_t _disk_run_list_row;
	const uint32_t _disk_run_list_col;
    uint32_t _current_run_index;
    Item*** _disk_run_list;

    // loser tree
	LoserTree* _loser_tree;

    SharedBuffer* _shared_buffer;

}; // class ScanPlan