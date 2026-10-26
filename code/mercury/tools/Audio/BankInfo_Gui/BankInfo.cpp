// BankInfo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "BankInfo.h"

#include "Common\WaveBankReader.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

struct App
{
	App()
	{
		_list = 0;
		_txtFileName = 0;
		_txtBankName = 0;
		_txtVersion = 0;
		_txtEndian = 0;
		_txtStreaming = 0;
		_txtNumWaves = 0;
		_txtBuildBy = 0;
		_txtBankNameTitle = 0;
		_txtFileNameTitle = 0;
		_txtVersionTitle = 0;
		_txtEndianTitle = 0;
		_txtStreamingTitle = 0;
		_txtNumWavesTitle = 0;
		_txtBuildByTitle = 0;
		//_lineDividerBankProperties = 0;
		//_lineDividerWaves = 0;
		_txtBankPropertiesTitle = 0;
		_txtWavesTitle = 0;
		_lineDividerTop = 0;

		_pBank = 0;
	}
	~App()
	{
		if (_pBank)
			delete _pBank;
	}

	HWND _list;

	HWND _txtBankPropertiesTitle;
	HWND _txtWavesTitle;
	//HWND _lineDividerBankProperties;
	//HWND _lineDividerWaves;
	HWND _lineDividerTop;

	HWND _txtFileName;
	HWND _txtBankName;
	HWND _txtVersion;
	HWND _txtEndian;
	HWND _txtStreaming;
	HWND _txtNumWaves;
	HWND _txtBuildBy;

	HWND _txtFileNameTitle;
	HWND _txtBankNameTitle;
	HWND _txtVersionTitle;
	HWND _txtEndianTitle;
	HWND _txtStreamingTitle;
	HWND _txtNumWavesTitle;
	HWND _txtBuildByTitle;

	Audio::WaveBank* _pBank;
};

App theApp;

const int kWaveListColumn_Name				= 0;
const int kWaveListColumn_Format			= 1;
const int kWaveListColumn_Frequency			= 2;
const int kWaveListColumn_Channels			= 3;
const int kWaveListColumn_Bits				= 4;
const int kWaveListColumn_Loop				= 5;
const int kWaveListColumn_SizeK				= 6;
const int kWaveListColumn_SizeBytes			= 7;
const int kWaveListColumn_DataStart			= 8;
const int kWaveListColumn_Flags				= 9;
const int kWaveListColumn_Markers			= 10;
//const int kWaveListColumn_Duration			= 8;

const int NumFormatStrings = 11;

char* pchFormatLookup [NumFormatStrings] =
{
	"PCM Float",
	"PCM Signed",
	"PCM Unsigned",
	"PCM Signed",
	"PCM Unsigned",
	"PCM Signed",
	"PCM Unsigned",
	"ADPCM",
	"XMA",
	"XMA2",
	"MP3"
};

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
HWND				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void WriteOutWaveData(Audio::WaveBank* pBank);

bool bDoSystemPause=false;

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void ShowUsage(HWND hWnd)
{
	MessageBox(hWnd, "Usage: BankInfo [/w] bankfile\n\n  /w\t Write out raw wave data to individual files\n\n", "BankInfo", MB_OK | MB_ICONINFORMATION);
	exit(0);
}
//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
const char* LookupFormatString(int format)
{
	static Util::C_String strUnknown = "Unknown Format";

	if (format < 0 || format >= NumFormatStrings)
		return strUnknown.Chars();

	return pchFormatLookup[format];
}

#define MAX_PLACES 10 /* enough places for ULONG_MAX */

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
char* CommaDelimitNumber(unsigned long integer)
{
	static char fdigits[MAX_PLACES + (MAX_PLACES / 3) + 1];
	char digits[MAX_PLACES + 1];
	char *dgt = digits, *fdgt = fdigits;
	int places;

	places = sprintf(digits, "%lu", integer); /* convert integer to string */
	/* places = digits in string */
	while (*fdgt++ = *dgt++) /* while there are more digits*/
	{
		if (--places) /* if places-1 > 0 */
		{
			if ( !(places % 3) ) /* if places is multiple of 3 */
				*fdgt++ = ','; /* insert a comma here */
		}
	}
	return fdigits;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
bool AddEntryToListView(Audio::WaveBank::Wave* pWave, int index)
{		
	if (!pWave)
		return false;

	if (!theApp._list)
		return false;

	LVITEM item = { 0 };

	item.mask		= LVIF_TEXT | LVIF_PARAM;
	item.iItem		= index;
	item.iSubItem	= 0;
	item.pszText	= (LPSTR)pWave->m_baseHeader.pchName;

	int pos = ListView_InsertItem(theApp._list, &item);

	// add to the listview
	if (pos != -1)
	{
		ListView_SetItemText(theApp._list, pos, kWaveListColumn_Channels, (LPSTR)Util::C_String().Printf("%d", pWave->NumChannels()).Chars());
		ListView_SetItemText(theApp._list, pos, kWaveListColumn_Bits, (LPSTR)Util::C_String().Printf("%d", pWave->BitsPerSample()).Chars());
		ListView_SetItemText(theApp._list, pos, kWaveListColumn_Format, (LPSTR)LookupFormatString(pWave->m_baseHeader.uFormat));
		ListView_SetItemText(theApp._list, pos, kWaveListColumn_Frequency, (LPSTR)Util::C_String().Printf("%d", pWave->SampleRate()).Chars());
		if (pWave->DataLength() < 1024)
		{
			ListView_SetItemText(theApp._list, pos, kWaveListColumn_SizeK, (LPSTR)Util::C_String().Printf("%d bytes ", pWave->DataLength()).Chars());
		}
		else
		if (pWave->DataLength() < 1024 * 1024)
		{
			ListView_SetItemText(theApp._list, pos, kWaveListColumn_SizeK, (LPSTR)Util::C_String().Printf("%d k", (int)pWave->DataLength()/1024).Chars());
		}
		else
			ListView_SetItemText(theApp._list, pos, kWaveListColumn_SizeK, (LPSTR)Util::C_String().Printf("%.2f MB", (F32)pWave->DataLength()/1024.f/1024.f).Chars());

		ListView_SetItemText(theApp._list, pos, kWaveListColumn_SizeBytes, (LPSTR)CommaDelimitNumber(pWave->DataLength()));
		ListView_SetItemText(theApp._list, pos, kWaveListColumn_DataStart, (LPSTR)CommaDelimitNumber(pWave->m_baseHeader.uDataStart));
		ListView_SetItemText(theApp._list, pos, kWaveListColumn_Flags, (LPSTR)Util::C_String().Printf("0x%x", pWave->m_baseHeader.uFlags).Chars());
		ListView_SetItemText(theApp._list, pos, kWaveListColumn_Markers, pWave->MarkerChunk() ? "Yes" : "No");

		// make this into a function?
		//F32 duration = pWave->LengthInSecs();
		//F32 n, ms = modf(duration, &n);
		//F32 s = fmod(duration, 60.f);
		//duration = duration / 60.f;
		//int m = (int)fmod(duration, 60.f);
		//duration = duration / 60.f;

		//ListView_SetItemText(theApp._list, pos, kWaveListColumn_Duration, 
		//	(LPSTR)Util::C_String().Printf("%02d:%s%.3f", 
		//	m,											// mins
		//	s < 10.f ? "0" : "",
		//	s											// secs
		//	).Chars());

		ListView_SetItemText(theApp._list, pos, kWaveListColumn_Loop, pWave->Loop() ? "Yes" : "No");
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
bool PopulateGui(Audio::WaveBank* pBank)
{
	if (!pBank)
		return false;

	SetWindowRedraw(theApp._list, FALSE);

	SetWindowText(theApp._txtFileName, pBank->Filename()/*Util::ExtractFilename(pBank->Filename())*/);
	SetWindowText(theApp._txtBankName, pBank->m_BankHeader.pchName);
	SetWindowText(theApp._txtVersion, Util::C_String().Printf("%d", pBank->m_BankHeader.uVersion).Chars());
	SetWindowText(theApp._txtEndian, Util::C_String().Printf("%s", pBank->m_BankHeader.chEndian==0?"Little   [PC]":"Big   [360, PS3]").Chars());
	SetWindowText(theApp._txtStreaming, Util::C_String().Printf("%s", pBank->IsStreamingBank()?"Yes":"No").Chars());
	SetWindowText(theApp._txtNumWaves, Util::C_String().Printf("%d", pBank->m_BankHeader.uNumWaves).Chars());

	for (int i=0; i<(int)pBank->NumWaves(); i++)
	{
		AddEntryToListView(pBank->GetWave(i), i);
	}
	
	SetWindowRedraw(theApp._list, TRUE);

	return true;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
bool ProcessCmdArgs(HWND hWnd, int argc, WCHAR* argv[])
{
	if (argc == 1 || (argc > 1 && (*argv[1] == '?' || _wcsicmp(argv[1], L"/?") == 0 || _wcsicmp(argv[1], L"-help") == 0)))
	{
		ShowUsage(hWnd);
	}

	bool bWriteOutWaveData=false;
	int argNum=1;

	// if arguments start with / or - then see if its a request for writing the wave data
	if ((argv[argNum][0] == '/') || (argv[argNum][0] == '-'))
	{
		if (argv[1][1] == 'w' || argv[1][1] == 'W')
			bWriteOutWaveData = true;
		else
		{
			printf("Unknown switch '%s'\n\n", argv[argNum]);
			ShowUsage(hWnd);
		}

		argNum++;
	}

	if (_wcsicmp(argv[argNum] + wcslen(argv[argNum]) - strlen(Audio::g_pchWbFileExt), L".baf") == 0)
	{
		char pchStr[256] = { 0 };
		wcstombs(pchStr, argv[argNum], 255);

		Audio::WaveBank* pBank = new Audio::WaveBank(pchStr);
		if (pBank)
		{
			//Audio::WaveBank bank(pchStr);
			Audio::WaveBankReader reader(pBank);

			if (reader.Read(!bWriteOutWaveData)!=Audio::Success)
 				exit(0);

			//PopulateGui(bank);

			if (bWriteOutWaveData)
				WriteOutWaveData(pBank);

			theApp._pBank = pBank;
		}
	}
	else
	{
		printf("Unknown argument '%s'\n\n", argv[argNum]);
		ShowUsage(hWnd);
	}
	return true;
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void WriteOutWaveData(Audio::WaveBank* pBank)
{
	printf("\nWriting out raw wav data...");

	Util::C_String strPath = pBank->Filename()/*Util::ExtractDirectory(pBank->Filename())*/;
	Util::C_String strExt = "unknown";

	for (U32 i=0; i<pBank->NumWaves(); i++)
	{
		Audio::WaveBank::Wave* wave = pBank->GetWave(i);

		Audio::File f;

		switch (wave->m_baseHeader.uFormat)
		{
			case Audio::Core::uPCM16BitSignedInt: 
			case Audio::Core::uPCM32BitFloat: 
				strExt = "wav";
				break;
			case Audio::Core::uXMA16Bit: 
				strExt = "xma";
				break;
			case Audio::Core::uADPCM4Bit: 
				strExt = "adpcm";
				break;
		}

		Util::C_String fname = Util::C_String().Printf("%s%s.%s.raw", strPath.Chars(), wave->m_baseHeader.pchName, strExt.Chars());
		printf("\n%s ...", fname.Chars());
		f.OpenWrite(fname.Chars());
		f.Write(wave->m_pvWaveData, wave->m_baseHeader.uDataLength);
		f.Close();

		printf("ok");
		
	}
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void ErrorMsgFn(const char* pchText, void* pContext)
{
	HWND hWnd = (HWND)pContext;
	MessageBox(hWnd, pchText, "BankInfo", MB_OK | MB_ICONERROR);
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void WarningMsgFn(const char* pchText, void* pContext)
{
	HWND hWnd = (HWND)pContext;
	MessageBox(hWnd, pchText, "BankInfo", MB_OK | MB_ICONWARNING);
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
void DisplayMessageFn(U32 attribs, const char* pchText, void* pContext)
{
	HWND hWnd = (HWND)pContext;
	MessageBox(hWnd, pchText, "BankInfo", MB_OK | MB_ICONINFORMATION);
}

//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	//UNREFERENCED_PARAMETER(lpCmdLine);

	INITCOMMONCONTROLSEX iccex;

	iccex.dwSize	= sizeof(iccex);
	iccex.dwICC		= ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_PROGRESS_CLASS;

	InitCommonControlsEx(&iccex);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_BANKINFO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	HWND  hWnd = InitInstance (hInstance, nCmdShow);

	if (!hWnd)
	{
		return FALSE;
	}

	// setup callbacks so any errors get reported via a message box
	SetErrorReportCallback(ErrorMsgFn, hWnd);
	SetWarningReportCallback(WarningMsgFn, hWnd);
	Audio::SetPrintStringCallback(DisplayMessageFn, hWnd);


	WCHAR** pwchArglist;
	int nArgs;
//	int i;

	pwchArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if( NULL == pwchArglist )
	{
		OutputDebugString("CommandLineToArgvW failed\n");
		return 0;
	}
	
	ProcessCmdArgs(hWnd, nArgs, pwchArglist);
	LocalFree(pwchArglist);


	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BANKINFO));

	PopulateGui(theApp._pBank);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BANKINFO));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	//wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	//wcex.hbrBackground	= GetSysColorBrush(COLOR_MENU)/*(HBRUSH)(COLOR_WINDOW+1)*/;
	wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_BANKINFO);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	CW_USEDEFAULT, 0, 800/*CW_USEDEFAULT*/, 600, NULL, NULL, hInstance, NULL);

	if (hWnd)
	{
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);
	}

	return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
		case WM_CREATE:
		{
			if (1)
			{
				theApp._lineDividerTop = CreateWindowEx(0, WC_STATIC, "",  0X10 | WS_CHILD | WS_VISIBLE, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);

				/*theApp._lineDividerBankProperties = CreateWindowEx(0, WC_STATIC, "",  0X10 | WS_CHILD | WS_VISIBLE, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);*/
				theApp._txtBankPropertiesTitle = CreateWindowEx(0, WC_STATIC, "  Bank Properties",  WS_CHILD | WS_VISIBLE, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);

				/*theApp._lineDividerWaves = CreateWindowEx(0, WC_STATIC, "",  0X10 | WS_CHILD | WS_VISIBLE, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);*/
				theApp._txtWavesTitle = CreateWindowEx(0, WC_STATIC, "Waves:",  WS_CHILD | WS_VISIBLE, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);

				theApp._txtFileName = CreateWindowEx(0, WC_STATIC, "",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);

				theApp._txtBankName = CreateWindowEx(0, WC_STATIC, "",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);
				
				theApp._txtVersion = CreateWindowEx(0, WC_STATIC, "",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);

				theApp._txtEndian = CreateWindowEx(0, WC_STATIC, "",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);

				theApp._txtStreaming = CreateWindowEx(0, WC_STATIC, "",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);

				theApp._txtNumWaves = CreateWindowEx(0, WC_STATIC, "",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);


				theApp._txtFileNameTitle = CreateWindowEx(0, WC_STATIC, "File Name:",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);

				theApp._txtBankNameTitle = CreateWindowEx(0, WC_STATIC, "Internal Name:",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);
				
				theApp._txtVersionTitle = CreateWindowEx(0, WC_STATIC, "Version:",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);

				theApp._txtEndianTitle = CreateWindowEx(0, WC_STATIC, "Endian:",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);

				theApp._txtStreamingTitle = CreateWindowEx(0, WC_STATIC, "Streaming:",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);

				theApp._txtNumWavesTitle = CreateWindowEx(0, WC_STATIC, "Number of Waves:",  WS_CHILD | WS_VISIBLE /* | WS_BORDER */, 
					0, 0, 0, 0, hWnd, (HMENU)(intptr_t)0, 0, 0);


				RECT rect;
				GetClientRect(hWnd, &rect);

				if ((theApp._list = CreateWindowEx(
						WS_EX_CLIENTEDGE, WC_LISTVIEW, _T(""), LVS_REPORT | WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_SHOWSELALWAYS | LVS_SORTASCENDING, 
						20, 20, 
						100, 100, 
						hWnd, (HMENU)(intptr_t)0, 0, 0
					)) == 0)
				{
					OutputDebugString("FAILED TO CREATE LIST\n");
					return -1;
				}

				///**DragAcceptFiles(*this, true);*/

				ListView_SetExtendedListViewStyleEx(theApp._list,
						LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP,
						LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP
					);

				GetClientRect(hWnd, &rect);

				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Name");
					col.cx		= 200;

					ListView_InsertColumn(theApp._list, kWaveListColumn_Name, &col);
				}

				GetClientRect(hWnd, &rect);

				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Format");
					col.cx		= 80;

					ListView_InsertColumn(theApp._list, kWaveListColumn_Format, &col);
				}
				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Frequency");
					col.cx		= 70;

					ListView_InsertColumn(theApp._list, kWaveListColumn_Frequency, &col);
				}
				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Channels");
					col.cx		= 60;

					ListView_InsertColumn(theApp._list, kWaveListColumn_Channels, &col);
				}
				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Bits");
					col.cx		= 40;

					ListView_InsertColumn(theApp._list, kWaveListColumn_Bits, &col);
				}
				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Loop");
					col.cx		= 40;

					ListView_InsertColumn(theApp._list, kWaveListColumn_Loop, &col);
				}
				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Size in Bytes");
					col.cx		= 80;

					ListView_InsertColumn(theApp._list, kWaveListColumn_SizeBytes, &col);
				}

				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Size");
					col.cx		= 80;

					ListView_InsertColumn(theApp._list, kWaveListColumn_SizeK, &col);
				}

				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Data Start");
					col.cx		= 80;

					ListView_InsertColumn(theApp._list, kWaveListColumn_DataStart, &col);
				}

				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Flags");
					col.cx		= 80;

					ListView_InsertColumn(theApp._list, kWaveListColumn_Flags, &col);
				}

				{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Markers");
					col.cx		= 80;

					ListView_InsertColumn(theApp._list, kWaveListColumn_Markers, &col);
				}

				// this doesnt work for all formats.
				// store in wavebank struct? would be useful for reference/debugging even though not used by the engine
				/*{
					LVCOLUMN col = { 0 };

					col.mask	= LVCF_TEXT | LVCF_WIDTH;
					col.pszText	= _T("Length (mm:ss.ms)");
					col.cx		= 100;

					ListView_InsertColumn(theApp._list, kWaveListColumn_Duration, &col);
				}*/

				HWND h = GetFirstChild(hWnd);
				while (h != 0)
				{
					SetWindowFont(h, GetStockObject(DEFAULT_GUI_FONT), false);
					h = GetNextSibling(h);
				}
			}
		}
		break;

		case WM_SIZE:
		{	
			RECT rect = { 0 };
			if (GetClientRect(hWnd, &rect)==false)
				OutputDebugString("GetClientRect() failed\n");
		
			int curY = 0;
			int vertMargin = 8;
			int horizMargin = 8;
			int leftEdge = rect.left + vertMargin;
			int leftEdgeIndented = rect.left + vertMargin*2;
			int rightEdge = rect.right - vertMargin;
			int textMargin = rect.left + 120;
			int vertTextSpace = 16;
			int sectionSpace = 8;

			SetWindowPos(theApp._lineDividerTop, 0, rect.left, rect.top, rect.right-rect.left, 2, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += 10;

			//SetWindowPos(theApp._txtBankPropertiesTitle, 0, leftEdge, rect.top + curY, 80, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(theApp._txtBankPropertiesTitle, 0, 0, rect.top, rect.right, rect.bottom, SWP_NOACTIVATE | SWP_NOZORDER);
			//SetWindowPos(theApp._lineDividerBankProperties, 0, leftEdge, rect.top + curY+16, rect.right-rect.left-vertMargin, 2, SWP_NOACTIVATE | SWP_NOZORDER);
			curY+= vertTextSpace;

			SetWindowPos(theApp._txtFileNameTitle, 0, leftEdgeIndented, rect.top + curY, 50, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(theApp._txtFileName, 0, textMargin, rect.top + curY, 400, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += vertTextSpace;

			SetWindowPos(theApp._txtBankNameTitle, 0, leftEdgeIndented, rect.top + curY, 80, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(theApp._txtBankName, 0, textMargin, rect.top + curY, 200, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += vertTextSpace;

			SetWindowPos(theApp._txtVersionTitle, 0, leftEdgeIndented, rect.top + curY, 50, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(theApp._txtVersion, 0, textMargin, rect.top + curY, 200, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += vertTextSpace;

			SetWindowPos(theApp._txtEndianTitle, 0, leftEdgeIndented, rect.top + curY, 50, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(theApp._txtEndian, 0, textMargin, rect.top + curY, 200, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += vertTextSpace;

			SetWindowPos(theApp._txtStreamingTitle, 0, leftEdgeIndented, rect.top + curY, 50, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(theApp._txtStreaming, 0, textMargin, rect.top + curY, 200, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += vertTextSpace;

			SetWindowPos(theApp._txtNumWavesTitle, 0, leftEdgeIndented, rect.top + curY, 90, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			SetWindowPos(theApp._txtNumWaves, 0, textMargin, rect.top + curY, 200, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			curY += vertTextSpace+sectionSpace;

			SetWindowPos(theApp._txtWavesTitle, 0, leftEdge, rect.top + curY, 65, 16, SWP_NOACTIVATE | SWP_NOZORDER);
			//SetWindowPos(theApp._txtWavesTitle, 0, 0, 0, rect.right, rect.bottom, SWP_NOACTIVATE | SWP_NOZORDER);

			//SetWindowPos(theApp._lineDividerWaves, 0, leftEdge+40, rect.top + curY+4, rect.right-rect.left-40-vertMargin, 2, SWP_NOACTIVATE | SWP_NOZORDER);
			curY+= 16;

			SetWindowPos(theApp._list, 0, 
				leftEdge, rect.top + curY, 
				rect.right - rect.left - vertMargin*2, rect.bottom-rect.top-curY-vertMargin, 
				SWP_NOACTIVATE | SWP_NOZORDER);
		}
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			PostQuitMessage(0);


		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		//ExtTextOut(ps.hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, 0, 0, 0);
		EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_NOTIFY:
	{
		NMHDR* hdr = (NMHDR*)lParam;
		if (hdr->code == LVN_KEYDOWN)
		{
			NMLVKEYDOWN* nmlKeyDown = (NMLVKEYDOWN*) lParam; 
			if (nmlKeyDown->wVKey == VK_ESCAPE)
				PostQuitMessage(0);

			static bool bToggle=false;
			if (bToggle)
			{
				ListView_SetItemState(theApp._list, 0xffffffff, LVIS_SELECTED, LVIS_SELECTED);
			}
			else
			{
				ListView_SetItemState(theApp._list, 0xffffffff, 0, LVIS_SELECTED);
			}
			bToggle = !bToggle;
		}
		}
		break;
	//case WM_CTLCOLORSTATIC:
		//return (int)(void*)GetSysColorBrush(COLOR_MENU);
	/*case WM_CTLCOLOREDIT :
		return (int)(void*)GetSysColorBrush(COLOR_MENU);*/
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
