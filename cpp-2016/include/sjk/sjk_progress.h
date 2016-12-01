#ifndef SJK_PROGRESS_H
#define SJK_PROGRESS_H

#include "sjk_timing.h"

namespace sjk
{

	struct progress_threading
	{

		// if you need multi threading, make one of these that uses atomics + a lock on the str()
		struct dummy
		{
			using INT = int;
			using STRING = std::string;

			void value_set(const int i){ m_i = i;}
			int value() const { return m_i;}
			int max() const { return m_max;}
			void max_set(const int m){ m_max = m;}

			private:
			STRING m_s;
			INT m_i {0};
			INT m_max{0};
		};
	};

	template <typename THREADING = typename sjk::progress_threading::dummy>
	struct progress
	{

		private:
		using T = THREADING;
		T m_threading;

		public:
		progress(const T& threading) :
		m_threading(threading){

		}
		enum class state_t {CANCELLED = -1, NONE, STARTING, RUNNING, ENDING, FINISHED};


		// return < 0 to cancel the operation
		virtual int on_progress(const state_t state) = 0;

		virtual ~progress(){}

		int val() const
		{
			return T::value();
		}

		int max() const{
			return T::max();
		}

		std::string info() const{
			std::string retval = T::string();
			return retval;
		}

	};
}

#endif // SJK_PROGRESS_H
