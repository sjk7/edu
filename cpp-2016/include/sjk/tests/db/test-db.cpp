/*/
#include "../../sjk_terminal.h"
#include "../../sjk_record_set.h"
#include "../../sjk_timing.h"
#include "../../sjk_file.h"
#include "../../sjk_variant.h"
#include <cassert>
#include <vector>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>


using namespace std;



static uint64_t assctr = 0;
#ifndef ASSERT
#define ASSERT(x) assctr++; assert(x);
#endif

//////////////////////////////////
#pragma pack(push, 1)
struct rec
{
	char artist [100] = {0};
	char title  [100] = {0};
};
#pragma pack(pop)
/////////////////////////////////

typedef sjk::db::record<rec> rec_t;
typedef sjk::db::core<rec_t> mydb_t;


void read_all(mydb_t& mydb)
{
	rec_t r;
	auto ctr = mydb.record_count();
	ctr = 0;
	// cout << mydb.file_size() << endl;
	sjk::db::index_t idx = sjk::db::index_t::IDX_FIRST;
	auto begin = mydb.record_position(idx);
	mydb.seek(begin);
	while (mydb.read_record(r))
	{
		ctr++;
		ASSERT(r.date_created() != 0);
		ASSERT(r.date_modified() == 0);
		ASSERT(r.uid() != sjk::db::uid_type::UID_BAD);
	}

	uint32_t rc = static_cast<uint32_t>(mydb.record_count());
	(void)rc;
	using namespace sjk::db::rel_ops;
	ASSERT(r.uid() == rc);
	ASSERT(rc == r.uid());
	ASSERT(ctr == rc); // we should've read them all
}
void test_bad_uid_insert(mydb_t& mydb)
{
	rec_t r;
	r.info.uid = static_cast<sjk::db::uid_t>(1);
	int fail = 0;
	(void) fail;
	try {
		mydb.update(r, false);
	} catch (const std::exception& e) { (void)e;
		//cout << "Caught (expected) exception when adding existing uid" << endl;
		//cout << e.what() << endl;
		fail = 1;
	}

	ASSERT(fail);
}
void test_delete(mydb_t& mydb)
{
	rec_t r;
	int fail = 0;
	(void) fail;
	try {
		mydb.delete_record(r);
	} catch (const std::exception& e) {
		cout << "Caught (expected) exception when trying to delete a bad uid'd record ..." << endl;
		cout << e.what() << endl;
		fail = 1;
	}
	assert(fail); fail = 0;

	r.info.uid = sjk::db::uid_t::UID_FIRST;
	mydb.delete_record(r);
	ASSERT(r.info.flags & sjk::db::record_flags::ERASED);

	rec_t r2;
	auto idx = mydb.uid_to_index(r.info.uid);
	ASSERT(idx != sjk::db::index_t::IDX_BAD);
	auto pos = mydb.record_position(idx);
	ASSERT(pos == 0); // its the first record we are deleting. Needs changing once we write a header
	mydb.seek(pos);
	ASSERT(mydb.pos() == pos);
	bool b = mydb.read_record(r2);
	ASSERT(b); (void) b;
	ASSERT(r2.info.flags & sjk::db::record_flags::ERASED);

}

void do_test(bool erase_first, bool do_test_delete){

	if (erase_first){
		remove(fname());
	}
	sjk::file f;
	try {
		f.open(fname(), std::ios::binary | std::ios::out | std::ios::in);
	} catch (const std::exception& e) {
		cout << "Handled (expected) exception: " << e.what() << endl;
		// create the file if it doesn't exist
		f.open(fname(), std::ios::binary | std::ios::out);
		ASSERT(f.is_open());
		f.close();
		f.open(fname(), std::ios::binary | std::ios::in | std::ios::out);
	}
	ASSERT(f.is_open());

	sjk::timer t;
	mydb_t mydb(f);
	cout << "population of " << mydb.record_count() << " records took: " << t.stop() << " ms." << endl;
	
	if (mydb.record_count() == 0)
	{
		auto ctr = 0; rec_t r;
		while (ctr < 100000){
			r.info.uid = sjk::db::uid_type::UID_BAD;
			r.info.date_created = 0; // ^^ setting these two fields gets a timestamp and uid automatically
			mydb.update(r, false);
			ctr++;
		}
	}
	test_bad_uid_insert(mydb);
	read_all(mydb);

	if (do_test_delete){
		test_delete(mydb);
	}

}

void do_update_all()
{
	sjk::file f;
	f.open(fname());
	mydb_t db(f);
	rec_t r;
	uint32_t i = 0;
	cout << "Updating every single record (might take a while) ..." << endl;

	const std::string stit = "Title: ";

	while (db.read_record(r)){
		std::string s = std::to_string(i);
		strcpy(r.m_data.artist, s.c_str());
		std::string tit(stit);
		tit += s;
		strcpy(r.m_data.title,tit.c_str());
		auto where = db.record_position(i);
		db.seek(where);
		db.update(r, true);
		i++;
	}

	ASSERT(i == db.record_count()); // Did we do all records?
	db.close();
	f.open(fname());
	db.open(f);

	int x = 0;
	while (db.read_record(r))
	{
		std::string s = r.m_data.artist;
		ASSERT(!s.empty());
		ASSERT(stoi(s.c_str()) == x);
		++x;
	}

	 // Did we do all records?
	ASSERT(static_cast<size_t>(x) == db.record_count());


}

static uint32_t m_cbcount = 0;
struct mycallback : sjk::db::notifier<rec_t>
{
	mycallback(){
		cout << "mycallback constructor " << endl;
	}
	virtual ~mycallback(){}
	
	virtual void prepopulate(const size_t) override{}

	virtual int on_record_populate(rec_t&) override {
		m_cbcount++;
		return 0;
	}
};

// test open() with a callback for every record read
void do_open_callback()
{
	sjk::file f(fname());
	sjk::timer t;
	mycallback mycb;
	mydb_t db(f, &mycb);
	
	assert(f.is_open());
	assert(m_cbcount == db.record_count());
	cout << "population of " << m_cbcount << " records took: " << t.stop() << " ms." << endl;
}
using namespace sjk::db;

struct artpop_t : public populator<std::string, rec_t, artpop_t>
{
	using pop_t = populator<std::string, rec_t, artpop_t>;
	artpop_t(const std::string& name) : pop_t(name){}
	protected:
	virtual void populate(rec_t& r) override{
		m_vec_values.push_back(r.m_data.artist);
	}
	
};
struct titpop_t : public populator<std::string, rec_t, titpop_t>
{
	using pop_t = populator<std::string, rec_t, titpop_t>;
	titpop_t(const std::string& name) : pop_t(name){}
	protected:
	virtual void populate(rec_t& r) override{
		m_vec_values.push_back(r.m_data.title);
	}

};

void test_record_set()
{
	sjk::file f;
	sjk::timer t;
	cout << endl <<  "Now populating a record set ..." << endl;
	typedef sjk::db::core<rec_t> mydb_t;
	typedef record_set<mydb_t> rs_t;
	mydb_t db(f);
	rs_t rs(db);
	artpop_t apt("artist");
	titpop_t tpt("title");

	rs.column_add(apt);
	rs.column_add(tpt);
	f.open(fname());
	db.open(f);
	ASSERT(rs.row_count() == db.record_count());
	cout <<  "populating the record set, with "<<
		rs.columns().size() << " columns, " << "took: " << t.stop() << " ms." << endl;
	cout << endl << "Some artist values: " << endl;
	
	auto pcol = rs.columns().at(0);
	sjk::db::row_t rw;
	int i = 0;
	auto cnt = pcol->row_count();
	
	cout << "Now iterating over the actual values ... " << endl;
	
	typedef column<std::string, rec_t, artpop_t> art_col_t;
	typedef column<std::string, rec_t, titpop_t> tit_col_t;
	
	t.start();
	art_col_t* pc = dynamic_cast<art_col_t*>(pcol);
	assert(pcol);
	cnt = pc->row_count();
	rw = 0; i = 0;
	while (rw < cnt){
		const std::string& s = pc->value(rw);
		if (i % 5000 == 0) {
			cout << i << ":" << "\"" << s << "\"" << ",";
		}
		++i;
		rw++;
	}
	cout << endl << "iteration over *actual* values took: " << t.stop() << " ms." << endl;
	cout << endl << "iterating artist column ..." << endl;
	t.start();
	std::string s; rw = 0; i = 0;
	while (rw < cnt){
		s = pcol->value_string(rw);
		if (i % 5000 == 0) {
			cout << i << ":" << "\"" << s << "\"" << ",";
		}
		++i;
		rw++;
	}
	ASSERT(cnt == i);
	cout << endl << "iteration over stringized values took: " << t.stop() << " ms." << endl;
	cout << endl << "Let's try a sort ..." << endl;
	t.start();
	pc->sort(std::greater<std::string>());
	cout << "sorting those " << cnt.value() << " values (descending) took: " << t.stop() << " ms." << endl;
	rw = 0;
	cout << "Now, the first artist value is: " << pc->value(rw) << endl;

	icolumn<rec_t>* pcolt = rs.columns().at(1);
	tit_col_t* pct = dynamic_cast<tit_col_t*>(pcolt);
	assert(pct);

	cout << endl << "Let's sort the titles (ascending) ... " << endl;
	t.start();
	pct->sort();
	cout << "sorting titles took: " << t.stop() << " ms." << endl;
	cout << "And, the first title value is: " << pct->value(rw) << endl;
	
	
}

int main()
{
	auto tmr = sjk::timer();

	do_test(true, false);
	do_test(false, true);
	do_test(false, false);
	do_update_all();
	do_open_callback();
	test_record_set();


	auto took = tmr.stop();
	cout << "High res is steady? " << std::boolalpha << sjk::detail::highres_is_steady() << endl;
	cout << "Program execution took: " << took << " milliseconds" << std::endl;
	cout << "Assertions passed: " << assctr << endl << endl;
	// sjk::terminal::done();
	return 0;
}
/*/
