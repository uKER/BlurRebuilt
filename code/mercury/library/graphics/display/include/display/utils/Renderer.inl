#ifndef DISPLAY_UTILS_RENDERER_INL
#define DISPLAY_UTILS_RENDERER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE const bool I_Renderer::IsRendererCreated() const
{
	return m_Created[E_RendererState_Involatile] && m_Created[E_RendererState_Volatile];
}

INLINE const bool I_Renderer::IsRendererCreated(const E_RendererState state) const
{
	assert(state < E_RendererState_Count);
	return m_Created[state];
}

INLINE const Util::C_String& I_Renderer::GetRendererName() const
{
	return m_Name;
}

INLINE const E_RendererType I_Renderer::GetRendererType() const
{
	return m_Type;
}

INLINE ResourceBundler::I_ResourceBundle* I_Renderer::GetResourceBundle() 
{
	return m_pResourceBundle;
}

INLINE void I_Renderer::SetAssetPath(const Util::C_String& path)
{
	m_AssetPath = path;
}

INLINE void I_Renderer::SetResourceBundle(ResourceBundler::I_ResourceBundle* pResourceBundle)
{
	m_pResourceBundle = pResourceBundle;
}

INLINE const Util::C_String& I_Renderer::GetAssetPath() const
{
	return m_AssetPath;
}

INLINE bool I_Renderer::Created(const E_RendererState type) const
{
	return m_Created[type];
}

INLINE bool I_Renderer::Initialised() const
{
	return m_Initialised;
}

}; // namespace Display

#endif // DISPLAY_UTILS_RENDERER_INL
