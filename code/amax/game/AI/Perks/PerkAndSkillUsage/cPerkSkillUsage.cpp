#include "stdafx.h"
#include <Perks/PerkLogic/cPerkLogic.h>
#include <perks/PerkSystem/cPerkSystem.h>
#include <AI/AI_Amax.h>
#include <AI/AI_AmaxSetup.h>
#include <AI/AI_Messages.h>
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <GameLogic/Pickups/PickupBase.h>
#include <Game/Game.h>
#include <GameApp.h>
#include <game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/ActorStudioActors/ActorStudioActors.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <Game/Messaging/PickupActorMessages.h>
#include <Perks/PerkNitro/cPerkNitro.h>
#include <Perks/PerkBarge/cPerkBarge.h>
#include <Perks/PerkShock/cPerkShock.h>
#include <Perks/PerkShunt/cPerkShunt.h>
#include <Perks/PerkBolt/cPerkBolt.h>
#include <Perks/PerkShield/cPerkShield.h>
#include <Perks/PerkMine/cPerkMine.h>
#include <Perks/PerkRepair/cPerkRepair.h>
#include <data/idata.h>
#include <avoidance/iavoidance.h>
#include <gameservertime/igameservertime.h>
#include <system/isystemai.h>
#include <perks/PerkSystem/iPerkSystem.h>
#include <rubberbanding/irubberbandingai.h>
#include <AI/AI_AmaxXt.h>
#include <AI/AI_AmaxData.h>
#include <algorithm>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif



Debug::Float g_DebugNitroCoolDown("AmaxAI.Perks.General.NitroCoolDown", 6.0f); 
Debug::Float g_debugProjectileCacheDistance("AmaxAI.Perks.General.ProjectileCacheDistance", 80.f); // was 60.0f
Debug::Bool g_debugAiPerkOverrideTestBarge("AmaxAI.Perks.General.OverrideTestBarge", false);
Debug::Bool g_debugAiPerkOverrideTestNitro("AmaxAI.Perks.General.OverrideTestNitro", false);
Debug::Bool g_debugAiPerkOverrideTestBolt("AmaxAI.Perks.General.OverrideTestBolt", false);
Debug::Bool g_debugAiPerkOverrideTestShock("AmaxAI.Perks.General.OverrideTestShock", false);
Debug::Bool g_debugAiPerkOverrideTestShunt("AmaxAI.Perks.General.OverrideTestShunt", false);
Debug::Bool g_debugAiPerkOverrideTestMine("AmaxAI.Perks.General.OverrideTestMine", false);
 

Debug::Bool g_debugLogProcessing("AmaxAI.Perks.General.Processing", false);
Debug::Bool g_debugLogPerkInfo("AmaxAI.Perks.General.Perkinfo", false);

Debug::Float g_debugAiBlindFireSkill("AmaxAI.Perks.Bolt.BoltSkill", 1.0f); 
Debug::Float g_debugAiMinBlindFireDelay("AmaxAI.Perks.Bolt.BoltMinDelay", 0.1f);


static Debug::Logger &g_LoggerAIPerks = Debug::GetLogger("amax.AI.Perks");

// TODO : Check groups...
// TODO : Add in the skill...
// TODO : Difficulty...
// TODO : Try a simple priority algorithm, instead of evaluation.

namespace airacingtech
{
	using namespace Game;

	CPerkSkillUsage::CPerkSkillUsage() : m_pIData(0), m_pIGameServerTime(0), m_pIAvoidance(0), m_pIPerkNitro(0), m_pIPerkShunt(0),
										m_pIPerkShock(0), m_pIPerkBarge(0), m_pIPerkBolt(0), m_pIPerkShield(0), m_pIPerkMine(0), 
										m_pIPerkRepair(0), m_EvalauteAI(0), m_lastTime(0.0f), m_pIPerkSystem(0), m_Update(false),
										m_pPerkPrefs(0), m_PerkPrefsTemplateIndex(-1), m_SkillzTemplateIndex(-1), m_pAISkillz(0)
	{

		Reset();
		m_FireMessages.Reset(); 


		for(int i=0; i<8; i++)
		{
			m_IhavePerks[i] = 0;
			m_EvaluatePerks[i] = -999;

			// we could ignore the repair here....
			PerkPriorities perkPri(i,  1.0f / 8.0f);
			m_PerkPriorities.push_back(perkPri);
			
		}

		m_BoltTimeStamp = -1.0f;
		m_BoltDelay = 1.0f;
		m_fireTimeStamp = -1;
		m_fireType = -1;

		m_BargeTimeStamp = -1.0f;
		m_BargeDelay = 1.0;

		m_GeneralFireTimeStamp = -1.0f;
		m_GeneralDelay = 0.5f;
		
		m_NitroCoolDownTime = -1.0f;
		m_MineTimeStamp = -1.0f;
		m_BoltCount = 0;
	
		m_StateChangeDelay = 3.0f;
		m_StateChangeTime = -1.0f;
		
		

	}

	CPerkSkillUsage::~CPerkSkillUsage()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_pIPerkNitro = 0;
		m_pIPerkShunt = 0;
		m_pIPerkShock = 0;
		m_pIPerkBarge = 0;
		m_pIPerkBolt = 0;
		m_pIPerkShield = 0;
		m_pIPerkMine = 0;
		m_pIPerkRepair = 0;
		m_pIPerkSystem = 0;
		Reset();
		m_FireMessages.Reset(); 
	}

	void CPerkSkillUsage::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PERKS_SKILL_USAGE, CID_PERKS_SKILL_USAGE);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PERKS_SKILL_USAGE, CPerkSkillUsage::CreateMe, CPerkSkillUsage::DestroyMe, CHash("PerkSkillUsageAI"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERKS_SKILL_USAGE, MT_POST_INIT_AI_SYSTEM);	
		//GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERKS_SKILL_USAGE, MT_PRE_UPDATE_GAME_COMPONENTS);
		//GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERKS_SKILL_USAGE, MT_UPDATE_GAME_COMPONENTS);
		//GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERKS_SKILL_USAGE, MT_POST_UPDATE_GAME_COMPONENTS);
	}

	IComponent *CPerkSkillUsage::CreateMe()
	{
		return new CPerkSkillUsage;
	}

	bool CPerkSkillUsage::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPerkSkillUsage::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CPerkSkillUsage::Deinit(void)
	{

	}

	EMessageResult CPerkSkillUsage::HandleMessage(const CMessage &msg)
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


			case MT_PRE_UPDATE_GAME_COMPONENTS:
				{
					PreUpdate();
				}
				break;


			case MT_UPDATE_GAME_COMPONENTS:
				{
					Update();
				}
				break;


			case MT_POST_UPDATE_GAME_COMPONENTS:
				{
					PostUpdate();
				}
				break;

			default:
				break;
		}

		return msgRet;
	}

	EComponentTypeId CPerkSkillUsage::GetComponentTypeId(void)
	{
		return CID_PERKS_SKILL_USAGE;
	}

	void CPerkSkillUsage::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));
	
		m_pIAvoidance = static_cast<IAvoidance *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_AVOIDANCE));
		
		// lets cache the perk pointers.....
		m_pIPerkNitro = static_cast<IPerkNitro *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERK_NITRO));
		m_pIPerkShunt = static_cast<IPerkShunt *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERK_SHUNT));
		m_pIPerkShock = static_cast<IPerkShock *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERK_SHOCK));
		m_pIPerkBarge = static_cast<IPerkBarge *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERK_BARGE));
		m_pIPerkBolt = static_cast<IPerkBolt *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERK_BOLT));
		m_pIPerkShield = static_cast<IPerkShield *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERK_SHIELD));
		m_pIPerkMine = static_cast<IPerkMine *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERK_MINE));
		m_pIPerkRepair = static_cast<IPerkRepair *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERK_REPAIR));


		CObjectId systemID("AmaxSystemAI");
		m_pISystem = static_cast<ISystemAI *>(GlobalsComponent::GetObjectManager().QueryInterface(systemID, IID_SYSTEM_AI));


		CObjectId perkSystemID("PerkSystemAISystem");
		m_pIPerkSystem = static_cast<IPerkSystem *>(GlobalsComponent::GetObjectManager().QueryInterface(perkSystemID, IID_PERK_SYSTEM));
	}

	
#define _BATCHING_
//#define _PLAYER_TESTING_

	bool CPerkSkillUsage::ShouldEvaluate()
	{
		bool eval = false;
		int indexCar = m_pIData->GetState().m_index;
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(indexCar);

#ifdef _BATCHING_

#ifdef _PLAYER_TESTING_
		if (m_pIPerkSystem && !m_pIPerkSystem->GetAIPerkUsage() && m_pIData && m_pIData->GetVehicleLogic() && pLogicActor && !pLogicActor->IsGhosted() && !pLogicActor->IsWreckedOrEliminated())
#else
		if (m_pIPerkSystem && !m_pIPerkSystem->GetAIPerkUsage() && m_pIData && !m_pIData->IsPlayer() && 
			m_pIData->GetVehicleLogic() && m_pIData->GetVehicleLogic()->GetLogicState()->GetRacePercentage() < 1.0f && 
			pLogicActor && !pLogicActor->IsGhosted() && !pLogicActor->IsWreckedOrEliminated())
#endif
		{
			int upperLimit = m_pIPerkSystem->GetUpperBatchLimit();
			int lowerLimit = m_pIPerkSystem->GetLowerBatchLimit();

			if (m_pIData->GetState().m_index >= lowerLimit && m_pIData->GetState().m_index < upperLimit)
			{
				eval = true;

			
			
				if (g_debugLogProcessing)
					LOGDEBUG(g_LoggerAIPerks, "Processing Vehicle index %d between range %d and %d ", m_pIData->GetState().m_index, lowerLimit, upperLimit);
			}
				
		}
#else
		if (m_pIPerkSystem && !m_pIPerkSystem->GetAIPerkUsage() && m_pIData && m_pIPerkSystem->GetEvaluateAiIndex() == m_pIData->GetState().m_index &&
			!m_pIData->IsPlayer() && m_pIData->GetVehicleLogic() && m_pIData->GetVehicleLogic()->GetLogicState()->GetRacePercentage() < 1.0f)
		{
			eval = true;
		}
#endif

		return eval;
	}
	

	void CPerkSkillUsage::PreUpdate()
	{	
		

		
	}

	void CPerkSkillUsage::NitroCoolDown()
	{
		int indexCar = m_pIData->GetState().m_index;
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(indexCar);
		VehiclePickupInfo& pickupInfo = pLogicActor->GetPickupInfo();

		if(pickupInfo.mCurrentlyUsingType == E_PICKUP_TYPE_NITROUS)
		{
			m_pIData->GetState().m_usingNitro = true;
			m_NitroCoolDownTime = m_pIGameServerTime->GetTime() + g_DebugNitroCoolDown;
			m_pIData->GetState().m_usingCoolDownNitroExtended = false;
		}
		else
		{
			m_pIData->GetState().m_usingNitro = false;
			m_pIData->GetState().m_usingCoolDownNitroExtended = false;
		}

		if (m_pIGameServerTime->GetTime() < m_NitroCoolDownTime)
		{
			m_pIData->GetState().m_usingNitro = true;

			float diff = m_NitroCoolDownTime - m_pIGameServerTime->GetTime();
			const float halfCoolDown = g_DebugNitroCoolDown * 0.5f;
			if (diff < halfCoolDown)
			{
				m_pIData->GetState().m_usingCoolDownNitroExtended = true;
				m_pIData->GetState().m_usingNitro = false;
			}
		}
	}

	void CPerkSkillUsage::LogInfoPerks()
	{
		int indexCar = m_pIData->GetState().m_index;
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(indexCar);
		VehiclePickupInfo& pickupInfo = pLogicActor->GetPickupInfo();
		int sizeOfStore = pickupInfo.mPickupStore.Size();
				
		LOGDEBUG(g_LoggerAIPerks, "Vehicle index %d has %d pickups which are : ", indexCar, sizeOfStore);

		for(int i=0; i<sizeOfStore; i++)
		{
			switch(pickupInfo.mPickupStore[i].mType)
			{

			case E_PICKUP_TYPE_NITROUS:	
				LOGDEBUG(g_LoggerAIPerks, "Nitro : ");
				break;

			case E_PICKUP_TYPE_SHOCK:						
				LOGDEBUG(g_LoggerAIPerks, "Shock : ");						
				break;

			case E_PICKUP_TYPE_SHUNT:
				LOGDEBUG(g_LoggerAIPerks, "Shunt : ");
				break;

			case E_PICKUP_TYPE_BLINDFIRE:							
				LOGDEBUG(g_LoggerAIPerks, "BlindFire : ");		
				break;

			case E_PICKUP_TYPE_BARGE:	
				LOGDEBUG(g_LoggerAIPerks, "Barge : ");
				break;

			case E_PICKUP_TYPE_BOMB:
				LOGDEBUG(g_LoggerAIPerks, "Mine : ");
				break;

			case E_PICKUP_TYPE_REPAIR:	
				LOGDEBUG(g_LoggerAIPerks, "Repair : ");
				break;

			case E_PICKUP_TYPE_SHIELD:						
				LOGDEBUG(g_LoggerAIPerks, "Shield : ");
				break;
			default:
				break;
			}
		}

		LOGDEBUG(g_LoggerAIPerks, " ");
		LOGDEBUG(g_LoggerAIPerks, " ");
		LOGDEBUG(g_LoggerAIPerks, " ");

	}


	void CPerkSkillUsage::Update()
	{
		m_Update = ShouldEvaluate();


		/*
		E_PICKUP_TYPE_NITROUS,
		E_PICKUP_TYPE_SHOCK,
		E_PICKUP_TYPE_SHUNT,
		E_PICKUP_TYPE_BLINDFIRE,
		E_PICKUP_TYPE_BARGE,
		E_PICKUP_TYPE_BOMB


		//secondary pick-ups
		E_PICKUP_TYPE_REPAIR,
		E_PICKUP_TYPE_SHIELD,

		*/

		if (m_Update)
		{
			if (g_debugLogPerkInfo)
				LogInfoPerks();
				
			m_FireMessages.Reset();

			m_ActivePickCountDebug = 0;
			SortPerksInPriorityOrder();
			CacheProjectiles();
			EvaluatePerksIHave();
			EvaluateSituation();

			// SB - KNOCK THESE OUT BEFORE SUBMITTING
			if (g_debugAiPerkOverrideTestBolt)
				TestBlindFire();

			if (g_debugAiPerkOverrideTestShunt)
				TestShunt();

			if (g_debugAiPerkOverrideTestBarge)
				TestBarge();

			if (g_debugAiPerkOverrideTestMine)
				TestMine();

			if (g_debugAiPerkOverrideTestShock)
				TestShock();

			//TestRepair();
			//TestShield();

			if (g_debugAiPerkOverrideTestNitro)
				TestNitro();

			if (!g_debugAiPerkOverrideTestNitro && !g_debugAiPerkOverrideTestShock && !g_debugAiPerkOverrideTestMine &&
				!g_debugAiPerkOverrideTestBarge && !g_debugAiPerkOverrideTestShunt && !g_debugAiPerkOverrideTestBolt)
			{
				SelectPerkBasedUponEvaluations();

				KeepOneSlotFreeRule();
			}

			NitroCoolDown();


			if (g_debugLogPerkInfo)
				LogInfoPerks();
		}	
		else
		{
			// clear the projectiles out
			m_Projectiles.Clear();
			m_FireMessages.Reset();

		}

	}

	void CPerkSkillUsage::PostUpdate()
	{
		if (m_Update)
		{

		}
	}

	bool CPerkSkillUsage::KeepOneSlotFreeRule()
	{
		bool v = false;
		EvaluatePerksIHave();

			
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(m_pIData->GetState().m_index);
		VehiclePickupInfo& pickupInfo = pLogicActor->GetPickupInfo();

		int sizeOfStore = pickupInfo.mPickupStore.Size();
		
		// we have a full stash, lets fire least favourite perk off.....don`t car what it is.
		if (sizeOfStore >= 3)
		{
			SelectLeastFavouritePerkBasedUponEvaluations();

			v = true;

		}

		return v;
	}


	void CPerkSkillUsage::SelectLeastFavouritePerkBasedUponEvaluations()
	{
		int firePerkType = -1;
		float eval = FLT_MAX;
		bool backwards = false;

		for(int i=0; i<static_cast<int>(m_PerkPriorities.size()); i++)
		{
			int perkType = m_PerkPriorities[i].m_Type;
			if (m_IhavePerks[perkType] && perkType != E_PICKUP_TYPE_SHOCK)
			{
				if (m_PerkPriorities[i].m_Eval >= 0.0f && m_PerkPriorities[i].m_Eval <= 1.0f)
				{
					eval = m_PerkPriorities[i].m_Eval;
					backwards = m_PerkPriorities[i].m_FireBackwards;

					if (m_pIData->GetVehicleLogic())
					{
						int rank = m_pIData->GetVehicleLogic()->GetLogicState()->GetRank();
						if (rank < 3)
						{
							backwards = true;
						}
					}
				}
				else
				{
					eval = 0.5f;
					backwards = false; 

					if (m_pIData->GetVehicleLogic())
					{
						int rank = m_pIData->GetVehicleLogic()->GetLogicState()->GetRank();
						if (rank < 3)
						{
							backwards = true;
						}
					}
				}

				firePerkType = m_PerkPriorities[i].m_Type;
			}

			
		}

		if (firePerkType != -1)
		{
			switch(firePerkType)
			{
			case E_PICKUP_TYPE_NITROUS:	
				FireNitro();
				break;

			case E_PICKUP_TYPE_SHOCK:	
				{
					if (m_pISystem)
					{				
						if (m_pIData->GetVehicleLogic())
						{
							int rank = m_pIData->GetVehicleLogic()->GetLogicState()->GetRank();
							if (rank == 1)
							{
								// Darren turn back to normal as it un-balances the game : Chanjit and PeteH
								//FireShock(true);
								FireShock();
							}
							else
							{
								FireShock();
							}
						}
						else
						{
							FireShock();
						}
					}
					else
					{
						FireShock();
					}					
				}			
				break;

			case E_PICKUP_TYPE_SHUNT:
				FireShunt(backwards);
				break;

			case E_PICKUP_TYPE_BLINDFIRE:							
				FireBlindFire(backwards);			
				break;

			case E_PICKUP_TYPE_BARGE:	
				FireBarge();
				break;

			case E_PICKUP_TYPE_BOMB:
				FireMine(backwards);
				break;

			case E_PICKUP_TYPE_REPAIR:	
				FireRepair();
				break;

			case E_PICKUP_TYPE_SHIELD:						
				FireShield();
				break;

			default:
				break;
			}
		}
		else
		{
			// we need to fire a shock as we have all three
			if (m_IhavePerks[E_PICKUP_TYPE_SHOCK])
			{
				FireShock();
			}
		}
	}

	void CPerkSkillUsage::SelectPerkBasedUponEvaluations()
	{

		int firePerkType = -1;
		float eval = FLT_MIN;
		bool backwards = false;

		for(int i=0; i<static_cast<int>(m_PerkPriorities.size()); i++)
		{
			if (m_PerkPriorities[i].m_Eval > eval && m_PerkPriorities[i].m_Eval >= 0.0f && m_PerkPriorities[i].m_Eval <= 1.0f)
			{
				eval = m_PerkPriorities[i].m_Eval;
				firePerkType = m_PerkPriorities[i].m_Type;
				backwards = m_PerkPriorities[i].m_FireBackwards;
			}
		}

		if (eval != FLT_MIN && firePerkType != -1)
		{
			switch(firePerkType)
			{
			case E_PICKUP_TYPE_NITROUS:	
				FireNitro();
				break;

			case E_PICKUP_TYPE_SHOCK:						
				FireShock();							
				break;

			case E_PICKUP_TYPE_SHUNT:
				FireShunt(backwards);
				break;

			case E_PICKUP_TYPE_BLINDFIRE:							
				FireBlindFire(backwards);			
				break;

			case E_PICKUP_TYPE_BARGE:	
				FireBarge();
				break;

			case E_PICKUP_TYPE_BOMB:
				FireMine(backwards);
				break;

			case E_PICKUP_TYPE_REPAIR:	
				FireRepair();
				break;

			case E_PICKUP_TYPE_SHIELD:						
				FireShield();
				break;

			default:
				break;
			}
		}

	}
	

	void CPerkSkillUsage::SelectPerkBasedUponPriority()
	{

	}


	void CPerkSkillUsage::SortPerksInPriorityOrder()
	{
		IRubberbandingAI *pIRubberBanding = m_pIData->GetRubberBanding();

		int PerkPrefsTemplateIndex = -1;
		int SkillzTemplateIndex = -1;
		
		if (pIRubberBanding)
		{
			AIEventSegmentData *pEventSegmentData = pIRubberBanding->GetEventBalancingSegmentData();
			if (pEventSegmentData)
			{
				// ai system to find these data
				PerkPrefsTemplateIndex = pEventSegmentData->m_PerkPrioritiesTemplateIndex;
				m_pPerkPrefs = AIAmaxXt::Instance().GetAIPerkPrefsData(PerkPrefsTemplateIndex);
				
				m_SkillzTemplateIndex = pEventSegmentData->m_AttributeTemplateIndex;
				m_pAISkillz = AIAmaxXt::Instance().GetAIPerkSkillzData(m_SkillzTemplateIndex);	
			}
		}
		

		bool sortStuff = false;
		if (PerkPrefsTemplateIndex != m_PerkPrefsTemplateIndex && PerkPrefsTemplateIndex != -1)
		{
			sortStuff = true;
			m_PerkPrefsTemplateIndex = PerkPrefsTemplateIndex;
		}
			
		if (sortStuff)
		{
			for(int i=0; i<8;i++)
			{
				m_PerkPriorities[i].m_Type = i;

				if (m_pPerkPrefs)
				{
					switch(i)
					{
					case 0:	
						m_PerkPriorities[i].m_Value = m_pPerkPrefs->m_AIPerkPrefsParams.m_Nitro;
						break;

					case 1:				
						m_PerkPriorities[i].m_Value = m_pPerkPrefs->m_AIPerkPrefsParams.m_Shock;							
						break;

					case 2:
						m_PerkPriorities[i].m_Value = m_pPerkPrefs->m_AIPerkPrefsParams.m_Shunt;
						break;

					case 3:		
						m_PerkPriorities[i].m_Value = m_pPerkPrefs->m_AIPerkPrefsParams.m_BlindFire;			
						break;

					case 4:	
						m_PerkPriorities[i].m_Value = m_pPerkPrefs->m_AIPerkPrefsParams.m_Barge;
						break;

					case 5:
						m_PerkPriorities[i].m_Value = m_pPerkPrefs->m_AIPerkPrefsParams.m_Mine;
						break;

					case 6:	
						m_PerkPriorities[i].m_Value = m_pPerkPrefs->m_AIPerkPrefsParams.m_Repair;
						break;

					case 7:						
						m_PerkPriorities[i].m_Value = m_pPerkPrefs->m_AIPerkPrefsParams.m_Shield;
						break;

					default:
						break;
					}
				}
				else
				{
					//Rubbish default values if no template
					m_PerkPriorities[i].m_Value = 1.0f / 8.0f;
				}
			}

			sort( m_PerkPriorities.begin(), m_PerkPriorities.end(), SortPerksByPriority() );	



			
		}

		// Test the sort
		for(int i=0; i<static_cast<int>(m_PerkPriorities.size()); i++)
		{
			int t = m_PerkPriorities[i].m_Type;
			float v = m_PerkPriorities[i].m_Value;

			m_PerkPriorities[i].Reset();
		}
	}

	float CPerkSkillUsage::GetPerkPreferenceValue(int perkType)
	{
		float val = 0.0f;
		
		for(int i=0; i<static_cast<int>(m_PerkPriorities.size()); i++)
		{
			if (perkType == m_PerkPriorities[i].m_Type)
			{
				val = m_PerkPriorities[i].m_Value;
				break;
			}
		}

		return val;
	}



	void CPerkSkillUsage::CacheProjectiles()
	{
		m_Projectiles.Clear();

		int vehIdx = m_pIData->GetState().m_index;

		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor* pLogicActor = pLayer->GetLogicVehicleActor(vehIdx);
		Maths::HVec3 vehPos = pLogicActor->GetWorldMatrix().AW();
		
		Game::ActivePickupList& pickupList = Game::C_PickupsManager::Instance().GetActivePickupList();
		for( ActivePickupList::Iterator it = pickupList.Begin(); it != pickupList.End(); it++ )
		{
			m_ActivePickCountDebug++;
			C_PickupBase* pPickup = it->GetValue();

			if(pPickup)
			{
				Game::AITrackableData trackableData = pPickup->GetAITrackableData();

				// Temporary If, until SteveB fixed the perk constructor with the correct data
				//if (trackableData.mFiringIdx != vehIdx || ((pPickup->GetType() == E_PICKUP_TYPE_BOMB && pPickup->GetSubType() != E_PICKUP_SUB_TYPE_REVERSED) || pPickup->GetType() == E_PICKUP_TYPE_SHOCK) )
				if(trackableData.CanTrack(vehIdx))
				{
					// We only care about projectiles that are close enough
					float dist = Maths::Distance(vehPos, pPickup->GetMatrix().AW());
					if(dist < g_debugProjectileCacheDistance)
						m_Projectiles.PushBack(pPickup);
				}
			}
		}
	}

	void CPerkSkillUsage::SwitchToPickup(int type)
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(m_pIData->GetState().m_index);
		VehiclePickupInfo& pickupInfo = pLogicActor->GetPickupInfo();

		if(type != E_PICKUP_TYPE_NONE)
		{
			for(int i = 1; i < pickupInfo.mPickupStore.Size(); i++)
			{
				if(pickupInfo.mPickupStore[i].mType == type)
				{
					Game::Actors::PickupShuffleMsg shuffleMsg(m_pIData->GetState().m_index, true, i);
					pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &shuffleMsg);
					break;
				}
			}
		}
	}

	int CPerkSkillUsage::GetPickupStashSize()
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(m_pIData->GetState().m_index);
		VehiclePickupInfo& pickupInfo = pLogicActor->GetPickupInfo();

		return pickupInfo.mPickupStore.Size();
	}


	void CPerkSkillUsage::GeneralFirePerk(int perkType, bool backwards)
	{
		Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(m_pIData->GetState().m_index);
		pickupFireMsg.mPickupOverride = static_cast<E_PICKUP_TYPE>(perkType);
		pickupFireMsg.mBackwards = backwards;

		FirePerkMessage(&pickupFireMsg);
	}

	void CPerkSkillUsage::FireBlindFire(bool backwards)
	{
		
		// TODO : Tie this in with a skill ???
		m_BoltDelay = 0.75f; // 0.75 is reasonable...... a nice delay
		

		if (g_debugAiPerkOverrideTestBolt)
		{
			m_BoltDelay *= g_debugAiBlindFireSkill;
			m_BoltDelay = Maths::Max(m_BoltDelay, g_debugAiMinBlindFireDelay);
		}
		else
		{
			if (m_pAISkillz && m_pPerkPrefs)
			{

				if (backwards)
				{
					m_BoltDelay *= m_pAISkillz->m_Blindfire.m_Offensive;//m_AISkillzParams.m_BackwardFiring;
				}
				else
				{
					m_BoltDelay *= m_pAISkillz->m_Blindfire.m_Offensive;//m_AISkillzParams.m_ForwardFiring;
				}

				m_BoltDelay = Maths::Max(m_BoltDelay, g_debugAiMinBlindFireDelay);
			
				
				if (m_pAISkillz && m_pPerkPrefs)
				{
					if (m_pAISkillz->m_Blindfire.m_Offensive <= 0.25f)
					{
						m_GeneralDelay = 8.0f;	
					}
					else if (m_pAISkillz->m_Blindfire.m_Offensive <= 0.5f)
					{
						m_GeneralDelay = 6.0f;	
					}
					else if (m_pAISkillz->m_Blindfire.m_Offensive <= 0.75f)
					{
						m_GeneralDelay = 4.0f;	
					}
					else if (m_pAISkillz->m_Blindfire.m_Offensive <= 1.0f)
					{
						m_GeneralDelay = 2.0f;	
					}
				}	
			
			
			}
		}

		

		if (m_IhavePerks[E_PICKUP_TYPE_BLINDFIRE] > 0)
		{
			if (m_fireType == E_PICKUP_TYPE_BLINDFIRE)
			{
				if (m_pIGameServerTime->GetTime() > m_BoltTimeStamp ) 
				{
					m_BoltCount++;
					// Simple time stamp on the bolt firing......
					m_BoltTimeStamp = m_pIGameServerTime->GetTime() + m_BoltDelay;

					m_GeneralFireTimeStamp = m_pIGameServerTime->GetTime() + m_GeneralDelay;

					Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(m_pIData->GetState().m_index);
					pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_BLINDFIRE;
					pickupFireMsg.mBackwards = backwards;

					FirePerkMessage(&pickupFireMsg);

					if (m_BoltCount >= 3)
					{
						m_BoltTimeStamp = m_pIGameServerTime->GetTime() + 4.0f; // 4 seconds to reload
					}

				}
			}
			else
			{
				if (m_pIGameServerTime->GetTime() > m_BoltTimeStamp && m_pIGameServerTime->GetTime() > m_GeneralFireTimeStamp) 
				{
					// Simple time stamp on the bolt firing......
					m_BoltTimeStamp = m_pIGameServerTime->GetTime() + m_BoltDelay;

					m_GeneralFireTimeStamp = m_pIGameServerTime->GetTime() + m_GeneralDelay;

					Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(m_pIData->GetState().m_index);
					pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_BLINDFIRE;
					pickupFireMsg.mBackwards = backwards;

					FirePerkMessage(&pickupFireMsg);
					m_BoltCount = 0;
				}
			}

			
		}
		
	}

	
	void CPerkSkillUsage::FireShunt(bool backwards)
	{

		if (m_pAISkillz && m_pPerkPrefs)
		{
			if (m_pAISkillz->m_Shunt.m_Offensive <= 0.25f)
			{
				m_GeneralDelay = 8.0f;	
			}
			else if (m_pAISkillz->m_Shunt.m_Offensive <= 0.5f)
			{
				m_GeneralDelay = 6.0f;	
			}
			else if (m_pAISkillz->m_Shunt.m_Offensive <= 0.75f)
			{
				m_GeneralDelay = 4.0f;	
			}
			else if (m_pAISkillz->m_Shunt.m_Offensive <= 1.0f)
			{
				m_GeneralDelay = 2.0f;	
			}
		}	

		if (m_IhavePerks[E_PICKUP_TYPE_SHUNT] > 0)
		{
			if (m_pIGameServerTime->GetTime() > m_GeneralFireTimeStamp)
			{
				Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(m_pIData->GetState().m_index);
				pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_SHUNT;
				pickupFireMsg.mBackwards = backwards;

				m_GeneralFireTimeStamp = m_pIGameServerTime->GetTime() + m_GeneralDelay;

				FirePerkMessage(&pickupFireMsg);
			}
		}
	}

	void CPerkSkillUsage::FireBarge()
	{
		m_BargeDelay = 8.0f;

		if (m_pAISkillz && m_pPerkPrefs)
		{
			if (m_pAISkillz->m_Barge.m_Offensive <= 0.25f)
			{
				m_BargeDelay = 8.0f;	
				m_GeneralDelay = 8.0f;
			}
			else if (m_pAISkillz->m_Barge.m_Offensive <= 0.5f)
			{
				m_BargeDelay = 6.0f;	
				m_GeneralDelay = 6.0f;
			}
			else if (m_pAISkillz->m_Barge.m_Offensive <= 0.75f)
			{
				m_BargeDelay = 4.0f;	
				m_GeneralDelay = 4.0f;
			}
			else if (m_pAISkillz->m_Barge.m_Offensive <= 1.0f)
			{
				m_BargeDelay = 2.0f;	
				m_GeneralDelay = 2.0f;
			}
		}


		if (m_IhavePerks[E_PICKUP_TYPE_BARGE] > 0)
		{

			if (m_pIGameServerTime->GetTime() > m_BargeTimeStamp) 
			{
				// Simple time stamp on the bolt firing......
				m_BargeTimeStamp = m_pIGameServerTime->GetTime() + m_BargeDelay;

				m_GeneralFireTimeStamp = m_pIGameServerTime->GetTime() + m_GeneralDelay;

				Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(m_pIData->GetState().m_index);
				pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_BARGE;

				FirePerkMessage(&pickupFireMsg);
			}
		}

		
	}

	void CPerkSkillUsage::FireNitro()
	{

		if (m_pAISkillz && m_pPerkPrefs)
		{
			if (m_pAISkillz->m_Nitro.m_Offensive <= 0.25f)
			{
				m_GeneralDelay = 8.0f;	
			}
			else if (m_pAISkillz->m_Nitro.m_Offensive <= 0.5f)
			{
				m_GeneralDelay = 6.0f;	
			}
			else if (m_pAISkillz->m_Nitro.m_Offensive <= 0.75f)
			{
				m_GeneralDelay = 4.0f;	
			}
			else if (m_pAISkillz->m_Nitro.m_Offensive <= 1.0f)
			{
				m_GeneralDelay = 2.0f;	
			}
		}	

		if (m_IhavePerks[E_PICKUP_TYPE_NITROUS] > 0)
		{

			if (m_pIGameServerTime->GetTime() > m_GeneralFireTimeStamp)
			{
				Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(m_pIData->GetState().m_index);
				pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_NITROUS;

				m_GeneralFireTimeStamp = m_pIGameServerTime->GetTime() + m_GeneralDelay;

				FirePerkMessage(&pickupFireMsg);
			}
			
		}
	}

	void CPerkSkillUsage::FireShock(bool drop)
	{
		if (m_pAISkillz && m_pPerkPrefs)
		{
			if (m_pAISkillz->m_Shock.m_Offensive <= 0.25f)
			{
				m_GeneralDelay = 8.0f;	
			}
			else if (m_pAISkillz->m_Shock.m_Offensive <= 0.5f)
			{
				m_GeneralDelay = 6.0f;	
			}
			else if (m_pAISkillz->m_Shock.m_Offensive <= 0.75f)
			{
				m_GeneralDelay = 4.0f;	
			}
			else if (m_pAISkillz->m_Shock.m_Offensive <= 1.0f)
			{
				m_GeneralDelay = 2.0f;	
			}
		}	

		if (m_IhavePerks[E_PICKUP_TYPE_SHOCK] > 0)
		{
			if (m_pIGameServerTime->GetTime() > m_GeneralFireTimeStamp)
			{
				Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(m_pIData->GetState().m_index);
				pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_SHOCK;

				m_GeneralFireTimeStamp = m_pIGameServerTime->GetTime() + m_GeneralDelay;

				FirePerkMessage(&pickupFireMsg, drop);
		
			}
		}
	}

	void CPerkSkillUsage::FireMine(bool backwards)
	{
		if (m_pAISkillz && m_pPerkPrefs)
		{
			if (m_pAISkillz->m_Mine.m_Offensive <= 0.25f)
			{
				m_GeneralDelay = 8.0f;	
			}
			else if (m_pAISkillz->m_Mine.m_Offensive <= 0.5f)
			{
				m_GeneralDelay = 6.0f;	
			}
			else if (m_pAISkillz->m_Mine.m_Offensive <= 0.75f)
			{
				m_GeneralDelay = 4.0f;	
			}
			else if (m_pAISkillz->m_Mine.m_Offensive <= 1.0f)
			{
				m_GeneralDelay = 2.0f;	
			}
		}	

		if (m_IhavePerks[E_PICKUP_TYPE_BOMB] > 0)
		{
			if (m_pIGameServerTime->GetTime() > m_GeneralFireTimeStamp)
			{

				Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(m_pIData->GetState().m_index);
				pickupFireMsg.mForwards = backwards;
				pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_BOMB;

				m_GeneralFireTimeStamp = m_pIGameServerTime->GetTime() + m_GeneralDelay;

				// reset it
				m_MineTimeStamp = -1.0f;

				FirePerkMessage(&pickupFireMsg);
			}
		}
	}

	void CPerkSkillUsage::FireShield()
	{

		if (m_pAISkillz && m_pPerkPrefs)
		{
			if (m_pAISkillz->m_Shield.m_Defensive <= 0.25f)
			{
				m_GeneralDelay = 4.0f;	
			}
			else if (m_pAISkillz->m_Shield.m_Defensive <= 0.5f)
			{
				m_GeneralDelay = 3.0f;	
			}
			else if (m_pAISkillz->m_Shield.m_Defensive <= 0.75f)
			{
				m_GeneralDelay = 2.0f;	
			}
			else if (m_pAISkillz->m_Shield.m_Defensive <= 1.0f)
			{
				m_GeneralDelay = 1.0f;	
			}
		}	

		if (m_IhavePerks[E_PICKUP_TYPE_SHIELD] > 0)
		{
			if (m_pIGameServerTime->GetTime() > m_GeneralFireTimeStamp)
			{
				Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(m_pIData->GetState().m_index);
				pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_SHIELD;

				m_GeneralFireTimeStamp = m_pIGameServerTime->GetTime() + m_GeneralDelay;

				FirePerkMessage(&pickupFireMsg);
			}
		}
	}

	void CPerkSkillUsage::FireRepair()
	{

		if (m_pAISkillz && m_pPerkPrefs)
		{
			if (m_pAISkillz->m_Repair.m_Defensive <= 0.25f)
			{
				m_GeneralDelay = 5.0f;	
			}
			else if (m_pAISkillz->m_Repair.m_Defensive <= 0.5f)
			{
				m_GeneralDelay = 4.0f;	
			}
			else if (m_pAISkillz->m_Repair.m_Defensive <= 0.75f)
			{
				m_GeneralDelay = 3.0f;	
			}
			else if (m_pAISkillz->m_Repair.m_Defensive <= 1.0f)
			{
				m_GeneralDelay = 2.0f;	
			}
		}	


		if (m_IhavePerks[E_PICKUP_TYPE_REPAIR] > 0)
		{
			if (m_pIGameServerTime->GetTime() > m_GeneralFireTimeStamp)
			{
				Game::Actors::C_VehicleFirePickupMsg pickupFireMsg(m_pIData->GetState().m_index);
				pickupFireMsg.mPickupOverride = E_PICKUP_TYPE_REPAIR;

				m_GeneralFireTimeStamp = m_pIGameServerTime->GetTime() + m_GeneralDelay;

				FirePerkMessage(&pickupFireMsg);
			}
		}
	}


#define _USE_MESSAGE_LIST_

	void CPerkSkillUsage::FirePerkMessage(Game::Actors::C_VehicleFirePickupMsg *pMessage, bool drop)
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(m_pIData->GetState().m_index);
		VehiclePickupInfo& pickupInfo = pLogicActor->GetPickupInfo();
	
		SwitchToPickup(pMessage->mPickupOverride);

#ifdef _USE_MESSAGE_LIST_

		AddFireMessage(pLogicActor, pMessage, drop);
#else
		pLogicActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, pMessage);
#endif
		m_fireTimeStamp = m_pIGameServerTime->GetTime();
		m_fireType = pMessage->mPickupOverride;
		
	}

	void CPerkSkillUsage::EvaluatePerksIHave()
	{
		Game::C_RaceLogicLayer *pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::C_VehicleLogicActor *pLogicActor = pLayer->GetLogicVehicleActor(m_pIData->GetState().m_index);
		VehiclePickupInfo& pickupInfo = pLogicActor->GetPickupInfo();
		
		int sizeOfStore = pickupInfo.mPickupStore.Size();

		for(int i=0; i<8; i++)
		{
			// reset perk info.....
			m_IhavePerks[i] = 0;
			m_EvaluatePerks[i] = -999;
			
			for(int j=0; j<sizeOfStore; j++)
			{
				if (pickupInfo.mPickupStore[j].mType == i)
				{
					m_IhavePerks[i]++;
				}
			}
		}
	}






	void CPerkSkillUsage::TestBlindFire()
	{
		
		FireBlindFire(false);

#if 0
		// this is just a test to see if we can fire off the pickups.......
		if (m_IhavePerks[E_PICKUP_TYPE_BLINDFIRE] > 0)
		{
			for( int i = 1; i <= 32; i *= 2)
			{
				AvoidCar* pAvoidCar = m_pIAvoidance->GetAvoidCar(i);

				if(pAvoidCar && pAvoidCar->IsValid())
				{
					TargetEvaluation* pEval = m_pIPerkBolt->GetVehicleEvaluation(pAvoidCar->m_pIData->GetState().m_index);
					if(pEval && pEval->mEval < 1.f)
					{
						FireBlindFire(pEval->mFireBackwards);
						return;
					}
				}
			}

			for( int i = 0; i < GetNumProjectiles(); i++ )
			{
				TargetEvaluation* pEval = m_pIPerkBolt->GetProjectileEvaluation(i);
				if(pEval && pEval->mEval < 1.f)
				{
					FireBlindFire(pEval->mFireBackwards);
					return;
				}
			}
		}
#endif
	}

	void CPerkSkillUsage::TestShunt()
	{
		// this is just a test to see if we can fire off the pickups.......
		if (m_IhavePerks[E_PICKUP_TYPE_SHUNT] > 0)
		{
			for( int i = 0; i < Game::MAX_RACELOGIC_VEHICLES; i++ )
			{
				TargetEvaluation* pEval = m_pIPerkShunt->GetVehicleEvaluation(i);
				if(pEval && pEval->mEval < 1.f)
				{
					FireShunt(pEval->mFireBackwards);
					return;
				}
			}

			for( int i = 0; i < GetNumProjectiles(); i++ )
			{
				if(m_pIPerkShunt->GetProjectileEvaluation(i) < 1.f)
				{
					FireShunt(true);
					return;
				}
			}
		}
	}

	void CPerkSkillUsage::TestBarge()
	{
		// this is just a test to see if we can fire off the pickups.......
		if (m_IhavePerks[E_PICKUP_TYPE_BARGE] > 0)
		{
			for( int i = 0; i < Game::MAX_RACELOGIC_VEHICLES; i++ )
			{
				if(m_pIPerkBarge->GetVehicleEvaluation(i) < 1.f) 
				{
					FireBarge();
					return;
				}
			}

			for( int i = 0; i < GetNumProjectiles(); i++ )
			{
				if(m_pIPerkBarge->GetProjectileEvaluation(i) < 1.f)
				{
					FireBarge();
					return;
				}
			}
		}
	}
	
	void CPerkSkillUsage::TestNitro()
	{
		// this is just a test to see if we can fire off the pickups.......
		if (m_IhavePerks[E_PICKUP_TYPE_NITROUS] > 0)
		{
			// we can now fire the pickup...............
			FireNitro();
		}
	}
	
	void CPerkSkillUsage::TestShock()
	{
		// this is just a test to see if we can fire off the pickups.......
		if (m_IhavePerks[E_PICKUP_TYPE_SHOCK] > 0 && m_pIPerkShock->GetEvaluationValue() <= 1.f)
		{
			FireShock();
		}
	}
	
	void CPerkSkillUsage::TestMine()
	{
		// this is just a test to see if we can fire off the pickups.......
		if (m_IhavePerks[E_PICKUP_TYPE_BOMB] > 0)
		{
			for( int i = 1; i <= 32; i *= 2)
			{
				AvoidCar* pAvoidCar = m_pIAvoidance->GetAvoidCar(i);

				if(pAvoidCar && pAvoidCar->IsValid())
				{
					TargetEvaluation* pEval = m_pIPerkMine->GetVehicleEvaluation(pAvoidCar->m_pIData->GetState().m_index);
					if(pEval && pEval->mEval < 1.f)
					{
						FireMine(pEval->mFireBackwards);
						return;
					}
				}
			}

			for( int i = 0; i < GetNumProjectiles(); i++ )
			{
				TargetEvaluation* pEval = m_pIPerkMine->GetProjectileEvaluation(i);
				if(pEval && pEval->mEval < 1.f)
				{
					FireMine(pEval->mFireBackwards);
					return;
				}
			}

			if(m_pIPerkMine->GetDropEvaluation() < 1.0f)
			{
				FireMine(false);
				return;
			}
		}
	}
	
	void CPerkSkillUsage::TestShield()
	{
		// this is just a test to see if we can fire off the pickups.......
		if (m_IhavePerks[E_PICKUP_TYPE_SHIELD] > 0)
		{
			for( int i = 1; i <= 32; i *= 2)
			{
				AvoidCar* pAvoidCar = m_pIAvoidance->GetAvoidCar(i);

				if(pAvoidCar && pAvoidCar->IsValid())
				{
					float eval = m_pIPerkShield->GetVehicleEvaluation(pAvoidCar->m_pIData->GetState().m_index);
					if(eval < 1.f)
					{
						FireShield();
						return;
					}
				}
			}	

			for( int i = 0; i < GetNumProjectiles(); i++ )
			{
				float eval = m_pIPerkShield->GetProjectileEvaluation(i);
				if(eval < 1.f)
				{
					FireShield();
					return;
				}
			}
		}
	}
	
	void CPerkSkillUsage::TestRepair()
	{
		// this is just a test to see if we can fire off the pickups.......
		if (m_IhavePerks[E_PICKUP_TYPE_REPAIR] > 0 && m_pIPerkRepair->GetEvaluationValue() < 1.f)
		{
			// we can now fire the pickup...............
			FireRepair();
		}
	}

	void CPerkSkillUsage::EvaluateSituation()
	{
		if (m_pIData && m_pIAvoidance)
		{	
			for(int i=0; i<8;i++)
			{
				if (m_IhavePerks[i] > 0)
				{
					switch(i)
					{
						case E_PICKUP_TYPE_NITROUS:	
							EvaluateNitro();
							break;

						case E_PICKUP_TYPE_SHOCK:						
							EvaluateShock();							
							break;

						case E_PICKUP_TYPE_SHUNT:
							EvaluateShunt();
							break;

						case E_PICKUP_TYPE_BLINDFIRE:							
							EvaluateBolt();			
							break;

						case E_PICKUP_TYPE_BARGE:	
							EvaluateBarge();
							break;

						case E_PICKUP_TYPE_BOMB:
							EvaluateMine();
							break;

						case E_PICKUP_TYPE_REPAIR:	
							EvaluateRepair();
							break;

						case E_PICKUP_TYPE_SHIELD:						
							EvaluateShield();
							break;

						default:
							break;
					}
				}
			}
		}
	}


	void CPerkSkillUsage::EvaluateBolt()
	{
		if (m_pIPerkBolt)
		{
			m_pIPerkBolt->Evaluate();


			bool offensiveBackwards = false;
			float offensiveEval = FLT_MIN;	
			for( int i = 1; i <= 32; i *= 2)
			{
				AvoidCar* pAvoidCar = m_pIAvoidance->GetAvoidCar(i);

				if(pAvoidCar && pAvoidCar->IsValid())
				{
					TargetEvaluation* pEval = m_pIPerkBolt->GetVehicleEvaluation(pAvoidCar->m_pIData->GetState().m_index);

					float eval = pEval->mEval;
						
					if(pEval && eval > 0.0f && eval > offensiveEval && eval <= 1.0f)
					{
						offensiveEval = eval;
						offensiveBackwards = pEval->mFireBackwards;
					}
				}
			}


			float defensiveEval = FLT_MIN;	

			for( int i = 0; i < m_pIPerkBolt->GetNumProjectiles(); i++ )
			{
				TargetEvaluation* pEval = m_pIPerkBolt->GetProjectileEvaluation(i);
				float eval = 1.0f - pEval->mEval;
				
				if(pEval && pEval->mEval > 0.0f && eval > defensiveEval && eval <= 1.0f)
				{
					defensiveEval = eval;
				}
			}

			
			if (m_pAISkillz && m_pPerkPrefs)
			{
				// really need to add/subtract a fuzzy amount based upon the skill, not a multiplier.....
			
				if (offensiveBackwards)
				{
					offensiveEval *= m_pAISkillz->m_Blindfire.m_Offensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_BlindFire;
				}
				else
				{
					offensiveEval *= m_pAISkillz->m_Blindfire.m_Offensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_BlindFire;
				}
				

				defensiveEval *= m_pAISkillz->m_Blindfire.m_Defensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_BlindFire;	

			}


			PerkPriorities *pPerkPri = GetPerkPriorities(E_PICKUP_TYPE_BLINDFIRE);
			if (pPerkPri)
			{
				if (offensiveEval > defensiveEval)
				{
					pPerkPri->m_Eval = offensiveEval;
					pPerkPri->m_FireBackwards = offensiveBackwards;
				}
				else
				{
					pPerkPri->m_Eval = defensiveEval;
					pPerkPri->m_FireBackwards = true;
				}
			}
		}

	}

	void CPerkSkillUsage::EvaluateShunt()
	{
		int situation = m_pIAvoidance->GetOverallSituation();

		if (m_pIPerkShunt)
		{
			m_pIPerkShunt->Evaluate();


			bool offensiveBackwards = false;
			float offensiveEval = FLT_MIN;	
			for( int i = 0; i < Game::MAX_RACELOGIC_VEHICLES; i++ )
			{
				
				TargetEvaluation* pEval = m_pIPerkShunt->GetVehicleEvaluation(i);
				float eval = 1.0f - pEval->mEval;

				if(pEval && pEval->mEval > 0.0f && eval > offensiveEval)
				{
					offensiveEval = eval;
					offensiveBackwards = pEval->mFireBackwards;
				}
			}

			float defensiveEval = FLT_MIN;	

			for( int i = 0; i < m_pIPerkShunt->GetNumProjectiles(); i++ )
			{
				float eval = 1.0f - m_pIPerkShunt->GetProjectileEvaluation(i);

				if(eval > 0.0f && eval > defensiveEval && eval < 1.0f)
				{
					defensiveEval = eval;
				}
			}

			if (m_pAISkillz && m_pPerkPrefs)
			{
				// really need to add/subtract a fuzzy amount based upon the skill, not a multiplier.....
				
				if (offensiveBackwards)
				{
					offensiveEval *= m_pAISkillz->m_Shunt.m_Offensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Shunt;
				}
				else
				{
					offensiveEval *= m_pAISkillz->m_Shunt.m_Offensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Shunt;
				}
				
				defensiveEval *= m_pAISkillz->m_Shunt.m_Defensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Shunt;	

			}


			PerkPriorities *pPerkPri = GetPerkPriorities(E_PICKUP_TYPE_SHUNT);
			if (pPerkPri)
			{
				if (offensiveEval > defensiveEval)
				{
					pPerkPri->m_Eval = offensiveEval;
					pPerkPri->m_FireBackwards = offensiveBackwards;
				}
				else
				{
					pPerkPri->m_Eval = defensiveEval;
					pPerkPri->m_FireBackwards = true;
				}
			}
			
		}

	}

	void CPerkSkillUsage::EvaluateShock()
	{
		int situation = m_pIAvoidance->GetOverallSituation();
		if (m_pIPerkShock)
		{
			m_pIPerkShock->Evaluate();

			float offensiveEval = FLT_MIN;
			float eval = 1.0f - m_pIPerkShock->GetEvaluationValue();
			if (eval > 0.0f && eval < 1.0f)
			{
				offensiveEval = eval;
			}
			
			if (m_pAISkillz && m_pPerkPrefs)
			{
				// really need to add/subtract a fuzzy amount based upon the skill, not a multiplier.....
				offensiveEval *= m_pAISkillz->m_Shock.m_Offensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Shock;

			}

			PerkPriorities *pPerkPri = GetPerkPriorities(E_PICKUP_TYPE_SHOCK);
			if (pPerkPri)
			{
				pPerkPri->m_Eval = offensiveEval;
			}

		}
	}

	void CPerkSkillUsage::EvaluateNitro()
	{
		int situation = m_pIAvoidance->GetOverallSituation();
		if (m_pIPerkNitro)
		{
			m_pIPerkNitro->Evaluate();

			float offensiveEval = 1.0f;

			if (m_pAISkillz && m_pPerkPrefs)
			{
				// really need to add/subtract a fuzzy amount based upon the skill, not a multiplier.....
				offensiveEval *= m_pAISkillz->m_Nitro.m_Offensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Nitro;
			}

			PerkPriorities *pPerkPri = GetPerkPriorities(E_PICKUP_TYPE_NITROUS);
			if (pPerkPri)
			{
				pPerkPri->m_Eval = offensiveEval;
			}

		}
	}

	void CPerkSkillUsage::EvaluateMine()
	{
		int situation = m_pIAvoidance->GetOverallSituation();
		if (m_pIPerkMine)
		{
			if (m_MineTimeStamp < 0.0f)
			{
				m_MineTimeStamp = m_pIGameServerTime->GetTime();
			}

			m_pIPerkMine->Evaluate();

			float offensiveEval = FLT_MIN;
			bool offensiveBackwards = false;
			

			float dropEval = 1.0f - m_pIPerkMine->GetDropEvaluation();


			for( int i = 1; i <= 32; i *= 2)
			{
				AvoidCar* pAvoidCar = m_pIAvoidance->GetAvoidCar(i);

				if(pAvoidCar && pAvoidCar->IsValid())
				{
					TargetEvaluation* pEval = m_pIPerkMine->GetVehicleEvaluation(pAvoidCar->m_pIData->GetState().m_index);
					float eval = 1.0f - pEval->mEval;

					if(pEval && pEval->mEval > 0.0f && eval > offensiveEval)
					{
						offensiveEval = eval;
						offensiveBackwards = pEval->mFireBackwards;
					}

				}
			}

			float defensiveEval = FLT_MIN;	

			for( int i = 0; i < m_pIPerkMine->GetNumProjectiles(); i++ )
			{
				TargetEvaluation* pEval = m_pIPerkMine->GetProjectileEvaluation(i);
				float eval = 1.0f - pEval->mEval;

				if(pEval && pEval->mEval > 0.0f && eval > defensiveEval)
				{
					defensiveEval = eval;
				}
			}

			


			// TODO : dropping mines in a extra defensive way....
/*
			if(m_pIPerkMine->GetDropEvaluation() < 1.0f)
			{
				FireMine(false);
				return;
			}
*/

			if (m_pAISkillz && m_pPerkPrefs)
			{
				// really need to add/subtract a fuzzy amount based upon the skill, not a multiplier.....
				offensiveEval *= m_pAISkillz->m_Mine.m_Offensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Mine;
									
				defensiveEval *= m_pAISkillz->m_Mine.m_Defensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Mine;	

			}


			PerkPriorities *pPerkPri = GetPerkPriorities(E_PICKUP_TYPE_BOMB);
			if (pPerkPri)
			{

				if (dropEval > 0.0f && dropEval < 1.0f)
				{
					pPerkPri->m_Eval = dropEval * m_pPerkPrefs->m_AIPerkPrefsParams.m_Mine;
					pPerkPri->m_FireBackwards = false;
				}
				else if (offensiveEval > 0.0f && offensiveEval < 1.0f && defensiveEval > 0.0f && defensiveEval < 1.0f)
				{
					if (offensiveEval > defensiveEval)
					{
						pPerkPri->m_Eval = offensiveEval;
						pPerkPri->m_FireBackwards = offensiveBackwards;
					}
					else
					{
						pPerkPri->m_Eval = defensiveEval;
						pPerkPri->m_FireBackwards = false;//true; // true;
					}
				}
				else if (offensiveEval > 0.0f && offensiveEval < 1.0f)
				{
					pPerkPri->m_Eval = offensiveEval;
					pPerkPri->m_FireBackwards = offensiveBackwards;
				}
				else if (defensiveEval > 0.0f && defensiveEval < 1.0f)
				{
					pPerkPri->m_Eval = defensiveEval;
					pPerkPri->m_FireBackwards = false;//true; // true;
				}
				else
				{
					pPerkPri->m_Eval = 999.0f;
				}
			}
		}
	}

	void CPerkSkillUsage::EvaluateBarge()
	{
		// TODO : Add in the skill, below doesn`t work very well with one on one, it will always be good at defending. ?????

		int situation = m_pIAvoidance->GetOverallSituation();
		if (m_pIPerkBarge)
		{
			m_pIPerkBarge->Evaluate();

			// TODO : A higher skill if we hit more than one.
		
			float offensiveEval = FLT_MIN;
			for( int i = 0; i < Game::MAX_RACELOGIC_VEHICLES; i++ )
			{
				float eval = 1.0f - m_pIPerkBarge->GetVehicleEvaluation(i);
				if(eval < 1.0f && eval > offensiveEval && eval > 0.0f) 
				{
					offensiveEval = eval;
				}
			}

			float defensiveEval = FLT_MIN;			
			for( int i = 0; i < m_pIPerkBarge->GetNumProjectiles(); i++ )
			{
				float eval = 1.0f - m_pIPerkBarge->GetProjectileEvaluation(i);
				if(eval < 1.0f && eval > defensiveEval && eval > 0.0f) 
				{
					defensiveEval = eval;
				}
			}

			if (m_pAISkillz && m_pPerkPrefs)
			{
				// really need to add/subtract a fuzzy amount based upon the skill, not a multiplier.....
				offensiveEval *= m_pAISkillz->m_Barge.m_Offensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Barge;

				defensiveEval *= m_pAISkillz->m_Barge.m_Defensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Barge;	

			}


			PerkPriorities *pPerkPri = GetPerkPriorities(E_PICKUP_TYPE_BARGE);
			if (pPerkPri)
			{
				pPerkPri->m_Eval = Maths::Max(offensiveEval, defensiveEval);
				pPerkPri->m_FireBackwards = false;
			}
		}
	}

	void CPerkSkillUsage::EvaluateShield()
	{
		int situation = m_pIAvoidance->GetOverallSituation();
		if (m_pIPerkShield)
		{
			m_pIPerkShield->Evaluate();


			float offensiveEval = FLT_MIN;
			for( int i = 1; i <= 32; i *= 2)
			{
				AvoidCar* pAvoidCar = m_pIAvoidance->GetAvoidCar(i);

				if(pAvoidCar && pAvoidCar->IsValid())
				{
					float eval = 1.0f - m_pIPerkShield->GetVehicleEvaluation(pAvoidCar->m_pIData->GetState().m_index);
					if(eval > 0.0f && eval < 1.0f && eval > offensiveEval)
					{
						offensiveEval = eval;
					}
				}
			}	

			float defensiveEval = FLT_MIN;
			for( int i = 0; i < m_pIPerkShield->GetNumProjectiles(); i++ )
			{
				float eval = 1.0f - m_pIPerkShield->GetProjectileEvaluation(i);
				if(eval > 0.0f && eval < 1.0f && eval > defensiveEval)
				{
					defensiveEval = eval;	
				}
			}

			if (m_pAISkillz && m_pPerkPrefs)
			{
				// really need to add/subtract a fuzzy amount based upon the skill, not a multiplier.....
				offensiveEval *= m_pAISkillz->m_Shield.m_Defensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Shield;

				defensiveEval *= m_pAISkillz->m_Shield.m_Defensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Shield;	

			}


			PerkPriorities *pPerkPri = GetPerkPriorities(E_PICKUP_TYPE_SHIELD);
			if (pPerkPri)
			{
				pPerkPri->m_Eval = Maths::Max(offensiveEval, defensiveEval);
				pPerkPri->m_FireBackwards = false;
			}



		}
	}

	void CPerkSkillUsage::EvaluateRepair()
	{
		int situation = m_pIAvoidance->GetOverallSituation();
		if (m_pIPerkRepair)
		{
			m_pIPerkRepair->Evaluate();

			float eval = 1.0f - m_pIPerkRepair->GetEvaluationValue();
			if (m_pAISkillz && m_pPerkPrefs)
			{
				eval *= m_pAISkillz->m_Repair.m_Defensive * m_pPerkPrefs->m_AIPerkPrefsParams.m_Repair;
			}

			PerkPriorities *pPerkPri = GetPerkPriorities(E_PICKUP_TYPE_REPAIR);
			if (pPerkPri)
			{
				pPerkPri->m_Eval = eval;
				pPerkPri->m_FireBackwards = false;
			}
		}
	}

	PerkPriorities *CPerkSkillUsage::GetPerkPriorities(int perkType)
	{
		PerkPriorities *pPerkPri = 0;
		for(int i=0; i<(int)m_PerkPriorities.size(); i++)
		{
			if (m_PerkPriorities[i].m_Type == perkType)
			{
				pPerkPri = &m_PerkPriorities[i];
				break;
			}

		}
		return pPerkPri;
	}

	void CPerkSkillUsage::PredictVehiclePos(Game::Actors::C_VehicleActor* pVehActor, Maths::HVec3& predictPos, float lookAheadTime)
	{
		if(pVehActor)
		{
			Maths::HVec3 velocity = pVehActor->GetVehiclePhysics()->GetVelocity();
			predictPos += (velocity * lookAheadTime);
		}
	}

	void CPerkSkillUsage::PredictProjectilePos(Game::C_PickupBase* pPickup, Maths::HVec3& predictPos, float lookAheadTime)
	{
		if(pPickup)
		{
			Game::AITrackableData& data = pPickup->GetAITrackableData();
			predictPos += (data.mVelocity * lookAheadTime);
		}
	}



	float CPerkSkillUsage::PredictWillHitTarget(Maths::HVec3 shotPos, Maths::HVec3 shotVel, Maths::HVec3 targetPos, float missDot)
	{
		Maths::C_Vertex pos1(shotPos);
		Maths::C_Vertex pos2(shotPos + shotVel);

		Maths::HVec3 dir1	= Maths::Normalise(pos2 - pos1);
		Maths::HVec3 dir2	= Maths::Normalise(targetPos - pos1);
		float dot			= Maths::DotProduct(dir1, dir2);

		float eval = 2.f;
		if(dot >= 0.f)
		{
			float inverseDot			= 1.f - dot;
			float inverseMissDot		= 1.f - missDot;
			float numIncrements			= inverseMissDot / missDot;

			eval = inverseDot + (inverseDot * numIncrements);
		}

		return eval;
	}

	float CPerkSkillUsage::GetTargetDistanceAhead(int idx)
	{
		float distRoundTrack = 0.f;

		IData* pData = m_pISystem->GetData(idx);
		if(pData)
		{
			ISplineTracker* pSplineTracker	= pData->GetSplineTracker();
			if(pSplineTracker)
			{
				const airacingtech::RouteInfo* pLogicRoute = pSplineTracker->GetLogicRoute();
				if(pLogicRoute && pLogicRoute->m_pRoute)
				{
					int numSegs					= pLogicRoute->m_pRoute->GetNumSegments();
					float logicCombined			= pData->GetVehicleLogic()->GetLogicState()->GetLogicPositionCombined();
					distRoundTrack				= (logicCombined * numSegs);
				}
			}
		}


		return distRoundTrack;
	}


	void CPerkSkillUsage::FireMessages()
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
	}

	int CPerkSkillUsage::GetGroup(IData *pIData)
	{
		int myGroup = 0;

		if (pIData)
		{
			IRubberbandingAI *pIRubber = 0;
			AIEventSegmentData *pEventBalancingSegmentDataME = 0;		
			pIRubber = pIData->GetRubberBanding();
			if (pIRubber)
			{
				pEventBalancingSegmentDataME = pIRubber->GetEventBalancingSegmentData();
				if (pEventBalancingSegmentDataME)
				{
					myGroup = pEventBalancingSegmentDataME->m_Group; 
				}
			}
		}
		

		return myGroup;
	}
	
}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif
