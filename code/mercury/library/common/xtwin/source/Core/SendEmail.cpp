#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/WndCore.h"
#include <Xt/XtTool.h>

#include <shellapi.h> // for RecycleFile

#include <mapi.h> // for SendEmail
#include <time.h>

struct Email
{
	XtString subject;
	XtString body;
};

DWORD CALLBACK SendEmailProc(void* context)
{
	Email* email = (Email*)context;

	HINSTANCE lib = LoadLibrary(_T("MAPI32.DLL"));

	if (lib != 0)
	{
		LPMAPISENDMAIL fn = (LPMAPISENDMAIL)GetProcAddress(lib, "MAPISendMail");

		if (fn != 0)
		{
			MapiMessage msg = { 0 };

			msg.lpszSubject		= (LPSTR)(const char*)email->subject;
			msg.lpszNoteText	= (LPSTR)(const char*)email->body;

			fn(0, 0, &msg, MAPI_DIALOG, 0);
		}

		FreeLibrary(lib);
	}

	delete email;

	return 0;
}

void SendEmail(const XtString& subject, const XtString& body)
{
	Email* email = new Email;

	if (!email)
		return;

	email->subject	= subject;
	email->body		= body;

	HANDLE hdl = CreateThread(0, 0, SendEmailProc, email, 0, 0);

	if (hdl != 0)
	{
		CloseHandle(hdl);
	}
	else
	{
		delete email;
	}
}

XtString GetExeBuildTime()
{
	u8* module = (u8*)GetModuleHandle(0);

	IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)module;
	IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(module + dos->e_lfanew);

	__time32_t secs = nt->FileHeader.TimeDateStamp;

	tm t;
	_localtime32_s(&t, &secs);

	XtString result;

	static char* months[] =
	{
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec"
	};

	const char* th = "th";

	if ((t.tm_mday < 10) || (t.tm_mday > 20))
	{
		switch(t.tm_mday % 10)
		{
			case 1: th = "st"; break;
			case 2: th = "nd"; break;
			case 3: th = "rd"; break;
		}
	}

	result = Format("%02i%s %s %04i %02i:%02i.%02i",
			t.tm_mday,
			th,
			months[t.tm_mon],
			t.tm_year + 1900,
			t.tm_hour,
			t.tm_min,
			t.tm_sec
		);

	return result;
}

bool RecycleFile(const XtString& path)
{
	SHFILEOPSTRUCT fo = { 0 };

	Sz8 clean = path;

	if (!CleanUpFilename(clean, false))
		return false;

	TCHAR filename[MAX_PATH];

	_sntprintf_s(filename, MAX_PATH, _TRUNCATE, _T("%s%c"), (LPCTSTR)clean, _T('\0'));

	fo.wFunc	= FO_DELETE;
	fo.pFrom	= filename;
	fo.fFlags	= FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

	return SHFileOperation(&fo) == 0;
}

bool SetClipboardText(HWND hwnd, const XtString& text)
{
	if (!OpenClipboard(hwnd))
		return false;

	bool result = false;

	if (EmptyClipboard())
	{
		int length = text.Length() + 1;

		if (HGLOBAL hdl = GlobalAlloc(GMEM_MOVEABLE, length))
		{
			if (void* mem = GlobalLock(hdl))
			{
				memcpy(mem, text.Text(), length);

				GlobalUnlock(hdl);

				if (SetClipboardData(CF_TEXT, hdl) != 0)
					result = true;
				else
					GlobalFree(hdl);
			}
			else
				GlobalFree(hdl);
		}
	}

	CloseClipboard();

	return result;
}

bool GetClipboardText(HWND hwnd, XtString& text)
{
	if (!IsClipboardFormatAvailable(CF_TEXT))
		return false;

	if (!OpenClipboard(hwnd))
		return false;

	bool result = false;

	if (HANDLE hdl = GetClipboardData(CF_TEXT))
	{
		if (void* mem = GlobalLock(hdl))
		{
			text.Set((const char*)mem, strnlen_s((const char*)mem, GlobalSize(hdl)));

			GlobalUnlock(hdl);

			result = true;
		}
	}

	CloseClipboard();

	return result;
}

XtUt::Dictionary<XtString, Gdiplus::Bitmap> gBitmapCache;

Gdiplus::Bitmap* CacheBitmap(XtString filename)
{
	if (!*filename)
		return 0;

	if (Gdiplus::Bitmap* bitmap = gBitmapCache[filename])
		return bitmap;

	Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(MbToWc(filename));

	if (!bitmap || (bitmap->GetLastStatus() != Gdiplus::Ok))
	{
		delete bitmap;

		DebugLn("Failed to load bitmap: %s", filename.Text());
		return 0;
	}

	if (!XtUt::Insert(&gBitmapCache, filename, bitmap))
		return 0;

	return bitmap;
}

void FreeBitmapCache()
{
	Free(&gBitmapCache);
}