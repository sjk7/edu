// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WINTESTDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WINTESTDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WINTESTDLL_EXPORTS
#define WINTESTDLL_API __declspec(dllexport)
#else
#define WINTESTDLL_API __declspec(dllimport)
#endif

#ifndef CAPI
#define CAPI __stdcall
#endif

// This class is exported from the win-test-dll.dll
class WINTESTDLL_API Cwintestdll {
public:
	Cwintestdll(void);
	// TODO: add your methods here.
};

extern WINTESTDLL_API int nwintestdll;

WINTESTDLL_API int fnwintestdll(void);

#ifdef __cplusplus
extern "C"
{
#endif

	int CAPI test_a_function();
#ifdef __cplusplus
}
#endif
