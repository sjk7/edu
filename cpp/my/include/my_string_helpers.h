// my_string_helpers.h
#pragma once

#include <cctype>
#include <cstring>

namespace my {
	namespace string_helpers {

		/**
		* skip_spaces - Removes leading whitespace from @s.
		* @s: The string to be stripped.
		* 
		* Returns a pointer to the first non-whitespace character in @s.
		*/
		static inline char *skip_spaces(const char *str)
		{
			while (isspace(*str))
				++str;
			return (char *)str;
		}

		/**
		* strim - Removes leading and trailing whitespace from @s.
		* @s: The string to be stripped.
		*
		* Note that the first trailing whitespace is replaced with a %NUL-terminator
		* in the given string @s. Returns a pointer to the first non-whitespace
		* character in @s.
		*/
		char *strim(char *s, bool triml = 1, bool trimr = 1)
		{
			size_t size;
			char *end;

			size = strlen(s);

			if (!size)
				return s;
			if (trimr) {
				end = s + size - 1;
				while (end >= s && isspace(*end))
					end--;
				*(end + 1) = '\0';
			}
			
			if (triml) {
				return skip_spaces(s);
			}
			else {
				return s;
			}
		}
	}
}