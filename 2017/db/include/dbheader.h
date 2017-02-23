// dbheader.h. Steve. Sat. 18th Feb. 2017. No restrictions. Hope it helps you!

#ifndef DBHEADER_H
#define DBHEADER_H

#include "macros.h" // NO_COPY
#include "dbcolumns.h"

#include <fstream>
#include <ctime>
#include <string>

namespace cpp
{
	namespace db
	{
		struct core;
		struct rows_t;

		enum class open_flags { defaults, create, read, readwrite };
		// header contains persistent data relating to the number of columns and their "types"
		// NOTE: that "types" are used for collation only (ie how the data is interpreted for sorts,
		// for example) but as far as we are concerned, all data is a string.
		struct header {

				NO_COPY(header);
				friend struct core;
				static constexpr const auto npos = static_cast<size_t>(-1);
				friend struct rows_t;
				static constexpr int16_t VERSION()
				{
					return 1;
				}
				static constexpr uint64_t MAGIC()
				{
					return 199259319;
				}

				header(std::fstream& f, open_flags flags = open_flags::defaults) : m_f(f), m_flags(flags)
				{
					std::ios_base::sync_with_stdio(false);
					m_flags = flags; // gets rid of clang "unused" warning.

					if (flags == open_flags::defaults) {
						flags = open_flags::readwrite;
					}

					if (flags == open_flags::create) {
						// user is creating or overwriting existing, don't care about header reading.
						create_default_cols();
						return;
					}

					read();

				}



				columns_t& columns()
				{
					return m_cols;
				}
				const columns_t& columns_const() const
				{
					return m_cols;
				}
				int dirty() const
				{
					return m_dirty;
				}
				std::ios_base::streamoff first_row_position() const
				{
					THROW_ASSERT(m_data_start_pos > 0, -1, "first_row_position: there must be data start position");
					return m_data_start_pos;
				}

			protected:
				mutable std::ios_base::streamoff m_data_start_pos{0};

				void read()  {
					m_f.exceptions(std::ios_base::goodbit); // don't throw exceptions here.
					m_f.seekg(0);
					std::string s;
					s.resize(1024);
					m_f.read(&s[0], 1024);
					const auto read = m_f.gcount();

					if (read == 0) {
						THROW_ERR(-EEXIST, "Could not read from file. Zero-byte file suspected.");
					}

					auto trimpos = s.find_first_of('\0');

					if (trimpos == npos) trimpos = s.length();

					const std::string trimmed_data = s.substr(0, trimpos);
					parse(trimmed_data);
				}

				void parse(const std::string& data)
				{
					using namespace cpp::strings;
					if (data.empty()) {
						THROW_ERR(-2, "header::parse(): ", "no data to parse.");
					}

					if (data.find(TAB) == npos || data.find(PIPE) == npos) {
						THROW_ERR(-2, "header::parse(): " , "invalid header data (no delims)");
					}

					if (data.find(DNLS) == npos) {
						THROW_ERR(-2, "Parse error. no dnl");
					}

					using cpp::strings::strvec_t;
					strvec_t v;
					size_t found_dnl_pos = cpp::strings::split(data, DNLS, v);
					m_data_start_pos = found_dnl_pos + DNLS.size();
					auto& trimmed_data = data; // trimmed before we were called

					if (trimmed_data.find("\n") == npos) {
						THROW_ERR(-2, "UNEXPECTED:", "trimmed data has no nl.");
					}

					strvec_t mine = cpp::strings::split(trimmed_data, NLS);

					if (mine.size() < NUM_DEFAULT_COLUMNS) {
						THROW_ERR(-2, "UNEXPECTED:", "incorrect column count.");
					}

					auto& magic = mine[0];

					if (!strings::is_numeric(magic)) {
						THROW_ERR(-2, "Magic in file is not numeric: ", magic);
					}

					auto imagic = std::stoull(magic);

					if (imagic != MAGIC()) {
						THROW_ERR(-2, "UNEXPECTED:", "Bad magic number. Wanted:",
								  MAGIC(), ".Got: ", imagic, "(", magic, ")");
					}

					auto& ver = mine[1];

					if (!strings::is_numeric(ver)) {
						THROW_ERR(-2, "Version in file is not numeric: ", ver);
					}

					auto iver = std::stoi(ver);
					const auto right_ver = VERSION();
					THROW_ASSERT(iver == right_ver , -2, "UNEXPECTED:", "Bad version. Wanted:",
									right_ver, ".Got: ", iver, "(", ver, ")");
	

					try {
						m_cols.from_string(mine[2]);
					} catch (const cpp::error& e) {
						THROW_ERR(-2, "Fatal error parsing columns: " , e.what() , "(original error code)", e.code());
					} catch (const std::exception& e) {
						THROW_ERR(-2, "Fatal exception parsing columns: " , e.what());
					}


				}

				void create_default_cols()
				{
					m_cols.add("uid", column_types::TYPE::INT);
					m_cols.add("created", column_types::TYPE::DATE);
					m_cols.add("modified", column_types::TYPE::DATE);
					save(true);
				}

				void seek_to_row(const rowidx_t& row)
				{
					const auto& cols = m_cols;
					m_f.exceptions(std::ios_base::goodbit);
					m_f.clear();
					THROW_ASSERT(cols.size(), -1, "Cannot seek to row if there are no columns.");
					int64_t record_size = cols.total_data_width();
					auto seekpos = first_row_position() + (record_size * row.value());
					m_f.seekp(seekpos);
					if (!m_f){
						if (m_f.eof()){
							m_f.clear();
						}else{
							THROW_ASSERT(!m_f.bad(), -1, "seek_to_row: stream bad.");
						}
					}
				}

				bool save(bool force = false)
				{
					using namespace cpp::strings;
					if (!force) {
						if (!dirty()) return false;
					}

					try {
						m_f.exceptions(std::ios_base::goodbit);
						m_f.clear();
						m_f.seekp(0);
						const auto s = m_cols.serialize();
						auto sm = std::to_string(MAGIC());
						sm += NL;
						sm += std::to_string(VERSION());
						sm += NL;
						m_f.write(&sm[0], sm.size());
						m_f.write(&s[0], s.size());
						std::string the_end;
						the_end += NL;
						the_end += NL; // \n\n to terminate the header
						m_f.write(&the_end[0], the_end.size());

						if (!m_f) {
							THROW_ERR(-1, "could not write header stream");
						}

						m_data_start_pos = sm.size() + s.size() + the_end.size();
						m_f.flush();
						m_dirty = false;
					} catch (const cpp::error& e) {
						THROW_ERR(e.code(), "Fatal error saving columns: ", e.what());
					} catch (const std::exception& e) {
						THROW_ERR(-1, "Fatal error saving columns: ", e.what());
					}

					return true;
				}


			private:
				std::fstream& m_f;
				open_flags m_flags;
				columns_t m_cols;


				int m_dirty {0};
		}; // struct header



	}
}
#endif // DBHEADER_H
