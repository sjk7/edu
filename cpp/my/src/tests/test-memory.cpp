// test-memory.cpp

#include "../../include/my.h"
#include "../../include/my_memory.h"
#include "../../include/my_storage.h"

using my::cout;
using my::cerr;
using my::endl;

struct x {};
struct y {};

int main()
{
	my::posint positive;
	cout << "positive maximum value = " << my::posint::maxi() << endl;
	cout << my::posint::maxi() / my::storage::TERABYTE << " TeraBytes." << endl;

	cout << "positive maximum value = " << my::posint::mini() << endl;
	cout << "positive=" << positive << endl;
	positive++;
	cout << "positive=" << positive << endl;
	positive--;
	cout << "positive=" << positive << endl;
	//positive--;
	//cout << "positive=" << positive << endl;

	char buf [1024];
	strcpy(buf, "Hello, you cunt");
	auto mylen = my::posint(strlen("Hello, you cunt")+ 1);
	my::span<char> sp(buf, mylen);
	ASSERT(sp.size() == mylen);
	ASSERT(sp.size() == 16);
	ASSERT(sp.capacity() == sp.size());
	for (const auto& c : sp)
	{
		cout << c;
		
	}
	cout << endl;
}

