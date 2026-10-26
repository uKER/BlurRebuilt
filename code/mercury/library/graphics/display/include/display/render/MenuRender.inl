#ifndef DISPLAY_RENDER_MENU_RENDER_INL
#define DISPLAY_RENDER_MENU_RENDER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE const Util::C_String& C_MenuItem::GetName()
{
	return m_Name;
}

INLINE void C_MenuItem::SetName(const Util::C_String& name)
{
	m_Name = name;
}

INLINE bool C_MenuRender::GetShow()
{
	return m_Show;
}

INLINE void C_MenuRender::SetShow(const bool show)
{
	m_Show = show;
}

INLINE void C_MenuRender::IncX(const F32 delta)
{
	m_X += delta;
}

INLINE void C_MenuRender::IncY(const F32 delta)
{
	m_Y += delta;
}

INLINE void C_MenuRender::IncW(const F32 delta)
{
	m_W *= delta;
}

INLINE void C_MenuRender::IncH(const F32 delta)
{
	m_H *= delta;
}
	
INLINE C_MenuRender* MenuRender()
{
	assert(g_pMenuRender);
	return g_pMenuRender;
}

INLINE const bool MenuRenderExists()
{
	return g_pMenuRender != NULL;
}

}; // namespace Display

#endif // DISPLAY_RENDER_MENU_RENDER_INL
