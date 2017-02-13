#pragma once
#ifndef SJK_VARIANT_INCLUDED
#define SJK_VARIANT_INCLUDED

#include "sjk_exception.h"
#include <typeinfo>
#include <string>
#include <memory>
#include <iostream>
#include "sjk_collections.h"

namespace sjk
{
	struct var
	{
	private:
		struct iface{
			virtual ~iface(){}
			virtual std::string id() const = 0;
			virtual void* pv() const = 0;
			virtual const std::type_info& type_id() const = 0;
			virtual iface* clone() const = 0;
			virtual std::ostream& to_stream(std::ostream& os) = 0;

		};
#ifdef _MSC_VER
#pragma warning (disable: 26493)
#endif
		template <typename D>
		struct d : public iface
		{
		private:
            virtual ~d(){}
			//general case

			
			template<typename T>
			static inline void stream(std::ostream& os, const T& t){
				// return os << t;
				return;
			}

			template<typename T>
			static inline void stream(std::ostream& os, const std::string& t) {
				os << t;
			}

			template<typename T>
			static inline void stream(std::ostream& os, const char* t) {
				os << t;
			}

			//this is overload - not specialization
			template<typename T>
			static inline void stream(std::ostream& os, const std::vector<T>& v)
			{
				sjk::stream_vector(os, v);
			}
			
		public:
			typedef D value_type;

			d(const D& r) : m_d(r){}
			// yes, I know this will copy. Its inherently thread-safe this way.
			// var uses value semantics. We could make m_d a reference, but it could dangle.

			virtual std::string id() const override { return typeid(D).name();}
			virtual d* clone() const override { return new d(m_d); }
			value_type value() const{ return m_d;}
			virtual void* pv() const override{ return static_cast<void*>(&m_d); }
			virtual const std::type_info& type_id() const override{ return typeid(D);}

			virtual std::ostream& to_stream(std::ostream& os) override{
				stream(os, m_d); 
				return os;
			}
			D value() { return m_d; }
			D& value_ref() { 
				return m_d; 
			}
			//private:
			mutable D m_d;
			
		};

		std::unique_ptr<iface> m_p;

		public:
       virtual ~var(){}
		template <typename T>
		var(const T& t) : m_p(new d<T>(t)){
			// remove this assertion if you really have a use case for a pointer, but its prolly a mistake,
			// since I am a value type...
			static_assert(!std::is_pointer<T>::value, "Did you really mean to assign a pointer to a var??");
		}

		template <typename Q, size_t N> var(Q (&buf)[N]) : m_p(new d<std::string>(buf)){
			static_assert(!std::is_array<Q>::value, "blah");
		}

		var() : m_p(nullptr){}

		template <typename T>
		var(d<T>* p) : m_p(p){}

		var(var const & other) :
			m_p(other.m_p ? other.m_p->clone() : nullptr) {}


		var(var &&o) : 
			m_p(std::move(o.m_p))
		{ 
			// o.m_p = nullptr; <-- unique_pointer does this, hence:
			assert(o.m_p == nullptr);
		}


		var &operator=(var &&o)
		{
			if (this != &o)
			{
				m_p = std::move(o.m_p);
			}
			return *this;
		}

		friend std::ostream& operator<<(std::ostream& os, const var& v){
			if (!v.m_p) return os; // streaming an empty variant should be harmless
			v.m_p->to_stream(os);
			return os;
		}
		std::string id() const{
			if (m_p) return m_p->id();
			return std::string();
		}

		auto clone() const -> decltype(m_p->clone()){
			return m_p->clone();
		}

		template <typename T>
		
		bool operator==(const T& other) const  {
			if (!m_p) return false;
			auto& t1 = m_p->type_id();
			auto& t2 = typeid(T);

			if (t1 == t2){
				const T& r = cast<T>();
				return r == other;
			}else{
				std::cerr <<
				"var: operator==: WARNING: Cannot compare equality for types: "
				 << t1.name() << " AND " << t2.name() << "    "
				 << "Try cast()ing this variant to the known type and then compare like for like."
				 << std::endl;
			}
			return false;
		}

		template <typename T>
		// FIXME: should be const.
		bool operator != (const T& other) const { return !operator==(other); }

		bool operator==(const var& other) = delete;

#ifdef _MSC_VER
#pragma warning (disable : 26491)
#endif
		// this was T&, but now is T to ensure a copy is made, simplifying thread safety and keeping value semantics.
		// I expect std::move() to help us out when it can.
		template <typename T>
		T cast(T* = nullptr) const{
				
			
			if (!m_p){
				SJK_EXCEPTION("cast(): variant object is null");
			}
			if (typeid(T) != m_p->type_id()){
				std::cerr << "cast(): variant type mismatch.\nRequested type: " <<
							 typeid(T).name()<<  "\nActual type is: "<<  m_p->type_id().name() << std::endl;
				SJK_EXCEPTION("cast(): variant type mismatch.\nRequested type: ",
							  typeid(T).name(), "\nActual type is: ",  m_p->type_id().name());
			}
			void* pv = m_p->pv();

			T* p = static_cast<T*>(pv);
			return *p;
		}

		template <typename T>
		T* cast_ptr(T* = nullptr)
		{
			if (!m_p) {
				SJK_EXCEPTION("cast(): variant object is null");
			}
			if (typeid(T) != m_p->type_id()) {
				std::cerr << "cast(): variant type mismatch.\nRequested type: " <<
					typeid(T).name() << "\nActual type is: " << m_p->type_id().name() << std::endl;
				SJK_EXCEPTION("cast(): variant type mismatch.\nRequested type: ",
					typeid(T).name(), "\nActual type is: ", m_p->type_id().name());
			}
			void* pv = m_p->pv();

			T* p = static_cast<T*>(pv);
			return p;
		}

		bool empty() const{	return m_p == nullptr;}

		template <typename T>
		operator T*() { return static_cast<T*>(m_p.get()); }

		template <typename T>
		operator T() {	return cast<T>();}

		// get the *actual* *thing* in the underlying pointer

		template <typename T>
		operator T&() {
			using TT = typename std::decay<T>::type;
			d<TT>* p = dynamic_cast<d<TT>*>(m_p.get());
			if (!p) {
				SJK_EXCEPTION("variant reference cast failed");
			}
			return p->value_ref();
		}

		

#ifdef _MSC_VER
#pragma warning (default: 26493)
#endif

#ifdef _MSC_VER
#pragma warning (default : 26491)
#endif


	};






} // namespace sjk
#endif
