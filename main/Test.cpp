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
	//redirect stdout to trace_file_name
    FILE* fp = freopen(trace_file_name, "w", stdout); 

	printf(
	"\n|------------------------Input Arguments-------------------------|\n"
	"|%-26s|%29d Records|\n"
	"|%-26s|%31d Bytes|\n"
	"|%-26s|%37s|\n"
	"|----------------------------------------------------------------|\n\n\n",
	"Records need generating", row_count,"Every record's length", row_size,"Trace file name", trace_file_name);

	printf("|----------------------------------------------------------------|\n");
	printf("|Scan & Filter & In-memory sort phase                            |\n");
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
	for (int i = 0; i < hdd_group_lens.size(); i++) {
		if (hdd_group_lens[i] > 0) {
			real_hdd_group_lens.push_back(hdd_group_lens[i]);
		}
	}
	delete it;
	delete plan;
	auto later = std::chrono::high_resolution_clock::now();
    auto timestamp_later = std::chrono::time_point_cast<std::chrono::milliseconds>(later);
    long long milliseconds_later = timestamp_later.time_since_epoch().count();
	printf(
	"||--------------------------------------------------------------||\n"
	"|Scan & Filter & In-memory sort phase end                        |\n"
	"|%-19s%10lld ms                                |\n"
	"|----------------------------------------------------------------|\n\n\n",
	"Total time cost:",milliseconds_later - milliseconds_now);
	
	printf("|----------------------------------------------------------------|\n");
	printf("|External merge sort phase                                       |\n");
	//need ssd_group_count 、hdd_group_count、row_size、each_group_row_count、batch_size
	DiskScan * d_scan = new DiskScan(ssd_group_lens, real_hdd_group_lens, row_size, 300);
	d_scan->ReadFromDisk();
	d_scan->MultiwayMerge();
	delete d_scan;

	auto later1 = std::chrono::high_resolution_clock::now();
    auto timestamp_later1 = std::chrono::time_point_cast<std::chrono::milliseconds>(later1);
    long long milliseconds_later1 = timestamp_later1.time_since_epoch().count();
	printf(
	"||--------------------------------------------------------------||\n"
	"|External merge sort phase end                                   |\n"
	"|%-19s%10lld ms                                |\n"
	"|----------------------------------------------------------------|\n\n\n",
	"Total time cost:",milliseconds_later1 - milliseconds_later);
	// traceprintf("---------------External merge sort phase cost time: %lld ms------------------\n\n", milliseconds_later1 - milliseconds_later);

	printf("|----------------------------------------------------------------|\n");
	printf("|Result Verify phase                                             |\n");
	Verify* v = new Verify(row_size, (unsigned long long)row_count*(unsigned long long)row_size, MAX_DRAM);
	v->verify();
	delete v;

	auto later2 = std::chrono::high_resolution_clock::now();
    auto timestamp_later2 = std::chrono::time_point_cast<std::chrono::milliseconds>(later2);
    long long milliseconds_later2 = timestamp_later2.time_since_epoch().count();
	printf(
	"||--------------------------------------------------------------||\n"
	"|Result Verify phase end                                         |\n"
	"|%-19s%10lld ms                                |\n"
	"|----------------------------------------------------------------|\n\n\n",
	"Total time cost:",milliseconds_later2 - milliseconds_later1);
	
	// traceprintf("---------------Verify phase cost time: %lld ms------------------\n", milliseconds_later2 - milliseconds_later1);
	fclose(fp);
	return 0;
} // main
