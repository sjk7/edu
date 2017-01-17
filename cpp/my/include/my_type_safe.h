// my_type_safe.h
#pragma once

#include "my.h"
#include <numeric>

namespace my
{
	template <class Tag, class T, T min_value = std::numeric_limits<T>::min(), 
							T max_value = std::numeric_limits<T>::max(), T default_value = T()>
	struct minmax_t {
		minmax_t() : m_val(default_value) { bounds_check(); }

		// Explicit constructor:
		explicit minmax_t(T val) : m_val(val) { bounds_check(); }

		static constexpr T mini() { return min_value; }
		static constexpr T maxi() { return max_value; }

		// Explicit conversion to get back the impl:
		operator T() const { return m_val; }
		friend bool operator==(const minmax_t& a, const minmax_t& b) { return a.m_val == b.m_val; }
		friend bool operator!=(const minmax_t& a, const minmax_t& b) { return a.m_val != b.m_val; }

	protected:
		void bounds_check() const { ASSERT(m_val >= min_value && "(My) Bounds check failed. value lower than minimum.");
									ASSERT(m_val <= max_value && "(My) Bounds check failed. value higher than maximum.");}

		void assign(const T& newval) { m_val = newval; bounds_check(); }
		T value() const { return m_val; }
		T m_val;

	private:
		
	};

	
	
	template <class Tag, class T, T min_value = std::numeric_limits<T>::min(), 
									T max_value = std::numeric_limits<T>::max(),
									T default_value = T()>
	struct arithmetic : public minmax_t<Tag, T, min_value, max_value, default_value >
	{
		using me = arithmetic<Tag, T, min_value, max_value, default_value>;
		using mybase = minmax_t<Tag, T, min_value, max_value, default_value>;
	public:
		T operator+(const T& other) { this->m_val += other; this->bounds_check(); return this->m_val; }
		T operator-(const T& other) { this->m_val -= other; this->bounds_check(); return this->m_val; }
		T operator++() { T t = this->m_val; this->m_val++; this->bounds_check(); return t; }
		T& operator++(const int) { ++this->m_val; this->bounds_check(); return this->m_val; }
		T operator--() { T t = this->m_val; this->m_val--; this->bounds_check(); return t; }
		T& operator--(const int) { this->m_val--; this->bounds_check(); return this->m_val; }
		T operator*=(const T other) { this->m_val *= other; this->bounds_check(); return this->m_val; }
		T operator/=(const T other) { this->m_val /= other; this->bounds_check(); return this->m_val; }
		T& operator+=(const me other) { this->m_val += other->m_val; this->bounds_check(); return this->m_val; }
		T& operator+=(const T other) { this->m_val += other; this->bounds_check(); return this->m_val; }
		T& operator-=(const me other) { this->m_val -= other->m_val; this->bounds_check(); return this->m_val; }
		T& operator-=(const T other) { this->m_val -= other; this->bounds_check(); return this->m_val; }

		bool operator!() { return !this->m_val; }
		bool operator<(const me& other) { return this->m_val < other.value(); }
		bool operator>(const me& other) { return this->m_val > other.value(); }
		bool operator<=(const me& other) { return this->m_val <= other.value(); }
		bool operator>=(const me& other) { return this->m_val >= other.value(); }

	};

	template <class Tag, class T, T min_value = std::numeric_limits<T>::min(),
									T max_value = std::numeric_limits<T>::max(),
									T default_value = T()>
	class num : public arithmetic<Tag, T, min_value, max_value, default_value>
	{
		using myty = minmax_t<Tag, T, min_value, max_value, default_value>;
		using base_t = arithmetic<Tag, T, min_value, max_value, default_value >;
		using me = num<Tag, T, min_value, max_value, default_value >;
		


	public:
		num() {this->assign(default_value); }
		num(const T&& other) { this->assign(other); }
		num(const size_t other) { 
			this->assign(static_cast<T>(other)); 
		}
		template <typename X>
		num(const X other) {
			this->assign(static_cast<T>(other));
		}
		num& operator=(const T&& other) { this->assign(other); return *this; }
		
	};

	struct fake_t {};
	using posint = num<fake_t, int64_t, 0>;
	struct fake_neg_t {};
	using negint = num<fake_neg_t, int64_t, std::numeric_limits<int>::min(), 0>;

} // namespace my
