#ifndef DISPLAY_SHADER_VERTEX_SHADERS_H
#define DISPLAY_SHADER_VERTEX_SHADERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Shader/VertexShader.h"

namespace Display
{

void CreateVertexShaders();

void DestroyVertexShaders();

struct VertexShader_XY
{
	static C_VertexShader m_VertexShader;
	static C_VertexShader m_VertexShaderVertexColour;
};

}; // namespace Display

#endif // DISPLAY_SHADER_VERTEX_SHADERS_H
