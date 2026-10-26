#ifndef DISPLAY_SHADER_VERTEX_SHADER_UTILS_H
#define DISPLAY_SHADER_VERTEX_SHADER_UTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Shader/ShaderUtils.h"
#include "Display/Shader/VertexShader.h"
#include "Display/Shader/VertexShaders.h"

namespace Display
{

extern Util::C_String g_VertexShaderExt;

#ifdef PLATFORM_PC
bool CreateVertexShaderFromFunc(const Util::C_String& name, const U32 flags, C_VertexShader* pVertexShader, const U32 size, const C_ShaderData& data);
#endif

bool CreateVertexShaderFromFunc(const Util::C_String& name, const U32 flags, C_VertexShader* pVertexShader, const U32 size, const S32* pData);
bool CreateVertexShaderFromFile(const Util::C_String& name, const U32 flags, C_VertexShader* pVertexShader);

void RefreshVertexShaders();

}; // namespace Display

#endif // DISPLAY_SHADER_VERTEX_SHADER_UTILS_H
