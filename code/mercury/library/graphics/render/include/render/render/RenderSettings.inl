#ifndef RENDER_RENDER_SETTINGS_INL
#define RENDER_RENDER_SETTINGS_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{

INLINE const bool C_RenderSettings::ColourBufferSupported()
{
	return m_ColourBufferSupported;
}

INLINE const bool C_RenderSettings::ColourBufferEnabled()
{
	return m_ColourBufferEnabled;
}

INLINE const bool C_RenderSettings::DepthStencilBufferSupported()
{
	return m_DepthStencilBufferSupported;
}

INLINE const bool C_RenderSettings::DepthStencilBufferEnabled()
{
	return m_DepthStencilBufferEnabled;
}

INLINE const bool C_RenderSettings::HDRSupported()
{
	return m_HDRSupported;
}

INLINE const bool C_RenderSettings::HDREnabled()
{
	return m_HDREnabled;
}

INLINE const bool C_RenderSettings::DOFSupported()
{
	return m_DOFSupported;
}

INLINE const bool C_RenderSettings::DOFEnabled()
{
	return m_DOFEnabled;
}

INLINE const bool C_RenderSettings::ShadowMapSupported()
{
	return m_ShadowMapSupported;
}

INLINE const bool C_RenderSettings::ShadowMapEnabled()
{
	return m_ShadowMapEnabled;
}

INLINE const bool C_RenderSettings::OcclusionQueriesSupported()
{
	return m_OcclusionQueriesSupported;
}
	
INLINE const bool C_RenderSettings::OcclusionQueriesEnabled()
{
	return m_OcclusionQueriesEnabled;
}
	
INLINE const F32 C_RenderSettings::ShadowMapCullingDistance()
{
	return m_ShadowMapCullingDistance;
}

} // namespace Render

#endif // RENDER_RENDER_SETTINGS_INL
