#pragma once
#ifndef SJK_COLLECTIONS_H
#define SJK_COLLECTIONS_H
#include <vector>
#include <map>
#include <cassert>
#include <algorithm>
#include "sjk_exception.h"
#include "sjk_macros.h"
#include "sorted_vector.h"
#include <ostream>



template <typename T>
static inline  std::ostream& stream_v(std::ostream& os, const std::vector<T>& v)
{
	for (const auto& thing : v) {
		os << thing << "\t";
	}
	os << "\n";
	return os;
}

template <typename T>
static inline  std::ostream& operator <<(std::ostream& os, const std::vector<T>& v)
{
	return stream_v(os, v);
}



namespace sjk
{

	template <typename T>
	static inline std::ostream& stream_vector(std::ostream& os, const std::vector<T>& v) {
		return stream_v(os, v);
	}
	struct sortable
	{
		enum class sortorder { asc, desc };
		enum class sortkind { value, index };
		struct sortflags {
			using T = uint32_t;
			static constexpr T none = 0; static constexpr T case_insens = 1; static constexpr T case_sens = 2;
			T flags{ 0 };
			sortflags(const T t) : flags(t) {}
			sortflags() {}
			operator T() { return flags; }
		};
		virtual void sort(sortorder order = sortorder::asc, sortkind = sortkind::value, sortflags flags = sortflags()) = 0;
		virtual ~sortable() {}
	};



	namespace collections
	{
		
		template <class T>
		struct is_vector {
			static bool const value = false;
		};

		template<class T>
		struct is_vector<std::vector<T> > {
			using type = T;
			static bool const value = true;
		};

		template<class T>
		struct is_vector<sorted_vector<T> > {
			using type = T;
			static bool const value = true;
		};

		template <class T>
		struct is_sorted_vector {
			static bool const value = false;
		};

		template<class T>
		struct is_sorted_vector<sorted_vector<T> > {
			using type = T;
			static bool const value = true;
		};
		
		template <typename T>
		struct name_values
		{
			using vec_t = std::vector<T>;
			using CI = typename vec_t::const_iterator;
			using IT = typename vec_t::iterator;
			using x = typename std::remove_pointer<T>::type;
			using keytype = typename x::keytype;
			using map_t = std::map<keytype, T>;
			bool push_back(const T& t)
			{
				static_assert(std::is_pointer<T>::value, "name_values should be POINTERS");
				if (exists(t->name())) return false;
				m_map[t->name()] = t;
				m_vec.push_back(t);
				return true;
			}


			T from_index(const typename vec_t::size_type i) {
				if (i >= size()) {
					SJK_EXCEPTION("index ", i, "out of bounds");
				}
				return m_vec[i];
			}


			T& from_key(const std::string& name)
			{
				if (m_map.find(name) == m_map.end()) {
					SJK_EXCEPTION("no item with name", name);
				}
				return m_map[name];
			}

			const vec_t& vec() const { return m_vec; }
			CI cbegin() const { return m_vec.cbegin(); }
			CI cend() const { return m_vec.cend(); }
			const map_t& map() const { return m_map; }

			typename vec_t::size_type size() const { return m_vec.size(); }
			IT begin() { return m_vec.begin(); }
			IT end() { return m_vec.end(); }
			CI cbegin() { return m_vec.cbegin(); }
			CI cend() { return m_vec.cend(); }
			bool empty() const { return m_vec.empty(); }
			void clear() { m_vec.clear(); m_map.clear(); }

			bool exists(const keytype& t) const { return m_map.find(t) != m_map.end(); }
			bool population_deferred() const { return m_flags == 1; }
			void population_deferred_set(bool deferred_value) { if (deferred_value) m_flags = 1; else m_flags == 0; }

		protected:
			vec_t m_vec;
			map_t m_map;
			int m_flags{ 0 };

		};

		template <typename V, typename CMP = typename std::less<typename V::value_type::second_type> >
		static inline void sort_by_pair_second(V& v, CMP c = CMP())
		{
			using VT = typename V::value_type;
			std::stable_sort(v.begin(), v.end(),
				[&](const VT& pr1, const VT& pr2)
			{
				return c(pr1.second, pr2.second);
			});
		}

		// sort a VECTOR of pair by the first item in the pair in an ordinary vector
		template <typename V, typename CMP = std::less<typename V::value_type::first_type>, typename U = V >
		static inline void sort_by_pair_first(V& v, CMP cmp = CMP(),
			typename std::enable_if<!sjk::collections::is_sorted_vector<U>::value>::type* = 0 ) 
		{
			using pr_t = typename V::value_type;
			std::stable_sort(v.begin(), v.end(), [&](const pr_t& pr1, const pr_t& pr2)
			{
				return cmp(pr1.first, pr2.first);
			});
			
		}

		// sort a VECTOR of pair by the first item in the pair, where V is a sorted_vector
		template <typename V, typename CMP = typename V::CMP, typename U = V >
		static inline void sort_by_pair_first(V& v, CMP cmp = CMP(),
			typename std::enable_if<sjk::collections::is_sorted_vector<U>::value>::type* = 0) 
		{
			using pr_t = typename V::value_type;
			std::stable_sort(v.begin(), v.end(), [&](const pr_t& pr1, const pr_t& pr2)
			{
				return cmp(pr1.first, pr2.first);
			});

		}


		/*!
		 *	actually erases elements with the match value in pair.second.
		 *  Returns how many elements were actually erased from v
		 */
		template <typename VEC, typename T>
		static inline size_t remove_pair_second_matches(VEC& v, const T& match)
		{
			using VT = typename VEC::value_type;
			auto it = std::remove_if(v.begin(), v.end(),
				[&](VT& pr) {return pr.second == match; }
			);
			const auto diff = static_cast<std::ptrdiff_t>(std::distance(it, v.end()));
			v.erase(it, v.end());
			assert(diff >= 0);
			return static_cast<size_t>(diff);
		}




	} // namespace collections
} // namespace sjk

#endif // SJK_COLLECTIONS_H
