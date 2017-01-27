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

namespace assert_ctr {
    uint64_t ctr = 0;
}

using playout::db::mydb_t;
using dbptr_t = std::unique_ptr<mydb_t>;

dbptr_t db_make(sjk::cfile& f, bool& existed, bool delete_first = true, std::string spath = sjk::file::tempfile("foo.database"))
{
    existed = false;


    if (sjk::file::exists(spath)) {
        auto sz = std::experimental::filesystem::file_size(spath);
        cout << "Existing file: " << spath << " has size: " << sz / 1024 << " kB" << endl;
        if (sz > 0) {
            existed = true;
            assert(existed);
        }
        if (delete_first) {
            sjk::file::delete_file(spath);
            existed = false;
        }
    }

    auto flags = sjk::file::flags_default_create;
    if (existed) {
        flags = sjk::file::flags_default;
    }

    f.open(spath, flags);
	if (delete_first){
		f.close();
		f.open(spath, sjk::file::flags_default);
	}
    cout << "Using file: " << spath << endl;

    // mydb_t db(f);
    return std::make_unique<mydb_t>(f);
}

void db_append_records(mydb_t& db, int64_t howmany = 10000)
{
    using rec_t = playout::db::rec_t;
    using playout::strings;
    using playout::tones;

    sjk::timer t;
    db.seek(0, std::ios_base::end);
    rec_t r;
    std::string a("Artist field ");
    std::string tit("Title field ");

	auto c = db.record_count();
    if (c){
        c -=1; // don't go negative when the db is empty!!
    }

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
        const size_t s = spi.size(); ASSERT(s == tones::MAX_INTROS);

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
        ASSERT(c == db.record_count());
        auto ms = t.stop();
        cout << "Adding " << howmany << " records took : " << ms << " ms. "
             << "(" << ms / 1000 << " secs.)" << endl << endl;
    }

}

void print_info(mydb_t& db){
    cout << "************* db info ***********";
    cout << "Records: " << db.record_count() << endl;
    cout << "Columns: " << "\n" ;
    using playout::db::icache;
    using sjk::sortable;
    using sjk::db::index_t;
    sjk::timer tmr;

    for (icache* p : db.caches().vec()){
        if (p->size() == 0){
            cout << p->name() << " is empty." << endl;
            continue;
        }
        cout << p->name() << ": size=" << p->size() <<
                " sorted by ascending index=" << p->is_sorted_by_index() << "\n";
        cout << "Please wait, sorting ..." <<"\n";
        cout << "First record: " << p->value(index_t(0)) << "\n";

        cout << "Sorting descending ..." << endl;
        tmr.start();
        p->sort(sortable::sortorder::desc, sortable::sortkind::value);

        cout << "Sorting the column took: " << tmr.stop() << " ms\n";
        cout << p->name() << ": size=" << p->size() <<
                " sorted by ascending index=" << p->is_sorted_by_index() << "\n";
        ASSERT(!p->is_sorted_by_index());
        cout << "First record: " << p->value(index_t(0)) << "\n\n";
        cout << "Now sorting " << p->name() << " by index ascending\n";
        tmr.start();
        p->sort(sortable::sortorder::asc, sortable::sortkind::index);
        cout << "Took : "<< tmr.stop() << " ms.\n";
        ASSERT(p->is_sorted_by_index());
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


int64_t  import_dps(const std::string& filepath, std::string dps_path = "")
{
	
	sjk::cfile f;
	assert(!filepath.empty());
	bool existed = false;
	std::unique_ptr<mydb_t> pdb = db_make(f, existed, true, filepath);

	cout << "Please wait, importing data from dps library ..." << endl;
	sjk::timer t;
	if (dps_path.empty()){
#ifdef _WIN32
	dps_path = "V:\\STEVE\\MyDocs\\hugelib\\NewLib.dat";
#else
	dps_path = "../../Desktop/V_DRIVE/STEVE/MyDocs/hugelib/NewLib.dat";
#endif
	}

	playout::db::dps_import::importer imp(dps_path);
	cout << "Getting data from the dps file took: " << t.stop() << " ms." << endl;
	cout << "Max Number of DPS records to import: " << imp.m_lib.m_vec.size() << endl;
	// mem();
	
	int64_t total = static_cast<int64_t>(imp.m_lib.m_vec.size());
	const auto& v = imp.m_lib.m_vec;
	int ctr = 0;
	int num_del = 0;
	using rec_t = playout::db::rec_t;
	using playout::strings;
	using playout::tones;
	rec_t r;
	sjk::db::index_t idx(0);
	t.start();
	pdb->seek(0, std::ios_base::end);

	for (const auto& ton : v)
	{
		if (ton.toninfo.markedaserased || ton.toninfo.ispopulated == 0) {
			num_del++;
		}
		else {
			playout::db::record_from_dps(ton, r);
			pdb->update(r, true, idx);
		}
		
		ctr++;
	}

	
	cout << "Took " << t.stop() << " ms to import a dps library." << endl;
	// mem();
	cout << "Imported " << pdb->record_count() << " records" << endl;
	cout << "DB file size is: " << pdb->file_size() / 1024 / 1024 << " MB." << endl;
	cout << "DPS library imported from had a file size of: " << imp.m_lib.m_f.size_bytes() / 1024 / 1024 << " MB." << endl;
	
	cout << endl <<  "Actual number of populated and unerased records is: " << total - num_del << endl;
	// cout << "filesize div sizeof records = " << pdb->file_size() / sizeof(r) << endl;
	return total - num_del;

}

/*/
int main()
{
	// sjk::terminal::test_progress();
	// sjk::terminal::test_progress2();

    {
		sjk::terminal term(false);
        sjk::cfile f;
        {
            sjk::timer t;
            bool existed = false;
            std::unique_ptr<mydb_t> pdb = db_make(f, existed, false);
            cout << "Took " << t.stop() << " ms to create a playout database." << endl;
            size_t to_add = 2000;

            if (existed) {
                to_add = 0;
            }
            mem();

            if (to_add) {
                cout << "You'll need to wait, adding " << to_add << " records ..." << endl;
                db_append_records(*pdb.get(), to_add);
            }

            cout << "File Size of " << pdb->file_size() / 1024 / 1024 << " MB." << endl;
            cout << "Please wait, populating db ..." << endl;
            pdb->populate();
            mem();

            cout << "File Size of " << pdb->file_size() / 1024 / 1024 << " MB." << endl;
            cout << "db has " << pdb->record_count() << " unerased records." << endl;

            print_info(*pdb);
			sort_tones_descending(*pdb);
        }
        cout << "-----------------------------" << endl;


    }
    mem();
    return 0;
	

}
/*/

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
		std::string spath = "/home/steve/Desktop/V_DRIVE/STEVE/converted_from_dps.database";
#else
		std::string spath = "V:/STEVE/converted_from_dps.database";
#endif

		int64_t num_dps_records = import_dps(spath);
		cout << "dps imported found: " << num_dps_records << " records." << endl;
		{
			sjk::timer t;
			bool existed = false;
			std::unique_ptr<mydb_t> pdb = db_make(f, existed, false, spath);
			cout << "Took " << t.stop() << " ms to create a playout database." << endl;
			cout << "Number of records: " << pdb->record_count(false) << endl;
			cout << "File size is: " << (double)sjk::file::size(spath) / 1024.0 / 1024.0 << " MBytes." << endl;
			cout << endl;
			assert(num_dps_records == pdb->record_count(false));
			pdb->populate();
			cout << "Number of records in the database : " << pdb->record_count() << endl;
			auto pcol = pdb->caches().vec().at(0);
			cout << "first column's name is: " << pcol->name() << endl;
			cout << "first column has " << pcol->size() << " rows." << endl;
			cout << "First value is: " ;
			cout << pcol->value(sjk::db::row_t(0)) << endl;
			cout << "Should be the same as...";
			cout << pcol->to_string(sjk::db::row_t(0)) << endl;

		} // db falls out of scope
	} // term falls out of scope
}


