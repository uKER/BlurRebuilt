#include "stdafx.h"
#include <actor_studio_msg_shared/Messages.h>

namespace ActorStudioMsgShared
{
	bool C_DragMsg::RelativelyEqual(const C_DragMsg& to, S32 maxUlps)
	{
		Maths::HVec4 ori = m_Orientation;
		Maths::HVec4 oriTo = to.m_Orientation;
		
		bool retVal = 
			Compare(m_Position[0], to.m_Position[0], maxUlps) &&
			Compare(m_Position[1], to.m_Position[1], maxUlps) &&
			Compare(m_Position[2], to.m_Position[2], maxUlps) &&
			Compare(ori[0], oriTo[0], maxUlps) &&
			Compare(ori[1], oriTo[1], maxUlps) &&
			Compare(ori[2], oriTo[2], maxUlps) &&
			Compare(ori[3], oriTo[3], maxUlps);

		return retVal;
	}

	bool C_DragMsg::Compare(F32 x, F32 y, S32 maxUlps)
	{
		S32 x1=*(S32*)&x; S32 y1=*(S32*)&y;
		if (x1<0) x1 = 0x80000000-x1;
		if (y1<0) y1 = 0x80000000-y1;

		return abs(x1-y1) < maxUlps;
	}
}