#ifndef RENDER_DEFERRED_DROP_SHADOWS_H
#define RENDER_DEFERRED_DROP_SHADOWS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Btl/Containers/Vector.h>

namespace Render
{
namespace Deferred
{

struct C_DropShadow
{
	MathsStorage::Float3	m_vPosition;
	F32						m_fRadius;
	F32						m_fIntensity;
};

typedef Btl::Vector<C_DropShadow> DropShadows;

struct C_VisibleDropShadow
{
	Maths::C_Rect			m_rectangle;
	C_DropShadow*			m_pShadow;
};

typedef Btl::Vector<C_VisibleDropShadow> VisibleDropShadows;

enum E_DropShadowShader
{
	E_DropShadowShader_1 = 0,
	E_DropShadowShader_2,
	E_DropShadowShader_3,
	E_DropShadowShader_4,
	E_DropShadowShader_5,
	E_DropShadowShader_6,
	E_DropShadowShader_7,
	E_DropShadowShader_8,
	E_DropShadowShader_Count
};

class C_DropShadows : public Display::I_Renderer
{
public:

	C_DropShadows();
	~C_DropShadows();

	bool	OnCreate();
	bool	OnCreateVolatile();
	void	OnDestroy();
	void	OnDestroyVolatile();

	void	Render() const;

	void	AddShadow(const MathsStorage::Float3& vPosition, const F32 fRadius, const F32 fIntensity);
	
	void	SetTesselation(const U32 x, const U32 y)											{ m_HorizontalTiles = x; m_VerticalTiles = y; }
	void	SetDepthTexture(const Display::C_Texture2D* pDepthTexture)							{ m_pDepthTexture = pDepthTexture; }
	void	SetNormalTexture(const Display::C_Texture2D* pNormalTexture)						{ m_pNormalTexture = pNormalTexture; }

private:

	void	GatherVisibleShadows(VisibleDropShadows& visibleShadows) const;
	//return the last visible shadow processed, false if it has more shadows
	bool	GatherVisibleParameters(const VisibleDropShadows& visibleShadows, const Maths::C_Rect& rectangle, U32& firstProcessedShadow, U32& shadowCount, Display::C_Float4* pPositionFalloffs, Display::C_Float4* pIntensities) const;

private:

	mutable DropShadows m_DropShadows;

	U32	m_HorizontalTiles;
	U32	m_VerticalTiles;

private:

	Display::C_RenderTexture2D	m_RenderTexture;

	const Display::C_Texture2D*	m_pDepthTexture;
	const Display::C_Texture2D*	m_pNormalTexture;

	Display::C_VertexBuffer		m_VertexBuffer;

	Display::C_VertexShader		m_VertexShader;
	Display::C_PixelShader		m_PixelShader[E_DropShadowShader_Count];

	Display::C_PixelShaderParameterArray	m_PositionParameter[E_DropShadowShader_Count];
	Display::C_PixelShaderParameterArray	m_IntensityParameter[E_DropShadowShader_Count];
	Display::C_PixelShaderParameter			m_DepthParameter[E_DropShadowShader_Count];
	Display::C_PixelShaderParameter			m_LuminanceParameter[E_DropShadowShader_Count];
	Display::C_PixelShaderParameter			m_ViewUpVectorParameter[E_DropShadowShader_Count];

};

}; // namespace Deferred
}; // namespace Render

#endif // RENDER_DEFERRED_DROP_SHADOWS_H
