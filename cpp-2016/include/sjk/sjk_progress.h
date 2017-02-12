#ifndef SJK_PROGRESS_H
#define SJK_PROGRESS_H

#include "sjk_timing.h"

namespace sjk
{
	namespace progress {
		enum class state_t { CANCELLED = -1, NONE, STARTING, RUNNING, ENDING, FINISHED };
		
		// single-threaded progress
		struct progress_st
		{

			// if you need multi threading, make one of these that uses atomics + a lock on the str()
			struct impl
			{
				using INT = int64_t;
				using STRING = std::string;
				impl(const impl& other) = delete;
				impl& operator=(const impl& other) = delete;
				impl() {}

				STRING string() const { return m_s; }

				int start(const std::string& s, const INT max) { 
					m_sw.start();
					m_state = state_t::STARTING;
					m_max = max; m_val = 0;
					m_s = s; m_state = state_t::STARTING; return notify(); 
				}
				int progress(const INT val, const char* info = nullptr) {
					m_state = state_t::RUNNING;
					m_val = val;
					if (info) {
						m_s = info;
					}
					return notify();
				}
				void end() {
					m_sw.stop();
					m_state = state_t::FINISHED;
					m_val = m_max;
					notify();
				}

				sjk::timer_t taken() const {
					return m_sw.elapsed();
				}
			protected:
				virtual int on_progress(const INT val, const INT max, const state_t, const char* const info) const = 0;

			private:
				STRING m_s;
				INT m_val{ 0 };
				INT m_max{ 0 };
				state_t m_state;
				sjk::timer_t m_last_progress_time;
				sjk::timer_t m_interval{ 100 };

				int  notify() {
					// multi-threaded would need to copy m_s here
					const auto nw = sjk::now();
					if (m_state == state_t::RUNNING) {
						if (nw - m_last_progress_time < m_interval) {
							return 0;
						}
					}
					m_last_progress_time = sjk::now();
					return on_progress(m_val, m_max, m_state, m_s.c_str());
				}
				sjk::stopwatch m_sw;
			};
		};

		

		template <typename IMPL = sjk::progress::progress_st>
		struct progress_t : public IMPL
		{

		public:
			using T = IMPL;
		protected:


		};

		using iprogress = progress_t<typename progress_st::impl>;
	} // namespace progress
}

#endif // SJK_PROGRESS_H
