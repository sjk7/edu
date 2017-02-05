
// my_tests.h : stuff to help you test your code
#pragma once

#include <utility>
namespace my {
	namespace tests {
		
		template <typename T> 
		T test_move_constructor(T& t) {
			T foo(t);
			T bar(std::move(foo));
			return bar;
		}

		template <typename T>
		void test_move_assignment(const T& t) {
			T foo(t);
			T bar;
			bar = std::move(foo);
		}

		template <typename T>
		void test_copy_constructor(const T& t) {
			T foo(t);
			T bar(foo);
		}

		template <typename T>
		void test_copy_assignment(const T& t) {
			T foo(t);
			T bar;
			bar = foo;
		}
	}
}