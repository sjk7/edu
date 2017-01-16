// my_type_safe.h
#pragma once

#include "my.h"
namespace my
{
	
	template<class Tag, class T, T default_value = T()>
	class type
	{
	public:

		// Defaults to ID::invalid()
		type() : m_val(default_value) { }

		// Explicit constructor:
		explicit type(T val) : m_val(val) { }

		// Explicit conversion to get back the impl:
		operator T() const { return m_val; }

		friend bool operator==(const type& a, const type& b) { return a.m_val == b.m_val; }
		friend bool operator!=(const type& a, const type& b) { return a.m_val != b.m_val; }

	protected:
		void value_set(const T& newval) { m_val = newval; }
		T m_val;
	private:
		
	};
	
	
	template <class Tag, class T, T default_value = T()>
	struct arithmetic : public type<Tag, T, default_value >
	{
		using me = arithmetic<Tag, T, default_value >;
	public:
		T operator+(const T& other) { m_val += other; return m_val; }
		T operator-(const T& other) { m_val -= other; return m_val; }
		T operator++() { T t = m_val; m_val++; return t; }
		T& operator++(const int other) { ++m_val; return m_val; }
		T operator--() { T t = m_mt; m_val--; return t; }
		T& operator--(const int other) { m_val--; return m_val; }
		T operator*=(const T other) { m_val *= other; return m_val; }
		T operator/=(const T other) { m_val /= other; return m_val; }
		T& operator+=(const me other) { m_val += other.m_val; return m_val; }
		T& operator+=(const T other) { m_val += other; return m_val; }
		T& operator-=(const me other) { m_val -= other.m_val; return m_val; }
		T& operator-=(const T other) { m_val -= other; return m_val; }
		bool operator==(const me& other) { return other.m_val == m_val; }
		bool operator==(const T& other) { return other == m_val; }
		bool operator!=(const T& other) { return !(operator==(other, m_val;)) }
		bool operator!=(const me& other) { return !(operator==(other.m_val, m_val;)) }
		bool operator!() { return !m_val; }
		bool operator<(const me& other) { return value() < other.value(); }
		bool operator>(const me& other) { return value() > other.value(); }
		bool operator<=(const me& other) { return value() <= other.value(); }
		bool operator>=(const me& other) { return value() >= other.value(); }
		
		T value() const { return m_val; }
	protected:
		
		
		
	private:
		
		
	};

	template<class Tag, class T, T default_value = T()>
	class number_type : public arithmetic<Tag, T, default_value>
	{
		using myty = type<Tag, T, default_value>;
		using base_t = arithmetic<Tag, T, default_value >;
		using me = number_type<Tag, T, default_value >;
	public:
		
		explicit number_type(const T t) { myty::m_val = t; }
		number_type& operator=(const T& other) { myty::m_val = other; return *this; }
	};
	
	/*/
	template <typename T>
	class number : public arithmetic<T>
	{
		using mytype = number<T>;
	public:
		using type = T;
		number() { }
		number(number&& other) { value_set(other.m_t); }
		number(const number& other)  { value_set(other.m_t); }
		explicit number(const T other) { value_set(other); }
		number& operator= (const number& other) { m_t= other.m_t; return *this; }
		explicit operator T() const { return value(); }
		T assign(const T& other) { value_set(other); return value(); }
#ifdef HAVE_MY_OUTSTREAM
		friend outstream &operator<<(outstream &output,
			const mytype& i) {
			output << i.value();
			return output;
		}
#endif
		
	private:



		
	};

	/*/
} // namespace my
