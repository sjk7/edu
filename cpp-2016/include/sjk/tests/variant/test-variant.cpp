#include "../../sjk_terminal.h"
#include "../../sjk_variant.h"
#include <cassert>
#include <vector>
#include <iostream>

using namespace std;


static int assctr = 0;
#ifndef ASSERT
#define ASSERT(x) assctr++; assert(x);
#endif


using namespace sjk;

void test_empty(){
	var v;
	ASSERT(v.empty());
	std::string sfail;

	try {
		int x = v;
		ASSERT(x == 0);

	} catch (const std::exception& e) {
		sfail = e.what();

	}

	ASSERT(!sfail.empty());
	var v1 = v;
	ASSERT(v1.empty());


}

void test_scalar(){
	var vempty;
	std::string sty = vempty.id();
	ASSERT(sty.empty());

	var v = 1942;
	int i = v.cast<int>();
	ASSERT(i == 1942);

	std::string sfail;
	try {
		double d = v.cast(&d);
		ASSERT(d == 1942.0);
	} catch (const std::exception& e) {
		sfail = e.what();
		// cout << sfail << endl;
	}
	ASSERT(!sfail.empty()); sfail.clear();

	int& iref = i;
	var vref(iref);
	ASSERT(vref.cast<int>() == 1942);
	iref = 1943;
	ASSERT(vref.cast<int>() == 1942); // because it should be a copy, even for a reference
	vref = iref;
	iref = 1944;
	ASSERT(vref.cast<int>() == 1943); // because it should be a copy, even for a reference

	int& myref = vref.cast<int>();
	ASSERT(myref == 1943);
	myref = 1944;
	ASSERT(vref.cast<int>() == 1944); // you really do get a ref if you ask for it!


	/*/
	var vp(&iref); // wtf? a pointer? I made this so it will not compile, since var has value semantics
	int* z = vp.cast<int*>();
	ASSERT(*z == 1944);
	/*/
}

void test_udt(){
	struct udt{
		udt(int a = 0, int b = 0) : m_a(a), m_b(b){}
		int m_a;
		int m_b;
	};

	udt u(1,2);
	var v = u;
	ASSERT(!v.empty());
	ASSERT(v.cast<udt>().m_a == 1);
	ASSERT(v.cast<udt>().m_b == 2);

	typedef std::vector<udt> udtvec_t;
	udtvec_t udtvec;
	udtvec.push_back(u);
	udtvec.resize(100000);
	v = udtvec;

	ASSERT(v.cast<udtvec_t>().size() == 100000);
	ASSERT(v.cast<udtvec_t>().at(0).m_a == 1);
	ASSERT(v.cast<udtvec_t>().at(0).m_b == 2);
}

int main()
{
	test_udt();
	test_scalar();
	test_empty();

	var v(100);
	int& i = v.cast<int>();
	ASSERT(i == 100);
	i = 101;
	int x = v.cast<int>();
	ASSERT(x == 101);

	var v2 = 200;
	ASSERT(v2 == 200);
	typedef typename std::vector<std::string> svec_t;
	svec_t svec{"steve", "ria", "doog", "eb"};
	v = svec;
	svec_t& vecb = v.cast<svec_t>();
	ASSERT(vecb[0] == "steve");
	std::string sty = v.id();
	// cout << "vec string var type reported as: " << sty << endl;
	svec_t cpy = v.cast(&cpy);
	ASSERT(cpy.size() == 4);


	std::string s;
	try {
		int a = v.cast(&a);

	} catch (const std::exception& e) {
		s = e.what();
		// cout << s << endl;
	}

	cout << "test-variant: All tests passed. (" << assctr << " assertions.)" << endl;

	return 0;
}
