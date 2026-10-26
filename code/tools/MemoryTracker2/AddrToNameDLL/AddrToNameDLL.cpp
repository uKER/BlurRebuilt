// AddrToNameDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "AddrToNameDLL.h"

#include <dbghelp.h>
#include <assert.h>
#include <iostream>
#include <xdevkit.h>
#include <atlbase.h>
#include "dia2.h"
#include "xbdm.h"

#include <string>
#include <vector>

#ifdef _MANAGED
#pragma managed(push, off)
#endif

struct Module
{
    Module( ULONG_PTR address, size_t size, CComPtr<IDiaSession>& psession )
        : m_Address( address ), m_Size( size ), m_psession( psession )
    {
    }
    
    ULONG_PTR   m_Address;
    size_t      m_Size;
    CComPtr<IDiaSession> m_psession;
};

std::vector<Module> m_ModuleList;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

#define MAX_LENGTH_DEBUG_STR	1024
void DBPrint(const char *c, ...)
{
	char string[MAX_LENGTH_DEBUG_STR + 1];
	va_list arglist;
	
	va_start(arglist, c);
	_vsnprintf_s(string, _countof(string), _TRUNCATE, c, arglist);
	va_end(arglist);

	assert(strlen(string) < MAX_LENGTH_DEBUG_STR);

	OutputDebugStringA(string);
}

void DBPrintW(const WCHAR *c, ...)
{
	WCHAR string[MAX_LENGTH_DEBUG_STR + 1];
	va_list arglist;
	
	va_start(arglist, c);
	_vsnwprintf_s(string, _countof(string), _TRUNCATE, c, arglist);
	va_end(arglist);

	assert(wcslen(string) < MAX_LENGTH_DEBUG_STR);

	OutputDebugStringW(string);
}

// This is an example of an exported variable
ADDRTONAMEDLL_API int nAddrToNameDLL=0;

HANDLE g_hProcess = (HANDLE)1;

//bool LoadDbgHelp()
//{
//    char* xedkDir;
//    size_t xedkDirSize;
//    errno_t err = _dupenv_s(&xedkDir, &xedkDirSize, "xedk");
//    
//	if (err || !xedkDir)
//        return false;
//
//	std::string dbgHelpPath = std::string( xedkDir ) + "\\bin\\win32\\dbghelp.dll";
//
//	// Call LoadLibrary on the dll.
//	HMODULE hDll = LoadLibraryA(dbgHelpPath.c_str());
//
//	free(xedkDir);
//
//	return hDll != NULL;
//}

extern "C" ADDRTONAMEDLL_API bool LoadSymbolsForModule(unsigned int baseAddress, unsigned int moduleSize, unsigned char* buffer, int size)
{
	assert(sizeof(DM_PDB_SIGNATURE) == size);

	DM_PDB_SIGNATURE* signature = ((DM_PDB_SIGNATURE*)buffer);

    // Create a DIA2 data source
    CComPtr<IDiaDataSource> pSource;
    HRESULT hr = CoCreateInstance(__uuidof(DiaSource), NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**) &pSource);

    if (FAILED(hr))
	{
		switch (hr)
		{
		case REGDB_E_CLASSNOTREG:
			DBPrint("Failed to create CoCreateInstance: REGDB_E_CLASSNOTREG\n");
			break;

		case CLASS_E_NOAGGREGATION:
			DBPrint("Failed to create CoCreateInstance: CLASS_E_NOAGGREGATION\n");
			break;

		case E_NOINTERFACE:
			DBPrint("Failed to create CoCreateInstance: E_NOINTERFACE\n");
			break;

		default:
			DBPrint("Failed to create CoCreateInstance HR(0x%x)\n", hr);
			break;
		}

        return false;
	}

    const char* pdbPath = signature->Path;

    // Ask DbgHelp to look for the PDB file. Because DbgHelp has previously been told
    // to use the XEDK symbol server this should locate system debug information.
    char resultPath[MAX_PATH];
    BOOL findResult = SymFindFileInPath( g_hProcess, 0,
                const_cast<char*>(pdbPath),
                const_cast<GUID*>(&signature->Guid), signature->Age, 0,
		        SSRVOPT_GUIDPTR,
                resultPath, 
                NULL,
                NULL);

    // If DbgHelp found the symbols then adjust our path.
    if( findResult )
        pdbPath = resultPath;

    // Convert the filename to wide characters for use with DIA2.
    wchar_t wPdbPath[ _MAX_PATH ];
    size_t convertedChars;
    mbstowcs_s( &convertedChars, wPdbPath, pdbPath, _TRUNCATE );

    // See if there is a PDB file at the specified location, and if so,
    // load it, checking the GUID and age to make sure that it is the correct
    // PDB file. Note that the timeStamp is not used anymore.
    hr = pSource->loadAndValidateDataFromPdb( wPdbPath, const_cast<GUID*>(&signature->Guid), 0, signature->Age );
    if( FAILED( hr ) )
    {
        // Check the error code for details on why the load failed, which
        // could be because the file doesn't exist, signature doesn't match,
        // etc.
		DBPrintW(L"Failed to 'loadAndValidateDataFromPdb' - '%s'\n", wPdbPath);
        return false;
    }

	DBPrintW(L"Loaded: %s\n", wPdbPath);

    // Create a session for the just loaded PDB file.
    CComPtr<IDiaSession> psession;
    if( FAILED( pSource->openSession( &psession ) ) ) 
    {
		OutputDebugString(L"Failed to \"openSession\"\n");
        return false;
    }

    // Tell DIA2 where the module was loaded.
    psession->put_loadAddress( (ULONG_PTR)baseAddress );

    // Add this session to a list of loaded modules.
    m_ModuleList.push_back( Module( (ULONG_PTR)baseAddress, moduleSize, psession ) );
    
	return true;
}

extern "C" ADDRTONAMEDLL_API void Shutdown()
{
}

extern "C" ADDRTONAMEDLL_API void Reset()
{
	m_ModuleList.clear();
}

extern "C" ADDRTONAMEDLL_API int Initialise()
{
	//if (!LoadDbgHelp())
	//{
	//	DBPrint("Unable to load dbghelp.dll from Xenon SDK.\n");
	//	return 1;
	//}

    // Create an XboxManager object to let us get the symbol server path.
    CComPtr<IXboxManager> spManager;
	HRESULT hr1 = spManager.CoCreateInstance(__uuidof(XboxManager));
	if( !SUCCEEDED( hr1 ) )
    {
        DBPrint("Couldn't create XboxManager object.\n");
        return -1;
    }

    // Get the XEDK symbol server path.
	CComBSTR bstrSymbolServerPath = NULL;
    hr1 = spManager->get_SystemSymbolServerPath(&bstrSymbolServerPath);
	if( !SUCCEEDED( hr1 ) )
    {
        DBPrint("Couldn't get XEDK symbol server path.\n");
        return -2;
    }

    // Convert the symbol server path from wide characters to char.
    char symbolServerPath[MAX_PATH];
    sprintf_s( symbolServerPath, "%S", bstrSymbolServerPath );

    // Now build up a complete symbol search path to give to DbgHelp.

    // Add the XEDK symbol server to the symbol search path.
    std::string fullSearchPath = symbolServerPath;

    CHAR* ntSymbolPath;
    size_t symbolPathSize;
    errno_t err = _dupenv_s( &ntSymbolPath, &symbolPathSize, "_NT_SYMBOL_PATH" );
    if( !err && ntSymbolPath )
	{
        fullSearchPath += ";" + std::string( ntSymbolPath );
		free( ntSymbolPath );
    }

    // Add the current directory to the search path.
    fullSearchPath += std::string(";.");

	DBPrint("SymPath='%s'\n", const_cast<char*>(fullSearchPath.c_str()));

    // Pass the symbol search path on to DbgHelp.
	if (!SymInitialize(g_hProcess, const_cast<char*>(fullSearchPath.c_str()), FALSE))
	{
		DBPrint("SymInitialize failed.\n");
		return -3;
	}

//#ifdef _DEBUG
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_EXACT_SYMBOLS | SYMOPT_DEBUG );
//#else
//    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_EXACT_SYMBOLS | SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS);
//#endif

	//ULONG ulModAddress = 0x82000000;

 //   DWORD64 dw64BaseAddress = SymLoadModule64(g_hProcess, 0, strPathOfXbeFile, NULL, ulModAddress, 0);
 //   if (!dw64BaseAddress)
 //   {
 //       DBPrint("Unable to load symbols for module %s.\n", strPathOfXbeFile);
 //       return 1;
 //   }

 //   assert(dw64BaseAddress == ulModAddress);

	return 0;
}

extern "C" ADDRTONAMEDLL_API bool GetName(unsigned int address, char* buffer, int maxLen)
{
    bool success = false;
    //CHAR symbolName[1000] = {0};
    ULONG displacement = 0;
    CHAR filename[500] = {0};
    ULONG lineNumber = 0;

    // Scan through the list of loaded modules to find the one that contains the
    // requested address.
    for( size_t i = 0; i < m_ModuleList.size(); ++i )
    {
        // Find what module's address range the address falls in.
        if( address > m_ModuleList[i].m_Address && address < m_ModuleList[i].m_Address + m_ModuleList[i].m_Size )
        {
            CComPtr<IDiaSession>& pSession = m_ModuleList[i].m_psession;

            // Find the symbol using the virtual address--the raw address. This
            // only works if you have previously told DIA where the module was
            // loaded, using put_loadAddress.
            // Specify SymTagPublicSymbol instead of SymTagFunction if you want
            // the full decorated names.
            IDiaSymbol* pFunc;
            HRESULT result = pSession->findSymbolByVA( address, SymTagFunction, &pFunc );
            if ( SUCCEEDED( result ) && pFunc )
            {
                // Get the name of the function.
                CComBSTR functionName = 0;
                pFunc->get_name( &functionName );
                if( functionName )
                {
                    // Convert the function name from wide characters to char.
                    sprintf_s(buffer, maxLen, "%S", functionName );

                    // Get the offset of the address from the symbol's address.
                    ULONGLONG symbolBaseAddress;
                    pFunc->get_virtualAddress( &symbolBaseAddress );
                    displacement = address - (ULONG)symbolBaseAddress;
                    success = true;

                    // Now try to get the filename and line number.
                    // Get an enumerator that corresponds to this instruction.
                    CComPtr< IDiaEnumLineNumbers > pLines;
                    const DWORD instructionSize = 4;
                    if ( SUCCEEDED( pSession->findLinesByVA( address, instructionSize, &pLines ) ) )
                    {
                        // We could loop over all of the source lines that map to this instruction,
                        // but there is probably at most one, and if there are multiple source
                        // lines we still only want one.
                        CComPtr< IDiaLineNumber > pLine;
                        DWORD celt;
                        if ( SUCCEEDED( pLines->Next( 1, &pLine, &celt ) ) && celt == 1 )
                        {
                            // Get the line number.
                            pLine->get_lineNumber( &lineNumber );

                            // Get the source file object, and then its name.
                            CComPtr< IDiaSourceFile > pSrc;
                            pLine->get_sourceFile( &pSrc );
                            CComBSTR sourceName = 0;
                            pSrc->get_fileName( &sourceName );
                            // Convert from wide characters to ASCII.
                            sprintf_s( filename, "%S", sourceName );
                        }
                    }
                }
            }
        }
    }

    //if( success )
    //{
    //    // Print out the symbol name and the offset of the address from
    //    // that symbol.
    //    printf( "    %8X: %s+%u", address, symbolName, displacement );

    //    // Now print out the filename/linenumber information if we have it.
    //    if( filename[0] )
    //    {
    //        // Get the filename part of the path.
    //        const CHAR* filepart = strrchr( filename, '\\' );
    //        if( filepart )
    //            ++filepart;
    //        else
    //            filepart = filename;

    //        printf(" - %s(%u)\n", filepart, lineNumber);
    //    }
    //    else
    //        printf(" - error getting source hFile/line number\n");
    //}
    //else
    //    printf( "    %8X: Symbol lookup failed.\n", address );

	return success;
}

extern "C" ADDRTONAMEDLL_API bool GetFilenamePosition(unsigned int address, char* buffer, int maxLen, int& linePos)
{
    bool success = false;
    //CHAR symbolName[1000] = {0};
    ULONG displacement = 0;
    //CHAR filename[500] = {0};
    ULONG lineNumber = 0;

    // Scan through the list of loaded modules to find the one that contains the
    // requested address.
    for( size_t i = 0; i < m_ModuleList.size(); ++i )
    {
        // Find what module's address range the address falls in.
        if( address > m_ModuleList[i].m_Address && address < m_ModuleList[i].m_Address + m_ModuleList[i].m_Size )
        {
            CComPtr<IDiaSession>& pSession = m_ModuleList[i].m_psession;

            // Find the symbol using the virtual address--the raw address. This
            // only works if you have previously told DIA where the module was
            // loaded, using put_loadAddress.
            // Specify SymTagPublicSymbol instead of SymTagFunction if you want
            // the full decorated names.
            IDiaSymbol* pFunc;
            HRESULT result = pSession->findSymbolByVA( address, SymTagFunction, &pFunc );
            if ( SUCCEEDED( result ) && pFunc )
            {
                // Get the name of the function.
                CComBSTR functionName = 0;
                pFunc->get_name( &functionName );
                if( functionName )
                {
                    // Get the offset of the address from the symbol's address.
                    ULONGLONG symbolBaseAddress;
                    pFunc->get_virtualAddress( &symbolBaseAddress );
                    displacement = address - (ULONG)symbolBaseAddress;
                    success = true;

                    // Now try to get the filename and line number.
                    // Get an enumerator that corresponds to this instruction.
                    CComPtr< IDiaEnumLineNumbers > pLines;
                    const DWORD instructionSize = 4;
                    if ( SUCCEEDED( pSession->findLinesByVA( address, instructionSize, &pLines ) ) )
                    {
                        // We could loop over all of the source lines that map to this instruction,
                        // but there is probably at most one, and if there are multiple source
                        // lines we still only want one.
                        CComPtr< IDiaLineNumber > pLine;
                        DWORD celt;
                        if ( SUCCEEDED( pLines->Next( 1, &pLine, &celt ) ) && celt == 1 )
                        {
                            // Get the line number.
                            pLine->get_lineNumber( &lineNumber );
							linePos = lineNumber;

                            // Get the source file object, and then its name.
                            CComPtr< IDiaSourceFile > pSrc;
                            pLine->get_sourceFile( &pSrc );
                            CComBSTR sourceName = 0;
                            pSrc->get_fileName( &sourceName );
                            // Convert from wide characters to ASCII.
                            sprintf_s(buffer, maxLen, "%S", sourceName );
                        }
                    }
                }
            }
        }
    }

	return success;
}


// This is the constructor of a class that has been exported.
// see AddrToNameDLL.h for the class definition
CAddrToNameDLL::CAddrToNameDLL()
{
}

CAddrToNameDLL::~CAddrToNameDLL()
{
}