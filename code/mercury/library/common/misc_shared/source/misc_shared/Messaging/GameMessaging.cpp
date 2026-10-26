#include "MiscSharedPch.h"
#include <misc_shared/Messaging/GameMessaging.h>

#include <misc_shared/Messaging/MessageDebug.h>


/**
@namespace GameMessaging

Provides basic message types for games to build upon.

Games must unsure that message identifiers assigned by their code, or by other
systems, do not conflict with those provided by E_GameMsg.

In non-Master configurations, the RegisterMessageDebugInfo() function
should be used to add debug info.
*/

namespace GameMessaging
{

#ifndef CFG_GOLD

/**
If your game is using the GameMessaging message types then it should call 
this function near startup to register debug info for those message types.
*/
void RegisterMessageDebugInfo()
{
	const char* pMessageNames [] = 
	{
		"E_GameMsg_Update",
		"E_GameMsg_PreUpdate",
		"E_GameMsg_PostUpdate",
		"E_GameMsg_FinalUpdate",
		"E_GameMsg_UpdateEnable",
		"E_GameMsg_UpdateDisable",
		"E_GameMsg_ClientUpdate",
		"E_GameMsg_PausedUpdate",
		"E_GameMsg_ChangeState",
		"E_GameMsg_DebugRenderOn",
		"E_GameMsg_DebugRenderOff",
		"E_GameMsg_DebugRenderSpecific",
		"E_GameMsg_DebugBreak"
	};

	REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, E_GameMsg_Count, E_GameMsg_Update);
}

#endif // ndef CFG_GOLD

} // namespace GameMessaging
