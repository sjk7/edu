#pragma once
#ifndef STRINGS_H
#define STRINGS_H

#include <cstring>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <ostream>
#include <algorithm> // std::search

namespace cpp
{

	namespace strings
	{
		using namespace std::string_literals;
		// if using qt creator, put this in the .pro file:
		// CONFIG += c++1z # latest and greatest c++ version.


		using strvec_t = std::vector<std::string>;
		static constexpr const char  PIPE = '|';
		static constexpr const char  TAB = '\t';
		static constexpr const char  NL = '\n';
		static const auto DNLS = "\n\n"s;
		static const auto NLS = "\n"s;

		bool is_numeric(const std::string& number)
		{
			char* end = 0;

			try {
				std::strtod(number.c_str(), &end);
			}
			catch (...) {
				return false;
			}

			return end != 0 && *end == 0;
		}


		bool is_integer(const std::string& number)
		{
			return is_numeric(number.c_str()) && strchr(number.c_str(), '.') == 0;
		}

		// return the first position where needle is found in haystack, or std::string::npos if not found.
		static inline std::string::size_type split(const std::string& haystack,
			const std::string& delim,
			strvec_t& stringsout,
			bool clearvec = true)
		{

			strvec_t& result = stringsout;
			if (clearvec) result.clear();

			if (delim.empty()) {
				result.emplace_back(haystack);
				return std::string::npos;
			}

			size_t ret = haystack.find(delim);
			auto substart = haystack.begin(); 
			const auto end = haystack.cend();
			
			for (;;) {
				auto subend = std::search(substart, haystack.end(), delim.begin(), delim.end());

				if (subend == end && substart == subend) {
					break; // if the delimiter is right on the end, then this is the end.
				}

				result.emplace_back(substart, subend);
				if (subend == end) break;
				substart = subend + delim.size();
			}

			return ret;
		}


		strvec_t split(const std::string& s, const std::string& delim)
		{
			strvec_t ret;
			split(s, delim, ret);
			return ret;
		} // split (convenience version)

		strvec_t split(const std::string& s, const char delim)
		{
			strvec_t ret;
			split(s, std::string(1, delim), ret);
			return ret;
		} // split (convenience version)

		// Implement << for pairs: this is needed to print out mappings where range
		// iteration goes over (key, value) pairs.
		template <typename T, typename U>
		std::ostream& operator<<(std::ostream& out, const std::pair<T, U>& p)
		{
			out << "[" << p.first << ", " << p.second << "]";
			return out;
		}

		template <typename T>
		std::ostream& operator<<(std::ostream& os, const std::vector<T>& c)
		{
			for (const auto& thing : c) {
				os << thing << '\t';
			}

			os << '\n';
			return os;
		}

		template <typename K, typename V>
		std::ostream& operator<<(std::ostream& os, const std::map<K, V>& c)
		{
			for (const auto& thing : c) {
				os << thing << '\t';
			}

			os << '\n';
			return os;
		}
		template <typename V>
		std::ostream& operator<<(std::ostream& os, const std::set<V>& c)
		{
			for (const auto& thing : c) {
				os << thing << '\t';
			}

			os << '\n';
			return os;
		}

		inline void replace_all(std::string& source, const std::string& from, const std::string& to)
		{
			using namespace std;
			string newString;
			newString.reserve(source.length());  // avoids a few memory allocations

			string::size_type lastPos = 0;
			string::size_type findPos;

			while (string::npos != (findPos = source.find(from, lastPos)))
			{
				newString.append(source, lastPos, findPos - lastPos);
				newString += to;
				lastPos = findPos + from.length();
			}

			// Care for the rest after last occurrence
			newString += source.substr(lastPos);

			source.swap(newString);
		}





	} // namespace strings
} // namespace cpp

#endif // STRINGS_H
