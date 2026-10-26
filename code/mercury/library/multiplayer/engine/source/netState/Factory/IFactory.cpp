#include "stdafx.h"
#include "IFactory.h"
#include "netstate/stateManager.h"
#include "netState/gameState.h"
#include "netState/netEntity/entityHeader.h"
#include "netstate/netHandle/netHandle.h"
#include "netstate/netHandle/netEntityHandle.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

I_Factory*	I_Factory::m_pxInstance = NULL;

I_Factory::I_Factory(int uGameStates)
	: m_uNumGameStates(uGameStates)
{
	assert(!m_pxInstance && "only one factory should be instantiated!");
	m_pxInstance = this;

	assert(uGameStates > 0);
	m_xGameNetEntity = new NetEntityList[uGameStates];
}

I_Factory::~I_Factory()
{
	m_pxInstance = NULL;

	if(m_xGameNetEntity)
		delete [] m_xGameNetEntity;

	m_xGameNetEntity = NULL;
}

bool I_Factory::CreatedGameObject(int uType, int uAuthority, int uDesiredNetId, int uSize)
{
	C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(RelevantGameState(uType)); 
	if(!pxGameState)
		return false;

	// peak at net id
	int uNetId = GetPredeterminedNetId(uDesiredNetId, uType);

	// create net entity adapter
	C_GameNetEntity* pxGameEntity = CreateNetEntityAdapter(uType, uAuthority, uNetId);
	if(!pxGameEntity) 
	{
		assert(false);
		return false;
	}

	// create net-state
	if(uSize == -1) uSize = pxGameEntity->CalculateSize();
	C_Header xStateInfo(uNetId, uType, uAuthority, RelevantNetStateType(uType), uSize);
	return pxGameState->CreateNetState(xStateInfo, pxGameEntity, uSize);
}

bool I_Factory::CreateGameObject(C_Header& xStateInfo)
{
	int uGameStateId = RelevantGameState(xStateInfo.m_uTypeId);
	C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uGameStateId); 
	if(!pxGameState)
		return false;

	const int uEntityId		= xStateInfo.m_uEntityId;
	const int uEntitySize	= xStateInfo.m_uSize;
	const int uType			= xStateInfo.m_uTypeId;
	const char uAuthorityId = xStateInfo.m_uAuthorityId;
	const char eStateType	= xStateInfo.NetStateType();
	const int uServerId		= pxGameState->ServerId();
	const bool bServer		= pxGameState->IsServer();
	const bool bAuthority	= uAuthorityId == pxGameState->MyHostId();
	
	if(bServer)
	{
		// we don't create actors via the netFactory, instead we notify the factory when an onject is created
		// the net system creates actors via the netFactory on clients.
		assert(!"don't create actors via the netFactory directly!");
		return false;
	}

	if(uEntityId == g_sNetIdAutoAssign)
	{
		assert(!"attempt to auto assign a netId?");
		return false;
	}

	// create game-side object
	bool bSuccess = CreateGameSideObject(xStateInfo, uGameStateId);

	if(bSuccess)
	{
		// create net entity adapter
		C_GameNetEntity* pxGameEntity = CreateNetEntityAdapter(uType, uAuthorityId, uEntityId);
		if(!pxGameEntity)
		{
			assert(false);
			return false;
		}
	}

	return bSuccess;
}

bool I_Factory::DestroyNetEntity(const int uNetId, int const uTypeId, const bool bDestroyActor)
{
	int uGameStateId = RelevantGameState(uTypeId);
	C_NetHandle handle(uNetId, uGameStateId);
	C_GameNetEntity* pxGameEntity = FindGameEntity(handle);
	if(!pxGameEntity)
		return false;

	return DestroyNetEntity2(pxGameEntity->NetHandle().NetId(), RelevantGameState(uTypeId), bDestroyActor);
}

bool I_Factory::DestroyNetEntity2(const int uEntityId, int const uGameStateId, const bool bDestroyActor)
{
	C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uGameStateId);
	if(!pxGameState)
		return false;

	C_EntityState* pxEntityState = pxGameState->EntityState(uEntityId); assert(pxEntityState);
	if(!pxEntityState)
		return false;
	
	// destroy actor
	bool bSuccess = true;
	
	if(bDestroyActor)
	{
		C_NetHandle xNetHandle(uEntityId, uGameStateId);
		DestroyGameSideObject(xNetHandle);
	}

	// destroy net state
	bSuccess &= pxGameState->RequestDeleteNetState(uEntityId);
	if(bSuccess)
	{
		RemoveNetEntityAdapter(C_NetHandle(uEntityId, uGameStateId));
	}

	assert(bSuccess);
	return bDestroyActor ? bSuccess : true;
}

C_GameNetEntity* I_Factory::CreateNetEntityAdapter(int uType, int uAuthority, int uDesiredNetId)
{
	int uGameStateId = RelevantGameState(uType);
	C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uGameStateId); 
	if(!pxGameState)
		return NULL;

	int	uFreeNetId = pxGameState->GetFreeNetId(uDesiredNetId);
	if(uFreeNetId != uDesiredNetId)
	{
		assert(!"C_SessionFactory::CreateNetEntityAdapter() - desired netId is unavailable!");
		return NULL;
	}

	C_NetHandle	xNetHandle(uDesiredNetId, uGameStateId);
	C_GameNetEntity* pxGameEntity = new C_GameNetEntity(xNetHandle, uType);
	m_xGameNetEntity[uGameStateId].Insert(xNetHandle.NetId(), pxGameEntity);
	return pxGameEntity;
}

void I_Factory::RemoveNetEntityAdapter(const C_NetHandle& xNetHandle)
{
	assert(xNetHandle.GameStateId() < m_uNumGameStates);

	m_xGameNetEntity[xNetHandle.GameStateId()].Erase(xNetHandle.NetId());
}

int I_Factory::GetPredeterminedNetId(int uDesiredNetId, int uType)
{
	int uGameStateId = RelevantGameState(uType);
	C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uGameStateId); 
	if(!pxGameState)
		return false;

	return pxGameState->GetFreeNetId(uDesiredNetId);
}

bool I_Factory::Authority(int uHostId, C_NetHandle& xNetHandle)
{
	int uNetId = xNetHandle.NetId();
	int uGameStateId = xNetHandle.GameStateId();

	if(uGameStateId < 0 || uGameStateId >= m_uNumGameStates)
		return false;

	NetEntityList::Iterator xEntity = m_xGameNetEntity[uGameStateId].Find(uNetId);
	if(xEntity == m_xGameNetEntity[uGameStateId].End())
		return false;

	C_GameNetEntity& pxGameNetEntity = *xEntity->GetValue();	
	C_EntityState* pxNetState = xNetHandle.GetEntityState();
	if(!pxNetState)
	{
		assert(!"fail!");
		return false;
	}

	if(pxNetState->Header().m_uAuthorityId != uHostId)
		return false;

	return true;
}

C_GameNetEntity* I_Factory::FindGameEntity(const C_NetHandle& xNetHandle) const
{
	int uNetId = xNetHandle.NetId();
	int uGameStateId = xNetHandle.GameStateId();

	if(uGameStateId < 0 || uGameStateId >= m_uNumGameStates)
		return NULL;

	NetEntityList::Iterator xNetEntity = m_xGameNetEntity[uGameStateId].Find(uNetId);
	if(xNetEntity == m_xGameNetEntity[uGameStateId].End())
		return NULL;

	return xNetEntity->GetValue();
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif