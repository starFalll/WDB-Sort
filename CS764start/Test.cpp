#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include <chrono>

int main (int argc, char * argv [])
{
	TRACE (TRACE_SWITCH);
	Plan * const scan_plan = new ScanPlan (7,30);//scan的private成员参数_input初始化为7，input就是RowCount
	//Plan * scan_plan = new ScanPlan(10000000);
	FilterPlan * filter_plan = new FilterPlan ( scan_plan );//上面构造的scan_plan作为filter_plan的private参数_input，类型为plan*
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
