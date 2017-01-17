// my_memory.h
#include "my_type_safe.h"
#pragma once

#include <utility> // std::move

namespace my
{
	using sz_t = my::posint;
	template <typename T>
	struct ptrs {
		ptrs(T* const pbegin, T* const pend, T* const pcapacity = nullptr) 
			: p(pbegin), e(pend), c(pcapacity == nullptr ? pend : pcapacity)
		{
			if (p) {
				ASSERT(e && "ptrs: beginning but no end!");
				ASSERT(e >= p && "ptrs: end must be AFTER or EQUAL to the beginning!");
			}
		}

		ptrs(ptrs& other) { swap(*this, other); }
		ptrs(ptrs&& other) : p(other.p), e(other.e), c(other.c) {}
		ptrs& operator=(ptrs&& other) {
			swap(*this, other);
		}
		ptrs& operator=(ptrs other) {
			using namespace std;
			swap(*this, other);
		}
	
		friend void swap(const ptrs& one, const ptrs& two) {
			std::swap(one.p, two.p);
			std::swap(one.e, other.e);
			std::swap(one.c, other.c);
		}

		T* const p; // begin
		T* const e; // end
		T* const c; // capacity end, if different from e
	};

	template <typename T>
	class span
	{
	private:
		ptrs<T> m_p;
		void swap(span& s, span& other) {
			using namespace std;
			swap(s, other);
		}
	public:
		span(T* const start, T* const end, T* const capacity) : m_p(start, end, capacity) {}
		span(T* const start, sz_t size) : m_p(start, start + size) {}
		span(span& other) { swap(*this, other); }
		span(span&& other) { swap(*this, other); }
		span& operator=(span& other) { swap(*this, other); }
		span& operator=(span&& other) { swap(*this, other); }

		T* const begin() { return m_p.p; }
		T* const cbegin() const { return m_p.p; }
		T* const end() { return m_p.e; }
		T* const cend() const { return m_p.e; }
		T* const cap() const { return m_p.c; }
		T* const cap() { return m_p.c; }
		sz_t size() const { return m_p.e - m_p.p; }
		sz_t capacity() const { return m_p.c - m_p.p; }



		
	};
} // namespace my
