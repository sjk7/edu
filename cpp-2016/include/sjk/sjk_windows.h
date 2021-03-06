#pragma once
// sjk_windows.h

#ifndef __linux

#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef STRICT
#define STRICT
#endif


#ifdef WANT_WINDOWS_H
#	include <Windows.h>
#endif

#endif // #ifndef __linux