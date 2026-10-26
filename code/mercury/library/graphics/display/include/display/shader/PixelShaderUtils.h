#ifndef DISPLAY_SHADER_PIXEL_SHADERUTILS_H
#define DISPLAY_SHADER_PIXEL_SHADERUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Shader/ShaderUtils.h"
#include "Display/Shader/PixelShader.h"
#include "Display/Shader/PixelShaders.h"

namespace Display
{

extern Util::C_String g_PixelShaderExt;

#ifdef PLATFORM_PC
bool CreatePixelShaderFromFunc(const Util::C_String& name, const U32 flags, C_PixelShader* pPixelShader, const U32 size, const C_ShaderData& data);
#endif

bool CreatePixelShaderFromFunc(const Util::C_String& name, const U32 flags, C_PixelShader* pPixelShader, const U32 size, const S32* pData);
bool CreatePixelShaderFromFile(const Util::C_String& name, const U32 flags, C_PixelShader* pPixelShader);

void RefreshPixelShaders();

}; // namespace Display

#endif // DISPLAY_SHADER_PIXEL_SHADERUTILS_H
