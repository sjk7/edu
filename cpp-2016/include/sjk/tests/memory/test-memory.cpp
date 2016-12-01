#include "../../sjk_terminal.h"
#include "../../sjk_memory.h"


using namespace std;

void test_span(){

	int x = 77;
	sjk::span<int> s = x;
	assert(!s.empty());
	sjk::span<int> s1;
	s1 = s;
	s1 = x;
	assert(s1 == 77);
	assert(s == 77);
	

	sjk::span<int> empty;
	assert(empty.empty());
	int failed = 0;	
	try{
		volatile int bad = empty; (void) bad;	
	}
	catch(const std::exception& e){
		cout << "Correctly threw runtime_error" << endl;
		cout << e.what() << endl;
		failed = 1;	
	}

	assert(failed);
	failed = 0;

	unsigned long long large = 101;
	sjk::span<unsigned long long> ls(large);
	assert(ls == 101);
	assert(ls.size() == 1);
	assert(ls.size_bytes() == sizeof(unsigned long long));
	char small = ls;
	assert(small == 101);
	
	int myint = 12345;
	sjk::span<int> copy_but_smaller(myint);
	copy_but_smaller = ls;
	cout << "smaller (converted) version is: " << (int)copy_but_smaller << endl;
	cout << "Was expecting: " << large << endl;



}

int main(){

	test_span();	
	sjk::terminal::done();

}
