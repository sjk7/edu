//my_storage.h
#pragma once

#include <cstdint>

namespace my
{
	namespace storage
	{
		static const int64_t  BYTE = 1;
		static const int64_t  KILOBYTE = 1024;
		static const int64_t  MEGABYTE = 1024 * 1024;
		static const int64_t  GIGABYTE = 1024 * 1024 * 1024;
		static const int64_t  TERABYTE = 1024LL * 1024 * 1024 * 1024;
	}
}