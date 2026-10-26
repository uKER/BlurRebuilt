#ifndef GAME_COMPONENTS_H
#define GAME_COMPONENTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "misc_shared/Messaging/MessageBase.h"
#include "misc_shared/UID/FourCC.h"
#include "actor_system/ActorComponent.h"

#include <shared_actors/Components/SharedComponents.h>

class C_GameWorld;



class I_GameActorComponent : public Actor::I_ActorComponent
{
public:
	
	virtual bool		OnMessage(const Messaging::C_MessageBase *pMessage ) {return I_ActorComponent::OnMessage(pMessage);}	// component message handler

};

namespace Components
{

// enum for game component interfaces
enum E_GameComponentIF
{

	E_GameComponentIF_HiBits = FOURCC_HI20('AGCI'), // AMAX Game Component Interface

#ifndef NOT_USING_AUDIO_COMPONENETS
	E_IF_AudioEntity,

//	// Audio Components
	E_AudioEntity_Spawned,
	E_AudioEntity_AdditionalSpawned,
	E_AudioEntity_ActorStudio,
#endif
	E_MovementDualSpline,

//	Vehicle Logic Component Interfaces
	E_IF_VehicleComponent_Damage,
	E_IF_VehicleComponent_HandOfGod,
	E_IF_VehicleComponent_Statistics,
	E_IF_VehicleComponent_Points,
	E_IF_VehicleComponent_Pickups,
	E_IF_VehicleComponent_Pickups_AI,
	E_IF_VehicleComponent_Pickups_Targetting,
	E_IF_VehicleComponent_AIController,
	E_IF_VehicleComponent_PlayerRubberBanding,
	E_IF_VehicleComponent_Performance_Modifier,
	E_IF_VehicleComponent_Domination,
	E_IF_VehicleComponent_Drafting,
	E_IF_VehicleComponent_E3Controller,
	E_IF_VehicleComponent_Rumble,
	E_IF_VehicleComponent_Challenges,
	E_IF_VehicleComponent_Fan_Demand,
	E_IF_VehicleComponent_Tutorials,
	E_IF_VehicleComponent_Sensor,
	E_IF_VehicleComponent_NewScoring,
	E_IF_VehicleComponent_MP_IdleKickController,
	E_IF_VehicleComponent_Abilities,
	E_IF_VehicleComponent_SuperControlController,
};

enum E_GameComponent
{
	E_GameComponent_HiBits = FOURCC_HI20('AGCM'), // AMAX Game Component

//	Vehicle Logic Components
	E_VehicleComponent_Damage,
	E_VehicleComponent_HandOfGod,
	E_VehicleComponent_Statistics,
	E_VehicleComponent_Points,
	E_VehicleComponent_Pickups,
	E_VehicleComponent_Pickups_AI,
	E_VehicleComponent_Pickups_Targetting,
	E_VehicleComponent_AIController,
	E_VehicleComponent_Performance_Modifier,
	E_VehicleComponent_Drafting,
	E_VehicleComponent_E3Controller,
	E_VehicleComponent_Rumble,
	E_VehicleComponent_PlayerRubberBanding,
	E_VehicleComponent_Fan_Demand,
	E_VehicleComponent_Tutorials,
	E_VehicleComponent_Sensor,
	E_VehicleComponent_NewScoring,
	E_VehicleComponent_Abilities,
	E_VehicleComponent_SuperControlController,

//	MP-specific Vehicle Logic Components
	E_VehicleComponent_MP_Domination,
	E_VehicleComponent_MP_Network_Damage,
	E_VehicleComponent_MP_Network_Pickups,
	E_VehicleComponent_MP_Local_Pickups,
	E_VehicleComponent_MP_Pickups_AI,
	E_VehicleComponent_MP_Challenges,
	E_VehicleComponent_MP_IdleKickController,
	E_VehicleComponent_MP_Network_Abilities,
};

void RegisterGameComponents(void);

} // namespace Components


#endif
