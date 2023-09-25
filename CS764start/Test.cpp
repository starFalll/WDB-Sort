#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"

int main (int argc, char * argv [])
{
	TRACE (TRACE_SWITCH);

	// Plan * const plan = new ScanPlan (7);
	Plan * plan = new SortPlan ( new FilterPlan ( new ScanPlan (7) ) );

	Iterator * it = plan->init ();
	it->run ();
	delete it;

	delete plan;

	return 0;
} // main
