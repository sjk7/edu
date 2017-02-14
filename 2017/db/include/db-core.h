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

namespace sjk
{
	namespace dbcore
	{

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

		struct columnindex_t : public numeric<int16_t> {};
		template <typename T>
		struct column
		{
			using vec_t =  std::vector<T>;
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
					ss << col << '\t';
				}
				std::string ret(ss.str());
				ret = ret.substr(0, ret.length() - 1);
				ret += "\r\n";
				return ret;
			}
		private:
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

		template <typename T, typename C>
		class core
		{
		public:
			static constexpr int VERSION = 1;
			
			using ENTRY_TYPE = T;
			using CALLBACK_TYPE = C;
			using entryvec_t = std::vector<T>;
			using UID_GEN = uid_generator<uid_t>;
			using column_t = column<T>;
			using columns_t = namevalues<std::string, column_t>;

#pragma pack(1)
			struct header { 
				static constexpr int64_t MAGIC = 199259963558819;
				int16_t version; 
				columns_t cols;
			};
#pragma pack()


			core(std::fstream& f) : m_f(f) {
				populate();
			}

			columns_t& columns() { return m_header.cols; }
			void populate() 
			{

				T entry;
				constexpr auto sz = sizeof(entry);
				while (!m_f.eof())
				{
					read_header();
					m_f.read( (char*)&entry, sz);
					if (m_f) {
						m_callback.pop_data(entry, columnindex_t{});
						assert(entry.uid != UID_INVALID() && "uid on entries should always be valid.");
					}
					else {
						if (m_f.eof()) break;
						assert(m_f.eof() &&  "Error other than eos not expected from the file");
						break;
					}
				}
								
				if (m_header.cols.empty()) {
					header_add_defaults();
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
			
			void read_header() {
				m_f.seekg(0);
				m_f.exceptions(0);

			}

			void write_header() {
				m_f.seekp(0);
				std::string s = m_header.cols.serialize();
				m_f.write((char*)&header::MAGIC, sizeof(header::MAGIC));
				assert(m_f && "file not ok after writing MAGIC header");
				m_f.write(s.data(), s.size());
				assert(m_f && "file not ok after writing column data");
			}

			void header_add_defaults() {
				auto& cols = m_header.cols;
				cols.add(std::make_pair("uid", column<uid_t>()));
				cols.add("date_modified");
				cols.add("date_created");
				write_header();
			}
		};

	}
}