#include "stdafx.h"
#include "StartPositionActor.h"

namespace Actors
{

using namespace Maths;

bool StartPositionActor::InitialiseImpl()
{
	mWorldMatrix = Load(GetCreationData()->m_WorldMatrix);
	return true;
}

bool StartPositionActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	return Actor::C_Actor::OnMessage(pMessage);
}

}

//The sorcery definition for the creation data structure
XT_TYPE(Actors::StartPositionCreate, "StartPosition")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(mStartNumber, "StartNumber")
//	XT_FIELD(mStartType, "StartType")
XT_END_TYPE
