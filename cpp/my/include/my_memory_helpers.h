#pragma once
// my_memory_helpers.h
#include "my_type_safe.h"
#include "my_allocator.h"


#pragma warning (disable: 558) // stupid MS std::copy() warnings!
#pragma warning (disable : 4127) // MS moaning about ASSERT("wtf" == 0);
#pragma warning (disable : 4130) // MS moaning about ASSERT("wtf" == 0);
#pragma warning (disable : 4996) // MS: ffs, std::copy is STANDARD
#include <utility> // std::move

#pragma warning (disable: 26481)
#pragma warning (disable: 26493)

#ifndef CHECK_BOUNDS
#	define CHECK_BOUNDS(idx, sz, msg){ ASSERT(idx < sz && idx >= 0 && #msg);}
#endif


namespace my
{
	template <typename T>
	inline my::mem::alloc<T>& myalloc() {
		static my::mem::alloc<T> a;
		return a;
	}
	using sz_t = my::posint;
	template<typename T>
	inline T* new_array(const sz_t sz,
		typename std::enable_if<!std::is_integral<T>::value>::type* = 0)
	{
		return new T[sz];
	}
	template<typename T>
	FORCEINLINE T* new_array(const sz_t sz,
		typename std::enable_if<std::is_integral<T>::value>::type* = 0)
	{
		bysz_t bysz = sizeof(T) * sz;
		return static_cast<T*>(malloc(bysz));
		//auto& a = myalloc<T>();
		//return a.next(sz);
	}

	template<typename T>
	inline void delete_array(T* p,
		typename std::enable_if<!std::is_integral<T>::value>::type* = 0)
	{
		ASSERT(p && "delete_array (not integral): you sent me null!");
		if (!p) return;
		return delete[] p;
	}
	template<typename T>
	FORCEINLINE void delete_array(T* p, const size_t,
		typename std::enable_if<std::is_integral<T>::value>::type* = 0)
	{
		ASSERT(p && "delete_array (integral): you sent me null!");
		if (!p) return;
		return free(p);
		//auto& a = myalloc<T>();
		//return a.release(p, sz);
	}


	template <typename T>
	inline T* realloc_array(const sz_t sz, T* p, const sz_t,
		typename std::enable_if<std::is_integral<T>::value>::type* = 0)
	{
		ASSERT(p && "realoc_array(integral): bad (null) existing pointer passed in");
		bysz_t bysz = sizeof(T) * sz;
		T* ptr = static_cast<T*>(realloc(p, bysz));
		if (ptr) {
			return ptr;
		}
		else { 
			throw std::bad_alloc();
		}
	}

	template <typename T>
	inline T* realloc_array(const sz_t sz, T* p, const sz_t oldsize,
		typename std::enable_if<!std::is_integral<T>::value>::type* = 0)
	{
		ASSERT(p && "realoc_array(not integral): bad (null) existing pointer passed in");
		T* ptr = new T[sz];
#pragma warning (disable : 4996) // MS: ffs, std::copy is STANDARD
		if (oldsize) {
			std::copy(p, p + oldsize, ptr);
		}
		delete[] p;
		return ptr;
	}

	template <typename T>
	T* new_array(T* pexisting, const sz_t oldsize, const sz_t newsize)
	{
		if (oldsize) {
			ASSERT(pexisting && "new_array(): if you want me to effectively realloc(), then I need the old pointer");
		}
		WARN(newsize == 0 && "new_array(): why are we allocating size zero?");
		
		if (oldsize == 0) {
			return new_array<T>(newsize);
		}
		if (pexisting && oldsize) {
			return realloc_array<T>(newsize, pexisting, oldsize);
		}
		ASSERT(0 && "new_array() : unexpected path leading to me returning nullptr");
		
		//auto& a = myalloc<T>();
		//T* pnew = a.next(newsize);
		//std::copy(pexisting, pexisting + oldsize, pnew);
		// return pnew;
		return nullptr;
	}

	template <typename T>
	struct ptrs {
		ptrs(T* pbegin, T* pend, T* pcapacity = nullptr) 
			: m_d(pbegin, pend, pcapacity)
		{
			if (m_d.p) {
				ASSERT(m_d.e && "ptrs: beginning but no end!");
				ASSERT(m_d.e >= m_d.p && "ptrs: end must be AFTER or EQUAL to the beginning!");
			}
		}
		ptrs() {}
		ptrs(ptrs&& other) { m_d = std::move(other.m_d); this->reset(); }
		ptrs& operator=(ptrs&& other) { swap(*this, other); this->reset(); }
		ptrs& operator=(const ptrs&& other) { swap(*this, other); this->reset(); }
		ptrs& operator=(ptrs other) { using namespace std; swap(*this, other); }
	
		friend void swap(ptrs& one, ptrs& two) { std::swap(one.m_d, two.m_d); }

		T* begin() { return this->m_d.p; }
		const T* end() const { if (!this->m_d.p) return 0; return m_d.e; ASSERT(m_d.e == m_d.p + size();) }
		ptrdiff_t capacity() const { if (this->m_d.p) return m_d.c - m_d.p; else return 0; }
		ptrdiff_t size() const { if (m_d.p)return m_d.e - m_d.p; else return 0; }

		T* const capacity_ptr() { return m_d.c; }
		T* const end_ptr() { return m_d.e; }
		T* const begin_ptr() { return m_d.p; }
		const T* const cbegin() const { return m_d.p; }
		const T* const cend() const { return m_d.e; }
		// lightweight option to change size(): just moves the end pointer: but make sure you have enough
		// capacity() first!!
		void set_size(const sz_t newsize) {
			m_d.e = m_d.p + newsize;
			ASSERT(m_d.c >= m_d.e && "set_size(): end must never be greater than capacity"); 
		}

	private:
		struct d {
			d(T* pbegin = 0, T* pend = 0, T* pcapacity = nullptr):
				p(pbegin), e(pend), c(pcapacity == nullptr ? pend : pcapacity){}
			
			T*  p; // begin
			T*  e; // end
			T*  c; // capacity end, if different from e
			void reset() {
				p = nullptr; e = nullptr; c = nullptr;
			}
		};
	protected:
		d m_d;
	};

	template <typename T>
	class span
	{
	private:
		ptrs<T> m_p;
		void swap(span& s, span& other) {using namespace std::swap(s.m_p, other.m_p);}
	public:
		using iterator = T*;
		using const_iterator = const T*;
		using type = T*;

		span(T* start, T* end, T* capacity) : m_p(start, end, capacity) {}
		span(T* start, sz_t sz) : m_p(start, start + sz.iptr_t() ) {}
		span(span& other) { 
			swap(*this, other); 
		}
		span(span&& other) { 
			swap(*this, other); 
		}
		span& operator=(span& other) { 
			swap(*this, other); }
		span& operator=(span&& other) { 
			swap(*this, other); }

		FORCEINLINE T* const begin() { return m_p.begin_ptr(); }
		const T* const cbegin() const { return m_p.cbegin(); }
		T* const end() { return m_p.end_ptr(); }
		const T* const cend() const { return m_p.cend(); }
		T* const cap() { return m_p.capacity_ptr(); }
		FORCEINLINE sz_t size() const { return m_p.size(); }
		sz_t capacity() const { return m_p.size(); }

		FORCEINLINE bysz_t size_in_bytes() const { return bysz_t(m_p.size()) * sizeof(T); }
		bysz_t capacity_in_bytes() const { return bysz_t(m_p.capacity()) * sizeof(T); }
	
	};

	// a dynamic buffer that has the notion of capacity() as well as size()
	// Non-copyable, but moveable
	template <typename T, uint8_t GUARD_SZ = 2>
	struct dyn_buf : public ptrs<T> {
		
		static constexpr uint8_t GUARD_SIZE() { return GUARD_SZ; }
		dyn_buf() : ptrs<T>(0, 0, 0) {}
		dyn_buf(const dyn_buf& other) = delete;// we can't be copied, only moved //{ this->m_d = other.m_d; }
		dyn_buf(dyn_buf&& other) { this->m_d = std::move(other.m_d); other.set_moved_from(); }
		// dyn_buf& operator=(dyn_buf other) { std::swap(other.m_d, this->m_d); return *this; }
		dyn_buf& operator=(dyn_buf&& other) { this->m_d = std::move(other.m_d); other.set_moved_from(); return *this;	}
		dyn_buf& operator=(const dyn_buf&& other) {	swap(*this, other);	return *this;}

		// you may access up to size() + GUARD_SZ  before I will start to complain,
		// regardless of if you have enough capacity()
		T& operator[](const sz_t idx) { 
			ASSERT(idx <= capacity());
			CHECK_BOUNDS(idx, size() + GUARD_SZ, "dyn_buf: index out of bounds.");
			return this->m_d.p[idx]; 
		}
		void reset() { this->m_d.e = this->m_d.p; }

		~dyn_buf() { cleanup(); }

		void create(const sz_t sz) {
			ASSERT(this->m_d.p == nullptr);
			this->m_d.p = new_array<T>(sz + GUARD_SZ);
			this->m_d.e = this->m_d.p + sz.to_size_t();
			this->m_d.c = this->m_d.e + GUARD_SZ;
		}
		FORCEINLINE void cleanup() {
			if (this->m_d.p) {
				delete_array(this->m_d.p, this->capacity());
				
				this->m_d.p = nullptr;
				this->m_d.e = nullptr;
				this->m_d.c = nullptr;
			}
		}

		// if you want to copy data from somewhere else
		// right into this buffer after its been resize()d, then you set copy_from and copy_size
		void resize(const sz_t newsize,
			bool keep_data = true,
			const T* const copy_from = nullptr,
			const sz_t copy_size = 0) 
		{
			auto newcap = capacity();
			auto oldsize = this->size();
			if (this->capacity() <= newsize + GUARD_SZ) {

				auto dcap = (double)newsize;
				dcap *= 1.25;
				dcap += static_cast<double>(GUARD_SZ);
				newcap = posint(dcap);
#pragma warning(disable : 26423)
				T* ptemp = nullptr;
				
				if (keep_data && oldsize) {
					ptemp = new_array(this->m_d.p, oldsize, newcap);
				}
				else {
					ptemp = new_array<T>(newcap);
				}
				this->m_d.p = ptemp;
				this->m_d.e = ptemp + newsize;
				this->m_d.c = ptemp + newcap;
			}

			ASSERT(copy_size <= newcap && "resize(): the size you want me to copy in is bigger than my capacity!");
			if (copy_size && copy_from) {
#ifdef NDEBUG
				::memcpy(this->m_d.p, copy_from, copy_size);
#else
				span<const T> src(copy_from, copy_size);
				span<T> dst(this->m_d.p, newcap);
				memcpy(dst, src);
#endif
				this->m_d.e = this->m_d.p + copy_size;
			}
			if (keep_data && oldsize) {
				auto how_much = newcap - oldsize;
				this->my_memset(how_much, this->m_d.p + oldsize);
			}
			else {
				// just memset all the new stuff
				this->my_memset(newcap - copy_size, this->m_d.p + copy_size);
			}
		}
		

		// don't do any cleanup when eventually destroyed if moved from
		void set_moved_from() {
			this->m_d.p = nullptr;
		}

	private:



		template <typename X = T>
		static FORCEINLINE void my_memset(sz_t sz, T* p,
			typename std::enable_if_t<std::is_integral<X>::value>* = 0)
		{
			memset(p, 0, sizeof(T) * sz);
		}
		
		template <typename X = T>
		static FORCEINLINE void my_memset(sz_t, T*,
			typename std::enable_if_t<!std::is_integral<X>::value>* = 0)
		{ /*/ a no-op for Ts with constructors./*/ }



	};


} // namespace my
#pragma warning (default: 26481)
#pragma warning (default: 26493)