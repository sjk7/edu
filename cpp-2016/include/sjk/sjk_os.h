#pragma once

#ifndef SJK_OS_H
#define SJK_OS_H
#include "sjk_macros.h"

namespace sjk
{
	namespace os {
#ifdef _WIN32
		//define something for Windows (32-bit and 64-bit, this part is common)
#define SJK_OS_STRING "Windows32"
#define OS_32_BIT
#ifdef _WIN64
#define OS_64_BIT
#undef CHL_OS_STRING
#define CHL_OS_STRING "Windows64"
		//define something for Windows (64-bit only)
#endif
#elif __APPLE__

#if TARGET_IPHONE_SIMULATOR
		// iOS Simulator
#define CHL_OS_STRING "iOSSim"
#elif TARGET_OS_IPHONE
		// iOS device
#define CHL_OS_STRING "iOS"
#elif TARGET_OS_MAC
		// Other kinds of Mac OS
#define CHL_OS_STRING "MacOS"
#else
#   error "Unknown Apple platform"
#endif
#elif __linux__
		// linux
#ifdef __LP64__
#define CHL_OS_STRING "Linux 64-bit"
#define OS_64_BIT
#else
#define CHL_OS_STRING "Linux (32 bit)"
#define OS_32_BIT
#endif
#elif __unix__ // all unices not caught above
		// Unix
#define CHL_OS_STRING "Unix"
#elif defined(_POSIX_VERSION)
		// POSIX
#define CHL_OS_STRING "Posix"
#else
#   error "Unknown compiler"
#endif

	} // namespace os
} // namespace sjk


#endif // SJK_OS_H
