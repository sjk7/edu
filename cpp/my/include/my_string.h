// my_string.h : a minimal string class

#pragma once
#include "my_memory.h"
#include <utility> // std::min
#include <array>
#include "my_strings_native.h"
#include <ctype.h> // ::toupper, ::tolower

namespace my
{

	template <typename T = char, my::sz_t::type SSO_CAP = 40, int fixed_width = 0>
	class string_base
	{
		static const int8_t GUARD = 2;
		using dyn_t = my::dyn_buf<T, GUARD>;
		using size_type = sso_sz_t; // allows -1 for flexibility

		struct d
		{
			static constexpr int USING_DYN = -1;
			sso_sz_t m_sso_size{};
			std::array<T, SSO_CAP + GUARD> m_sso_store;
			FORCEINLINE void set_moved_from() {
				m_dyn.set_moved_from();
			}
			dyn_t m_dyn;
		};
		
		d m_d;
	public:

		using value_type = std::remove_const_t<T>;

		static const my::sz_t sso_max_capacity() noexcept { return SSO_CAP; }
		~string_base() {}
		string_base() noexcept { m_d.m_sso_store[0] = '\0'; }
		string_base(const string_base& rhs) {
			if (rhs.empty()) { ASSERT(m_d.m_sso_size == 0); return; }
			if (m_d.m_sso_size > 0) { // optimization for short string.
				m_d.m_sso_store = rhs.m_d.m_sso_store;
			}
			else {
				m_d.m_sso_size = 0; // so the call to size() in append() does not invalidate invariants
				append(rhs);
				m_d.m_sso_size = rhs.m_d.m_sso_size;
			}
		}

		

		string_base(string_base&& rhs) noexcept : string_base()   {
			::memcpy(&m_d, &rhs.m_d, sizeof(m_d));
			rhs.m_d.set_moved_from();
		}
		string_base& operator=(const string_base&& rhs)noexcept { return mymove(std::forward<string_base>(rhs)); }
		string_base& operator=(string_base&& rhs) noexcept { return mymove(std::forward<string_base>(rhs)); }
		string_base& operator=(const string_base& rhs) { reset(); append(rhs); return *this; }
		string_base& operator=(const T* s) { reset(); append(s); return *this; }
		

		// This could be implemented in terms of swap(), but this is fantastically faster
		string_base& mymove(string_base&& rhs) noexcept {
			// m_d = std::move(rhs.m_d);
			::memcpy(&m_d, &rhs.m_d, sizeof(m_d));
			rhs.m_d.set_moved_from();
			return (*this);
		}
		string_base(const T* s) :string_base() { append(s); }
		string_base(const T* s, sz_t sz) : string_base() { 
			resize(sz); 
			if (sz) {
				::memcpy(begin(), s, sz);
			}
		}
		string_base(const T c, posint n) : string_base() {
			auto sz = n;
			if (n >= SSO_CAP) {
				m_d.m_dyn.create(sz.to_size_t());
				std::fill_n(m_d.m_dyn.m_d.p, m_d.m_dyn.size(), c);
			}
			else {
				std::fill_n(m_d.m_sso_store.begin(), sz.to_size_t(), c);
				m_d.m_sso_size = n;

			}
			terminate_string();
		}

		FORCEINLINE bool empty() const noexcept { return size() == 0; }
		inline sz_t capacity() const noexcept { return m_d.m_sso_size >= 0 ? SSO_CAP : m_d.m_dyn.capacity(); }



		// the only way to dellocate memory once it has been allocated
		void shrink_to_fit() noexcept {
			auto sz = size();
			if (sz == 0) {
				cleanup();
				m_d.m_sso_size = 0;
			}
			else {
				auto cap = capacity();
				if (cap > sz) {
					m_d.m_dyn.resize(sz);
				}
			}
			reset();
		}

		friend outstream& operator<<(outstream& out, const string_base& me) { out << me.c_str();	return out; }
		
		// std::string: I'm looking at you!
		template <typename X>
		string_base& operator+=(const X& rhs) { return append(rhs.c_str(), rhs.size()); }
		template <typename X>
		string_base& operator+=(const T* rhs) { return append(rhs); }
		string_base& operator+=(const char* rhs) { return append(rhs); }


		bool operator==(const T* other) noexcept {
			if (other == nullptr) return false;
			my::sz_t l = strlen(other);
			my::sz_t l2 = strlen(data()); // NOTE: we are a STRING. size() might be huge with a zillion nulls after the first character.
			if (l != l2) return false;
			posint sz = std::min(l.to_size_t(), size().to_size_t());
			auto mc = memcmp(other, c_str(), sz);
			if (mc == 0) return true;
			return false;
		}
		bool operator==(const string_base& other) const noexcept {
			if (other.size() != size()) { return false; }
			auto sz = (std::min)(other.size(), size());
			auto cmp = ::memcmp(other.c_str(), c_str(), sz);
			return cmp == 0;
		}
		bool operator !=(const T* other) const noexcept { return !(operator=(other)); }
		bool operator !=(const string_base& other) { return !(operator=(other)); }
		bool operator<(const string_base& rhs) { 
			auto sz = (std::min)(rhs.size(), size()); 
			return memcmp(c_str(), rhs.c_str(), sz) < 0;}
		bool operator>(const string_base& rhs) {
			auto sz = (std::min)(rhs.size(), size());
			return memcmp(c_str(), rhs.c_str(), sz) > 0;
		}

		FORCEINLINE my::sz_t size() const noexcept { 
			
			return m_d.m_sso_size.value() >= 0 ? m_d.m_sso_size.value() : m_d.m_dyn.size();
		}

		const T* c_str() const noexcept { return cbegin(); }
		const T* const cbegin() const noexcept { string_base* me = const_cast<string_base*>(this);	return me->begin(); }
		
		T* begin() noexcept { return m_d.m_sso_size >= 0 ? m_d.m_sso_store.data() : m_d.m_dyn.begin(); }
		T* const data() noexcept { return begin(); }
		static inline posint strlen(const char* s) noexcept { 
			if (!s) return 0; return ::strlen(s); }
		static inline posint strlen(const wchar_t* s) noexcept { if (!s) return 0;  return ::wcslen(s); }

		// synonym for reset(). See also shrink_to_fit().
		void clear() { reset(); }
		// reset the string to a fresh state, without allocating or deallocating any memory. See also shrink_to_fit().
		void reset() noexcept { m_d.m_dyn.reset();  m_d.m_sso_size = 0; terminate_string(); }
		static sz_t max_size() noexcept{ return sz_t::maxi(); }
		T& operator[](const sz_t idx) { CHECK_BOUNDS(idx, size(), "string [] operator out of bounds"); return *(begin() + idx); }
		
		// operator const T* () const { return cbegin(); }
		
		string_base substr(sz_t start, sz_t length = 0) const {
			if (empty()) return string_base();
			if (length == 0) length = size() - start;
			string_base ret(cbegin() + start, length);
			return std::move(ret);
		}
		string_base& append(const string_base& other) {
			return append(other.c_str(), other.size());
		}
		
		string_base& append(const T* s, sso_sz_t newsize = -1)
		{
			auto oldsize = size();
			posint slen = newsize.value() >= 0 ? newsize.value() : strlen(s).value();
			newsize = oldsize + slen;
			if (newsize == 0) return *this;
			if (newsize < SSO_CAP) {
				::memcpy(&m_d.m_sso_store[0] + oldsize, s, slen * sizeof(T));
			}
			sz_t copy_size = 0;
			if (oldsize < SSO_CAP) {
				copy_size = m_d.m_sso_size;
			}
			else {
				copy_size = 0;
			}
			if (newsize < SSO_CAP) { 
				m_d.m_sso_size = newsize;
				terminate_string();
				return *this;
			}else { 
				m_d.m_sso_size = -1;	
			}
			bool need_extra_copy = false;
			const T* ptr = &m_d.m_sso_store[0];
			if (copy_size == 0) {
				ptr = s; copy_size = slen;
				need_extra_copy = false;
			}
			else {
				need_extra_copy = true;
			}

			m_d.m_dyn.resize(newsize, true, ptr, copy_size);
			if (need_extra_copy) {
				::memcpy(m_d.m_dyn.begin() + m_d.m_dyn.size(), s, slen);
			}
			terminate_string();
			return *this;
		}


		inline friend void swap(string_base& one, string_base& two) noexcept {
			using std::swap;
			swap(one.m_d, two.m_d); 
		}

	

		// unconditonally sizes the string to newsize. Does not free memory.
		// Always preserves existing data, even if it needs to truncate it.
		void resize(my::posint newsize, bool keep_data = true) {

			my::posint oldsize = size();
			if (newsize == oldsize) return;

			if (newsize < oldsize) {
				// this needs to be cheap
				auto dynsz = m_d.m_dyn.size();
				if (dynsz) {
					ASSERT(m_d.m_dyn.capacity() >= newsize && "resizing smaller: dynamic capacity not correct");
					m_d.m_dyn.set_size(newsize);
					m_d.m_sso_size = -1;
				}
				else {
					m_d.m_sso_size = newsize;
				}
				// we won't mess about going to and from sso once the buffer has been made:
				terminate_string();
				return;
			}

			if (newsize >= SSO_CAP) {
				const auto cap = capacity();
				int n = (int)newsize; (void)n;
				if (newsize >= cap) {
					resize_dyn(newsize, keep_data);
				}
				else {
					terminate_string();
					return;
				}


				m_d.m_sso_size = -1;
			}
			else {

				my::posint cpysize = std::min(newsize, oldsize);
				if (oldsize >= SSO_CAP) {
					if (keep_data) {
#ifdef NDEBUG
						::memcpy(m_d.m_sso_store.data(), cbegin(), cpysize);
#else
						span<T>dst(m_d.m_sso_store.data(), m_d.m_sso_store.size());
						span<const T>src(begin(), cpysize);
						memcpy(dst, src);
#endif
					}
				}
				m_d.m_sso_size = newsize.value();
				ASSERT(m_d.m_sso_size < SSO_CAP);
			}

			terminate_string();
		}



	private:


		void cleanup() noexcept { m_d.m_dyn.cleanup(); }

		void resize_dyn(const my::posint newsize, bool keep_data = true) {
			ASSERT(newsize >= SSO_CAP); // else why are you here?
			if (m_d.m_sso_size > 0 && keep_data) {
				// don't lose data across sso to dynamic buffer boundary
				m_d.m_dyn.resize(newsize, keep_data, m_d.m_sso_store.data(), m_d.m_sso_size);
			}
			else {
				m_d.m_dyn.resize(newsize, keep_data);
			}
		}


		FORCEINLINE void terminate_string() noexcept {
			auto sz = m_d.m_dyn.size();

			if (m_d.m_sso_size >= 0) {
				sz = m_d.m_sso_size;
				ASSERT(m_d.m_sso_store.size() > SSO_CAP + 1);
				m_d.m_sso_store[sz] = '\0';
			}
			else {
				auto cap = m_d.m_dyn.capacity();
				ASSERT(cap > (sz + 1)); (void)cap;
				m_d.m_dyn[sz] = '\0';
			}
		}

	public:

		string_base<char> toUTF8() const {

			string_base<char> sw;
			sz_t::type siz = size().value();
			sz_t::type bysz = (sz_t::type)(siz * sizeof(T) > 12 ? siz : 12);
			bysz += 1;
			sw.resize(bysz * sizeof(T));
			int sz = (int)sw.size();
			int inlenb = (int)(size() * sizeof(T));
			int inlen = inlenb;
			int ret = 0;
#pragma warning(disable : 4127)
			if (sizeof(T) == 2) {
				ret = my::cstrings::UTF16LEToUTF8((unsigned char*)sw.data(), &sz, (const unsigned char*)cbegin(), &inlenb);
			}
			else {
				int outlen = (int)(size() + 1 + 5); // some fudging to get the converter to work reliably
				inlenb += 5;
				sw.resize(outlen);
				ret = my::cstrings::isolat1ToUTF8((unsigned char*)sw.data(), &outlen, (const unsigned char*)cbegin(), &inlenb);
				if (ret == 0) {
					ret = outlen - 1;

				}
			}
			// ret is the number of bytes written.
			switch (ret)
			{
			case 0: {
				sw.resize(0);
				if (inlen) throw std::runtime_error("toUTF8: expected output, but there was none.");
				break;
			}

			case -1: {
				sw.resize(0);
				throw std::runtime_error("toUTF8: lack of space.");
				break;
			}
			case -2: sw.resize(0);   throw std::runtime_error("toUTF8: invalid characters. UTF-16 not valid."); break;
			default:
				sw.resize(ret);
#ifndef __llvm__
				return std::move(sw); // shouldn't , but oh! MSVC. FFS.
#else
				return sw;
#endif
			};

			return sw;
		}


	}; // string_base

	typedef string_base<char> str;
	typedef string_base<wchar_t> wstr;

} //namespace my