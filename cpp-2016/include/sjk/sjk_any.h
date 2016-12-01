#ifndef SJK_VARIANT_H
#define SJK_VARIANT_H
#include <string>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <typeinfo>
#include <type_traits>
#include <iostream>
#include <vector>

namespace sjk
{
	struct any
	{
		struct buf
		{
			~buf(){
				clear();
			}

			static constexpr int SSO_SIZE = 64;
			template <typename T>
			buf(const T& t) : m_sz(sizeof(t)), m_pbuf(nullptr), m_ty(typeid(t).name()){
				static_assert(std::is_pod<T>::type::value, "any: must contain pods only");
				assert(m_sz >= 0);
				copy_in(reinterpret_cast<const char*>(&t));
			}

			template <typename T>
			buf (const std::vector<T>& v)
			{
				using vt = typename T::value_type;
				static_assert(std::is_pod<vt>::type::value, "any: vector must contain pods only");
				m_sz = v.size() * sizeof(T::value_type);
				m_pbuf = static_cast<char*>(malloc(usize()));
				assert(m_pbuf);
				memcpy(m_pbuf, v.data(), usize());
			}

			buf (const buf& other) : m_sz(other.m_sz), m_pbuf(nullptr), m_ty(other.m_ty){
				copy_in(other.data());
			}

			const char* data() const  {
				if (m_pbuf){
					return m_pbuf;
				} else {
					return &m_ssobuf[0];
				}
			}

			size_t usize() const{
				return static_cast<size_t>(m_sz);
			}
			int size() const{ return m_sz; }

			template <typename T> T&
			value() {
				if (empty()) throw "var is empty";
				const std::string& s = typeid(T).name();
				if (s != m_ty) {
					std::cerr << "any types do not match, casting " << s << " to " << m_ty << std::endl;
					throw "bad var cast";
				}
				T* pt = (T*)(data());
				return *pt;
			}


			bool empty() const{
				return m_sz == 0;
			}


			private:
			char m_ssobuf[SSO_SIZE];
			int m_sz;
			char* m_pbuf;
			std::string m_ty;

			void copy_in(const char* from){
				assert(m_sz >= 0);
				if (m_sz <= SSO_SIZE){
					memcpy(m_ssobuf, from, usize());
				}else{
					m_pbuf = static_cast<char*>(malloc(static_cast<size_t>(m_sz)));
					assert(m_pbuf);
					memcpy(m_pbuf, from, usize());
				}
			}

			void clear(){
				if (m_pbuf){
					free(m_pbuf);
					m_pbuf = 0;
				}
				m_sz = 0;
			}
		};
		private:
		buf m_buf;

		public:
		template <typename T>
		any (const T& t) : m_buf(t){}
		any(const any& other) : m_buf(other.m_buf){}


		template <typename T> T&
		value() {return m_buf.value<T>();}

	};

}

#endif // SJK_VARIANT_H
