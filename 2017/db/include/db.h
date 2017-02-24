#pragma once
#ifndef DB_H
#define DB_H

#ifndef _CRT_SECURE_NO_WARNINGS
#	define _CRT_SECURE_NO_WARNINGS
#endif

// if using qt creator, put this in the .pro file:
// CONFIG += c++1z # latest and greatest c++ version.

#include "dbrows.h"


namespace cpp
{


	namespace db
	{


		struct core {

			using rowidx_t = cpp::db::rowidx_t;
			using uid_t = cpp::db::uid_t;

			core(std::fstream& f, open_flags flags = open_flags::defaults)
				: m_f(f), m_head(m_f, flags), m_rows(m_head, flags) {}

			core(const core& other) = delete;
			core& operator=(const core& other) = delete;

			void save()
			{
				m_rows.save(m_head);
			}

			const rows_t& rows() const {
				return m_rows;
			}

			const columns_t& columns() const
			{
				return m_head.columns_const();
			}

			int32_t rowcount() const
			{
				const auto& cols = m_head.columns_const();
				const auto& col = cols[0];
				return static_cast<int32_t>(col.values().size());
			}

			row_t next_free_row()
			{
				auto& cols = m_head.columns();
				auto nfri = cols.next_free_row_index();
				assert(nfri != ROW_INVALID);
				return row_t(nfri, &cols);
			}

			void rowcount_set(const rowidx_t& newcount) {
				m_head.columns().rowcount_set(newcount.value());
			}

			void row_commit(row_t& row) {
				row.pop_default_columns();
				const auto& uid = row.uid();
				assert(uid.is_valid() && "row_commit: invalid uid");
				m_rows.dirty_add(row);
			}

			void column_add(const std::string& name,
				const column_types::TYPE type = column_types::STRING,
				size_t width = USE_DEFAULT_WIDTH)
			{
				if (rowcount()) {
					THROW_ERR(-1, "Adding more columns when there are rows is not supported.",
						"You must add columns only to an empty database.");
				}

				auto& cols = m_head.m_cols;
				cols.add(name, type, width);
				m_head.m_dirty = 1;
			}


			template <typename... ARGS>
			// add a row with new data
			void add(ARGS&& ... args)
			{
				const size_t n = sizeof...(ARGS);
				auto& cols = m_head.m_cols;

				if (n != cols.size() - NUM_DEFAULT_COLUMNS) {
					THROW_ERR(-3, "db::add(): the number of arguments must either be ",
						"the same as the total number of columns -1, or the ",
						"total number of columns - NUM_DEFAULT_COLUMNS",
						"You sent me", n, "arguments, but the total number of columns is:",
						cols.size());
				}

				const auto rws = rowcount();
				cols.rowcount_set(rws + 1);
				row_t row(rowidx_t(rws), cols, m_rows.uidgen(), std::forward<ARGS>(args)...);
				const auto uid = row.uid();

				if (uid >= 0) {
					m_rows.dirty_add(row);
				}
			}

			void add_row(const std::vector<std::string>& values, const rowidx_t& rw)
			{
				auto& cols = m_head.columns();
				cols.add_row_data(rw, values);
			}

			std::ostream& print_cols(std::ostream& os) const
			{
				using namespace std;
				if (columns().empty()) {
					os << "No columns: database empty." << endl;
				}
				else {
					os << "columns ... " << endl;
					for (const column* pcol : columns().vector_const()) {
						os << pcol->name() << "|" << pcol->type_as_str() << "|" << pcol->width() << "    ";
					}
				}
				os << endl;

				return os;
			}

			void print_rows(std::ostream& os, int num_records = -1) const
			{
				auto rc = rowcount();
				using namespace std;
				rowidx_t cur(0);
				std::string s;

				while (cur < rc)
				{
					for (const auto p : columns().vector_const())
					{
						p->value_const(cur, s);
						os << s << "    ";
					}
					++cur;
					os << endl;
					if (cur >= num_records && num_records != -1) {
						break;
					}
				};
				os << endl;
			}

			void print(std::ostream& os, int num_records = -1) const
			{
				using namespace std;
				os << "----------------------------------------------------" << endl << endl;
				os << "Database version is: " << header::VERSION() << endl;
				os << "Built: " << __DATE__ << " " << __TIME__ << "." << endl;
				if (num_records == -1) num_records = rowcount();
				print_cols(os);
				print_rows(os, num_records);

			}


			void sort(const std::string& column_name, const cpp::sortorder order)
			{
				auto& cols = columns_non_const();
				auto& col = cols[column_name];
				std::cout << col.name();
				auto& v = col.values_non_const();

				const auto ty = col.type();
				switch (ty)
				{
				case cpp::db::column_types::INT:
				case cpp::db::column_types::INT64: // this is also good for dates, without saying anything.
				{

					sort_int_values(v, order);
					break;
				}
				case cpp::db::column_types::STRING:
				{
					sort_string_values(v, order);
					break;
				}
				default:
					assert(0); // implement your own sort by sending the appropriate comparator
				};

				cols.m_sortstate.order = order;
				cols.m_sortstate.sortcol = &col;
				cols.index_after_sort();
			}

			uid_t uid_from_index(const rowidx_t& row)
			{
				auto& cols = columns();
				if (cols.empty()) {
					THROW_ERR(-1, "uid_from_index: no columns.");
				}
				auto psortcol = cols.m_sortstate.sortcol;
				if (psortcol) 
				{
					return psortcol->value_uid(row);
				}
				else {
					const auto& uid_col = cols[0];
					return uid_col.value_uid(row);
				}
			}

			rowidx_t uid_to_index(const uid_t& uid) 
			{
				return m_rows.row_idx_from_uid(uid);
			}

			row_t row_from_uid(const uid_t& uid) {
				return m_rows.row_from_uid(uid);
			}

			row_t row_from_index(const rowidx_t& idx)
			{
				uid_t u = uid_from_index(idx);
				return m_rows.row_from_uid(u);
			}

		protected:
			std::fstream& m_f;
			header m_head;
			rows_t m_rows;
			
			columns_t& columns_non_const() { return m_head.columns(); }

			template <typename T>
			void sort_int_values(T& v, const cpp::sortorder order) {
				if (order == cpp::sortorder::ASC) {
					std::stable_sort(v.begin(), v.end(), [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
				}
				else {
					std::stable_sort(v.begin(), v.end(), [](const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; });
				}
			}

			template <typename T>
			void sort_string_values(T& v, const cpp::sortorder order) {
				if (order == cpp::sortorder::ASC) {
					std::stable_sort(v.begin(), v.end(), [](const auto& lhs, const auto& rhs) {
						return _stricmp(lhs.second.c_str(), rhs.second.c_str()) < 0; });
				}
				else {
					std::stable_sort(v.begin(), v.end(), [](const auto& lhs, const auto& rhs) {
						return _stricmp(lhs.second.c_str(), rhs.second.c_str()) > 0; });
				}
			}


		};// core


	} // namespace db



} // namespace cpp

#endif // DB_H
