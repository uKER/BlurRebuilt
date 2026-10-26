#ifndef RENDER_SHADOW_SHADOW_MAP_STANDARD_H
#define RENDER_SHADOW_SHADOW_MAP_STANDARD_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Render/Shadow/ShadowMap.h"

namespace Render
{
namespace Shadow
{

//! C_StandardMapShaders.

struct C_StandardMapShaders
{
	Display::C_VertexShader	m_vsDepth;
	Display::C_PixelShader	m_psDepth;
};

//! C_StandardMap.

class C_StandardMap : public C_ShadowMap
{
public:

	C_StandardMap();
	~C_StandardMap();

	bool OnCreate();
	void OnDestroy();

	void SetShaders();
	void SetTexture(const U32 sampler, const U32 lightMatrixRegister, const U32 shadowMatrixRegister);

	void Bind();
	void Unbind();
	
	void DebugRender();

public:

	static bool CreateShaders();
	static void DestroyShaders();

	static C_StandardMapShaders		m_Shaders;

private:

	Display::C_Matrix				m_LightMatrix;
	Display::C_Matrix				m_ShadowMatrix;
	Maths::HMat44					m_hLightMatrix;
	Maths::HMat44					m_hShadowMatrix;
	Maths::HMat44					m_hTextureMatrix;
};

extern C_StandardMap* g_pPcfShadowMap;

INLINE bool PcfShadowMapExists()
{
	return g_pPcfShadowMap ? true : false;
}

INLINE C_StandardMap* PcfShadowMap()
{
	assert(g_pPcfShadowMap);
	return g_pPcfShadowMap;
}

} // namespace Shadow
} // namespace Render

#endif // RENDER_SHADOW_SHADOW_MAP_STANDARD_H
