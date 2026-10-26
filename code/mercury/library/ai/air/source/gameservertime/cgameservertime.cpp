#include "stdafx.h"
#include <gameservertime/cgameservertime.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif


namespace airacingtech
{

	CGameServerTime::CGameServerTime()
	{
		SetFrameRate(30); // default
		m_time = 0.0f;
	}

	CGameServerTime::~CGameServerTime()
	{

	}

	void CGameServerTime::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_GAMESERVER_TIME, CID_GAMESERVER_TIME);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_GAMESERVER_TIME, CGameServerTime::CreateMe, CGameServerTime::DestroyMe, CHash("GameServerTime"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_GAMESERVER_TIME, MT_POST_INIT_AI_SYSTEM);
	}

	IComponent *CGameServerTime::CreateMe()
	{
		return new CGameServerTime;
	}

	bool CGameServerTime::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}

	// Virtual IComponent methods
	bool CGameServerTime::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}


	void CGameServerTime::Deinit(void)
	{

	}

	EMessageResult CGameServerTime::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;
		switch (msg.mType)
		{
			case MT_POST_INIT_AI_SYSTEM:
				{
					msgRet =  MR_TRUE;
					
				}
				break;


			default:
				break;
		}

		return msgRet;
	}


	EComponentTypeId CGameServerTime::GetComponentTypeId(void)
	{
		return CID_GAMESERVER_TIME;
	}
}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


