#ifndef RENDER_DYNAMICEFFECT_PARTICLE_EVENTINSTANCEHANDLE_H
#define RENDER_DYNAMICEFFECT_PARTICLE_EVENTINSTANCEHANDLE_H

#ifdef _MSC_VER
#pragma once
#endif

// include node here since it is unlikely to change
#include <Dynamic/Handle.h>

namespace DynamicEffect
{
namespace Particle
{

class C_EventInstance;

class C_EventInstanceHandle : public C_Handle<C_EventInstance>
{
public:
	C_EventInstanceHandle() : C_Handle<C_EventInstance>() {}
	C_EventInstanceHandle(C_EventInstance* pEventInstance) : C_Handle<C_EventInstance>(pEventInstance) {}

	friend class C_EventInstanceHandleNamed;
};

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EVENTINSTANCEHANDLE_H
