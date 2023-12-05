#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "DiskScan.h"
#include "Verify.h"
#include <chrono>
#include <unistd.h>

int main (int argc, char * argv [])
{
	int opt;
	int row_count = 0, row_size = 0;
	char* trace_file_name = nullptr;
	while((opt = getopt(argc, argv, "c:s:o:")) != -1){
		switch(opt){
			case 'c':
				row_count = std::stoi(optarg);
				break;
			case 's':
				row_size = std::stoi(optarg);
				break;
			case 'o':
				trace_file_name = optarg;
				break;
			default:
				printf("Wrong Parameter\n");
				return 1;
		}
	}
	printf("%d, %d, %s\n", row_count, row_size, trace_file_name);

	row_count = 2621440;
	row_size = 50;
	TRACE (TRACE_SWITCH);
	Plan * const scan_plan = new ScanPlan (row_count,row_size);
	//Plan * scan_plan = new ScanPlan(10000000);
	FilterPlan * filter_plan = new FilterPlan ( scan_plan );
	Plan * plan = new SortPlan ( filter_plan );

	Iterator * it = plan->init ();
	// filter_plan->SetPredicate(INCL, GT, 1000000);
	auto now = std::chrono::high_resolution_clock::now();
    auto timestamp_now = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    long long milliseconds_now = timestamp_now.time_since_epoch().count();

	it->run ();
	auto later = std::chrono::high_resolution_clock::now();
    auto timestamp_later = std::chrono::time_point_cast<std::chrono::milliseconds>(later);
    long long milliseconds_later = timestamp_later.time_since_epoch().count();
	printf("cost time: %lld ms\n", milliseconds_later - milliseconds_now);
	auto ssd_group_lens = it->SSD_TEMP->getGroupLens();
	auto hdd_group_lens = it->HDD_TEMP->getGroupLens();
	for (int i = 0; i < ssd_group_lens.size(); i++) {
		std::cout<<"ssd group:"<<i+1 <<" size:" << ssd_group_lens[i]<<std::endl;
	}
	for (int i = 0; i < hdd_group_lens.size(); i++) {
		std::cout<<"hdd group:"<<i+1 <<" size:" << hdd_group_lens[i]<<std::endl;
	}

	delete it;
	delete plan;
	//need ssd总组数ssd_group_count 、hdd总组数hdd_group_count、每行大小row_size、每组总行数each_group_row_count、每组一次读多少行batch_size 按顺序输入
	DiskScan * d_scan = new DiskScan(ssd_group_lens, hdd_group_lens, row_size, 300);
	d_scan->ReadFromDisk();
	printf("end read, begin final merge...\n");
	d_scan->MultiwayMerge();
	delete d_scan;

	Verify* v = new Verify(row_size, row_count*row_size, MAX_DRAM);
	v->verify();
	delete v;

	return 0;
} // main
