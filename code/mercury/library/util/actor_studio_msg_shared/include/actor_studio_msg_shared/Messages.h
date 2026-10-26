#ifndef ACTOR_STUDIO_MSG_SHARED_MESSAGES
#define ACTOR_STUDIO_MSG_SHARED_MESSAGES

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_studio_msg_shared/MessageTypes.h>

namespace ActorStudioMsgShared
{
	class C_DragMsg
	{
	public:
		Maths::HVec3		m_Position; ///< new position
		Maths::C_Quaternion	m_Orientation; ///< new orientation

		bool RelativelyEqual(const C_DragMsg& to, S32 maxUlps);

	private:
		bool Compare(F32 x, F32 y, S32 maxUlps);
	};
}

#endif //ACTOR_STUDIO_MSG_SHARED_MESSAGES