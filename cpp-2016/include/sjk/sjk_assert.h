#pragma once
#ifndef SJK_ASSERT_H
#define SJK_ASSERT_H

#ifdef _MSC_VER
#	pragma warning (disable : 26461)
#endif

#include <cstdint>
#include <cassert>

namespace assert_ctr
{
	extern uint64_t ctr;
	// return the number of assertions we 'passed'.
	static inline uint64_t total(){ return ctr; }
} // namespace assert_ctr

#ifndef ASSERT
#	define ASSERT(x) assert_ctr::ctr++; assert(x);
#endif

#endif // SJK_ASSERT_H
