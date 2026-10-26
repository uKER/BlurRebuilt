
#include "stdafx.h"
#include "app.h"
#include <share.h>
#include <ShellAPI.h>
#include "CaptureClient.h"
#include "DiskWriter.h"
#include "Xenon.h"
#include <winsock.h>

HWND MainWnd=NULL;
static NOTIFYICONDATA Nid;
static FILE *DebugFile=NULL;
static HICON Icon[4]={0};
static int MainVer=0x1017;
char MainPath[260]="C:\\Screens";
static HMENU Menu=NULL;

static HBITMAP Bitmap = NULL;
unsigned char *BitmapPixels = NULL;
int ShowImages = 0;

// Log to debug file 'zout.txt':
int dprintf(char *format, ...)
{
	va_list val=NULL;

	if (DebugFile==NULL) DebugFile=_fsopen("zout.txt","wt",_SH_DENYNO);
	if (DebugFile==NULL) return 1;

	va_start(val,format);
	vfprintf(DebugFile,format,val);
	fflush(DebugFile);

	va_end(val);
	return 0;
}

static int UpdateDialog()
{
	SetWindowText(GetDlgItem(MainWnd,IDC_EDIT1),MainPath);

	return 0;
}

static void OpenFolder()
{
	ShellExecute(NULL,"explore",MainPath,NULL,NULL,SW_NORMAL);
}

static void OpenApp()
{
	SetForegroundWindow(MainWnd);
	ShowWindow(MainWnd,SW_NORMAL);
}

static void ConnectToIp()
{
	// Get IP address to connect to:	
	char ipString[128] = {0};
	GetWindowText(GetDlgItem(MainWnd, IDC_EDIT2), ipString, sizeof(ipString)-1);

	unsigned int ip = htonl(inet_addr(ipString));
	if (ip == 0)
		return;

	g_CaptureClient.Connect(ip); // Connect asynchronous
}

static bool PhotoCheckBox()
{
	return SendMessage(GetDlgItem(MainWnd,IDC_PHOTO),BM_GETCHECK,0,0) == BST_CHECKED;
}

// Dialog message proc
static INT_PTR CALLBACK DialogProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	if (msg==0x400)
	{
		// Message from the tray icon:
		if (LOWORD(lparam)==WM_LBUTTONDBLCLK)
		{
			OpenApp();
			return 1;
		}

		if (LOWORD(lparam)==WM_RBUTTONDOWN)
		{
			// Right-click menu:
			POINT pt={0,0};
			GetCursorPos(&pt);
			
			SetForegroundWindow(hwnd);
			TrackPopupMenu(Menu,TPM_RIGHTBUTTON|TPM_RIGHTALIGN,pt.x,pt.y,0,hwnd,NULL);
			PostMessage(hwnd,WM_NULL,0,0);
			return 1;
		}
	}

	if (msg==WM_SYSCOMMAND && wparam==SC_MINIMIZE) { ShowWindow(hwnd,SW_HIDE); return 1; }

	if (msg==WM_COMMAND)
	{
		int id=0,notify=0;
		id=LOWORD(wparam); notify=HIWORD(wparam);
		
		if (id==IDC_OPEN_APP) 
		{
			OpenApp();
			return 1;
		}

		if (id==IDC_FOLDER)
		{
			OpenFolder();
			return 1;
		}

		if (id==IDC_EXIT_APP)
		{
			SendMessage(hwnd,WM_CLOSE,0,0);
			return 1;
		}
		
		if (id==IDC_EDIT1 && notify==EN_CHANGE)
		{
			GetWindowText(GetDlgItem(hwnd,id),MainPath,sizeof(MainPath)-1);
			g_CaptureClient.DiskWriter()->SetPath(MainPath);
			
			ConfigSave(); // Save change to config file
			return 1;
		}
		
		if (id == IDC_CONNECT)
		{
			ConnectToIp();
			return 1;
		}

		if (id == IDC_DISCONNECT)
		{
			g_CaptureClient.Disconnect();
			return 1;
		}
		
		if (id == IDC_SCREEN_CAPTURE)
		{
			g_CaptureClient.ScreenCapture(false);
			return 1;
		}

		if (id == IDC_PHOTO_CAPTURE)
		{
			g_CaptureClient.ScreenCapture(true);
			return 1;
		}
	}

	if (msg==WM_CLOSE) { DestroyWindow(hwnd); return 1; }
	if (msg==WM_DESTROY) { PostQuitMessage(0); return 1; }

	if (msg == WM_PAINT) MainShowImage();
	if (msg == WM_SIZE) MainShowImage();

	return 0;
}

// Insert an icon into the system tray
static int TrayInit()
{
	memset(&Nid,0,sizeof(Nid));

	Nid.cbSize=sizeof(Nid);
	Nid.hWnd=MainWnd;
	Nid.uFlags=NIF_MESSAGE | NIF_ICON | NIF_TIP;

	Nid.uCallbackMessage=0x400;
	Nid.hIcon=Icon[2];
	strcpy(Nid.szTip,"XenWatch");

	Shell_NotifyIcon(NIM_ADD,&Nid);

	// Make right-click menu:
	Menu=CreatePopupMenu();
	AppendMenu(Menu,MF_STRING,IDC_OPEN_APP,"Open XenWatch");
	AppendMenu(Menu,MF_STRING,IDC_SCREEN_CAPTURE,"Screen Capture");
	AppendMenu(Menu,MF_STRING,IDC_FOLDER,"Open Folder");
	AppendMenu(Menu,MF_SEPARATOR,0,"");
	AppendMenu(Menu,MF_STRING,IDC_EXIT_APP,"Exit");
	SetMenuDefaultItem(Menu,IDC_OPEN_APP,0);
	return 0;
}

int TrayChange(int type)
{
	Nid.hIcon=Icon[type&3];

	Shell_NotifyIcon(NIM_MODIFY,&Nid);
	return 0;
}

int TrayInfo(const char *text)
{
	if (text)
	{
		Nid.uFlags |= NIF_INFO;
		strcpy(Nid.szInfo, text);
	}
	else
	{
		Nid.uFlags &= ~NIF_INFO;
		Nid.szInfo[0] = 0;
	}
	
	Shell_NotifyIcon(NIM_MODIFY, &Nid);
	return 0;
}

static int TrayExit()
{
	DestroyMenu(Menu);
	Menu=NULL;

	Shell_NotifyIcon(NIM_DELETE,&Nid);
	return 0;
}

void MainShowImage()
{
	ShowImages = ( SendMessage(GetDlgItem(MainWnd,IDC_CHECK1),BM_GETCHECK,0,0) == BST_CHECKED );

	if (!ShowImages || !IsWindowVisible(MainWnd))
		return;

	// Blit bitmap to the screen:
	HDC hdc = GetDC(MainWnd);
	HDC bitdc = CreateCompatibleDC(hdc);
	HBITMAP bitPrev = (HBITMAP)SelectObject(bitdc, Bitmap);

	RECT rect={0,0,0,0};
	GetClientRect(MainWnd,&rect);
	
	int offset = 192;
	int width = rect.right;
	int height = rect.bottom - offset;
	
	if (width  > 1280) width  = 1280;
	if (height > 720 ) height = 720;
	
	SetStretchBltMode(hdc, COLORONCOLOR);
	StretchBlt(hdc, 0,offset,width,height, bitdc, 0,0,1280,720, SRCCOPY);
	UpdateWindow(MainWnd);
	
	SelectObject(bitdc, bitPrev);
	DeleteDC(bitdc);
	ReleaseDC(MainWnd, hdc);
}

static void ComboAdd(char *text)
{
	SendMessage(GetDlgItem(MainWnd,IDC_COMBO1), CB_ADDSTRING, BST_CHECKED, (LPARAM)text);
}

static void FpsAdd(const char *text, bool selected = false)
{
	HWND hc = GetDlgItem(MainWnd,IDC_COMBO2);
	SendMessage(hc, CB_ADDSTRING, BST_CHECKED, (LPARAM)text);
	if (selected)
		SetWindowText(hc, text);
}

static float DriveFree(const char *drive)
{
	DWORD totalClusters = 0, freeClusters = 0, sectors = 0, bytes = 0;
	GetDiskFreeSpace(drive, &sectors, &bytes, &freeClusters, &totalClusters);

	float megabytes = (float)sectors * (float)bytes / 1048576.f;
	return (float)freeClusters * megabytes;
}

static int LastCheck = 0, LastStatus = 0;
static int Remain = -100; // Remaining screenshots to copy
static int InitialRemain = 0; // Initial number of frames to copy

static int RemainValue()
{
	char text[32] = {0};
	GetWindowText(GetDlgItem(MainWnd, IDC_COMBO1), text, sizeof(text) - 1);
	if (text[0])
		return strtol(text, NULL, 0);

	return -100;
}

static float FpsValue()
{
	char text[32] = {0};
	GetWindowText(GetDlgItem(MainWnd, IDC_COMBO2), text, sizeof(text) - 1);
	if (text[0])
		return (float)strtod(text, NULL);

	return 0.f;
}

static void UpdateRemain()
{
	Remain = RemainValue();

	if (Remain != -100)
	{
		Remain--;
		if (Remain <= 1)
		{
			// <=1 is a bodge because it takes a while to stop capturing :-(
			
			Remain = InitialRemain + 1;
			SendMessage(GetDlgItem(MainWnd, IDC_MOVIE_CAPTURE), BM_SETCHECK, 0, 0);
		}
		
		char text[32] = {0};
		sprintf(text, "%d", Remain);
		SetWindowText(GetDlgItem(MainWnd, IDC_COMBO1), text);
	}
}

static void SavedImage()
{
	// Print information about the image:
	C_DiskWriter* writer = g_CaptureClient.DiskWriter();
	Util::C_String whole = writer->LastFilename();

	Util::C_String text;
	text.Printf(" (%.1f fps)", writer->ImagesPerSecond());
	whole += text;
	
	// Check for free disk space
	char drive[4] = {0, 0, 0, 0};
	strncpy(drive, MainPath, 3);
	float freeSpace = DriveFree(drive);
	text.Printf(" %.0f Mb Free", freeSpace);
	whole += text;

	SetWindowText(GetDlgItem(MainWnd,IDC_STATUS), whole.Chars());

	if (freeSpace < 512.f)
	{
		// Pop up a warning in the corner:
		text.Printf("Warning - %s has only %.0f Mb Free", drive, freeSpace);
		TrayInfo(text.Chars());
	}

	// Show bitmap:
	MainShowImage();

	UpdateRemain();	
}

static void XenonStatus(int connected)
{
	char text[128]={0};

	sprintf(text,"(%s)",connected?"Connected":"Not Connected");

	SetWindowText(GetDlgItem(MainWnd,IDC_TARGET),text);

	TrayChange(connected?0:2);
}

static void XenonShowIp()
{
	unsigned int ip = XenonAltAddress();
	char text[128] = {0};
	sprintf(text, "%d.%d.%d.%d", (ip>>24)&255, (ip>>16)&255, (ip>>8)&255, ip&255);
	SetWindowText(GetDlgItem(MainWnd, IDC_EDIT2), text);
}

static void IdleLoop()
{
	g_CaptureClient.SetBitmapPixels(ShowImages ? BitmapPixels : 0);
	g_CaptureClient.Update();

	C_CaptureClient::E_State status = g_CaptureClient.Status();
	if (status != LastStatus)
	{
		switch (status)
		{
			case C_CaptureClient::E_NotConnected:
				XenonStatus(0);
			break;
			case C_CaptureClient::E_Connected:
				XenonStatus(1);
			break;
			case C_CaptureClient::E_Working:
				TrayChange(1);
			break;
		}
	}
	LastStatus = status;

	int now = GetTickCount();
	if ( now - LastCheck >= 2000)
	{
		if (status == 0)
			ConnectToIp();
		else
			g_CaptureClient.SendPing();

		LastCheck = now;
	}

	if (g_CaptureClient.m_imageCaptured)
	{
		g_CaptureClient.m_imageCaptured = 0;
		SavedImage();
	}

	static int prevMovie = 0;
	bool normalMovie = SendMessage(GetDlgItem(MainWnd, IDC_MOVIE_CAPTURE), BM_GETCHECK, 0, 0) == BST_CHECKED;
	bool photoMovie  = SendMessage(GetDlgItem(MainWnd, IDC_PHOTO_MOVIE),   BM_GETCHECK, 0, 0) == BST_CHECKED;
	bool movie = normalMovie || photoMovie;

	if (!prevMovie && movie)
	{
		g_CaptureClient.MovieCaptureStart(FpsValue(), photoMovie);
		int rv = RemainValue();
		if (rv != -100)
			InitialRemain = rv;
	}
	
	if (prevMovie && !movie)
		g_CaptureClient.MovieCaptureStop();

	prevMovie = movie;
}

// Main Win32 entry point:
int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
	MSG msg={0,0,0,0,0};
	char text[64]="";

	DeleteFile("zout.txt");

	ConfigLoad();
	Icon[0]=LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON1));
	Icon[1]=LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON2));
	Icon[2]=LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON3));

	MainWnd=CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DIALOG1),NULL,DialogProc);
	sprintf(text,"XenWatch v%x.%.3x",MainVer>>12,MainVer&0xfff);
	SetWindowText(MainWnd,text);
	SendMessage(MainWnd,WM_SETICON,ICON_SMALL,(LPARAM)Icon[0]);

	SendMessage(GetDlgItem(MainWnd,IDC_CHECK1), BM_SETCHECK, BST_CHECKED, 0);
	ComboAdd("300");
	ComboAdd("1800");
	ComboAdd("9000");
	FpsAdd("30 fps", true);
	FpsAdd("60 fps");

	HDC hdc = 0;
	BITMAPINFO bmi;
	memset(&bmi,0,sizeof(bmi));

	CreateCompatibleDC(NULL);
	bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth=1280;
	bmi.bmiHeader.biHeight=-720;
	bmi.bmiHeader.biPlanes=1;
	bmi.bmiHeader.biBitCount=24;
	Bitmap=CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,(void **)&BitmapPixels,NULL,0);
	
	UpdateDialog();
	
	TrayInit();
	XenonInit();
	XenonShowIp();
	XenonStatus(0);
	C_CaptureClient::NetworkInit();
	g_CaptureClient.SetSynchronous(0); // Use asynchronous capturing
	ConnectToIp();
	
	LastCheck = GetTickCount();
	// Message pump:
	for (;;)
	{
		int ret = PeekMessage(&msg,NULL,0,0,PM_REMOVE);
		
		if (ret)
		{
			if (msg.message == WM_QUIT) break;
			
			if (IsDialogMessage(MainWnd, &msg)==0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			IdleLoop();
			Sleep(1);
		}
	}

	g_CaptureClient.Disconnect();
	C_CaptureClient::NetworkExit();
	TrayExit();
	ConfigSave();

	if (DebugFile) fclose(DebugFile);
	DebugFile=NULL;
	
	return 0;
}
