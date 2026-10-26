#ifndef DISPLAY_SHADER_PIXEL_SHADERS_H
#define DISPLAY_SHADER_PIXEL_SHADERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Shader/PixelShader.h"

namespace Display
{

void CreatePixelShaders();

void DestroyPixelShaders();

struct PixelShader
{
	static C_PixelShader			m_PixelShaderColour;
	static C_PixelShaderParameter	m_PixelShaderColour_Colour;

	static C_PixelShader			m_PixelShaderVertexColour;
	static C_PixelShader			m_PixelShaderTexture;
	static C_PixelShader			m_PixelShaderTextureRGB;
	static C_PixelShader			m_PixelShaderTextureR;
	static C_PixelShader			m_PixelShaderTextureG;
	static C_PixelShader			m_PixelShaderTextureB;
	static C_PixelShader			m_PixelShaderTextureA;
	static C_PixelShader			m_PixelShaderTextureD;
	
#if defined(PLATFORM_PS3)
	static C_PixelShader			m_PixelShaderDepth;
	static C_PixelShaderParameter	m_PixelShaderDepth_PixelSize;
	
	static C_PixelShader			m_PixelShaderTexture2x2;
	static C_PixelShaderParameter	m_PixelShaderTexture2x2_PixelSize;
#endif
	
#if defined(PLATFORM_PS3)
	static C_PixelShader			m_PixelShaderTexture2xAA;
	static C_PixelShaderParameter	m_PixelShaderTexture2xAA_PixelSize;
	
	static C_PixelShader			m_PixelShaderTexture4xAA;
	static C_PixelShaderParameter	m_PixelShaderTexture4xAA_PixelSize;
	
	static C_PixelShader			m_PixelShaderTextureFast;
	static C_PixelShaderParameter	m_PixelShaderTextureFast_PixelSize;
	
	static C_PixelShader			m_PixelShaderColAndDep;
	static C_PixelShaderParameter	m_PixelShaderColAndDep_PixelSize;
#endif
};

}; // namespace Display

#endif // DISPLAY_SHADER_PIXEL_SHADERS_H
