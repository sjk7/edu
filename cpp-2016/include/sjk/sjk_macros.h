#pragma once
#ifndef SJK_MACROS_H
#define SJK_MACROS_H

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef __GNUC__
/*code for GNU C compiler */
#elif _MSC_VER
/*usually has the version number in _MSC_VER*/
#ifndef _HAS_ITERATOR_DEBUGGING
#define _HAS_ITERATOR_DEBUGGING 0
#else
#ifdef FORCE_ITERATOR_DEBUGGING_OFF
#undef _HAS_ITERATOR_DEBUGGING
#define _HAS_ITERATOR_DEBUGGING 0
#endif
#endif
#define _NO_DEBUG_HEAP  1
#define MSVC
#ifndef __FUNCTION__
#define __func__ __FUNCTION__
#endif
/*code specific to MSVC compiler*/
#elif __BORLANDC__
/*code specific to borland compilers*/
#elif __MINGW32__
/*code specific to mingw compilers*/
#endif

#ifndef CLASS_UNCOPIABLE
#define CLASS_UNCOPIABLE(C) \
  C(const C&) = delete; \
  C &operator=(const C&) = delete
#endif

#ifndef STRUCT_UNCOPIABLE
	#define STRUCT_UNCOPIABLE CLASS_UNCOPIABLE
#endif

#ifdef _WIN32
#ifndef STRICT
#define STRICT
#endif
#ifndef NOMINMIX
#define NOMINMAX
#endif
#ifndef NO_CASECMP_IN_SJK
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif
// its important ALL windows programs #include <windows.h> so any name clashes (macros, etc) are obvious right away.
#include "sjk_windows.h"
#endif //_WIN32

/*/
* CONFIG(release, debug|release) {
#This is a release build
DEFINES += SJK_RELEASE
} else {
#This is a debug build
DEFINES += SJK_DEBUG
}
/*/
#ifdef SJK_RELEASE
#ifndef NDEBUG
#ifndef SJK_RELEASE_ASSERTS // if you really, really want asserts() in release mode, define this
#define NDEBUG // prevent assert() working in release.
#endif
#endif
#endif

#ifndef IGN_UNUSED
#ifdef __GNUC__
#define IGN_UNUSED __attribute__ ((unused))
#else
#define IGN_UNUSED
#endif
#endif

#endif // SJK_MACROS_H
