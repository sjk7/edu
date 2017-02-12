#ifdef _WIN32
#	define _HAS_ITERATOR_DEBUGGING 0
#	define _SCL_SECURE 0
#endif


#include "../../sjk_terminal.h"
#include "sjk_mem.h"
#include "../../sjk_cfilestream.h"
#include "../../sjk_file.h"
#include "../../sjk_strings.h"
#include "../../../playout_lib/playout_db.h"
#include "../../sjk_variant.h"

using std::cout;
using std::endl;

using playout::db::mydb_t;
using dbptr_t = std::unique_ptr<mydb_t>;



void db_append_records(mydb_t& db, int64_t howmany = 10)
{
    using rec_t = playout::db::rec_t;
    using playout::strings;
    using playout::tones;

    sjk::timer t;
    db.seek(0, std::ios_base::end);
    rec_t r;
    std::string a("Artist field ");
    std::string tit("Title field ");

	// auto oldcount = db.record_count();
	auto c = db.record_count();
	howmany += c;

    std::string proper_long = " I don't want this string to be sso'd, thanks, and now its waaay too long to fit, with some more junk here.";

    // note: using a std::array<> instead here causes clang (3.8) to hang forever
    static sorted_vector<std::string> catnames
    { "A cat", "a cat", "b cat", "E cat", "D cat", "f cat", "G cat", "Z cat",
        ".catname-starts-withdot", "A", "C", "B", "TRANS", "XMAS", "LOVE", "JINX" };
	
	static bool b = false;
	if (!b)
	{
		catnames.sort();
		catnames.resize(strings::MAX_NUM_CATS);
		b = true;
	}
    ASSERT(catnames.size() == strings::MAX_NUM_CATS);

    while (c < howmany)
    {
        std::string num = std::to_string(c);

        std::string temp(num + " "); temp += a;
        if (c != 0) {
            temp += " I don't want this string to be sso'd, thanks.";
        }
        else {
            temp += proper_long;
        }

        if (c <= 1) {
            // this enables me to check I am properly null-terminating the string
            memset(&r.m_data.strvals.artist[0], 1, strings::MAX_STR);
        }
        size_t copied = sjk::str::cstring_from_string(&r.m_data.strvals.artist[0], strings::MAX_STR, temp);

        // always the length of the string, plus the terminating null, unless its as big as it can get.
        if (c == 0) {
            ASSERT(copied == strings::MAX_STR);
        }
        if (c == 1) {
            ASSERT(copied == temp.size() + 1);
        }

        if (c <= 1) {
            // check that I did, in fact, null terminate the string, both for the too long version and the normal case.
            char pc = *(&r.m_data.strvals.artist[0] + (copied - 1));
			ASSERT(pc == 0); (void)pc;
        }
        if (c == 0) {
            // because it *was* truncated
            std::string stest(&r.m_data.strvals.artist[0]);
            ASSERT(stest != temp);
            ASSERT(stest == temp.substr(0, strings::MAX_STR - 1));
        }
        if (c == 1) {
            std::string stest(&r.m_data.strvals.artist[0]);
            ASSERT(stest == temp); // because it wasn't truncated
        }


        temp = num + " "; temp += tit;
        copied = sjk::str::cstring_from_string(&r.m_data.strvals.title[0], strings::MAX_STR, temp);
        ASSERT(copied == temp.size() + 1 || copied == strings::MAX_STR);

        auto spi = span_from_tones(&r.m_data.tonevals.intros[0], tones::MAX_INTROS);
        auto sps = span_from_tones(&r.m_data.tonevals.sectones[0], tones::MAX_SECTONES);
		const size_t s = spi.size(); ASSERT(s == tones::MAX_INTROS); (void)s;

        for (uint32_t i = 0; i < tones::MAX_INTROS; i++) {

            if (c == 1000) {
                spi[i] = tones::value_type{ 1000 , 1001 };
                sps[i] = tones::value_type{ 1002, 1003 };

                if (i == tones::MAX_INTROS - 1) {
                    spi[i] = tones::value_type{ static_cast<uint32_t>(tones::NOT_SET), static_cast<uint32_t>(tones::NOT_SET) };
                    sps[i] = tones::value_type{ static_cast<uint32_t>(tones::NOT_SET), static_cast<uint32_t>(tones::NOT_SET) };
                }
            }
            else {
                spi[i] = tones::value_type{ i, i + 1 };
                sps[i] = tones::value_type{ i, i + 1 };

                if (i == tones::MAX_INTROS - 1) {
                    spi[i] = tones::value_type{ static_cast<uint32_t>(tones::NOT_SET), static_cast<uint32_t>(tones::NOT_SET) };
                    sps[i] = tones::value_type{ static_cast<uint32_t>(tones::NOT_SET), static_cast<uint32_t>(tones::NOT_SET) };
                }
            }
        }


        catnames[0] = num + " is the db idx as a catname";
		for (size_t i = 0; i < strings::MAX_NUM_CATS -1; i++)
        {
            auto& mya = r.m_data.strvals.categories[i];
            sjk::span<char> catspan(mya.data(), mya.size());
            auto mycopied = catspan.copy(catnames[i]);
			if (catnames[i].size()) {
				ASSERT(mycopied > 0); (void)mycopied;
			}
        }
	

        db.update(r, true);
        c++;
    }

    if (howmany) {
		auto cnow = db.record_count();
		ASSERT(cnow == howmany); (void)cnow;
        auto ms = t.stop();
        cout << "Adding " << howmany << " records took : " << ms << " ms. "
             << "(" << ms / 1000 << " secs.)" << endl << endl;
    }

}

using playout::db::icache;
using sjk::db::index_t;
void first_last(icache* p, std::string& first, std::string& last) {


	first = p->to_string(index_t(0));
	cout << "\nFirst record: " << first << "\n";
	last = std::string( p->to_string(index_t(p->size() - 1)) );
	cout << "Last record:  " << last << "\n\n";

}
void print_info(mydb_t& db){
    cout << "************* db info ***********";
    cout << "Records: " << db.record_count() << endl;
    
    using playout::db::icache;
    using sjk::sortable;
    using sjk::db::index_t;
    sjk::timer tmr;
	cout << "Columns info (" << db.caches().vec().size() << ") columns ..." << "\n";
	for (icache* p : db.caches().vec())
	{
		cout << p->name() << ", ";
	}
	cout << endl;
	const auto& vec = db.caches().vec();

    for (icache* p : vec){
        if (p->size() == 0){
            cout << p->name() << " is empty." << endl;
            continue;
        }
        cout << p->name() << ": size=" << p->size() <<
                " sorted by ascending index says (expecting true)" << p->is_sorted_by_index() << "\n\n";
		ASSERT(p->is_sorted_by_index());
        
		std::string first, last;
		first_last(p, first, last);

		tmr.start();
		cout << "Sorting " << p->name() << " by descending INDEX ..." << endl;
		p->sort(sortable::sortorder::desc, sortable::sortkind::index);
		tmr.stop();
		cout << "Sorting " << p->name() << " by descending INDEX took: " << tmr.stop() << " ms." << endl;
		std::string this_first, this_last;
		first_last(p, this_first, this_last);
		// All we've done is flipped the order. Here's the test for that:
		ASSERT(this_last == first);
		ASSERT(this_first == last);

        cout << "Sorting " << p->name() << " by descending VALUE..." << endl;
        tmr.start();
        p->sort(sortable::sortorder::desc, sortable::sortkind::value, sortable::sortflags::case_insens);
        cout << "Sorting the column '" << p->name() << "' took: " << tmr.stop() << " ms\n";

        ASSERT(!p->is_sorted_by_index());
		first_last(p, first, last);

		cout << "\n";
		cout << "Now sorting " << p->name() << " by ASCENDING VALUE ..." << endl;
		tmr.start();
		p->sort(sortable::sortorder::asc, sortable::sortkind::value, sortable::sortflags::case_insens);
		cout << "Sorting by asc value took: " << tmr.stop() << " ms." << endl;
		first_last(p, this_first, this_last);
		ASSERT(this_first == last);
		ASSERT(this_last == first);
		cout << "\n";

		tmr.start();
		cout << "Is the default sort any faster than the specialized string ones? ..." << endl;
		p->sort();
		cout << "Default ascending sort took: " << tmr.stop() << " ms." << endl;
		first_last(p, first, last);

        cout << "Now sorting " << p->name() << " by INDEX ascending\n";
        tmr.start();
        p->sort(sortable::sortorder::asc, sortable::sortkind::index);
        cout << "Took : "<< tmr.stop() << " ms.\n";
        ASSERT(p->is_sorted_by_index());
		first_last(p, first, last);
		cout << " --------------------------------------- \n";
		cout << " --------------------------------------- \n\n";
    }

}

void sort_tones_descending(mydb_t& db) {
	auto p = db.caches().map().at("sectones");
	using sjk::db::index_t;
	ASSERT(p);
	using sjk::sortable;
	p->sort(sortable::sortorder::desc, sortable::sortkind::value);
	cout << "First record: " << p->value(index_t(0)) << "\n\n";
	index_t idx = p->index_at(index_t(0));
	cout << "index with highest value is : " << idx << endl;
	ASSERT(idx == 1000);
}



struct myprogress : public sjk::progress::iprogress
{
	virtual int on_progress(const INT val, const INT max, const sjk::progress::state_t state , const char* const info) const {
		if (val > 0 && max > 0) {
			double pc = (double)val / (double)max;
			sjk::terminal::print_progress(pc);
		}
		if (state == sjk::progress::state_t::STARTING) {
			if (info) std::cout << info << std::endl;
		}else if (state == sjk::progress::state_t::FINISHED) {
			sjk::terminal::print_progress(1.0f);
			std::cout << std::endl << " Took " << this->taken() << " ms." << endl;
		}
		return 0;
	}
};

int main()
{
	sjk::cfile f;
	{
#ifdef NDEBUG
		bool wait_for_return_key = false;
#else
		bool wait_for_return_key = true;
#endif
		sjk::terminal term(wait_for_return_key);
		// mem();
#ifdef __linux
		std::string dbpath = "/home/steve/Desktop/V_DRIVE/STEVE/converted_from_dps.database";
#else
		std::string dbpath = "V:/STEVE/converted_from_dps.database";
#endif

#ifdef _WIN32
			std::string dps_lib_path = "V:\\STEVE\\MyDocs\\hugelib\\NewLib.dat";
			SetConsoleOutputCP(CP_UTF8);
#else
			std::string dps_lib_path = "../../Desktop/V_DRIVE/STEVE/MyDocs/hugelib/NewLib.dat";
#endif

		
		myprogress myprog;
		int64_t num_dps_records =
			playout::db::dps_import::import_dps(dbpath, dps_lib_path, &myprog);


		cout << "dps imported found: " << num_dps_records << " records." << endl;
		{
			
			bool existed = false;

			std::unique_ptr<mydb_t> pdb = playout::db::db_open(dbpath, f, existed, false);
			cout << "Number of records: " << pdb->record_count(false) << endl;
			cout << "Database file size is: " << (double)sjk::file::size(dbpath) / 1024.0 / 1024.0 << " MBytes." << endl;
			cout << endl;
			assert(num_dps_records == pdb->record_count(false));
			pdb->populate(&myprog);
			cout << "Number of records in the database : " << pdb->record_count() << endl;
			auto pcol = pdb->caches().vec().at(0);
			cout << "first column's name is: " << pcol->name() << endl;
			cout << "first column has " << pcol->size() << " rows." << endl;
			cout << "First value is: " ;
			cout << pcol->value(sjk::db::row_t(0)) << endl;
			cout << "Should be the same as...";
			cout << pcol->to_string(sjk::db::row_t(0)) << endl;

			db_append_records(*pdb);
			print_info(*pdb);

		} // db falls out of scope
	} // term falls out of scope
}


