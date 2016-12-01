#pragma once
#ifndef SJK_EXCEPTION_H
#define SJK_EXCEPTION_H

#ifdef _WIN32
#include "sjk_windows.h"
#endif
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cassert>
#include <utility>
#include <string>
#include <cerrno>




#ifdef _MSC_VER
#define PRETTY_FUNC __FUNCSIG__
#else
#define PRETTY_FUNC __PRETTY_FUNCTION__
#endif

namespace sjk {
	class exception : public std::runtime_error {
		std::string msg;
		std::ostringstream* m_pos;
		const char* m_pfile;
		const char* m_pfunc;
		int m_line;

		static std::string get_last_error_as_string(int err = 0) {
			if (err == 0) {
				err = errno;
			}
			if (err == 0) {
				return "errno is zero: no error.";
			}

			return strerror(err);
		}

#ifdef _WIN32
		static std::string _get_last_error_as_string(DWORD dwerror = 0)
		{
			//Get the error message, if any.
			if (dwerror == 0) {
				dwerror = ::GetLastError();
			}


			if (dwerror == 0) {
				return get_last_error_as_string();
			}

			LPSTR messageBuffer = nullptr;
			size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dwerror, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

			std::string message(messageBuffer, size);

			//Free the buffer.
			LocalFree(messageBuffer);

			return message;
		}
#else
		static std::string _get_last_error_as_string(int err = 0) { return get_last_error_as_string(err); }

#endif

	public:
		static std::string error_string(int err = errno)  {
#ifdef _WIN32
			return _get_last_error_as_string((DWORD)err);
#else
			return _get_last_error_as_string(err);
#endif
		}
		// exception() : std::runtime_error("error"), m_pos(nullptr), m_pfile(nullptr), m_line(0){}
		void log(){
			assert(m_pos);
			if (m_pfile) {
				assert(m_pos && m_pfile && m_line && m_pfunc);
				std::string sfile(m_pfile);
				std::string::size_type pos = sfile.find_last_of("/");
				if (pos != std::string::npos && pos + 1 < sfile.length()) {
					sfile = sfile.substr(pos + 1);
				}
				*m_pos << std::endl << "In file: " << sfile << std::endl;
			}
			if (m_line > 0) {
				*m_pos << "line: " << m_line << std::endl;
			}
			if (m_pfunc) {
				*m_pos << "function: " << m_pfunc << "()" << std::endl << std::endl;
			}
			msg = m_pos->str();
			std::cerr << msg << std::endl;
		}


#ifdef _MSC_VER
#pragma warning(disable: 26485)
#endif
		template<typename First, typename ...Rest>
		void log(First && first, Rest && ...rest)
		{
			assert(m_pos);
			*m_pos << std::forward<First>(first) << " ";
			log(std::forward<Rest>(rest)...);
		}

		template<typename First, typename ...Rest>
		exception(const char* func, const char *file, int line, First && first, Rest && ...rest) :
			std::runtime_error("error")
		{
			static std::ostringstream os;
			m_pos = &os; m_pfile = file; m_line = line; m_pfunc = func;
			log(std::forward<First>(first),std::forward<Rest>(rest)...);
		}

		template<typename First, typename ...Rest>
		exception(double ,  First && first, Rest && ...rest) :
			std::runtime_error("error")
		{
			static std::ostringstream os;
			m_pos = &os; m_line = -1; m_pfile = nullptr; m_pfunc = nullptr;
			log(std::forward<First>(first), std::forward<Rest>(rest)...);
		}


		virtual ~exception(){}
		const char *what() const noexcept {
			return msg.c_str();
		}
	};
} // namespace sjk
//#define FOO(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define SJK_EXCEPTION(...) throw sjk::exception(PRETTY_FUNC,__FILE__, __LINE__, __VA_ARGS__);
#define SJK_EXCEPT(...) throw sjk::exception(3.14, __VA_ARGS__);

#endif
