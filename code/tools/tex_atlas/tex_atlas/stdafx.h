#pragma once

#include <windows.h>

#include <tex_atlas/TexAtlas.h>

namespace TexAtlas
{
	#include <tools_common/List.h>

	inline bool IsPowerOf2(int v) 
	{
		return (v & (v - 1)) == 0;
	}

	inline unsigned int RoundUpPow2(unsigned int v) 
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		return v+1;
	}
}
#include <tex_atlas/StringTable.h>
#include <tools_common/MathsUtility.h>
#include <tools_common/StringBuilder.h>
#include <core/utility/stringproc.h>
#include <tools_debug/DebugManager.h>
#include <crc/crc32.h>

