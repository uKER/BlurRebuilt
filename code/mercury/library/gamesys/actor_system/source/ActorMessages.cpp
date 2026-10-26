#include "ActorSystemPch.h"
#include <actor_system/ActorMessages.h>

namespace Actor
{

/**
@ingroup actor_group_messages

If your game is using the Actor System message types then it should call 
this function near startup to register debug info for those message types.
*/
void RegisterMessageDebugInfo()
{
	// E_ActorMsg_Count
	{
		const char* pMessageNames [] = 
		{
			"E_ActorMsg_GoToSleep",
			"E_ActorMsg_WakeUp",
			"E_ActorMsg_SleepHeartbeat",
			
			"E_ActorMsg_Kill",

			"E_ActorMsg_AddSubscriber",
			"E_ActorMsg_RemoveSubscriber",
			"E_ActorMsg_SetSubscriberMask",

			"E_ActorMsg_SetUpdateMask",

			"E_ActorMsg_ComponentPrimed",
			"E_ActorMsg_ComponentCreated",
			"E_ActorMsg_ActorComponentCreated",

			"E_ActorMsg_Created",
			"E_ActorMsg_Killed",
		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, E_ActorMsg_Count, E_ActorMsg_Begin);
	}
}

} // namespace Actor
