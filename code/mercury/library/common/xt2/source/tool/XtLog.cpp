#include "Common/XtPch.h"
#include <Xt/XtTool.h>
#include <PsApi.h>
#include <time.h>

namespace
{
	bool CreateDirectories(const XtString& path)
	{
		const char* start = path;
		const char* drive = start;

		if ((drive[0] == '\\') && (drive[1] == '\\'))
		{
			while((*drive == '\\') || (*drive == '/'))
				drive++;

			while((*drive != '\\') && (*drive != '/') && (*drive != '\0'))
				drive++;
		}
		else
		{
			for(const char* p = path; *p; p++)
			{
				if (*p == ':')
				{
					drive = p + 1;
					break;
				}
			}
		}

		do
		{
			while((*drive == '\\') || (*drive == '/'))
				drive++;

			while((*drive != '\\') && (*drive != '/') && (*drive != '\0'))
				drive++;

			if (*drive == '\0')
				break;

			XtString subPath(start, (int)(drive - start));

			if (CreateDirectory(subPath, 0) == FALSE)
			{
				if (GetLastError() != ERROR_ALREADY_EXISTS)
					return false;
			}
		}
		while(*drive != '\0');

		return true;
	}

	XtString GetExeBuildTime()
	{
		XtU8* module = (XtU8*)GetModuleHandle(0);

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

		result = Format("%i%s %s %04i %02i:%02i.%02i",
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
}

HANDLE gXtLogHandle = INVALID_HANDLE_VALUE;

void XtLogInit(const char* logPath, const char* appName)
{
	if (gXtLogHandle != INVALID_HANDLE_VALUE)
		return;

	Sz8 basePath = logPath;

	if (!CleanUpPath(basePath, false, true))
		return;

	if (!CreateDirectories(basePath))
		return;

	SYSTEMTIME time;
	GetLocalTime(&time);

	Sz8 baseName = Format("%s\\%s %04i-%02i-%02i %02i.%02i.%02i", basePath.Text(), appName, time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

	for(int retries = 0; retries < 10; retries++)
	{
		Sz8 filename = Format("%s %c.txt", baseName.Text(), 'A' + retries);

		if (!CreateDirectories(filename))
			continue;

		gXtLogHandle = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_NEW, 0, 0);

		if (gXtLogHandle != INVALID_HANDLE_VALUE)
			break;
	}

	XtLog("XTLG", "Started logging for %s at %04i-%02i-%02i %02i:%02i:%02i.", appName, time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	XtLog("XTLG", "Xt db version %i", XtGetDbVersion());
	XtLog("XTLG", "Build date: %s", GetExeBuildTime().Text());

	XtLog("XTLG", "Resident modules:");

	HMODULE modules[2048];
	DWORD needed = 0;

	if (EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &needed) != FALSE)
	{
		int count = needed / sizeof(HMODULE);

		for(int i = 0; i < count; i++)
		{
			char fname[MAX_PATH];

			if (GetModuleFileNameEx(GetCurrentProcess(), modules[i], fname, MAX_PATH) > 0)
				XtLog("XTLG", "    %s", fname);
		}
	}
}

void XtLogV(const char* area, const char* msg, va_list vl)
{
	if (gXtLogHandle == INVALID_HANDLE_VALUE)
		return;

	const int kMaxLength = 512;

	char buf[kMaxLength];

	SYSTEMTIME time;
	GetLocalTime(&time);

	int resultTime = _snprintf_s(buf, kMaxLength, _TRUNCATE, "%02i:%02i:%02i %4.4s ", time.wHour, time.wMinute, time.wSecond, area);

	if (resultTime < 0)
		return;

	int resultMsg = _vsnprintf_s(buf + resultTime, kMaxLength - resultTime, _TRUNCATE, msg, vl);

	if (resultMsg < 0)
		return;

	char* p = &buf[kMaxLength - 2];

	if ((resultTime + resultMsg + 2) < kMaxLength)
		p = &buf[resultTime + resultMsg];

	*p++ = '\r';
	*p++ = '\n';

	DWORD written;

	WriteFile(gXtLogHandle, buf, p - buf, &written, 0);
}

void XtLog(const char* area, const char* msg, ...)
{
	if (gXtLogHandle == INVALID_HANDLE_VALUE)
		return;

	va_list vl;
	va_start(vl, msg);
	XtLogV(area, msg, vl);	
	va_end(vl);
}