#ifndef ACTOR_STUDIO_MSG_SHARED_MESSAGE_TYPES
#define ACTOR_STUDIO_MSG_SHARED_MESSAGE_TYPES

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>

namespace ActorStudioMsgShared
{
	enum E_ActorStudioMsg
	{
		E_ActorStudioMsg_HiBits = FOURCC_HI20('ASTD'),

		E_ActorStudioMsg_Drag,
		E_ActorStudioMsg_EndDrag,
		E_ActorStudioMsg_Selected,
		E_ActorStudioMsg_Deselected,

		E_ActorStudioMsg_End, 
		E_ActorStudioMsg_Begin = E_ActorStudioMsg_HiBits + 1,
		E_ActorStudioMsg_Count = E_ActorStudioMsg_End - E_ActorStudioMsg_Begin
	};
}

#endif //ACTOR_STUDIO_MSG_SHARED_MESSAGE_TYPES