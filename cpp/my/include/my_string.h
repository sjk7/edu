// my_string.h : a minimal string class

#pragma once
#include "my_memory.h"
#include <utility> // std::min
#include <array>

namespace my
{
	inline int& some_int()
	{
		static int i = 77;
		return i;
	}
	template <my::sz_t::type SSO_CAP = 16, int fixed_string = 0, typename T = char>
	class string
	{
		static const int8_t GUARD = 2;

		struct dyn_t : public ptrs<T> {
			dyn_t() : ptrs<T>(0,0,0){}
			dyn_t(const dyn_t& other) {	m_d = other.m_d;}
			dyn_t& operator=(dyn_t& other) { std::swap(other.m_d, m_d); return *this; }
			dyn_t(dyn_t&& other) { 
				m_d = std::move(other.m_d); 
			}
			dyn_t& operator=(dyn_t&& other) { 
				swap(*this, other); return *this;
			}
		
			~dyn_t() { cleanup(); }
			void create(const bysz_t sz) {
				cleanup();
				m_d.p = new T[sz + GUARD];
				m_d.e = m_d.p + sz.to_size_t();
				m_d.c = m_d.e + GUARD;
			}
			void cleanup(){
				if (m_d.p) {
					delete[] m_d.p;
					m_d.p = nullptr;
					m_d.e = nullptr;
					m_d.c = nullptr;
				}
			}
		};
		// dynamic buffer when SSO_CAP exceeeded
		dyn_t m_dyn;
	public:
		// a count of Ts
		static constexpr my::sz_t sso_size() { return SSO_CAP; }
		~string() { cleanup(); }
		string() {  init(); }
		string(const string& rhs) : string() {  append(rhs.c_str()); }
		string& operator=(string rhs) { swap(rhs, *this); return *this;}
		string(string&& other) : string() { swap(*this, other); }
		string(const T* s) :string() { append(s); }
		string(const T c, posint n): string(){
			auto sz = n;
			if (n >= SSO_CAP) {
				m_dyn.create(sz.to_size_t());
				std::fill_n(m_dyn.m_d.p, m_dyn.size(), c);
			}
			else {
				std::fill_n(m_ar.begin(), sz.to_size_t(), c);
				m_sso_sz = n;
				
			}
			terminate_string();
		}
				
		friend outstream& operator<<(outstream& out, const string& me) {
			out << me.c_str();
			return out;
		}
		template <typename X>
		string& operator+=(const X& other) { return append(other.c_str()); }


		bool operator==(const T* other) {
			if (other == nullptr) return false;
			auto l = strlen(other);
			if (l != size()) return false;
			posint sz = std::min(l, size().to_size_t());
			auto mc = memcmp(other, c_str(), sz);
			if (mc == 0) return true;
			return false;
		}
		bool operator==(const string& other) { 
			if (other.size() != size())
			{
				return false;
			}
			auto sz = (std::min)(other.size(), size());
			auto cmp = ::memcmp(other.c_str(), c_str(), sz);
			return cmp == 0;
		}
		bool operator !=(const T* other) { return !(operator=(other)); }
		bool operator !=(const string& other) { return !(operator=(other)); }

		my::bysz_t size() const{ 
			auto sz = m_dyn.size();
			if (sz <= 0) {
				sz = m_sso_sz;
			}
			return sz;
		}

		my::bysz_t capacity() const { if (m_sso_sz > 0) return m_sso_sz; else return m_dyn.capacity(); }

		const T* c_str() const { return cbegin(); }

		const T* const cbegin() const {
			string* me = const_cast<string*>(this);
			return me->begin();
		}
		

		T* begin() {
			auto sz = m_dyn.size();
			auto ret = m_dyn.begin();
			if (sz <= 0) {
				ret = m_ar.data();
			}
			return ret;
			
		}

		string& append(const T* s) {
			auto sz = size();
			auto oldsize = sz;
			if (s) {
				auto sl = ::strlen(s);
				auto newsz = sl + oldsize;
				posint copied = 0;
				if (newsz >= SSO_CAP) {
					resize_dynamic(newsz, sz >= m_sso_sz);
					oldsize = sz;
					if (oldsize < m_sso_sz && m_sso_sz > 0) { // here, sz is OLD size, before the append. If there's stuff in there,
						// we don't want to lose it.
						span<T> dst (m_dyn.begin(), m_dyn.capacity());
						span<T> src (m_ar.data(), sz);
						memcpy(dst, src);
						copied = src.size();
					}
					
					auto cap = capacity();
					span<T> dst(m_dyn.m_d.p + oldsize, cap);
					span<T> src((char*)s, sl);
					memcpy(dst, src);
					m_sso_sz = -1;
					ASSERT(capacity() >= size() + GUARD);
				}
				else {
					
					// fits in small buffer
					m_dyn.m_d.e = m_dyn.m_d.p;
					ASSERT(m_dyn.size() == 0);
					ASSERT(m_dyn.capacity() >= sz);
					
					span<T> sp(m_ar.data(), m_ar.size());
					span<T> sps((char*)s, sl);
					memcpy(sp, sps);
					m_sso_sz = sps.size();
				}
				terminate_string();
			}
			
			return *this;
		}

		inline friend void swap(string& one, string& two) noexcept {
			using std::swap;
			swap(one.m_dyn, two.m_dyn); swap(one.m_ar, two.m_ar);
			swap(one.m_sso_sz, two.m_sso_sz);
		}

	private:

		void init(){
			m_sso_sz = 0;
			terminate_string();
		}
		void cleanup() { m_dyn.cleanup(); }
		my::sz_t::type m_sso_sz;
		mutable std::array<T, SSO_CAP + GUARD> m_ar;

		void resize_dynamic(const my::posint newsz, bool keep_data) {
			ASSERT(newsz >= sso_size()); // else why are you here?
			m_sso_sz = 0;
			if (m_dyn.capacity() <= newsz + GUARD) {
				
				auto dcap = (double)newsz;
				dcap *= 1.5;
				dcap += (double)GUARD;
				auto newcap = posint(dcap);
				T* ptemp = new T[newcap];
				T* ptempend = ptemp + newcap.to_size_t();
				if (keep_data && m_dyn.size()) {
					span<T> dst(ptemp, newcap); 
					span<T> src(m_dyn.m_d.p, m_dyn.size());
					memcpy(dst, src);
				}
				m_dyn.cleanup();
				m_dyn.m_d.p = ptemp;
				m_dyn.m_d.e = ptemp + newsz;
				m_dyn.m_d.c = ptemp + newcap;
			}
		}


		void terminate_string() {
			auto sz = m_dyn.size();
			if (sz == 0) {
				sz = m_sso_sz;
				ASSERT(m_ar.size() > SSO_CAP + 1);
				m_ar[sz] = '\0';
				m_ar[sz + 1] = '\0';
			}
			else {
				ASSERT(m_dyn.capacity() > (sz + 1));
				m_dyn.m_d.p[sz] = '\0';
				m_dyn.m_d.p[sz + 1] = '\0';
			}
		}



	};

} //namespace my