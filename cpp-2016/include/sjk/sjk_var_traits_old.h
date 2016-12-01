#pragma once
#ifndef SJK_VAR_TRAITS_H
#define SJK_VAR_TRAITS_H

#include "sjk_enum_flags.h"
#include <iostream>
#include <vector>

namespace sjk {
	
	namespace traits {


		typedef	enum class ty : unsigned int {
				none = 0, u8 = 1, i8 = 2, u16 = 4, i16 = 8,
				u32 = 16, i32 = 32, u64 = 64, i64 = 128, flt = 256, dbl = 512,
				str = 1024, vec = 2048,
				internal_storage = 16328, user_pod = 65536
			} types;

		enum class fail_code {
			noerror = 0, flags_too_high = -100, flags_not_exclusive, no_type_set, not_a_power_of_2,
			not_a_container, unknown
		};

		static inline bool is_numeric_type(const types t) {
			if ((int)t >= (int)types::str) return false;
			return true;
		}

		static inline bool is_unsigned_type(const types t) {
			using namespace sjk::enum_flags;
			if (!is_numeric_type(t)) return false;
			return (t & types::u16) || (t & types::u32) || (t & types::u8) || (t & types::u64);
		}

		static inline bool is_signed_type(const types t) {
			using namespace sjk::enum_flags;
			if (!is_numeric_type(t)) return false;
			return (t & types::i16) || (t & types::i32) || 
				(t & types::i8) || (t & types::i64) || (t & types::dbl) || (t & types::flt);
		}

		// string IS NOT counted as a container type, BTW.
		static inline bool is_container_type(const types t) {
			if ((int)t >= (int)types::vec) return false;
			return true;
		}


		// returns zero if this type could be a container, with the reason returned
	   static fail_code is_valid_container_type(types t) {
			using namespace  sjk::enum_flags;

			if (t & types::vec)
			{
				return fail_code::noerror;
			}
			else {
				return fail_code::not_a_container;
			}
		}

		static inline fail_code is_valid_basic_type(const types t)
		{
			if (t == types::none) return fail_code::no_type_set;
			if ((int)t > (int)types::str) return fail_code::flags_too_high;
			if (sjk::enum_flags::is_power_of_2(t)) {}
			else { return fail_code::not_a_power_of_2; }
			return fail_code::noerror;

		}

		static inline fail_code is_valid_type(const types t) {
			auto f = is_valid_container_type(t);
			if (f == fail_code::not_a_container) {
				return is_valid_basic_type(t);
			}
			else {
				return f;
			}
		
		}

#       ifndef REPORT_SIZE
#			define REPORT_SIZE(SZ) static constexpr size_t size_bits = SZ;
#		endif

		template <typename T>
		struct traits_extended{
			using type = std::is_standard_layout<T>;
			static constexpr bool value = false; };

		template <typename T> struct var_traits {
			static constexpr types value = types::user_pod; // we assume its a pod, and simply static_assert if it is not
			typedef T type;
			static_assert(!std::is_pointer<T>::value, 
				"pointers not supported in variant. You'll need to dereference it");

			static_assert(traits_extended<T>::type::value,
				"Steve says: unhandled variant type. Add a trait type and enum value for it.");

		};

		template <typename T> struct pod_type
		{
			typedef typename std::is_pod<T>::type type;
		};

		// incredibly, char , at least on Windows, is not the same type as either
		// uint8_t or int8_t
		template <> struct var_traits<char> {
			typedef char type;
			static constexpr types value = types::u8;
			REPORT_SIZE(sizeof(char))
		};

		template <> struct var_traits<std::string> {
			typedef std::string type;
			static constexpr types value = types::str;
			REPORT_SIZE((size_t)~0) // no (valid) static size for string, we'll do that on the fly
		};

		template <> struct var_traits<std::true_type::type>
		{
			typedef std::true_type type;
			using t = var_traits<std::true_type::type>;
			static constexpr types value = types::user_pod;
			REPORT_SIZE( sizeof (type) )
		};

		
		template <> struct var_traits<uint8_t> {
			typedef uint8_t type;
			static constexpr types value = types::u8;
			REPORT_SIZE(sizeof(uint8_t))
		};
		template <> struct var_traits <int8_t> {
			typedef char int8_t;
			static constexpr types value = types::i8;
			REPORT_SIZE(1)
		};
		template <> struct var_traits <uint16_t> {
			typedef uint16_t type;
			static constexpr types value = types::u16;
			REPORT_SIZE(2)
		};
		template <> struct var_traits <int16_t> {
			typedef int16_t type;
			static constexpr types value = types::i16;
			REPORT_SIZE(2)
		};
		template <> struct var_traits <int32_t> {
			typedef int32_t type;
			static constexpr types value = types::i32;
			REPORT_SIZE(4)
		};
		template <> struct var_traits <uint32_t> {
			typedef uint32_t type;
			static constexpr types value = types::u32;
			REPORT_SIZE(4)
		};
		template <>  struct var_traits <uint64_t> {
			typedef uint64_t  type;
			static constexpr types value = types::u64;
			REPORT_SIZE(8)
		};
		template <> struct var_traits <int64_t> {
			typedef int64_t type;
			static constexpr types value = types::u64;
			REPORT_SIZE(8)
		};
		template <> struct var_traits <float> {
			typedef float type;
			static constexpr types value = types::flt;
			REPORT_SIZE(sizeof(float))
		};
		template <> struct var_traits <double> {
			typedef double type;
			static constexpr types value = types::dbl;
			REPORT_SIZE(sizeof(double))
		};

		template <typename T> struct var_traits <std::vector<T>> {

			typedef typename std::vector<T> type;
			typedef typename type::value_type vt;
			static constexpr types value = (types)((uint32_t)var_traits<vt>::value | (uint32_t)types::vec);
		};

		template <typename T>
		constexpr static inline sjk::traits::types deduce_type(const T&) noexcept {
			using traits = sjk::traits::var_traits<T>;
			return traits::value;
		}
		/*/
		template<class T>
		struct is_vector<std::vector<T> > {
			using type = T;
			static bool const value = true;
		};
		/*/

		template <typename T>
		static inline bool is_variant_convertible(const T& t, types ty)
		{
			if (ty == types::none) return true; // if we are empty, we convert to a default constructed whatever, so OK.
			auto mytype = deduce_type(t);
			if (mytype == ty) return true;
			if ((int)(mytype) >= (int)types::user_pod) return true; // we have to trust the user knows what they are doing

			if (is_numeric_type(ty) == is_numeric_type(mytype)) {
				return ty == mytype; // if we are not strict here, we will make an invalid conversion between say signed char and int (when signed char < 0)
			}
			if (is_container_type(ty) != is_container_type(mytype)) {
				return false;
			}
			
			using namespace sjk::enum_flags;
			if (ty & types::str) {
				if (!(mytype & types::str))
				{
					return false;
				}
				else {
					return true;
				}
			}
			
			return false;
		}

	} // namespace traits
} //namespace sjk

#endif
