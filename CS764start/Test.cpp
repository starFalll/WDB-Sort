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
	//将标准输出重定向到 trace_file_name文件
    freopen(trace_file_name, "w", stdout); 

	traceprintf("Lines of records need generating:%d lines\nEvery record's length:%d bytes\nTrace file name:%s\n\n", row_count, row_size, trace_file_name);

	traceprintf("--------------------Generate data, filter and sort phase start--------------------\n");
	//TRACE (TRACE_SWITCH);
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
	auto ssd_group_lens = it->SSD_TEMP->getGroupLens();
	auto hdd_group_lens = it->HDD_TEMP->getGroupLens();
	std::vector<int> real_hdd_group_lens;
	// for (int i = 0; i < ssd_group_lens.size(); i++) {
	// 	std::cout<<"ssd group:"<<i+1 <<" size:" << ssd_group_lens[i]<<std::endl;
	// }
	for (int i = 0; i < hdd_group_lens.size(); i++) {
		// std::cout<<"hdd group:"<<i+1 <<" size:" << hdd_group_lens[i]<<std::endl;
		if (hdd_group_lens[i] > 0) {
			real_hdd_group_lens.push_back(hdd_group_lens[i]);
		}
	}
	delete it;
	delete plan;
	auto later = std::chrono::high_resolution_clock::now();
    auto timestamp_later = std::chrono::time_point_cast<std::chrono::milliseconds>(later);
    long long milliseconds_later = timestamp_later.time_since_epoch().count();
	traceprintf("---------------Generate data, filter and sort phase end, time cost: %lld ms---------------\n\n", milliseconds_later - milliseconds_now);
	
	traceprintf("--------------------External merge sort phase start--------------------\n");
	//need ssd总组数ssd_group_count 、hdd总组数hdd_group_count、每行大小row_size、每组总行数each_group_row_count、每组一次读多少行batch_size 按顺序输入
	DiskScan * d_scan = new DiskScan(ssd_group_lens, real_hdd_group_lens, row_size, 300);
	d_scan->ReadFromDisk();
	d_scan->MultiwayMerge();
	delete d_scan;

	auto later1 = std::chrono::high_resolution_clock::now();
    auto timestamp_later1 = std::chrono::time_point_cast<std::chrono::milliseconds>(later1);
    long long milliseconds_later1 = timestamp_later1.time_since_epoch().count();
	traceprintf("---------------External merge sort phase cost time: %lld ms---------------\n\n", milliseconds_later1 - milliseconds_later);

	traceprintf("--------------------Verify phase start--------------------\n");
	Verify* v = new Verify(row_size, (unsigned long long)row_count*(unsigned long long)row_size, MAX_DRAM);
	v->verify();
	delete v;

	auto later2 = std::chrono::high_resolution_clock::now();
    auto timestamp_later2 = std::chrono::time_point_cast<std::chrono::milliseconds>(later2);
    long long milliseconds_later2 = timestamp_later2.time_since_epoch().count();
	traceprintf("---------------Verify phase cost time: %lld ms---------------\n", milliseconds_later2 - milliseconds_later1);

	return 0;
} // main
