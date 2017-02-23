// db-cpp.h : file-based simple database in one header
#pragma once
#ifndef DB_CPP_H_INCLUDED
#define DB_CPP_H_INCLUDED
#include <vector>
#include <cassert>
#include <fstream>
#include <string>


namespace cpp
{
	using strvec_t = std::vector<std::string>;
	template <typename T = const char*>
	strvec_t split(const std::string& s, const std::string& delim)
	{
		strvec_t ret;
		split(s, delim, ret);
		return ret;

	}

	size_t split(const std::string& str, const std::string& delim, strvec_t& v)
	{
		v.clear();
		size_t prev = 0, pos = 0; size_t ret = std::string::npos;

		do
		{
			pos = str.find(delim, prev);
			if (ret == std::string::npos) ret = pos;
			if (pos == std::string::npos) pos = str.length();
			std::string token = str.substr(prev, pos - prev);
			if (!token.empty()) v.push_back(token);
			prev = pos + delim.length();
		} while (pos < str.length() && prev < str.length());
		return ret;
	}

	template <typename T>
	struct numeric
	{
		using TY = numeric<T>;
		using UNDERLYING_TYPE = T;
		numeric() {}
		numeric(const T& other) : m_t(other) {}
		numeric(const TY& other) : m_t(other.m_t) {}
		numeric& operator=(const TY& other) { m_t = other.m_t; }
		T& operator()() { return m_t; }
		const T& operator()() const { return m_t; }
		bool operator==(const T& rhs) const { return m_t == rhs; }
		bool operator==(const TY& rhs)const { return m_t == rhs.mt; }
		bool operator!=(const T& rhs) const { return rhs != m_t; }
		bool operator!=(const TY& rhs)const { return rhs.m_t != m_t; }
		TY& operator++() { ++m_t; return *this; }

	private:
		T m_t{ T() };
	};

	class fixed_length_string
	{
	public:
		fixed_length_string(const std::string& s, size_t len) :
			m_len(len)
		{
			assign(s, len);
		}
		fixed_length_string() : m_len(0) {}
		size_t size_max() const { return m_len; }
		size_t size_contents() const { return value().length(); }
		void reserve(const size_t sz) { m_s.reserve(sz); }
		const std::string& value_fixed_len() const {
			return m_s;
		}
		const std::string& value() const {
			auto f = m_s.find('\0');
			if (f != std::string::npos) {
				return m_s.substr(0, f);
			}
			else {
				return m_s;
			}
		}

		void assign(const std::string& s, size_t len)
		{
			m_len = len;
			auto l = s.length();
			if (l >= m_len) {
				m_s = s.substr(0, m_len);
			}
			else {
				m_s = s;
				m_s.resize(m_len); // pad with nulls
			}
		}

	protected:
		size_t m_len;
		std::string m_s;

	};


	namespace db
	{
		using rw_t = uint64_t;
		using rowindex_t = numeric<rw_t>;
		using colindex_t = numeric<int16_t>;

		struct core
		{

		};
	}
}










#endif