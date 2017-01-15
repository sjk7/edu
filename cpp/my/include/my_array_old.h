// my_array.h : the most basic building block for RAII arrays
#pragma once

// The reason I don't add the include here is that we may get significantly improved
// compile times if your source file #includes the things it needs
#ifndef MY_HDR_INCLUDED_
#	error Using my: please include my.h FIRST.
#endif

#ifndef MY_WANT_CHRONO_
#define MY_WANT_CHRONO_
#endif

#ifndef MY_WANT_THREAD_
#define MY_WANT_THREAD_
#endif

#ifndef WANT_TYPE_TRAITS
#define WANT_TYPE_TRAITS // we use enable_if
#endif

#include "my_memory.h"

namespace my {
	

	template <typename T, typename S = my::memory::size_type>
	struct array : no_copy{
		using size_type = S;
		using ptr_t = my::memory::ptrs<T> ;
	private:
		ptr_t m_d;
	public:
		array() noexcept { _init(); }
		array(const array& other) { return _copy_from(other); }
		~array() noexcept { _destroy(); }

		T& operator[](const int idx) {
			return m_d.m_p.p[idx];
		}

		S capacity() const noexcept { return m_d.cap; }
		S size() const { return m_d.size(); }

		const T* data() const noexcept { return m_d.m_p.p; }
		int empty() const noexcept { return m_d.m_p.sz == 0; }
		const T* begin() const noexcept { return m_d.p; }
		const T* end() const noexcept { return m_d.m_p.p + m_d.m_p.sz; }
		S resize(const S newsize, bool initialize = false) { return _resize(newsize, initialize); }

		array& append(const array& other) {

			const S sz = other.size();
			if (sz <= 0) return *this;
			S oldsize = size();
			S newsz = sz + oldsize;
			S nw = resize(newsz);
			if (nw != newsz) {
				return *this;
			}
			memory::memcpy((char*)begin() + oldsize, other.data(), sz);
			return *this;
		}

		// ////////////////////////////////////////////////////////////////////////////////////////////////
		// See: http://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom/3279550#3279550
		// ////////////////////////////////////////////////////////////////////////////////////////////////
		// move constructor
		array(array&& other)
			: array() // initialize via default constructor, C++11 only
		{
			swap(*this, other);
		}
		array& operator=(array other) { 
			swap(*this, other); 
			return *this; 
		}
		// copy & swap idiom:
		friend void swap(array& first, array& second) noexcept
		{
			// enable ADL (not necessary in our case, but good practice)
			using std::swap;
			swap(first.m_d, second.m_d);
		}
		// ////////////////////////////////////////////////////////////////////////////////////////////////
		// See: http://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom/3279550#3279550
		// ////////////////////////////////////////////////////////////////////////////////////////////////
		// DOES NOT free any memory.
		void clear(bool init = false) noexcept { 
			if (init) {
				// called from copy constructor, etc
				m_d = d{};
				return;
			}
			const auto sz = m_d.m_p.sz;
			m_d.m_p.e = m_d.m_p.p + sz;
			m_d.m_p.sz = 0;
			// NOTICE: we don't free any memory here: capacity stays the same, and we set the size to 0.
		}
		// after this: capacity() == size()
		// Use this if you want to free system memory: all other resizing does not free up memory for performance reasons.
		int shrink_to_fit() {
			if (m_d.sz <= 0) return 0;
			T* ptr = realloc(m_d.p, m_d.sz);
			if (!ptr) {
				return m_d.sz;
			}
			else {
				m_d.p = ptr;
				m_d.e = ptr + m_d.size;
				m_d.capacity = m_d.sz;
			}
			assert(size() == capacity());
			return m_d.sz;
		}
		static constexpr int GUARD = 2; // always make room for a termination of '\0\0' to make ourselves string-friendly.
		
		// reserve at least newsize elements in memory, without affecting size(), but will affect capacity()
		S reserve(const S newsize, bool initialize_memory = false) {
			const S ret = _resize(newsize, initialize_memory);
			assert(m_d.cap == ret);
			m_d.sz = oldsize;
			return ret;
		}

		S default_construct_all() { return _default_construct_elements(); }

		void push_back(const T& t) {
			
			static_assert(std::is_default_constructible<T>::value, "Need default constructor.");
			auto sz = size();
			resize(sz + 1);
			T* p = m_d.m_p.p + sz;
			memset(p, 0, sizeof(T)); // this should be safe. Its just memory until we copy on the next line
			*p = t;
		}
	private:

		// you definitely should *not* be using this, you will leak!

		S _default_construct_elements() {
			if (empty()) return 0;

			T* p = m_d.p;
			const T* const e = m_d.e;
			assert(e - p >= size());
			while (c < n)
			{
				*(++p) = T();
				n++;
			}
		}
		array& _copy_from(const array& other) {
			if (other == *this) return *this;
			_init();
			if (other.is_empty()) {
				return *this;
			}
			_resize(other.size(), true);
			memory::memcpy(m_d.p, other.data(), other.size());
			return *this;
		}
		void _init() { m_d.reset(); }
		void _destroy() { 
			if (m_d.p) {
				free(m_d.p);
				m_d.p = nullptr; m_d.e = nullptr;
			}
			_init(); 
		}
		
		// caution: newsize parameter is in T units,
		// output is in BYTES
		S _mallocsize(const S newsize) {
			auto so = sizeof(T);
			auto bytes = so * newsize;
			double d((double)bytes);
			d *= 1.25;
			bytes = (size_t)((S)d);
			auto m = bytes % so;
			bytes += m;
			assert(bytes % so == 0);
			bytes += (GUARD * 2); // 2 extra BYTES, regardless of what T is
			return (int64_t)bytes;
		}


		S _resize(const S newsize, bool initialize = false, bool force_smaller = false) {
			assert(newsize >= 0);
			if (newsize == 0) {
				
				if (force_smaller) {
					_destroy();
				}
				else {
					_init();
				}
				return 0;
			}
			
			if (!force_smaller) {
				int64_t free_ele = m_d.m_p.cap - GUARD - newsize;
				if (free_ele <= 0) {

				}
				else {
					// we have enough room from an earlier resize:
					auto& p = m_d.m_p;
					p.sz = newsize;
					return p.sz;
				}
			}


			const S ns = _mallocsize(newsize);
			assert(ns > 0);
			if (m_d.m_p.p == 0) {
				if (initialize) {
					m_d.m_p.p = my::memory::calloc<T>(ns, (int64_t)1);
				}
				else {
					m_d.m_p.p = my::memory::malloc<T>(ns);
				}
				
			}
			else {
				T* ptr = (T*)my::memory::realloc(m_d.m_p.p, ns);
				if (!ptr) {
					return m_d.m_p.sz;
				}
				m_d.m_p.p = ptr;
				
			}

			
			if (m_d.m_p.p) { 
				m_d.m_p.cap = (int64_t)(ns / sizeof(T));
				m_d.m_p.sz = newsize;
				m_d.m_p.e = m_d.m_p.p + m_d.m_p.sz; 
			}
			else { _init(); }
			add_termination();
			return m_d.m_p.sz;
		}


		// we always terminate the buffer, so we are always safe to be turned into a string.
			// NOTE: the termination is always two BYTES,
			// regardless of what T is
		void add_termination() {

			if (!m_d.m_p.e) return; // empty means empty, though.
			char*  myp = (char*)m_d.m_p.e;
			const char* mye = ((const char*)end() )+ GUARD;
			const char* e = (char*)m_d.m_p.p + m_d.m_p.cap; // the "real" end (sssh, its a secret!)
			(void)e;
			auto d = mye - myp; assert(d == 2); (void)d;
			assert(mye <= e);
			
			
			while (myp < mye) {
				ASSERT(myp < e);
				*myp = 0;

				if (myp >= mye + GUARD) {
					break;
				}
				myp++;
			}
			return;
		}
	}; // my::array
} // namespace my
