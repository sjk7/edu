// macros.h : handy C++ macros. Of course macros are evil, so I try to put them all in one place.
// Steve. Fri 17th Feb 2017. Open-source licence
#pragma once
#ifndef MACROS_H
#define MACROS_H

#include "runtime_error.h"

#ifndef NO_COPY
#define NO_COPY( class ) \
	class(const class& other) = delete;\
	class(const class&& other) = delete;\
	class& operator=(const class& other) = delete;\
	class& operator=(const class&& other) = delete;

#endif

#ifndef BOUNDS_CHECK
#define BOUNDS_CHECK(v,idx)\
				{\
					if ( idx >=  v.size() )\
					{\
						THROW_ASSERT(index < v.size(), -1, "Index", index , " out of bounds. Size is: ", v.size());\
					}\
				}
#endif




#endif // MACROS_H
