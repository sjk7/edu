// db-test.cpp
#include "../include/db.h"
#include <iostream>
#include "../include/text_importer.h"
#include <cstdlib> // atod


using std::endl;
using std::cout;
using std::cerr;



void make_db(const char* filename)
{
	bool existed = true;
	cout << "Compiler version (__cplusplus): " << __cplusplus << endl;
	std::fstream f(filename , std::ios_base::binary | std::ios::in | std::ios::out);

	if (!f) {
		existed = false;
		f.open(filename, std::ios_base::binary | std::ios::out);
		f.close();
		f.open(filename, std::ios_base::binary | std::ios::in | std::ios::out);
		assert(f);
	}

	cpp::db::open_flags flags = cpp::db::open_flags::create;

	if (existed) {
		flags = cpp::db::open_flags::readwrite;
	}

	cpp::db::core db(f, flags);
	const auto& cols = db.columns();
	using namespace cpp::strings;
	auto colcount = cols.size();
	cout << "Database up and running, with " << colcount << " columns: \n"
		 << cols.serialize() << endl;
	cout << "row count is: " << db.rowcount() << endl;
	auto rowcount = db.rowcount();

	if (colcount == cpp::db::NUM_DEFAULT_COLUMNS) {
		db.column_add("artist", cpp::db::column_types::TYPE::STRING, 60);
		assert(cols[std::string("artist")].width() == 60);
		db.column_add("title", cpp::db::column_types::STRING, 60);
		db.column_add("album", cpp::db::column_types::STRING, 60);
		db.column_add("path", cpp::db::column_types::STRING, 256);
		db.column_add("duration", cpp::db::column_types::INT);
		colcount = cols.size();
		cout << "Columns added. Now " << colcount << " columns: \n"
			 << cols.serialize() << endl;
		assert(colcount == 8);
		db.save();
	} else {
		assert(colcount == 8);
		if (rowcount == 0){
			assert(cols[0].name() == "uid");
			db.add("Abba",
				   "The Winner Takes It All",
				   "Arrival",
				   "C:\\audio\\Abba - The Winner Takes It All.mp3",
				   234000);
			db.add("ZZ Top",
				   "Sharp Dressed Man",
				   "ZZ Top's Greatest Hits, Vol II",
				   "C:\\audio\\ZZ Top - Sharp Dressed Man.mp3",
				   295000);
			db.save();
		}
	}

	db.print(cout, -1);

	cout << endl;
}

static std::string dps_millis(const std::string& s)
{
	if (s.empty()) return std::string("0");
	if (cpp::strings::is_numeric(s))
	{
		double d = std::atof(s.c_str());
		d *= 1000;
		int i = static_cast<int32_t>((d + 0.5));
		return std::to_string(i);
	}
	return std::string("0");

}

static std::string  dps_opener(const std::string& s)
{
	static std::string TRUE("1");
	static std::string FALSE("0");
	if (s.empty()) return FALSE;

	static const std::string VBTRUE = std::string("True");
	if (s == VBTRUE){
		return TRUE;
	}
	return FALSE;
}

std::string dps_tab_delimit(const std::string& str, const std::string& old_d)
{
	static const std::string TAB("\t");
	std::string ret(str);

	cpp::strings::replace_all(ret, old_d, TAB);
	return ret;
}

void import(cpp::text_importer& imp, cpp::db::core& db)
{

	if (db.columns().size() <= 3){
		db.column_add("Artist");	// 0
		db.column_add("Title");		// 1
		db.column_add("Album");		// 2
		db.column_add("Path", cpp::db::column_types::STRING, 256 + 1);	// 3
		db.column_add("Dur", cpp::db::column_types::INT64);		// 4
		db.column_add("Cats", cpp::db::column_types::STRING, 256 + 1); // 5		// tab sep
		db.column_add("Lvls", cpp::db::column_types::STRING, 256 + 1); // 6	// tab sep
		db.column_add("Tones", cpp::db::column_types::STRING, 256 + 1);			 // 7	// tab sep
		db.save();
	}

	cpp::db::columns_t::fields_t v;
	v.resize(db.columns().size() - cpp::db::NUM_DEFAULT_COLUMNS);

	const auto& col = imp.m_col;
	if (col.size() == 0) return;
	auto n = static_cast<cpp::db::rowidx_t::impl>(col[0].m_data.size());
	auto r = 0;
	cpp::db::row_t rw;
	auto rows_needed = db.rowcount() + n;
	db.rowcount_set(cpp::db::rowidx_t(rows_needed));

	static const std::string FWD_SLASH("/");
	static const std::string TAB("\t");
	std::string path;

	while (r < n -1) // there's an empty line at the end.
	{
		rw = db.next_free_row();
		rw["Artist"] = col["Artist"].m_data[r];
		rw["Title"] = col["Title"].m_data[r];
		rw["Album"] = col["Album"].m_data[r];
		rw["Path"] = col["FilePath"].m_data[r];
		rw["Dur"] = dps_millis(col["Duration"].m_data[r]);
		rw["Cats"] = dps_tab_delimit(col["Category"].m_data[r], FWD_SLASH);
		rw["Lvls"] = col["CatLevel"].m_data[r];
		rw["Tones"] = dps_millis(col["CueIn"].m_data[r]) +
				TAB + dps_millis(col["Intro"].m_data[r]) +
				TAB + dps_millis(col["Outro"].m_data[r]) +
				TAB + dps_millis(col["DropStart"].m_data[r]) +
				TAB + dps_millis(col["Sectone"].m_data[r]) +
				TAB + dps_millis(col["Speed"].m_data[r])  +
				TAB + dps_millis(col["Tert"].m_data[r]) +
				TAB + col["Volume"].m_data[r] +
				TAB + dps_opener(col["Opener"].m_data[r]) ;

		db.row_commit(rw);
		++r;
	}

	db.rowcount_set(cpp::db::rowidx_t(r));
	db.save();

}


void dps_db()
{
#ifndef _WIN32
	const char* dps_import_filename = "/home/steve/DPSExport.txt";
	const char* dpsdb = "/home/steve/dps.db";
#else
	const char* dps_import_filename = "c:/users/steve/desktop/DPSExport.txt";
	const char* dpsdb = "c:/users/steve/desktop/dps.db";
#endif

	using dps_importer = cpp::text_importer;

	std::fstream f(dpsdb, std::ios_base::in | std::ios::out | std::ios_base::binary);
	cpp::db::open_flags flags = cpp::db::open_flags::readwrite;
	if (!f){
		f.open(dpsdb, std::ios_base::out | std::ios_base::binary);
		assert(f);
		f.close();
		f.open(dpsdb, std::ios_base::out | std::ios_base::in |std::ios_base::binary);
		flags = cpp::db::open_flags::create;
	}
	assert(f);
	cout << "Opening db " << dpsdb << " ..." << endl;
	cpp::db::core db(f, flags);
	cout << "DB open." << endl;

	if (db.rowcount() < 1000){
		cout << "Importing from DPSExport.txt ... " << endl;
		auto t1 = cpp::timer();
		dps_importer importer(dps_import_filename);
		import(importer, db);
		cout << "Import complete." << endl;
		auto t2 = cpp::timer();
		cout << "Import took " << t2 - t1 << " ms." << endl;
	}


	const auto& rows = db.rows();
	const auto& cols = db.columns();
	cout << "DB has " << rows.size() << " rows." << endl;
	cout << "DB has " << cols.size() << " columns." << endl;

	auto&  uid_col = cols["uid"];
	cout << "Uid column has " << uid_col.rowcount() << " rows" << endl;
	assert(uid_col.rowcount() == rows.size());
	const auto frow = cpp::db::rowidx_t(0);
	
	auto& artist_col = cols["Artist"];
	auto uid = artist_col.value_uid(frow);
	
	auto rw = rows.row_from_uid(uid);
	
	cout << "First artist has uid: " << artist_col.value_uid(frow) << endl;
	cout << "first row has Title: " << rw["Title"] << endl;
	cout << "First artist is: " << cols["Artist"].value_const2(frow) << endl << endl;

	auto t1 = cpp::timer();
	db.sort("Artist", cpp::sortorder::ASC);
	auto t2 = cpp::timer();
	
	cout << "Took " << t2 - t1 << " ms to sort artist ascending" << endl;
	uid = db.uid_from_index(frow);
	uid = artist_col.value_uid(frow);
	cout << "First artist has uid: " << uid << endl;
	rw = rows.row_from_uid(uid);
	cout << "first row has Title: " << rw["Title"] << endl;
	cout << "First artist is: " << rw["Artist"] << endl << endl;

	t1 = cpp::timer();
	db.sort("Artist", cpp::sortorder::ASC);
	t2 = cpp::timer();
	cout << "Took " << t2 - t1 << " ms to sort artist ascending (again)" << endl;
	cout << "First artist has uid: " <<   endl;
	rw = rows.row_from_uid(uid);
	cout << "first row has Title: " << rw["Title"] << endl;
	cout << "First artist is: " << artist_col.value_const2(frow) << endl << endl;

	t1 = cpp::timer();
	db.sort("Artist", cpp::sortorder::DESC);
	t2 = cpp::timer();

	cout << "Took " << t2 - t1 << " ms to sort artist descending." << endl;
	uid = artist_col.value_uid(frow);
	cout << "First artist has uid: " << uid  << endl;
	rw = rows.row_from_uid(uid);
	cout << "first row has Title: " << rw["Title"] << endl;
	cout << "First artist is: " << artist_col.value_const2(frow) << endl << endl;



	t1 = cpp::timer();
	db.sort("Artist", cpp::sortorder::DESC);
	t2 = cpp::timer();
	cout << "Took " << t2 - t1 << " ms to sort artist descending (again)." << endl;
	uid = artist_col.value_uid(frow);
	cout << "First artist has uid: " << uid << endl;
	rw = rows.row_from_uid(uid);
	cout << "first row has Title: " << rw["Title"] << endl;
	cout << "First artist is: " << artist_col.value_const2(frow) << endl << endl;


	t1 = cpp::timer();
	db.sort("Artist", cpp::sortorder::ASC);
	t2 = cpp::timer();
	cout << "Took " << t2 - t1 << " ms to sort artist ascending (again)" << endl;

	uid = artist_col.value_uid(frow);
	cout << "First artist has uid: " << uid << endl;
	rw = rows.row_from_uid(uid);
	cout << "first row has Title: " << rw["Title"] << endl;
	cout << "First artist is: " << artist_col.value_const2(frow) << endl << endl;


	std::string s;
	cout << "Hit return to quit." << endl;
	getline(std::cin, s);
	return;
//	db.print(cout, 20);
//	cout << endl;



}

int main()
{
	dps_db();
	

	//const char* filename = "/home/steve/testdb.db";


	/*/
	try {
		make_db(filename);
	} catch (const cpp::error& e) {
		if (e.code() == -EEXIST) {
			cerr << "Warning. deleting zero-byte file." << endl;
			::remove(filename);
			make_db(filename);
		} else {
			throw (e);
		}
	} catch (const std::exception& e) {
		throw (e);
	}
	/*/


} // int main
