#ifndef GRAVITY_EVENT_CALLBACK_H
#define GRAVITY_EVENT_CALLBACK_H

#include "gravity/sharedphysics/MemoryAllocator.h"
#include "gravity/sdk/BodyEvents.h"

namespace Gravity
{
	class C_EventCallback
	{
	public:
		virtual void ProcessEvents(SharedPhysics::C_MemoryIterator<C_BodyEvent>& eventIter, const U32 numEvents) = 0;
	};

}


#endif
