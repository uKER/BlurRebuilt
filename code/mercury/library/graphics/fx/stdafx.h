#define _CRT_SECURE_NO_WARNINGS

#if defined (PLATFORM_PC)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <core\core.h>
#include <core/maths.h>
#include <core/containers/string.h>

#include <string.h>
#if !PLATFORM_PS3 && !PLATFORM_SPU
#include <malloc.h>
#endif

#if !CFG_SPEED && !PLATFORM_SPU
#define ENABLE_DEBUGVARIABLES 1
#endif

#pragma warning(disable:4244)	// conversion from 'x' to 'y' possible loss of data

#if !CFG_CONVERTER
#include <Display2/Display2Includes.h>
#endif

#if PLATFORM_SPU
// TEMPORARY HACK TO GET SPU VERSION TO COMPILE
namespace Display
{
	//struct C_Texture2D 
	//{
		//const void*& GetGPUTexture() const;
	//};
	struct C_CommandBuffer {};

	//struct C_PixelShaderParameterBase {};

	//struct C_VertexShader {};
	//struct C_PixelShaderParameterArray {};
}
#endif

#include <system/debuglog.h>

