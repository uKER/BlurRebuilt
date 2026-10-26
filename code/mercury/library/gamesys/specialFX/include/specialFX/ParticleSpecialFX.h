#ifndef PARTICLESPECIALFX_H
#define PARTICLESPECIALFX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>
#include <SpecialFX/SpecialFX.h>
#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>

namespace SpecialFX
{
class C_ParticleSpecialFX : public C_BaseSpecialFXComponent
{
public:
	C_ParticleSpecialFX();

	//XT Data
	XtVec3													m_Offset;
	XtF32													m_CollisionHeightOffset;
	XtHandle<DynamicEffect::Particle::XtInterface::Event>	m_ParticleEvent;

private:
	virtual I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const;
};
}

#endif
