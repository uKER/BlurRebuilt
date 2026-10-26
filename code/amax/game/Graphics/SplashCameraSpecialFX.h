#ifndef SPLASHCAMERASPECIALFX_H
#define SPLASHCAMERASPECIALFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SpecialFX/SpecialFX.h>
#include "SplashCamera/SplashCamera.h"

namespace Game
{

class C_SplashCameraSpecialFX : public SpecialFX::C_BaseSpecialFXComponent
{
public:
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_EffectGraph;
	XtF32										m_Time;
	XtHandle<SplashCamera::C_SplashType>		m_SplashCameraType;
	XtF32										m_Radius;
	XtF32										m_SplashesPerSecond;
	XtF32										m_Velocity;
	XtF32										m_Spread;

	virtual SpecialFX::I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const;
};
}
#endif