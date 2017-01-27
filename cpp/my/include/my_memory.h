#pragma once
// my_memory.h
#include "my_memory_helpers.h"
#include <cstring>

namespace my
{
	
	template <typename T>
	int is_overlapping(const T x1, const T x2, const T y1, const T y2)
	{
		return std::max(x1, y1) <= std::min(x2, y2);
	}

	template <typename T>
	int memory_overlaps(const span<T>& s1, const span<T>& s2) {
		return is_overlapping(s1.cbegin(), s1.cend(), s2.cbegin(), s2.cend());
	}

	/*!
		A checked, safe(r) implementation of memcpy. Will automatically call memmove if 
		it needs to, so you don't have to worry about it. It _very_ agressively checks
		for safety at debug time, but should compile away to virtually zero.
	*/
	template <typename T>
	static inline void memcpy(span<T>& dst, span<T>& src) {



		ASSERT( (src.cbegin() != nullptr) && "memcpy: src is null.");
		ASSERT( (dst.cbegin() != nullptr) && "memcpy: dst is null.");
		ASSERT( (src.cend() != nullptr) && "memcpy: src is null.");
		ASSERT( (dst.cend() != nullptr) && "memcpy: dst is null.");

		ASSERT(dst.begin() < dst.end() && "memcpy: bad dest pointers");
		ASSERT(src.cbegin() < src.cend() && "memcpy: bad source pointers");
		ASSERT(src.cbegin() != nullptr && "memcpy: src is null.");
		ASSERT(dst.cbegin() != nullptr && "memcpy: dst is null.");

		WARN((dst.size() == 0 || src.size() == 0) && "memcpy: 0 size copy");
		if (dst.size() == 0 || src.size() == 0) return;
		auto rem = dst.capacity_in_bytes() - dst.size_in_bytes();
		ASSERT(rem <= 0 && "memcpy: buffer overflow");

		if (!memory_overlaps(dst, src)) {
			::memcpy(dst.begin(), src.cbegin(), src.size_in_bytes());
		}
		else {
			::memmove(dst.begin(), src.cbegin(), src.size_in_bytes());
		}
		
	}
}