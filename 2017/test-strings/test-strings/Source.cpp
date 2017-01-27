#include "../../../cpp/my/include/my_string.h"

using namespace my;

my::string<10> hello() {

	cout << "Source.cpp: " << reinterpret_cast<void *>(&some_int) << endl;
	return my::string<10>("Hello Source.cpp");
}