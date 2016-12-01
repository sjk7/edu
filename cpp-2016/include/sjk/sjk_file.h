#pragma once

#ifndef SJK_FILE_H
#define SJK_FILE_H

#include "sjk_io.h"
#include "sjk_exception.h"
#include <fstream>
#include "sjk_cfilestream.h"
#include <experimental/filesystem>

namespace sjk
{
    struct file : public sjk::io::device
    {
        static inline std::string tempfile(const std::string& name){
            std::string ret;
            namespace fs = std::experimental::filesystem;
            fs::path dir = fs::temp_directory_path();
            fs::path file (name);
            fs::path full_path = dir / file;
            ret = full_path.u8string();

            return ret;
        }

        virtual std::string path() override {
            return m_spath;
        }

        static inline int delete_file(const std::string& path){
            if (path.empty() || path.length() <= 3){
                SJK_EXCEPTION("file path ", path, " too short to delete_file()");
            }
            return remove(path.c_str());
        }
        static inline bool exists(const std::string& path){
            namespace fs = std::experimental::filesystem;
            return fs::exists(path);
        }

        static constexpr auto flags_default = std::ios::binary | std::ios::in | std::ios::out;
        static constexpr auto flags_default_create = std::ios::binary | std::ios::out;
        file(const std::string& filepath, io::flags::flag_t flags = flags_default)
        {
            open(filepath, flags);
        }
        file(){}
        virtual ~file(){close();}

        file(const file& other) = delete;
        file& operator =(const file& other) = delete;

        // device interface
        public:
        virtual int open(const std::string& path,
                         io::flags::flag_t flags = flags_default) override{
            std::string s = path;
            close();
            if (!path.empty()){
                m_f.open(path.c_str(), flags);
                if (!m_f){
                    if (errno){
                        SJK_EXCEPTION("Failed to open file: ", path, "\nError:", errno, strerror(errno));
                    }else{
                        SJK_EXCEPTION("Failed to open file: ", path);
                    }
                }
            }
            return 0;
        }

        operator bool() const {
            return bool(m_f);
        }

        virtual int close() override{
            m_f.close();
            m_f.clear();
            return 0;
        }

        virtual int64_t read(io::span_t& into)  override{
            m_f.read(into.begin(), into.size_bytes());
            return static_cast<int64_t>(m_f.gcount());

        }
        virtual int64_t write(io::span_t& from) override{

            std::streamsize cb = from.size_bytes();
            m_f.write(from.begin(), cb);
            if (!m_f){
                return -1;
            }
            return cb;
        }
        virtual bool at_end() const override{
            return m_f.eof();
        }
        virtual void clear_errors() override{
            m_f.clear();
        }
        virtual size_t size_bytes() const override{
            m_f.clear();
            auto was = m_f.tellg();
            m_f.seekg(0, std::ios::beg);
            m_f.seekg(0, std::ios::end);
            std::streamoff ret = m_f.tellg();
            m_f.seekg(was, std::ios::beg);
            m_f.clear();
            return static_cast<size_t>(ret);
        }

        virtual int64_t seek(int64_t pos, std::ios::seekdir whence = std::ios::beg) override {
            m_f.clear();
            m_f.seekg(pos, whence);
            m_f.seekp(pos, whence);
            if (!m_f){ // seeking to the end of the file is fair enough:
                if (m_f.eof()){
                    m_f.clear();
                    return pos;
                }
                return -1;
            }
            return pos;
        }

        virtual int64_t position() const override {
            return m_f.tellg();
        }
        virtual int is_open() const override{ return m_f.is_open();}

        const std::string& path() const{ return m_spath;}
        private:
        mutable std::fstream m_f;
        std::string m_spath;

    };

    struct cfile : public sjk::cfilestream, public sjk::io::device
    {

        cfile(const cfile& other) = delete;
        cfile& operator =(const cfile& other) = delete;
        cfile() : m_bopen(false) {}
        cfile(const std::string& path, sjk::io::flags::flag_t flags = io::flags::defaults)
            : m_bopen(false)
        {
            if (!path.empty()) {
                int x = open(path, flags);
                if (x) {
                    SJK_EXCEPTION("failed to open c file");
                }
            }
        }

        virtual std::string path() override {
            return m_spath;
        }


        // Inherited via device
        virtual int open(const std::string & path, sjk::io::flags::flag_t flags = io::flags::defaults) override
        {
            std::string mode_string = "rb+";
            if ((flags & std::ios_base::out)){

            }
            int o = sjk::cfilestream::open(path, "rb+", true);
            if (o == 0) {
                m_bopen = true;
            }
            else {
                m_bopen = false;
            }
            return o;
        }
        virtual int is_open() const override { return m_bopen; }
        virtual int close() override {
            sjk::cfilestream::close();
            m_bopen = false;
            return 0;
        }
        virtual int64_t read(io::span_t & into) override {
            return sjk::cfilestream::read(into.data(), into.size_bytes());
        }
        virtual int64_t write(io::span_t & from) override {
            return cfilestream::write(from.begin(), from.size_bytes());
        }
        virtual bool at_end() const override {
#ifdef _MSC_VER
#pragma warning(disable: 4800)
#endif
            return cfilestream::eof();
        }
        virtual int64_t seek(int64_t pos, std::ios::seekdir whence = std::ios::beg) override {
            return cfilestream::seek(pos, whence);
        }
        virtual int64_t position() const override {
            return cfilestream::pos();
        }
        virtual void clear_errors() override {

            return;
        }

        virtual size_t size_bytes() const override {
            return cfilestream::size_bytes();
        }
        protected:
        bool m_bopen;
    };

}

#endif // SJK_FILE_H
