// win-test-dll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "win-test-dll.h"


// This is an example of an exported variable
WINTESTDLL_API int nwintestdll=0;

// This is an example of an exported function.
WINTESTDLL_API int fnwintestdll(void)
{
    return 42;
}

int CAPI test_a_function()
{
	return 77;
}

// This is the constructor of a class that has been exported.
// see win-test-dll.h for the class definition
Cwintestdll::Cwintestdll()
{
    return;
}
