// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _WIN32_WINNT 0x0500 

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

#include <core/core.h>
#include <core/containers.h>
#include <core/Maths.h>

#include <attk/Attk.h>

#include <system/serialise.h>
#include <system/misc/stringutil.h>

#include <d3d9.h>
#include <D3dx9tex.h>

//Include TCLAP
#include <CmdLine.h>
