// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <mmreg.h>      // For WAVEFORMATEX
#include <mmsystem.h>      
#include <stdlib.h>

// mercury includes
#include "core/core.h"
#include "core/containers.h"
#include "system/serialise.h"
#include "system/xml/xml.h"
#include "system/misc/StringUtil.h"

#include <iostream>
#include <tchar.h>
#include <string>

#include "helperfuncs.h"

extern bool gbDoSystemPause;
extern bool gbWriteBankToDest;
extern bool gbQuantizeEnabled;
extern int giQuantizeMethod;
extern int giResamplerQuality;
extern Util::C_String gstrOutputPath;
extern bool gbWriteIntermediateFiles;
extern bool gbWriteIntermediateBitDepthFiles;
extern bool gbWriteIntermediateQuantizedFiles;
extern bool gbWriteIntermediateResampledFiles;
extern bool gbWriteIntermediateEncodedFiles;
extern bool gbDeleteIntermediateFiles;
extern bool gbVerboseMode;
