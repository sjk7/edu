#pragma once

#include "my_array.h"
#include "my_string_helpers.h"
#include <ctype.h> // ::toupper, ::tolower

namespace my
{
	template<class T> // silly to #include another header just for this.
	const T& min(const T& a, const T& b) { return (b < a) ? b : a;	}
	

	
	template <typename TY>
	struct string_t : public array<TY>
	{
		using array_t = array<string_t>;

	private:



		// only used if we have different pointers to underlying storage
		// (ie we advanced the begin() pointer instead of resizing when trim() was called, etc)
		
		#define SSO_SIZE  30
#ifndef WIN32
		static inline char* i64toa(char *dest, const int64_t src)
		{
			// sprintf(dest, "%" PRId64, src);
			// dest = ::_i64toa(src, dest, 10);
			return dest;
		}
#else
		static inline char* i64toa(char *dest, const int64_t src)
		{
			::_i64toa(dest, src);
			return dest;
		}

#endif

		// sso_t m_sso;
	public:

		static constexpr int64_t BIG_STR = -1;
		string_t() : m_lsso(0) {}

		template <typename T>
		string_t(const T& number) : m_lsso(0) {
			int64_t r = resize(SSO_SIZE, true);
			string_t::i64toa((char*)begin(), number);
			auto len = strlen(c_str(), int());
			resize(len);
		}

		string_t(int64_t len, char x): m_lsso(0)
		{
			resize(len);
			my::memory::memset((char*)begin(), x, len);
		}

#ifdef __linux__
#define _itoa ::itoa
#endif

		string_t(const int& number) : m_lsso(0) {
			int64_t r = resize(SSO_SIZE, true);
			::_itoa(number, (char*)begin(),10);
			auto len = strlen(c_str(), int());
			resize(len);
		}


		string_t(const int64_t number) : m_lsso(0) {
			int64_t r = resize(SSO_SIZE, true);
			string_t::i64toa((char*)begin(), number);
			auto len = strlen(c_str(), int());
			resize(len);
		}

		string_t(const string_t& other) : array<TY>(), m_lsso(0) {
			auto sz = other.size();
			/// m_ptrs = other.m_ptrs; <-- not needed, since this copy's pointers will be correct
			if (sz) {
				resize(sz);
				auto p1 = other.data();
				memory::memcpy((char*)begin(), p1, other.size());
				null_terminate();
			}
		}

		string_t(const char* p, int64_t cb = -1) : m_lsso(0) {
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
		friend void swap(string_t& first, string_t& second) noexcept
		{
			// enable ADL (not necessary in our case, but good practice)
			using std::swap;
			if (first.m_lsso > 0 || second.m_lsso > 0) {
				swap(first.m_sso, second.m_sso);
				swap(first.m_lsso, second.m_lsso);
			}

			if (first.m_ptrs.p || second.m_ptrs.p) {
				swap(first.m_ptrs, second.m_ptrs);
			}
			
			if (!first.base_array().empty() || !second.base_array().empty()) {
				swap(first.base_array(), second.base_array());
			}
		}
		// move constructor
		string_t(string_t&& other)
			: string_t() // initialize via default constructor, C++11 only
		{
			swap(*this, other);
		}
		string_t& operator=(string_t rhs) {
			clear(true); // avoid garbage if we are dealing with malloc()ed memory.
			swap(*this, rhs);
			rhs.clear(true);
			return *this;
		}
		string_t& operator=(const char* p) {
			clear();
			if (!p) return *this;
			int sz = strlen(p);
			append(p, sz);
			return *this;
		}


		int strlen(const char* p) {	return strlen(p, int()); }
		size_t strlen_st(const char* p) { size_t sz = 0; return strlen(p, sz); }
		
		void to_upper(string_t& ret) { transform(ret, ::toupper); }
		void to_lower(string_t& ret) { transform(ret, ::tolower); }

		void to_lower() { transform(*this, ::tolower); }
		void to_upper() { transform(*this, ::toupper); }

		template <typename T>
		string_t& operator +=(const T& other) {	return append(other);}
		
		string_t& operator +=(const int& other) { return append(other); }
		string_t& operator +=(const int64_t& other) { return append(other); }

		bool operator==(const string_t& rhs) {
			auto sz1 = size();
			auto sz2 = rhs.size();
			if (sz1 != sz2) { return false;}
			auto p1 = data(); auto p2 = rhs.data();
			return memcmp(p1, p2, sz1) == 0;
		}

		bool operator!=(const string_t& rhs) { return !operator==(rhs); }
		bool operator==(const char* other) { return string_t(other) == *this;	}
		const char* const c_str() const { return data(); }

		string_t& append(const char* p, int64_t len = -1)
		{
			if (!p) return *this;
			if (len < 0) len = strlen(p);
			if (len <= 0) return *this;
			auto oldsize = size();
			auto newsize = len + oldsize;
			auto actual = resize(newsize);
			assert(actual == newsize);
			memory::memcpy((char*)data() + oldsize, p, len);
			null_terminate();
			return *this;

		}
		string_t& append(const string_t& other)
		{
			if (other.empty()) return *this;
			return append(other.c_str(), other.size());
		}

		int64_t size() const {
			if (m_ptrs.p) {
				return m_ptrs.size();
			}
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
						if (m_lsso == SSO_SIZE) {
							my::memory::memcpy((char*)array<char>::begin(), (const char*)m_sso, m_lsso + sizeof(char));
							my::memory::memset(m_sso, (char)0, SSO_SIZE + GUARD);
						}
						else {
							my::memory::memmove((char*)begin(), m_sso, m_lsso + sizeof(char));
						}
						null_terminate();
						
					}
					m_lsso = BIG_STR;
				}
			}
			else {
				assert(m_lsso == BIG_STR);
				ret = array<char>::resize(newsize);
			}
			if (init_mem && ret > 0) {
				my::memory::memset((char*)data(), '\0', ret);
			}
			return ret;
		}
		const char* const data() const {
			
			if (m_ptrs.p) {
				return m_ptrs.p;
			}
			if (m_lsso >= 0) {
				return m_sso;
			}
			else {
				if (!empty()) {
					return array<char>::begin();
				}

			}

			if (!m_sso[1] == 0) {
				my::memory::memset( (char*)m_sso, (const char)0, SSO_SIZE);
			}
			return m_sso;


		}
		const char* const begin() const { return data(); }
		const char* const end() const {
			if (m_ptrs.e) { return m_ptrs.e; }
			if (m_lsso >= 0)  return m_sso + m_lsso;
			return array<char>::end();
		}

		void clear(bool b = false) { m_lsso = 0; array<char>::clear(b); m_ptrs = memory::ptrs<TY>{}; }
		bool empty() const { return m_lsso <= 0 && array<char>::empty(); }

		static constexpr int64_t npos = -1;

		// utility functions ... //
		// find the needle in this string.
		// Returns npos if not found, otherwise the zero-based position from the beginning.
		int64_t find(const string_t& needle, bool case_sensitive = true) const noexcept
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

		void trim(const bool trim_left, const bool trim_right) {
			ptrs<char> myptrs((char*)begin(),(char*) end());

			if (!myptrs.p) {
				return;
			}
			char* p = my::string_helpers::strim(myptrs.p, trim_left, trim_right);
			m_ptrs.p = p; m_ptrs.e = p + strlen(p);
		}
		
		void trim_l() { return trim(true, false); }
		void trim_r() { return trim(false, true); }
		
		// splits a string into substrings in the vector,
		// and returns the zero-based position of the first found position.
		// If the delimter is not found, v is emptied, and the return value is npos
		int64_t split(const string_t& d, my::array<string_t>& v)
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
				if (!found_ptr) {
					// don't miss out the very last block of text after the last found position:
					if (psrc < e) {
						auto final_len = e - psrc;
						my::string_t sfinal(psrc, final_len);
						v.push_back(sfinal);
					}
					break;
				}
				if (!found)	found = mysz - (e - found_ptr);
				int64_t sz = (int64_t)(found_ptr - psrc);
				assert(sz <= mysz);
				string_t s((const char*)psrc, sz);
				v.push_back(s);
				psrc += sz+1;
				if (psrc + d_len >= e) {
					break;
				}
			} while (found_ptr);

			string_t sresults("There were "); sresults += v.size(); sresults += " results: ";
			puts(sresults);

			for (const auto& thing : v)
			{
				puts(thing.c_str());
			}

			return found;

		}

	private:
		int64_t m_lsso;
		char m_sso[SSO_SIZE + 10];
		my::memory::ptrs<TY> m_ptrs{};

		TY* begin_non_const (){
			return static_cast<TY*>(begin());
		}

		// assumes you already tested for empty, etc: (this is why I am private)
		const char* const _find_ci(const string_t& the_needle) const noexcept
		{
			string_t haystack(*this); haystack.to_lower();
			string_t needle(the_needle); needle.to_lower();
			char* ptr = strstr((char* const)haystack.c_str(), needle.c_str());
			if (!ptr) return nullptr;
			const auto sz = haystack.size();
			auto pos = (sz - (haystack.end() - ptr));
			return begin() + pos;

		}

		// we always terminate with TWO nulls, so that we can handle someone using wchar_t
		void null_terminate() {
			char* p = (char*)end();
			*++p = '\0';
			*p = '\0';
		}

		array<char>& base_array() { return *this; }
		template <typename F>
		void transform(string_t& dst, F which = ::toupper) {
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


	typedef string_t<char> string;
}