#include "Core/StdAfx.h"
#include "Application/SorceryApp.h"
#include <crtdbg.h>
#include "XtWin/WndClasses.h"
#include "XtWin/SendEmail.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	//_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_CHECK_EVERY_1024_DF);

	{
		ULONG HeapFragValue = 2;

		if (HeapSetInformation(
				GetProcessHeap(),
				HeapCompatibilityInformation,
				&HeapFragValue,
				sizeof(HeapFragValue)
			))
		{
			//DebugLn("Using Low Fragmentation Heap");
		}
		else
		{
			DebugLn("WARNING: Using Standard Heap, memory allocation may be extremely slow");

			if (IsDebuggerPresent())
				DebugLn("\tMake sure debugger environment includes \"_NO_DEBUG_HEAP=1\"");
		}
	}

	srand(GetTickCount());

	XtLogInit("Z:\\Tools\\Temp\\Logs\\Sorcery", "Sorcery");

	{
		INITCOMMONCONTROLSEX iccex;

		iccex.dwSize	= sizeof(iccex);
		iccex.dwICC		= ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_LINK_CLASS;

		InitCommonControlsEx(&iccex);

		Gdiplus::GdiplusStartupInput	gdiplusStartupInput;
		ULONG_PTR						gdiplusToken;

		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

		RegisterXtWinClasses();

		if (SorceryApp* app = new SorceryApp)
		{
			if (app->Initialise())
			{
				for(;;)
				{
					if (!WndCorePumpMessage(0))
						break;
				}
			}

			app->Destroy();

			delete app;
		}

		FreeBitmapCache();

		Gdiplus::GdiplusShutdown(gdiplusToken);
	}

	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);

	return 0;
}