#include "stdafx.h"
#include "netHandle.h"
#include "netstate/gameState.h"
#include "netstate/stateManager.h"

// C_NetHandle
C_NetHandle::C_NetHandle(int uNetId, int uGameStateId)
	: m_uNetId(uNetId)
	, m_uGameStateId(uGameStateId)
{
	;
}

C_EntityState* C_NetHandle::GetEntityState()
{ 
	C_GameState* pxGameState = C_StateManager::GetInstance()->GetGameState(m_uGameStateId); 
	if(!pxGameState)
	{
		assert(!"fail!");
		return NULL;
	}

	return pxGameState->EntityState(m_uNetId);
}