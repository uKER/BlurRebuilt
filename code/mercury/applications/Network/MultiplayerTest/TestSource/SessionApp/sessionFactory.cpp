
#include "stdafx.h"
#include "sessionFactory.h"
#include "netStream/netSerialise.h"
#include "sessionEnums.h"
#include "netstate/stateManager.h"
#include "netstate/gameState.h"
#include "netstate/netHandle/netEntityHandle.h"
#include "netstate/netHandle/netHandle.h"
#include "netstate/netEntity/entityHeader.h"
#include "TestSource/sessionApp/TestSession.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

I_Factory * I_Factory::CreateInstance(void)
{
	return new C_SessionFactory(1);
}

C_SessionFactory::C_SessionFactory(int uGameStates) : I_Factory(uGameStates)	
{
	;
}

C_SessionFactory::~C_SessionFactory()
{
	;
}


bool C_SessionFactory::CreateGameSideObject(C_Header& xStateInfo, int uGameStateId)
{
	bool bSuccess = false;
	switch(xStateInfo.m_uTypeId)
	{
	default: 
		{
			BIZ_ASSERT_MSG(false, "unknown entity!");
			bSuccess=false;
			break;
		}
	case E_NetEnt_HostSessionState:
	CASE_PER_CLIENT(E_NetEnt_PeerSessionState)
		{
			bSuccess=Test::Session::instance()->factoryCreate(xStateInfo.m_uTypeId, false);
			break;
		}
	}
	return bSuccess;
}

bool C_SessionFactory::DestroyGameSideObject(class C_NetHandle& xHandle)
{
	C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(xHandle.GameStateId());
	if(!pxGameState)
		return false;

	C_EntityState* pxEntityState = pxGameState->EntityState(xHandle.NetId()); assert(pxEntityState);
	if(!pxEntityState)
		return false;

	bool bSuccess = false;
	const int eTypeId = pxEntityState->Header().m_uTypeId;
	switch(eTypeId)
	{
	default: 
		{
			BIZ_ASSERT_MSG(false, "unknown entity!");
			bSuccess=false;
			break;
		}
	case E_NetEnt_HostSessionState:
	CASE_PER_CLIENT(E_NetEnt_PeerSessionState)
		{
			bSuccess=Test::Session::instance()->factoryDestroy(eTypeId);
			break;
		}
	}
	return bSuccess;
}

bool C_SessionFactory::SerialiseNetworkEntity(class netStream* stream, class C_GameNetEntity& xGameNetEntity)
{ 
	switch(xGameNetEntity.Type())
	{
	default: 
		{
			BIZ_ASSERT_MSG(false, "unknown entity!");
			return false;
		}
	case E_NetEnt_HostSessionState:
	CASE_PER_CLIENT(E_NetEnt_PeerSessionState)
		{
			return Test::Session::instance()->factorySerialise(xGameNetEntity.Type(), stream);
		}
	}
}

bool C_SessionFactory::DeSerialiseNetworkEntity(class netStream* stream, class C_GameNetEntity& xGameNetEntity)
{
	switch(xGameNetEntity.Type())
	{
	default: 
		{
			BIZ_ASSERT_MSG(false, "unknown entity!");
			return false;
		}
	case E_NetEnt_HostSessionState:
	CASE_PER_CLIENT(E_NetEnt_PeerSessionState)
		{
			return Test::Session::instance()->factoryDeserialise(xGameNetEntity.Type(), stream);
		}
	}
}

int	C_SessionFactory::RelevantGameState(int uType)
{
	switch(uType)
	{
	default: 
		{
			BIZ_ASSERT_MSG(false, "unknown entity!");
			return -1;
		}
	case E_NetEnt_HostSessionState:
	CASE_PER_CLIENT(E_NetEnt_PeerSessionState)
		{
			return 0;
		}
	}
}

E_HandleType C_SessionFactory::GetHandleType(int uTypeId)
{
	switch(uTypeId)
	{
	default: 
		{
			BIZ_ASSERT_MSG(false, "unknown entity!");
			return E_Handle_Invalid;
		}
	case E_NetEnt_HostSessionState:
	CASE_PER_CLIENT(E_NetEnt_PeerSessionState)		return E_Handle_Network;
	};
}

int	C_SessionFactory::RelevantNetStateType(int uType)
{
	switch(uType)
	{
	default: 
		{
			BIZ_ASSERT_MSG(false, "unknown entity!");
			return E_NetState_PeerBroadcast;
		}
	case E_NetEnt_HostSessionState:
		{
			return E_NetState_ServerBroadcast;
		}
	CASE_PER_CLIENT(E_NetEnt_PeerSessionState)
		{
			return E_NetState_PeerBroadcast;
		}
	}
}

const char* C_SessionFactory::GameStateString(int eGameState)
{
	switch(eGameState)
	{
	default: 
		{
			BIZ_ASSERT_MSG(false, "unknown game state!");
			return "unknown";
		}
	case 0:
		{
			return "lobby";
		}
	}		
}

const char* C_SessionFactory::NetEntityString(int eType)
{
	switch(eType)
	{
	default:									return "unknown";
	case E_NetEnt_HostSessionState:				return "NetHost";
	CASE_PER_CLIENT(E_NetEnt_PeerSessionState)	return "NetPeer";
	}
}

const char* C_SessionFactory::NetStateString(int eType)
{
	switch(eType)
	{
	case E_NetState_Deleting:					return "Delete      ";
	case E_NetState_ServerBroadcast:			return "ServBC      ";
	case E_NetState_PeerBroadcast:				return "PeerBC      ";
	case E_NetState_PeerToServer:				return "Peer2S      ";
	case E_NetState_ServerToPeer:				return "Serv2P      ";
	default:									return "Unknwn      ";
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
