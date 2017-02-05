// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <Windows.h>
#pragma comment(lib, "winmm")

using namespace std;

template <typename T> T test_construct(T& t, int n = 10000000)
{

	T retval;
	for (int i = 0; i < n; ++i) {
		auto tm = timeGetTime();
		t = std::to_string(tm);
		T other(t);
		t.append("I need to make this long enough to avoid sso");
		DWORD dw = std::atoi(other.c_str());
		if (dw % 20 == 0) {
			retval = other;
			other.clear();
		}
		
	}
	return retval;

}

int main()
{
	std::string s;

	DWORD d = timeGetTime();
	s = test_construct(s);
	cout << s << endl;
	DWORD d2 = timeGetTime();
	cout << "Result was: " << s << endl;
	cout << "Time taken " << std::to_string(d2 - d) << endl;
    return 0;
}

