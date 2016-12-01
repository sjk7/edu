#ifndef SJK_SOCKET_H
#define SJK_SOCKET_H

#include "sjk_io.h"
#include "sjk_exception.h"
#include "sjk_timing.h"
#include <vector>
#include <memory>

#include <sys/types.h>
#ifdef __linux
#include <sys/socket.h>
#include <unistd.h> // close()
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#else
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef STRICT
#define STRICT
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif
#include "sjk_uri.h"


namespace sjk
{

	namespace net
	{
#ifndef SOCK_WOULD_BLOCK
#	ifdef _WIN32
#		define SOCK_WOULD_BLOCK WSAEWOULDBLOCK
#		define SOCK_ALREADY WSAEALREADY
#		define SOCK_CONN WSAEISCONN
#		define SOCK_CONN_TIMED_OUT WSAETIMEDOUT
#	else
#		define SOCK_WOULD_BLOCK EAGAIN
#		define SOCK_ALREADY EALREADY
#		define SOCK_CONN DONT_KNOW
#		define SOCK_CONN_TIMED_OUT ETIMEDOUT // not sure
#	endif
#endif

		static inline int socket_error()
		{

#			ifdef _WIN32
			return WSAGetLastError();
#			else
			return errno;
#			endif
		}
		static inline std::string socket_error_str(int err = -1)
		{
			if (err == -1) { err = socket_error(); }
			return sjk::exception::error_string(err);
		}
#ifndef _WIN32
		using SOCKET = int;
#endif
		static constexpr SOCKET SOCK_INVALID = -1;

		class addr_info
		{

		public:
			~addr_info() {
				clear();
			}

			struct ipinfo;
			using infoptr_t = ipinfo*;
			using addrvec_t = typename std::vector<infoptr_t>;

		private:
			addrvec_t m_addrs;
			addrinfo* result{ nullptr };
			void clear() {
				if (result) {
					freeaddrinfo(result);
				}
				for (auto p : m_addrs) {
					if (p) delete p;
					p = nullptr;
				}
				m_addrs.clear();
			}

		public:


			// if an instance of ipinfo survives its containing addr_info,
			// then the behaviour is undefined.
			struct ipinfo
			{
				friend class addr_info;
				enum class version_t { unk = 0, v4 = 4, v6 = 6 };
				~ipinfo() {
					clear();
				}

				size_t ip_size() const {
					if (m_version == version_t::v4) {
						return sizeof(sockaddr_in);
					}
					else {
						return sizeof(struct sockaddr);
					}
				}

				const struct sockaddr* sockadd() const { return (const struct sockaddr* const)m_sockaddr; }
				const std::string& ip() const { return m_sip; }
				const std::string& canonical() const { return m_scanonical; }
				version_t ip_version() const { return m_version; }
				int family() const { return m_family; }
			private:

				ipinfo(const ipinfo& other) = delete;
				ipinfo& operator=(const ipinfo& other) = delete;


				ipinfo(version_t v) : m_version(v) { m_family = static_cast<int>(v); }
				ipinfo(const int family) : m_version(version_t::unk), m_family(family) {}


				//struct sockaddr_in* m_sockaddr4 { nullptr };
				struct sockaddr* m_sockaddr{ nullptr };
				std::string m_sip, m_scanonical;
				version_t m_version{ version_t::unk };
				int m_family{ 0 };

				void clear() {
					m_scanonical.clear();
					m_sip.clear();
					m_sockaddr = nullptr;

				}

			};


			addr_info(const sjk::uri& u)
			{
				clear();
				if (u.host().empty()) {
					SJK_EXCEPTION("Invalid host");
				}
				if (u.port() <= 0) {
					SJK_EXCEPTION("Invalid port ZERO?!");
				}

				struct addrinfo* ptr{ nullptr };
				struct addrinfo hints;


				char ipstringbuffer[46];
				unsigned int ipbufferlength = 46;

				memset(&hints, 0, sizeof(hints));
				hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				hints.ai_flags = AI_CANONNAME; // "official" name of the host
				const char* myptr = 0;
				DWORD dwbuflen = 0;


				std::string sport = std::to_string(u.port());
				int i = getaddrinfo(u.host().c_str(), sport.c_str(), &hints, &result);
				if (i != 0) {
					result = nullptr;
#ifdef WIN32
#define gai_strerror_ gai_strerrorA
#else
#define gai_strerror_ gai_strerror
#endif
					SJK_EXCEPTION("getaddrinfo failed, for: ", u.host(), "\n", ::gai_strerror_(i));
				}
				using namespace std;
				struct sockaddr_in  *sockaddr_ipv4;
				for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

					i++;
					infoptr_t pinfo = nullptr;
					if (ptr->ai_family == AF_INET) {
						pinfo = new ipinfo(ipinfo::version_t::v4);
					}
					else if (ptr->ai_family == AF_INET6) {
						pinfo = new ipinfo(ipinfo::version_t::v6);
					}
					else {
						pinfo = new ipinfo(ptr->ai_family);
					}
					m_addrs.emplace_back(pinfo);

					switch (ptr->ai_family) {

					case AF_UNSPEC:
						//printf("Unspecified\n");
						break;
					case AF_INET:
						pinfo->m_sockaddr = ptr->ai_addr;
						sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
						//printf("\tIPv4 address %s\n",
							//inet_ntoa(sockaddr_ipv4->sin_addr));
						myptr = inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, ipstringbuffer, ipbufferlength);
						if (!myptr) {
							// note the error, but continue
							std::cerr << "Ipv4 resolver: " << net::socket_error() << std::endl;
						}
						pinfo->m_sip = myptr;
						break;
					case AF_INET6:
						//printf("AF_INET6 (IPv6)\n");
						// the InetNtop function is available on Windows Vista and later
						// sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
						// printf("\tIPv6 address %s\n",
						//    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

						// We use WSAAddressToString since it is supported on Windows XP and later
						pinfo->m_sockaddr = reinterpret_cast<struct sockaddr*>(ptr->ai_addr);
						// The buffer length is changed by each call to WSAAddresstoString
						// So we need to set it for each iteration through the loop for safety
						ipbufferlength = 46;
#ifdef _WIN32
						dwbuflen = static_cast<DWORD>(ipbufferlength);
						i = WSAAddressToStringA(pinfo->m_sockaddr, (int)ptr->ai_addrlen, NULL,
							ipstringbuffer, &dwbuflen);
#else
						myptr = inet_ntop(AF_INET, pinfo->m_sockaddr, ipstringbuffer, ipbufferlength);
						if (!myptr) {
							// note the error, but continue
							std::cerr << "Ipv6 resolver: " << net::socket_error() << std::endl;
						}
						i = 0;
#endif
						if (i) {
							std::cerr << "Ipv6 resolver: " << net::socket_error() << std::endl;
						}
						else {
							//printf("\tIPv6 address %s\n", ipstringbuffer);
							pinfo->m_sip = ipstringbuffer;
						}
						break;
#ifdef _WIN32
					case AF_NETBIOS:
						printf("AF_NETBIOS (NetBIOS)\n");
						break;
#endif
					default:
						//printf("Other %ld\n", ptr->ai_family);
						break;
					}
					//printf("\tSocket type: ");
					switch (ptr->ai_socktype) {
					case 0:
						//printf("Unspecified\n");
						break;
					case SOCK_STREAM:
						//printf("SOCK_STREAM (stream)\n");
						break;
					case SOCK_DGRAM:
						//printf("SOCK_DGRAM (datagram) \n");
						break;
					case SOCK_RAW:
						//printf("SOCK_RAW (raw) \n");
						break;
					case SOCK_RDM:
						//printf("SOCK_RDM (reliable message datagram)\n");
						break;
					case SOCK_SEQPACKET:
						//printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
						break;
					default:
						//printf("Other %ld\n", ptr->ai_socktype);
						break;
					}
					//printf("\tProtocol: ");
					switch (ptr->ai_protocol) {
					case 0:
						//printf("Unspecified\n");
						break;
					case IPPROTO_TCP:
						//printf("IPPROTO_TCP (TCP)\n");
						break;
					case IPPROTO_UDP:
						//printf("IPPROTO_UDP (UDP) \n");
						break;
					default:
						//printf("Other %ld\n", ptr->ai_protocol);
						break;
					}
					//printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
					// printf("\tCanonical name: %s\n", ptr->ai_canonname);
					// cout.flush();
					if (ptr->ai_canonname) {
						pinfo->m_scanonical = ptr->ai_canonname;
					}
				}
			}


			const addrvec_t& addresses() const { return m_addrs; }
			addrvec_t::const_iterator begin() const { return m_addrs.cbegin(); }
			addrvec_t::const_iterator end() const { return m_addrs.cend(); }
			size_t size() const { return m_addrs.size(); }

		private:


		};


		/*!
		 * \brief The socket class. Wraps a cross-platform socket
		 */
		class socket
		{
		public:
			
			socket() : m_sock(SOCK_INVALID) { init(1); }
			~socket() { close_skt(); init(-1); }
			enum class state {
				SOCK_STATE_ERROR = -1, SOCK_STATE_NONE, SOCK_STATE_CONNECTING, SOCK_STATE_CONNECTED,
				SOCK_STATE_OPENING, SOCK_STATE_CLOSING
			};

			bool blocking() const {
				return m_blocking;
			}
		protected:
			SOCKET m_sock{ SOCK_INVALID };
			state m_state{ state::SOCK_STATE_NONE };
			bool m_blocking{ false };

			int connect(addr_info::ipinfo* p) {
				assert(p);
				assert(p->sockadd());
				return ::connect(m_sock, p->sockadd(), static_cast<socklen_t>(p->ip_size()));
			}

			// NOTE: for a socket we DO NOT return the # of bytes.
			// Instead, datalen() is set in the span, and we return exactly what recv does
			int64_t read(io::span_t &into)
			{
				int sz = (int)into.size_bytes();
				char* pb = (char*)into.data();
				int remain = sz;
				int64_t numread = 0;
				int rc = 0;
				int iters = 0;

				while (remain > 0)
				{
					iters++;
					rc = ::recv(m_sock, pb, remain, 0);
					if (rc == 0) {
						
						break;
					}
					else if (rc > 0) {
						numread += rc;
						into.datalen_set(numread);
						remain -= rc;
					}
					else {
						// rc < 0
						int err = net::socket_error();
						if (err == SOCK_WOULD_BLOCK) {
							sjk::sleep(10);
							continue;
						}
						assert("some error" == 0);
					}
				}

				return rc;
			}

			// writes all the data and returns when done. Might throw
			int64_t write(io::span_t &from) {
				
				int sz_span = (int)from.size_bytes();
				int remain = sz_span;
				int64_t wrote = 0;
				const char* pb = (const char*)from.data();

				while (remain > 0)
				{
					int rc = ::send(m_sock, pb, remain, 0);
					if (rc == 0) {
						// other end closed gracefully.
					}
					else if (rc < 0)
					{
						assert(0);
					}
					else if (rc > 0)
					{
						remain -= rc;
						wrote += rc;
						pb += rc;
					}

				}

				return wrote;

			}

			//int getsockopt(int sockfd, int level, int optname,
			//	void *optval, socklen_t *optlen);
			
			int getsockopt(int& rv, int level = SOL_SOCKET, int optname = SO_ERROR)
			{
				int optlen = (int)sizeof(int);
				int ret = ::getsockopt(m_sock, level, optname, (char*)&rv, &optlen);
				assert(ret == 0);
				return ret;
			}

			bool blocking_set(bool blocking)
			{
				SOCKET fd = m_sock;
				if (fd < 0) {
					assert("no socket to set blocking" == 0);
				}

#ifdef WIN32
				unsigned long mode = blocking ? 0 : 1;
				bool ret = (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
				if (ret) {
					m_blocking = blocking;
				}
				return ret;
#else
				int flags = fcntl(fd, F_GETFL, 0);
				if (flags < 0) return false;
				flags = blocking ? (flags&~O_NONBLOCK) : (flags | O_NONBLOCK);
				bool ret = (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
				if (ret) {
					m_blocking = blocking;
				}
				return ret;
#endif
			}

			// gets the actual socket's error, which may be different from WSAGetLastError()
			// this is protected, because WSAGetLastError should be ample. If it isn't, this is here to expose if you want.
			// One notable time it is useful is when trying to connect a non-blocking socket. This gets the actual failure reason in 'doze
			int get_socket_error() const {

				int error = -1;
#ifdef _WIN32
				int len = sizeof(error);
#else
				socklen_t len = sizeof(error);
#endif
				int gso = ::getsockopt(m_sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
				assert(gso == 0); (void)gso;
				return error;

			}

			int close_skt() {
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif
				int ret = 0;
				if (m_sock) {
					ret = shutdown(m_sock, SHUT_RDWR);
#ifdef _WIN32
					ret = ::closesocket(m_sock);
#else
					ret = ::close(m_sock);
#endif
					m_sock = SOCK_INVALID;
				}
				return ret;
			}

			SOCKET sock_create(bool blocking = true) {
				assert(m_sock == SOCK_INVALID);
				if ((m_sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == SOCK_INVALID)
				{
					SJK_EXCEPTION("create() failed, with ", socket_error(), " : ", socket_error_str());
				}
				blocking_set(blocking);
				return m_sock;
			}
			void init(const int incr) {
				static int init_count = 0;
				if (init_count == 0) {
#ifdef WIN32
					WORD wVersionRequested;
					WSADATA wsaData;
					wVersionRequested = MAKEWORD(2, 2);

					int err = WSAStartup(wVersionRequested, &wsaData);
					if (err != 0) {
						SJK_EXCEPTION("WSAStartup failed with error:", err);
					}
#endif
				}
				init_count += incr;
				assert(init_count >= 0);
				if (init_count == 0) {
#ifdef WIN32
					int err = WSACleanup();
					assert(err == 0);
					std::cout << "WSACleanup called, with err = " << err << std::endl;
#endif
				}
			}
		};

		/*!
		 * \brief The sock class. Adds io::device interface to a socket
		 */

		class sock : public socket, public sjk::io::device
		{

		public:

		protected:
			sjk::uri m_uri;
			virtual ~sock() {}
		private:
		public:
			// device interface

			virtual int open(const std::string &path, io::flags::flag_t flags) override
			{
				(void)path; (void)flags;
				return 0;
			}
			virtual int is_open() const override
			{
				return m_state >= state::SOCK_STATE_CONNECTING;
			}
			virtual int close() override
			{
				return socket::close_skt();
			}
			virtual int64_t read(io::span_t &into) override
			{
				return socket::read(into);
			}
			virtual int64_t write(io::span_t &from) override
			{
				return socket::write(from);
			}
			virtual bool at_end() const override
			{
				return 0;
			}
			virtual size_t size_bytes() const override
			{
				return 0;
			}
			virtual int64_t seek(int64_t, std::ios::seekdir) override
			{
				return -1;
			} // sockets can't seek
			virtual int64_t position() const override
			{
				return 0;
			}
			virtual void clear_errors() override
			{}
			virtual std::string path() override
			{
				return std::string();
			}
		protected:


		};

		/*!
		 * \brief The sock_client class : a socket that connects to a server
		 */
		class sock_client : public sock
		{
		public:
			virtual ~sock_client() {}
			sock_client() {}
			sock_client(const sjk::uri& u)
			{
				connect_to_server(u);
			}

			int connect_to_server(const sjk::uri& u) {

				close();
				sock_create(false);
				addr_info addrs(u);

				if (addrs.size() == 0) {
					SJK_EXCEPTION("No addresses resolved, for: ", u.host(), " Check the address is correct.");
				}
				do_connect(addrs, u.timeout());
				return 0;
			}

		protected:

			void do_connect(const addr_info& addrs, int timeoutms)
			{
				int connected = -1;
				int ctr = 0;
				bool was_blocking = socket::blocking();
				socket::blocking_set(false);
				std::stringstream sserrors;
				sjk::timer t;
				int err = 0;

				for (const auto p : addrs)
				{
					while (true) {
						int rv = socket::connect(p);
						if (rv) {
							err = net::socket_error();
						}
						if (err == SOCK_CONN) {
							connected = ctr;
							break;
						}
						if (err == SOCK_WOULD_BLOCK || err == SOCK_ALREADY) {
							sjk::sleep(5);
							if (timeoutms > 0 && t.elapsed() > timeoutms) {
								socket::close_skt();
								SJK_EXCEPTION("Timed out (library layer) connecting to ", p->ip(),
									"\n", "After ", t.elapsed(), " milliseconds.");
								break;
							}
							int opt = 0;
							int optrv = getsockopt(opt);
							if (opt) {
								socket::close_skt();
								if (opt == SOCK_CONN_TIMED_OUT) {
									SJK_EXCEPTION("Timed out (socket layer) connecting to ", p->ip(),
										"\n" , "After " , t.elapsed() ," milliseconds.");
								}
								else {
									assert(0); // what's this??
								}
							}
						}
						else {
							if (rv == 0) {
								std::cout << "connected in: " << t.elapsed() << " ms." << std::endl;
								connected = ctr;
								break;
							}
							else {
								SJK_EXCEPTION("Failed connecting to ip: ", p->ip(),
									"\n", net::socket_error_str());
								break;
							}
						}
					};
					if (connected >= 0) {
						break;
					}
					ctr++;
				}

			}

		private:


		};
	}
}


#endif // SJK_SOCKET_H
