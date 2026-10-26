#include "stdafx.h"

#include <Game/Game.h>

#include <GameLogic\ActorStudioActors\CutSceneVehiclePositionActor.h>

#include <AI\AI_Messages.h>

// Gameplay Editor Create
XT_TYPE(Actors::C_CutSceneVehiclePositionActorCreate, "CutSceneVehiclePositionActor")
	XT_BASE(Actors::C_PosActorCreate)
//	XT_FIELD(m_VehicleIndex, "VehicleIndex")
XT_END_TYPE

namespace Actors
{

bool C_CutSceneVehiclePositionActor::CreateImpl()
{
	if(!C_GameActor::CreateImpl())
		return false;

	const C_CutSceneVehiclePositionActorCreate* positionActorCreate(GetCreationData());
	m_WorldMatrix = GetCreationData()->GetCreationMatrix();
	bool created = true;

	return created;
}

bool C_CutSceneVehiclePositionActor::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	//switch(pMessage->Type())
	//{
	//	default:
			return C_GameActor::OnMessage(pMessage);
	//}
	//return true;
}

}	//namespace Actors