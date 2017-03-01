#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <exception>
#include <atomic>

#ifndef ASSERT
#	include <cassert>
#	define ASSERT assert
#endif

#include <chrono>
using namespace std::chrono_literals;


namespace cpp {
	template <typename T, typename M = std::recursive_mutex, typename C = std::condition_variable_any>
	class blocking_queue
	{
		int m_pushed; int m_popped;
	public:

		blocking_queue() : m_pushed(0), m_popped(0), m_quit(false), m_size(0) {}
		virtual ~blocking_queue() {
			post_quit();
		}

		blocking_queue(const blocking_queue& other) = delete;
		blocking_queue& operator=(const blocking_queue& other) = delete;

		bool empty() const {
			std::lock_guard<M> lock(m_mut);
			return m_q.empty();
		}

		void push(const T& s)
		{
			{
				std::lock_guard<M> lock(m_mut);
				m_q.push(s);
				m_size = m_q.size();
				m_pushed++;

			}
			m_cv.notify_one();
		}

		void clear() {

			{
				std::unique_lock<M> lk(m_mut);
				while (m_q.size()) {
					m_q.pop();
				}
			}
			m_cv.notify_one(); // ensure pop() wakes up

		}

		using milli_t = std::chrono::milliseconds;

		int pop(T& t, milli_t timeoutms = milli_t::max())
		{

			try
			{
				std::unique_lock<M> lk(m_mut);
				if (m_quit) {
					std::cerr << "you tried to pop() something from the q when it was dead\n";

					return -2;
				}
				auto wait_result = m_cv.wait_for(lk, timeoutms , [&] {
					
					if (!m_q.empty() || m_quit) return true;
					return false;
				});
				if (m_quit) {
					if (m_q.empty()) {
						return -1;
					}
				}
				
				if (!wait_result) {
					return ETIMEDOUT;
				}
				m_popped++;

				ASSERT(!m_q.empty());
				t = m_q.front();
				m_q.pop();
				m_size = m_q.size();
				return 0;
			}
			catch (const std::exception& e) {
				std::string se = e.what();
				std::cerr << se << std::endl;
				return -10;
			}
		}

		void post_quit() {
			if (m_quit == 1)
			{
				return;
			}
			m_quit = 1;
			clear(); // important if you are holding smart pointers: you don't want them hanging around any more!
		}
		int quit() const {
			return m_quit;
		}

		int size() const {
			return (int)m_size;
		}
	protected:


	private:
		std::atomic<int> m_quit;
		std::atomic<size_t> m_size;
		std::queue<T> m_q;
		M m_mut;
		C m_cv;

	};
}