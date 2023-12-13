#include <stdlib.h>
#include <stdio.h>
#include "defs.h"

// -----------------------------------------------------------------

Trace::Trace (bool const trace, char const * const function,
		char const * const file, int const line)
	: _output (trace), _function (function), _file (file), _line (line)
{
	_trace (">>>>>");
} // Trace::Trace

Trace::Trace (bool const trace, char const * const function,
			char const * const file, int const line, 
			int const first, int const second, int const third)
	: _output (trace), _function (function), _file (file), _line (line)
{
	_trace (">>>>> first:" + std::to_string(first) + " second:" + std::to_string(second) + " third:" + std::to_string(third));
}

Trace::Trace (bool const trace, char const * const function,
			char const * const file, int const line, 
			std::string const first, std::string const second, std::string const third)
	: _output (trace), _function (function), _file (file), _line (line)
{
	_trace (">>>>> first:" + first + " second:" + second + " third:" + third);
}

Trace::Trace (bool const trace, char const * const function,
			char const * const file, int const line, std::string const msg)
	: _output (trace), _function (function), _file (file), _line (line)
{
	_trace (">>>>> MSG:" + msg);
}

Trace::~Trace ()
{
	_trace ("<<<<<");
} // Trace::~Trace

void Trace::_trace (std::string lead)
{
	if (_output)
		printf ("%s %s (%s:%d)\n", lead.c_str(), _function, _file, _line);
} // Trace::_trace

// -----------------------------------------------------------------

size_t Random (size_t const range)
{
	return (size_t) rand () % range;
} // Random

size_t Random (size_t const low_incl, size_t const high_incl)
{
	return low_incl + (size_t) rand () % (high_incl - low_incl + 1);
} // Random

size_t RoundDown (size_t const x, size_t const y)
{
	return x - (x % y);
} // RoundDown

size_t RoundUp (size_t const x, size_t const y)
{
	size_t const z = x % y;
	return (z == 0 ? x : x + y - z);
} // RoundUp

bool IsPowerOf2 (size_t const x)
{
	return x > 0 && (x & (x - 1)) == 0;
} // IsPowerOf2

size_t lsb (size_t const x)
{
	size_t const y = x & (x - 1);
	return x ^ y;
} // lsb

size_t msb (size_t const x)
{
	size_t y = x;
	for (size_t z;  z = y & (y - 1), z != 0;  y = z)
		; // nothing
	return y;
} // msb

int msbi (size_t const x)
{
	int i = 0;
	for (size_t z = 2;  z <= x;  ++ i, z <<= 1)
		; // nothing
	return i;
} // msbi

char const * YesNo (bool const b)
{
	return b ? "Yes" : "No";
} // YesNo

char const * OkBad (bool const b)
{
	return b ? "Ok" : "Bad";
} // OkBad
