#include "stdafx.h"
#include <Perks/PerkShock/cPerkShock.h>
#include <data/idata.h>
#include <avoidance/iavoidance.h>
#include <Perks/PerkAndSkillUsage/iPerkSkillUsage.h>
#include <gameservertime/cgameservertime.h>
#include <system/isystemai.h>

Debug::Float g_debugAIPerksShockDistanceFromFirer("AI.Perks.Shock.DistanceFromFirer", 20.f);

namespace airacingtech
{
	CPerkShock::CPerkShock()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_pIPerkSkillUsage = 0;

		m_pRaceLogicLayer = 0;
		m_pLogicActor = 0;

		m_LastEvaluationValue = 2.f;
		m_pISystem = 0;
	}

	CPerkShock::~CPerkShock()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_pIPerkSkillUsage = 0;

		m_pRaceLogicLayer = 0;
		m_pLogicActor = 0;
		m_pISystem = 0;
	}

	void CPerkShock::RegisterComponentType()
	{
		
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PERK_SHOCK, CID_PERK_SHOCK);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PERK_SHOCK, CPerkShock::CreateMe, CPerkShock::DestroyMe, CHash("PerkShock"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_SHOCK, MT_POST_INIT_AI_SYSTEM);


	}

	IComponent *CPerkShock::CreateMe()
	{
		return new CPerkShock;
	}

	bool CPerkShock::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPerkShock::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CPerkShock::Deinit(void)
	{
		return;
	}

	EMessageResult CPerkShock::HandleMessage(const CMessage &msg)
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

		default:
			break;
		}

		return msgRet;
	}


	EComponentTypeId CPerkShock::GetComponentTypeId(void)
	{
		return CID_PERK_SHOCK;
	}

	void CPerkShock::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

		m_pIAvoidance = static_cast<IAvoidance *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_AVOIDANCE));

		m_pIPerkSkillUsage = static_cast<IPerkSkillUsage *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERKS_SKILL_USAGE));

		m_pRaceLogicLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		if(m_pRaceLogicLayer)
		{
			m_pLogicActor	= m_pRaceLogicLayer->GetLogicVehicleActor(m_pIData->GetState().m_index);
		}

		CObjectId systemID("AmaxSystemAI");
		m_pISystem = static_cast<ISystemAI *>(GlobalsComponent::GetObjectManager().QueryInterface(systemID, IID_SYSTEM_AI));

	}



	void CPerkShock::Evaluate()
	{
		m_LastEvaluationValue = 2.f;

		if (m_pIAvoidance && m_pIData && m_pIGameServerTime && m_pRaceLogicLayer && m_pLogicActor && m_pISystem)
		{

			int myGroup = 0;

			if (m_pIPerkSkillUsage)
			{
				myGroup = m_pIPerkSkillUsage->GetGroup(m_pIData);
			}


			int shockTargetIdx = -1;
			VehiclePickupInfo& pickupInfo = m_pLogicActor->GetPickupInfo();

			// We might already have this calculated.
			if(pickupInfo.mPickupStoreIndex >= 0 && pickupInfo.mPickupStoreIndex < pickupInfo.mPickupStore.Size() && 
				pickupInfo.mPickupStore[pickupInfo.mPickupStoreIndex].mType == E_PICKUP_TYPE_SHOCK)
			{
				shockTargetIdx = pickupInfo.mCurrentTargetIdx;
			}
			else
			{
				shockTargetIdx = m_pLogicActor->EvaluateShockTarget();
			}

			if(shockTargetIdx != -1 && shockTargetIdx != m_pLogicActor->GetVehicleIndex())
			{
				Game::Actors::C_VehicleLogicActor* pActor = m_pRaceLogicLayer->GetLogicVehicleActor(shockTargetIdx);


				bool evaluate = true;
				IData *pIDataTarget = m_pISystem->GetData(shockTargetIdx);
				if (pIDataTarget)
				{
					int avoidGroup = m_pIPerkSkillUsage->GetGroup(pIDataTarget);

					if (myGroup == 1 || avoidGroup == 2 || (myGroup == avoidGroup && myGroup != 0))
					{
						evaluate = false;
					}
				}


				if(pActor && evaluate)
				{

					float myDist	= m_pIPerkSkillUsage->GetTargetDistanceAhead(m_pLogicActor->GetVehicleIndex());
					float hisDist	= m_pIPerkSkillUsage->GetTargetDistanceAhead(shockTargetIdx);

					float diff		= hisDist - myDist;

					// Doesn't handle lapping, but that might not matter
					if(diff > 0.1f)
						m_LastEvaluationValue = g_debugAIPerksShockDistanceFromFirer / diff;
				}
			}
		}
	}
}




