// my_memory.h
#pragma once

#include "my.h"
#include <algorithm>


namespace my
{
	struct no_copy { no_copy() {} no_copy(const no_copy&) = delete; no_copy& operator=(const no_copy& other) = delete; };

	namespace memory
	{
		
		// What's all this? SIGNED types for memory functions? YES!
		// It makes mistakes and overruns easier to spot, and you don't have to
		// litter your code with casts.
		typedef int64_t size_type;

		template <typename T>
		int is_overlapping(const T x1, const T x2, const T y1, const T y2)
		{
			return std::max(x1, y1) <= std::min(x2, y2);
		}



		// Generic pointer struct to help you manage memory.
		template <typename T, typename ST = size_type>
		struct range {
			using type = T;
			using size_type = ST;
			using mytype = range<T, ST>;

			T const* p; T const *e; T const* e_real;
			range(T* ptr, T* end , T* ptr_e_real = nullptr)
				: p(ptr), e(end), e_real(ptr_e_real) {

				ASSERT(end >= ptr && "range: end cannot be before the begin");
				if (e && !e_real) {
					e_real = e;
				}
			}

			range(const T& t) : p(&t), e(p + sizeof(T)), e_real(p + sizeof(T)) {}
			
			T* begin() { return (T*)p; }
			T* end() { return (T*)e; }
			const T* const cbegin() const { return p; }
			const T* const cend() const { return e; }

			bool operator!() const { return !p; }
			operator bool() const { return p != nullptr; }

			bool empty() const { return p = e; }
			ST size() const { return e - p; }
			ST capacity() const { return e_real - p; }
			void reset() { p = nullptr; e = nullptr; e_real = nullptr; }

			static int is_overlapping(const mytype& a, const mytype& b)
			{
				return ::is_overlapping(a, b);
			}
		};
			

		// if you want a UDT that is reluctant to convert to an intrinsic, then I'm your man.
		// Very valuable for type safety
		template<typename T>
		struct numeric
		{
			using mytype = numeric<T>;
			

			explicit numeric(const int64_t i) : m_t(i) {}
			explicit numeric(const double d) : m_t((T)(d + 0.5)) {}
			numeric(const numeric& other) : m_t(other.m_t) {} // only allow implicit conversions to self
			numeric() : m_t(0) {}
			mytype& operator=(const numeric<T>& other) { m_t = other.m_t; }
			mytype& operator=(const T& other) { m_t = other; }

			// const type& operator() const { return *this; }
			const T& value() const { return m_t; }
			T& operator +=(T i) { m_t += i;	return m_t;	}
			
			template <typename X>
			int operator >= (const T other) const { return m_t >= other; }
			int operator >= (const int other) const { return m_t >= static_cast<T>(other); }
			int operator == (const mytype& other) { return m_t == other.m_t; }
			int operator == (const T& other) { return m_t == other; }
			int operator == (int& other) { return m_t == static_cast<T>(other); }

			operator bool() const { return m_t != 0; } // without this, cant do: if(thistype == 0) etc
		private:
			T m_t;
		};

		typedef numeric<int64_t> byte_size_t;


		template <typename T>
		int memory_overlaps(const range<T>& r1, const range<T>& r2) {
			return is_overlapping(r1.cbegin(), r1.cend(), r2.cbegin(), r2.cend());
		}


		template <typename T, typename SZ = size_type>
		static inline T* malloc(const byte_size_t sz)
		{
			
			ASSERT(sz >= 0 && "malloc: requested an invalid (negative) size)");
			WARN(sz == 0 && "malloc: requested 0 bytes");
			return (T*)::malloc(static_cast<size_t>(sz.value()));
		}

		template <typename T, typename SZ = size_type>
		static inline T* calloc(const SZ sz, const SZ sz_tcb = 1)
		{
			ASSERT(sz_tcb > 0 && "calloc: requested an invalid (negative or zero) size for clearing)");
			ASSERT(sz >= 0 && "calloc: requested an invalid (negative) size)");
			WARN(sz == 0 && "calloc: requested 0 bytes");
			return (T*)::calloc(static_cast<size_t>(sz), static_cast<size_t>(sz_tcb));
		}

		template <typename T, typename SZ = size_type>
		static inline T* realloc(const T* const p, const SZ sz)
		{
			ASSERT(p && "realloc: null pointer passed in!");
			ASSERT(sz > 0);
			WARN(sz == 0 && "realloc: requested 0 bytes");
			return (T*)::realloc((void*)p, static_cast<size_t>(sz));
		}

		template <typename T, typename SZ = size_type>
		static inline void memmove(T* dest, const T* src, const SZ sz) {

			ASSERT(dest && "memcpy: destination is null!"); (void)dest; (void)src; (void)sz;
			ASSERT(src && "memcpy: source is null!");
			ASSERT(sz >= 0 && "mempcy: size is negative!");
			WARN(sz == 0 && "memcpy: no-op as size is zero");
			if (sz == 0) return;
			memmove(dest, src, sz);
		}
				
		template <typename T, typename SZ = byte_size_t>
		static inline void memcpy(const range<T>& dest, const range<T>& src, const SZ sz) {
			
			ASSERT(dest && "memcpy: destination is null!"); 
			ASSERT(src && "memcpy: source is null!");
			ASSERT(sz >= 0 && "mempcy: size is negative!");
			WARN(sz == 0 && "memcpy: no-op as size is zero");
			
			if (sz == 0) return;
			if (memory_overlaps(dest, src)) {
				// memmove(dest, src, sz);
			}

		}




		template <typename T, typename SZ = size_type>
		static inline void memset(T* dest, const T t, const SZ sz) {

			ASSERT(sz >= 0 && "memset: size is negative!");
			ASSERT(dest && "memset: destination is null!"); (void)dest; (void)t; (void)sz;
			WARN(sz == 0 && "memset: no-op as size is zero");
			if (sz == 0) return;
		}

		template <typename T>
		struct dynamic_buffer
		{
		private:
			range<T> m_ptrs; // class layout is important.
		public:
			static constexpr int64_t GUARD = 2;
			dynamic_buffer(const size_type sz = 0) : m_ptrs(nullptr, nullptr)
			{
				create(sz);
			}
			~dynamic_buffer() {
				myfree();
			}
			
		private:
			

			void myfree() {
				if (m_ptrs.p) {
					free(m_ptrs.begin());
					m_ptrs.reset();
				}
			}

			byte_size_t get_capacity_for_size(byte_size_t& sz) {
				double d = (double)sz.value();
				static const double GROW_FACTOR = 1.3;
				d *= GROW_FACTOR;
				byte_size_t mysz (d);
				mysz += 2;
				return mysz;
			}

			void create(const size_type sz) {
				if (sz > 0) {
					myfree();
					grow(sz);
				}
			}

			void grow(const size_type sz) {
				byte_size_t bysize(sz * size_type(sizeof(T)));
				byte_size_t bysz = get_capacity_for_size(bysize);
				bysz += GUARD;
				m_ptrs.p = my::memory::malloc<T>(bysz);
				m_ptrs.e = m_ptrs.p + sz;
				m_ptrs.e_real = m_ptrs.p + m_ptrs.capacity();
			}
		};

		// sbo_t: small buffer optimization (you might want this for strings, arrays, etc).
		template <typename T, size_type N = 64>
		struct sbo_t
		{
			using type = T;
			using size_type = size_type;
			static constexpr size_type SBO_SIZE = N;
			static constexpr size_type SBO_MAX_DATA = SBO_SIZE - dynamic_buffer<T>::GUARD;
		private: // class layout is important
			range<T> m_ptrs;
			T buf[SBO_SIZE];
			dynamic_buffer<T> m_d;
		public:
			sbo_t() : m_ptrs(buf, buf, buf + SBO_SIZE)  {
				static_assert(N >= sizeof(wchar_t), "sso size too small");
				buf[0] = '\0';
				buf[1] = '\0';
			}

			size_type size() const { return m_ptrs.size(); }
			size_type capacity() const { return m_ptrs.capacity(); }

			T const* cbegin() const { return m_ptrs.cbegin(); }
			T const* cend() const { return m_ptrs.cend(); }
			T* begin() { return m_ptrs.begin(); }
			T* end() { return m_ptrs.end(); }

			size_type append(const T& t) {

				const size_type oldsz = size();
				const size_type newsz = oldsz+1;
				if (newsz >= SBO_MAX_DATA) {
					// grow
					assert(0);
				}
				else {
					byte_size_t bysz(newsz * (size_type)sizeof(T));
					range<T> r(t);
					memcpy(m_ptrs, r, bysz);
				}

				return 0;
			}

		private:

			

		};



	}
}
