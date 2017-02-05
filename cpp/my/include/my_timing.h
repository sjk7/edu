#pragma once
#ifndef _TIMING_H_INCLUDED_
#define _TIMING_H_INCLUDED_

#include "my.h"

#include <chrono>
#include <thread>
#ifdef _WIN32
 // #include <mmsystem.h>
#else
#	include <unistd.h>
#endif


namespace my {
	typedef std::chrono::milliseconds::rep timer_t;

	// returns millis since epoch
	static inline timer_t timer()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	static inline timer_t now() { return timer(); }
	static inline timer_t timegettime() { return timer(); }
	static inline timer_t timeGetTime() { return timer(); }

	void sleep(uint32_t millis) {
		std::this_thread::sleep_for(std::chrono::milliseconds(millis));
	}


	class stopwatch
	{
	public:
		stopwatch(bool start_now = false) : m_start(0), m_finish(0) {
			if (start_now) { m_start = my::timer(); }
		}

		timer_t start() { const timer_t old = elapsed(); m_start = timer(); return old; }
		timer_t stop() { m_finish = my::timer(); return elapsed(); }

		timer_t elapsed() const {
			// cover the case where sum1 called stop() without start()
			if (m_start == 0) return 0;
			m_finish = my::timer();
			if (m_finish < m_start) return 0;
			return m_finish - m_start;
		}

		virtual ~stopwatch() {}
	private:
		timer_t m_start;
		mutable timer_t m_finish;
	};
} // namespace chl

#endif // _TIMING_H_INCLUDED_

