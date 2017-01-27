#pragma once
#ifndef SJK_WINFILE_H
#define SJK_WINFILE_h

#define NOMINMAX
#ifndef STRICT
#define STRICT
#endif

#ifdef WANT_WINDOWS_H
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#endif

#include "sjk_io.h"
#include "sjk_exception.h"
// NOTE: In my tests, on an SSD, this is actually slower (about a third) than using C's file API!!
// You should always profile your code, so use this if you want, but I found it slow.

namespace sjk
{
	namespace windows
	{
		using namespace sjk::io;
		class file : public sjk::io::device
		{
		private:
			HANDLE m_hfile;
			std::string m_spath;
			mutable int64_t m_size;

		public:
			file() : m_hfile(INVALID_HANDLE_VALUE), m_size(-1) {}
			file(const std::string& path, io::flags::flag_t f = io::flags::defaults) : m_hfile(INVALID_HANDLE_VALUE), m_size(-1)
			{
				if (!path.empty()) {
					open(path);
				}
			}
			file(const file& other) = delete;
			file& operator=(const file& other) = delete;
			

			virtual ~file() 
			{
				close();
			}

			static __forceinline LONGLONG GetFilePointerEx(HANDLE hFile) {
				LARGE_INTEGER liOfs = { 0 };
				LARGE_INTEGER liNew = { 0 };
				SetFilePointerEx(hFile, liOfs, &liNew, FILE_CURRENT);
				return liNew.QuadPart;
			}
			static inline LONGLONG GetFileSizeExWindows(const HANDLE h, const std::string& path) {
				LARGE_INTEGER li{ 0 };
				assert(!path.empty());
				assert(h != INVALID_HANDLE_VALUE && h);
				BOOL b = ::GetFileSizeEx(h, &li);
				assert(b);
				if (!b) {
					SJK_EXCEPTION("Failed to get file size: ", path, " GetLastError says: ", GetLastError());
				}
				return li.QuadPart;
			}

			size_t size() const {
				if (m_size < 0) {
					m_size = GetFileSizeExWindows(m_hfile, m_spath);
				}
				return m_size;
			}
			// NOT a ref to allow re-opening.
			std::string path() const {
				return m_spath;
			}

			// Inherited via device
			virtual int open(const std::string& path, flags::flag_t flags = flags::defaults) override
			{
				close();
				if (path.empty()) {
					SJK_EXCEPTION("Attempted to open a winfile when path was empty. Not allowed."
						 " If you mean close(), say close()!");
				}
				m_spath = path;
				m_hfile = ::CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_NEW, 0, 0);
				if (m_hfile == INVALID_HANDLE_VALUE) {
					const DWORD dw = GetLastError();
					assert(dw == ERROR_FILE_EXISTS);
					if (dw == ERROR_FILE_EXISTS) {
						m_hfile = ::CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
					}
				}
				if (!m_hfile) {
					SJK_EXCEPTION("Unable to open file: " , path, " GetLastError says: ", GetLastError());
				}
				if (m_hfile == INVALID_HANDLE_VALUE) {
					if (GetLastError()) {
						return (int)GetLastError();
					}
					return -1;
				}
				else {
					return NOERROR;
				}
			}
			virtual int is_open() const override {
				return m_hfile != INVALID_HANDLE_VALUE;
			}
			virtual int close() override {
				if (m_hfile != INVALID_HANDLE_VALUE) {
					::CloseHandle(m_hfile);
				}
				m_hfile = INVALID_HANDLE_VALUE;
				m_spath.clear();
				m_size = -1;
				return 0;
			}
			virtual int64_t read(span_t & into) override {
				/*/
				BOOL WINAPI ReadFile(
					_In_        HANDLE       hFile,
					_Out_       LPVOID       lpBuffer,
					_In_        DWORD        nNumberOfBytesToRead,
					_Out_opt_   LPDWORD      lpNumberOfBytesRead,
					_Inout_opt_ LPOVERLAPPED lpOverlapped
				);
				/*/
				DWORD read = 0;
				BOOL b = ::ReadFile(m_hfile, into.data(), into.size_bytes_d(), &read, NULL);
				if (read == into.size_bytes() && b != 0) {
					return static_cast<int64_t>(read);
				}
				const DWORD dw = GetLastError();
				assert(dw != ERROR_IO_PENDING); // apparently, this is not an error.
				SJK_EXCEPTION("Failed to read file: ", m_spath, " GetLastError says: ", GetLastError());
				
				return 0;
			}
			virtual int64_t write(span_t & from) override {
				DWORD wrote = 0;
				BOOL b = ::WriteFile(m_hfile, from.begin(), from.size_bytes_d(), &wrote, NULL);
				if (wrote != from.size_bytes()) {
					const DWORD dw = GetLastError();
					assert(dw != ERROR_IO_PENDING); // apparently, this is not an error.
					SJK_EXCEPTION("Failed to write to file: ", m_spath, " GetLastError says: ", GetLastError());
				}
				
				if (m_size != -1) {
					if (static_cast<int64_t>(from.size_bytes()) + GetFilePointerEx(m_hfile) > m_size) {
						m_size = -1;
					}
				}
				return static_cast<int64_t>(wrote);
			}

			virtual bool at_end() const override {
				if (m_size < 0) {
					m_size = GetFileSizeExWindows(m_hfile, m_spath);
				}
				return GetFilePointerEx(m_hfile) == m_size;
			}
			virtual size_t size_bytes() const override
			{
				return size();
			}
			virtual int64_t seek(int64_t pos, std::ios::seekdir whence = std::ios::beg) override {
				LARGE_INTEGER to{0};
				to.QuadPart = pos;
				LARGE_INTEGER actual{0};
				DWORD method = FILE_BEGIN;
				if (whence == SEEK_END) {
					method = FILE_END;
				}
				else if (whence == SEEK_CUR) {
					method = FILE_CURRENT;
				}
				BOOL b = ::SetFilePointerEx(m_hfile, to, &actual, FILE_BEGIN);
				if (actual.QuadPart != to.QuadPart || !b)
				{
					SJK_EXCEPTION("Failed to seek in file: ", m_spath, 
						" to position: " , pos, ", where whence is: ", whence ," and size is: ",
						size_bytes(), " GetLastError says: ", GetLastError());
				}
				return pos;
			}
			virtual int64_t position() const override
			{
				return static_cast<int64_t>(GetFilePointerEx(m_hfile));
			}
			virtual void clear_errors() override {
				return;
			}
		};
	}
}



#endif //#ifndef SJK_WINFILE_H