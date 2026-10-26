// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ADDRTONAMEDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ADDRTONAMEDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef ADDRTONAMEDLL_EXPORTS
#define ADDRTONAMEDLL_API __declspec(dllexport)
#else
#define ADDRTONAMEDLL_API __declspec(dllimport)
#endif

// This class is exported from the AddrToNameDLL.dll
class ADDRTONAMEDLL_API CAddrToNameDLL {
public:
	GUID x;

	CAddrToNameDLL(void);
	~CAddrToNameDLL(void);
	// TODO: add your methods here.
};

extern ADDRTONAMEDLL_API int nAddrToNameDLL;

ADDRTONAMEDLL_API int fnAddrToNameDLL(void);
