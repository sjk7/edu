#include "../../../cpp/my/include/my_string.h"

MY_DECLARE_OUTSTREAMS()

using namespace my;
my::string<10> hello();

int main()
{
	string<10> s("Hello");
	cout << s << endl;
	cout << "main.cpp: " << reinterpret_cast<void *>(&some_int) << endl;
	cout << hello() << endl;

}