// event-app-2.cpp

#include "../../../my.h"
#include "../../../my_string.h"
#include "../../include/eventapp.h"

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


int main() {

	my::string s("Apples, Oranges, Pears, etc");
	my::string::array_t ar;
	s.split(",", ar);
	assert(ar.size() == 4);

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
