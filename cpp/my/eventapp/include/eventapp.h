// eventapp.h

#pragma once
#ifndef EVENT_APP_H_INCLUDED
#define EVENT_APP_H_INCLUDED

#include <queue>
#include <mutex>


namespace my {
	namespace app {
		struct msg_t
		{
			struct messages {
				enum m { error = -1, idle, startup, msg,  request_quit, terminated };
			};
			msg_t() : msg(0), wparam(0), lparam(0), pv(nullptr) {}
			int msg;
			intptr_t wparam;
			intptr_t lparam;
			void* pv;
		};

		struct app_base : my::no_copy
		{
			typedef std::lock_guard<std::mutex> locker;
			
			static app_base* global_app(app_base* papp = nullptr)
			{
				static app_base* static_app_ptr = 0;
				if (papp) {
					static_app_ptr = papp;
				}
				return static_app_ptr;
			}

			app_base() {
				if (!global_app()) {
					global_app(this);
				}
			}
			struct states {
				enum state { idle, startup, msg, msg_posted, quit, terminate };
			};

			// Only returns once access to the queue is granted, which may be
			// a while if the event loop is busy doing something and returns how many messages,
			// including this one, are in the message queue.
			int post_message(msg_t& m) {
				int ret = 0;
				m_mut.lock();
				try {
					m_q.push(m);
					ret = (int)m_q.size();
				}
				catch (const std::exception& e) {
					fprintf(stderr, "%s", e.what());
				}
				return ret;
			}
						
			virtual int send_message(msg_t& m) = 0;
			
			// returns -1 if there is no msg, > 0 if there are some
			int msg_get(msg_t& m) {
				int cnt = 1;
				while (cnt = next_message(m) > 0) {
					cnt = next_message(m);
					if (cnt > 0) {
						return states::msg_posted;
					}
				}
	
				return states::idle;
			}

		protected:
			mutable std::mutex m_mut;


		private:
			std::queue<msg_t> m_q;
			

			// are there any posted messages to deal with?
			// returns < 0 if there are none.
			int next_message(msg_t& m)
			{
				if (m_q.empty()) return -1;
				m = m_q.front();
				m_q.pop();
				return (int)m_q.size();
			}
		};

		static inline app_base* application() {
			return app_base::global_app();
		}

		template <typename T>
		struct app : app_base
		{
			static constexpr int DO_DEFAULT = 0;

			app(T& t) : m_userapp(t) {}
			int run() {
				int ret = msg_loop();
				msg_t m{}; m.msg = msg_t::messages::terminated;
				m_userapp.on_terminated(m);
				return ret;
			}
			
			virtual int send_message(msg_t& m) {
				int ret = _process_message(m);
				return ret;
			}

			int msg_loop() {
				T& userapp = m_userapp;
				msg_t msg{};
				m_ret = -1; int rv = 0;
				msg.msg = msg_t::messages::startup;
				rv = userapp.on_started(msg);
				if (rv < 0) return rv;
				msg.msg = 0;

				puts("app running.\n");
				while (msg_get(msg) == 0)
				{
					
					{
						m_ret = _process_message(msg);
					}
					
					if (m_ret == states::terminate) {
						return m_ret;
					}
					else if (m_ret < 0) {
						return m_ret;
					}
				}

				return m_ret;
			}

			int exit_code() const { return m_ret; }
			int handle_msg(msg_t& msg) {
				auto& m = msg.msg;
				int rc = 0;
				switch (m)
				{
					case msg_t::messages::request_quit: 
					{
						int x = m_userapp.on_request_quit(msg);
						if (x != DO_DEFAULT) {
							return states::idle;
						}
						return states::terminate;
					}

				default:
					break;
				};
				return rc;
			}
		private:
			
			int _process_message(msg_t& msg) {
				locker lck(m_mut);
				int rv = 0; int rq = 0;
				if (msg.msg == msg_t::messages::request_quit) {
					rq = m_userapp.on_request_quit(msg);
					if (rq > 0) {
						msg.msg = 0;
						return 0; // user cancelled quit.
					}
				}
				rv = m_userapp.on_msg(msg);
				if (rv == DO_DEFAULT) {
					if (msg.msg == msg_t::messages::request_quit) {
						if (rq > 1) msg.msg = msg_t::messages::idle;
					}
					rv = handle_msg(msg);
				}
				if (rv == states::terminate) {
					rv = states::terminate; // this is a graceful end.

				}
				return rv;
			}
			int m_ret;
			T& m_userapp;
			

		};

		template <typename T>
		struct terminal_app_base : my::no_copy
		{
			terminal_app_base(T& t) :m_userapp(t), m_base(t), m_idle_ms(100) { m_base.run(); }
			int on_msg(msg_t& m)
			{
				int ret = 0;
				ret = m_userapp.handle_message(m);
				if (m.msg == msg_t::messages::idle)
				{
					my::sleep(m_idle_ms);
				}
				
				return ret;
			}
			virtual int on_started(msg_t&) { puts("on_started.\n"); return 0; }
			virtual int on_request_quit(msg_t&) { puts("on_request_quit.\n"); return 0; }
			virtual int on_terminated(msg_t&) { puts("on_terminated.\n"); return 0; }
			int exit_code() const { return m_base.exit_code(); }
		private:
			T& m_userapp;
			app<T> m_base;
			int m_idle_ms; // how long to sleep when idle.

		};

	}
}



#endif // EVENT_APP_H_INCLUDED