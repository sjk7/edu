#pragma once
// my_memory.h
#include "my_memory_helpers.h"
#include <cstring>
#include <algorithm>

namespace my
{
	
	template <typename X, typename Y = X>
	inline int is_overlapping(const X x1, const X x2, const Y y1, const Y y2)
	{
		ASSERT(&x2 >= &x1 && &y2 >= &y1 && "is_overapping: wrong pointer order");
		(void)y2; (void)x1;
#pragma warning (disable: 26481)
#pragma warning (disable: 6305)
		intptr_t cx1 = reinterpret_cast<intptr_t>(x1);
		intptr_t cx2 = reinterpret_cast<intptr_t>(x2);
		intptr_t cy1 = reinterpret_cast<intptr_t>(y1);
		intptr_t cy2 = reinterpret_cast<intptr_t>(y2);
		intptr_t szX = static_cast<intptr_t>(sizeof(X));
		intptr_t szY = static_cast<intptr_t>(sizeof(X));
		int b = cy1 > cx1 + szX && cy1 < cx2 + szX;
		if (b) return b;
		b = cx1 > cy1 + szY && cx1 < cy2 + szY;
		return b;
#pragma warning (default: 6305)
	}

	template <typename T>
	inline int memory_overlaps(const span<T>& s1, const span<const T>& s2) {
		return is_overlapping(s1.cbegin(), s1.cend(), s2.cbegin(), s2.cend());
	}
	/*!
		A checked, safe(r) implementation of memcpy. Will automatically call memmove if 
		it needs to, so you don't have to worry about it. It _very_ agressively checks
		for safety at debug time, but should compile away to virtually zero.
	*/
#ifndef NDEBUG
	template <typename T, typename U = const T>
	FORCEINLINE void memcpy(span<T>& dst, span<const U>& src) {
		
		
		ASSERT( (src.cbegin() != nullptr) && "memcpy: src is null.");
		ASSERT( (dst.cbegin() != nullptr) && "memcpy: dst is null.");
		ASSERT( (src.cend() != nullptr) && "memcpy: src is null.");
		ASSERT( (dst.cend() != nullptr) && "memcpy: dst is null.");

		ASSERT(dst.begin() < dst.end() && "memcpy: bad dest pointers");
		ASSERT(src.cbegin() < src.cend() && "memcpy: bad source pointers");
		ASSERT(src.cbegin() != nullptr && "memcpy: src is null.");
		ASSERT(dst.cbegin() != nullptr && "memcpy: dst is null.");

		const auto dst_sz = dst.size();
		const auto src_sz = src.size();

		WARN((dst_sz == 0 || src_sz == 0) && "memcpy: 0 size copy");
		if (dst_sz == 0 || src_sz == 0) return;
		const auto rem = dst.capacity_in_bytes() - dst.size_in_bytes(); (void)rem;
		ASSERT(rem <= 0 && "memcpy: buffer overflow");
		
		size_t szcopy = std::min(src.size_in_bytes(), dst.size_in_bytes());
		if (!memory_overlaps(dst, src)) {
			::memcpy(dst.begin(), src.begin(), szcopy);
		}
		else {
			ASSERT("I think you meant to use memmove!!" == 0);
			::memmove(dst.begin(), src.cbegin(), szcopy);
		}
		
	}
#else
	
	// not defined in release mode: use memcpy natively for speed.

#endif
}