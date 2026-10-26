#include "stdafx.h"
#include <GameLogic\RaceRules\Multiplayer\MpRaceAction_GiftPerk.h>

#include <GameLogic\MultiplayerUtilities.h>
#include <GameLogic\Structures\NetGameVars.h>
#include <Network\Misc\NetDefines.h>
#include <Network/Core/NetServices.h>
#include <Network/player/NetPlayerManager.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\GameComponents.h>
#include <Game\Messaging\PickupActorMessages.h>

#include <ui\UiHelpers.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>
#include <Ui\game\Multiplayer\MultiplayerHelpers.h>

#include <System/Misc/Random.h>


namespace Game
{


C_MpRaceAction_GiftPerk::C_MpRaceAction_GiftPerk(F32 p_Period, E_PICKUP_TYPE p_Type)
: C_MpRaceActionBase(p_Period)
{
	m_PickupType = p_Type;
	m_RandomGenerator = new Util::C_Random();
	m_RandomGenerator->Seed((U32)Time::GetCurrentTime());
}

C_MpRaceAction_GiftPerk::~C_MpRaceAction_GiftPerk()
{
	SAFE_DELETE_PTR(m_RandomGenerator);
}

E_PICKUP_TYPE C_MpRaceAction_GiftPerk::ChooseRandomPickup()
{
	UI::E_Pickup uiPickup = (UI::E_Pickup)m_RandomGenerator->Rand(UI::E_Pickup_Max-1);
	return UI::ConvertUiToGame_Pickup(uiPickup);
}

//	Gifts to all local vehicles
void C_MpRaceAction_GiftPerk_All::PerformAction()
{
	NetVehicleIndexStack localRacing;
	if (GetLocalRacingList(localRacing) == 0)
		return;

	for(S32 i = 0; i < localRacing.Size(); i++)
	{
		E_PICKUP_TYPE type;
		if(m_PickupType != E_PICKUP_TYPE_NONE)
		{
			type = m_PickupType;
		}
		else
		{
			type = ChooseRandomPickup();
		}
		Game::Actors::C_GrantPickupMsg grantPickupMsg(localRacing[i], type);
		Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor(localRacing[i]);
		if(pLogicActor)
			pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);
	}
}

//	Gifts to a single vehicle
void C_MpRaceAction_GiftPerk_Random::PerformAction()
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetPlayer* pHost = services.PlayerManager().HostPlayer(false);
	if (!(pHost && pHost->LocalPlayer()))
		return;

	NetVehicleIndexStack globalRacing;
	if (GetGlobalRacingList(globalRacing) == 0)
		return;

	//	Choose vehicle
	U8 vehicleIndex = globalRacing[m_RandomGenerator->Rand(globalRacing.Size())];
	
	//	Get/Choose pickup type
	E_PICKUP_TYPE type;
	if(m_PickupType != E_PICKUP_TYPE_NONE)
	{
		type = m_PickupType;
	}
	else
	{
		type = ChooseRandomPickup();
	}

	if(VehicleIsLocal(vehicleIndex))
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(vehicleIndex, type);
		Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor(vehicleIndex);
		if(pLogicActor)
			pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);
	}
	else
	{
		//C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupAdded(UI::GetLevelState().mCars[vehicleIndex].mNetJoinId, type, true);
	}	
}




void C_MpRaceAction_GiftPerk_First::PerformAction()
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetPlayer* pHost = services.PlayerManager().HostPlayer(false);
	if (!(pHost && pHost->LocalPlayer()))
		return;

	NetVehicleIndexStack globalRacing;
	if (GetGlobalRacingList(globalRacing) == 0)
		return;

	//	Get/Choose pickup type
	E_PICKUP_TYPE type;
	if(m_PickupType != E_PICKUP_TYPE_NONE)
	{
		type = m_PickupType;
	}
	else
	{
		type = ChooseRandomPickup();
	}

	//	Find leading vehicle index
	U8 vehicleIndex = 0;
	F32 greatestDistance = 0;
	for(int i = 0; i < globalRacing.Size(); i++)
	{
		Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(globalRacing[i]);
		if(pLogicActor)
		{
			if(pLogicActor->GetPositionInfo().mLogicPositionCombined > greatestDistance)
			{
				vehicleIndex = globalRacing[i];
				greatestDistance = pLogicActor->GetPositionInfo().mLogicPositionCombined;
			}
		}
	}

	if(VehicleIsLocal(vehicleIndex))
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(vehicleIndex, type);
		Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor(vehicleIndex);
		if(pLogicActor)
			pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);
	}
	else
	{
		//C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupAdded(UI::GetLevelState().mCars[vehicleIndex].mNetJoinId, type, true);
	}
}

void C_MpRaceAction_GiftPerk_Last::PerformAction()
{
	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
	RacerNet::C_NetPlayer* pHost = services.PlayerManager().HostPlayer(false);
	if (!(pHost && pHost->LocalPlayer()))
		return;

	NetVehicleIndexStack globalRacing;
	if (GetGlobalRacingList(globalRacing) == 0)
		return;

	//	Get/Choose pickup type
	E_PICKUP_TYPE type;
	if(m_PickupType != E_PICKUP_TYPE_NONE)
	{
		type = m_PickupType;
	}
	else
	{
		type = ChooseRandomPickup();
	}

	//	Find leading vehicle index
	U8 vehicleIndex = 0;
	F32 leastDistance = FLT_MAX;
	for(int i = 0; i < globalRacing.Size(); i++)
	{
		Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(globalRacing[i]);
		if(pLogicActor)
		{
			if(pLogicActor->GetPositionInfo().mLogicPositionCombined < leastDistance)
			{
				vehicleIndex = globalRacing[i];
				leastDistance = pLogicActor->GetPositionInfo().mLogicPositionCombined;
			}
		}
	}

	if(VehicleIsLocal(vehicleIndex))
	{
		Game::Actors::C_GrantPickupMsg grantPickupMsg(vehicleIndex, type);
		Game::Actors::C_VehicleLogicActor* pLogicActor = UI::GetVehicleLogicActor(vehicleIndex);
		if(pLogicActor)
			pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);
	}
	else
	{
		//C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_PickupAdded(UI::GetLevelState().mCars[vehicleIndex].mNetJoinId, type, true);
	}
}


} // namespace Game