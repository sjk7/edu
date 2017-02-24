// dbrows.h. Steve. Sat 18th Feb 2017. No restrictions. Hope it helps you!

#pragma once
#ifndef DBROWS_H
#define DBROWS_H

#include "dbheader.h"
#include "dbcolumns.h"
#include "utils.h" // filesize()
#include <unordered_map>


namespace cpp
{
	namespace db
	{
		template <typename T>
		struct uid_gen {
				NO_COPY(uid_gen);
				uid_gen() {}
				T next()
				{
					return ++m_t;
				}

				void startpoint_set(T t)
				{
					m_startpoint = t;
				}
				T startpoint() const
				{
					return m_startpoint;
				}

			private:
				T m_startpoint {0};
				T m_t {0};
		}; // uid_gen


		using uidgen_t = uid_gen<uid_t>;



		struct row_t {

				using uid_t = cpp::db::uid_t;
				using idx_t = cpp::db::rowidx_t;
				friend struct rows_t; // already forward declared for header
				

				template <typename T>
				static inline void pop(columns_t& cols, const rowidx_t row, column::idx_t colindex, const T& t)
				{
					assert(colindex < cols.size_s() && "pop() (row): bad column index.");
					auto& col = cols[colindex];
					col.value_set(row, t, uid_t());
				}

				template <typename T, typename... ARGS>
				void pop(columns_t& cols, const rowidx_t row, column::idx_t colindex, const T& first, const ARGS& ... args)
				{
					assert(colindex < cols.size_s() && "pop() (row): bad column index during iteration.");
					auto& col = cols[colindex];
					col.value_set(row, first);
					colindex++;
					pop(cols, row, colindex, args...);
				}

				int64_t now_gmt() const
				{
					std::time_t t = std::time(nullptr);
					return static_cast<int64_t>(t);
				}

				template <typename... ARGS>
				// Use this constructor to "add" a row's data to cols when you don't have a uid
				row_t (db::rowidx_t idx, columns_t& cols, uidgen_t& gen, ARGS&& ... args)

				{
					using namespace std;
					db::rowidx_t test;
					cout << "Test is: " << test << endl;
					cout << "idx is: " << idx << endl;

					THROW_ASSERT(idx >= 0, -1, "row_t(): bad index");
					column::idx_t start_index = 0;
					const auto sz = sizeof...(ARGS);

					if (m_uid == UID_INVALID){
						m_uid = gen.next();
					}
					if (sz != cols.size()){
						THROW_ASSERT(sz == cols.size() - NUM_DEFAULT_COLUMNS, -1, "row_t(): bad number of variadic parameters");
						pop_default_columns(cols, idx);
					}

					if (sz != cols.size()) {
						start_index = NUM_DEFAULT_COLUMNS;
					}

					pop(cols, idx, start_index, std::forward<ARGS>(args)...);


					m_cols = &cols;
					m_idx = idx;
				}


				row_t(rowidx_t rwidx, columns_t* pcols = nullptr) : m_cols(pcols), m_idx(rwidx) {
					assert(m_uid == UID_INVALID); // should be automatic
					m_uid = rwidx.value(); // will be double-checked during commit()
				}

				row_t() : m_cols(nullptr){
					assert(m_uid == UID_INVALID); // should be automatic
					assert(m_idx == ROW_INVALID);
				}

				// returns mutable value (you get an actual reference to the value itself)
				std::string& operator[](const std::string& colname)
				{
					THROW_ASSERT(m_cols , -1,  "Impossible. Row does not have a pointer to the columns!");

					if (m_idx < 0) {
						THROW_ERR(-1, "row_t::operator[]: won't work if we do not have a rowindex.",
								  "My rowindex is:", m_idx, " and my uid is: " , m_uid);
					}

					auto& cols = *m_cols;
					auto idx = m_idx;
					auto& col = cols[colname];

					if (cols.m_sortstate.sortcol &&
						cols.m_sortstate.sortcol == &col) {
						idx = cols.sorted_uid_from_index(m_idx);
						

					}
	
					auto& value = col.value(idx);
					return value;
				}

				bool is_populated() const{ return m_uid == UID_INVALID; }

				// returns mutable value (you get an actual reference to the value itself)
				std::string& operator[](const column::idx_t idx)
				{
					assert(m_cols && "Impossible. Row does not have a pointer to the columns!");

					if (m_idx < 0) {
						THROW_ERR(-1, "row_t::operator[(idx_t)]: won't work if we do not have a rowindex.",
								  "My rowindex is:", m_idx, " and my uid is: " , m_uid);
					}

					auto& cols = *m_cols;
					auto& col = cols[idx];
					auto& value = col.value(m_idx);
					return value;
				}

				void pop_default_columns()
				{
					THROW_ASSERT(m_cols, -1, "pop_default_columns(): no columns");
					THROW_ASSERT(m_idx != ROW_INVALID, -1, "pop_default_columns(): invalid row");
					columns_t& cols = *m_cols;

					auto& uc = cols[0];
					auto& col = cols[0]; (void)col;
					assert(&uc == &col);


					uc.value_set(m_idx, m_uid);
					auto& dc = cols[1];

					if (dc.value(m_idx).empty()) {
						dc.value_set(m_idx, now_gmt(), m_uid);
					}

					cols[2].value_set(m_idx, now_gmt(), m_uid);

				}

				void pop_default_columns(columns_t& cols, const rowidx_t& rowindex)
				{
					if (cols.size() < NUM_DEFAULT_COLUMNS){
						WARN("pop_default_columns: NUM_DEFAULT_COLUMNS=",
							 NUM_DEFAULT_COLUMNS, " but we have", cols.size(), "columns.");
						return;
					}

					m_cols = &cols;
					m_idx = rowindex;
				}
				uid_t uid() const
				{
					return m_uid;
				}


				// Your "return value" sret should be reserve()d
				// to the total columns' width before calling me.
				void serialize(std::string& sret) const
				{
					const auto& rwidx = m_idx;
					assert(rwidx >= 0 && "cannot serialize a row unless it has a rowindex.");
					m_tmp.clear(); sret.clear();
					assert(sret.capacity() > 0);  // caller should reserve() it to the total columns' width for efficiency
					assert(m_tmp.capacity() > 0); // caller should reserve() it to the largest column for efficiency.
					column::idx_t colidx = 0;

					for (column* pc : m_cols->vector_non_const()) {
						pc->value_fixed_length(rwidx, m_tmp);
						sret.append(m_tmp);
						colidx++;
					}
					sret.append("\n");

					auto l = sret.length();
					auto l2 = m_cols->total_data_width();
					THROW_ASSERT(l == l2, -1 , "Incorrect data width during serialize()");
				}

				auto serialize_prepare()
				{
					assert(m_cols && "serialize_prepare: no cols!");
					auto ret = m_cols->total_data_width();
					m_tmp.reserve(ret);
					return ret;
				}

				rowidx_t index() const{
					return m_idx;
				}


			protected:
				mutable std::string m_tmp;
				std::vector<std::string> m_vec;
				uid_t m_uid;
				columns_t* m_cols;

				db::rowidx_t m_idx{ -1};


		}; // struct row_t



		struct rows_t {

			NO_COPY (rows_t);

			friend struct core;

				using uid_t = cpp::db::uid_t;
				using idx_t = cpp::db::rowidx_t;
				using vecidx_t = std::vector<idx_t>;
				using vecuid_t = std::vector<uid_t::impl>;
				using rowvec_t = std::vector<uid_t>;
				using rowmap_t = std::map<rowidx_t, row_t>;
				using urowmap_t = std::unordered_map<rowidx_t::impl, uid_t::impl>;

				using cols_t = columns_t;

				rows_t(header& head, open_flags flags) : m_columns(head.m_cols), m_head(head)
				{

					if (flags == open_flags::read || flags == open_flags::readwrite){
						read_all();
					}
				}

				static std::iostream::pos_type filesize(std::fstream& f)
				{
					return cpp::filesize(f);
				}

				// checks the size of the file for sanity, returns how many rows in the file.
				int64_t check_file_size(std::fstream& f)
				{
					int64_t fsize = filesize(f);
					const auto frp = m_head.first_row_position();
					assert(frp);
					auto data_size = fsize - frp;
					if (data_size<=0) return 0; // nothing to read!
					auto rec_size = m_columns.total_data_width();
					auto div = data_size % rec_size;
					THROW_ASSERT(div == 0,  -1, "Bad storage size.");
					auto num_records = data_size / rec_size;
					return num_records;

				}

				void read_all()
				{
					std::fstream& f = m_head.m_f;
					auto rowcount = check_file_size(f);
					if (!rowcount)return; // no error, just no rows.

					auto rec_size = m_columns.total_data_width();
					move_to_row_index(rowidx_t(0), m_head);
					rowidx_t idx(0);
					std::string s;
					s.resize(rec_size);
					m_columns.rowcount_set(static_cast<int32_t>(rowcount));
					uid_t uid;

					size_t cnt = static_cast<size_t>(rowcount);
					m_umap.reserve(cnt);
					m_imap.reserve(cnt);

					while (!f.eof())
					{
						f.read(&s[0],rec_size);
						const auto& plast = s.at(rec_size-1);
						THROW_ASSERT(plast == '\n', -10, "read_all(): incorrect row termination character");
						if (f.eof()){
							f.clear();
							break;
						}
						uid = m_columns.pop_row(idx, s);
						THROW_ASSERT(uid != uid_t::INVALID_VALUE(), -1, "Whilst reading file, uid was not valid.");
						// map indexes to uids so we can quickly
						// get a row index from a uid
						const auto i = idx.value();
						const auto u = uid.value();
						m_umap[u] = i;
						// also map uids so we can quickly
						// get a uid for a given index
						m_imap[i] = u;
						++idx;

					};

				}

				row_t row_from_uid(const uid_t& uid) const
				{
					assert(uid.is_valid() && "why isn't the uid valid?");
					rowidx_t idx = row_idx_from_uid(uid);
					return row_t(idx, &m_columns);
				}

				rowidx_t row_idx_from_uid(const uid_t& uid) const
				{
					const auto& m = m_umap;
					const auto e = m.end();
					const auto it = m.find(uid.value());

					if (it == e) {
						THROW_ERR(-1, "row, correspondinf to uid: ", uid, " is not found.");
					}
					assert(it->second == uid.value());
					return rowidx_t(it->second);
				}

				uidgen_t& uidgen ()
				{
					return m_uid_gen;
				}



				void save(header& head)
				{
					head.save();

					if (m_dirtyrows.empty()) return;

					int c = 0;
					rowidx_t last_idx(-1);
					auto& f = head.m_f;
					f.clear();
					std::string stmp;
					std::string srow;
					const auto& cols = head.m_cols;
					const auto largest_col = cols.largest();
					assert(largest_col && "Surely there must always be a largest column??");
					stmp.reserve(largest_col->width());
					auto tot_wid = cols.total_data_width();
					srow.reserve(tot_wid);

					// this works because the m_dirtyrows are in a map,
					// guaranteed to be sorted by ascending index.
					const auto& first_row = m_dirtyrows.begin()->second;
					m_head.seek_to_row(first_row.index());

					size_t cnt = m_dirtyrows.size() + cols.rowcount_u();
					m_umap.reserve(cnt);
					m_imap.reserve(cnt);

					for (auto& pr : m_dirtyrows) {
						const auto& row = pr.second;
						db::rowidx_t rwidx = row.m_idx;
						row.serialize(srow);
						assert(rwidx >= 0 && "whaa... can't save a row if it has no index!");

						if (c == 0) {
							const auto rsize = srow.size(); (void)rsize;
							assert(rsize == tot_wid  && "when saving a row, length is mismatched.");
						} else {
							if (rwidx != last_idx + 1) {
								move_to_row_index(rwidx, head, true);
							}
						}

						f.write(&srow[0], srow.size());
						assert(f && "file should be in a good state after writing a row.");
						last_idx = c;
						const auto i = rwidx.value();
						const auto u= row.m_uid.value();
						m_umap[u] = i;
						m_imap[i] = u;
						c++;
					};

					f.flush();
					update_uids();
					THROW_ASSERT(m_umap.size() == m_imap.size(), -1, "mapping error.");
					m_dirtyrows.clear();
				}

				// "dirty" rows. These need to be saved to storage
				void dirty_add(const row_t& row)
				{
					m_dirtyrows[row.index()] = row;
				}

				const rowidx_t::impl size() const {	return m_columns.rowcount();	}

			protected:
				uid_t m_uid {0};

				// populates the data with the uids we have.
				// Fail to call this after save() and only the first three columns
				// will have uids associated with the data.
				void update_uids() {

					auto& cols = m_columns;
					for (int i = NUM_DEFAULT_COLUMNS; i < cols.size(); ++i)
					{
						auto& col = cols[i];
						const auto& uid_col = cols[0];
						
						rowidx_t rw(0);
						for (auto& pr : col.values_non_const())
						{						
							auto uid = uid_col.value_uid(rw);
							assert(uid.is_valid()); // seen this when you forgot to resize the rows properly when u finished adding stuff
							pr.first.uid = uid;
							pr.first.idx = rw;
							++rw;
						}
					};

					std::cout << "updated";
				}

				mutable std::map<cpp::db::uid_t::impl, cpp::db::rowidx_t::impl> m_map_uid;

				uidgen_t m_uid_gen;
				// the rows in here want saving to storage
				rowmap_t m_dirtyrows;
				urowmap_t m_umap;	// get a uid from an index
				urowmap_t m_imap;	// get an index from a uid.
				// it's convenient if we can see the columns.
				cols_t& m_columns;
				header& m_head;
				

				void move_to_row_index(rowidx_t idx, header& head, bool for_writing = false)
				{
					auto startpos = head.first_row_position();
					std::fstream& f = m_head.m_f;
					f.exceptions(std::ios_base::goodbit);
					f.clear();
					size_t i = idx.to_size_t();

					auto mypos = startpos + (i * m_columns.total_data_width());

					if (for_writing) {
						f.seekp(mypos);
					} else {
						f.seekg(mypos);
					}

					assert(f);
				}


		}; // struct rows_t


	} // namespace db
} // namespace cpp


#endif // DBROWS_H
