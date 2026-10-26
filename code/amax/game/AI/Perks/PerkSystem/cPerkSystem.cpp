#include "stdafx.h"
#include "AI/AI_Amax.h"
#include "AI/AI_Messages.h"

#include <GameLogic/Messaging/GameLogicMessages.h>
#include <Game/Game.h>
#include <GameApp.h>
#include <game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <Perks/PerkSystem/cPerkSystem.h>
#include <system/isystemai.h>
#include <gameservertime/igameservertime.h>
#include <racedirector/iracedirector.h>
#include <GameLogic/Pickups/PickupBase.h>
#include <game/GameWorld/GameWorld.h>
#include <GameLogic/ActorStudioActors/ActorStudioActors.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <Game/Messaging/PickupActorMessages.h>
#include <data/idata.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>




#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif


Debug::Int g_DebugBatchLimit("AmaxAI.Perks.General.BatchLimit", 5); 


namespace airacingtech
{
	using namespace Game;

	CPerkSystem::CPerkSystem()
	{
		m_aiDontUsePerks = false;
		m_EvalauteAI = -1;
		m_pISystem = 0;
		m_lastTime = 0.0f;
		m_CheckPlayerLastTime = 0.0f;
		m_ShockOnTrack = false;
		m_ShuntOnTrack = false;
		m_pIRaceDirector = 0;
		m_pTrackData = 0;
		m_CheckPlayerLastTimeShunt = -1.0f;
		m_CheckPlayerLastTimeShock = -1.0f;
		m_IncrementShockTime = 3.0f;

		m_upperBatchLimit = 0;
		m_lowerBatchLimit = 0;
		m_FireMessages.Reset(); 
	}

	CPerkSystem::~CPerkSystem()
	{
		m_Perks.Destroy();// tobe on the safe side	
	}

	void CPerkSystem::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PERK_SYSTEM, CID_PERK_SYSTEM);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PERK_SYSTEM, CPerkSystem::CreateMe, CPerkSystem::DestroyMe, CHash("PerkSystemAI"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_SYSTEM, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_SYSTEM, MT_PRE_UPDATE_AI_SYSTEMS);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_SYSTEM, MT_POST_UPDATE_AI_SYSTEMS);
		
	}

	IComponent *CPerkSystem::CreateMe()
	{
		return new CPerkSystem;
	}

	bool CPerkSystem::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPerkSystem::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CPerkSystem::Deinit(void)
	{
		m_Perks.Destroy();// tobe on the safe side
		m_FireMessages.Reset(); 
	}

	EMessageResult CPerkSystem::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch (msg.mType)
		{

			case MT_POST_INIT_AI_SYSTEM:
			{
				msgRet =  MR_TRUE;
				CachedInterfaces();
				
			}
			break;

			case MT_PRE_UPDATE_AI_SYSTEMS:
			{
				PreUpdate();
			}
			break;

			case MT_POST_UPDATE_AI_SYSTEMS:
				{
					PostUpdate();
				}
				break;

			default:
				break;
		}

		return msgRet;
	}

	EComponentTypeId CPerkSystem::GetComponentTypeId(void)
	{
		return CID_PERK_SYSTEM;
	}

	void CPerkSystem::CachedInterfaces()
	{
		CObjectId systemID("AmaxSystemAI");
		m_pISystem = static_cast<ISystemAI *>(GlobalsComponent::GetObjectManager().QueryInterface(systemID, IID_SYSTEM_AI));
	
		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

		m_pIRaceDirector = static_cast<IRaceDirector *>(GlobalsComponent::GetObjectManager().QueryInterface(CObjectId("RaceDirectorSystem"), IID_RACEDIRECTOR));


		CObjectId trackdataSystem("TrackDataSystem");
		m_pTrackData = static_cast<ITrackDataAI *>(GlobalsComponent::GetObjectManager().QueryInterface(trackdataSystem, IID_TRACK_DATA_AI));


		m_ShockOnTrack = IsPerkOnTrack(E_PICKUP_TYPE_SHOCK);
		m_ShuntOnTrack = IsPerkOnTrack(E_PICKUP_TYPE_SHUNT);


	}

	
	void CPerkSystem::PreUpdate()
	{
		// update one ai only each frame for perk evaluation.
		if (m_pIGameServerTime && m_pISystem && !GetAIPerkUsage())
		{
			m_EvalauteAI++;
			if (m_EvalauteAI >= m_pISystem->GetNumOfAI() || m_EvalauteAI < 0)
			{
				m_EvalauteAI = 0;
			}

			m_upperBatchLimit += g_DebugBatchLimit;
			m_lowerBatchLimit = m_upperBatchLimit - g_DebugBatchLimit;

			//CheckIfPLayerTooFarInfront();
		}
	}

	void CPerkSystem::Update()
	{

	}

	void CPerkSystem::PostUpdate()
	{
		if (m_pIGameServerTime && m_pISystem && !GetAIPerkUsage())
		{
			if (m_upperBatchLimit >= m_pISystem->GetNumOfAI())
			{
				m_upperBatchLimit = 0;
				m_lowerBatchLimit = 0;
			}

		}

	}


	bool CPerkSystem::RequestToUsePerk(int perk, int indexOfAI)
	{
		// TODO : Perk Management here for the whole AI....

		return true;
	}
	
	void CPerkSystem::CheckIfPLayerTooFarInfront()
	{
		if (m_pISystem && m_pIGameServerTime && m_pIRaceDirector && AreCarsGreaterThan(5))
		{
			CheckPlayerWith(E_PICKUP_TYPE_SHUNT);
			CheckPlayerWith(E_PICKUP_TYPE_SHOCK);
		}
	}

	void CPerkSystem::CheckPlayerWith(E_PICKUP_TYPE firePerkType)
	{

		// get the player car
		IData *pFirstPlace = m_pIRaceDirector->GetAIAtPosition(1);
		IData *pSecondPlace = m_pIRaceDirector->GetAIAtPosition(2);
		BIZ_ASSERT(pFirstPlace && pSecondPlace);


		if (pFirstPlace->IsPlayer())
		{
			bool canFire = false;
			if (pSecondPlace->GetVehicleLogic() && pSecondPlace->GetVehicleLogic()->GetLogicState()->GetRacePercentage() < 1.0f)
			{
				canFire = true;
			}
			else
			{
				canFire = false;
			}



			float dist = FLT_MAX;
			bool tooFarInfront = IsPlayerTooFarInfront(dist);

			switch(firePerkType)
			{
			case E_PICKUP_TYPE_SHUNT:
				{
					if (tooFarInfront && m_pIGameServerTime->GetTime() > m_CheckPlayerLastTimeShunt && m_ShuntOnTrack && canFire)
					{
						if (m_CheckPlayerLastTimeShunt == -1.0f)
						{
							m_IncrementShuntTime = RandomInRange(15.0f, 25.0f);
							m_CheckPlayerLastTimeShunt = m_pIGameServerTime->GetTime() + 5.0f + RandomInRange(15.0f, 25.0f) + m_IncrementShuntTime;
							m_CheckPlayerLastTimeShock += RandomInRange(25.0f, 35.0f);
						}
						else
						{
							float checkDist = 75.0f;
							if (pFirstPlace->IsPlayer())
							{
								checkDist = 75.0f;
							}
							else
							{
								checkDist = 200.0f;
							}

							if (dist > checkDist && dist != FLT_MAX && !pSecondPlace->IsPlayer())
							{
								m_IncrementShuntTime = RandomInRange(15.0f, 25.0f);
								m_CheckPlayerLastTimeShunt = m_pIGameServerTime->GetTime() + 5.0f + RandomInRange(15.0f, 25.0f) + m_IncrementShuntTime;
								GrantAndFirePerk(pSecondPlace->GetState().m_index, firePerkType, pFirstPlace->GetState().m_index);

								// increment the shock time if a shunt has gone offf
								m_CheckPlayerLastTimeShock += RandomInRange(10.0f, 15.0f);
							}
						}
					}
				}
				break;

			case E_PICKUP_TYPE_SHOCK:
				{
					if (tooFarInfront && m_pIGameServerTime->GetTime() > m_CheckPlayerLastTimeShock && m_ShockOnTrack && canFire)
					{
						if (m_CheckPlayerLastTimeShock == -1.0f)
						{
							m_IncrementShockTime = RandomInRange(5.0f, 10.0f);
							m_CheckPlayerLastTimeShock = m_pIGameServerTime->GetTime() + 10.0f + RandomInRange(20.0f, 30.0f) + m_IncrementShockTime;
							m_CheckPlayerLastTimeShunt += RandomInRange(25.0f, 35.0f);
						}
						else
						{

							float checkDist = 75.0f;
							if (pFirstPlace->IsPlayer())
							{
								checkDist = 75.0f;
							}
							else
							{
								checkDist = 200.0f;
							}

							if (dist > checkDist && dist != FLT_MAX && !pSecondPlace->IsPlayer())
							{
								m_IncrementShockTime = RandomInRange(5.0f, 10.0f);
								m_CheckPlayerLastTimeShock = m_pIGameServerTime->GetTime() + 10.0f + RandomInRange(20.0f, 30.0f) + m_IncrementShockTime;
								GrantAndFirePerk(pSecondPlace->GetState().m_index, firePerkType, pFirstPlace->GetState().m_index);

								// increment the shunt when a shock as gone off
								m_CheckPlayerLastTimeShunt += RandomInRange(5.0f, 15.0f);
							}
						}
					}
				}
				break;

			default:
				break;

			}
		}
	}


	bool CPerkSystem::IsPlayerTooFarInfront(float &dist)
	{
		bool result = false;

		dist = FLT_MAX;

		if (AreCarsGreaterThan(2) && m_pTrackData)
		{

			AITrackData *pAITrackData = m_pTrackData->GetTrackData();

			// get the player car ans the 
			IData *pFirstPlace = m_pIRaceDirector->GetAIAtPosition(1);
			IData *pSecondPlace = m_pIRaceDirector->GetAIAtPosition(2);
			BIZ_ASSERT(pFirstPlace && pSecondPlace);

			IVehicleLogic *pFirstVehicleLogic = pFirstPlace->GetVehicleLogic();
			IVehicleLogic *pSecondVehicleLogic = pSecondPlace->GetVehicleLogic();

			float firstPos = pFirstVehicleLogic->GetLogicState()->GetLogicPositionCombined();       
			float secondPos = pSecondVehicleLogic->GetLogicState()->GetLogicPositionCombined();
			
			float diff = (firstPos - secondPos) * pAITrackData->m_TrackLogicData.m_noSamples * pAITrackData->m_TrackLogicData.m_stepSize;

			dist = diff;	

			if (diff > 30.0f)
			{
				result = true;
			}
		}

		return result;
	}

	void CPerkSystem::GrantAndFirePerk(int index, E_PICKUP_TYPE firePerkType, int targetIndex)
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(index);

		Game::Actors::C_GrantPickupMsg grantPickupMsg(index, firePerkType);
		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);

		// Shuffle
		VehiclePickupInfo& pickupInfo = pLogicActor->GetPickupInfo();

		for(int i = 1; i < pickupInfo.mPickupStore.Size(); i++)
		{
			if(pickupInfo.mPickupStore[i].mType == firePerkType)
			{
				Game::Actors::PickupShuffleMsg shuffleMsg(index, true, i);
				pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &shuffleMsg);
				break;
			}
		}

		pickupInfo.mCurrentTargetIdx = targetIndex;

		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(index, firePerkType);

#if 1
		AddFireMessage(pLogicActor, &pickupFireMsg);
#else
		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &pickupFireMsg);
#endif

	}
	
	bool CPerkSystem::IsPerkOnTrack(int perkType)
	{
		bool result = false;

		// check for a shock...
		for(int i=0; i<m_Perks.Size(); i++)
		{
			const Actor::C_Actor *pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(m_Perks[i].m_actorHandle);
			BIZ_ASSERT(pActor);
			::Actors::PickupPositionActor *pPerkActor = (::Actors::PickupPositionActor*)Actor::ActorCast< ::Actors::PickupPositionActor>(pActor);
			BIZ_ASSERT(pPerkActor);

			if (pPerkActor->GetEnumPickupType() == perkType)
			{
				result = true;	
				break;
			}
		}

		return result;
	}

	bool CPerkSystem::AreCarsGreaterThan(int numCars)
	{
		bool result = false;

		if (m_pISystem && m_pISystem->GetNumOfAI() > numCars)
		{
			result = true;
		}

		return result;
	}


	// Darren : We might want to put a multi-threaded lock around this to be on the safe side...
	void CPerkSystem::FireMessages()
	{
		for(int i=0; i<m_FireMessages.m_FireMessagesToFire.Size(); i++)
		{
			Game::Actors::C_VehicleLogicActor *pLogicActor = m_FireMessages.m_FireMessagesToFire[i].m_pLogicActor;
			Game::Actors::C_VehicleFirePickupMsg MSG = m_FireMessages.m_FireMessagesToFire[i].m_message;

			if (pLogicActor)
			{
				if (m_FireMessages.m_FireMessagesToFire[i].m_drop == false)
				{
					pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &MSG);
				}
				else
				{
					C_VehicleLogicDropPickupOnTrackMsg Msg(MSG.mVehicleIndex);
					pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &Msg);
				}
			}
		}

		m_FireMessages.Reset(); 


		if (m_pISystem)
		{
			int numAI = m_pISystem->GetNumOfAI();
			for(int i=0; i<numAI; i++)
			{
				IData *pIData = m_pISystem->GetData(i);
				if(pIData)
				{
					IPerkSkillUsage *pIPerkSkillUsage = static_cast<IPerkSkillUsage *>(GlobalsComponent::GetObjectManager().QueryInterface(pIData->GetObjectId(), IID_PERKS_SKILL_USAGE));
					if (pIPerkSkillUsage)
					{
						pIPerkSkillUsage->FireMessages();
					}
				}
				
			}
		}

	}

}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif
