#pragma once

#include <stdio.h>
#include <tchar.h>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <malloc.h>
#include <crtdbg.h>
#include "assert.h"
#define AUDIO_TOOLS

#include "common/Types.h"

#include "Audio.h"
#include "ControlScriptCommon.h"
#include "../common/CommandLineApp.h"
#include "SetupScriptParser.h"
