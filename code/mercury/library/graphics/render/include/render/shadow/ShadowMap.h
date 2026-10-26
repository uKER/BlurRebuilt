#ifndef RENDER_SHADOW_SHADOW_MAP_H
#define RENDER_SHADOW_SHADOW_MAP_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Shadow
{

extern ::Debug::Bool g_debugUseDepthStencilTexture;

//! E_ShadowMapMatrixType

enum E_ShadowMapMatrixType
{
	E_ShadowMapMatrixType_Standard = 0,
	E_ShadowMapMatrixType_Trapezoid,
};

//! C_ShadowMapSetup

class C_ShadowMapSetup
{
public:

	C_ShadowMapSetup();
	~C_ShadowMapSetup();

public:

	F32								m_NearClip;
	F32								m_FarClip;
	F32								m_ViewSize;
	Maths::HVec3					m_Position;
	Maths::HVec3					m_Direction;
	Maths::HMat44					m_EyeViewMatrix;
	Maths::HMat44					m_EyeProjMatrix;
	Maths::C_Rect					m_ViewExtents;
	F32								m_PerspDist;
	E_ShadowMapMatrixType			m_MatrixType;
#ifdef PLATFORM_PS3
	F32								m_PolygonOffsetFactor;
	F32								m_PolygonOffsetUnits;
#endif
};

//! C_ShadowMap

class C_ShadowMap : public Display::I_Renderer
{
public:

	C_ShadowMap();
	~C_ShadowMap();

	void Initialise(const U32 width, const U32 height, const U32 flags = 0);

	bool OnCreateVolatile();
	void OnDestroyVolatile();

	void Bind();
	void Unbind();

	void SetupView(const C_ShadowMapSetup& setup);
	void SetupStandardView(const C_ShadowMapSetup& setup);
	void SetupTrapezoidView(const C_ShadowMapSetup& setup);

public:

	Display::C_View					m_View;
	Display::C_RenderTexture2D		m_RenderTexture;

	Maths::HMat44					m_LightViewMatrix;
	Maths::HMat44					m_LightProjMatrix;

protected:

	bool							m_Bound;
	U32								m_Width;
	U32								m_Height;
	U32								m_Flags;
	U32								m_Levels;
	Display::E_Format				m_ColourFormat;
	Display::E_Format				m_DepthStencilFormat;
	Display::C_Viewport				m_Viewport;
	Display::C_ViewSetup			m_ViewSetup;
	Display::C_Surface				m_DepthStencilSurface;
	
	C_ShadowMapSetup				m_Setup;

};

} // namespace Shadow
} // namespace Render

#endif // RENDER_SHADOW_SHADOW_MAP_H
