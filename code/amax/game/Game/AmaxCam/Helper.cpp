
#include "stdafx.h"
#include "Helper.h"

#include "GameApp.h"
#include "Game/GameWorld/GameWorld.h"
#include "Game/GameActors/Vehicles/VehicleActor.h"
#include "GameLogic/Layers/RaceBookLayer.h"
#include "GameLogic/Layers/RaceLogicLayer.h"
#include "GameLogic/Layers/AmaxWorldLayer.h"
#include "GameLogic/RaceLogicCore/Core/RaceLogicCore.h"
#include "Vehicle/VehicleModel.h"

#include <actor_system/ActorSys.h>
#include <camera2.h>

// Amax Camera helper functions

namespace AmaxCam
{
using namespace Actor;
using namespace Cam2;
using namespace Game::Actors;
using namespace Maths;

static Game::C_AmaxWorldLayer* GetAmaxWorldLayer()
{
	C_GameApp* gameApp = C_GameApp::GetInstance();
	if (!gameApp)
		return 0;

	Game::C_RaceBookLayer* rbl = gameApp->GetRaceBookLayer();
	if (!rbl)
		return 0;

	return rbl->GetAmaxWorldLayer();
}

Game::C_GameWorld* GetGameWorld()
{
	Game::C_AmaxWorldLayer* awl = GetAmaxWorldLayer();
	if (!awl)
		return 0;

	return awl->GetGameWorld();
}

Actor::C_ActorList* GetActorList()
{
	Game::C_GameWorld* world = GetGameWorld();
	if (!world)
		return 0;
	
	return world->GetActorList();
}

RaceLogicCore* GetRaceLogicCore()
{
	Game::C_AmaxWorldLayer* awl = GetAmaxWorldLayer();
	if (!awl)
		return 0;

	return awl->GetRaceLogicCore();
}

Game::C_RaceLogicLayer* GetRaceLogicLayer()
{
	C_GameApp* gameApp = C_GameApp::GetInstance();
	if (!gameApp)
		return 0;

	Game::C_RaceBookLayer* rbl = gameApp->GetRaceBookLayer();
	if (!rbl)
		return 0;

	return rbl->GetRaceLogicLayer();
}

const C_VehicleLogicActor* GetVehicleLogicActor(int index)
{
	Game::C_RaceLogicLayer* rll = GetRaceLogicLayer();
	if (!rll)
		return 0;
	
	return rll->GetLogicVehicleActor(index);
}

Actor::C_ActorHandle GetVehicleHandle(S32 index)
{
	static Actor::C_ActorHandle nullHandle;

	RaceLogicCore* rlc = GetRaceLogicCore();
	if (!rlc)
		return nullHandle;

	return rlc->GetActorHandleFromVehicleIndex((U8)index);				 
}

// Get a Vehicle Actor from a actor handle - return 0 if not found (or not a Vehicle Actor)
const C_VehicleActor* GetVehicleActor(const C_ActorHandle& handle)
{
	C_ActorList* actorList = GetActorList();
	if (!actorList)
		return 0;
	
	return ActorCast_Safe<C_VehicleActor>( actorList->GetActorPtr(handle) );
}

// Convert a Vehicle Actor to a camera object
C_CamObj VehicleActorToCameraObject(const C_VehicleActor* vehicleActor)
{
	HMat43 matrix;
	if (vehicleActor)
		matrix = vehicleActor->GetRawMatrix();
	
	BIZ_ASSERT(matrix.AX().Finite());
	BIZ_ASSERT(matrix.AY().Finite());
	BIZ_ASSERT(matrix.AZ().Finite());
	BIZ_ASSERT(matrix.AW().Finite());
	
	C_CamObj object;
	object.position = matrix.AW();
	object.orient = C_Quaternion(matrix.Rotation());
	return object;
}

HVec3 GetVehicleVelocity(const C_VehicleActor* vehicleActor)
{
	Vehicle::C_VehicleModel* model = vehicleActor ? vehicleActor->GetVehicleModel() : 0;
	Vehicle::C_ModelInput* input = model ? model->GetModelInput() : 0;
	HVec3 veloc;
	if (input)
		veloc = input->m_vVelocity;
	return veloc;
}

} // End of namespace
