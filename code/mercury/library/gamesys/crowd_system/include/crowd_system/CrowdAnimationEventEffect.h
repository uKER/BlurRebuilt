#ifndef CROWD_ANIMATION_EVENT_EFFECT_H
#define CROWD_ANIMATION_EVENT_EFFECT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <dynamic/particle/generic/XtInterface.h>
#include <dynamic/particle/Event.h>
#include <dynamic/particle/EventInstance.h>

namespace AnimatedCrowd
{

//! C_CrowdAnimationEventEffect

class C_CrowdAnimationEventEffect
{
public:
	C_CrowdAnimationEventEffect();

	void			SetEvent(XtHandle<DynamicEffect::Particle::XtInterface::Event> eventHandle);
	DynamicEffect::Particle::C_Event*
					GetEvent() const;

	void			SetEventName(const char* pEventName);
	const char*		GetEventName() const;

	bool			IsValid() const;

private:
	XtHandle<DynamicEffect::Particle::XtInterface::Event>	m_EventHandle;
	const char*												m_pEventName;
};

}; // namespace AnimatedCrowd

#endif // CROWD_ANIMATION_EVENT_EFFECT_H
