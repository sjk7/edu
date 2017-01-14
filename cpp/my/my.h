// my.h : include this file FIRST in your projects using this library
#pragma once


#ifndef MY_HDR_INCLUDED
#	define MY_HDR_INCLUDED_
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef WANT_WINDOWS_H_
#	define NOMINMAX
#	include <Windows.h>
#endif

#include <cstdint>
#include <cstring>		// memmove, memcpy
#include <cassert>
#include <inttypes.h>	// PRId64

#ifdef MY_WANT_CHRONO_
#	include <chrono>
#endif

#ifndef MY_WANT_THREAD_
#	include<thread>
#endif

#ifdef WIN32
#	include <conio.h>
#else
#	include "eventapp/include/conio.h"
#endif

#ifdef WANT_TYPE_TRAITS_
#	include <type_traits>

#endif
