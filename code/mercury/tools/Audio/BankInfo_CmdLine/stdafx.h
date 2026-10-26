// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define USER_SAMH

// so we get the LogMessage output to the command line in WaveBankReader


#define PLATFORM_PC
#define COMPILER_VC

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define CHECK_FOR_MEMORY_LEAKS
#ifdef CHECK_FOR_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <malloc.h>
#include <crtdbg.h>
#endif

#define NON_NATIVE_BANK_READING_SUPPORT

#define AUDIO_RESTRICT

// shared tech core.h
#include <core\core.h>

#include <iostream>
#include <tchar.h>

#include "windows.h"
#include <assert.h>

#include "audio.h"

//#include <core\containers\string.h>

// shared audio library headers
#include "common\types.h"
#include "common\wavebank.h"
#include "common\file.h"
#include "common\array.h"

#include "../SystemHeaders.h"
#include "debug.h"

void Audio::Assert(bool bExp, char* strMessage);
