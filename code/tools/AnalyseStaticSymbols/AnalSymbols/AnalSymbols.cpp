// AnalSymbols.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <core/core.h>
#include <core/containers/string.h>

#include "windows.h"
#include "dbghelp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

ULONG g_cutoffPoint(0);
int g_totalSize(0);
int g_funcSize(0);
int g_dataSize(0);

HANDLE g_hProcess;

const char* GetErrorText(DWORD dwErr)
{ 
	static char errMsg[1024];
    char* lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dwErr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

	strcpy_s(errMsg, _countof(errMsg), lpMsgBuf); 
    
    LocalFree(lpMsgBuf);

	return errMsg;
}

char* getEnvVar(const char* name)
{
	size_t requiredSize;

	getenv_s(&requiredSize, NULL, 0, name);

	char* ptr = new char[requiredSize];

	// Get the value of the environment variable.
	getenv_s(&requiredSize, ptr, requiredSize, name);

	return ptr;
}

BOOL LoadDbgHelp()
{
    char* xedkDir = getEnvVar("xedk");

    char dbgHelpPath[500];

    // Set the xedkDir value to blank if no %xedk% var exists in the environment.
    if (!xedkDir)
	{
        xedkDir = new char[1];
		xedkDir[0] = '\0';
	}

    sprintf_s(dbgHelpPath, _countof(dbgHelpPath), "%s\\bin\\win32\\dbghelp.dll", xedkDir);

    // Call LoadLibrary on DbgHelp.DLL.
    HMODULE hDbgHelp = LoadLibrary(dbgHelpPath);

    // Print an error message and return zero if DbgHelp.DLL didn't load.
    if (!hDbgHelp)
    {
        if ( xedkDir[0] )
            printf("\nERROR:\n\nCouldn't load dbghelp.dll.\n");
        else
            printf("\nERROR:\n\nCouldn't load dbghelp.dll\nXEDK environment variable not set.\n");
        
		delete [] xedkDir;
		return false;
    }

	delete [] xedkDir;

    // DbgHelp.DLL loaded.  Return 1 for success.
    return true;
}

#include "cvconst.h"
#include <assert.h>

FILE *fhOutput(NULL);
int row(1), cell(1);
bool findData(true);

class Entry
{
public:
	Util::C_String	name;
	size_t			size;

	bool operator < (const Entry &B) const
	{
		//return strcmp(pNodeName->m_name, B.pNodeName->m_name)<0;
		return size > B.size;
	}
};

Ut::UtVector<Ut::DynamicVector<Entry> >  dataEntries;
Ut::UtVector<Ut::DynamicVector<Entry> >  funcEntries;

void xml_OutputToFile(const char *c, ...)
{
	char string[4096];
	va_list arglist;
	
	va_start(arglist, c);
	_vsnprintf_s(string, _countof(string), _TRUNCATE, c, arglist);
	va_end(arglist);

	assert(strlen(string) < _countof(string));

	fprintf(fhOutput, string);
	fprintf(fhOutput, "\n");
}

void xml_StartSheet(char *name)
{
	xml_OutputToFile("<Worksheet ss:Name=\"%s\">", name);
	xml_OutputToFile("<Table ss:DefaultRowHeight=\"16.000000\">");
	row=1;
	cell=1;
}

void xml_EndSheet(char *special)
{
	xml_OutputToFile("</Table>");
	if (special)
		xml_OutputToFile(special);
	xml_OutputToFile("</Worksheet>");
}

void xml_StartRow(char *extra=NULL)
{
	if (!extra)
		xml_OutputToFile("<Row ss:Index=\"%d\">", row);
	else
		xml_OutputToFile("<Row ss:Index=\"%d\" %s>", row, extra);
}

void xml_EndRow()
{
	xml_OutputToFile("</Row>");
	row++;
	cell=1;
}

void xml_StartCell(char *extra=NULL)
{
	if (!extra)
		xml_OutputToFile("<Cell ss:Index=\"%d\">", cell);
	else
		xml_OutputToFile("<Cell ss:Index=\"%d\" %s>", cell, extra);
}

void xml_EndCell()
{
	xml_OutputToFile("</Cell>");
	cell++;
}

BOOL CALLBACK SymEnumSymbolsProc(
  PSYMBOL_INFO pSymInfo,
  ULONG SymbolSize,
  PVOID UserContext
)
{
	g_totalSize += pSymInfo->Size;

	char *type = "Unknown";

	Entry e;
	e.name = pSymInfo->Name;
	e.size = pSymInfo->Size;

	switch (pSymInfo->Tag)
	{
	case SymTagFunction:
		type = "Function";
		g_funcSize += pSymInfo->Size;

		if (pSymInfo->Size > g_cutoffPoint)
			funcEntries.PushBack(e);

		break;

	case SymTagData:
		type = "Data";
		g_dataSize += pSymInfo->Size;

		if (pSymInfo->Size > g_cutoffPoint)
			dataEntries.PushBack(e);

		break;

	case SymTagPublicSymbol:
		{
		int mrf = 10;
		break;
		}

	case SymTagNull:
	case SymTagExe:
	case SymTagCompiland:
	case SymTagCompilandDetails:
	case SymTagCompilandEnv:
	case SymTagBlock:
	case SymTagAnnotation:
	case SymTagLabel:
	case SymTagUDT:
	case SymTagEnum:
	case SymTagFunctionType:
	case SymTagPointerType:
	case SymTagArrayType:
	case SymTagBaseType:
	case SymTagTypedef:
	case SymTagBaseClass:
	case SymTagFriend:
	case SymTagFunctionArgType:
	case SymTagFuncDebugStart:
	case SymTagFuncDebugEnd:
	case SymTagUsingNamespace:
	case SymTagVTableShape:
	case SymTagVTable:
	case SymTagCustom:
	case SymTagThunk:
	case SymTagCustomType:
	case SymTagManagedType:
	case SymTagDimension:
	default:
		{
		int mrf = 10;
		break;
		}
	}

	if (pSymInfo->Size > g_cutoffPoint)
	{
		//printf("%d,%s\n", pSymInfo->Size, pSymInfo->Name);

		//if (pSymInfo->Size == 16984)

		//switch (pSymInfo->Tag)
		//{
		//case SymTagFunction:
		//	fprintf(fhOutput, "%d,%s,%s\n", pSymInfo->Size, type, pSymInfo->Name);
		//	break;

		//case SymTagData:
		//	fprintf(fhOutput, "%d,%s,%s\n", pSymInfo->Size, type, pSymInfo->Name);
		//	break;

		//default:
		//	fprintf(fhOutput, "%d,%s,%s\n", pSymInfo->Size, type, pSymInfo->Name);
		//	break;
		//}
	}

	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
    if(argc < 2)
	{
        fprintf(stderr,"Usage: AnalSymbols exePath output.csv cutoffValue\n");
        exit(1);
    }

	if (argc == 4)
	{
		g_cutoffPoint = atoi(argv[3]);
	}

    if(!LoadDbgHelp()) {
        fprintf(stderr,"Loading XDK DbgHelp.dll failed\n");
        exit(2);
    }

    // Init the symbol API with an arbitrary unique number
    // this number is allowed to be the process handle but not required to be.
    DWORD err;
    g_hProcess = GetCurrentProcess(); 
	BOOL b = SymInitialize( g_hProcess, NULL, false );
    if( b == 0 )
    {
        err = GetLastError();
        printf("DBG : Error SymInitialize %#08lx\n", err );
        return 1;
    }

    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_EXACT_SYMBOLS | SYMOPT_DEBUG | SYMOPT_ALLOW_ABSOLUTE_SYMBOLS );

    unsigned long ulModAddress = 0x82000000 ; // default value anyway

    // Load the exe or DLL that you want symbols for.
    ULONG64 lm = SymLoadModule64( g_hProcess,  NULL, argv[1],  NULL, ulModAddress,  0 );
    err = GetLastError();
	if (lm == 0)
    {
		printf("symFile: %s\n", argv[1]);
        printf("DBG    : Error SymLoadModule64 %#08lx\n (%s)\n", err, GetErrorText(err));

		return 1;
    }

	char outputFilename[MAX_PATH];
	if (argc < 3)
	{
		strcpy_s(outputFilename, _countof(outputFilename), argv[1]);
		strcat_s(outputFilename, _countof(outputFilename), ".static-analysis.xls");
	}
	else
	{
		strcat_s(outputFilename, _countof(outputFilename), argv[2]);
	}

	errno_t en = fopen_s(&fhOutput, outputFilename, "w");
	if (fhOutput)
	{
		xml_OutputToFile("<?xml version=\"1.0\"?>");
		xml_OutputToFile("<?mso-application progid=\"Excel.Sheet\"?>");
		xml_OutputToFile("<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\" xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\" xmlns:html=\"http://www.w3.org/TR/REC-html40\">");
		xml_OutputToFile("");

		xml_OutputToFile("<Styles>");
			xml_OutputToFile("<Style ss:ID=\"TypeStyle\">");
				xml_OutputToFile("<Alignment ss:Horizontal=\"Center\"/>");
			xml_OutputToFile("</Style>");

			xml_OutputToFile("<Style ss:ID=\"MyNumber\">");
				xml_OutputToFile("<NumberFormat ss:Format=\"#,##0\"/>");
			xml_OutputToFile("</Style>");

			xml_OutputToFile("<Style ss:ID=\"Greyed Bold\">");
				xml_OutputToFile("<Font ss:FontName=\"Verdana\" ss:Size=\"10\" ss:Bold=\"1\"/>");
				xml_OutputToFile("<NumberFormat ss:Format=\"#,##0\"/>");
				xml_OutputToFile("<Interior ss:Color=\"#c0c0c0\" ss:Pattern=\"Solid\"/>");
				xml_OutputToFile("<Alignment/>");
			xml_OutputToFile("</Style>");

		xml_OutputToFile("</Styles>");
		xml_OutputToFile("");

		printf("Scanning the symbols...\n");

		if( SymEnumSymbols( g_hProcess, ulModAddress, 0, SymEnumSymbolsProc, (void*)0 ) == 0)
		{
			DWORD err = GetLastError();
			printf("DBG : Error SymEnumSymbols %#08lx\n (%s)\n", err, GetErrorText(err));
			return 1;
		}

		printf("Sorting them...\n");

		// Sort them now
		std::sort(&dataEntries[0], &dataEntries[dataEntries.Size()-1] + 1);
		std::sort(&funcEntries[0], &funcEntries[funcEntries.Size()-1] + 1);

		printf("Writing them...\n");

		xml_StartSheet("Data");
		{
			xml_OutputToFile("<Column ss:Index=\"1\" ss:Width=\"82.000000\"/>");
			xml_OutputToFile("<Column ss:Index=\"2\" ss:Width=\"96.000000\"/>");
			xml_OutputToFile("<Column ss:Index=\"3\" ss:Width=\"564.000000\"/>");

			xml_StartRow();
			xml_StartCell();
			xml_OutputToFile("<Data ss:Type=\"String\">Total Size:</Data>");
			xml_EndCell();
			xml_StartCell("ss:Formula=\"=SUBTOTAL(9,R[3]C[-1]:R[65532]C[-1])\" ss:StyleID=\"MyNumber\"");
			xml_OutputToFile("<Data ss:Type=\"Number\">0</Data>");
			xml_EndCell();
			xml_EndRow();
			xml_StartRow();
			xml_StartCell();
			xml_OutputToFile("<Data ss:Type=\"String\">Total Count:</Data>");
			xml_EndCell();
			xml_StartCell("ss:Formula=\"=SUBTOTAL(2,R[2]C[-1]:R[65531]C[-1])\" ss:StyleID=\"MyNumber\"");
			xml_OutputToFile("<Data ss:Type=\"Number\">0</Data>");
			xml_EndCell();
			xml_EndRow();

			xml_StartRow("ss:StyleID=\"Greyed Bold\"");
			xml_StartCell();
			xml_OutputToFile("<Data ss:Type=\"String\">Size</Data>");
			xml_EndCell();
			xml_StartCell();
			xml_OutputToFile("<Data ss:Type=\"String\">Type</Data>");
			xml_EndCell();
			xml_StartCell();
			xml_OutputToFile("<Data ss:Type=\"String\">Name</Data>");
			xml_EndCell();
			xml_EndRow();

			for (int i=0; i < dataEntries.Size() && i < 65535; i++)
			{
				const Entry& e = dataEntries[i];

				Util::C_String symName;

				symName = e.name.Replace("<", "&lt;");
				symName = symName.Replace(">", "&gt;");

				xml_StartRow();
				xml_StartCell("ss:StyleID=\"MyNumber\"");
				xml_OutputToFile("<Data ss:Type=\"Number\">%d</Data>", e.size);
				xml_EndCell();
				xml_StartCell("ss:StyleID=\"TypeStyle\"");
				xml_OutputToFile("<Data ss:Type=\"String\">Data</Data>");
				xml_EndCell();
				xml_StartCell();
				xml_OutputToFile("<Data ss:Type=\"String\">%s</Data>", symName.Chars());
				xml_EndCell();
				xml_EndRow();
			}
		}
		xml_EndSheet("<AutoFilter Range=\"R3C1:R3C3\" xmlns=\"urn:schemas-microsoft-com:office:excel\"/>");

		xml_StartSheet("Functions");
		{
			xml_OutputToFile("<Column ss:Index=\"1\" ss:Width=\"82.000000\"/>");
			xml_OutputToFile("<Column ss:Index=\"2\" ss:Width=\"96.000000\"/>");
			xml_OutputToFile("<Column ss:Index=\"3\" ss:Width=\"564.000000\"/>");
			
			xml_StartRow();
			xml_StartCell();
			xml_OutputToFile("<Data ss:Type=\"String\">Total Size:</Data>");
			xml_EndCell();
			xml_StartCell("ss:Formula=\"=SUBTOTAL(9,R[3]C[-1]:R[65532]C[-1])\" ss:StyleID=\"MyNumber\"");
			xml_OutputToFile("<Data ss:Type=\"Number\">0</Data>");
			xml_EndCell();
			xml_EndRow();
			xml_StartRow();
			xml_StartCell();
			xml_OutputToFile("<Data ss:Type=\"String\">Total Count:</Data>");
			xml_EndCell();
			xml_StartCell("ss:Formula=\"=SUBTOTAL(2,R[2]C[-1]:R[65531]C[-1])\" ss:StyleID=\"MyNumber\"");
			xml_OutputToFile("<Data ss:Type=\"Number\">0</Data>");
			xml_EndCell();
			xml_EndRow();

			xml_StartRow("ss:StyleID=\"Greyed Bold\"");
			xml_StartCell();
			xml_OutputToFile("<Data ss:Type=\"String\">Size</Data>");
			xml_EndCell();
			xml_StartCell();
			xml_OutputToFile("<Data ss:Type=\"String\">Type</Data>");
			xml_EndCell();
			xml_StartCell();
			xml_OutputToFile("<Data ss:Type=\"String\">Name</Data>");
			xml_EndCell();
			xml_EndRow();

			for (int i=0; i < funcEntries.Size() && i < 65535; i++)
			{
				const Entry& e = funcEntries[i];

				Util::C_String symName;

				symName = e.name.Replace("<", "&lt;");
				symName = symName.Replace(">", "&gt;");

				xml_StartRow();
				xml_StartCell();
				xml_OutputToFile("<Data ss:Type=\"Number\">%d</Data>", e.size);
				xml_EndCell();
				xml_StartCell("ss:StyleID=\"TypeStyle\"");
				xml_OutputToFile("<Data ss:Type=\"String\">Function</Data>");
				xml_EndCell();
				xml_StartCell();
				xml_OutputToFile("<Data ss:Type=\"String\">%s</Data>", symName.Chars());
				xml_EndCell();
				xml_EndRow();
			}
		}
		xml_EndSheet("<AutoFilter Range=\"R3C1:R3C3\" xmlns=\"urn:schemas-microsoft-com:office:excel\"/>");

		xml_OutputToFile("</Workbook>");

		fclose(fhOutput);
		
		printf("File      : %s\n", argv[1]);
		printf("Functions : %d\n", g_funcSize);
		printf("Data      : %d\n", g_dataSize);
		printf("Total Size: %d\n", g_totalSize);
	}
	else
	{
		printf("ERROR: Failed to open file. Open in Excel already?\n");
	}

	return 0;
}

