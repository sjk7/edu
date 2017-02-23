#ifndef RUNTIME_ERROR_H
#define RUNTIME_ERROR_H

#ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif


#include <iostream>
#include <exception>
#include <sstream>
#include <string>
#include <assert.h>
#include <errno.h>
#include <string.h>

namespace cpp
{

    class error: public std::runtime_error
    {
		public:
			static constexpr long ERROR_UNKNOWN = -1;

			error() noexcept : std::runtime_error("Unknown error")
			{
				ss << "Unknown error";
			}

			template <typename T, typename... Args>
			error(const long errcode, const T& first,  Args&& ... args) : std::runtime_error("Unknown error")
			{
				m_code = errcode;
				stream(std::forward<const T&>(first), std::forward<Args>(args)...) ;
			}

			/*/
			template <typename T, typename... Args>
			error(const T* first,  Args&& ... args) : std::runtime_error("Unknown error")
			{
				stream(std::forward<const T*>(first), std::forward<Args>(args)...) ;
			}
			/*/
			error(const error& rhs) : error()
			{
				m_code = rhs.code();
				m_s = rhs.m_s;
			}
			error& operator=(const error& rhs)
			{
				m_s = rhs.m_s;
				m_code = rhs.m_code;
				return *this;
			}

			static std::string system_error_string(long err_num = errno)
			{
				return std::string(strerror(err_num));
			}

			virtual const char* what() const noexcept
			{
				return m_s.c_str();
			}
			operator std::string() const
			{
				return what();
			}
			inline friend std::ostream& operator<<(std::ostream& os,
												   const error& e)
			{
				os << e.what();
				return os;
			}
			int code() const
			{
				return m_code;
			}
		private:
			std::stringstream ss;
			std::string m_s;
			long m_code { -1};

			template <typename FIRST> std::ostream& stream(const FIRST& first)
			{
				auto& retval = ss << first << " ";
				m_s = ss.str();
				return retval;
			}

			template <typename T, typename... Args>
			const std::ostream& stream(const T& first, const Args& ... args)
			{
				// I recurse until there is but one argument left ...
				ss << first << " ";
				// std::cout << "args size: " <<  sizeof...(Args) << std::endl;
				return stream(args...);
			}
    };


} // namespace cpp




// call this like THROW_ERR(my_err_code, "explanation", "more explanation" ...<more explanations or variables>);
// e.g. THROW_ERR(ENOENT, "That thing isn't where I expected it to be, when you sent me", some_arg);
#ifndef THROW_ERR
#	define THROW_ERR(code, ...) \
	do { static_assert(std::is_same<decltype(code), int>::value, "THROW_ERR expects an error code as the first argument");\
		(void)sizeof(code); cpp::error erxw ( ((const long)code), __VA_ARGS__, "\n", __FILE__, ":", __LINE__); \
		std::cerr << erxw.what() << std::endl; { throw erxw; } } while(0)
// the silly variable name is because there mustn't be
// another exception in scope at call site with the same name,
// else clang complains,
// "warning: variable 'e' is uninitialized when used within its own initialization [-Wuninitialized]"

#endif


#ifndef WARN
#	define WARN( ...)\
	do {  cpp::error e ( (-1), (" WARNING: "), __VA_ARGS__, "\n", __FILE__, ":", __LINE__); \
		std::cerr << e.what() << std::endl; } while(0)
#endif

// 					THROW_ASSERT(row < m_values.size(), "row: value_set(): row", row, "out of bounds.",
//					"values' size is: ", m_values.size())

#ifndef THROW_ASSERT
#	define THROW_ASSERT(expr, code, ...)  \
	do \
	{ \
		static_assert(std::is_same<decltype(code), int>::value, "THROW_ASSERT expects an error code as the first argument");\
		if (!(expr)) \
		{ \
			cpp::error e ( ((const long)code), __VA_ARGS__, "\n", __FILE__, ":", __LINE__); \
			std::cerr << e.what() << std::endl; \
			throw e; \
		} \
	} while(0)
#endif

namespace detail
{
	namespace demo
	{
		// we should be able to call these fellas in the following way:
		static inline void demonstrate_error_macros()
		{
			int x = 77;
			WARN("Just a single text message");
			WARN("first", "second", "third", "forever ...");
			WARN("You did something stupid, because x is", x, " Now go away and try again");
			WARN(-819, "Some number (-819) [", -819, "] first. Does it matter?");

			try {
				THROW_ERR(-1, "Found and error and chucking:", x, "... for your entertainment");
			} catch (cpp::error& e) {
				std::cerr << "\nCaught the error I threw. Here's the details...\n";
				std::cerr << e.what() << std::endl;
				assert(e.code() == -1);
			}

			try {
				THROW_ERR(-1000, "Found some other fake error and chucking:",
						  1234, "... for your entertainment.",
						  "\n More error info on a new line: Hello world.");
			} catch (cpp::error& e) {
				std::cerr << "\nCaught the second error I threw. Here's the details...\n";
				std::cerr << e.what() << std::endl;
				assert(e.code() == -1000);
			}
		}
	}
}



#endif // RUNTIME_ERROR_H
