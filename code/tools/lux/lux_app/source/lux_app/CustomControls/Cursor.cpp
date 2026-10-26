//***************************************************************************
// Copyright 1999                                  
//***************************************************************************
// @Author: Unknown
//
// @Cedits:
//
//	  Giving credit where credit is due...  Thanks for the help!!!
//    -----------------------------------------------------------
//		  I forgot who wrote this code?  
//    -----------------------------------------------------------
//
// @Blah, blah, blah...
//
// THIS CODE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.
// You may use and distribute this code provided that you do not
// remove this title header and that you do not charge money for it.  
//
//***************************************************************************
#include "Stdafx.h"
#include "lux_app/CustomControls/Cursor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//***************************************************************************
//
//***************************************************************************
BOOL LoadHandCursor( HCURSOR& hCursor )
{
	BOOL bResult = FALSE;

    // Get the windows directory
    CString strWndDir;
    GetWindowsDirectory( strWndDir.GetBuffer(MAX_PATH), MAX_PATH );
	strWndDir.ReleaseBuffer();

    strWndDir += _T("\\winhlp32.exe");

    // This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
    HMODULE hModule = LoadLibrary(strWndDir);

    if( hModule ) 
	{
        HCURSOR hHandCursor = ::LoadCursor( hModule, MAKEINTRESOURCE(106) );

        if (hHandCursor)
		{
            hCursor = CopyCursor(hHandCursor);
			bResult = TRUE;
		}
    }

    FreeLibrary(hModule);

	return bResult;
}

