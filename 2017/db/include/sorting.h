// sorting.h
#pragma once
#include <vector>
#include <algorithm>
#include <cstdint>

#ifdef __linux
#define stricmp strcasecmp
#define _stricmp strcasecmp
#define strnicmp strncasecmp
#endif

namespace cpp
{
    enum class sortorder{ NONE = 0, ASC, DESC };

    namespace detail {
        using sortorder_t = cpp::sortorder;



        namespace sorting
        {
            template <typename T>
            inline void sort_int_values(T& v, const cpp::sortorder order) {
                if (order == cpp::sortorder::ASC) {
                    std::stable_sort(v.begin(), v.end(), [](
                                     const auto* lhs, const auto* rhs) { return lhs->second < rhs->second; });
                }
                else {
                    std::stable_sort(v.begin(), v.end(), [](
                                     const auto* lhs, const auto* rhs) { return lhs->second > rhs->second; });
                }
            }

            template <typename T>
            inline void sort_string_values(T& v, const sortorder order ) {
                if (order == cpp::sortorder::ASC) {
                    std::stable_sort(v.begin(), v.end(), [](const auto* lhs, const auto* rhs) {
                        return _stricmp(lhs->second.c_str(), rhs->second.c_str()) < 0; });
                }
                else {
                    std::stable_sort(v.begin(), v.end(), [](const auto* lhs, const auto* rhs) {
                        return _stricmp(lhs->second.c_str(), rhs->second.c_str()) > 0; });
                }
            }


            template <typename T>
            // where T is a pair<uid, pair<uid, value>>, for example. I will sort on the second pair element.
            // NOTE: Once sorted use my index[] function to get the natural index you should display
            // eg: a,c,b = indexes 0,1,2. Sort them. Now a is at index 0, b is at index __2__ and
            // c is at index 3. So if you are displaying the items sorted.

            // Before sorting:          index[0] = 0, index[1] = 1 and index[2] = 2
            // After sorting:           index[0] = 0, index[1] = 2 and index[2] = 1
            // After sorting descending:index[0] = 2, index[1] = 1 and index[2] = 0

            // I also provide the uid[] function in case you want the uid for position n.
            // If you use either uid[] or index[] when I am not actually sorted, the
            // behaviour is undefined. To check for this, you might want to call rowcount(),
            // whose behaviour *is* defined: it simply returns zero if we haven't been sorted.
            struct sortable_t {

                using vec_t = std::vector<T>;
                using vecptr_t = std::vector<const T*>;
                using uidvec_t =std::vector<int32_t>;
                using idxvec_t = std::vector<int32_t>;
                vecptr_t m_vec;
				sortorder_t m_sortorder{cpp::sortorder::NONE};
                sortorder_t sortorder() const { return m_sortorder; }
                void sort( const sortorder_t order, bool is_string, const vec_t& vec, int must_gather)
                {
                    if (m_vec.empty() || must_gather){
                        gather(vec);
                    }

                    if (is_string){
                        sort_string_values(m_vec, order);
                    }else{
                        sort_int_values(m_vec, order);
                    }
                    create_results();


                }



                size_t rowcount() const {return m_vec.size();}
                int32_t rowcount_s() const {return static_cast<int32_t>
                            (m_vec.size());}
                const uidvec_t& uids() const { return m_vecuid; }
                int32_t uid (const int32_t idx )const {return m_vecuid[idx];}
				// If what you have is an index, I will return the sorted index.
				int32_t idx_at(const int32_t idx) const { 
					const auto& v = m_vec;
					const auto& valptr = v.at(idx);
					const auto& pr = valptr->first;
					return pr.idx.value();
				}
                int32_t idx (const int32_t uid_ )const{return m_vecidx[uid_];}

            private:

                void create_results(){
                    m_vecuid.clear();
                    m_vecuid.resize(m_vec.size());

                    m_vecidx.clear();
                    m_vecidx.resize(m_vec.size());
                    int32_t i = 0;

                    for (const auto* pr : m_vec){

                        m_vecuid[i] = pr->first.uid.value();
                        m_vecidx[i] = pr->first.idx.value();
                        ++i;
                    }

                }

                void gather(const vec_t& values){
                    m_vec.clear();
                    m_vec.resize(values.size());
                    uint32_t i = 0;
                    for (const auto& pr : values){
                        m_vec[i] = &(pr);
                        ++i;
                    }

                }
                uidvec_t m_vecuid;
                idxvec_t m_vecidx;
            };

            struct sorter_t
            {
                int16_t sortcolindex { -1 };
                sortorder_t order{ sortorder_t::NONE };
            };
        }
    }
}

