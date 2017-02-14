// dbapp.cpp
#include "../../db/include/db-core.h"
#include <string>

#pragma pack(1)
struct mydata { char artist[100]; char title[100]; char album[100]; int64_t some_int; };
using entry_t = sjk::dbcore::entry<mydata>;
#pragma pack()

struct mycallback_t;
using core_t = sjk::dbcore::core<entry_t, mycallback_t>;

struct mycallback_t
{


	inline int pop_data(entry_t& entry, sjk::dbcore::columnindex_t idx)
	{
		return -77;
	}

	inline int pop_complete(const int64_t num_entries, core_t& db) {
		using std::cout; using std::endl;
		cout << "population complete, with " << num_entries << " entries." << endl;
		if (num_entries == 0) {
			auto& cols = db.columns();
			
		}
		return 0;
	}
};

int main()
{
	using std::cout; using std::endl; using std::fstream; using std::cerr;
	

	entry_t myentry;
	
	fstream f; f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		f.open("foo.db", std::ios::out | std::ios::binary);
		core_t db(f);
	}
	catch (const std::exception& e) {
		cerr << e.what() << endl;
		assert(f.is_open());
	}

	return 0;
}