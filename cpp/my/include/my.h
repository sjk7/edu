// my.h : include this file FIRST in your projects using this library
#pragma once

#ifdef _MSC_VER
#	ifndef MSVC_WARN_HIGHEST_LEVEL
#		pragma warning(disable : 4710) // function not inlined.
#		pragma warning(disable : 4820) // x bytes of padding after data member in struct, etc.
#		pragma warning(disable : 4514) // unreferenced inline function has been removed.
#		pragma warning(disable : 4464) // include path contains (..). AFAIC, this is a convenient feature, not a bug!
#		pragma warning(disable : 4711) // function x was selected for automatic inline expansion
#		pragma warning(disable : 26461) // 
#		pragma warning (disable : 4996) // MS: ffs, std::copy is STANDARD
#		define SCL_SECURE_NO_WARNINGS
#	endif
#endif

#ifdef _MSC_VER
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __attribute__((always_inline))
#endif

#ifndef MY_HDR_INCLUDED
#	define MY_HDR_INCLUDED_
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _WIN32
#define WANT_WINDOWS_H_
#endif

#ifdef WANT_WINDOWS_H_
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
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

#ifdef _WIN32	// NOTE: MSVC and mingw define _WIN32: 
				// to get WIN32 (no underscore) requires Windows.h 
				//(which may not be desirable, not included, even on Windows!)
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
#ifdef _WIN32
			static T rc = "\r\n";
#else
			static T rc = "\n";
#endif
			return rc;
		}
	};
#pragma warning (disable : 26485)

#ifndef PRIu8 // mingw doesn't seem to define these
#define PRIu8        "hhu"
#define PRIu16       "hu"
#define PRIu32       "u"
#define PRIu64       "llu"
#define PRIuLEAST8   PRIu8
#define PRIuLEAST16  PRIu16
#define PRIuLEAST32  PRIu32
#define PRIuLEAST64  PRIu64
#define PRIuFAST8    PRIu8
#define PRIuFAST16   PRIu32
#define PRIuFAST32   PRIu32
#define PRIuFAST64   PRIu64
#define PRIuMAX      PRIu64
#ifdef _WIN64
#define PRIuPTR  PRIu64
#else
#define PRIuPTR  PRIu32
#endif
#endif

#ifndef PRId8
#define PRId8        "hhd"
#define PRId16       "hd"
#define PRId32       "d"
#define PRId64       "lld"
#define PRIdLEAST8   PRId8
#define PRIdLEAST16  PRId16
#define PRIdLEAST32  PRId32
#define PRIdLEAST64  PRId64
#define PRIdFAST8    PRId8
#define PRIdFAST16   PRId32
#define PRIdFAST32   PRId32
#define PRIdFAST64   PRId64
#define PRIdMAX      PRId64
#ifdef _WIN64
#define PRIdPTR  PRId64
#else
#define PRIdPTR  PRId32
#endif
#endif
	
	#define HAVE_MY_OUTSTREAM
	struct outstream
	{
		using W = decltype(stdout); // probably FILE*
		outstream(W w) : m_w(w){}

		template <typename T>
		outstream& operator << (const T& t) { output(t); return *this; }


	protected:
		void output(const uint8_t c) { fprintf(m_w, "%" PRIu8, c); }
		void output(const int8_t c) { fprintf(m_w, "%" PRId8, c); }
		void output(const uint16_t c) { fprintf(m_w, "%" PRIu16, c); }
		void output(const int16_t c) { fprintf(m_w, "%" PRId16, c); }
		void output(const int32_t i) { fprintf(m_w, "%" PRId32, i); }
		void output(const uint32_t i) { fprintf(m_w, "%" PRIu32, i); }
		void output(const int64_t& i) { fprintf(m_w, "%" PRId64, i); }
		void output(const std::string& s) { fprintf(m_w, "%s", s.data()); }
		void output(const uint64_t i) { fprintf(m_w, "%" PRIu64, i); }
		void output(const double d) { fprintf(m_w, "%f", d); }
		void output(const float d) { fprintf(m_w, "%f", d); }
		void output(const endl_s <const char* const>& s) { (void)s; fprintf(m_w, "%s", s.c_str()); }
		void output(const char& c) { fprintf(m_w, "%c", c); }
		void output(const char* s) { fprintf(m_w, "%s", s); }
		template<typename T>
		void output(const T* t) { const intptr_t i = static_cast<intptr_t>(t); fprintf(m_w, "%" PRIuPTR, i); }

	

		W m_w;
	};

	template <typename T>
	struct out_t : public outstream
	{
		
		out_t(W w = stdout) : outstream(w) {
#ifndef MY_AVOID_STATIC_INITIALIZER_CHECKS
			if (w == stdout || w == stderr) {
				m_ctr++;
				assert(m_ctr <= 2); // checks only 1 copy per app, 1 cerr, 1 cout
			}
#endif
		}
	
		W file() { return m_w; }
		
	private:
		static int m_ctr;
		
	};
	template <typename T>
	int out_t<T>::m_ctr = 0; // ODL not violated for templates

#pragma warning (default : 26485)

#ifndef DEFINED_MY_OUTS
#	define DEFINED_MY_OUTS
	extern out_t<decltype(stdout)> cout;
	extern out_t<decltype(stderr)> cerr;
	extern endl_s<const char* const> endl;
#endif


} // namespace my




#ifndef OUT_DEFINED_MY
#define OUT_DEFINED_MY
#pragma warning (disable : 26493)

#define MY_DECLARE_OUTSTREAMS() \
my::out_t<decltype(stdout)> my::cout = my::out_t<decltype(stdout)>(stdout);\
my::out_t<decltype(stderr)> my::cerr = my::out_t<decltype(stdout)>(stdout);\
my::endl_s<const char* const> my::endl = my::endl_s<const char* const>();\
using my::cout;\
using my::cerr;\
using my::endl;

#pragma warning (default : 26493)


#ifndef WARN
#define WARN(expr)\
	do{ \
	if ((expr) )my::cout << " WARNING: " << #expr << my::endl; } while(0)
#endif

#ifdef MY_ASSERTS_IN_RELEASE_MODE
#	ifdef NDEBUG
#	undef NDEBUG
#	endif
#endif

#ifndef NDEBUG
#pragma warning (disable : 4130) // MS moaning about ASSERT("wtf" == 0);
#define ASSERT(expr) \
    do { \
		if (!(expr)) {\
        my::cerr << "Assertion failed: " << #expr << " , at:\n" << (__FILE__) \
		 << ":" << (__LINE__) << "\nin function: " << __FUNCTION__; assert(expr);} } while(0)

#else
#	define ASSERT(expr)
#endif
#pragma warning (default : 4130) // MS moaning about ASSERT("wtf" == 0);
#endif


