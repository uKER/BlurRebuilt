#ifndef POSTSPECIALFX_H
#define POSTSPECIALFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SpecialFX/SpecialFX.h>


namespace Game
{
struct PostFxMatrix;

class C_PostSpecialFX : public SpecialFX::C_BaseSpecialFXComponent
{
public:
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_EffectGraph;
	XtF32										m_Time;
	XtArray<PostFxMatrix*>						m_Matrices;
	PostFxMatrix*								m_Root;
	XtF32										m_FilmGrainIntensity;
	XtF32										m_VignetteWeight;
	XtF32										m_TunnelAmount;
	XtF32										m_ColourSep, m_Blockiness, m_ScreenOverlay;
	XtF32										m_WorldSpaceEffectRadius;
	XtF32										m_WorldSpaceEffectCameraDirectionFactor;

	virtual SpecialFX::I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const;
};
}
#endif