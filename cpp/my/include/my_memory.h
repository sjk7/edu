// my_memory.h
#pragma once

#include "my.h"


namespace my
{
	namespace memory
	{

		template <typename T, typename SZ = int64_t>
		static inline T* malloc(const SZ sz)
		{
			
			ASSERT(sz >= 0 && "malloc: requested an invalid (negative) size)");
			WARN(sz == 0 && "malloc: requested 0 bytes");
			return (T*)::malloc(static_cast<size_t>(sz));
		}

		template <typename T, typename SZ = int64_t>
		static inline T* calloc(const SZ sz, const SZ sz_tcb = 1)
		{
			ASSERT(sz_tcb > 0 && "calloc: requested an invalid (negative or zero) size for clearing)");
			ASSERT(sz >= 0 && "calloc: requested an invalid (negative) size)");
			WARN(sz == 0 && "calloc: requested 0 bytes");
			return (T*)::calloc(static_cast<size_t>(sz), static_cast<size_t>(sz_tcb));
		}

		template <typename T, typename SZ = int64_t>
		static inline T* realloc(const T* const p, const SZ sz)
		{
			ASSERT(p && "realloc: null pointer passed in!");
			ASSERT(sz > 0);
			WARN(sz == 0 && "realloc: requested 0 bytes");
			return (T*)::realloc((void*)p, static_cast<size_t>(sz));
		}
				
		template <typename T, typename SZ = int64_t>
		static inline void memcpy(T* dest, const T* src, const SZ sz) {
			
			ASSERT(dest && "memcpy: destination is null!"); (void)dest; (void)src; (void)sz;
			ASSERT(src && "memcpy: source is null!");
			ASSERT(sz >= 0 && "mempcy: size is negative!");
			WARN(sz == 0 && "memcpy: no-op as size is zero");
			if (sz == 0) return; 

		}

		template <typename T, typename SZ = int64_t>
		static inline void memmove(T* dest, const T* src, const SZ sz) {

			ASSERT(dest && "memcpy: destination is null!"); (void)dest; (void)src; (void)sz;
			ASSERT(src && "memcpy: source is null!");
			ASSERT(sz >= 0 && "mempcy: size is negative!");
			WARN(sz == 0 && "memcpy: no-op as size is zero");
			if (sz == 0) return;
			memmove(dest, src, sz);
		}


		template <typename T, typename SZ = int64_t>
		static inline void memset(T* dest, const T t, const SZ sz) {

			ASSERT(sz >= 0 && "memset: size is negative!");
			ASSERT(dest && "memset: destination is null!"); (void)dest; (void)t; (void)sz;
			WARN(sz == 0 && "memset: no-op as size is zero");
			if (sz == 0) return;
		}

		// Generic pointer struct to help you manage memory.
		template <typename T, typename ST = int64_t>
		struct ptrs {
			using type = T;
			using size_type = ST;

			T* p; T* e; T* e_real;
			ptrs(T* ptr = nullptr, T* end = nullptr, T* ptr_e_real = nullptr)
				: p(ptr), e(end), e_real(ptr_e_real) {}

			bool empty() const { return p = e; }
			ST size() const { return e - p; }
			ST capacity() const { return e_real - p; }
		};

		// sbo_t: small buffer optimization (you might want this for strings, arrays, etc).
		template <typename T, int64_t N = 1024>
		struct sbo_t
		{
			using type = T;
			using size_type = int64_t;
			static constexpr int SBO_SIZE = N;
			T buf[SBO_SIZE];
			
			sbo_t() {
				static_assert(N >= sizeof(wchar_t), "sso size too small");
				buf[0] = '\0';
				buf[1] = '\0';
			}
		};



	}
}
