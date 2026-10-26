#ifndef RENDER_SHADOW_SHADOW_MAP_VARIANCE_H
#define RENDER_SHADOW_SHADOW_MAP_VARIANCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Render/Shadow/ShadowMap.h"

namespace Render
{
namespace Shadow
{

//! C_VarianceMapShaders.

struct C_VarianceMapShaders
{
	Display::C_VertexShader	m_vsDepth;
	Display::C_PixelShader	m_psDepth;
};

//! C_VarianceMap.

class C_VarianceMap : public C_ShadowMap
{
public:

	C_VarianceMap();
	~C_VarianceMap();

	void Initialise(const U32 width, const U32 height);

	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

	void SetShaders();
	void SetTexture(const U32 sampler, const U32 shadowMatrixRegister);

	void Bind();
	void Unbind();

	void DebugRender();

public:

	static bool CreateShaders();
	static void DestroyShaders();

	static C_VarianceMapShaders		m_Shaders;

public:

	Display::C_RenderTexture2D		m_ColourTexture;
	Display::C_RenderTexture2D		m_DepthStencilTexture;
	
private:
	
	Maths::HMat44					m_hTextureMatrix;

	Display::C_Matrix				m_ShadowMatrix;
};

extern C_VarianceMap* g_pVarShadowMap;

INLINE bool VarShadowMapExists()
{
	return g_pVarShadowMap ? true : false;
}

INLINE C_VarianceMap* VarShadowMap()
{
	assert(g_pVarShadowMap);
	return g_pVarShadowMap;
}

} // namespace Shadow
} // namespace Render

#endif // RENDER_SHADOW_SHADOW_MAP_VARIANCE_H
