
// my_tests.h : stuff to help you test your code
#pragma once

#include <utility>
namespace my {
	namespace tests {
		
		template <typename T> 
		void test_move_constructor(const T&) {
			T foo;
			T bar(std::move(foo));
		}

		template <typename T>
		void test_move_assignment(const T&) {
			T foo;
			T bar;
			bar = std::move(foo);
		}

		template <typename T>
		void test_copy_constructor(const T&) {
			T foo;
			T bar(foo);
		}

		template <typename T>
		void test_copy_assignment(const T&) {
			T foo;
			T bar;
			bar = foo;
		}
	}
}