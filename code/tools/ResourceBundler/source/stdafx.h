#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include <iostream>
#include <string>

#include <core/core.h>
#include <core/maths.h>
#include <core/containers.h>
#include <core/containers/string.h>
#include <core/utility/endian.h>

#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>
#include <System/DebugLog.h>

#include <tools_shared/ToolsShared.h>

#include <attk/attk.h>

#include <compress_lib/CompressLib.h>

#include <d3d9.h>      // Note: make sure this is the Xbox 360 flavor of D3D9.h!
#include <xgraphics.h>

#include <display/bundler/ResourceBundlerDefines.h>

static const U32 RESERVED_DATA_SIZE = 256;