#include "stdafx.h"

#include <Game/Game.h>

#include <ui/Game/UILua.h>
#include <ui/game/UIXt.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic/RaceModes/RaceModeBase.h>

#include <Game/Messaging/GameflowMessages.h>
#include <ui\Game\Gadgets\ButtonMessages.h>

#include <GameApp.h>

#include <Game/GameWorld/GameWorld.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic\Structures\LevelState.h>

#include <Network/CommunicatorManager.h>
#include <Network/Misc/NetDefines.h>
#include <Network/Core/NetRace.h>
#include <Network/Core/NetServices.h>
#include <Network/Player/NetPlayer.h>
#include <Network/Player/NetPlayerManager.h>
#include <Network/Router/NetRouterBase.h>
#include <Network/Message/NetMessage.h>
#include <Network/Misc/NetDatagramPacketTypes.h>
#include <Network/Core/NetLogger.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <RaceBook/RaceBookManager.h>

#include <Game/Messaging/PickupActorMessages.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>


extern Debug::Float g_debugNetworkLeftRaceGhostTime;
#include <GameLogic/SinglePlayer/SP_BossDemands.h>



namespace Game
{

C_LightPillarEffect C_RaceModeBase::mPillarEffects[ MAX_VEHICLES ];


C_RaceModeBase::C_RaceModeBase() : m_pRLC(0), m_pRaceLogicLayer(0), m_pLevelState(0)
{
}

void C_RaceModeBase::Initialise(Game::C_LevelState* pLevelState)
{
	SetInternalNameAndID("Nothing",E_RaceMode_Nothing);

	mLevelStarted = false;
	mRaceInProgress = false;

	SetLevelState(pLevelState);
}

void C_RaceModeBase::SetupRace()
{
}

bool C_RaceModeBase::SetupOpponents()
{
	return false;
}

void C_RaceModeBase::Update()
{
}

bool C_RaceModeBase::ShouldHandleMessages()
{
	Game::C_LevelState* pLevelState = GetLevelState();
	if(pLevelState->mRaceMode == GetID())
		return true;
	else
		return false;
}


void C_RaceModeBase::StartLevel()
{
	Game::Actors::RaceLogicCore *raceLogicCore = GetRLC();
	Game::C_LevelState* pLevelState = GetLevelState();

	if (!pLevelState->mInUI)
	{
		for( int i = 0; i < pLevelState->mPickupProbabilities.Size(); i++ )
		{
			if(pLevelState->mPickupsEnabled)
				raceLogicCore->BroadcastMessage(E_PickupMessage_EnableAllPickupTypes);
		}

		::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( Game::E_PickupMessage_UpdateFanDemandState );		// Update all the pickups

		for (U8 i=0; i<pLevelState->mNumCars; i++)
		{
			if (pLevelState->mCars[i].mState != E_VEHICLE_STATE_LEFT_RACE)
				pLevelState->mCars[i].mState = E_VEHICLE_STATE_WAITING_TO_START;
		}
	}

	mLevelStarted = true;
	mRaceFinished = false;
	mPrimaryVehicleFinished = false;

	for(int i = 0; i < MAX_VEHICLES; i++)
		mPillarEffects[i].reset();

	SP_BossDemands::Instance().RegisterBossDemandAction(E_BOSSDEMAND_ACTION_NEW_EVENT);
}

void C_RaceModeBase::StartRace()
{
	mRaceInProgress = true;

	Game::C_LevelState* pLevelState = GetLevelState();

	for (U8 i=0; i<pLevelState->mNumCars; i++)
	{
		if (pLevelState->mCars[i].mState == E_VEHICLE_STATE_WAITING_TO_START)
			pLevelState->mCars[i].mState = E_VEHICLE_STATE_RACING;
	}
	InitialiseSpotlights();
}

void C_RaceModeBase::Reset()
{
	mRaceInProgress =  mLevelStarted = false;
}

void C_RaceModeBase::UpdateHUD()
{

}

void C_RaceModeBase::SetupResults()
{
	mRaceInProgress = false;
}

void C_RaceModeBase::SetupFakeResults()
{
	mRaceInProgress = false;
}

void C_RaceModeBase::SetupQuitAndWinResults(lua_State *pState)
{
	mRaceInProgress = false;
}

void C_RaceModeBase::QuitLevel()
{
	Game::C_LevelState* pLevelState = GetLevelState();

	Game::C_RaceLogicLayer* pRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	if (pRaceLogicLayer)
	{
		for (U8 i=0; i < pLevelState->mNumCars; i++)
		{		
			Actors::C_VehicleLogicActor* pLogicActor = pRaceLogicLayer->GetLogicVehicleActor(i);
			if(pLogicActor)
			{
				pLogicActor->SendMessageToActor(pLogicActor->GetHandle(), Game::E_GameflowMessage_VehicleFinished);
			}
		}
	}

	for(int i=0;i<MAX_VEHICLES;i++ )
		mPillarEffects[i].reset();

	mRaceInProgress = false;
	mLevelStarted = false;
}

int C_RaceModeBase::SendInfoToLua(lua_State *pState)
{
	lua_pushstring( pState, "ID" );
	lua_pushnumber( pState, mID );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "Name" );
	lua_pushstring( pState, mName );
	lua_rawset( pState, -3 );

	return 1;
}

Game::Actors::RaceLogicCore	*C_RaceModeBase::GetRLC_AlwaysChecked() const
{
	BIZ_ASSERT_MSG(m_pRLC, "C_RaceModeBase::GetRLC() > The RaceLogicCore pointer's NULL, there be dragons!!!");
	return m_pRLC;
}

Game::Actors::RaceLogicCore	*C_RaceModeBase::GetRLC() const
{
	return m_pRLC;
}

void C_RaceModeBase::SetRLC(Game::Actors::RaceLogicCore	*pRLC) 
{
	m_pRLC = pRLC;
}

Game::C_RaceLogicLayer *C_RaceModeBase::GetRaceLogicLayer_AlwaysChecked() const
{
	BIZ_ASSERT_MSG(m_pRaceLogicLayer, "C_RaceModeBase::GetRaceLogicLayer() > The RaceLogicLayer pointer's NULL, there be dragons!!!");
	return m_pRaceLogicLayer;
}

Game::C_RaceLogicLayer *C_RaceModeBase::GetRaceLogicLayer() const
{
	return m_pRaceLogicLayer;
}

void C_RaceModeBase::SetRaceLogicLayer(Game::C_RaceLogicLayer *pRaceLogicLayer)
{
	m_pRaceLogicLayer = pRaceLogicLayer;
}

Game::C_LevelState* C_RaceModeBase::GetLevelState_AlwaysChecked() const
{
	BIZ_ASSERT_MSG(m_pLevelState, "C_RaceModeBase::GetLevelState() > The LevelState pointer's NULL, oh dear :(");
	return m_pLevelState;
}

Game::C_LevelState* C_RaceModeBase::GetLevelState() const
{
	return m_pLevelState;
}

void C_RaceModeBase::SetLevelState(Game::C_LevelState *pLevelState)
{
	m_pLevelState = pLevelState;
}


bool C_RaceModeBase::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	return false;
}

bool C_RaceModeBase::ReceiveReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U8 senderId, const bool partyChannel)
{
	/*Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	if (!netGameVars.m_InRace)
		return false;*/

	return false;
}

bool C_RaceModeBase::ReceiveUnReliableNetMessage(RacerNet::C_NetMessage& msg, const U8 msgType, const U32 numUnEncryptedBytes, const U8 senderId, const bool partyChannel)
{		
	/*Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	if (!netGameVars.m_InRace)
		return false;

	switch (msgType)
	{
	case RacerNet::eNetMessage_BasicVehicleData:
		HandleNetMessageBasicVehicleData(msg, senderId);
		break;
	}*/

	return false;
}

/*
bool C_RaceModeBase::HandleNetMessageBasicVehicleData(RacerNet::C_NetMessage& msg, const U8 senderId)
{
	Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
	netGameVars.m_ReceivedRemoteRacePackets = true;

	if (!netGameVars.m_PacketTransferEnabled)
		return false; // don't accept vehicle updates if we are not in a state to process them.

	bool ok = true;
	ok = msg.ReadOpen();
	if (ok)
	{
		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
		RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();

		U8 numCars;

		ok = ok && msg.ReadU8(numCars);

		Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
		Game::Actors::RaceLogicCore *raceLogicCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

		for (U8 i=0; i<numCars; i++)
		{
			U8 joinId, flags;
			U16 messageNo;
			RacingPhysics::Vehicle::CompressedNetworkControls controls;
			RacingPhysics::Vehicle::CompressedNetworkState state;

			ok = ok && msg.ReadU8(joinId);
			ok = ok && msg.ReadU16(messageNo);

			ok = ok && msg.ReadBuffer(&controls, sizeof(controls));
			ok = ok && msg.ReadBuffer(&state, sizeof(state));
			ok = ok && msg.ReadU8(flags);

			S8 index = GetLocalIndexFromJoinId(joinId);
			if (index != -1)
			{
				if (levelState.mOriginalNetworkHost)
				{
				}

				const Actor::C_ActorHandle vehicleHandle = raceLogicCore->GetActorHandleFromVehicleIndex(index);

				Game::Actors::C_VehicleUpdateNetworkControlsMsg vehicleControls;

				vehicleControls.m_CarControls = controls;
				vehicleControls.m_CarState = state;
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->SendMessageToActor(vehicleHandle, &vehicleControls);

				raceLogicCore->SetVehicleUsingPickup(index, (flags & 0x01) != 0);

				// Needs sending to VLC_Damage instead, or at least through VehicleLogicActor who holds the damage data.
				//U8 damageLevel = (flags >> 1) & 0x03;
				//if (damageLevel == 3)
				//{
				//	raceLogicCore->SetVehicleFatallyDamaged(index);

				//	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
				//	levelState.mCars[index].mState = E_VEHICLE_STATE_FATALLY_DAMAGED;
				//}
				//else
				//	raceLogicCore->SetVehicleDamageLevelZone(index, (E_HEALTH_LEVEL) damageLevel);
			}
		}

		if (ok)
			ExtractVoiceDataFromControlsUpdate(msg, senderId);

		ok = ok && msg.ReadClose();
	}

	return true;
}


S8 C_RaceModeBase::GetLocalIndexFromJoinId(U8 joinId)
{
	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

	for (U8 i=0; i<levelState.mNumCars; i++)
	{
		if (levelState.mCars[i].mNetJoinId == joinId)
			return (S8) i;
	}

	return -1;
}
*/

const UI::C_WideString512 C_RaceModeBase::GetScoreString()
{
	UI::C_WideString512 str;
	//str.Clear(); Dont need to clear it, the contructor calls it.
	
	return str;
}

const UI::C_WideString512 C_RaceModeBase::GetRulePassedString(U32 idx)
{
	UI::C_WideString512 str;
	//str.Clear(); Dont need to clear it, the contructor calls it.
	
	return str;
}

const UI::C_WideString512 C_RaceModeBase::GetRuleFailedString(U32 idx)
{
	UI::C_WideString512 str;
	//str.Clear(); Dont need to clear it, the contructor calls it.
	
	return str;
}

//
//// *** Network Functions
//// Build a list of the local player indices within the LevelState
//void C_RaceModeBase::CheckForEndOfRace(void)
//{
//	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
//	bool allFinished = true;
//	for (U8 i=0; i<levelState.mNumCars; i++)
//	{
//		if (levelState.mCars[i].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER || levelState.mCars[i].mControlType == E_VEHICLE_CONTROL_TYPE_NETWORK)
//			if (levelState.mCars[i].mState != E_VEHICLE_STATE_FINISHED && levelState.mCars[i].mState != E_VEHICLE_STATE_LEFT_RACE)
//				allFinished = false;
//	}
//
//	if (allFinished)
//	{
//		_UI_MESSAGE(E_GameflowMessage_RaceFinished);
//		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_RaceFinished );
//		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->BroadcastMessage( E_GameflowMessage_RaceFinished );
//
//		return;
//	}
//
//	if (mTimeOutTimerActive)
//		return;
//
//	mTimeOutTimerActive = true;
//	mTimeOutTimer = Time::GetPerformanceCounter();
//}
//
//
//
//
//
//
//bool C_RaceModeBase::SendControlsUpdate(void)
//{
//	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
//
//	NetVehicleIndexStack localToSend;
//	if (GetLocalPlayerList(localToSend) == 0)
//		return true;				// No data to send
//
//	// Build packet data
//
//	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
//	RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();
//
//	for (U8 i=0; i<levelState.mNumCars; i++)
//	{
//		if (levelState.mCars[i].mControlType == E_VEHICLE_CONTROL_TYPE_NETWORK)
//		{
//			RacerNet::C_NetPlayer *player = manager.FindFromJoinId(levelState.mCars[i].mNetJoinId, false);
//			if (player && player->PrimaryPlayer() && player->m_RaceData.m_InRaceVehicleIndex == i)
//			{
//				RacerNet::C_NetMessage& netMsg = services.Router().GetOutgoingUnReliable();
//				netMsg.CreateUnReliable(RacerNet::eNetMessage_BasicVehicleData, manager.LocalPrimaryJoinId(false), false);
//
//				BuildNetMessageBasicVehicleData(netMsg);
//				netMsg.WriteU16(levelState.mCars[i].mNetMessageNos[1]);
//				
//				const S32 remain = RacerNet::NET_MAX_DATAGRAM_SIZE - netMsg.DataSize();
//				const S32 voiceSize = player->VoiceSize() + sizeof(RacerNet::NetLocalVoiceSizes); 
//				if (remain > voiceSize)
//				{
//					AppendVoiceDataToControlsUpdate(netMsg, levelState.mCars[i].mNetJoinId);
//
//					netMsg.TerminateMessage();
//					if (player->m_RaceConnection.CanMessage())
//						player->SendUnReliable(netMsg, false, services.Router());
//				}
//				else
//				{
//					netMsg.TerminateMessage();
//					if (player->m_RaceConnection.CanMessage())
//						player->SendUnReliable(netMsg, false, services.Router());
//
//					manager.SendBasicVoiceMessageToPlayer(netMsg, services.Router(), levelState.mCars[i].mNetJoinId, false); 
//				}
//			}
//		}
//	}
//
//	//if (carState.mNetUpdateTimer > Game::NET_VEHICLE_UPDATE_TIME)
//
//	return true;
//}
//
//void C_RaceModeBase::AppendVoiceDataToControlsUpdate(RacerNet::C_NetMessage& msg, const U8 joinId)
//{
//	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
//	RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();
//	RacerNet::C_NetPlayer *p = manager.FindFromJoinId(joinId, false);
//	if (p)
//	{
//		bool ok = true;
//		RacerNet::NetLocalVoiceSizes localVoiceSize;
//		for (S32 i=0; i<localVoiceSize.Size(); i++)
//		{
//			const U32 size = p->VoiceSize(i);
//			assert(size <= 255); // only going to transmit this as U8 so can't go above 255 bytes per local player;
//			localVoiceSize[i] = static_cast<U8> (size);
//		}
//
//		// write number of voice bytes per local player.
//		for (S32 i=0; i<localVoiceSize.Size(); i++)
//			ok = ok && msg.WriteU8(localVoiceSize[i]);
//
//		// start unencrypted voice data. only voice data can be transmitted unencrypted.
//		ok = ok && msg.StartUnEncryptedData();
//
//		// copy buffered voice to message.
//		ok = ok && p->WriteOutgoingVoiceToMessage(msg);
//
//		if (!ok)
//		{
//			LOGWARN(RacerNet::g_Log, "C_RaceModeBase::AppendVoiceDataToControlsUpdate() - unable to pack voice data into race data.");
//		}
//	}
//	else
//	{
//		LOGERROR(RacerNet::g_Log, "C_RaceModeBase::AppendVoiceDataToControlsUpdate() - trying to send voice to unknown player.");
//	}
//}
//
//void C_RaceModeBase::ExtractVoiceDataFromControlsUpdate(RacerNet::C_NetMessage& msg, const U8 senderId)
//{
//	bool ok = true;
//
//	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
//	RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();
//	RacerNet::C_NetPlayer *p = manager.FindFromJoinId(senderId, false);
//	if (NULL == p)
//		return; // remote player no longer in our session.
//
//	const U64 remoteConsoleId = p->m_ConsoleId;
//
//	// read how much voice data we expect for each remote peer.
//	RacerNet::NetLocalVoiceSizes localVoiceSize;
//	for (S32 i=0; i<localVoiceSize.Size(); i++)
//		ok = ok && msg.ReadU8(localVoiceSize[i]);
//
//	if (ok)
//	{
//		U8 data[RacerNet::NET_VOICE_BUFFER_SIZE];
//		for (S32 i=0; i<localVoiceSize.Size() && ok; i++)
//		{
//			if (localVoiceSize[i] > 0)
//			{
//				// extract voice from msg
//				ok = ok && msg.ReadBuffer(data, localVoiceSize[i]);
//				if (ok)
//				{
//					RacerNet::C_NetPlayer* rt = manager.FindFromConsoleId(remoteConsoleId, i);
//					if (rt)
//						Communicator::C_Core::Instance().SubmitRemoteVoiceData(rt->m_User, data, localVoiceSize[i]);
//					else
//						ok = false;
//				}
//			}
//		}
//	}
//	else
//	{
//	}
//}
//
//bool C_RaceModeBase::BuildNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg)
//{
//	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
//
//	NetVehicleIndexStack localList;
//	GetLocalPlayerList(localList);
//
//	netMsg.WriteU8(localList.Size());
//
//	for (U8 i=0; i<localList.Size(); i++)
//	{
//		Game::Actors::RaceLogicCore *raceLogicCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
//		Game::Actors::C_VehicleActor *vehicle = (Game::Actors::C_VehicleActor *) raceLogicCore->GetVehicleActor(localList[i]);
//
//		RacingPhysics::Vehicle::CompressedNetworkControls controls;
//		RacingPhysics::Vehicle::CompressedNetworkState state;
//
//		vehicle->GetNetworkControlsAndState(&controls, &state);
//
//		netMsg.WriteU8(levelState.mCars[localList[i]].mNetJoinId);
//		netMsg.WriteU16(levelState.mCars[localList[i]].mNetMessageNos[0]);
//		netMsg.WriteBuffer(&controls, sizeof(controls));
//		netMsg.WriteBuffer(&state, sizeof(state));
//
//		U8 flags = 0;
//		flags |= raceLogicCore->IsVehicleUsingPickup(localList[i]) ? 1 : 0;
//
//		//
//		// Don't use this any more - use damage information in VehicleLogicActor.cpp
//		// 
//		//U8 damageLevel = raceLogicCore->GetVehicleDamageLevelZone(localList[i]);	// 00 - Light / 01 - Medium / 10 - Heavy
//		//if (raceLogicCore->IsVehicleFatallyDamaged(localList[i]))
//		//	damageLevel = 3;														// 11 - Fatal
//
//		//flags |= damageLevel << 1;
//
//		netMsg.WriteU8(flags);
//	}
//
//	return true;
//}
//
//
//void C_RaceModeBase::DisableVehiclesLostDuringLoad(void)
//{
//	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();
//	if (!services.IsInitialised())
//		return;
//
//	RacerNet::C_NetPlayerManager& manager = RacerNet::C_NetServices::Instance().PlayerManager();
//
//	NetVehicleIndexStack networkList;
//	GetNetworkPlayerList(networkList);
//
//	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
//	for (S32 i=0; i<networkList.Size(); i++)
//	{
//		const LevelStateCar& car = levelState.mCars[networkList[i]];
//
//		RacerNet::C_NetPlayer* p = manager.FindFromJoinId(car.mNetJoinId, false);
//		if (NULL == p || p->m_RaceData.m_InRaceVehicleIndex != networkList[i]) // player not found or new player joined on the same id but isn't in the race.
//			VehicleLeftRace((U8)networkList[i]);
//	}
//}
//
//
void C_RaceModeBase::VehicleLeftRace(const U8 vehicleIndex)
{
	Game::C_LevelState *pLevelState = GetLevelState();
	if (vehicleIndex < pLevelState->mNumCars)
	{
		pLevelState->mCars[vehicleIndex].mState = E_VEHICLE_STATE_LEFT_RACE;

		// Need to inform physics and renderer that car is no longer required.
		Game::Actors::C_VehicleLeftRaceMsg msg(vehicleIndex, g_debugNetworkLeftRaceGhostTime);

		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&msg);
	}
}

void C_RaceModeBase::InitialiseSpotlights()
{
	//Setup the spotlights' pointer to the game world
	C_LightPillarEffect::setGameWorld( C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld() );
}

void C_RaceModeBase::SetupSpotlight(const U8 spotlightIndex, const SpecialFX::C_SpecialFX* pEffect)
{
	BIZ_ASSERT_MSG(spotlightIndex < MAX_VEHICLES, "C_RaceModeBase::SetupSpotlight() > Invalid spotlight index");
	BIZ_ASSERT_MSG(pEffect, "C_RaceModeBase::SetupSpotlight() > Invalid special effect");

	C_LightPillarEffect& pillar = mPillarEffects[spotlightIndex];
	if(!pillar.active())
	{
		pillar.setEffect(pEffect);
		pillar.start();
	}
}


//
//void C_RaceModeBase::UpdateInRaceVoiceRecipients(void)
//{
//	const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
//	assert(levelState.mNetworkRace);
//
//	RacerNet::C_NetPlayerManager& plrManager = RacerNet::C_NetServices::Instance().PlayerManager();
//	RacerNet::C_NetRouterBase& router = RacerNet::C_NetServices::Instance().Router();
//
//	if (RacerNet::eVoiceControlStateGamePlay != plrManager.GetVoiceControlState())
//		return; // some other system is currently controlling the voice recipients.
//
//	Communicator::C_Core& voiceMgr = Communicator::C_Core::Instance();
//
//	/*	TODO -> For now we send voice to all players who are racing all the time. race modes may dictate we send voice to only our team or bandwidth
//		restrictions may mean we only send voice to a small number of the other racers. This needs to be decided.
//	*/
//
//	// build a list of remote consoles, voice data is only ever encoded for the primary remote player 
//	// even if their is multiple players on that remote console
//	RacerNet::NetConsoleIds remoteConsoles;
//	for (S32 i=0; i<plrManager.TotalSize(); i++)
//	{
//		RacerNet::C_NetPlayer* rp = plrManager.Index(i);
//		if (!rp->LocalPlayer())
//		{
//			// this will be rebuilt below.
//			rp->m_VoiceEncodeArray.ClearBits();
//
//			if (rp->PrimaryPlayer() && rp->IsInRace() && RacerNet::eNetPlayerDisconnectionTypeNone == rp->m_RaceConnection.m_DisconnectRequested)
//				remoteConsoles.PushBack(rp->m_ConsoleId);
//		}
//	}
//
//	// which local players should be encoding voice.
//	for (S32 localIndex=0; localIndex<plrManager.LocalSize(); localIndex++)
//	{
//		RacerNet::C_NetPlayer* lp = plrManager.Index(localIndex);
//		assert(lp->LocalPlayer());
//		if (voiceMgr.IsLocalTalkerRegistered(lp->m_ControllerPort))
//		{
//			// local player has voice capability, which remote players should we send voice to ?
//			for (S32 remote=0; remote<remoteConsoles.Size(); remote++)
//			{
//				// check all players on this remote console, no point sending voice if they all have this local player muted.  
//				bool anyRemoteValid = false;
//				for (S32 z=0; z<plrManager.TotalSize(); z++)
//				{
//					RacerNet::C_NetPlayer* rp = plrManager.Index(z);
//					if (remoteConsoles[remote] == rp->m_ConsoleId)
//					{
//						if (voiceMgr.IsRemoteTalkerRegistered(rp->m_User))
//						{
//							if (!voiceMgr.LocalMuted(lp->m_ControllerPort, rp->m_User) && !voiceMgr.RemoteMuted(lp->m_ControllerPort, rp->m_User))
//								anyRemoteValid = true; // this local player should send voice to this console.
//						}
//					}
//				}
//
//				if (anyRemoteValid)
//				{
//					RacerNet::C_NetPlayer* rp = plrManager.FindFromConsoleId(remoteConsoles[remote], 0);
//					assert(rp);
//					assert(!rp->LocalPlayer());
//					rp->m_VoiceEncodeArray.SetBit(localIndex);
//				}
//			}
//		}
//	}
//}

// ***

} // namespace Game
