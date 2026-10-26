#include "NightwingPch.h"
#include "netTestGameFactory.h"
#include "netTestGameWorld.h"
#include "netApp/netApp.h"
#include "netConnect/netGraphData.h"
#include "netStream/netSerialise.h"
#include "netEntities/components/netPartyData.h"
#include "netEntities/components/netGameData.h"
#include "netGroup/netPartyMediator.h"
#include "netGroup/netGameMediator.h"
#include "netState/stateManager.h"
#include "netState/gameState.h"
#include "netState/netEntity/entityHeader.h"
#include "netGame/netGameEnums.h"
#include "netGame/netHandle/netHandle.h"
#include "netGame/netHandle/netEntityHandle.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif

C_TestGameFactory::C_TestGameFactory()
	: C_Factory(E_State_Count)
{
	for(int uNetId = 0; uNetId < MAX_ENTITIES; ++uNetId)
		m_uLocalHandle[uNetId] = -1;
}
	
C_TestGameFactory::~C_TestGameFactory()
{;}

int	C_TestGameFactory::GetNetHandle(int uLocalId)
{
	for(int uNetId = 0; uNetId < MAX_ENTITIES; ++uNetId)
	{
		if(m_uLocalHandle[uNetId] == uLocalId)
			return uNetId;
	}

	return -1;
}

bool C_TestGameFactory::Authority(int uHostId, int uLocalId) 
{
	C_NetHandle xNetHandle(GetNetHandle(uLocalId), E_State_GameWorld);
	return C_Factory::Authority(uHostId, xNetHandle);
}

int C_TestGameFactory::PredictNetId()
{
	C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(E_State_GameWorld); 
	if(!pxGameState)
		return -1;

	return pxGameState->GetFreeNetId(-1);
}

bool C_TestGameFactory::CreateNetEntity(C_Header& xStateInfo)
{
	int uGameStateId = RelevantGameState(xStateInfo.m_uTypeId);
	C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uGameStateId); 
	if(!pxGameState)
		return false;

	const int eTypeId		= xStateInfo.m_uTypeId;
	const char uAuthorityId = xStateInfo.m_uAuthorityId;
	const char eStateType	= xStateInfo.NetStateType();
	const int uServerId		= pxGameState->ServerId();
	const bool bServer		= pxGameState->IsServer();
	const bool bAuthority	= uAuthorityId == pxGameState->MyHostId();

	if(bServer && xStateInfo.m_uEntityId != g_sNetIdAutoAssign)
	{
		assert(!"server should auto assign netId's!");
		xStateInfo.m_uEntityId = g_sNetIdAutoAssign;
	}


	int uParentLocalHandle = -1;
	C_NetHandle handle(xStateInfo.m_uParentEntityId, uGameStateId);
	C_GameNetEntity* pxParentHandle = xStateInfo.m_uParentEntityId >= 0 ? FindGameEntity(handle) : NULL;
	if(pxParentHandle)
	{
		uParentLocalHandle = GetLocalHandle(pxParentHandle->NetHandle().NetId());
	}

	bool bSuccess = false;
	switch(eTypeId)
	{
	case E_NetEnt_Actor_Predator:
	case E_NetEnt_Actor_Prey:
	case E_NetEnt_ActorCmp_Health:
		{
			netGame::C_GameWorld* pxGameWorld = netGame::C_GameWorld::GetInstance();
			if(!pxGameWorld) return false;
			int uLocalId = pxGameWorld->CreateActor(xStateInfo, bServer, uParentLocalHandle);
			if(uLocalId >= 0)
			{
				bSuccess = true;
			}
		}
		break;
	default:
		{
			bSuccess = C_Factory::CreateNetEntity(xStateInfo);
		}
	}

	assert(bSuccess);
	return bSuccess;
}

bool C_TestGameFactory::DestroyNetEntity2(const int uEntityId, int const uGameStateId, const bool bDestroyActor)
{
	C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uGameStateId);
	if(!pxGameState)
		return false;

	C_EntityState* pxEntityState = pxGameState->EntityState(uEntityId); assert(pxEntityState);
	if(!pxEntityState)
		return false;
	
	// this is used on clients when a netstate is destroyed on host 
	bool bActorDestroyed = false;
	if(bDestroyActor && pxEntityState)
	{
		switch(pxEntityState->Header().m_uTypeId)
		{
		case E_NetEnt_Actor_Predator:
		case E_NetEnt_Actor_Prey:
			{
				C_NetHandle handle(uEntityId, uGameStateId);
				C_GameNetEntity* pxGameEntity = FindGameEntity(handle);
				if(!pxGameEntity)
				{
					assert(!"error: no entity!");
					return false;
				}

				netGame::C_GameWorld* pxGameWorld = netGame::C_GameWorld::GetInstance();
				if(!pxGameWorld)
				{
					assert(pxGameWorld && "attempt to remove actor from gameworld, when gameworld already destroyed!");
					return false;
				}

				int uLocalHandle = GetLocalHandle(uEntityId);
				bActorDestroyed = pxGameWorld->DestroyActor(uLocalHandle, false);
			}
			break;
		case E_NetEnt_ActorCmp_Health: 	
			{	
				/* this is destroyed when parent entity is destroyed */	
				bActorDestroyed = true;
			}
			break; 
		default: 
			{
				return C_Factory::DestroyNetEntity2(uEntityId, uGameStateId, bDestroyActor);
			}
		}
	}

	// destroy net state
	pxGameState->RequestDeleteNetState(uEntityId);
	m_xGameNetEntity[uGameStateId].Erase(uEntityId);

	return bDestroyActor ? bActorDestroyed : true;
}


// app
bool C_TestGameFactory::DistributeEntity(const int uClientId, const int uMyHostId, const int uType, const int uNum)
{
	netGame::C_GameWorld* pxGameWorld = netGame::C_GameWorld::GetInstance();
	if(!pxGameWorld)
		return false;

	for(int i = 0; i < uNum; ++i)
	{
		pxGameWorld->CreateActor(C_Header(g_sNetIdAutoAssign, E_NetEnt_Actor_Predator, uClientId, E_NetState_PeerBroadcast, g_sCalculateSize, g_sNoParent), true);
		//CreateNetEntity(C_Header(g_sNetIdAutoAssign, E_NetEnt_Actor_Predator, uClientId, E_NetState_PeerBroadcast));
	}
	return true;
}

/*
bool C_TestGameFactory::SerialiseGameActor(class netStream* stream, C_GameNetEntity& xGameNetEntity)
{
	netGame::C_GameWorld* pxGameWorld = netGame::C_GameWorld::GetInstance();
	if(!pxGameWorld)
		return false;

	int uLocalHandle = GetLocalHandle(xGameNetEntity.NetHandle().NetId());
	netGame::C_GameActor* pxActor = pxGameWorld->GetActor(uLocalHandle);
	if(!pxActor) 
		return false;
	
	return pxActor->serialise(stream);
}

bool C_TestGameFactory::SerialiseGameComponent(class netStream* stream, C_GameNetEntity& xGameNetEntity)
{
	netGame::C_GameWorld* pxGameWorld = netGame::C_GameWorld::GetInstance();
	if(!pxGameWorld)
		return false;

	int uLocalHandle = GetLocalHandle(xGameNetEntity.NetHandle().NetId());
	netGame::C_GameActor* pxActor = pxGameWorld->GetActor(uLocalHandle);
	if(!pxActor)
		return false;

	netGame::I_TestAppActor* pxCmp = pxActor->GetComponent(xGameNetEntity.Type());
	if(!pxCmp)
		return false;
	
	return pxCmp->serialise(stream);
}

bool C_TestGameFactory::DeSerialiseGameActor(class netStream* stream, C_GameNetEntity& xGameNetEntity)
{
	netGame::C_GameWorld* pxGameWorld = netGame::C_GameWorld::GetInstance();
	if(!pxGameWorld)
		return NULL;

	int uLocalHandle = GetLocalHandle(xGameNetEntity.NetHandle().NetId());
	netGame::C_GameActor* pxActor = pxGameWorld->GetActor(uLocalHandle);
	if(!pxActor) 
		return false;

	return pxActor->deserialise(stream);
}

bool C_TestGameFactory::DeSerialiseGameComponent(class netStream* stream, C_GameNetEntity& xGameNetEntity)
{
	netGame::C_GameWorld* pxGameWorld = netGame::C_GameWorld::GetInstance();
	if(!pxGameWorld)
		return NULL;

	int uLocalHandle = GetLocalHandle(xGameNetEntity.NetHandle().NetId());
	netGame::C_GameActor* pxActor = pxGameWorld->GetActor(uLocalHandle);
	if(!pxActor)
		return NULL;

	netGame::I_TestAppActor* pxCmp = pxActor->GetComponent(xGameNetEntity.Type());
	if(!pxCmp)
		return false;

	return pxCmp->deserialise(stream);
}
/**/

int	C_TestGameFactory::RelevantGameState(int uType)
{
	switch(uType)
	{
	case E_NetEnt_Actor_Predator:	return E_State_GameWorld;
	case E_NetEnt_Actor_Prey:		return E_State_GameWorld;
	case E_NetEnt_ActorCmp_Health:	return E_State_GameWorld;
	default:						return C_Factory::RelevantGameState(uType);
	}
}

int	C_TestGameFactory::RelevantNetStateType(int uType)
{
	switch(uType)
	{
	case E_NetEnt_Actor_Predator:	return E_NetState_PeerBroadcast;
	case E_NetEnt_Actor_Prey:		return E_NetState_ServerBroadcast;
	case E_NetEnt_ActorCmp_Health:	return E_NetState_ServerBroadcast;
	default:						return C_Factory::RelevantNetStateType(uType);
	}
}

// util
const char* C_TestGameFactory::NetEntityString(int eType)
{
	switch(eType)
	{
	case E_NetEnt_Actor_Predator:	return "ActPred";
	case E_NetEnt_Actor_Prey:		return "ActPrey";
	case E_NetEnt_ActorCmp_Health:	return "ActHeal";
	default:						return C_Factory::NetEntityString(eType);
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif