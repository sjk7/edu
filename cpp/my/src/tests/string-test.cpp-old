// string-test.cpp
#		pragma warning(disable : 4464) // include path contains (..). AFAIC, this is a convenient feature, not a bug!
#include "../../include/my.h"
#include "../../include/my_tests.h"
#include "../../include/my_timing.h"
#include "../../include/my_string.h"

using my::cout;
using my::endl;

#include <string>
#include <vector>


std::vector<std::string> strv;
std::vector<my::string> mystrv;

template <typename T, typename C>
my::timer_t test_construction(T& t, const int n, const  T& init_val, my::stopwatch& sw, C& c)
{
	sw.start();
	int i = 0;
	while (i < n) {
		T t(1,(char)i);
		//c[i] = t;
		++i;
	}
	return sw.stop();
}

template <typename T>
my::timer_t test_moving(T& t, my::stopwatch& sw, int n = 100000000, int64_t sz = 1024)
{
	int i = 0;
	sw.start();
	T myt((T::size_type)sz, 'a');
	while (i < n)
	{
		my::tests::test_move_constructor(myt);
		++i;
	}
	t = myt;
	return sw.stop();
}

int main()
{

	my::stopwatch sw;


	auto ms1 = sw.stop();
	auto ms2 = ms1;

	auto tot1 = ms1;
	auto tot2 = ms1;
	double iter = 0.0;

	int sz = 1;
	for (int i = 0; i < 10; ++i) {
		
		iter++;
		ms1 = 0; ms2 = 0;
		{

			std::string stds(10, 'x');
			ms1 = test_moving(stds, sw, 10000, sz);
			cout << "stds size: " << stds.size() << endl;
			tot1 += ms1;
		}
		

		{
			my::string mys(10, 'x');
			ms2 = test_moving(mys, sw, 10000, sz);
			tot2 += ms2;
			cout << "my size: " << mys.size() << endl;
		}

		
		cout << "Results for size: " << sz << " ... " << endl;
		cout << "std::string took: " << ms1 << endl;
		cout << "my::string took : " << ms2 << endl << endl;
		
		
		sz *= 10;
	}

	sz = 1;
	for (int i = 0; i < 10; ++i) {

		iter++;
		ms1 = 0; ms2 = 0;

		{
			my::string mys(10, 'x');
			ms2 = test_moving(mys, sw, 10000, sz);
			cout << "my size: " << mys.size() << endl;
			tot2 += ms2;
		}

		{

			std::string stds(10, 'x');
			ms1 = test_moving(stds, sw, 10000, sz);
			cout << "stds size: " << stds.size() << endl;
			tot1 += ms1;
		}


		cout << "Results for size: " << sz << " ... " << endl;
		cout << "std::string took: " << ms1 << endl;
		cout << "my::string took : " << ms2 << endl << endl;


		sz *= 10;
	}

	double d1 = tot1 / (iter);
	double d2 = tot2 / (iter);

	cout << "Total time (std::string) : " << tot1 << " ms." << endl;
	cout << "Total time  (my::string) : " << tot2 << " ms." << endl;
	cout << " --------------------------------------------------- " << endl;
	
	cout << "Avg time (std::string)   : " << d1 << " ms." << endl;
	cout << "Avg time (my::string)    : " << d2 << " ms." << endl;
	cout << " --------------------------------------------------- " << endl;
	return 0;

	/*/
	
	const int n = 1000000;
	strv.resize(n);
	mystrv.resize(n);

	auto ms1 = test_construction(stds, n, std::string("test"), sw, strv);
	auto ms2 = test_construction(mys, n, my::string("test"), sw, mystrv);

	cout << "std::string = " << ms1 << endl;
	cout << "my::string = " << ms2 << endl;

	cout << g_ar[100] << endl;
	cout << g_ar2[100] << endl;
	/*/
	return 0;
}