#include "stdafx.h"
#include <Perks/PerkRepair/cPerkRepair.h>
#include <data/idata.h>
#include <avoidance/iavoidance.h>
#include <gameservertime/cgameservertime.h>

#include <GameApp.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>


namespace airacingtech
{
	CPerkRepair::CPerkRepair()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_LastEvaluationValue = 1.f;

		m_pRaceLogicLayer = NULL;
		m_pLogicActor = NULL;
	}

	CPerkRepair::~CPerkRepair()
	{

	}

	void CPerkRepair::RegisterComponentType()
	{
		
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PERK_REPAIR, CID_PERK_REPAIR);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PERK_REPAIR, CPerkRepair::CreateMe, CPerkRepair::DestroyMe, CHash("PerkRepair"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_REPAIR, MT_POST_INIT_AI_SYSTEM);


	}

	IComponent *CPerkRepair::CreateMe()
	{
		return new CPerkRepair;
	}

	bool CPerkRepair::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPerkRepair::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CPerkRepair::Deinit(void)
	{
		return;
	}

	EMessageResult CPerkRepair::HandleMessage(const CMessage &msg)
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


	EComponentTypeId CPerkRepair::GetComponentTypeId(void)
	{
		return CID_PERK_REPAIR;
	}

	void CPerkRepair::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

		m_pIAvoidance = static_cast<IAvoidance *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_AVOIDANCE));

		m_pRaceLogicLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		if(m_pRaceLogicLayer)
			m_pLogicActor	= m_pRaceLogicLayer->GetLogicVehicleActor(m_pIData->GetState().m_index);

	}

	void CPerkRepair::Evaluate()
	{
		if (m_pIAvoidance && m_pIData && m_pIGameServerTime && m_pLogicActor)
		{
			float healthPercent = ((static_cast<float>(m_pLogicActor->GetCurrentHealth() + 1.0f)) / static_cast<float>(m_pLogicActor->GetMaxHealth()));
			m_LastEvaluationValue = healthPercent;
		}
	}
	
}




