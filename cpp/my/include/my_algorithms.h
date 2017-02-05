// my algorithms.h
#pragma once

#include <algorithm>

namespace my {
	namespace algo {
		template<class FwdIt, class Compare = std::less<>>
		void quickSort(FwdIt first, FwdIt last, Compare cmp = Compare{})
		{
			auto const N = std::distance(first, last);
			if (N <= 1) return;
			auto const pivot = std::next(first, N / 2);
			std::nth_element(first, pivot, last, cmp);
			quickSort(first, pivot, cmp);
			quickSort(pivot, last, cmp);
		}
	}
}