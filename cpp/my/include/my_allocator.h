#pragma once
// my_allocator

#include <stdlib.h>
#include "my_storage.h"
#include <map>
#include <cstdint>

namespace my
{
	namespace mem
	{
		template <typename T>
		struct alloc
		{
		private:
			using map_t = std::map<intptr_t, size_t>;
			using pair_t = std::pair<intptr_t, size_t>;

			map_t m_map_used;
			map_t m_map_freed;
			
			T* find_freed_section(const size_t how_many_Ts) {
				for (const auto& pr : m_map_freed)
				{
					if (pr.second >= how_many_Ts) {
						return (T*)pr.first;
					}
				}
				return nullptr;
			}
		public:
			alloc(size_t max_size = my::storage::MEGABYTE * 500) : m_d(max_size) { create(); }
			size_t size() const { return m_d.end - m_d.begin; }
			size_t size_in_bytes() const { return size() * sizeof(T); }
			
			T* next(const size_t how_many_Ts = 1) {
				T* p = m_d.ptr;
				if (m_d.ptr + how_many_Ts >= m_d.end) {
					if (how_many_Ts >= size()) {
						throw std::bad_alloc();
					}
					p = find_freed_section(how_many_Ts);
					if (!p) {
						throw std::bad_alloc();
					}
				}
				m_map_used.insert(pair_t( (intptr_t)p, how_many_Ts) );
				m_map_freed.erase((intptr_t)p);
				m_d.ptr += how_many_Ts;
				return p;
			}

			void release(T* p, const size_t how_many_Ts = 1) {
				const auto it = m_map_used.find((intptr_t)p);
				if (it == m_map_used.end()) {
					throw std::out_of_range("bad pointer"); // you are trying to free a pointer that nobody was handed out!
				}
				const auto& pr = *it;
				if (pr.second != how_many_Ts) {
					ASSERT(pr.second == how_many_Ts && "Invalid size sent to release");
				}
				m_map_freed.insert(pr);
				m_map_used.erase((intptr_t)p);
			}
			const T* const end() { return m_d.end; }
			T* begin() { return m_d.begin; }


		private:
			
			struct d {
				d(size_t max_sz) : ptr(nullptr), begin(nullptr), end(nullptr), max_size(max_sz){
				}
				~d() {
					// kill();
				}
				void kill() {
					if (ptr) {
						delete[] ptr;
					}
					ptr = nullptr; begin = nullptr; end = nullptr;
				}
				T* create() {
					kill();
					size_t sz = max_size;
					while (!begin && sz) {
						if (sz > max_size) {
							throw std::bad_alloc();
							break;
						}
						try {
							begin = make(sz);
						}
						catch (...) { sz /=2; }
					}
					end = begin + sz;
					ptr = begin;
					return begin;
				}

				T* make(const size_t sz) {
					return new T [sz];
				}
				T* ptr;
				T* begin;
				T* end;
				size_t max_size;
			};
			d m_d;


			T* create() {
				T* ret = m_d.create();
				return ret;
			}

		};
	}
}