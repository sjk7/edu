#pragma once
#ifndef SJK_DB_H
#define SJK_DB_H

#include <cstdint>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include "sjk_assert.h"
#include "sjk_exception.h"
#include "sjk_io.h"
#include "sjk_timing.h"
#include "sorted_vector.h"

namespace sjk {
    namespace db {
        struct row_type {

            using vt = size_t;

            private:
            mutable vt m_t;

            public:
            using VALUE_TYPE = vt;
            static constexpr vt ROW_BAD = static_cast<vt>(~0);
            static constexpr vt ROW_FIRST = static_cast<vt>(0);

            explicit row_type(vt v) : m_t(v) {}
            row_type() : m_t(ROW_BAD) {
                assert(0);
                std::cout << "you prolly dont want this: row default construct makes an "
                             "invalid row!!"
                          << std::endl;
            }
            row_type& operator=(const vt v) {
                m_t = v;
                return *this;
            }

            bool is_valid() const { return m_t != ROW_BAD; }
			static row_type zero() { return row_type(0); }

            const vt& value() const { return m_t; }

            size_t to_int() { return static_cast<size_t>(m_t); }
            row_type& operator++() {
                m_t++;
                return *this;
            }
            row_type operator++(int) {
                const row_type temp = *this;
                ++*this;
                return temp;
            }
            bool operator<(const row_type& other) const { return m_t < other.m_t; }
            bool operator<(const vt& other) const { return m_t < other; }
            bool operator>(const row_type& other) const { return m_t > other.m_t; }
            bool operator>(const vt& other) const { return m_t > other; }
            bool operator==(const row_type& other) const { return m_t == other.m_t; }
            bool operator==(const vt& other) const { return m_t == other; }
            bool operator!=(const vt& other) const { return !(operator==(other)); }

            friend std::ostream& operator<<(std::ostream& os, const row_type& t) {
                os << t.m_t;
                return os;
            }
        };

        typedef row_type index_type;
        typedef index_type index_t;
        typedef row_type row_t;
        typedef uint32_t flag_t;
        namespace flags_options {
            constexpr flag_t NONE = 0;
            constexpr flag_t ERASED = 1;

        }  // namespace record_flags

        ///////////////////////////////////////////////////////////////////
        //////////////// packing area /////////////////////////////////////
#pragma pack(push, 1)
        struct record_info {
            flag_t flags{flags_options::NONE};
            sjk::timer_t date_created{0};
            sjk::timer_t date_modified{0};
        };  // struct record_info

        template <typename T>
        struct record {
            record_info info{};
            T m_data{};
            sjk::timer_t date_created() const { return info.date_created; }
            sjk::timer_t date_modified() const { return info.date_modified; }

        };  // struct record
#pragma pack(pop)
        ///////////////////////////////////////////////////////////////////
        //////////////// packing area ends ////////////////////////////////
        namespace {
            template <typename E>
            E increment_enum(E u, const int inc = 1) {
                using UT = typename std::underlying_type<E>::type;
                UT ut = static_cast<UT>(u);
                ut += inc;
                return static_cast<uid_t>(ut);
            }  // increment_enum()
        }  // namespace anon

        template <typename R>
        struct core {
            using record_type = R;
            typedef std::set<index_t> index_col_t;  // 2x quicker than sorted vec


            core(sjk::io::device& f) : m_f(f) {
                // allow for *not* opening right away, so views can be added
                // before population
                if (f.is_open()) {
                    open();
                }
            }
            virtual ~core() {}
            core(const core& other) = delete;
            core& operator=(const core& other) = delete;

            static index_t record_index_first() { return index_t::zero(); }

            int64_t record_count(bool include_erased = false) const {
                using namespace std;
                int64_t ret = m_index.size();
                if (include_erased)
                    ret += m_index_del.size();
                if (ret == 0) {
                    assert(m_f.is_open());
                    const size_t sz = m_f.size_bytes();
                    // cout << "record_count() ... file size: " << sz / 1024 << " kB." << endl;
                    constexpr size_t rsize = sizeof(R);
                    ret = sz / rsize;
                }
                return ret;
            }

            void clear() {
                m_index.clear();
                m_index_del.clear();
            }

            void close() {
                clear();
                m_f.close();
            }

            void open() {
                if (!m_f.is_open()) {
                    SJK_EXCEPTION("device is not open. open() it first.");
                }
                m_f.seek(0, std::ios_base::beg);
            }

            std::streamoff record_position(index_t idx) const {
                return sizeof(R) * idx.value();
            }

            void seek(std::streamoff where,
                      std::ios_base::seekdir whence = std::ios_base::beg) { m_f.seek(where, whence); }

            /*!
               * \brief update a record in the storage medium.
               * NOTE: The caller is responsible for making sure the device
               * is at the correct (and valid) position for writing the contents of r into
                           * backing storage
                           * You should make the correct index_t
                           * unless you are appending, in which case you can ignore it
               * Side-effects:
               * 1.)	Increases the write cursor by sizeof(record).
                                * 2.)  Ensures the record has an entry in m_index or m_index_del,
                                * as determined from the delete flag state.
                                * 3.) Sets a date_created on the record (if it is not already set)

                                *
                                * If something bad happens, an exception will be thrown.
               * \param r The data to write into the device.
               */
            void update(R& r, bool is_append, index_t idx = index_t(index_t::ROW_BAD)) {
                // preconditions_update(r, modifying_existing);
                if (r.date_created() == 0) {
                    r.info.date_created = sjk::now();
                }
                // m_f.seek(0, std::ios_base::end);<-- This makes us *very* slow, so push it
                // onto the caller.
				sjk::io::span_t span(r);
				const auto sz = span.size_bytes(); (void)sz;
				ASSERT(sz == sizeof(R));
                m_f.clear_errors();  // important: if a previous read took us to eos,
                // failbit etc will be set (even if the file is of zero size)

                if (is_append) {
                    idx = record_count(true);  // must do this before writing to get a correct record_count.
                } else {
                    assert(idx.is_valid());
                }
                const int64_t wr = m_f.write(span);
	
                if (wr != static_cast<int64_t>(span.size_bytes())) {
                    if (errno) {
                        SJK_EXCEPTION("Failed to update record: ", "\nError:", errno,
                                      strerror(errno));
                    } else {
                        SJK_EXCEPTION("Failed to update record: ");
                    }
                }

                assert(idx.is_valid());

                if (r.info.flags & flags_options::ERASED) {
                    add_index(m_index_del, m_index, idx);
                } else {
                    add_index(m_index, m_index_del, idx);
                }
            }

            /*!
               * \brief read_record reads the record at the current iodevice
   * position.
               * The caller is responsible for making sure the device is at a
   * valid and correct position.
               * Side-effect: advances the device read position by
   * sizeof(record).
               * \param r : receptacle for the record data from the iodevice
               * \return false if end of stream, or some exception is thrown.
               */
            inline bool read_record(R& r) {
				sjk::io::span_t span(r);
                const auto read = m_f.read(span);
                if (m_f.at_end()) {
                    return false;
                }
                if (read == static_cast<int64_t>(span.size_bytes())) {
                    return true;
                }
                SJK_EXCEPTION("unexpected (non-eof) when reading record");
            }

            /*/
            void read_all()
            {
                seek(0);
                R r;
                while (read_record(r))
                {
                    on_read_record(r);
                }
            }
            /*/

            //virtual int on_read_record(const R& r) = 0;

            std::streamoff file_size() const { return m_f.size_bytes(); }

            int64_t pos() const { return m_f.position(); }

            protected:
            sjk::io::device& m_f;
            /*!
   * \brief m_index : records that haven't been deleted.
   * Provided here for convenience, but your derived class
   * should decide how to manage this.
   */
            index_col_t m_index;
            /*!
   * \brief m_index_del : records that have been deleted.
   * Provided here for convenience, but your derived class
   * should decide how to manage this.
   */
            index_col_t m_index_del;

            void add_index(index_col_t& v, index_col_t& dwv, const index_t idx) {
                v.insert(idx);
                dwv.erase(idx);
            }

            /*!
   * \brief delete_record. Marks this record as erased in the storage.
   * The record will stay in the storage, but be marked as erased.
   *
   * \param r
   */
            void delete_record(index_t index) {
                // we *have* to read a record before we delete it,
                // since we are overwriting the whole record. If we
                // don't, and we undelete later, the record will be empty!
                const auto recpos = record_position(index);
                m_f.seek(recpos);
                const auto pos = m_f.position();

                R r;

                const bool ok = read_record(r);
                if (!ok) {
                    assert(ok);
                    SJK_EXCEPTION("cannot delete a record if I can't read it first.");
                }

                r.info.flags |= flags_options::ERASED;
                const auto sk = m_f.seek(pos);
                if (sk != pos) {
                    assert(sk == pos);
                    SJK_EXCEPTION(
                                "cannot delete a record if I can't seek to it, after "
                                "having read it.");
                }

                update(r, false, index);
            }
        };

    }  // namespace db

}  // namespace sjk

#endif  // SJK_DB_H
