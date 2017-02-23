#ifndef COLLECTION_H
#define COLLECTION_H
#include <vector>
#include <map>
#include <algorithm> // remove_if
#include "runtime_error.h"
#include "macros.h"

namespace cpp
{
	namespace collections
    {

		template<class Container, class F>
		auto erase_where(Container& c, F&& f)
		{
			return c.erase(std::remove_if(c.begin(),
										  c.end(),
										  std::forward<F>(f)),
						   c.end());
		}


		template <typename K, typename V, typename VV = V*> // K = KEY, V = VALUETYPE (in map), VV = VALUETYPE (in vec)
		/*!
		 * \brief The collection class : a container that can be accessed by key or
		 * by index. Moreover, the order of the values is completely independent
		 * of the always-sorted-by-key order properties of a map.
		 */
		class collection
		{
			public:
				using map_t			=		std::map<K, V>;
				using vec_t			=		std::vector<VV>;
				using addpr_t		=		std::pair<typename map_t::iterator, bool>;
				using const_iter	=		typename vec_t::const_iterator;
				using iter			=		typename vec_t::iterator;
				using size_type		=		typename vec_t::size_type;

				/*!
				 * \brief collection constructor
				 * \param reserve_how_many: How many rows in memory to reserve. Just like in a vector.
				 */
				collection(const size_t reserve_how_many = 1000)
				{
					reserve(reserve_how_many);
				}

#ifndef COLLECTION_COPIABLE
				NO_COPY(collection);
#endif

				// add a pair of <K,V> to me.
				// I might throw, if an item with that key existed already
				addpr_t add(const K& key, const V& value)
				{
					auto ret = m_map.emplace(std::make_pair(key, value));

					if (ret.second) {
						V& rv = m_map[key];
						m_vec.push_back(&rv);
					} else {
						THROW_ASSERT(0, -1, "collection::add() item with key ", key, " exists.");
					}

					return ret;
				}

				bool empty() const{
					return m_vec.empty();
				}

				// returns iterator and bool in a pair, the same as add() does.
				auto remove(const K& key)
				{
					auto it = m_map.find(key);
					if (it == m_map.end()){
						THROW_ASSERT("collection::remove() item with key", key, "does not exist");
					}

					auto ptr = &m_map[key];
					erase_where(m_vec, [&](const auto& thing){return &thing == &ptr;});
					auto ret = m_map.erase(key);

					return ret;
				}

				// get a signed version of size()
				int64_t size_s() const
				{
					return static_cast<int64_t>(size()) ;
				}
				size_type size() const
				{
					return m_vec.size();
				}

				/*!
				 * \brief operator [] const
				 * \param k: the key you are looking for
				 * \return a _reference_ to the actual item in the collection,
				 * but NOTE: throws cpp::error if the key is not found in the collection.
				 */
				const V& operator[] (const K& k) const
				{
					auto it = m_map.find(k);

					THROW_ASSERT(it != m_map.end(), -EINVAL, "collection: key", k, "does not exist in the collection.");
					return it->second;
				}

				/*!
				 * \brief operator [] non-const
				 * \param k: the key you are looking for
				 * \return a _reference_ to the actual item in the collection,
				 * but NOTE: throws cpp::error if the key is not found in the collection.
				 */
				V& operator[] (const K& k)
				{
					auto it = m_map.find(k);

					if (it == m_map.end()) {
						THROW_ASSERT(0, -1, "collection: key", k , "does not exist in the collection.");
					}

					return it->second;
				}

				const V& operator [] (const size_t idx) const
				{

					return *m_vec[idx];
				}

				V& operator [] (const size_t idx)
				{

					return *m_vec[idx];
				}

				// I'm sure you know what you are doing,
				// but using this for anything other than a range-based
				// for loop is going to end in tears.
				vec_t& vector(){ return m_vec;}
				const vec_t& vector_const() const { return m_vec; }

				const_iter cbegin() const
				{
					return m_vec.cbegin();
				}
				const_iter cend() const
				{
					return m_vec.cend();
				}
				iter begin()
				{
					return m_vec.begin();
				}
				iter end()
				{
					return m_vec.end();
				}
				void clear()
				{
					m_vec.clear();
					m_map.clear();
				}
				/*!
				 * \brief resize: just like resize()ing a vector.
				 * \param newsize
				 */
				void resize(const size_t newsize)
				{
					m_vec.resize(newsize);
				}

				/*!
				 * \brief reserve: just like reserve()ing a vector.
				 * \param sz
				 */
				void reserve(const size_t sz)
				{
					m_vec.reserve(sz);
				}

			protected:
				map_t m_map;
				vec_t m_vec;
		}; // class collection
	} // namespace collections
} // namespace cpp

#endif // COLLECTION_H
