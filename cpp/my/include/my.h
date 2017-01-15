// my.h : include this file FIRST in your projects using this library
#pragma once

#ifdef _MSC_VER
#	ifndef MSVC_WARN_HIGHEST_LEVEL
#		pragma warning(disable : 4710) // function not inlined.
#		pragma warning(disable : 4820) // x bytes of padding after data member in struct, etc.
#		pragma warning(disable : 4514) // unreferenced inline function has been removed.
#		pragma warning(disable : 4464) // include path contains (..). AFAIC, this is a convenient feature, not a bug!
#	endif
#endif

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
#	include "../eventapp/include/conio.h"
#endif

#ifdef WANT_TYPE_TRAITS_
#	include <type_traits>

#endif

namespace my
{
	template<typename T>
	struct endl_s
	{
		static T c_str() { 
			static T rc = "\r\n";
			return rc;
		}
	};
	
	template <typename T>
	struct out
	{
		using W = decltype(stdout);
		out(W w = stdout) : m_w(w) {
#ifndef MY_AVOID_STATIC_INITIALIZER_CHECKS
			m_ctr++;
			assert(m_ctr <= 2); // checks only 1 copy per app, 1 cerr, 1 cout
#endif
		}
		void output(const endl_s <const char* const>& s) { (void)s; fprintf(m_w, "%s", s.c_str()); }
		void output(const int64_t i) { fprintf(m_w, "%" PRId64, i);}
		void output(const uint64_t i) { fprintf(m_w, "%" PRIu64, i); }
		void output(const char* s) { fprintf(m_w, "%s", s);	}
		void output(const double d) { fprintf(m_w, "%f", d);}
		void output(const int32_t i) { fprintf(m_w, "%" PRId32, i); }
		void output(const uint32_t i) { fprintf(m_w, "%" PRIu32, i); }
		
		W file() { return m_w; }
		template <typename T>
		out& operator << (const T& t) {	output(t); return *this;}
	private:
		W m_w;
		static int m_ctr;
		
	};
	template <typename T>
	int out<T>::m_ctr = 0; // ODL not violated for templates

#ifndef DEFINED_MY_OUTS
#	define DEFINED_MY_OUTS
	static my::out<decltype(stdout)> cout(stdout);
	static my::out<decltype(stderr)> cerr(stderr);
	static my::endl_s<const char* const> endl;
#endif
}

#ifndef OUT_DEFINED_MY
#define OUT_DEFINED_MY

#ifndef WARN
#define WARN(what)\
	my::cout << " WARNING: " << (what);
#endif

#ifndef NDEBUG

#define ASSERT(expr) \
    do { \
		if (!(expr)) {\
        my::cerr << "Assertion failed: " << #expr << " , at:\n" << (__FILE__) \
		 << ":" << (__LINE__) << " in function: " << __FUNCTION__; abort();} } while(0)

#else
#	define ASSERT(expr)
#endif

#endif


