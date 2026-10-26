#ifndef RENDER_DEFERRED_DYNAMIC_LIGHTS_H
#define RENDER_DEFERRED_DYNAMIC_LIGHTS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Btl/Containers/Vector.h>

namespace Render
{
namespace Deferred
{

struct C_DynamicLight
{
	MathsStorage::Float3	m_vPosition;
	F32						m_fRadius;
	MathsStorage::Float3	m_vColour;
	bool					m_SlotFree;
};

typedef Btl::Vector<C_DynamicLight> DynamicLights;

struct C_VisibleDynamicLight
{
	Maths::C_Rect			m_rectangle;
	C_DynamicLight*			m_pLight;
};

typedef Btl::Vector<C_VisibleDynamicLight> VisibleDynamicLights;

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
	E_DynamicLightShader_Count
};

const U32 c_MaximumLights = 8;

class C_DynamicLights : public Display::I_Renderer
{
public:

	C_DynamicLights();
	~C_DynamicLights();

	bool	OnCreate();
	bool	OnCreateVolatile();
	void	OnDestroy();
	void	OnDestroyVolatile();

	void	Render() const;
	void	RenderSpotlightVolume(const MathsStorage::MatrixRow44& mWorld, const MathsStorage::Float4& vDirection, const MathsStorage::Float4& vColour, const Display::C_Texture3D* pTexture) const;

	U32		AddLight();
	void	RemoveLight(const U32 lightIndex);
	void	UpdateLight(const U32 lightSlot, const MathsStorage::Float3& vPosition, const F32 fRadius, const MathsStorage::Float3& vColour);

	void	SetTesselation(const U32 x, const U32 y)											{ m_HorizontalTiles = x; m_VerticalTiles = y; }
	void	SetDepthTexture(const Display::C_Texture2D* pDepthTexture)							{ m_pDepthTexture = pDepthTexture; }
	void	SetNormalTexture(const Display::C_Texture2D* pNormalTexture)						{ m_pNormalTexture = pNormalTexture; }

private:

	void	GatherVisibleLights(VisibleDynamicLights& visibleLights) const;
	bool	GatherVisibleParameters(const VisibleDynamicLights& visibleLights, const Maths::C_Rect& rectangle, U32& firstLight, U32& lightCount, Display::C_Float4* pPositionFalloffs, Display::C_Float4* pColours) const;

private:

	mutable DynamicLights	m_DynamicLights;
	mutable U32				m_FirstFreeDynamicLightSlot;

	U32	m_HorizontalTiles;
	U32	m_VerticalTiles;

private:

	Display::C_RenderTexture2D	m_RenderTexture;

	const Display::C_Texture2D*	m_pDepthTexture;
	const Display::C_Texture2D*	m_pNormalTexture;

	Display::C_VertexBuffer		m_VertexBuffer;

	Display::C_VertexShader		m_VertexShader;
	Display::C_PixelShader		m_PixelShader[E_DynamicLightShader_Count];

	Display::C_PixelShaderParameterArray	m_PositionParameter[E_DynamicLightShader_Count];
	Display::C_PixelShaderParameterArray	m_ColourParameter[E_DynamicLightShader_Count];
	Display::C_PixelShaderParameterArray	m_MatrixParameter[E_DynamicLightShader_Count];
	Display::C_PixelShaderParameter			m_DepthParameter[E_DynamicLightShader_Count];
	Display::C_PixelShaderParameter			m_LuminanceParameter[E_DynamicLightShader_Count];

	Display::C_VertexShader		m_VolumeSpotlightVertexShader;
	Display::C_PixelShader		m_VolumeSpotlightPixelShader;

	Display::C_PixelShaderParameterArray	m_VolumeSpotlightMatrixParameter;
	Display::C_PixelShaderParameterArray	m_VolumeSpotlightToWorldParameter;
	Display::C_PixelShaderParameter			m_VolumeSpotlightDirectionParameter;
	Display::C_PixelShaderParameter			m_VolumeSpotlightColourParameter;
	Display::C_PixelShaderParameter			m_VolumeSpotlightDepthParameter;
	Display::C_PixelShaderParameter			m_VolumeSpotlightLuminanceParameter;

};

}; // namespace Deferred
}; // namespace Render

#endif // RENDER_DEFERRED_DYNAMIC_LIGHTS_H
