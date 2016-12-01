#pragma once
#ifndef SJK_STRINGS_H
#define SJK_STRINGS_H

#include "sjk_macros.h"
#include "sjk_memory.h" // for span<> and friends
#include <string>
#include <cstring> // memcpy
#include <vector>
#include <cctype> // std::to_lower
#include <algorithm> 
#include <string.h>

namespace sjk
{
	namespace str
	{
		using vec_t = std::vector<std::string>;
#ifndef _WIN32
		[[gnu::unused]] 
#endif
		static auto STR_NOT_FOUND = std::string::npos;

		static inline void sort_vector(vec_t& v, bool ascending = true)
		{
			if (ascending) {
				std::stable_sort(v.begin(), v.end(),
								 [](const std::string& s1, const std::string& s2)
				{
					return strcasecmp(s1.c_str(), s2.c_str()) < 0;
				});
			}
			else {
				std::stable_sort(v.begin(), v.end(),
								 [](const std::string& s1, const std::string& s2)
				{
					return strcasecmp(s2.c_str(), s1.c_str()) < 0;
				});
			}
		}

		static inline void to_lower(std::string& s) {
			std::transform(s.begin(), s.end(), s.begin(),  ::tolower);
		}

		static inline void to_upper(std::string& s) {
			std::transform(s.begin(), s.end(), s.begin(), ::toupper);
		}

		static inline void to_upper(vec_t& v) {
			for (auto& s : v) {
				to_upper(s);
			}
		}

		static inline void cstring_to_cstring(
				const char* from, const size_t from_size,
				char* to, const size_t to_size)
		{
			sjk::span<const char>spfrom(from, from_size);
			sjk::span<char> spto(to, to_size);
			spfrom.copy_string(spto);


		}
		// returns the number of characters copied from the cstring, including the null termination
		static inline size_t string_from_cstring(const char* pcstring,
												 const size_t cstring_size,
												 std::string& s,
												 bool clear_first = true)
		{
			if (clear_first) s.clear();
			sjk::span<const char> src(pcstring, cstring_size);
			const size_t sz = src.size_bytes();
			s.resize(sz); if (sz == 0) return 0;
			memcpy(&s[0], src.begin(), sz);
			return sz + 1;
		}
#ifdef _MSC_VER
#pragma warning(disable : 26481)
#endif
		static inline char* rtrim_cstring(char *string, char junk = ' ')
		{
			char *original = string + strlen(string);
			while (original != string && *--original == junk)
				;

			if (*original != '\0')
				original[*original == junk ? 0 : 1] = '\0';
			return string;
		}
#ifdef _MSC_VER
#pragma warning(default : 26481)
#endif

		// Returns the number of bytes copied, including the null terminator.
		// Handles the case where truncation is necessary.
		static inline size_t cstring_from_string(char* pcstring_to, const size_t cstring_size_to, const std::string& s_from)
		{
			sjk::span<char> dest(pcstring_to, cstring_size_to);
			return dest.copy(s_from);
		}

		static inline void split(const std::string& needle,
								 const std::string& haystack ,
								 vec_t &out,
								 bool clear_first = true) {

			if (clear_first) out.clear();
			size_t start = haystack.find_first_not_of(needle), end = start;

			while (start != STR_NOT_FOUND) {
				// Find next occurence of delimiter
				end = haystack.find(needle, start);
				// Push back the token found into vector
				out.push_back(haystack.substr(start, end - start));
				// Skip all occurences of the delimiter to find new start
				start = haystack.find_first_not_of(needle, end);
			}
		}

		static inline std::string& ltrim(std::string& str, const char trim_what = ' ')
		{
			auto it2 = std::find_if(str.begin(), str.end(), [&](char ch)
			{ return trim_what != ch; });

			str.erase(str.begin(), it2);
			return str;
		}

		static inline std::string& rtrim(std::string& str, const char trim_what = ' ')
		{
			auto it1 = std::find_if(str.rbegin(), str.rend(),
									[&](char ch)
			{ return trim_what != ch; });
			/*/
								The base iterator refers to the element that is next
								to the element the reverse_iterator is currently pointing to
						/*/
			str.erase(it1.base(), str.end());
			return str;
		}

		static inline std::string& trim(std::string& s, const char trim_what = ' ')
		{
			s = rtrim(s, trim_what);
			s = ltrim(s, trim_what);
			return s;
		}

		/*!
				Slower than trim(), hence the longer name. But more convenient to code
				*/
		static inline std::string trim_string(const std::string& s, const char trim_what = ' ')
		{
			std::string tmp(s, trim_what);
			return trim(tmp, trim_what);
		}

		static constexpr const char XPLAT_PATH_SEP = '/';
#ifdef _WIN32
		static constexpr const char NATIVE_PATH_SEP = '\\';
#else
		static constexpr const char NATIVE_PATH_SEP = '/';
#endif
		static inline std::string& xplat_file_path(std::string& path)
		{
			std::replace(path.begin(), path.end(), NATIVE_PATH_SEP, XPLAT_PATH_SEP);
			return path;
		}

	} // namespace str
} // namespace sjk


#endif // #ifndef SJK_STRINGS_H
