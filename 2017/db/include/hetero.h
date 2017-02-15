// hetero.h
#pragma once
#include <vector>

namespace sjk
{
	namespace hetero
	{
		template <typename T>
		struct het_base 
		{
			using vec_t = std::vector<T>;
			T m_t;
		};



		
	}
}