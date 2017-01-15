// myarray.h

#pragma once
#include "my_memory.h"

namespace my {
	namespace memory
	{
		template <typename T, int SBO = 64, typename SZ = size_type>
		class array
		{
		private:
			sbo_t<T, SBO> z_sbo;
			range<T> z_ptrs;
			
		public:
				static constexpr SZ TSIZE = sizeof(T);
				static constexpr SZ MAX_SBO_DATA = TSIZE - 2;
				explicit array() : z_ptrs(z_sbo.begin(), z_sbo.end()) {}
				~array() {}
				
				SZ size() const { return z_size(); }
				SZ push_back(const T& t) { return z_sbo.append(t); }
				bool empty() const { return z_size() == 0; }
				void clear() { return z_clear(); }

		private:

				SZ z_size() const { return z_ptrs.size(); }
				void z_clear() {}
		};
	
	} // namespace memory
} // namespace my
