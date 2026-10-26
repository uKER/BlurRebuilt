#ifndef DEFERRED_PREPASS_TILED_VOLUME_SPOTLIGHTS_H
#define DEFERRED_PREPASS_TILED_VOLUME_SPOTLIGHTS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Btl/Containers/Vector.h>

namespace Deferred
{

struct C_TiledVolumeSpotlight
{
	MathsStorage::Float3		m_vPosition;
	MathsStorage::Float3		m_vColour;
	MathsStorage::MatrixRow44	m_mMatrix;
};

typedef Btl::Vector<C_TiledVolumeSpotlight> TiledVolumeSpotlights;

struct C_VisibleTiledVolumeSpotlight
{
	Maths::C_Rect				m_rectangle;
	U32							m_sortKey;
	C_TiledVolumeSpotlight*		m_pLight;
};

typedef Btl::Vector<C_VisibleTiledVolumeSpotlight> VisibleTiledVolumeSpotlights;

inline U32 RadixKey(const C_VisibleTiledVolumeSpotlight& light)
{
	return light.m_sortKey;
}

enum E_TiledVolumeSpotlightsShader
{
	E_TiledVolumeSpotlightsShader_1 = 0,
	E_TiledVolumeSpotlightsShader_2,
	E_TiledVolumeSpotlightsShader_3,
	E_TiledVolumeSpotlightsShader_4,
	E_TiledVolumeSpotlightsShader_5,
	E_TiledVolumeSpotlightsShader_6,
	E_TiledVolumeSpotlightsShader_7,
	E_TiledVolumeSpotlightsShader_8,
	E_TiledVolumeSpotlightsShader_9,
	E_TiledVolumeSpotlightsShader_10,
	E_TiledVolumeSpotlightsShader_11,
	E_TiledVolumeSpotlightsShader_12,
	E_TiledVolumeSpotlightsShader_Count
};

class C_TiledVolumeSpotlights : public Display::I_Renderer
{
	static const U32 c_MaximumLights = 12;
public:

	C_TiledVolumeSpotlights();
	~C_TiledVolumeSpotlights();

	bool	OnCreate();
	void	OnDestroy();

	void	Render() const;

	void	AddLight(const MathsStorage::Float3& vPosition, const MathsStorage::Float3& vColour, const MathsStorage::MatrixRow44& mMatrix);

	void	SetTesselation(const U32 x, const U32 y)											{ m_HorizontalTiles = x; m_VerticalTiles = y; }
	void	SetDepthTexture(const Display::C_Texture2D* pDepthTexture)							{ m_pDepthTexture = pDepthTexture; }
	void	SetNormalTexture(const Display::C_Texture2D* pNormalTexture)						{ m_pNormalTexture = pNormalTexture; }
	void	SetVolumeTexture(const Display::C_Texture3D* pVolumeTexture)						{ m_pVolumeTexture = pVolumeTexture; }

private:

	void	GatherVisibleLights(VisibleTiledVolumeSpotlights& visibleLights) const;
	void	GatherVisibleParameters(const VisibleTiledVolumeSpotlights& visibleLights, const Maths::C_Rect& rectangle, U32& lightCount, Display::C_Float4* pPositions, Display::C_Float4* pColours, MathsStorage::MatrixRow44* pMatrices) const;

private:

	mutable TiledVolumeSpotlights m_TiledVolumeSpotlights;

	U32	m_HorizontalTiles;
	U32	m_VerticalTiles;

private:

	Display::C_RenderTexture2D	m_RenderTexture;

	const Display::C_Texture2D*	m_pDepthTexture;
	const Display::C_Texture2D*	m_pNormalTexture;
	const Display::C_Texture3D*	m_pVolumeTexture;

	Display::C_VertexBuffer		m_VertexBuffer;

	Display::C_VertexShader		m_VertexShader;
	Display::C_PixelShader		m_PixelShader[E_TiledVolumeSpotlightsShader_Count];

	Display::C_PixelShaderParameterArray	m_PositionParameter[E_TiledVolumeSpotlightsShader_Count];
	Display::C_PixelShaderParameterArray	m_ColourParameter[E_TiledVolumeSpotlightsShader_Count];
	Display::C_PixelShaderParameterArray	m_MatrixParameter[E_TiledVolumeSpotlightsShader_Count];
	Display::C_PixelShaderParameter			m_DepthParameter[E_TiledVolumeSpotlightsShader_Count];

};

}; // namespace Deferred

#endif // DEFERRED_PREPASS_TILED_VOLUME_SPOTLIGHTS_H
