#include "stdafx.h"

#include "GameComponents.h"

#include <Game\GameActors\Components\VehicleLogic\VLC_Damage.h>
#include <Game\GameActors\Components\VehicleLogic\HandofGod\VLC_HandOfGod.h>
#include <Game\GameActors\Components\VehicleLogic\HandofGod\VLC_AIController.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_Network_Damage.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_E3Controller.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Domination.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Local_Pickups.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Network_Pickups.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Pickups_AI.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_PlayerRubberBanding.h>
#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_PointsSystem.h>
#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_Statistics.h>
#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups_AI.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups_Targetting.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Performance_Modifier.h>
#include <shared_actors/Components/Creation/BaseElementManagerComponentsCreate.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Drafting.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Rumble.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_FanDemands.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Tutorials.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MultiplayerChallenges.h>
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_IdleKickController.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Abilities.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Network_Abilities.h>
#include <Game\GameActors\Components\VehicleLogic\HandOfGod\VLC_SuperControlController.h>

#include <Game\GameActors\Components\VehicleLogic\VLC_Sensor.h>

#ifndef NOT_USING_AUDIO_COMPONENETS
#include "AudioComponents.h"
#endif

#include <shared_actors\Components\Creation\BaseElementManagerComponentsCreate.h>
#include <shared_actors/Components/Creation/MessageComponentsCreate.h>
#include <shared_actors\Components\Creation\PhysicsBodyComponentCreate.h>
#include <shared_actors/Components/Creation/UpdateManagerCreate.h>
#include <actor_system/GameComponentFactory.h>
#include <Game/Game.h>
#include "GameLogic/ActorStudioActors/MovementDualSpline.h"

#include <GameLogic/ActorStudioActors/DynamicActorComponents.h>


XT_LIBRARY(ComponentCreation)
	XT_DEPENDS_ON(SharedComponentCreation)
#ifndef NOT_USING_AUDIO_COMPONENETS
	XT_DEPENDS_ON(AudioComponentCreation)
#endif
XT_END_LIBRARY


// Register the actor components
void Components::RegisterGameComponents(void)
{
	using namespace Components;

	Component::C_GameComponentFactory *factory = Component::C_GameComponentFactory::GetInstance();
	assert(factory);

	//RegisterSharedComponents();

#ifndef NOT_USING_AUDIO_COMPONENETS
	// audio components
	factory->Register<C_AudioEntity_Spawned>();
	factory->Register<C_AudioEntity_AdditionalSpawned>();
	factory->Register<C_AudioEntity_ActorStudio>();
#endif

#ifndef NOT_USING_AUDIO_COMPONENETS
	Component::C_ComponentCreatorFactory::Register<C_AudioEntity_SpawnedCreateInfo>();
	Component::C_ComponentCreatorFactory::Register<C_AudioEntity_AdditionalSpawnedCreateInfo>();
#endif

#ifndef NOT_USING_AUDIO_COMPONENETS
	factory->RegisterComponentIFName(E_IF_AudioEntity, "E_IF_AudioEntity");

	factory->RegisterComponentIFName(E_AudioEntity_Spawned, "E_AudioEntity_Spawned");
	factory->RegisterComponentIFName(E_AudioEntity_AdditionalSpawned, "E_AudioEntity_AdditionalSpawned");
	factory->RegisterComponentIFName(E_AudioEntity_ActorStudio, "E_AudioEntity_ActorStudio");
#endif


	// registering a unique indentifier string to the system
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Damage, "E_IF_VehicleComponent_Damage");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_HandOfGod, "E_IF_VehicleComponent_HandOfGod");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Points, "E_IF_VehicleComponent_Points");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Statistics, "E_IF_VehicleComponent_Statistics");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Pickups, "E_IF_VehicleComponent_Pickups");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Pickups_AI, "E_IF_VehicleComponent_Pickups_AI");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Pickups_Targetting, "E_IF_VehicleComponent_Pickups_Targetting");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_AIController, "E_IF_VehicleComponent_AIController");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_PlayerRubberBanding, "E_IF_VehicleComponent_PlayerRubberBanding");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Performance_Modifier, "E_IF_VehicleComponent_Performance_Modifier");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Domination, "E_IF_VehicleComponent_Domination");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Drafting, "E_IF_VehicleComponent_Drafting");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Challenges, "E_IF_VehicleComponent_Challenges");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_E3Controller, "E_IF_VehicleComponent_E3Controller");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Rumble, "E_IF_VehicleComponent_Rumble");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Fan_Demand, "E_IF_VehicleComponent_Fan_Demand");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Tutorials, "E_IF_VehicleComponent_Tutorials");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_E3Controller, "E_IF_VehicleComponent_MP_IdleKickController");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Sensor, "E_IF_VehicleComponent_Sensor");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_NewScoring, "E_IF_VehicleComponent_NewScoring");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_Abilities, "E_IF_VehicleComponent_Abilities");
	factory->RegisterComponentIFName(E_IF_VehicleComponent_SuperControlController, "E_IF_VehicleComponent_SuperControlController");



	// every concrete component type should be registered in this way
	factory->Register<C_VehicleComponent_Damage>();
	factory->Register<C_VehicleComponent_HandOfGod>();
	factory->Register<C_VehicleComponent_PointsSystem>();
	factory->Register<C_VehicleComponent_Statistics>();
	factory->Register<C_VehicleComponent_Pickups>();
	factory->Register<C_VehicleComponent_Pickups_AI>();
	factory->Register<C_VehicleComponent_Pickups_Targetting>();
	factory->Register<C_VehicleComponent_AIController>();
	factory->Register<C_VehicleComponent_PlayerRubberBanding>();
	factory->Register<C_VehicleComponent_Performance_Modifier>();
	factory->Register<C_VehicleComponent_MP_Domination>();
	factory->Register<C_VehicleComponent_MP_Local_Pickups>();
	factory->Register<C_VehicleComponent_MP_Network_Damage>();
	factory->Register<C_VehicleComponent_MP_Network_Pickups>();
	factory->Register<C_VehicleComponent_MP_Pickups_AI>();
	factory->Register<C_VehicleComponent_MP_Challenges>();
	factory->Register<Components::C_BaseDamageableElementManager>();
	factory->Register<C_VehicleComponent_Drafting>();
	factory->Register<C_VehicleComponent_E3Controller>();
	factory->Register<C_VehicleComponent_Rumble>();
	factory->Register<C_VehicleComponent_Fan_Demand>();
	factory->Register<C_VehicleComponent_Tutorials>();
	factory->Register<C_VehicleComponent_MP_IdleKickController>();
	factory->Register<Kanzen::C_VLC_Sensor>();
	factory->Register<C_VehicleComponent_NewScoring>();
	factory->Register<C_VehicleComponent_Abilities>();
	factory->Register<C_VehicleComponent_Network_Abilities>();
	factory->Register<C_VehicleComponent_SuperControlController>();

	// run time allocation, this shouldn't be called if allocating from sorcery
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Damage_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_HandOfGod_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_PointsSystem_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Statistics_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Pickup_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Pickup_AI_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Pickup_Targetting_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_AIController_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_PlayerRubberBanding_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Performance_Modifier_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_MP_Domination_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_MP_Local_Pickups_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_MP_Network_Damage_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_MP_Pickup_AI_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_MP_Network_Pickups_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_MP_Challenges_CreateInfo>();

	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Drafting_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_E3Controller_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Rumble_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Fan_Demand_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Tutorials_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_MP_IdleKickController_CreateInfo>();

	Component::C_ComponentCreatorFactory::Register<Kanzen::I_VLC_Sensor_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Abilities_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_Network_Abilities_CreateInfo>();
	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_SuperControlController_CreateInfo>();

	Component::C_ComponentCreatorFactory::Register<Components::C_BaseDamageableElementManagerCreateInfo>();
	Component::C_ComponentCreatorFactory::Register<Components::C_BasePhysicalObjectPhysicsBodyCreateInfo>();

	Component::C_ComponentCreatorFactory::Register<I_VehicleComponent_NewScoring_CreateInfo>();


	RegisterDynamicActorComponents();
}
