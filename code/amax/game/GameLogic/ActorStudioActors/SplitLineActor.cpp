#include "stdafx.h"
#include "SplitLineActor.h"

namespace Actors
{
	bool SplitLineActor::InitialiseImpl()
	{
		mWorldMatrix = Load(GetCreationData()->m_WorldMatrix);
		return true;
	}

	bool SplitLineActor::OnMessage(const Messaging::C_MessageBase *pMessage)
	{
		return true;
	}
}

//The sorcery definition for the creation data structure
XT_TYPE(Actors::SplitLineCreate, "SplitLine")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(mSplitNumber, "SplitNumber")
XT_END_TYPE
