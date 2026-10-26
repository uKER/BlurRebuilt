#ifndef RENDER_RENDER_SETTINGS_H
#define RENDER_RENDER_SETTINGS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{

//! C_RenderSettings

class C_RenderSettings
{
public:

	C_RenderSettings();
	~C_RenderSettings();
	
	static const bool ColourBufferSupported();
	static const bool ColourBufferEnabled();
	static void ColourBufferEnabled(const bool colourBufferEnabled);

	static const bool DepthStencilBufferSupported();
	static const bool DepthStencilBufferEnabled();
	static void DepthStencilBufferEnabled(const bool depthStencilBufferEnabled);

	static const bool HDRSupported();
	static const bool HDREnabled();
	static void HDREnabled(const bool hdrEnabled);

	static const bool DOFSupported();
	static const bool DOFEnabled();
	static void DOFEnabled(const bool dofEnabled);
	
	static const bool ShadowMapSupported();
	static const bool ShadowMapEnabled();
	static void ShadowMapEnabled(const bool shadowMapEnabled);

	static const bool OcclusionQueriesSupported();
	static const bool OcclusionQueriesEnabled();
	static void OcclusionQueriesEnabled(const bool occlusionQueriesEnabled);

	static const F32 ShadowMapCullingDistance();
	static void ShadowMapCullingDistance(const F32 shadowMapCullingDistance);

	static void InitialiseSettings();

	static void DumpSettings();

private:

	static bool			m_ColourBufferEnabled;
	static bool			m_ColourBufferSupported;
	
	static bool			m_DepthStencilBufferEnabled;
	static bool			m_DepthStencilBufferSupported;

	static bool			m_HDREnabled;
	static bool			m_HDRSupported;

	static bool			m_DOFEnabled;
	static bool			m_DOFSupported;

	static bool 		m_ShadowMapEnabled;
	static bool 		m_ShadowMapSupported;

	static bool			m_OcclusionQueriesEnabled;
	static bool			m_OcclusionQueriesSupported;

	static F32			m_ShadowMapCullingDistance;
};

} // namespace Render

#include "Render/Render/RenderSettings.inl"

#endif // RENDER_RENDER_SETTINGS_H
