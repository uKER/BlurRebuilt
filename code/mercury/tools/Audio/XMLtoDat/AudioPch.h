// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#define AUDIO_TOOLS

#define AUDIO_INCLUDE_NAMES

#ifdef XENON_AUDIO
#define BIG_ENDIAN
#endif // XENON_AUDIO

#pragma once

#include <malloc.h>
#include <crtdbg.h>
#include <assert.h>

#include <iostream>
#include <tchar.h>

#ifdef XML_TO_DAT
#include <windows.h>
#define DBPrint	printf
#endif

namespace Audio
{
	void Assert(bool bExp, char* strMessage);
};

/*
#include "../../../Shared/Memory/MemoryAllocatorOverload.h"
#include "../../../Shared/Memory/MemorySystemID.h"
*/
#include "core/core.h"

#include "audio.h"

#include "minmax.h"

#include "common/Types.h"

#include "common/array.h"

#include "common/bitlimitedpagedarray.h"

#include "exception.h"

#include "xmlParser.h"

#include "common/types.h"

#include "common/3dmaths.h"

#include "../../core/include/Core/Core.h"
//#include "Core/audio2.h"
#include "Core/audio_core.h"
#include "Core/dspeffect.h"

#include "Common/Queue.h"

//extern inline U16 DoEndian(U16 uValue);
//extern inline S16 DoEndian(S16 sValue);
//extern inline U32 DoEndian(U32 uValue);
//extern inline S32 DoEndian(S32 sValue);
//extern inline F32 DoEndian(F32 fValue);

// forward decl for parser
namespace Audio { class Instance; }

#include "ComponentHeaders.h"
#include "ComponentStrings.h"

//#include "scriptVM.h"

#include "stringTable.h"

#include "../common/CommandLineApp.h"
#include "Diagnostics.h"
#include "Constant.h"
#include "XMLtoDatCommon.h"
#include "ScriptBuilder.h"
#include "audioDatafile.h"