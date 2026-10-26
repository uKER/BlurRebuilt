#define _CRT_SECURE_NO_WARNINGS

#if defined(PLATFORM_PC)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <core\core.h>
#include <core/maths.h>
#include <core/containers/string.h>

#include <string.h>
#include <malloc.h>

#pragma warning(disable:4244)	// conversion from 'x' to 'y' possible loss of data

#include <system/debuglog.h>
#include "texture_export/texture_export.h"
#include "ngl/mercury_to_ngl.h"

