#pragma once
#ifndef SJK_CFILESTREAM_INCLUDED
#define SJK_CFILESTREAM_INCLUDED

#ifndef _WIN32
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#endif
#include <sys/types.h>
#include <unistd.h>
#ifndef _ftelli64
#define _ftelli64 ftello
#define _fseeki64 fseeko
#endif
#endif

#include <cstdio>
#include <string>
#include <cassert>
#include "sjk_io.h"

#ifndef _WIN32
#define _fread_nolock fread
#define _fseeki64_nolock fseek
#define _fwrite_nolock fwrite
#define _fflush_nolock fflush
#define _ftelli64_nolock ftell
#endif

namespace sjk {
    // a (thin) wrapper on C's FILE*
    // In my tests on 'doze, there's nothing to tell this apart from std::fstream in terms of performance.
    class cfilestream
    {
        public:
        cfilestream() : m_f(0) {}
        cfilestream(const std::string& path, io::flags::flag_t f = io::flags::defaults) : m_f(0)
        {
            (void)f;
            const int rc = open(path, "rb+", true);
            if (rc) {
                SJK_EXCEPTION("cfilestream failed in open, with return value: ", rc);
            }
        }
        virtual ~cfilestream() { close(); }

        int open(const std::string& path, io::flags::flag_t f = io::flags::defaults) {
            (void)f;
            return open(path, "rb+", true);
        }

        void clear_errors() {
            clearerr(m_f);
        }

        int is_open() const {
            return m_f != 0;
        }
        int open(const std::string& path, std::string mode = "rb+", bool allowed_create_if_not_exist = true)
        {
            close();
            assert(!path.empty()); // I'm sure you didn't mean to try to open me with an empty path, right?
            if (path.empty()) return-1;
            if (mode.empty()) mode = "rb+";
            m_spath = path;
            m_f = fopen(path.c_str(), mode.c_str());
            if (!m_f) {
                const int e = errno;
                if (e == ENOENT) {
                    if (allowed_create_if_not_exist) {
                        std::string s("wb+");
                        m_f = fopen(path.c_str(), s.c_str());
                        assert(m_f); // file didn't exist, so we created one, but now this surely must succeed?
                        fclose(m_f); m_f = 0;
                        m_f = fopen(path.c_str(), mode.c_str());
                        assert(m_f);
                    }
                }
            }
            if (m_f) return 0;
            return -1;
        }

        size_t read(void* buf, const size_t size_bytes) {
            assert(buf); if (!m_f) return size_t(~0);
            return _fread_nolock(buf, 1, size_bytes, m_f);
        }

        int64_t read(io::span_t& into)  {
            const int64_t r = read(into.begin(), into.size_bytes());
            return r;

        }

        size_t write(void* data, size_t size_bytes) {
            assert(data);
            if (!m_f)return size_t(~0);
            // return fwrite(data, 1, size_bytes, m_f);
            return _fwrite_nolock(data, 1, size_bytes, m_f);
        }

        int64_t write(const io::span_t& sp) {
            const int64_t w = write(sp.begin(), sp.size_bytes());
            return w;
        }

        int at_end() const {
            return eof();
        }

        int errorf() {
            if (m_f) return ferror(m_f);
            return 0;
        }
        // possible values for where: SEEK_SET(beg), SEEK_CUR, SEEK_END#
        // returns where we seeked to if successful, or -1 on failure
        int64_t seek(const size_t pos, const int where = SEEK_SET) const {
            if (!m_f) return -1;

            if (_fseeki64_nolock(m_f, pos, where) == 0) {
                return static_cast<int64_t>(pos);
            }
            return -1;
        }

        int flush() const {
            if (!m_f) return -77;
            return _fflush_nolock(m_f);
            // return fflush(m_f);
        }

        void close() {
            if (m_f) {
                fclose(m_f);
                m_f = 0;

            }
        }

        int eof() const {
            if (!m_f) return -77;
            return feof(m_f);
        }

        size_t pos() const {
            if (m_f) {
                return _ftelli64_nolock(m_f);
            }
            else
            {
                return size_t(~0);
            }
        }
        size_t size_bytes() const {
            const size_t was = pos();
            _fseeki64_nolock(m_f, 0, SEEK_END);
            const size_t sz = pos();
            _fseeki64_nolock(m_f, was, SEEK_SET);
            return (size_t)sz;
        }
        const std::string& filepath() const { return m_spath; }

        protected:
        mutable FILE* m_f;
        std::string m_spath;
    };

} // namespace sjk


#endif
