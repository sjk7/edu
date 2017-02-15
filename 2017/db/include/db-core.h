#pragma once
// db-core.h
#include <fstream>
#include <iostream>
#include <cstdint>
#include <ctime>
#include <vector>
#include <cassert>
#include <map>
#include <string>
#include <sstream>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "hetero.h"

namespace sjk
{
	namespace dbcore
	{
		using strvec_t = std::vector<std::string>;

		template <typename T = const char*>
		strvec_t split(const std::string& s, const std::string& delim)
		{
			strvec_t ret;
			split(s, delim, ret);
			return ret;

		}
		size_t split(const std::string& str, const std::string& delim, strvec_t& v)
		{
			v.clear();
			size_t prev = 0, pos = 0; size_t ret = std::string::npos;
			
			do
			{
				pos = str.find(delim, prev);
				if (ret == std::string::npos) ret = pos;
				if (pos == std::string::npos) pos = str.length();
				std::string token = str.substr(prev, pos - prev);
				if (!token.empty()) v.push_back(token);
				prev = pos + delim.length();
			} while (pos < str.length() && prev < str.length());
			return ret;
		}

		template <typename T>
		struct numeric
		{
			using TY = numeric<T>;
			using UNDERLYING_TYPE = T;
			numeric() {}
			numeric(const T& other) : m_t(other) {}
			numeric(const TY& other) : m_t(other.m_t) {}
			numeric& operator=(const TY& other) { m_t = other.m_t; }
			T& operator()() { return m_t; }
			const T& operator()() const { return m_t; }
			bool operator==(const T& rhs) const { return m_t == rhs; }
			bool operator==(const TY& rhs)const { return m_t == rhs.mt; }
			bool operator!=(const T& rhs) const { return rhs != m_t; }
			bool operator!=(const TY& rhs)const { return rhs.m_t != m_t; }
			TY& operator++() { ++m_t; return *this; }

		private:
			T m_t{ T() };
		};

		struct columnindex_t : public numeric<int16_t> {
			columnindex_t(const int16_t& i) : numeric<int16_t>(i) {}
			columnindex_t() {}

		};
	
		struct column
		{
			
			column(const std::string& name, columnindex_t index) : m_sname(name), m_index(index) {}
			column() {}
			const std::string& name() const { return m_sname; }
			const columnindex_t index() const { return m_index; }
		private:
			std::string m_sname;
			columnindex_t m_index;
		};

		template <typename K, typename T>
		// a collection that can be accessed by KEY, or by INDEX
		struct namevalues
		{
			using map_t = std::map<K, T>;
			using vec_t = std::vector<T>;
			using pair_t = std::pair<K, T>;
			using insert_t = typename std::pair<typename map_t::iterator, bool>;

			insert_t add(const pair_t& pr) {
				auto ret = m_map.insert(pr);
				if (ret.second == true) {
					m_vec.push_back(pr.second);
				}
				return ret;
			}

			bool exists(const T& value) { return m_map.find(value) != m_map.end(); }
			bool empty() { return size() == 0; }
			const typename vec_t::size_type size() const { return m_vec.size(); }
			T& value(const K& key) const { return m_map[key]; }
			T& value(const K& key) { return m_map[key]; }
			T& value(const typename vec_t::size_type& idx) const { return m_vec.at(idx); }
			T& value(const typename vec_t::size_type& idx) { return m_vec.at(idx); }
			const auto& map() const { return m_map; }
			const auto& vec() const { return m_vec; }
			std::string serialize() const {
				std::stringstream ss;
				for (const auto& col : m_vec)
				{
					ss << col.first << '\t';
				}
				std::string ret(ss.str());
				ret = ret.substr(0, ret.length() - 1);
				ret += "\r\n";
				return ret;
			}
		private:
		protected:
			map_t m_map;
			vec_t m_vec;

		};

		template <typename T>
		struct uid_generator
		{
			T next() {
				return ++mt;
			}
			T m_t;
		};
		
		using uid_t = numeric<uint64_t>;
		using index_t = int64_t;
		using date_t = int64_t;
		

#pragma pack(1)
		template <typename T>
		// one of these for each db entry. T is the type of *your* struct
		struct entry
		{
			using mytype = entry<T>;
			using UDATA = T;
			uid_t uid;
			date_t created;
			date_t modified;
			T data;
#ifndef NDEBUG
			// ~entry() { static_assert(std::is_pod<mytype>::value, "user data must be a POD type."); }
#endif
		};

#pragma pack()

		static inline uid_t UID_INVALID() { return uid_t{ 0 }; }
		using column_coll = namevalues<std::string, columnindex_t>;

		struct columns_t : public column_coll
		{
			using base = namevalues<std::string, columnindex_t>;
			auto add(const std::string& s) {
				columnindex_t ci(static_cast<uint16_t>(size()));
				auto ret = base::add(std::make_pair(s, ci));
				return ret;
			}

			auto columnindex(const std::string& name) {
				auto f = this->map().find(name);
				if (f == this->map().end()) {
					return columnindex_t(-1);
				}
				else {
					return f->second;
				}
			}
		};

		template <typename T, typename C>
		class core
		{
		public:
			static constexpr int VERSION = 1;
			
			using ENTRY_TYPE = T;
			using CALLBACK_TYPE = C;
			using entryvec_t = std::vector<T>;
			using UID_GEN = uid_generator<uid_t>;
			

#pragma pack(1)
			struct header { 
				static constexpr int64_t MAGIC = 199259963558819;
				static constexpr int16_t VERSION = 1;
				columns_t cols;
			};
#pragma pack()


			core(std::fstream& f) : m_f(f) {
				assert(f.is_open() && "dbcore: file must be opened.");
				populate();
			}

			columns_t& columns() { return m_header.cols; }
			void populate() 
			{

				T entry;
				constexpr auto sz = sizeof(entry);
				while (!m_f.eof())
				{
					if (read_header() == NOERROR)
					{
						m_f.read((char*)&entry, sz);
						if (m_f) {
							m_callback.pop_data(entry, columnindex_t{});
							assert(entry.uid != UID_INVALID() && "uid on entries should always be valid.");
						}
						else {
							if (m_f.eof()) break;
							assert(m_f.eof() && "Error other than eos not expected from the file");
							break;
						}
					}
					else {
						if (m_header.cols.empty()) {
							header_add_defaults();
						}
					}
				}

				int handled = m_callback.pop_complete(num_entries(), *this);
				(void)handled;

			}

			int64_t num_entries() const { return static_cast<int64_t>(m_vec.size()); }
		protected:
			std::fstream& m_f;
			C m_callback;
			entryvec_t m_vec;
			UID_GEN m_uidgen;
			header m_header; 
		private:
			
			int read_header() {
				m_f.exceptions(0);
				m_f.seekg(0);
				m_f.clear();
				std::string s(1024, '\0');
				m_f.read(&s[0], 1024);
				
				if (s.empty()) {
					return -1;
				}
				const char* TAB = "\t";
				strvec_t v = split(s, "\r\n");
				if (v.size() < 3){
					throw std::runtime_error("read_header: corrupt file. Incorrect signature");
				}

				strvec_t splut_tab = split(v[0], TAB);
				if (splut_tab.size() != 2) {
					throw std::runtime_error("read_header: corrupt file. Malformed start.");
				}
				if (splut_tab[0].empty()) {
					if (splut_tab.size() != 2) {
						throw std::runtime_error("read_header: corrupt file. No magic.");
					}
				}
				int64_t my_magic = std::atoll(splut_tab[0].c_str());
				if (my_magic != header::MAGIC) {
					throw std::runtime_error("read_header: corrupt file. Magic fail.");
				}
				if (splut_tab[1].empty()) throw std::runtime_error("read header: No version specified.");
				int ver = std::atoi(splut_tab[1].c_str());
				if (ver != header::VERSION) {
					// of course if you know how to upgrade it, don't throw this!
					throw std::runtime_error("read_header: Incorrect version");
				}
			
				return NOERROR;

			}

			void write_header() {
				assert(m_f.is_open() && "write_header(): file must be open!");
				m_f.clear();
				m_f.seekp(0);
				std::string s = m_header.cols.serialize();
				m_f << m_header.MAGIC << "\t";
				assert(m_f && "file not ok after writing MAGIC header");
				m_f << VERSION << "\r\n";
				m_f.write(s.data(), s.size());
				assert(m_f && "file not ok after writing column data");
			}

			void header_add_defaults() {
				auto& cols = m_header.cols;
				columnindex_t idx(0);
				cols.add("uid");
				cols.add("date_modified");
				cols.add("date created");
				write_header();
			}
		};

	}
}