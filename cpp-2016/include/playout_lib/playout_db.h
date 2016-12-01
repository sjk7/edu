#ifndef PLAYOUT_DB_H
#define PLAYOUT_DB_H

#include "pl_types.h"
#include "../dps/dps_lib.h"

namespace playout
{
    namespace db
    {
        typedef sjk::db::record<rec> rec_t;
        typedef sjk::db::icache icache;

        struct mydb_t : public sjk::db::core<rec_t>
        {

            using base = sjk::db::core<rec_t>;
            using icache_t = icache;
            using idx_t = sjk::db::index_t;

            typedef std::pair<std::string, idx_t > strpair_t;
            typedef std::pair<sjk::timer_t, idx_t > timepair_t;
            typedef std::pair<tones::value_type, idx_t > tonepair_t;
            typedef std::pair< std::vector<std::string> , idx_t > strvecpair_t;

            static inline void make_multi_str_pair()
            {

            }

            typedef std::vector<icache_t*> cachevec_t;
            typedef sjk::collections::name_values<icache_t*> cache_coll_t;
            using row_t = sjk::db::row_t;
            enum class state{ none, deleting };
            state m_state;

            mydb_t(sjk::io::device& d) : base(d), m_state(state::none), m_artists("artist"),
                m_titles("title"), m_paths("paths"), m_dates_created("dates_created"),
                m_intros("intros"),m_sectones("sectones") , m_cats("category")
            {
                m_caches.push_back(&m_artists);
                m_caches.push_back(&m_titles);
                m_caches.push_back(&m_dates_created);
                m_caches.push_back(&m_intros);
                m_caches.push_back(&m_sectones);
                m_caches.push_back(&m_cats);
                m_caches.push_back(&m_paths);
            }

            mydb_t(const mydb_t& other) = delete;
            mydb_t& operator=(const mydb_t& other) = delete;

            virtual ~mydb_t(){}

            void reserve(const int64_t sz){
                for (auto p : m_caches){
                    p->reserve((sz));
                }
            }
            void clear(){
                for (auto p : m_caches){
                    p->clear();
                }
                base::clear();
                m_state = state::none;
            }


            template <typename V>
            static inline void pop_multicache(V& v,
                                              sjk::span<tones::value_type>& sp)
            {
                v.clear();

                for (size_t i = 0; i < tones::MAX_INTROS; i++) {
                    if (sp[i].m_head == TONE_NOT_SET) break;
                    v.push_back(sp[i]);
                }

            }

            void populate()
            {
                sjk::timer t;
                rec_t r;
                int64_t ctr = 0;
                seek(record_position(base::record_index_first()));

                auto rc = record_count();

                const size_t sz = sizeof(r);
                if (m_f.size_bytes() % sz != 0){
                    SJK_EXCEPTION("playoutdb: size mismatch for file: ", m_f.path());
                }

                clear();
                reserve(rc);
                auto& artvec = m_artists.values();
                auto& titvec = m_titles.values();
                auto& datevec = m_dates_created.values();
                auto& introvec = m_intros.values();
                auto& secvec = m_sectones.values();
                auto& catvec = m_cats.values();

                const auto cap = static_cast<int64_t>(catvec.capacity());
                ASSERT(cap >= rc); // because reserve() should have done this

                typedef typename std::vector<tonepair_t::first_type> v_t; v_t v;

                v.reserve(tones::MAX_INTROS);

                using svec = sorted_vector<std::string>;
                svec tempcats;
                tempcats.reserve(strings::MAX_NUM_CATS);
                const std::string empty_string{};
                const svec empty_cat_vec{};

                while (read_record(r))
                {
                    idx_t idx(ctr);
                    if (!(r.info.flags & sjk::db::flags_options::ERASED)){
                        artvec.emplace_back(strpair_t(r.m_data.strvals.artist, idx));
                        titvec.emplace_back(strpair_t(r.m_data.strvals.title, idx));
                        datevec.emplace_back(timepair_t(r.date_created(), idx));

                        if (ctr < 100){
                            strings::pop_cats_from_record(r, idx, catvec, tempcats);
                        }else{
                            catvec.emplace_back(std::make_pair(empty_cat_vec, idx));
                        }

                        sjk::span<tones::value_type> spintros(&r.m_data.tonevals.intros[0], tones::MAX_INTROS);
                        pop_multicache(v, spintros);
                        introvec.push_back(std::make_pair(v, idx));
                        sjk::span<tones::value_type> spsectones(&r.m_data.tonevals.sectones[0], tones::MAX_SECTONES);
                        pop_multicache(v, spsectones);
                        secvec.push_back(std::make_pair(v, idx));

                        m_index.insert(sjk::db::index_t(ctr));
                    }else{
                        m_index_del.insert(sjk::db::index_t(ctr));
                    }
                    ctr++;
                }; // while read_record(r)

                auto recs = record_count(); (void)recs;
                ASSERT(ctr == recs);
                ASSERT(artvec.size() == static_cast<size_t>(ctr) );
                ASSERT(titvec.size() == static_cast<size_t>(ctr));
                ASSERT(datevec.size() ==  static_cast<size_t>(ctr));

                std::cout << "population took: " << t.stop() << " ms." << std::endl;
            }

            sjk::var value(const std::string& cacheid,  const row_t rw)
            {
                auto x = m_caches.from_key(cacheid);
                return x->value(rw);
            }

            icache* cache(const std::string& name){
                return m_caches.from_key(name);
            }

            icache* cache(const size_t index) {
                return m_caches.from_index(index);
            }

            const cache_coll_t& caches() const {
                return m_caches;
            }


            struct record_eraser
            {
                record_eraser(mydb_t& db) : m_db(db), m_deleted(0) {
                    if (m_db.m_state != state::none) {
                        SJK_EXCEPTION("It is an error to try to erase records on a busy database");
                    }
                    m_db.delete_prepare();
                }
                ~record_eraser() { m_db.delete_complete(m_deleted); }

                bool delete_record(const idx_t idx) {
                    m_deleted++;
                    return m_db.delete_record(idx);
                }
                private:
                mydb_t& m_db;
                size_t m_deleted;
            };

            protected:
            cache_coll_t m_caches;
            sjk::db::cache_t<std::string> m_artists;
            sjk::db::cache_t<std::string> m_titles;
            sjk::db::cache_t<std::string> m_paths;
            sjk::db::cache_t<sjk::timer_t> m_dates_created;
            sjk::db::multicache_t<tones::value_type> m_intros;
            sjk::db::multicache_t<tones::value_type> m_sectones;
            sjk::db::multicache_t<std::string> m_cats;

            // ---------- NB: All delete functions are protected because you
            // ---------- need to use a record_eraser()
            /*!
                                 * \brief delete_complete
                                 * \param expected : how many records you expect to delete.
                                 */
            void delete_complete(const int64_t expected) {

                (void)expected;
                for (auto p : m_caches) {
                    auto num = p->delete_erased(); (void)num;
                    if (p->size()) {
                        ASSERT(num == expected);
                    }
                }
                m_state = state::none;
            }

            void delete_prepare()
            {
                for (auto p : m_caches) {
                    if (!p->is_sorted_by_index()) {
                        p->sort(sortable::sortorder::asc, sortable::sortkind::index);
                    }
                }
                m_state = state::deleting;
            }

            bool delete_record(const sjk::db::index_t idx) {
                ASSERT(m_state == state::deleting);
                base::delete_record(idx);
                size_t ok = 0;
                for (auto p : m_caches) {
                    bool er = p->mark_erased(idx); (void)er;
                    ASSERT(er);
                    const auto sz = p->size();
                    if (sz) {
                        ASSERT(er);
                    }
                    ok++;
                }
                if (ok == m_caches.size()) {
                    return true;
                }
                return false;
            }
        }; // mydb_t

        static inline void cats_from_vector(const sjk::str::vec_t& cats, playout::strings& strs) {
                using namespace sjk::str;
                size_t i = 0;
                auto myset = dps::cat_set_from_vec(cats);
                for (const auto& catname : myset)
                {
                        if (i >= strings::MAX_NUM_CATS - 1) break; // hit category limit
                        cstring_from_string(&strs.categories[i][0], strings::MAX_CAT_LEN, catname);
                        ++i;
                }
        }

        static inline int year_sanitize(int& yr)
        {
            if (yr < 1900 || yr > 2100){
                yr = 0;
            }
            return yr;
        }

        static inline void record_from_dps_strings(const dps::TFIXEDTONINFOEX& ton, rec_t& r)
        {
            const auto& t = ton.toninfo;
            auto& d = r.m_data;

            using namespace dps;
            using namespace sjk::str;
            r.info.date_created = 0; r.info.date_modified = 0;

            auto& strings = d.strvals;

            string_from_dps(t.artist, dps::CAT_MAX, strings.artist, strings::MAX_STR);
            string_from_dps(t.title, dps::CAT_MAX, strings.title, strings::MAX_STR);
            string_from_dps(ton.album, dps::ALB_MAX, strings.album, strings::MAX_STR);

            std::string tmp;
            string_from_cstring(t.filepath, dps::Max_Path, tmp);
            tmp = rtrim(tmp);
            if (tmp.find("://") != std::string::npos){
                std::cout << "this is a stream: " << tmp << std::endl;
            }
            tmp = sjk::str::xplat_file_path(tmp);
            string_from_dps(tmp.data(), tmp.size(), strings.path, strings::MAX_FILE_PATH);

            string_from_cstring(t.category, dps::CAT_MAX, tmp);
            tmp = rtrim(tmp);
            vec_t cats = cats_from_string(tmp);
            cats_from_vector(cats, r.m_data.strvals);

            string_from_cstring(t.year, 4, tmp);
            try{
                sjk::str::rtrim(tmp);
                auto yr = std::stoi(tmp);
                year_sanitize(yr);
                r.m_data.year = static_cast<uint16_t>(yr);
            }catch(...)
            {
                r.m_data.year = 0;

            }

            d.strvals.opener[0] = t.opener != 0;
        }

        tones::type duration_sanitize(double ddur){
            if (ddur < -1){
                return tones::STREAM_DURATION;
            }
            tones::type ret = static_cast<tones::type>(ddur * 1000);
            return ret;
        }

        tones::VOLUME_TYPE volume_sanitize(tones::VOLUME_TYPE dpsvol){
            tones::VOLUME_TYPE vol = dpsvol;
            if (vol == -1) vol = tones::VOLUME_NOT_SET;
            if (vol > 0) vol = tones::VOLUME_NOT_SET;
            if (vol < tones::VOLUME_MIN) vol = tones::VOLUME_MIN;
            return vol;
        }

        tones::type cue_in_sanitize(double dps_cuein)
        {
            if (dps_cuein <= 0){
                return tones::NOT_SET;
            }
            return static_cast<tones::type>(dps_cuein * 1000.0);
        }

        tones::type sectone_sanitize(double dur, double sec)
        {
            if (sec >= dur){
                return tones::NOT_SET;
            }
            return static_cast<tones::type>(sec * 1000.0);
        }

        tones::type intro_sanitize(double intro)
        {
            if (intro <= 0){
                return tones::NOT_SET;
            }
            // TODO: check intro in relation to cuein as far as DPS was concerned.
            return static_cast<tones::type>(intro * 1000.0);
        }

        tones::SPEED_TYPE speed_sanitize(double dps_speed)
        {
            int i = static_cast<int>(dps_speed * 100.0);
            if (i == 0 || i == 100){
                return tones::SPEED_NOT_SET;
            }else{
                assert(i <= std::numeric_limits<tones::SPEED_TYPE>::max());
                assert(i >= std::numeric_limits<tones::SPEED_TYPE>::min());
                return static_cast<tones::SPEED_TYPE>(i);
            }
        }

        static inline void record_from_dps(const dps::TFIXEDTONINFOEX& ton, rec_t& r)
        {
            record_from_dps_strings(ton, r);
            auto& dps = ton.toninfo;
            auto& mytones = r.m_data.tonevals;
            mytones.volume = volume_sanitize(dps.volume);
            mytones.duration = duration_sanitize(dps.duration);
            mytones.cue_in = cue_in_sanitize(dps.cuein);


            mytones.intros[0].m_head = intro_sanitize(dps.intro);
            mytones.intros[0].m_tail = mytones.intros[0].m_head;

            mytones.sectones[0].m_head = sectone_sanitize(dps.duration, dps.sectone);
            mytones.sectones[0].m_tail = mytones.sectones[0].m_head;

            mytones.speed = speed_sanitize(dps.speed);
        }

        namespace dps_import
        {
            struct importer
            {
                dps::dps_lib m_lib;
                importer(std::string path) : m_lib(path){
                    // all the work is done in the constructor.
                    // Now, if there was no exception, we have
                    // all the dps data in a vector
                }
            };

        } // namespace dps_import

    } // namespace db
} // namespace playout

#endif // PLAYOUT_DB_H
