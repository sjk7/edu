
#pragma once
#ifndef SJK_SYSFILE_H
#define SJK_SYSFILE_H

#include "sjk_io.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>

namespace sjk {
	using namespace sjk::io;
	// The lowest-level file io possible (unbuffered)
	class sysfile : public sjk::io::device
	{
		int m_f;
		mutable int64_t m_size;
		std::string m_spath;
	public:

		sysfile(const std::string& path, flags::flag_t f = flags::defaults) : m_f(0), m_size(-1)
		{
			open(path, f);
		}
		virtual ~sysfile() {}

		sysfile(const sysfile& other) = delete;
		sysfile& operator=(const sysfile& other) = delete;

		std::string path() const {
			return m_spath;
		}

		// Inherited via device
		virtual int open(const std::string & path, flags::flag_t flags = flags::defaults) override {
			if (m_f) close();
			if (path.empty()) return -1;
			m_f = _creat(path.c_str(), _S_IREAD | _S_IWRITE);
			m_spath = path;
			m_size = -1;
			return m_f != 0;
		}

		virtual int is_open() const override {
			return m_f != 0;
		}

		virtual int close() override {
			if (m_f) {
				_close(m_f);
				m_f = 0;
			}
			return 0;
		}

		virtual int64_t read(span_t & into) override {
			return _read(m_f, into.data(), static_cast<unsigned int>(into.size_bytes()));
		}

		virtual int64_t write(span_t & from) override {
			int64_t ret = _write(m_f, from.begin(), static_cast<unsigned int>(from.size_bytes()));
			if (m_size != -1) {
				if (pos() + ret > mysize()) {
					m_size = -1;
				}
			}
			return ret;
		}

		int64_t mysize() const {
			if (m_size < 0) {
				m_size = _filelengthi64(m_f);
			}
			return m_size;
		}
		int myeof() const {
			return pos() >= mysize();
		}
		virtual bool at_end() const override {
			return myeof() != 0;
		}

		virtual size_t size_bytes() const override
		{
			return static_cast<size_t>(mysize());
		}

		virtual int64_t seek(int64_t pos, std::ios::seekdir whence = std::ios::beg) override
		{
			return _lseeki64(m_f, pos, whence);
		}

		int64_t pos() const {
			return _telli64(m_f);
		}

		virtual int64_t position() const override {
			return pos();
		}

		virtual void clear_errors() override {
			return;
		}

	};// class sysfile

} // namespace sjk


#endif // SJK_SYSFILE_H
