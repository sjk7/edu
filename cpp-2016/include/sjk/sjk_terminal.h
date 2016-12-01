#pragma once
#ifndef SJK_TERMINAL_INCLUDED
#define SJK_TERMINAL_INCLUDED


#ifdef _WIN32
#include "sjk_windows.h"
#endif

#ifdef _MSC_VER
#	pragma warning(disable: 26495)
#	pragma warning(disable: 26481)
#	pragma warning(disable: 26493) // casts warning
#	pragma warning(disable: 26491) // casts warning
#endif

#	ifdef _WIN32
#		ifndef _INC_WINDOWS
#			ifndef IGNORE_NO_WINDOWS_H
#				error "Steve says: windows programs must #include<windows.h>, else you may get name conflicts you don't know about"	
#			endif
#		endif
#		endif

#include <iostream>
#include <utility>
#include <string>
#include "stdio.h"
#include "sjk_assert.h"
#include "sjk_timing.h"


#define DEFINE_ASSERT_CTR namespace assert_ctr {uint64_t ctr; }



namespace sjk {

	struct terminal {

        terminal(bool wait_for_return_key = false,
				 std::string exit_msg= "All done. ") :
            m_smsg(exit_msg),
            m_bwait_for_return_key(wait_for_return_key)
        {
			if (wait_for_return_key){
				exit_msg += "Hit return to exit." ;
			}
        }

        ~terminal(){
            done(m_smsg, m_bwait_for_return_key);
        }

		int done(std::string msg = "All done. Hit return to exit.", bool wait = true) {
			using std::endl; using std::cout; using std::cin; using std::getline;
			cout << "Assertions passed: " << assert_ctr::ctr << endl;
			auto ms = m_tmr.stop();
			cout << "Execution time: " << ms << " ms (" << ((float)ms /1000.0f) << " secs.)" <<  endl;
			cout << msg << endl;
			if (wait){
				std::string s;
				getline(cin, s);
			}
			return 0;
		} // int done()

		static inline void test_progress(){
			double progress = 0.0;
			while (progress < 1.0) {
				int barWidth = 70;

				std::cout << "[";
				int pos = static_cast<int>(barWidth * progress);
				for (int i = 0; i < barWidth; ++i) {
					if (i < pos) std::cout << "=";
					else if (i == pos) std::cout << ">";
					else std::cout << " ";
				}
				std::cout << "] " << int(progress * 100.0) << " %\r";
				std::cout.flush();

				progress += 0.16; // for demonstration only
				sjk::sleep(900);
			}
			std::cout << std::endl;
		}

		#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
		#define PBWIDTH 60

		static void print_progress (double percentage)
		{
			int val = (int) (percentage * 100);
			int lpad = (int) (percentage * PBWIDTH);
			int rpad = PBWIDTH - lpad;
			printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
			fflush (stdout);
		}

		static void test_progress2()
		{
			int i = 0;
			while (i < 100){
				++i;
				sjk::sleep(500);
				print_progress((double)((double)i / 100.0));
			}
		}

    private:
		sjk::timer m_tmr;
        std::string m_smsg;
        bool m_bwait_for_return_key;


	}; // struct terminal 


} // namespace sjk

#endif
