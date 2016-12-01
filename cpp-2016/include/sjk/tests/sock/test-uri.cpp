
#include "../../sjk_terminal.h"
#include "../../sjk_socket.h"

using namespace std;
DEFINE_ASSERT_CTR;



void test_url10()
{
	using sjk::uri;
	// malformed
	std::string s("http://user:pass@google.com:8888/search.php?region=uk");
	uri u(s);
	ASSERT(u.host() == "google.com");
	ASSERT(u.query() == "region=uk");
	ASSERT(u.protocol() == "http");
	ASSERT(u.url() == s);
	ASSERT(u.path() == "/search.php");
	ASSERT(u.port() == 8888);
	ASSERT(u.user() == "user");
	ASSERT(u.pass() == "pass");
}


void test_url9()
{
	using sjk::uri;
	// malformed
	std::string s("http://:google.com/search.php?region=uk");
	uri u(s);
	ASSERT(u.host() == "");
	ASSERT(u.query() == "region=uk");
	ASSERT(u.protocol() == "http");
	ASSERT(u.url() == s);
	ASSERT(u.path() == "/search.php");
	ASSERT(u.port() == 80);
	ASSERT(u.user() == "");
	ASSERT(u.pass() == "");
}

void test_url8()
{
	using sjk::uri;
	// malformed
	std::string s("http://@google.com/search.php?region=uk");
	uri u(s);
	ASSERT(u.host() == "@google.com");
	ASSERT(u.query() == "region=uk");
	ASSERT(u.protocol() == "http");
	ASSERT(u.url() == s);
	ASSERT(u.path() == "/search.php");
	ASSERT(u.port() == 80);
	ASSERT(u.user() == "");
	ASSERT(u.pass() == "");
}


void test_url7()
{
	using sjk::uri;
	// malformed
	std::string s("http://:@google.com/search.php?region=uk");
	uri u(s);
	ASSERT(u.host() == "google.com");
	ASSERT(u.query() == "region=uk");
	ASSERT(u.protocol() == "http");
	ASSERT(u.url() == s);
	ASSERT(u.path() == "/search.php");
	ASSERT(u.port() == 80);
	ASSERT(u.user() == "");
	ASSERT(u.pass() == "");
}


void test_url6()
{
	using sjk::uri;
	std::string s("http://username:password@google.com:8000/search.php?region=uk");
	uri u(s);
	ASSERT(u.host() == "google.com");
	ASSERT(u.query() == "region=uk");
	ASSERT(u.protocol() == "http");
	ASSERT(u.url() == s);
	ASSERT(u.path() == "/search.php");
	ASSERT(u.port() == 8000);
	ASSERT(u.user() == "username");
	ASSERT(u.pass() == "password");
}




void test_url5()
{
	using sjk::uri;
	std::string s("http://username:password@google.com/search.php?region=uk");
	uri u(s);
	ASSERT(u.host() == "google.com");
	ASSERT(u.query() == "region=uk");
	ASSERT(u.protocol() == "http");
	ASSERT(u.url() == s);
	ASSERT(u.path() == "/search.php");
	ASSERT(u.port() == 80);
	ASSERT(u.user() == "username");
	ASSERT(u.pass() == "password");
}

void test_url4()
{
	using sjk::uri;
	std::string s("http://google.com:443/search.php?region=uk");
	uri u(s);
	ASSERT(u.host() == "google.com");
	ASSERT(u.query() == "region=uk");
	ASSERT(u.protocol() == "http");
	ASSERT(u.url() == s);
	ASSERT(u.path() == "/search.php");
	ASSERT(u.port() == 443);
}



void test_uri3()
{
	using sjk::uri;
	std::string s("http://google.com/search.php?region=uk");
	uri u(s);
	ASSERT(u.host() == "google.com");
	ASSERT(u.query() == "region=uk");
	ASSERT(u.protocol() == "http");
	ASSERT(u.url() == s);
	ASSERT(u.path() == "/search.php");
	ASSERT(u.port() == 80);
}


void test_uri2()
{
	using sjk::uri;
	uri u("http://google.com/search.php");
	ASSERT(u.host() == "google.com");
	ASSERT(u.query() == "");
	ASSERT(u.protocol() == "http");
	ASSERT(u.url() == "http://google.com/search.php");
	ASSERT(u.path() == "/search.php");
	ASSERT(u.port() == 80);
}

void test_uri1()
{
	using sjk::uri;
	uri u("http://google.com");
	ASSERT(u.host() == "google.com");
	ASSERT(u.query() == "");
	ASSERT(u.protocol() == "http");
	ASSERT(u.url() == "http://google.com");
	ASSERT(u.path() == "");
}

int main()
{
	{
		sjk::terminal term(false);
		sjk::net::sock_client sock;
		assert(!sock.is_open());
		test_uri1();
		test_uri2();
		test_uri3();
		test_url4();
		test_url5();
		test_url6();
		test_url7();
		test_url8();
		test_url9();
		test_url10();

	}
	return 0;
}
