#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include <chrono>

int main (int argc, char * argv [])
{
	TRACE (TRACE_SWITCH);
	//edit e_size
	int e_size = 16;
	//edit row_count
	int row_count = 100000;
	Plan * const scan_plan = new ScanPlan ( row_count , e_size );
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

	//todo edit batch_size
	int batch_size = 5;
	//todo edit group_row
	int group_row = 100;

	SortIterator * disk_it = new SortIterator(e_size , batch_size, group_row);
	disk_it -> MultiwayMergeFromDisk();
	delete it;

	delete plan;

	return 0;
} // main
