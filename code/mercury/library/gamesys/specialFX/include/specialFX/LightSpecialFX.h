#ifndef LIGHTSPECIALFX_H
#define LIGHTSPECIALFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SpecialFX/SpecialFX.h>


namespace SpecialFX
{

class C_LightSpecialFX : public C_BaseSpecialFXComponent
{
public:
	XtArray<C_HermiteKeyFrame>					m_EffectGraph;
	XtF32										m_Time;
	XtVec3										m_Offset;
	XtRgb										m_Colour;
	XtF32										m_Radius;
	XtF32										m_CullDistance;

	virtual I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const;
};
}

#endif