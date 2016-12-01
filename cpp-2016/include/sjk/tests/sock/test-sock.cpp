// test-sock.cpp
#include "../../../../include/sjk/sjk_terminal.h"
#include "../../../../include/sjk/sjk_socket.h"

DEFINE_ASSERT_CTR;
using namespace std;


void test_sock_known_no_exist_domain()
{
	sjk::uri u("http://thisdoesnotexistnowaynohow.co.uk");
	int fail = false;

	try {
		sjk::net::sock_client s(u);
	}
	catch (const sjk::exception& e) {
		(void)e;
		fail = true;
	}

	ASSERT(fail);

}

void test_sock_good_domain_bad_port(bool forever_wait = false)
{
	cout << "Connecting to a known good domain, bad port. Please wait..." << endl;
	int timeout = 2000;
	if (forever_wait) timeout = sjk::uri::WAIT_TIMEOUT_FOREVER;
	sjk::uri u("http://google.com:7654", timeout);
	int fail = false;

	try {
		sjk::net::sock_client s(u);
	}
	catch (const sjk::exception& e) {
		fail = true;
		(void)e;
	}

	ASSERT(fail);

}

void test_sock_good_domain_bad_port_forever()
{
	cout << "Connecting to a known good domain, bad port. Please wait..." << endl;
	sjk::uri u("http://google.com:7654", sjk::uri::WAIT_TIMEOUT_FOREVER);
	int fail = false;

	try {
		sjk::net::sock_client s(u);
	}
	catch (const sjk::exception& e) {
		fail = true;
		(void)e;
	}

	ASSERT(fail);

}

void test_ipv6()
{
	sjk::uri u("http://ipv6.google.com");
	int fail = false;

	try {
		sjk::net::sock_client s(u);
	}
	catch (const sjk::exception& e) {
		fail = true;
		(void)e;
	}

	ASSERT(fail);
}

/*/

int mstest()
{
	//-----------------------------------------
	// Declare and initialize variables
	WSADATA wsaData;
	int iResult;
	INT iRetval;

	DWORD dwRetval;

	int i = 1;

	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;

	struct sockaddr_in  *sockaddr_ipv4;
	//    struct sockaddr_in6 *sockaddr_ipv6;
	LPSOCKADDR sockaddr_ip;

	char ipstringbuffer[46];
	DWORD ipbufferlength = 46;


	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	printf("Calling getaddrinfo with following parameters:\n");
	std::string ip = "google.com";
	//printf("\tnodename = %s\n", argv[1]);
	//printf("\tservname (or port) = %s\n\n", argv[2]);

	//--------------------------------
	// Call getaddrinfo(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfo structures containing response
	// information
	dwRetval = getaddrinfo(ip.c_str(), "80" , &hints, &result);
	if (dwRetval != 0) {
		printf("getaddrinfo failed with error: %d\n", dwRetval);
		WSACleanup();
		return 1;
	}

	printf("getaddrinfo returned success\n");

	// Retrieve each address and print out the hex bytes
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		printf("getaddrinfo response %d\n", i++);
		printf("\tFlags: 0x%x\n", ptr->ai_flags);
		printf("\tFamily: ");
		switch (ptr->ai_family) {
		case AF_UNSPEC:
			printf("Unspecified\n");
			break;
		case AF_INET:
			printf("AF_INET (IPv4)\n");
			sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
			printf("\tIPv4 address %s\n",
				inet_ntoa(sockaddr_ipv4->sin_addr));
			break;
		case AF_INET6:
			printf("AF_INET6 (IPv6)\n");
			// the InetNtop function is available on Windows Vista and later
			// sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
			// printf("\tIPv6 address %s\n",
			//    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

			// We use WSAAddressToString since it is supported on Windows XP and later
			sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
			// The buffer length is changed by each call to WSAAddresstoString
			// So we need to set it for each iteration through the loop for safety
			ipbufferlength = 46;
			iRetval = WSAAddressToStringA(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
				ipstringbuffer, &ipbufferlength);
			if (iRetval)
				printf("WSAAddressToString failed with %u\n", WSAGetLastError());
			else
				printf("\tIPv6 address %s\n", ipstringbuffer);
			break;
		case AF_NETBIOS:
			printf("AF_NETBIOS (NetBIOS)\n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_family);
			break;
		}
		printf("\tSocket type: ");
		switch (ptr->ai_socktype) {
		case 0:
			printf("Unspecified\n");
			break;
		case SOCK_STREAM:
			printf("SOCK_STREAM (stream)\n");
			break;
		case SOCK_DGRAM:
			printf("SOCK_DGRAM (datagram) \n");
			break;
		case SOCK_RAW:
			printf("SOCK_RAW (raw) \n");
			break;
		case SOCK_RDM:
			printf("SOCK_RDM (reliable message datagram)\n");
			break;
		case SOCK_SEQPACKET:
			printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_socktype);
			break;
		}
		printf("\tProtocol: ");
		switch (ptr->ai_protocol) {
		case 0:
			printf("Unspecified\n");
			break;
		case IPPROTO_TCP:
			printf("IPPROTO_TCP (TCP)\n");
			break;
		case IPPROTO_UDP:
			printf("IPPROTO_UDP (UDP) \n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_protocol);
			break;
		}
		printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
		printf("\tCanonical name: %s\n", ptr->ai_canonname);
	}

	freeaddrinfo(result);
	WSACleanup();

	return 0;
}
/*/

int main()
{
	sjk::terminal terminal;
	{
		{
			sjk::terminal term(false, "(for socket send / rcv test)");
			sjk::net::sock_client sck;
			sjk::uri u("http://google.com/");
			sck.connect_to_server(u);
			sjk::span<const char> myspan = "GET / HTTP/1.1\r\nConnection: close\r\n\r\n";
			sjk::io::span_t iospan = myspan;
			int64_t wrote = sck.write(iospan);
			std::string sdata(65535, ' ');
			sjk::io::span_t rcv(sdata);
			int64_t read = sck.read(rcv);
			ASSERT(read == 0);
			ASSERT(rcv.datalen() > 0);
			sdata.resize(rcv.datalen());
			cout << rcv.data() << endl;

		}
		cout << "------------------------------" << endl << endl;

		
		sjk::terminal term;
		test_ipv6(); // NOTE: cannot get this to resolve an ipv6 address at all in windows, in linux it just looks wrong
		test_sock_known_no_exist_domain();
		test_sock_good_domain_bad_port(false);
		test_sock_good_domain_bad_port(true);
		

		cout << "done" << endl;
	}
}
