// typedef.h

#pragma once
#include <cassert>
#include <limits>

namespace cpp
{

	template <typename T, T bad_value = 0,
			  T min_value = std::numeric_limits<T>::min(),
			  T max_value =  std::numeric_limits<T>::max()>
	struct tag_base
	{
		//static constexpr T min_value(){ return std::numeric_limits<T>::min() ;}
		//static constexpr T max_value(){ return std::numeric_limits<T>::max() ;}
		using IMPL = T;
		static constexpr T BAD = bad_value;
		static constexpr T MIN_VALUE = min_value;
		static constexpr T MAX_VALUE = max_value;
		//static constexpr T BAD_VALUE(){ return bad_value; }
	};

	template<typename TAG >
	// A class that "behaves" like a number, but is type-safe.
	class num
	{
		public:
		using impl = typename TAG::IMPL;
		using impltype = typename TAG::IMPL;
		using TAG_TYPE = TAG;

		static impl INVALID_VALUE(){
			static impl i = TAG::BAD;
			return i;
		}

		friend std::ostream& operator << (std::ostream& os, const num& n)
		{
			return os << n.m_val;
		}
		impl value() const
		{
			return m_val;
		}
		size_t to_size_t() const
		{
			return static_cast<size_t>(m_val) ;
		}

		num (const num& i) : m_val(i) {}
		num() : m_val(INVALID_VALUE()){}
		explicit num (const impl& i) : m_val(i) {}

		explicit num (const impl&& i)  noexcept : m_val(std::move(i))  {}

		num (const num&& i) noexcept : m_val(std::move(i)) {}
		num& operator=(const impl&& rhs) noexcept { m_val = std::move(rhs);return *this;}
		num& operator=(const num&& rhs) noexcept  { m_val = std::move(rhs.m_val); return *this; }
		num& operator=(const impl& rhs) noexcept	{ m_val = rhs; return *this; }
		num& operator=(const num& rhs) = default;


		explicit operator impl() const
		{
			return m_val;
		}

		friend bool operator==(const num a, const num b)
		{
			return a.m_val == b.m_val;
		}
		friend bool operator!=(const num a, const num b)
		{
			return a.m_val != b.m_val;
		}
		friend bool operator==(const impl a, const num b)
		{
			return a == b.m_val;
		}
		friend bool operator!=(const impl a, const num b)
		{
			return a != b.m_val;
		}
		friend bool operator==(const num a, const impl b)
		{
			return a.m_val == b;
		}
		friend bool operator!=(const num a, const impl b)
		{
			return a.m_val!= b;
		}


		num operator++(int)
		{
			auto oldval = m_val++;
			cr();
			return num(oldval);
		}
		num& operator++()
		{

			++m_val;
			cr();
			return *this;
		}
		num& operator+(int i)
		{
			m_val += i;
			cr();
			return *this;
		}
		num& operator+(const num& n)
		{
			m_val += n.m_val;
			cr();
			return *this;
		}

		num operator--(int)
		{
			auto oldval = m_val--;
			cr();
			return num(oldval);
		}
		num& operator--()
		{
			--m_val;
			cr();
			return *this;
		}
		num& operator-(int i)
		{
			m_val -= i;
			cr();
			return *this;
		}
		num& operator-(const num& n)
		{
			m_val -= n.m_val;
			cr();
			return *this;
		}

		num& operator*(const int i)
		{
			m_val *= i;
			cr();
			return *this;
		}
		num& operator*(const num& n)
		{
			m_val *= n.m_val;
			cr();
			return *this;
		}
		num& operator*=(const int i)
		{
			m_val *= i;
			cr();
			return *this;
		}
		num& operator*=(const num& n)
		{
			m_val *= n.m_val;
			cr();
			return *this;
		}

		num& operator/(const int i)
		{
			m_val /= i;
			cr();
			return *this;
		}
		num& operator/(const num& n)
		{
			m_val /= n.m_val;
			cr();
			return *this;
		}
		num& operator/=(const int i)
		{
			m_val /= i;
			cr();
			return *this;
		}
		num& operator/=(const num& n)
		{
			m_val /= n.m_val;
			cr();
			return *this;
		}
		bool operator >=(const num& rhs)  const { return m_val >= rhs.m_val;}
		bool operator >=(const int rhs)  const { return m_val >= rhs;}

		bool operator >(const num& rhs)  const { return m_val > rhs.m_val;}
		bool operator >(const int rhs)  const { return m_val > rhs;}

		bool operator <=(const num& rhs)  const { return m_val <= rhs.m_val;}
		bool operator <=(const int rhs)  const { return m_val <= rhs;}

		bool operator <(const num& rhs)  const { return m_val < rhs.m_val;}
		bool operator <(const int rhs)  const { return m_val < rhs;}

		private:
		impl m_val;

		void cr()
		{
			verify(m_val);
		}

		static inline bool verify( const impl& i )
		{
			(void)i;
			assert(i >= TAG::MIN_VALUE);
			assert(i <= TAG::MAX_VALUE);
			return true;
		}
	}; // struct num
} // namespace cpp
