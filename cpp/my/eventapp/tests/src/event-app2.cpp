// event-app-2.cpp

#include "../../../my.h"
#include "../../../my_string.h"
#include "../../../../my/include/my_timing.h"
#include "../../include/eventapp.h"

#include <iostream>
using std::cout;
using std::endl;

#define MSG_SAY_HELLO 77
struct my_app : my::app::terminal_app_base<my_app>
{
	
	my_app() : terminal_app_base(*this) {}
	int handle_message(my::app::msg_t& msg) {
		static int ctr = 0;
		ctr++; 

		if (msg.msg == MSG_SAY_HELLO) {
			const char * s = (const char*)msg.lparam;
			puts("my app got a message, saying:");
			puts(s);
			return 558;
		}
		return 0;
		//if (ctr !=2 ) { return 0; }
		//msg.msg = my::app::msg_t::messages::request_quit;
	}
};

static int app_alive;
void threadfunc()
{
	
	int ctr = 0;
	while (app_alive == 1) {
		my::sleep(100);
		ctr++;
		if (ctr % 5 == 0) {
			my::app::msg_t msg{};
			msg.msg = MSG_SAY_HELLO;
			my::string s("This is a thread saying hello [send_message]");
			const char* ffs = s;
			puts(s);
			msg.lparam = (intptr_t)s.c_str();
			auto app = my::app::application();
			int ret = app->send_message(msg);
			s = "Result returned from send_message was: ";
			s.append(ret);
			puts(s);
		}
	}
}

#include <string>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

// trim from start
static inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}

#include <vector>
typedef std::vector<std::string> strv;

int test_trimming() {

	my::stopwatch sw(true);
	int i = 0;
	strv v;

	my::timer_t myavg = 0;
	my::timer_t stdavg = 0;

	const int cnt = 1000000;
	v.resize(cnt);

	while (i < cnt) {
		my::string s(" whitespace on left");
		s.trim_l();
		assert(s == "whitespace on left");
	//	s += i;
		v[i] = s;
		++i;
	}
	stdavg += sw.stop();
	cout << "My way TooK " << stdavg << " ms." << endl;
	
	sw.start();
	i = 0;
	cout << v[7890] << endl;
	
	while (i < cnt) {
		std::string s(" whitespace on left");
		assert(ltrim(s) == "whitespace on left");
		s += std::to_string(i);;
		v[i] = s;
		++i;
	}
	myavg += sw.stop();
	cout << "std::string way " << myavg << " ms." << endl;


	sw.start();
	i = 0;
	cout << v[7890] << endl;

	while (i < cnt) {
		my::string s(" whitespace on left");
		s.trim_l();
		assert(s == "whitespace on left");
		s += std::to_string(i).c_str();
		v[i] = s;
		++i;
	}
	stdavg += sw.stop();
	///cout << "my way agn: " << sw.stop() << " ms." << endl;


	sw.start();
	i = 0;
	cout << v[7890] << endl;

	while (i < cnt) {
		std::string s(" whitespace on left");
		assert(ltrim(s) == "whitespace on left");
		s += std::to_string(i);;
		v[i] = s;
		++i;
	}
	myavg += sw.stop();

	cout << "Results:    " << "Mine: " << myavg / 2 << " ms." << endl;
	cout << "            " << "STL: " << stdavg / 2 << " ms." << endl;

	auto ffs = v[9999];
	my::string s(" whitespace on left");
	s.trim_l();
	assert(s == "whitespace on left");

	return 0;
}


template <typename T, typename C>
uint64_t test_appending(const T& t, C& v, int cnt)
{
	my::stopwatch sw;
	T s;
	T tot;

	sw.start();
	int i = 0;
	while (i < cnt) {

		s += std::to_string(i).c_str();
		tot += s;
		// v[i] = s;
		++i;
	}
	auto ret = sw.stop();
	cout << "string size is: "<<  tot.size() << endl;
	if (s.size() < 150) {
		cout << "longest string was: " << s << endl;
	}
	return ret;
}

template <typename T>
uint64_t test_creation(const T& t, strv& v, int cnt)
{
	my::stopwatch sw;
	T s;

	sw.start();
	int i = 0;
	while (i < cnt) {
		s = std::to_string(i).c_str();
		v[i] = s;
		++i;
	}
	if (s.size() < 150) {
		cout << "longest string was: " << s << endl;
	}
	return sw.stop();
}


void test_appending_small() {

	const int cnt = 9000;
	strv v;
	v.resize(cnt);
	uint64_t myavg = 0;
	uint64_t stlavg = 0;
	
	cout << "Testing mine ... " << endl;
	int n = 5;

	my::array<my::string> vmy;
	vmy.resize(cnt);

	for (int i = 0; i < n; ++i) {
		cout << "Test mine: " << i + 1 << " of " << n << endl;
		myavg += test_appending(my::string(), vmy, cnt);
	}

	cout << "Testing stl: ... " << endl;

	for (int i = 0; i < n; ++i) {
		cout << "Test stl: " << i + 1 << " of " << n << endl;
		stlavg +=	test_appending(std::string(), v, cnt);
	}
	



	cout << "Results:    " << "Mine: " << myavg / n << " ms." << endl;
	cout << "            " << "STL: " << stlavg / n << " ms." << endl;
}

int main() {

	// return test_trimming();
	test_appending_small();
	my::string s("Apples, Oranges, Pears, etc");
	my::string::array_t ar;
	s.split(",", ar);
	assert(ar.size() == 4);
	system("PAUSE");

}
int test_app()
{
	app_alive = 1;
	{
		std::thread t(threadfunc);
		t.detach();
		my_app myapp;

		my::string s("app exit code is ");
		my::string ss(myapp.exit_code());
		s += ss;
		puts(s.c_str());

		my::string suc;
		s.to_upper(suc);
		puts(suc.c_str());
	} puts("my_app dropped out of scope.");

	app_alive = 0;
	while (app_alive >= 0) {
		my::sleep(100);
	}
	puts("Hit any key to exit");
	_getch();
	return 0;
}
