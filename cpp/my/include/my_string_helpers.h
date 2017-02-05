#pragma once
// my_string_helpers.h
#include "my_strings_native.h"
#include "my_type_safe.h"

namespace my
{
	using find_t = sso_sz_t;
	// helper functions to add functionality to my::string



	namespace strings
	{
		template <typename STR> int validate_offset(const STR& s, int64_t& offset) {
			if (offset < 0) offset = 0;
			int bad = 0;
			if (offset >= s.size()) bad = 1;
			ASSERT(!bad && "find: invalid offset");
			if (bad) return -1;
			return NOERROR;
		}

		/// <summary>
		/// Finds the first substring equal to str, after the offset provided
		/// </summary>
		template<typename STR>
		find_t find(const char* needle, const STR& haystack, int64_t offset = -1) {
			
			ASSERT(needle&& "find needs a needle!");
			if (!needle) return -1;

			if (haystack.empty()) return -1;
			if (validate_offset(haystack, offset) != NOERROR) return -1;
						
			const char* p = strstr(haystack.c_str() + offset, needle);
			if (!p) {
				return -1;
			}
			return find_t(p - haystack.cbegin());
		}
		/// <summary>
		/// Finds the first substring equal to the first count characters of the 
		/// character string pointed to by needle. needle can include null characters.
		/// </summary>
		template <typename STR>
		find_t find(const STR& needle, const STR& haystack,  int64_t offset, int64_t count)
		{
			if (offset == -1) offset = 0;
			if (validate_offset(haystack, offset) != NOERROR) return -1;
			if (needle.empty() || haystack.empty()) return -1;
			STR myneedle = needle.substr(0, count);
			find_t f = find(myneedle.c_str(), haystack, offset);
			if (f == -1) return f;
			return f;
		}

		/// <summary>
		/// Find a needle in a haystack ** can handle nulls **
		/// </summary>
		template <typename STR>
		find_t find(typename STR::value_type needle, const STR& haystack,  int64_t offset = 0)
		{
			if (validate_offset(haystack, offset) != NOERROR) return -1;
			const char* p = static_cast<const char*>
				(::memchr(haystack.c_str() + offset, (int)needle, haystack.size() - offset));
			if (!p) return -1;
			return find_t(p - haystack.cbegin());
		}

		/// <summary>
		/// Find a needle in a haystack ** can handle embedded nulls **
		/// </summary>
		template <typename STR>
		find_t find(STR& needle, const STR& haystack, int64_t offset = 0)
		{
			if (validate_offset(haystack, offset) != NOERROR) return -1;
			const char* p = static_cast<const char*>
				(my::cstrings::memmem(haystack.c_str(), haystack.size(), needle.c_str(), needle.size()));
			if (!p) return -1;
			return find_t(p - haystack.cbegin());
		}

		
		template<typename STR>
		find_t find_rev(const char* needle, const STR& haystack) {
			
			ASSERT(needle && "find_rev needs a needle!");
			
			const char* p = my::cstrings::strrstr(haystack.c_str(), needle);
			if (!p) return -1;
			return find_t(p - haystack.cbegin());
		}
		
		template<typename STR> find_t rfind(const char* needle, const STR& haystack) { 
			return find_rev(needle, std::forward<const STR&>(haystack)); 
		}



	}
}