#ifndef AMAX_GRAPHICS_LIGHTING_POINTLIGHTS_H
#define AMAX_GRAPHICS_LIGHTING_POINTLIGHTS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Btl/Containers/Vector.h>
#include <Btl/Containers/Radix.h>

struct C_PointLight
{
	MathsStorage::Float3	m_vPosition;
	F32						m_fRadius;
	MathsStorage::Float3	m_vColour;
	F32						m_fRcpRadiusSq;
};

typedef Btl::Vector<C_PointLight> PointLights;

struct C_VisiblePointLight
{
	Maths::C_Rect			m_rectangle;
	U32						m_sortKey;
	F32						m_near;
	F32						m_far;
	C_PointLight*			m_pLight;
};

typedef Btl::Vector<C_VisiblePointLight> VisiblePointLights;

inline U32 RadixKey(const C_VisiblePointLight& light)
{
	return light.m_sortKey;
}

enum E_PointLightShader
{
	E_PointLightShader_1 = 0,
	E_PointLightShader_2,
	E_PointLightShader_3,
	E_PointLightShader_4,
	E_PointLightShader_5,
	E_PointLightShader_6,
	E_PointLightShader_7,
	E_PointLightShader_8,
	E_PointLightShader_9,
	E_PointLightShader_10,
	E_PointLightShader_11,
	E_PointLightShader_12,
	E_PointLightShader_Count
};

const U32 c_MaximumLights = 12;

class C_PointLights : public Display::I_Renderer
{
public:

	C_PointLights();
	~C_PointLights();

	bool	OnCreate();
	bool	OnCreateVolatile();
	void	OnDestroy();
	void	OnDestroyVolatile();

	void	Render() const;

	void	AddLight(const MathsStorage::Float3& vPosition, const F32 fRadius, const MathsStorage::Float3& vColour);

	void	SetTessellation(const U32 x, const U32 y)											{ m_HorizontalTiles = x; m_VerticalTiles = y; }
	void	SetNormalDepthTexture(const Display::C_Texture2D* pNormalDepthTexture)				{ m_pNormalDepthTexture = pNormalDepthTexture; }
	
	void	SetFarClip(const F32 fFarClip)														{ m_FarClip = fFarClip; }

private:

	void	GatherVisibleLights(VisiblePointLights& visibleLights) const;
	void	GatherVisibleParameters(const VisiblePointLights& visibleLights, const Maths::C_Rect& rectangle, U32& lightCount, Display::C_Float4* pPositionFalloffs, Display::C_Float4* pColours, F32& near, F32& far) const;

private:

	mutable PointLights m_PointLights;

	mutable	VisiblePointLights						m_VisibleLights;
	mutable Btl::RadixSorter<VisiblePointLights>	m_RadixSorter;

	U32	m_HorizontalTiles;
	U32	m_VerticalTiles;

private:

	const Display::C_Texture2D*	m_pNormalDepthTexture;

	Display::C_VertexBuffer		m_VertexBuffer;

	Display::C_VertexShader		m_VertexShader;
	Display::C_PixelShader		m_PixelShader[E_PointLightShader_Count];

	Display::C_PixelShaderParameterArray	m_PositionParameter[E_PointLightShader_Count];
	Display::C_PixelShaderParameterArray	m_ColourParameter[E_PointLightShader_Count];

	F32							m_FarClip;

};

#endif // AMAX_GRAPHICS_LIGHTING_POINTLIGHTS_H
