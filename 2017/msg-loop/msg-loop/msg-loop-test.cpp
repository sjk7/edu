#include "event_macros.h"
#include "q.h"
#include <string>
#include <thread>


USING_STD_OUT;

using MSG = int;

void print_thread() {
	cout << " Thread id: " << std::this_thread::get_id() << endl;
}


struct event_callback
{
	virtual int callback(int a, const std::string& s, int b) = 0;

};

class background_worker 
{
public:
	background_worker(event_callback* callback, std::string sdata, MSG m) 
		:	data(sdata),
			msg(m),
			m_pcallback(callback),
			m_thread(&background_worker::threadfunc, this) {}

	void threadfunc() { action(); }
	virtual void action() = 0;
	void callback_set(event_callback* pcb) { m_pcallback = pcb; }
	
protected:
	std::thread m_thread;
	event_callback* m_pcallback{ 0 };
	std::string data;
	MSG msg{ 0 };
};

class message_pump : public event_callback
{
	using lock_t = std::lock_guard<std::recursive_mutex>;
public:
	message_pump() : m_thread(&message_pump::message_loop, this)
	{
		// the standard guarantees that the thread has started
		// before the constructor returns
	}

	virtual void message_loop()
	{
		while (true)
		{
			background_worker* bw = 0;
			auto pop_result = m_q.pop(bw, 50ms);
			if (pop_result == ETIMEDOUT) {
				if (idle() < 0) {
					cout << "message_loop got a quit signal";
					break;
				}
			}
			else {
				// background worker ready
				
			}
		};
	}

	virtual int callback(int a, const std::string& s, int b) {
		cout << "message loop recvd callback, with a = " << a << ", s=" << s << ", and b = b" << endl;
		return 0;
	}

	// return negative to end the loop.
	virtual int idle() { return m_quit; }
	void quit() {
		m_quit = -1;
		m_thread.join();
	}

private:
	// order of class members is important.
	// For example, the blocking queue must be fully constructed before the thread runs
	cpp::blocking_queue<background_worker*> m_q;
	std::atomic<int> m_quit{ 0 };
	std::thread m_thread;
	

};

struct my_worker : public background_worker
{
	using base_t = background_worker;

	my_worker(event_callback* callback, std::string sdata, MSG m) :
		base_t(callback, sdata, m) {}
		
	
	virtual void action() {
		
		cout << "my_worker : doing lengthy action on thread "; print_thread();
		cout << "in this case, sleeping for 20 seconds..." << endl;
		std::this_thread::sleep_for(20s);

	}
};

int main()
{
	message_pump p;
	my_worker b(&p, "Hello world", 1010);
	
	int i = 0;
	while (true) {
		std::this_thread::sleep_for(100ms);
		++i;

		if (i == 15) {
			b.action();
		}
		if (i % 5 == 0) {
			cout << "Main counter: " << i << endl;
		}
		if (i == 100) {
			p.quit();
			break;
		}
	}
	return 0;
}