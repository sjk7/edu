#pragma once
#ifndef SJK_ENUM_FLAGS_H
#define SJK_ENUM_FLAGS_H

#include <type_traits>
#ifndef NOERROR
#define NOERROR 0
#endif

namespace sjk {
		namespace enum_flags {
		// A function definition that will only be called for enum types
#		ifndef FUNC_MATCH_ENUM
#			define  FUNC_MATCH_ENUM template <class E, class U = E ,\
			typename V = typename std::enable_if<std::is_enum<U>::value>::type > static inline
#		endif
#		ifndef USE_U_TYPE
#			define USE_U_TYPE(E) using T = typename std::underlying_type<E>::type;
#		endif

			template <typename Y>
			static int is_power_of_2(const Y lhs) {
				unsigned int x = (unsigned int)lhs;
				int ret =  ((x != 0) && !(x & (x - 1)));
				return ret;
			}

		// The bitwise operators operator&, operator|, operator^, operator~, operator&=, operator|=, and operator^=


		// --> logical OR read and write --------------------------
		FUNC_MATCH_ENUM	E operator |(const E l, const E r)
		{
			// USE_U_TYPE(E);
			return E(T(l) | T(r));
		}

		FUNC_MATCH_ENUM E& operator |= (E& lhs, E rhs) {

			// USE_U_TYPE(E);
			lhs = lhs | rhs;
			return lhs;
		}
		///////////////////////////////////////////////////////////
		// --> logical AND read and write -------------------------
                FUNC_MATCH_ENUM	unsigned int operator &(const E l, const E r)
		{
			USE_U_TYPE(E);
                        return (unsigned int)(T(l) & T(r));// ? true : false;
		}
		FUNC_MATCH_ENUM E& operator &= (E& lhs, E rhs) {
			USE_U_TYPE(E);
			lhs = (E)((T)lhs & (T)rhs);
			return lhs;
		}
		///////////////////////////////////////////////////////////
		// --> logical XOR read and write -------------------------
		FUNC_MATCH_ENUM	bool operator ^(const E l, const E r)
		{
			USE_U_TYPE(E);
			return E(T(l) ^ T(r));
		}
		FUNC_MATCH_ENUM E& operator ^= (E& lhs, E rhs) {
			USE_U_TYPE(E);
			lhs = (E)((T)lhs ^ (T)rhs);
			return lhs;
		}
		///////////////////////////////////////////////////////////
		// --> logical TWIDDLE (BITWISE NOT)  ---------------------
		FUNC_MATCH_ENUM	E operator ~(const E l)
		{
			USE_U_TYPE(E);
			return (E)~(T)(l);
		}

		FUNC_MATCH_ENUM unsigned int operator==(const unsigned int l , const E r) {
			return (unsigned int)r == l;
		}
		FUNC_MATCH_ENUM int operator==(const int l, const E r) {
			return (int)r == l;
		}



		FUNC_MATCH_ENUM
		int compare_powers(const E m_type, const E ty) {
			if (sjk::enum_flags::is_power_of_2(m_type))
			{
				if (!sjk::enum_flags::is_power_of_2(ty)) {
					return 1;
				}
			}
			return NOERROR;
		}

		} // namespace enum_flags
} // namespace sjk
#endif

