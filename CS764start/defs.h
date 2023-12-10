#pragma once

#include <stddef.h>
#include <stdint.h>
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <pthread.h>

typedef uint8_t byte;
typedef char*  FieldType;
typedef uint32_t GroupCount;
typedef uint32_t BatchSize;
typedef uint64_t RowCount;
typedef uint32_t RowSize;

//#define TRACE_SWITCH false
#define TRACE_SWITCH true

#define slotsof(a)	(sizeof (a) / sizeof (a[0]))

//#define nullptr	((void *) NULL)

#define yesno(b)	((b) ? "yes" : "no")

// SSD Path
#define SSD_PATH_INPUT "./input/SSD.csv"

#define SSD_PATH_TEMP "./temp/SSD.csv"

// HDD Path
#define HDD_PATH_INPUT "./input/HDD.csv"

#define HDD_PATH_TEMP "./temp/HDD.csv"

// Hash Table DIR
#define HASH_TABLE_DIR "./hashtable/"

// RES_HDD Path
#define RES_HDD_PATH "./output/RES_HDD.csv"

// SSD Block per access
#define SSD_BLOCK (int32_t) 10 * 1024

#define SSD_LATENCY (float) 0.1

#define SSD_BANDWIDTH (unsigned long) 100*1024*1024

// HDD Block per access
#define HDD_BLOCK (int32_t) 1 * 1024 * 1024

#define HDD_LATENCY (float) 10.0

#define HDD_BANDWIDTH (unsigned long) 100*1024*1024

// CPU cache 1 MB
#define MAX_CPU_CACHE (uint32_t) 1024 * 1024

// DRAM 100 MB
#define MAX_DRAM (uint32_t) 100 * 1024 * 1024

// SSD 10 GB
#define MAX_SSD (unsigned long long) 10 * 1024 * 1024 * 1024

// Output buffer
#define OUTPUT_BUFFER (int32_t) 10 * 1024 * 1024

// call-through to assert() from <assert.h>
//
void Assert (bool const predicate,
		char const * const file, int const line);
//
#if defined ( _DEBUG )  ||  defined (DEBUG)
#define DebugAssert(b)	Assert ((b), __FILE__, __LINE__)
#else // _DEBUG DEBUG
#define DebugAssert(b)	(void) (0)
#endif // _DEBUG DEBUG
//
#define FinalAssert(b)	Assert ((b), __FILE__, __LINE__)
//	#define FinalAssert(b)	(void) (0)
//
#define ParamAssert(b)	FinalAssert (b)

#define traceprintf printf ("%s:%d:%s ", \
		__FILE__, __LINE__, __FUNCTION__), \
	printf

// -----------------------------------------------------------------

class Trace
{
public :

	Trace (bool const trace, char const * const function,
			char const * const file, int const line);
	Trace (bool const trace, char const * const function,
			char const * const file, int const line, 
			int const first, int const second, int const third);
	Trace (bool const trace, char const * const function,
			char const * const file, int const line, 
			std::string const first, std::string const second, std::string const third);
	Trace (bool const trace, char const * const function,
			char const * const file, int const line, std::string const msg);
	~Trace ();

private :

	void _trace (std::string lead);

	bool const _output;
	char const * const _function;
	char const * const _file;
	int const _line;
}; // class Trace

#define TRACE(trace)	{Trace __trace (trace, __FUNCTION__, __FILE__, __LINE__);}
#define TRACE_ITEM(trace, first, second, third)	{Trace __trace (trace, __FUNCTION__, __FILE__, __LINE__, first, second, third);}
#define TRACE_MSG(trace, msg) {Trace __trace(trace, __FUNCTION__, __FILE__, __LINE__, msg);}

// -----------------------------------------------------------------

template <class Value> inline bool odd (
		Value const value, int bits_from_lsb = 0)
{
	return ((value >> bits_from_lsb) & Value (1)) != 0;
}
//
template <class Value> inline bool even (
		Value const value, int bits_from_lsb = 0)
{
	return ((Value (1) << bits_from_lsb) & value) == 0;
}

// templates for generic min and max operations
//
template <class Val> inline Val min (Val const a, Val const b)
{
	return a < b ? a : b;
}
//
template <class Val> inline Val max (Val const a, Val const b)
{
	return a > b ? a : b;
}
//
template <class Val> inline Val between (Val const value, Val const low, Val const high)
	// 'value' but no less than 'low' and no more than 'high'
{
	return (value < low ? low : value > high ? high : value);
}
//
template <class Val> inline void extremes (Val const value, Val & low, Val & high)
	// retain 'value' in 'low' or 'high' keeping track of extremes
{
	if (value < low) low = value;
	if (value > high) high = value;
}

// templates for generic division
// should work for any integer type
//
template <class Val> inline Val divide (Val const a, Val const b)
{
	return 1 + (a - 1) / b;
} // divide
//
template <class Val> inline Val roundup (Val const a, Val const b)
{
	return b * divide (a, b);
} // roundup

template <class Val> inline void exchange (Val & a, Val & b)
{
	Val const c = a;
	a = b;
	b = c;
} // exchange

template <class Iter> inline Iter getmid (Iter a, Iter b, Iter c)
{
	if ( * b < * a) {
		if ( * c < * b) return b;
		return * a < * c ? a : c;
	}
	else {
		if ( * c < * a) return a;
		return * b < * c ? b : c;
	}
} // getmid

template <class Val> inline void swap (Val & a, Val & b) {
	Val tmp = a;
	a = b;
	b = tmp;
}

// -----------------------------------------------------------------

template <class Val> inline Val mask (int const from, int const to)
{
	// from: index of lsb set to 1, range 0..31
	// to: index of msb set to 1, range from..31
	// from==to ==> single bit set
	//
	return (((Val) 2) << to) - (((Val) 1) << from);
} // Value mask (int from, int to)

size_t Random (size_t const range);
size_t Random (size_t const low_incl, size_t const high_incl);
size_t RoundDown (size_t const x, size_t const y);
size_t RoundUp (size_t const x, size_t const y);
bool IsPowerOf2 (size_t const x);
size_t lsb (size_t const x);
size_t msb (size_t const x);
int msbi (size_t const x);
char const * YesNo (bool const b);
char const * OkBad (bool const b);
