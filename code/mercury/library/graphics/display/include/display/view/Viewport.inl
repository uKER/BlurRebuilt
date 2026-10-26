#ifndef DISPLAY_VIEW_VIEWPORT_INL
#define DISPLAY_VIEW_VIEWPORT_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
	
INLINE const U32 C_Viewport::GetX() const
{
	return m_X;
}

INLINE const U32 C_Viewport::GetY() const
{
	return m_Y;
}

INLINE const U32 C_Viewport::GetW() const
{
	return m_W;
}

INLINE const U32 C_Viewport::GetH() const
{
	return m_H;
}

INLINE const F32 C_Viewport::GetMinZ() const
{
	return m_MinZ;
}

INLINE const F32 C_Viewport::GetMaxZ() const
{
	return m_MaxZ;
}

}; // namespace Display

#endif // DISPLAY_VIEW_VIEWPORT_INL
