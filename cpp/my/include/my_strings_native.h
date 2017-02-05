// my_strings_native.h
#pragma once

#include <cctype>
#include <cstring>


namespace my {
	// helper utilities for C-style strings
	namespace cstrings {
		/*
		* Find the first occurrence of the byte string s in byte string l.
		*/

		const void *
			memmem(const void *l, size_t l_len, const void *s, size_t s_len)
		{
			char *cur, *last;
			const char *cl = (const char *)l;
			const char *cs = (const char *)s;

			/* we need something to compare */
			if (l_len == 0 || s_len == 0)
				return NULL;

			/* "s" must be smaller or equal to "l" */
			if (l_len < s_len)
				return NULL;

			/* special case where s_len == 1 */
			if (s_len == 1)
				return memchr(l, (int)*cs, l_len);

			/* the last position where its possible to find "s" in "l" */
			last = (char *)cl + l_len - s_len;

			for (cur = (char *)cl; cur <= last; cur++)
				if (cur[0] == cs[0] && memcmp(cur, cs, s_len) == 0)
					return cur;

			return NULL;
		}


		/*
		* The strrstr() function finds the last occurrence of the substring needle
		* in the string haystack. The terminating nul characters are not compared.
		*/
#ifndef strrstr
		inline const char* strrstr(const char *haystack, const char *needle)
		{
			const char *r = NULL;

			if (!needle[0])
				return (char*)haystack + strlen(haystack);
			while (1) {
				const char *p = strstr(haystack, needle);
				if (!p)
					return r;
				r = p;
				haystack = p + 1;
			}
		}
#endif

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
#pragma warning (disable : 26400)
			size_t size = 0;
			char *end = nullptr;

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

		/**
		* isolat1ToUTF8:
		* @out:  a pointer to an array of bytes to store the result
		* @outlen:  the length of @out
		* @in:  a pointer to an array of ISO Latin 1 chars
		* @inlen:  the length of @in
		*
		* Take a block of ISO Latin 1 chars in and try to convert it to an UTF-8
		* block of chars out.
		* Returns 0 if success, or -1 otherwise
		* The value of @inlen after return is the number of octets consumed
		*     as the return value is positive, else unpredictiable.
		* The value of @outlen after return is the number of ocetes consumed.
		*/
		int
			isolat1ToUTF8(unsigned char* out, int *outlen,
				const unsigned char* in, int *inlen) {
			unsigned char* outstart = out;
			const unsigned char* base = in;
			const unsigned char* processed = in;
			unsigned char* outend = out + *outlen;
			const unsigned char* inend;
			unsigned int c;
			int bits;
#pragma warning(disable : 4244)
			inend = in + (*inlen);
			while ((in < inend) && (out - outstart + 5 < *outlen)) {
				c = *in++;

				/* assertion: c is a single UTF-4 value */
				if (out >= outend)
					break;
				if (c < 0x80) { *out++ = c;                bits = -6; }
				else { *out++ = ((c >> 6) & 0x1F) | 0xC0;  bits = 0; }

				for (; bits >= 0; bits -= 6) {
					if (out >= outend)
						break;
					*out++ = ((c >> bits) & 0x3F) | 0x80;
				}
				processed = (const unsigned char*)in;
			}
			*outlen = out - outstart;
			*inlen = processed - base;
			return(0);
		}


		/*/
		* UTF16LEToUTF8:
		*@out:  a pointer to an array of bytes to store the result
		* @outlen:  the length of @out, in bytes. Make it at least twice the size of the input, else it doesn't always work properly.
		* Resize it after using the return value as a count of bytes.
		* @inb:  a pointer to an array of UTF - 16LE passed as a byte array
		* @inlenb:  the length of @in in BYTES
		*
		* Take a block of UTF - 16LE ushorts in and try to convert it to an UTF - 8
		* block of chars out.This function assumes the endian property
		* is the same between the native type of this machine and the
		* inputed one.
		*
		* Returns the number of bytes written, or -1 if lack of space, or -2
		* if the transcoding fails(if *in is not a valid utf16 string)
		*     The value of *inlen after return is the number of octets consumed
		*     if the return value is positive, else unpredictable.
		/*/
		static int
			UTF16LEToUTF8(unsigned char* out, int *outlen,
				const unsigned char* inb, int *inlenb)
		{
			unsigned char* outstart = out;
			const unsigned char* processed = inb;
			unsigned char* outend = out + *outlen;
			unsigned short* in = (unsigned short*)inb;
			unsigned short* inend;
			unsigned int c, d, inlen;
			unsigned char *tmp;
			const unsigned long xmlLittleEndian = 1;
			int bits;
#pragma warning(disable : 4244)
			if ((*inlenb % 2) == 1)
				(*inlenb)--;
			inlen = *inlenb / 2;
			inend = in + inlen;
			while ((in < inend) && (out - outstart + 5 < *outlen)) {
				if (xmlLittleEndian) {
					c = *in++;
				}
				else {
					tmp = (unsigned char *)in;
					c = *tmp++;
					c = c | (((unsigned int)*tmp) << 8);
					in++;
				}
				if ((c & 0xFC00) == 0xD800) {    /* surrogates */
					if (in >= inend) {           /* (in > inend) shouldn't happens */
						break;
					}
					if (xmlLittleEndian) {
						d = *in++;
					}
					else {
						tmp = (unsigned char *)in;
						d = *tmp++;
						d = d | (((unsigned int)*tmp) << 8);
						in++;
					}
					if ((d & 0xFC00) == 0xDC00) {
						c &= 0x03FF;
						c <<= 10;
						c |= d & 0x03FF;
						c += 0x10000;
					}
					else {
						*outlen = out - outstart;
						*inlenb = processed - inb;
						return(-2);
					}
				}

				/* assertion: c is a single UTF-4 value */
				if (out >= outend)
					break;
#pragma warning(disable : 4244)
				if (c < 0x80) { *out++ = c;                bits = -6; }
				else if (c < 0x800) { *out++ = ((c >> 6) & 0x1F) | 0xC0;  bits = 0; }
				else if (c < 0x10000) { *out++ = ((c >> 12) & 0x0F) | 0xE0;  bits = 6; }
				else { *out++ = ((c >> 18) & 0x07) | 0xF0;  bits = 12; }

				for (; bits >= 0; bits -= 6) {
					if (out >= outend)
						break;
					*out++ = ((c >> bits) & 0x3F) | 0x80;
				}
				processed = (const unsigned char*)in;
			}
			*outlen = out - outstart;
			*inlenb = processed - inb;
			return *outlen;
		}
#pragma warning(default : 4244)


	}




} // namespace strings

#pragma warning (default : 26400)