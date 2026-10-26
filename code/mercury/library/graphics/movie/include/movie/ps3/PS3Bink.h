#ifndef MOVIE_PS3_BINK_H
#define MOVIE_PS3_BINK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Display/Shader/VertexShader.h>
#include <Display/Shader/PixelShader.h>

namespace Movie
{

//! C_PS3Shaders

class C_PS3Shaders
{
public:

	C_PS3Shaders();
	~C_PS3Shaders();

	bool CreateShaders();
	void DestroyShaders();

	void SetVertexShaderBink();
	void SetVertexShaderBink_Adjust(const F32* pAdjust);
	void SetVertexShaderBink_UVRange(const F32* pUVRange);
	
	void SetPixelShaderBink();
	void SetPixelShaderBink_AlphaLevel(const F32 alphaLevel);

	void SetPixelShaderBinkNoAlpha();
	void SetPixelShaderBinkNoAlpha_AlphaLevel(const F32 alphaLevel);

	static bool AllocVertexMemory(const U32 size, U8** ppAddress);
	static void FreeVertexMemory(const U32 size, U8* ppAddress);

	static bool AllocTextureMemory(const U32 size, U8** ppAddress);
	static void FreeTextureMemory(const U32 size, U8* ppAddress);

public:

	Display::C_VertexShader 			m_vsBink;
	Display::C_VertexShaderParameter	m_AdjustParam;
	Display::C_VertexShaderParameter	m_UVRangeParam;
	
	Display::C_PixelShader				m_psBink;
	Display::C_PixelShaderParameter		m_AlphaParam;

	Display::C_PixelShader				m_psBinkNoAlpha;
	Display::C_PixelShaderParameter		m_AlphaNPAParam;

	F32 								screen_width;
	F32 								screen_height;

	U32 								vertex_size;
	U32 								vertex_offset;
	U8* 								vertex_address;
};

}; // namespace Movie

#endif // MOVIE_PS3_BINK_H
