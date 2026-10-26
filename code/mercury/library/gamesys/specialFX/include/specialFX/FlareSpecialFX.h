#ifndef FLARESPECIALFX_H
#define FLARESPECIALFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SpecialFX/SpecialFX.h>
#include <Dynamic/Light/Generic/XtInterface.h>

namespace SpecialFX
{

class C_FlareSpecialFX : public SpecialFX::C_BaseSpecialFXComponent
{
public:
	//XT Data
	XtVec3													m_Offset;
	XtF32													m_RotationX;
	XtF32													m_RotationY;
	XtF32													m_Time;
	XtArray<SpecialFX::C_HermiteKeyFrame>					m_IntensityGraph;
	XtHandle<DynamicEffect::Light::XtInterface::Light>		m_XtHandle;

	virtual SpecialFX::I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const;
};
}

#endif
