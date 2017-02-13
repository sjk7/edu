#pragma once
#ifndef SJK_DB_CACHE_H
#define SJK_DB_CACHE_H

#include "sjk_db.h"
#include "sjk_variant.h"
#include "sjk_collections.h"

#ifdef _MSC_VER
#	pragma warning(disable: 4702)
#endif

namespace sjk {
    namespace db {
        using index_t = sjk::db::index_t;
        using row_t = sjk::db::row_t;

        struct icache : public sortable {
            typedef std::string keytype;
            // virtual void on_read_record(const R& r) = 0;
            virtual void clear() = 0;
            virtual void reserve(const int64_t sz) = 0;
            virtual const std::string& name() const = 0;
			// Use this to get a value from a db row
            virtual sjk::var value(const sjk::db::row_t& rw) const = 0;
			// Use this to iterate values, if you must.
			virtual sjk::var value_at_index(const size_t idx) const = 0;
            
            virtual const std::string to_string(const row_t& rw) const = 0;
			// get the (file) index at row position
            virtual sjk::db::index_t index_at(const row_t& row) = 0;
            virtual int64_t size() const = 0;
            virtual int64_t delete_erased() = 0;
            virtual bool mark_erased(const row_t& idx) = 0;
            // used for an optimization where we won't need to sort
            // if we are already sorted by index ascending when records are deleted.
            virtual bool is_sorted_by_index() const = 0;
			virtual uint32_t flags() const = 0;
            virtual ~icache() {}
        };

        template <typename V>
        static inline size_t remove_if_bad(V&& v) {
            static sjk::db::row_type bad(sjk::db::row_t::ROW_BAD);
            return sjk::collections::remove_pair_second_matches(std::forward<V>(v), bad);
        }


        namespace detail{

			template < class T >
			inline std::ostream& operator << (std::ostream& os, const std::vector<T>& v)
			{
				os << "[";
				for (typename std::vector<T>::const_iterator ii = v.begin(); ii != v.end(); ++ii)
				{
					os << " " << *ii;
				}
				os << " ]";
				return os;
			}

            template <typename T>
            struct accessor
            {
                accessor(const T& t) : m_t(t) {}
                const T& m_t;
                const std::string value() const {
                    static std::string s; (void)s;
					// If I fail, you need to provide ostream& << for whatever is in here
					std::stringstream ss; ss << m_t;
					return std::string(ss.str());

                }
            };


            template <>
            struct accessor<std::string>
            {
                accessor(const std::string& t) : m_t(t) {}
                const std::string& m_t;
                const std::string& value() const {
                    return m_t;
                }
            };
        }

        template <typename VT>
        struct cache_store : public VT, public icache {
            using vec_t = VT;
            using A = typename VT::allocator_type;
            using V = typename VT::value_type;

            static_assert(sjk::collections::is_vector<VT>::value,
                          "Cache store must be templated on a vector");

            static_assert(std::is_same<typename V::second_type, index_t>::value,
                          "Cache store must be templated on a vector, where"
                          " the vector contains pairs, the first type is some T,"
                          " and the second type is an index");


            // Get the "value" when the cache contains a number of values
            // per index entry. (You'll get possibly one or more values in a vector,
            // or maybe just an empty vector, but you will get *something*)
            template <typename T,
                      typename U = typename T::size_type,
                      typename Z = typename T::value_type>
            static inline var cache_get_value(const T& val)  {
                return sjk::var(val);
            }
            // Get a "value" when the cache contains one value per index entry.
            template <typename T>
            static inline var cache_get_value(const T& val) {
                const auto& v = val.first;
                return sjk::var(v);
            }

            // sorting by index:
            template <typename T>
            static inline void sort_by_index(T& vec, const sortable::sortorder so) {
                if (so == sortable::sortorder::asc) {
                    sjk::collections::sort_by_pair_second(vec);
                }
                else {
                    sjk::collections::sort_by_pair_second(vec, std::greater<index_t>());
                }
            }

            // sorting the values when each index entry has one item:
            template <typename X, typename CMP, typename V, typename U = X>
            static inline void sort_by_pair_first(
                    V& vec,
                    typename std::enable_if<!sjk::collections::is_vector<U>::value>::type* =
                    0) {
                sjk::collections::sort_by_pair_first<V, CMP>(vec);
            }
            // sorting the values when each index entry can have multiple items:
            template <typename X,
                      typename CMP,
                      typename V,
                      typename = typename std::enable_if<
                          sjk::collections::is_vector<X>::value>::type>
            static inline void sort_by_pair_first(V& v) {
                CMP c;
                using pr_t = typename V::value_type;
                std::stable_sort(v.begin(), v.end(), [&](const pr_t& p1, const pr_t& p2) {
                    return c(p1.first, p2.first);
                });


            }

			/*/
			template <typename V,
						typename = typename std::enable_if<
					sjk::collections::is_vector<X>::value>::type>
				
				static inline void sort_by_pair_first(V& vec, case_insens_compare cmp) {
				auto& v = vec;
				
				using pr_t = typename V::value_type;
				std::stable_sort(v.begin(), v.end(), [&](const pr_t& p1, const pr_t& p2) {
					return cmp(p1.first, p2.first);
				});

			}
			/*/


            public:
            cache_store(const std::string& name, uint32_t flags = 0) :
				m_sname(name), 
				m_bsorted_by_index_ascending(true),
				m_flags (flags){}
            virtual ~cache_store() {}
            cache_store(const cache_store& c) = delete;
            cache_store& operator=(const cache_store& c) = delete;

            vec_t& values() { return *this; }
            const vec_t& values_const() const { return *this; }


			
            virtual void sort(sortorder order = sortorder::asc,
                              sortkind kind = sortkind::value, sortable::sortflags flags = sortable::sortflags()) override {

                using ft = typename V::first_type;
                using so_t = sjk::sortable::sortorder;
                m_bsorted_by_index_ascending = false;
				

                if (kind == sortkind::index) {
                    sort_by_index(*this, order);
                    m_bsorted_by_index_ascending = (order == sjk::sortable::sortorder::asc);
                } else {
                    if (order == so_t::asc) {
						if (flags & sortable::sortflags::case_insens) {
							sort_by_pair_first<ft, ci_cmp_less<ft>, VT>(*this);
						}
						else {
							if (flags & sortable::sortflags::case_sens) {
								sort_by_pair_first<ft, cs_cmp_less<ft>, VT>(*this);
							}
							else {
								sort_by_pair_first<ft, cs_cmp_less<ft>, VT>(*this);
							}
						}

                    } else {
						if (flags & sortable::sortflags::case_insens) {
							sort_by_pair_first<ft, ci_cmp_greater<ft>,  VT>(*this);
						}
						else {
							if (flags & sortable::sortflags::case_sens) {
								sort_by_pair_first<ft, ci_cmp_greater<ft>, VT>(*this);
							}
							else {
								sort_by_pair_first<ft, std::greater<ft>,   VT>(*this);
							}
						}
                    }
                }
            }

            public:
            virtual void clear() override {}
            virtual void reserve(const int64_t sz) override { VT::reserve(sz); }
            virtual const std::string& name() const override { return m_sname; }

			virtual sjk::var value_at_index(const size_t idx) const override
			{
				auto v = VT::at(idx);
				return v;
			}

            virtual var value(const row_t& rw) const override {
                auto v = rw.value();
#ifdef DB_CACHE_ACCESS_CHECK
                assert(v < size());
#endif
                const auto& val = VT::operator[](v);
				return std::move(val.first);
            }

            virtual const std::string to_string(const row_t& rw) const override {
                const auto i = rw.value();
                auto& val = VT::operator[](i).first;
                auto a = detail::accessor<typename VT::value_type::first_type>(val);
				return a.value();
            }

            virtual index_t index_at(const row_t& row) override {
                const auto& pr = this->at(row.value());
                return pr.second;
            }
            virtual int64_t size() const override { return VT::size(); }
            virtual int64_t delete_erased() override {
                const auto ret = remove_if_bad(*this);
                return ret;
            }
            virtual bool mark_erased(const row_t& idx) override {
                if (this->empty())
                    return false;
                auto& val = this->at(idx.value());
                ASSERT(val.second ==
                       idx);  // must be sorted and in agreement with what you are sending.
                val.second = sjk::db::index_type::ROW_BAD;
                return true;
            }

            virtual bool is_sorted_by_index()  const override { return m_bsorted_by_index_ascending; }
			virtual uint32_t flags() const override { return m_flags; }

            protected:
            std::string m_sname;
			
            // used for an optimization where we won't need to sort
            // if we are already sorted by index ascending when records are deleted.
            bool m_bsorted_by_index_ascending;
			uint32_t m_flags{ 0 };
        };

        template <typename T>
        using pr_ty = std::pair<T, index_t>;

        template <typename T>
        using vec_ty = std::vector< pr_ty<T> >;

        template <typename T>
        using pr_ty_multi = std::pair<std::vector<T>, index_t>;

        struct pop_null {};

        /*!
		* A cache where each cache item has a single entry.
		* A cache item consists of a pair of values: the first pair value is the value we want to store,
			and the second is the database index for this item.
		*/
        template <typename T>
        using cache_t = cache_store< vec_ty<T> >;

        /*!
 * A cache where each cache item can have multiple entries.
 * Where T is the type of data we want to store, and there are many of them:
 so we have a pair of < vector <values> , index >
 */
        template <typename T>
        using multicache_t = cache_store<std::vector<pr_ty_multi<T>>>;

    }  // namespace db

}  // namespace sjk

#endif  // SJK_DB_CACHE_H
