// test-memory.cpp


#include "../../include/my_string.h"
#include "../../include/my_storage.h"

MY_DECLARE_OUTSTREAMS()

struct x {};
struct y {};

template <int i1, int i2>
void strapp(my::string<i1>& s1, const my::string<i2>& s2) {
	cout << "ffs";
	s1 += s2;
	cout << s1;
	ASSERT(s1 == "aaaaaaaaaaOnce upon a time, in a land far, far away ... The quick sso<20> string said hello");
	std::string sstring("\nHello from C++ string");
	cout << endl << endl;
	cout << sizeof(sstring) << endl;
	my::string<8> mys;
	cout << sizeof(mys) << endl;
	s1 += sstring;
	cout << s1;
}


void test_morph(my::string<10>&s1, my::string<20>&s2) {
	strapp(s1, s2);
}
void test_string_small_to_large()
{
	my::string<10> s('a', 10);
	auto sz = s.size();
	ASSERT(sz == 10);
	ASSERT(s == "aaaaaaaaaa");
	s.append("Once upon a time, in a land far, far away ...");
	ASSERT(s.size() == 55);
	auto cap = s.capacity();
	ASSERT(s == "aaaaaaaaaaOnce upon a time, in a land far, far away ...");
	ASSERT(cap == 84);
	sz = s.size();
	ASSERT(sz == 55);
	my::string<20> s2(" The quick sso<20> string said hello");
	test_morph(s, s2);

}

void test_string_large_to_small()
{
	my::string<1024> s('c', 1024);
	ASSERT(s.size() == 1024);
	s = "Much shorter";
	ASSERT(s == "Much shorter");

}

void test_string_small() {
	my::string<10> sfill('a', 10);
	ASSERT(sfill.size() == 10);
	ASSERT(sfill == "aaaaaaaaaa");

	my::string<16, 0> s;
	ASSERT(s.size() == 0);
	const char* const p = s.c_str();
	ASSERT(p != nullptr);
	ASSERT(*p == 0);
	s.append("Hello, world!");
	ASSERT(s.size() == 13);
	my::string<16,0> other(s);
	ASSERT(s == other);
	ASSERT(s.size() == 13);
	ASSERT(other.size() == 13);
	ASSERT(strcmp(s.c_str(), "Hello, world!") == 0);

}
int main()
{


	my::posint positive;
	cout << "positive maximum value = " << my::posint::maxi() << endl;
	cout << (my::posint::maxi() / my::storage::TERABYTE) << " TeraBytes." << endl;

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

	cout << "size in bytes: " << sp.size_in_bytes() << endl;
	cout << endl;

	test_string_small_to_large();
	test_string_large_to_small();
	test_string_small();
	
}

