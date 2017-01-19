#pragma once
// my_memory.h
#include "my_memory_helpers.h"
#include <cstring>

namespace my
{
	template <typename T>
	static inline void memcpy(span<T>& dest, const span<T>& src) {

		::memcpy(dest.begin(), src.begin(), dest.size_in_bytes());
	}
}