#pragma once
#ifndef SJK_TIMING_H
#define SJK_TIMING_H
#include <chrono>
#include <type_traits>
#include <thread>
#include <cstdint>

namespace sjk
{
	namespace detail
	{

		static constexpr bool highres_is_steady()
		{
			return std::chrono::high_resolution_clock::is_steady;
		}

		typedef std::conditional<highres_is_steady(),
									std::chrono::high_resolution_clock,
									std::chrono::steady_clock>::type clock;
		static inline auto now() -> decltype (clock::now()){
			return clock::now();
		}

	} // namespace detail

	typedef uint64_t timer_t;
	static inline timer_t ms_since_epoch()
	{
		return static_cast<timer_t>(detail::clock::now().time_since_epoch() / std::chrono::milliseconds(1));
	}

	static inline timer_t now(){ return ms_since_epoch();}
	static inline void sleep(const timer_t ms)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}

	struct timer
	{
		timer() : m_start(now()), m_end(0){}
		timer_t stop(){ m_end = now(); return m_end - m_start;}
		timer_t start(){ timer_t ret = stop(); m_start = now(); return ret;}
		timer_t elapsed() const { return now() - m_start; }
		private:
		timer_t m_start; timer_t m_end;
	};

} // namespace sjk


#endif // SJK_TIMING_H
