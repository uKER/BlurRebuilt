#pragma once

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#include <string>

#include <core/core.h>

#include <core/containers/vector.h>
#include <core/containers/list.h>
#include <compress_lib/CompressLib.h>

// Xbox Required Headers
#include "windows.h"
#include "d3d9.h"      // Note: make sure this is the Xbox 360 flavor of D3D9.h!
#include "xgraphics.h"
#include "d3dx9math.h"

#if defined(ENABLE_NVIDIA)
// NVidia Required Headers
#include "dxtlib\dxtlib.h"
#endif

