#pragma once

#include "my_array.h"
#include <ctype.h> // ::toupper, ::tolower

namespace my
{
	template<class T> // silly to #include another header just for this.
	const T& min(const T& a, const T& b) { return (b < a) ? b : a;	}
	struct string : public array<char>
	{
		using array_t = array<string>;
	private:
		#define SSO_SIZE  30
#ifndef WIN32
		static inline char* i64toa(char *dest, const int64_t src)
		{
			sprintf(dest, "%" PRId64, src);
			return dest;
		}
#else
		static inline char* i64toa(char *dest, const int64_t src)
		{
			::_i64toa(dest, src);
			return dest;
		}

#endif

		char m_sso[SSO_SIZE + GUARD];
		// Only > 0 if we are using the sso
		int64_t m_lsso;
	public:

		static constexpr int64_t BIG_STR = -1;
		string() : m_lsso(0) {}
		string(const int64_t number) : m_lsso(0) {
			int64_t r = resize(SSO_SIZE, true);
			string::i64toa((char*)begin(), number);
			auto len = strlen(c_str(), int());
			resize(len);
		}

		string(const string& other) : array<char>(), m_lsso(0) {
			auto sz = other.size();
			if (sz) {
				resize(sz);
				memmove((char*)begin(), other.data(), other.size());
			}
		}

		string(const char* p, int64_t cb = -1) : m_lsso(0) {
			clear();
			if (!p ) return;
			auto sz = cb;
			if (sz <= 0) {
				sz = strlen(p, int64_t());
			}
			if (sz <= 0) return;
			append(p, sz);
		}

		// ////////////////////////////////////////////////////////////////////////////////////////////////
		// See: http://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom/3279550#3279550
		// ////////////////////////////////////////////////////////////////////////////////////////////////
		
		// copy & swap idiom:
		friend void swap(string& first, string& second) noexcept
		{
			// enable ADL (not necessary in our case, but good practice)
			using std::swap;
			if (first.m_lsso > 0 || second.m_lsso > 0) {
				swap(first.m_sso, second.m_sso);
				swap(first.m_lsso, second.m_lsso);
			}
			
			swap(first.base_array(), second.base_array());
		}
		// move constructor
		string(string&& other)
			: string() // initialize via default constructor, C++11 only
		{
			swap((string)*this, (string)other);
		}


		operator const char* const () {return c_str(); }

		string operator=(string rhs) {
			clear(); // avoid garbage if we are dealing with malloc()ed memory.
			swap(*this, rhs);
			return *this;
		}
		string& operator=(const char* p) {
			clear();
			if (!p) return *this;
			int sz = strlen(p);
			append(p, sz);
			return *this;
		}

		int strlen(const char* p) {	return strlen(p, int()); }
		size_t strlen_st(const char* p) { size_t sz = 0; return strlen(p, sz); }
		
		void to_upper(string& ret) { transform(ret, ::toupper); }
		void to_lower(string& ret) { transform(ret, ::tolower); }

		void to_lower() { transform(*this, ::tolower); }
		void to_upper() { transform(*this, ::toupper); }

		string& operator +=(const string& other) {	return append(other);}
		bool operator==(const string& rhs) {
			auto sz1 = size();
			auto sz2 = rhs.size();
			if (sz1 != sz2) { return false;}
			return memcmp(data(), rhs.data(), sz1) == 0;
		}

		bool operator!=(const string& rhs) { return !operator==(rhs); }
		const char* const c_str() const { return data(); }

		string& append(const char* p, int64_t len = -1)
		{
			if (!p) return *this;
			if (len < 0) len = strlen(p);
			if (len <= 0) return *this;
			auto oldsize = size();
			auto newsize = len + oldsize;
			newsize = resize(newsize);
			memcpy((char*)data() + oldsize, p, len);
			return *this;

		}
		string& append(const string& other)
		{
			if (other.empty()) return *this;
			return append(other.c_str(), other.size());
		}

		int64_t size() const {
			if (m_lsso >= 0) {
				return m_lsso;
			}
			else {
				return array<char>::size();
			}
		}

		int64_t resize(int64_t newsize, bool init_mem = false)
		{
			int64_t ret = size();
			if (m_lsso >= 0) {
				if (newsize <= SSO_SIZE) {
					// nothing to do: everything will fit.
					m_sso[newsize] = '\0';
					m_sso[newsize + 1] = '\0';
					ret = newsize;
					m_lsso = newsize;
				}
				else {
					ret = array<char>::resize(newsize);
					if (m_lsso > 0) {
						memcpy((char*)begin(), m_sso, m_lsso);
						memset(m_sso, 0, SSO_SIZE + GUARD);
					}
					m_lsso = BIG_STR;
				}
			}
			if (init_mem && ret > 0) {
				memset((char*)data(), 0, ret);
			}
			return ret;
		}
		const char* const data() const {
			
			if (m_lsso >= 0) {
				return m_sso;
			}
			else {
				if (!empty()) {
					return array<char>::begin();
				}

			}

			if (!m_sso[1] == 0) {
				memset( (void*)m_sso, 0, SSO_SIZE);
			}
			return m_sso;


		}
		const char* const begin() const { return data(); }
		const char* const end() const {
			if (m_lsso >= 0)  return m_sso + m_lsso;
			return array<char>::end();
		}

		void clear() { m_lsso = 0; array<char>::clear(); }
		bool empty() const { return m_lsso <= 0 && array<char>::empty(); }

		static constexpr int64_t npos = -1;

		// utility functions ... //
		// find the needle in this string.
		// Returns npos if not found, otherwise the zero-based position from the beginning.
		int64_t find(const string& needle, bool case_sensitive = true) const noexcept
		{
			auto sz = size();
			auto nsz = needle.size();

			if (nsz > size()) return npos;
			if (sz <= 0) return npos;
			if (needle.empty()) return npos;
			const char* ptr = nullptr;

			const auto cmpsize = my::min(nsz, sz);

			if (!case_sensitive) {
				ptr = _find_ci(needle);
			}
			else {
				ptr = strstr((char* const)c_str(), needle.c_str());
			}
			if (!ptr) return npos;
			return sz - (end() - ptr);
		}
		
		// splits a string into substrings in the vector,
		// and returns the zero-based position of the first found position.
		// If the delimter is not found, v is emptied, and the return value is npos
		int64_t split(const string& d, my::array<string>& v)
		{
			v.clear(); if (d.empty()) return npos;
			char* found_ptr = (char*)begin();
			char* psrc = (char*)begin();
			auto mysz = size();
			auto e = end();
			auto d_len = d.size();

			int64_t found = npos;

			do {
				found_ptr = strstr(psrc, d.data());
				if (!found_ptr) break;
				if (!found)	found = mysz - (e - found_ptr);
				int64_t sz = (int64_t)(found_ptr - psrc);
				assert(sz <= mysz);
				string s((const char*)psrc, sz);
				v.push_back(s);
				psrc += sz+1;
				if (psrc + d_len >= end()) {
					break;
				}
			} while (found_ptr);

			string sresults("There were "); sresults += v.size(); sresults += " results: ";
			puts(sresults);

			for (const auto& thing : v)
			{
				puts(thing.c_str());
			}

			return found;

		}

	private:

		// assumes you already tested for empty, etc: (this is why I am private)
		const char* const _find_ci(const string& the_needle) const noexcept
		{
			string haystack(*this); haystack.to_lower();
			string needle(the_needle); needle.to_lower();
			char* ptr = strstr((char* const)haystack.c_str(), needle.c_str());
			if (!ptr) return nullptr;
			const auto sz = haystack.size();
			auto pos = (sz - (haystack.end() - ptr));
			return begin() + pos;

		}

		array<char>& base_array() { return *this; }
		template <typename F>
		void transform(string& dst, F which = ::toupper) {
			auto sz = size();
			if (sz == 0) {
				dst.clear();
				return;
			}
			dst.resize(sz);
			char* p = (char*)dst.begin();
			const char* const e = dst.end();
			const char*  thisp = begin();
			F f = which;
			while (p < e) {
				*p++ = f(*(thisp++));
			}
		}

		template <typename T> static int strlen(const char* p, T dummy) {
			(void)dummy;
			if (!p) return 0;
			return static_cast<int>(::strlen(p));
		}
	};
}