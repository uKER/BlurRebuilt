#ifndef EFFECT_SHADOW_MAP_SETUP_H
#define EFFECT_SHADOW_MAP_SETUP_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Effect
{

class C_ShadowMapSetup
{
public:
	const Display::C_View*	m_pView;
	Maths::HVec3			m_vPosition;
	Maths::HVec3			m_vDirection;
	F32						m_fNearClip;
	F32						m_fFarClip;
	F32						m_fPerspDist;
	Maths::C_Rect			m_vViewExtents;
	U32						m_iWidth;
	U32						m_iHeight;
};

}; // namespace Effect

#endif // EFFECT_SHADOW_MAP_SETUP_H
