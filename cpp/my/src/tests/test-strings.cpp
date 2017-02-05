// #include<vld.h>
#include <stdio.h>
#include <tchar.h>
#include "../../../../cpp/my/include/my_algorithms.h"
#include "../../../../cpp/my/include/my_tests.h"
#include "../../../../cpp/my/include/my_timing.h"

#include "../../../../cpp/my/include/my_string.h"
#include "../../../../cpp/my/include/my_string_helpers.h"
#include "../../../../cpp/my/include/my_allocator.h"
#include <vector>
#include <algorithm>
#include <string>
#include <limits>

using namespace my;
#pragma warning (disable : 26493)
MY_DECLARE_OUTSTREAMS()


typedef std::vector<char> charvec_t;
struct quickstring : public charvec_t
{
	quickstring(const char* s) { this->reserve(60);  append(s); }
	quickstring() { this->reserve(60); }

	void append(const char* s) {
		if (s) {
			auto oldsize = this->size();
			auto slen = strlen(s);
			this->resize(oldsize + slen + 1);
			memcpy(this->data() + oldsize, s, slen);
		}
	}
	const char* c_str() const { return this->data(); }
	friend outstream& operator<<(outstream& out, const quickstring& me) { out << me.c_str();	return out; }

};

using my::str;
using my::wstr;

template <typename T>
T test_move_constructor(T& t, timer_t& took, int n = 10000000)
{
	my::stopwatch sw(true);
	T ret;
	T prefix(t);
	std::vector<T> v; v.reserve(n);
	std::string tmp;
	static char buf[1024];
	auto l = strlen(prefix.c_str());
	strcpy(buf, prefix.c_str());

	for (int i = 0; i < n; ++i) {
		sprintf(buf, "%i", i);
		tmp = buf;
		t = tmp.c_str();
		ret = my::tests::test_move_constructor(t);
		v.push_back(ret);
	}
	took = sw.stop();
	cout << v.size() << endl;
	int x = (int)my::timeGetTime() % v.size();
	cout << v.at(x) << endl;
	return ret;
}

template <typename T>
T resize_behaviour(T& t)
{
	cout << "max_size() of string is: " << t.max_size() << endl;
	cout << "default capacity() of str is : " << t.capacity() << endl;
	t = "Hello, but this needs to be longer than the sso, which I think is 15 in a std::string";
	ASSERT(t == "Hello, but this needs to be longer than the sso, which I think is 15 in a std::string");
	t.resize(t.capacity() * 100);
	t.resize(1);
	t.resize(t.capacity() * 1000);
	ASSERT(t.substr(0, 1) == "H");
	ASSERT(t[0] == 'H');

	t.resize(500);
	ASSERT(t[0] == 'H');
	t = "Hello, you cunt.";
	T s = t.substr(0, 5);
	ASSERT(s == "Hello");
	s[0] = 'Y';
	ASSERT(s == "Yello");
	return t;
}

template <typename T>
T test_resize(T& s, timer_t& took, int n = 100000)
{
	int i = 0;
	my::stopwatch sw(true);
	while (i++ < n) {
		s.resize(i);
	}
	s = "resize test.";
	took = sw.stop();
	return s;
}

void timing() {
	
	struct result { result() : ms(0) {}timer_t ms; std::string desc; 
		bool operator<(const result& rhs) { 
			return ms < rhs.ms; 
		}
	};
	typedef std::vector<result> resultvec_t;
	typedef my::str str_t;
	typedef my::string_base<char, 20> str_small_t;
	std::string reserved; reserved.reserve(50);
	resultvec_t results;
	results.resize(4);
	results[0].desc = "using my::string  (sso = 20)                                ";
	results[1].desc = "using std::string                                           ";
	results[2].desc = "using regular my::string                                    ";
	results[3].desc = "quickstring                                                 ";

	
	/*/
	results[2].desc = "using std::string (2nd go)                                  ";
	results[3].desc = "using my::string  (2nd go) : sso size: " + std::to_string(str_t::sso_max_capacity()) + "                   ";
	results[4].desc = "using my string(small sso) : sso size: " + std::to_string(str_t::sso_max_capacity()) + "                   ";
	results[5].desc = "using my string(small sso) (2nd go) : sso size: " + std::to_string(str_small_t::sso_max_capacity()) + "          ";
	std::string huge_str(8192, 'a');
	my::str my_huge(huge_str.c_str());
	ASSERT(my_huge.size() == my::sz_t(huge_str.size()));
	results[6].desc = "using huge std::string                                      ";
	results[7].desc = "using huge my::string                                       ";
	results[8].desc = "using std::string, but reserve()d first                     ";

	

	str_small_t mystr_s	("Move constructor test for str_small_t");
	/*/
	int n = 0;
#ifdef _DEBUG
	n = 10000;
#else
	n = 1000000;
#endif
	str_small_t mystr_s("Move constructor test for str_small_t");
	std::string mh("my_huge"); std::string sh("std::huge");
	str_t   mystr("Move constructor test for  my::string");
	std::string s("Move constructor test for std::string");
	quickstring qs("Move constructor test for quickstring");
		
	cout << test_move_constructor(mystr_s, results[0].ms, n) << endl;
	cout << test_move_constructor(s, results[1].ms, n) << endl;
	cout << test_move_constructor(mystr, results[2].ms, n) << endl;
	cout << test_move_constructor(qs, results[3].ms, n) << endl;
	// cout << test_resize(reserved, results[4].ms, n) << endl;
	// cout << test_move_constructor(mystr_s, results[4].ms, n) << endl;
	
	/*/
	cout << test_move_constructor(s, results[2].ms, n) << endl;
	cout << test_move_constructor(mystr, results[3].ms, n) << endl;
	
	cout << test_move_constructor(mystr_s,  results[4].ms,n) << endl;
	cout << test_move_constructor(mystr_s,  results[5].ms, n) << endl;
	cout << test_move_constructor(mh, results[6].ms, n) << endl;
	cout << test_move_constructor(sh, results[7].ms, n) << endl;
	cout << test_resize(reserved, results[8].ms, n) << endl;
	cout << "\n\n";
	/*/
	cout << "--------------------------------- Summary follows: -------------------------------\n\n";
	for (auto i : results) {
		if (i.ms) {
			cout << "result of test: " << i.desc << " = " << i.ms << endl;
		}
	}
	cout << "\n--------------------------------- Summary complete. -------------------------------\n\n";
	std::sort(results.begin(), results.end());
	cout << "---------------------------- (sorted) Summary follows: ------------------------------\n\n";
	for (auto i : results) {
		if (i.ms) {
			cout << "result of test: " << i.desc << " = " << i.ms << endl;
		}
	}
	cout << "\n--------------------------------- Summary complete. -------------------------------\n\n";

	exit(0);
}


void test_correctness()
{
	wstr ws(L"Hello, you!");
	ASSERT(ws.size() == 11);
	auto sb = ws.toUTF8();
	ASSERT(sb == "Hello, you!" && "Wide string should say Hello, you!");
	ws.resize(1);
	ASSERT(ws.size() == 1 && "Wide string resized to 1, so now his size should be 1.");

	ASSERT(ws.toUTF8() == "H" && "Wide string should have exactly one character");
	ws.resize(4);
	ASSERT(ws.toUTF8() == "H" && "Made bigger to size 4, but should still be equal to 'H'.");
	ws.resize(8192);
	auto sz = ws.size(); (void)sz;
	ASSERT(sz == 8192);
	ASSERT(ws.toUTF8() == "H" && "Made bigger to size 8192, but should still be equal to 'H'.");
	ASSERT(ws.size() == 8192);
	ws.resize(28192);
	ASSERT(ws == L"H"); // although its bigger, equality for string should still be for L"H", because it should be full of nulls after the 'H'
	ASSERT(ws.size() == 28192 && "Unexpected size()");
	sb = ws.toUTF8();
	
	ASSERT(sb == "H" && "Bigger again, but should still be equal to 'H'.");
	/////////////////////////////////////////////////////////////////////////
	str s("Hello");
	s.resize(1);
	ASSERT(s == "H");
	s.resize(9);
	ASSERT(s == "H");
	cout << s << endl;
	s.resize(1024);
	ASSERT(s == "H");
	cout << s << endl;
	cout << ws.toUTF8() << endl;
	
	s = "One two test";
	str s2(s);// .toUTF8();
	
	ASSERT(s2 == "One two test");
	s2 += " Three four five six seven";
	ASSERT(s2 == "One two test Three four five six seven");
	ASSERT((size_t)s2.size() == strlen("One two test Three four five six seven"));
	s2 = s2.toUTF8();
	ASSERT(s2 == "One two test Three four five six seven");

	int i = 0;
	s2.clear();
	ASSERT(s2.empty());

	s2.shrink_to_fit();
	ASSERT(s2.size() == 0);
	auto capacity = (s2.sso_max_capacity());
	cout << "capacity: " << capacity << endl;

	while (i < 1000)
	{
		s2 += std::to_string(i);
		++i;
	}
	std::string stds;
	str mys;
	resize_behaviour(stds);
	resize_behaviour(mys);
}

void test_finding()
{
	str mys = "01234567890345";
	std::string s = mys.c_str();


	auto found = s.find('0', 1);
	my::find_t myfound = strings::find('0', mys, 1);
	ASSERT(found == myfound.to_size_t());
	
	found = s.find("345");
	myfound  = my::strings::find("345", mys);
	ASSERT(found == myfound.to_size_t());

	found = s.find("345", 10);
	myfound = my::strings::find("345", mys, 10);
	ASSERT(found == myfound.to_size_t());

	found = s.find("3987", 5, 1);
	myfound = my::strings::find(my::str("3987"), mys, 5, 1);
	ASSERT(found == myfound.to_size_t());

	std::string stdwithnulls;
	stdwithnulls.resize(100);
	stdwithnulls.append("hello");

	my::str mywithnulls;
	mywithnulls.resize(100);
	mywithnulls.append("hello");

	found = stdwithnulls.find(std::string("hello"));
	ASSERT(found == 100);
	my::str myfind("hello");
	my::strings::find(myfind, mywithnulls);
	ASSERT(found == myfound.to_size_t());

	found = s.find("noexists");
	myfound = strings::find("noexists", mys);
	auto myszt = myfound.to_size_t(); (void)myszt;
	ASSERT(found == myszt);
	ASSERT(myszt == std::string::npos);

	
	found = s.find('5');
	myfound = strings::find('5', mys);
	ASSERT(found == myfound.to_size_t());

	found = s.rfind("0");
	myfound = strings::rfind("0", mys);
	ASSERT(found == myfound.to_size_t());

}
/*/
void test_alloc()
{
	
	
	my::mem::alloc<char> a;
	char* p = a.begin();
	char* pfirst = nullptr;

	while (p < a.end()-2)
	{
		// ptrdiff_t distance = a.end() - p;
		p = a.next();
		*p = 'p';
		if (!pfirst) {
			pfirst = p;
		}
	}
	
	try {
		p = a.next();
	}
	catch (const std::bad_alloc& e)
	{
		cerr << e.what() << endl;
	}
	a.release(pfirst);

	p = a.next();
	ASSERT(p == a.begin());


}
/*/

template <typename T>
T create_strings(T& t, std::vector<T>& v,  timer_t& took, int n = 10000) {

	typedef std::vector<T> vec_t;
	cout << "Adding " << n << " items to a vector..." << endl;
	v.reserve(n);
	my::stopwatch sw(true);

		for (int i = 0; i < 100000; ++i) {
			std::string s = std::to_string(i);
			T ts(s.c_str());
			v.push_back(ts);
		}
		took = sw.stop();

	cout << "Added " << n << " items to a vector." << endl;
	t = v.at(101);

	return t;
}

template <typename T, typename V>
T test_sorting(T& t, V& v, bool use_builtin_sort, timer_t& took) {

	cout << "Sorting the vector ... " << endl;
	my::stopwatch sw(true);
	if (use_builtin_sort) {
		std::sort(v.begin(), v.end());
	}
	else {
		my::algo::quickSort(v.begin(), v.end());
	}
	took = sw.stop();
	t = v[101];
	return t;
}

template <typename T> void print_vec(const T& v) {
	cout << "---- vector contents ----" << endl;
	for (int i = 0; i < 11; ++i) {
		cout << v[i] << endl;
	}
	cout << "-------------------------" << endl;
}
void do_test_sort() {
	std::string s;
	s.reserve(80);
	typedef std::vector<std::string> vecstd_t;
	typedef std::vector < my::str > vecmy_t;
	timer_t took = 0;

	vecstd_t vecstd;
	vecmy_t vecmy;
	s = create_strings(s, vecstd, took);
	cout << "Took " << took << " ms to create a bunch of (std)strings." << endl;


	my::str mys;
	mys = create_strings(mys, vecmy, took);
	cout << "Took " << took << "ms to create a bunch of (my)strings.\n\n" << endl;
	
	const int ctr = 1;

	for (int i = 0; i < ctr; ++i) {
		cout << "Testing sort with std::string (std::sort) ..." << endl;
		test_sorting(s, vecstd, true, took);
		cout << "std::sort on std::string took " << took << " ms.\n" << endl;
		print_vec(vecstd);
	}

	for (int i = 0; i < ctr; ++i) {
		cout << "Testing sort with my::string (std::sort) ..." << endl;
		test_sorting(mys, vecmy, true, took);
		cout << "std::sort on my::string took " << took << " ms.\n" << endl;
		print_vec(vecmy);
	}

	for (int i = 0; i < ctr; ++i) {
		cout << "Testing sort again with std::string (quicksort) ..." << endl;
		test_sorting(s, vecstd, false, took);
		cout << "quickSort on std::string took " << took << " ms.\n" << endl;
		print_vec(vecstd);
	}

	for (int i = 0; i < ctr; ++i) {
		cout << "Testing sort again with my::string (quicksort) ..." << endl;
		test_sorting(mys, vecmy, false, took);
		cout << "quickSort on my::string took " << took << " ms.\n" << endl;
		print_vec(vecmy);
	}

}
int main()
{
	//test_correctness();
	// test_finding();
	timing();
	//do_test_sort();
	return 0;
	
}