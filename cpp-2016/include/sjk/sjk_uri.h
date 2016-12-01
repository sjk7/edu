#ifndef SJK_URI_H
#define SJK_URI_H

#include <string>
#include <algorithm>
#include <cctype>
#include <cassert>
#include <iterator> // back_inserter

namespace sjk
{
	/*/
		Wikipedia:
		scheme:[//[user:password@]host[:port]][/]path[?query][#fragment]

	/*/
	class uri
	{
		private:
		std::string protocol_, host_, path_, query_ ,m_surl, m_spass, m_suser;
		int m_port{0};
		int m_timeout{ WAIT_TIMEOUT_DEFAULT };

		public:
			static constexpr int WAIT_TIMEOUT_FOREVER = -1;
			static constexpr int WAIT_TIMEOUT_DEFAULT = 20000;

		const std::string& protocol() const{ return protocol_;}
		const std::string& host() const { return host_; }
		const std::string& path() const{ return path_; }
		const std::string& query() const { return query_; }
		const std::string& url() const { return m_surl; }
		const std::string& pass() const { return m_spass;}
		const std::string& user() const { return m_suser; }
		int timeout() const { return m_timeout; }
		int port() const{ return m_port; }

		uri(const std::string& surl, int timeoutms = WAIT_TIMEOUT_DEFAULT) :
			m_timeout(timeoutms)
		{
			parse(surl);
		}
		uri(){}

		void clear(){
			 m_suser.clear(); m_spass.clear(); m_surl.clear();
			 protocol_.clear(); host_.clear(); path_.clear(); query_.clear();
			 m_port = 0;
		}

		std::string& parse_user_and_pass(const std::string::size_type at_pos,
										const std::string& url,
										const std::string::size_type colon_pos)
		{
			if (url.empty() || colon_pos == std::string::npos ||
				at_pos == std::string::npos)
			{
				return host_;
			}
			const auto sz = url.size();
			if (colon_pos >= sz || at_pos >= sz) return host_;

			m_suser = url.substr(0, colon_pos);
			const size_t passlen = at_pos - colon_pos - 1;
			if (passlen < sz && passlen){
				m_spass = url.substr(colon_pos + 1, passlen);
			}

			if (sz > at_pos + 1){
				host_ = url.data() + at_pos + 1;
			}

			return host_;
		}

		private:
		/*!
		  Precondition: your uri string should start with http://, file://, ftp:// , whatever
		 * \brief parse
		 * \param url_s
		 */
		void parse(const std::string& url_s)
		{
			using namespace std;
			clear(); m_port = 80;

			m_surl = url_s;
			static const string prot_end("://");
			auto prot_i = search(url_s.begin(), url_s.end(),
												   prot_end.begin(), prot_end.end());
			if (prot_i == url_s.end()){
				assert("urls should start with http://, etc" == 0);
			}
			protocol_.reserve(static_cast<size_t>(distance(url_s.begin(), prot_i)));
			transform(url_s.begin(), prot_i,
					  back_inserter(protocol_),
					  ::tolower); // protocol is icase
			if( prot_i == url_s.end() )
				return;
			advance(prot_i, prot_end.length());
			auto path_i = find(prot_i, url_s.end(), '/');
			host_.reserve(static_cast<size_t>(distance(prot_i, path_i)));
			transform(prot_i, path_i,
					  back_inserter(host_),
					  ::tolower); // host is icase

			std::string::size_type pos_at = host_.find('@');
			if (pos_at != std::string::npos){
				parse_user_and_pass(pos_at, host_, host_.find(':'));
			}

			auto query_i = find(path_i, url_s.end(), '?');
			path_.assign(path_i, query_i);
			if( query_i != url_s.end() )
				++query_i;
			query_.assign(query_i, url_s.end());

			auto& h = host_;
			auto pos = h.find(':');
			if (pos != std::string::npos){
				if (h.size() > pos + 1){
					const auto sport = h.substr(pos + 1);
					if (!sport.empty()){
						m_port = ::atoi(sport.c_str());
						if (m_port <= 0) m_port = 80;
					}
				}
				h = h.substr(0, pos);
			}


		}


	};
}

#endif // SJK_URI_H
