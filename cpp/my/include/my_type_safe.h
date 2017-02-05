// my_type_safe.h
#pragma once

#include "my.h"
#include <numeric>

#ifndef NOERROR
#	define NOERROR 0
#endif
namespace my
{
	template <typename Tagg, typename T, T min_value = std::numeric_limits<T>::min(), 
							T max_value = std::numeric_limits<T>::max(), T default_value = T()>
	struct minmax_t {
		minmax_t() : m_val(default_value) { }

		// Explicit constructor:
		explicit minmax_t(T val) : m_val(val) { bounds_check(); }

		static constexpr T mini() { return min_value; }
		static constexpr T maxi() { return max_value; }

		
		operator T() const { 	return static_cast<T>(m_val); 	}
		
		intptr_t iptr_t() const{ return static_cast<intptr_t>(m_val); }
		friend bool operator==(const minmax_t& a, const minmax_t& b) { return a.m_val == b.m_val; }
		friend bool operator!=(const minmax_t& a, const minmax_t& b) { return a.m_val != b.m_val; }

	protected:
		FORCEINLINE void bounds_check() const { 
#ifdef NDEBUG
#else
			ASSERT(m_val >= min_value && "(My) Bounds check failed. value lower than minimum.");
			ASSERT(m_val <= max_value && "(My) Bounds check failed. value higher than maximum.");
#endif
		}

		FORCEINLINE void assign(const T& newval) { m_val = newval; bounds_check(); }
		T value() const { return m_val; }
		T m_val;

	private:
		
	};

	
	
	template <typename Tag, typename T, T min_value = std::numeric_limits<T>::min(), 
									T max_value = std::numeric_limits<T>::max(),
									T default_value = T()>
	struct arithmetic : public minmax_t<Tag, T, min_value, max_value, default_value >
	{
		using me = arithmetic<Tag, T, min_value, max_value, default_value>;
		using mybase = minmax_t<Tag, T, min_value, max_value, default_value>;
	public:
		
		
		T& operator-(const T other) { this->m_val -= other; this->bounds_check(); return this->m_val; }
		T operator++() { T& t = this->m_val; this->m_val++; this->bounds_check(); return t; }
		T& operator++(const int) { ++this->m_val; this->bounds_check(); return this->m_val; }
		T operator--() { T& t = this->m_val; this->m_val--; this->bounds_check(); return t; }
		T& operator--(const int) { this->m_val--; this->bounds_check(); return this->m_val; }
		T operator*=(const T& other) { this->m_val *= other; this->bounds_check(); return this->m_val; }
		T operator/=(const T& other) { this->m_val /= other; this->bounds_check(); return this->m_val; }
		T& operator+=(const me& other) { this->m_val += other->m_val; this->bounds_check(); return this->m_val; }
		T& operator+=(const T& other) { this->m_val += other; this->bounds_check(); return this->m_val; }
		T& operator-=(const me& other) { this->m_val -= other->m_val; this->bounds_check(); return this->m_val; }
		T& operator-=(const T& other) { this->m_val -= other; this->bounds_check(); return this->m_val; }

		bool operator!() { return !this->m_val; }
		
		bool operator<(const me& other) { return this->m_val < other.m_val; }
		template <typename X> bool operator <(const X& other) { return this->m_val < other; }

		template <typename X> bool operator >(const X& other) { return this->m_val > other; }
		bool operator>(const me& other) { return this->m_val > other.m_val; }

		bool operator<=(const me& other) { return this->m_val <= other.m_val; }
		template <typename X> bool operator <=(const X& other) { return this->m_val < other; }
		
		bool operator>=(const me& other) { return this->m_val >= other.m_val; }
		template <typename X> bool operator >=(const X& other) { return this->m_val >= other; }

	};

	template <typename Tag, typename T, T min_value = std::numeric_limits<T>::min(),
									T max_value = std::numeric_limits<T>::max(),
									T default_value = T()>
	class num : public arithmetic<Tag, T, min_value, max_value, default_value>
	{
		using myty = minmax_t<Tag, T, min_value, max_value, default_value>;
		using base_t = arithmetic<Tag, T, min_value, max_value, default_value >;
		using me = num<Tag, T, min_value, max_value, default_value >;
		


	public:
		using type = T;
		num() {this->assign(default_value); }
		num(const T&& other) { 
#ifndef NDEBUG
			this->assign(static_cast<T>(other));
#else
			this->m_val = static_cast<T>(other);
#endif
		}
		num(const size_t other) { 
			this->assign(static_cast<T>(other)); 
		}
		template <typename X>
		FORCEINLINE num(const X& other) {
#ifndef NDEBUG
			this->assign(static_cast<T>(other));
#else
			this->m_val = static_cast<T>(other);
#endif
		}
		template <typename X>
		num(const X&& other) {
#ifndef NDEBUG
			this->assign(static_cast<T>(other));
#else
			this->m_val = std::move(other);
#endif
		}
		// num& operator=(const T&& other) { this->assign(other); return *this; }
		
		T value() const { return static_cast<T>(this->m_val); }
		size_t to_size_t() const{ return static_cast<size_t>(this->m_val); }
	};

	struct fake_t {};
	using posint = num<fake_t, ptrdiff_t, 0>;
	struct fake_neg_t {};
	using negint = num<fake_neg_t, ptrdiff_t, std::numeric_limits<int>::min(), 0>;

	
	struct fake_cb_t {};
	// use this to count bytes, as opposed to sz_t for count of Ts
	using bysz_t = num<fake_cb_t, ptrdiff_t, 0>;



	struct fake_by_t_ {};
	using byte_t = num<fake_by_t_, ptrdiff_t>;

	struct fake_bby_t_ {};
	using sso_sz_t = num<fake_by_t_, ptrdiff_t, -1>;

} // namespace my
