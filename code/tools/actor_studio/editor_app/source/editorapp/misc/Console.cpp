#include "stdafx.h"
#include <EditorApp/Misc/Console.h>


namespace Editor
{

// maximum mumber of lines the output console should have
static const WORD kMaxConsoleLines = 500;

void CreateConsole()
{
	//only create console if we're debugging :)
	int hConHandle;
	long lStdHandle;
	FILE *fp;

	if (IsDebuggerPresent())
	{
		CONSOLE_SCREEN_BUFFER_INFO coninfo;
	
		// allocate a console for this app
		AllocConsole();

		// set the screen buffer to be big enough to let us scroll text
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

		coninfo.dwSize.Y = kMaxConsoleLines;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
	}
	else
	{
		//if (!AttachConsole(ATTACH_PARENT_PROCESS))
		//	return;
	}

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	if (lStdHandle)
	{
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		if (hConHandle != -1)
		{
			fp = _fdopen( hConHandle, "w" );
			if (fp)
			{
				*stdout = *fp;
				setvbuf( stdout, NULL, _IONBF, 0 );
			}
		}
	}

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	if (lStdHandle)
	{
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		if (hConHandle != -1)
		{
			fp = _fdopen( hConHandle, "r" );
			if (fp)
			{
				*stdin = *fp;
				setvbuf( stdin, NULL, _IONBF, 0 );
			}
		}
	}

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	if (lStdHandle)
	{
		hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
		if (hConHandle != -1)
		{
			fp = _fdopen( hConHandle, "w" );
			if (fp)
			{
				*stderr = *fp;
				setvbuf( stderr, NULL, _IONBF, 0 );
			}
		}
	}

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well
	std::ios::sync_with_stdio();
}

void DestroyConsole()
{
	FreeConsole();
}

}
