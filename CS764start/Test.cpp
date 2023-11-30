#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include <chrono>
#include <unistd.h>

int main (int argc, char * argv [])
{
	int opt;
	int row_count = 0, column_size = 0;
	char* trace_file_name = nullptr;
	while((opt = getopt(argc, argv, "c:s:o:")) != -1){
		switch(opt){
			case 'c':
				row_count = std::stoi(optarg);
				break;
			case 's':
				column_size = std::stoi(optarg);
				break;
			case 'o':
				trace_file_name = optarg;
				break;
			default:
				printf("Wrong Parameter\n");
				return 1;
		}
	}
	printf("%d, %d, %s\n", row_count, column_size, trace_file_name);

	TRACE (TRACE_SWITCH);
	Plan * const scan_plan = new ScanPlan (5,16);
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
	delete it;

	delete plan;
	
	return 0;
} // main
