#ifndef DEFERRED_PREPASS_DYNAMIC_LIGHTS_H
#define DEFERRED_PREPASS_DYNAMIC_LIGHTS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Btl/Containers/Vector.h>

namespace Deferred
{

struct C_DynamicLight
{
	MathsStorage::Float3	m_vPosition;
	F32						m_fRadius;
	MathsStorage::Float3	m_vColour;
};

typedef Btl::Vector<C_DynamicLight> DynamicLights;

struct C_VisibleDynamicLight
{
	Maths::C_Rect			m_rectangle;
	U32						m_sortKey;
	F32						m_near;
	F32						m_far;
	C_DynamicLight*			m_pLight;
};

typedef Btl::Vector<C_VisibleDynamicLight> VisibleDynamicLights;

inline U32 RadixKey(const C_VisibleDynamicLight& light)
{
	return light.m_sortKey;
}

enum E_DynamicLightShader
{
	E_DynamicLightShader_1 = 0,
	E_DynamicLightShader_2,
	E_DynamicLightShader_3,
	E_DynamicLightShader_4,
	E_DynamicLightShader_5,
	E_DynamicLightShader_6,
	E_DynamicLightShader_7,
	E_DynamicLightShader_8,
	E_DynamicLightShader_2xMSAA_1,
	E_DynamicLightShader_2xMSAA_2,
	E_DynamicLightShader_2xMSAA_3,
	E_DynamicLightShader_2xMSAA_4,
	E_DynamicLightShader_2xMSAA_5,
	E_DynamicLightShader_2xMSAA_6,
	E_DynamicLightShader_2xMSAA_7,
	E_DynamicLightShader_2xMSAA_8,
	E_DynamicLightShader_Count
};

const U32 c_MaximumLights = 8;

class C_DynamicLights : public Display::I_Renderer
{
public:

	C_DynamicLights();
	~C_DynamicLights();

	bool	OnCreate();
	void	OnDestroy();

	void	Render() const;

	void	AddLight(const MathsStorage::Float3& vPosition, const F32 fRadius, const MathsStorage::Float3& vColour);

	void	SetTesselation(const U32 x, const U32 y)											{ m_HorizontalTiles = x; m_VerticalTiles = y; }
	void	SetDepthTexture(const Display::C_Texture2D* pDepthTexture)							{ m_pDepthTexture = pDepthTexture; }
	void	SetNormalTexture(const Display::C_Texture2D* pNormalTexture)						{ m_pNormalTexture = pNormalTexture; }

private:

	void	GatherVisibleLights(VisibleDynamicLights& visibleLights) const;
	void	GatherVisibleParameters(const VisibleDynamicLights& visibleLights, const Maths::C_Rect& rectangle, U32& lightCount, Display::C_Float4* pPositionFalloffs, Display::C_Float4* pColours, F32& near, F32& far) const;

private:

	mutable DynamicLights m_DynamicLights;

	U32	m_HorizontalTiles;
	U32	m_VerticalTiles;

private:

	const Display::C_Texture2D*	m_pDepthTexture;
	const Display::C_Texture2D*	m_pNormalTexture;

	Display::C_VertexBuffer		m_VertexBuffer;

	Display::C_VertexShader		m_VertexShader;
	Display::C_PixelShader		m_PixelShader[E_DynamicLightShader_Count];

	Display::C_PixelShaderParameterArray	m_PositionParameter[E_DynamicLightShader_Count];
	Display::C_PixelShaderParameterArray	m_ColourParameter[E_DynamicLightShader_Count];
	Display::C_PixelShaderParameter			m_DepthParameter[E_DynamicLightShader_Count];

};

}; // namespace Deferred

#endif // DEFERRED_PREPASS_DYNAMIC_LIGHTS_H
