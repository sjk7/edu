// dbcolumns.h, by Steve. Sat 18th Feb 2017. No restrictons. Hope it helps you!
#pragma once
#ifndef DBCOLUMNS_H
#define DBCOLUMNS_H

#include "strings.h"
#include "collection.h"
#include "dbtypes.h"
#include <inttypes.h> //strtoumax
#include <numeric> // std::accumulate
#include <algorithm> // std::max_element
#include "sorting.h"


namespace cpp
{


	namespace db
	{
		struct column;
		namespace detail {
			using sortorder = cpp::sortorder;
			
			struct sortstate_t {
				sortorder order{ sortorder::NONE };
				column* sortcol{ nullptr };
			};
		}

		struct core;
		struct header;
		struct columns_t;
		struct rows_t;
		

		struct column {


				using idx_t = int16_t;
				using vec_t = std::vector<valpr_t>;
				friend struct columns_t;
				friend struct core;
				friend struct rows_t;

				column() {} // must have default constructor to use in STL containers.
				column(const std::string& name, const std::string& type_name, const idx_t idx,
					   size_t width = USE_DEFAULT_WIDTH) :
					m_name(name), m_ty(column_types::TYPE::BAD), m_index(idx), m_width(width)
				{
					check_name();
					m_ty = column_types::type_from_name(type_name);
					check_width();
				}
				column(const std::string& name, const column_types::TYPE type, const idx_t idx,
					   size_t width = USE_DEFAULT_WIDTH)
					: m_name(name), m_ty(type), m_index(idx), m_width(width)
				{
					check_name();
					check_width();
				}

				const std::string& name() const
				{
					return m_name;
				}
				idx_t index() const
				{
					return m_index;
				}
				static std::string type_str (const column_types::TYPE t)
				{
					return column_types::type_to_name(t);
				}

				std::string type_as_str ()  const
				{
					return column_types::type_to_name(m_ty);
				}
				column_types::TYPE type() const
				{
					return m_ty;
				}
				size_t width() const
				{
					return m_width;
				}
				const vec_t& values() const
				{
					return m_values;
				}

	

				uid_t value_uid(const rowidx_t& idx) const
				{
					const auto& pr = m_values.at(idx.value());
					return pr.first.uid;
				}

				std::string& value(const rowidx_t& idx)
				{
					size_t st = idx.to_size_t();
					auto& pr = m_values[st];
					return pr.second;
				}

				// Preferred method of getting a const value.
				void value_const(rowidx_t idx, std::string& rv) const
				{
					size_t st = idx.to_size_t();
					const auto& pr = m_values[st];
					rv.assign(pr.second);
				}

				// DO NOT prefer this one for efficiency, see the overload idx, std::string.
				std::string value_const2(const rowidx_t& idx) const
				{
					size_t st = idx.to_size_t();
					const auto& pr = m_values[st];
					return pr.second;
				}

				void value_fixed_length(const rowidx_t rw, std::string& rv) const
				{
					value_const(rw, rv);
					rv.resize(width()); // if you were good, and reserve()d this beforehand, no biggie
				}

				void ensure_enough_rows(const rowidx_t row){
					size_t srow = row.to_size_t();
					if (srow >= rowcount_u() -1) {
						// m_values.resize(srow+1);
					}
				}

				void value_set(const rowidx_t& row, const std::string& value)
				{
					THROW_ASSERT(row < rowcount(), -1, "Not enough rows: use ensure_enough_rows()");
					using namespace std;
					stringstream ss;
					ss << value;
					THROW_ASSERT(row.to_size_t() < m_values.size(), -EINVAL, "row: value_set(): row", row, "out of bounds.",
								 "values' size is: ", m_values.size());

					auto& pr = m_values[row.to_size_t()];
					cout << pr.first.uid << "|" << pr.second << endl;
					pr.first.uid = uid_t();
					assert(0);
					pr.second = value;
				}

				// special-case setting the value of uid, since it is critical:
				void value_set(const rowidx_t& row, const uid_t& uid){

					THROW_ASSERT(uid != UID_INVALID, -1,  "must have a valid uid when saving") ;
					value_set(row, static_cast<uid_t::impltype>(uid), uid);

				}

				template <typename T>
				void value_set(const rowidx_t& row, const T& value, const uid_t& uid)
				{
					THROW_ASSERT(row < rowcount(), -1, "value_set: Not enough rows. Row is", row, " and rowcount is ", rowcount());

					using namespace std;
					stringstream ss; ss << value;
					THROW_ASSERT(row.to_size_t() < m_values.size(), -1, "row: value_set(): row", row, "out of bounds.",
								 "values' size is: ", m_values.size());
					//collections::bounds_checker(m_values, row);
					auto& pr = m_values[row.to_size_t()];

					pr.first.uid = uid;
					pr.first.idx = row;
					assert(uid.is_valid() && "invalid uid in value_set");
					pr.second = ss.str(); // watch this in clang. it shows "" in the debugger. Not true!
				}

				void value_set(const rowidx_t& row,  const char* value, const size_t wid, uid_t& uid)
				{
					using namespace cpp::strings;
					THROW_ASSERT(value != NULL , -1, "value_set (const char*): NULL value not permitted.");
					THROW_ASSERT(row < rowcount(), -1, "Not enough rows: use ensure_enough_rows()");
					if (!value) {
						value_set(row, ""s);
					}
					THROW_ASSERT(wid == width(),  -1, "value_set: incorrect width: expected ", width(), "but got: ", wid);
					
					if (m_index == 0) {
						auto tmpuid = static_cast<uid_t::impl>(strtoimax(value, nullptr, 10));
						// I *get* the uid for the first column, and you are expected to remember it for the rest.
						uid = tmpuid;
						assert(uid.value() != uid_t::INVALID_VALUE());
					}
					 
					// NOTE: we store the row data without all the nulls to conserve memory
					assert(uid.value() != uid_t::INVALID_VALUE());
					valpr_t val = std::make_pair(val_t(uid, row), std::string(value));
					
					m_values[row.to_size_t()] = val;
				}



				// get a UNsigned value of rowcount
				uint32_t rowcount_u() const
				{
					return static_cast<int32_t> (m_values.size());
				}

				rowidx_t::impl rowcount() const
				{
					return static_cast<rowidx_t::impl>(m_values.size());
				}



			private:
				/*!
				 * \brief rowcount_set: resize the vector of values for this column
				 * \param newsize
				 */
				void rowcount_set(const size_t newsize)
				{
					m_values.resize(newsize);
				}

				void check_width()
				{
					const auto width = m_width;

					if (m_ty == column_types::TYPE::STRING) {
						if (width == USE_DEFAULT_WIDTH) {
							m_width = 60;
						}
					}

					if (m_width == USE_DEFAULT_WIDTH || m_ty != column_types::TYPE::STRING) {
						m_width = column_types::width_for_type(m_ty);
					}
				}

				void check_name() const
				{
					using namespace cpp::strings;
					if (m_name.find(PIPE) != std::string::npos) {
						throw cpp::error(-3, "Character pipe ('|') is not allowed in column name");
					}

					if (m_name.find(TAB) != std::string::npos) {
						throw cpp::error(-3,"TAB character is not allowed in column name");
					}

					if (m_name.find(NL) != std::string::npos) {
						throw cpp::error(-3, "NEWLINE character is not allowed in column name");
					}
				}

				protected:
				vec_t&  values_non_const() { return m_values; }
				std::string m_name;
				column_types::TYPE m_ty { column_types::TYPE::BAD };
				idx_t m_index { -1};
				size_t m_width{0};
				vec_t m_values;


		}; // struct column

		using cc_t = cpp::collections::collection<std::string, column, column*>;

		struct columns_t : cc_t {

				friend struct  column;
				friend struct core;
				friend struct row_t;
				using base_t = cc_t;
				using K = std::string;
				using addpr_t = base_t::addpr_t;

				// NO_COPY(columns_t);
				columns_t() {}

				std::string serialize() const
				{
					std::stringstream ss;
					using namespace cpp::strings;

					try {
						for (const auto* pr : base_t::m_vec) {
							const auto& col = *pr;
							ss << col.name() << PIPE
							   << col.type_str(col.type()) << PIPE << std::to_string(col.width()) << TAB;
						}
					} catch (const cpp::error& e) {
						THROW_ERR(e.code(), "Fatal error serialising columns. ", e.what());
					} catch (const std::exception& e) {
						THROW_ERR( -1, "Fatal error serialising columns. ", e.what());
					}

					return std::string(ss.str());
				}

				base_t::addpr_t add(const std::string& name, const column_types::TYPE ty,
									size_t width = USE_DEFAULT_WIDTH)
				{
					column::idx_t index = static_cast<column::idx_t>(base_t::size());
					column c(name, ty, index, width);
					auto rv = base_t::add(name, c);

					if (!rv.second) {
						THROW_ERR(-1, "Could not add column (likely exists already), with name: ", name);
					}

					m_largest = nullptr;
					return rv;
				}

				using fields_t = std::vector<std::string>;
				// add all the data at once for the specified row
				void add_row_data(const rowidx_t& row, const fields_t& fields)
				{
					const size_t ncols = size();
					const auto fsize = fields.size();
					THROW_ASSERT(fsize == ncols || fsize == ncols - 3,  -1,
								 "add(): Incorrect number of row fields");

					THROW_ASSERT(row < rowcount(), -1, "add_row_data(): not enough rows.");



				}

				size_t total_data_width() const
				{
					if (m_total_data_width) return m_total_data_width;

					m_total_data_width = std::accumulate(base_t::cbegin(),
														 base_t::cend(), 0, // initial value
					[](const size_t sum, const column* p) {
						return static_cast<int>(sum + p->width());
					});
					m_total_data_width += 1; // every row is terminated in a new line.
					return m_total_data_width;
				}

				void from_string(const std::string& data)
				{
					base_t::clear();
					using namespace cpp::strings;
					using std::endl;
					using std::cerr;
					strvec_t v = split(data, TAB);
					strvec_t tmp;

					for (const auto& s : v) {
						tmp = split(s, PIPE);

						THROW_ASSERT(tmp.size() == 3, -2, "FATAL: column, at index:" , size(),
									  "invalid.\n Here's the data:\n", s);
		

						const size_t width = static_cast<size_t>(std::stoull(tmp[2]));
						add(tmp[0], column_types::type_from_name(tmp[1]), width);
					}

					return;
				}

				void rowcount_set (const rowidx_t::impl num_rows)
				{
					THROW_ASSERT(num_rows >= 0,  -1 , "columns():resize_data() numrows should never be negative,",
								 "But I am seeing: ", num_rows);
					THROW_ASSERT(colcount() > 0, -1, "Cannot add rows if there are no columns");

					size_t sz = num_rows;
					int idx = 0;
					auto old_count = -1;

					if (!base_t::empty()){
						old_count = rowcount();
						m_next_free_row_hint = old_count;
					}

					for (column* pc : base_t::m_vec) {
						THROW_ASSERT(pc, -1,  "unexpected null pointer in column collection at index ", idx);
						pc->rowcount_set(sz);
						idx++;
					}
				}

				rowidx_t next_free_row_index()
				{
					rowidx_t ret = m_next_free_row_hint;
					const auto rwcnt = rowcount();
					const auto uid_col = uid_column();
					THROW_ASSERT(uid_col != nullptr, -1, "next_free_row_index: no columns.");


					while (ret < rwcnt){
						if (uid_col->value_uid(ret) == UID_INVALID){
							m_next_free_row_hint = ret.value() + 1;
							return ret;
						}
						++ret;

					};

					ret = rowidx_t();
					THROW_ASSERT(ret != rowidx_t(), -1, "next_free_row_index: no free slots. Use rowcount_set()");
					return ret;
				}


				column* uid_column(){
					if (m_vec.empty()) return nullptr;
					return m_vec[0];
				}

				rowidx_t m_next_free_row_hint {0};

				int32_t colcount() const{
					if (m_vec.empty()) return 0;
					return static_cast<int32_t>(m_vec.size());
				}

				int32_t rowcount() const{
					if (m_vec.empty()) return 0;
					column* pc = m_vec[0];
					return static_cast<int32_t>(pc->m_values.size());
				}

				size_t rowcount_u() const{
					if (m_vec.empty()) return 0;
					column* pc = m_vec[0];
					return static_cast<size_t>(pc->m_values.size());
				}
				// the one with the most data.
				column* largest() const
				{
					if (!m_largest) {
						auto it = std::max_element(base_t::cbegin(), base_t::cend(),
						[] (column const * lhs, column const * rhs) {
							return lhs->width() < rhs->width();
						});

						if (it < base_t::cend()) {
							m_largest = *it;
						}
					}

					return m_largest;
				}


				// populate a row from data
				uid_t pop_row(const db::rowidx_t& rwidx, const std::string& data)
				{
					uid_t ret(uid_t::INVALID_VALUE());
					THROW_ASSERT(rwidx >= 0, -1, "pop_row(): rowindex invalid.");
					const char* ptr = data.c_str();
					const char* ptrend = &data[0] + data.size();

					column::idx_t colidx = 0;

					for (const auto pcol : base_t::m_vec)
					{
						const auto wid = pcol->width();

						pcol->value_set(rwidx, ptr, wid, ret);
						if (colidx == 0){
							ret = pcol->value_uid(rwidx);
							THROW_ASSERT(ret != uid_t::INVALID_VALUE(), -1,
										 "pop_row(): UIDs should never be invalid value");
						}

						ptr += wid;
						THROW_ASSERT(ptr <= ptrend, -1, "pop_row: unexpected: not enough data for row: " , rwidx);
						++colidx;
					}

					return ret;
				}

				const vec_t& vector() const { return m_vec; }


			protected:
				// the one with the most data.
				mutable column* m_largest{nullptr};
				mutable size_t m_total_data_width{0};
				vec_t&  vector_non_const() { return m_vec; }
				detail::sortstate_t m_sortstate;
				using idxvec_t = std::vector<rowidx_t>;
				idxvec_t m_sort_index; // allow fast lookup of index when we are sorted.
				
				// NOTE: only ever use this for the currently sorted column.
				// It would definitely better to build a vector for this when a column is sorted.
				rowidx_t sorted_uid_from_index(const rowidx_t& idx)
				{
					assert(idx.to_size_t() < m_sort_index.size()&& "bad index in sorted_idx_from_index.");
					rowidx_t r = m_sort_index[idx.to_size_t()];
					return r;
				}


				void index_after_sort()
				{
					idxvec_t& v = m_sort_index;
					v.clear();
					auto rc = rowcount_u();
					v.resize(rc);
					assert(m_sortstate.sortcol && "index_after_sort: wrong column.");
					auto p = m_sortstate.sortcol;
					auto& vals = p->values();

					size_t i = 0;
					for (const auto& pr : vals)
					{
						assert(pr.first.idx.is_valid());
						v[i] = pr.first.idx;
						++i;
					}

					assert(v.size() == vals.size() && "index_after_sort: size disagreement.");
				}


		}; // struct columns_t
	}
} // namespace cpp

#endif // DBCOLUMNS_H
