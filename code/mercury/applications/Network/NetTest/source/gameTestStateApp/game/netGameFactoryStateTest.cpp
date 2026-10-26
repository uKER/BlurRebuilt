#include "stdafx.h"
#include "netGameFactoryStateTest.h"
#include "netState/gameState.h"
#include "netState/stateManager.h"

extern int g_uGameStateToCreateEntitiesIn;

bool C_StateTestFactory::CreateNetEntity(C_Header& xStateInfo) 
{
	//assert(!"WRONG! how do we specify a game-state id?");
	bool bSuccess = false;

	switch(xStateInfo.m_uTypeId)
	{
	case E_NetEnt_Blank:
		{
			bSuccess = C_Factory::FactoryActorCreated(xStateInfo.m_uTypeId, 0, xStateInfo.m_uAuthorityId, xStateInfo.m_uEntityId, g_uDefaultEntitySize);
		}
		break;
	default:	assert(!"unknown type");
	}

	return bSuccess; 
}

bool C_StateTestFactory::DestroyNetEntity(const int uEntityId, const int uGameStateId, const bool bDestroyActor) 
{ 
	C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(uGameStateId); 
	if(!pxGameState)
		return false;

	C_EntityState* pxEntityState = pxGameState->EntityState(uEntityId); assert(pxEntityState);
	if(!pxEntityState)
		return false;

	const C_Header& xHeader = pxEntityState->Header();

	bool bSuccess = false;
	switch(xHeader.m_uTypeId)
	{
	case E_NetEnt_Blank:
		{
			bSuccess = true;
		}
		break;
	default:	assert(!"unknown type");
	}

	bSuccess &= pxGameState->RequestDeleteNetState(uEntityId);

	return bSuccess; 
}

int	C_StateTestFactory::RelevantGameState(int uType)
{
	assert(g_uGameStateToCreateEntitiesIn >= 0);

	switch(uType)
	{
	case E_NetEnt_Blank:	return g_uGameStateToCreateEntitiesIn;
	default:				return C_Factory::RelevantGameState(uType);
	}
}

int	C_StateTestFactory::RelevantNetStateType(int uType)
{
	switch(uType)
	{
	case E_NetEnt_Blank:	return E_NetState_PeerBroadcast;
	default:				return C_Factory::RelevantNetStateType(uType);
	}
}

// util
const char* C_StateTestFactory::NetEntityString(int eEntity)
{
	switch(eEntity)
	{
	case E_NetEnt_Blank:	return "Blank";
	default:				return "Unknown";
	}
}

const char* C_StateTestFactory::GameStateString(int eGameState)
{
	switch(eGameState)
	{
	case 0:		return "Host 0";
	case 1:		return "Host 1";
	case 2:		return "Host 2";
	case 3:		return "Host 3";
	case 4:		return "Host 4";
	case 5:		return "Host 5";
	case 6:		return "Host 6";
	case 7:		return "Host 7";
	default:	return "Unknown";
	}
}