#ifndef DISPLAY_VIEW_VIEW_INL
#define DISPLAY_VIEW_VIEW_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE const S32 C_View::GetIndex() const
{
	return m_Index;
}

INLINE const U32 C_View::GetType() const
{
	return m_Type;
}
	
INLINE const S32 C_View::GetUserIndex() const
{
	return m_UserIndex;
}

INLINE const C_Viewport& C_View::GetViewport() const
{
	return m_Viewport;
}

INLINE const C_ViewFrustum& C_View::GetViewFrustum() const
{
	return m_ViewFrustum;
}

INLINE const C_ViewSettings2D& C_View::GetViewSettings2D() const
{
	return m_ViewSettings2D;
}

INLINE const C_ViewSettings3D& C_View::GetViewSettings3D() const
{
	return m_ViewSettings3D;
}

INLINE void C_View::SetIndex(const S32 index)
{
	m_Index = index;
}

INLINE void C_View::SetType(const U32 type)
{
	m_Type = type;
}

INLINE void C_View::SetUserIndex(const S32 userIndex)
{
	m_UserIndex = userIndex;
}

INLINE void C_View::SetViewFrustum(const C_ViewFrustum& viewFrustum)
{
	m_ViewFrustum = viewFrustum;
}

}; // namespace Display

#endif // DISPLAY_VIEW_VIEW_INL
