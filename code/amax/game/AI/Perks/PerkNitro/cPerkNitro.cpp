#include "stdafx.h"
#include <Perks/PerkNitro/cPerkNitro.h>
#include <data/idata.h>
#include <avoidance/iavoidance.h>
#include <gameservertime/cgameservertime.h>


namespace airacingtech
{
	CPerkNitro::CPerkNitro()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
	}

	CPerkNitro::~CPerkNitro()
	{

	}

	void CPerkNitro::RegisterComponentType()
	{
		
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PERK_NITRO, CID_PERK_NITRO);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PERK_NITRO, CPerkNitro::CreateMe, CPerkNitro::DestroyMe, CHash("PerkNitro"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_NITRO, MT_POST_INIT_AI_SYSTEM);
	}

	IComponent *CPerkNitro::CreateMe()
	{
		return new CPerkNitro;
	}

	bool CPerkNitro::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPerkNitro::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CPerkNitro::Deinit(void)
	{
		return;
	}

	EMessageResult CPerkNitro::HandleMessage(const CMessage &msg)
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


	EComponentTypeId CPerkNitro::GetComponentTypeId(void)
	{
		return CID_PERK_NITRO;
	}

	void CPerkNitro::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

		m_pIAvoidance = static_cast<IAvoidance *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_AVOIDANCE));

	}

	void CPerkNitro::Evaluate()
	{
		if (m_pIAvoidance && m_pIData && m_pIGameServerTime)
		{

		}
	}
	
}




