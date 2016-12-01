#pragma once
#ifndef SJK_IO_H
#define SJK_IO_H

#include <cstdint>
#include "sjk_memory.h"
#include <ios>

#ifndef NOERROR
#define NOERROR 0
#endif

namespace sjk
{
	namespace io
	{
		static constexpr size_t SIZE_INVALID = static_cast<size_t>(~0);
		typedef sjk::span<sjk::byte> span_t;

		namespace flags
		{

            using flag_t = std::ios_base::openmode;
            static const flag_t defaults = std::ios::binary | std::ios::in;
		}
		struct device
		{
            // return 0 (NOERROR) if it went well.
			virtual int open(const std::string& path, flags::flag_t flags = flags::defaults) = 0;
			virtual int is_open() const = 0;
			virtual int close() = 0;
			virtual int64_t read(span_t& into)  = 0;
			virtual int64_t write(span_t& from) = 0;
			virtual bool at_end() const = 0;
			virtual size_t size_bytes() const = 0;
			virtual int64_t seek(int64_t pos, std::ios::seekdir whence = std::ios::beg) = 0;
			virtual int64_t position() const = 0;
			virtual void clear_errors() = 0;
			virtual std::string path() = 0;
			virtual ~device(){}
		};


	} // namespace io

} // namespace sjk

#endif // SJK_IO_H
