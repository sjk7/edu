#pragma once
// my_memory_helpers.h
#include "my_type_safe.h"


#include <utility> // std::move

namespace my
{
	using sz_t = my::posint;
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
		ptrs(ptrs&& other) { m_d = std::move(other.m_d); reset(); }
		ptrs& operator=(ptrs&& other) { swap(*this, other); reset(); }
		ptrs& operator=(ptrs other) {
			using namespace std;
			swap(*this, other);
		}
	
		friend void swap(ptrs& one, ptrs& two) { std::swap(one.m_d, two.m_d); }

		T* begin() { return m_d.p; }
		ptrdiff_t capacity() const { return m_d.c - m_d.p; }
		ptrdiff_t size() const { return m_d.e - m_d.p; }

		
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
		d m_d;
	};

	template <typename T>
	class span
	{
	private:
		ptrs<T> m_p;
		void swap(span& s, span& other) {using namespace std;swap(s, other);}
	public:
		using iterator = T*;
		using const_iterator = const T*;
		using type = T*;

		span(T* start, T* end, T* capacity) : m_p(start, end, capacity) {}
		span(T* start, sz_t sz) : m_p(start, start + sz.iptr_t() ) {}
		span(span& other) { swap(*this, other); }
		span(span&& other) { swap(*this, other); }
		span& operator=(span& other) { swap(*this, other); }
		span& operator=(span&& other) { swap(*this, other); }

		T* const begin() { return m_p.m_d.p; }
		T* const cbegin() const { return m_p.m_d.p; }
		T* const end() { return m_p.m_d.e; }
		T* const cend() const { return m_p.m_d.e; }
		T* const cap() const { return m_p.m_d.c; }
		T* const cap() { return m_p.m_d.c; }
		sz_t size() const { return m_p.m_d.e - m_p.m_d.p; }
		sz_t capacity() const { return m_p.m_d.c - m_p.m_d.p; }

		bysz_t size_in_bytes() const { return bysz_t(m_p.m_d.e - m_p.m_d.p); }
		bysz_t capacity_in_bytes() const { return bysz_t(m_p.m_d.c - m_p.m_d.p); }
	
	};
} // namespace my
