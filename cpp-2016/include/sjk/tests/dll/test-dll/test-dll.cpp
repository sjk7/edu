// test-dll. cpp

#include "../../../sjk_dll.h"
#include <iostream>
#include <string>

using namespace std;

static uint64_t assert_ctr::ctr = 0;

int load_dll(const std::string& dll_path, sjk::dll::loader& l)
{
	try
	{
		l.load(dll_path);
		cout << "Loaded dll, with path: " << l.path() << endl;
	}
	catch (const sjk::exception&)
	{
		return -1;
	}

	return NOERROR;
}

void* get_func(const std::string& funcname, sjk::dll::loader& l)
{
	void* ret = NULL;
	try {
		ret = static_cast<void*>(l.get_func_address(funcname));
	}
	catch (...)
	{
		return NULL;
	}
	
	return ret;
}

int main()
{
	{
		sjk::dll::loader l;
		int ret = load_dll("win-test-dll-noexist.dll", l);
		ASSERT(ret == -1);
		ret = load_dll("win-test-dll.dll", l);
		ASSERT(ret == 0);

		void* my_get_func = get_func("non-exist-function", l);
		ASSERT(my_get_func == NULL);


		void* pvfunc = l.get_func_address("test_a_function");
		ASSERT(pvfunc);

		//int CAPI test_a_function();

		typedef  int(__stdcall *fptr)();

		fptr p = (fptr)pvfunc;
		int x = p();
		ASSERT(x == 77);
	} // end of scope

	return 0;
}