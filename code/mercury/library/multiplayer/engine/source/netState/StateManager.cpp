#include "stdafx.h"
#include "statemanager.h"
#include "gamestate.h"
#include "netDebug/netDebug.h"
#include "netEnums/netEnums.h"

C_StateManager* C_StateManager::m_pxInstance = NULL;

C_StateManager::C_StateManager(const int uMaxGameStates) 
	: m_uMaxGameStates(uMaxGameStates)
{
	assert(!m_pxInstance && "only one state manager can be instantiated!" );
	if(m_pxInstance) 
		return;

	m_pxInstance = this;
	
	m_pxGameStates = new C_GameState*[m_uMaxGameStates];
	for(int i = 0; i < m_uMaxGameStates; ++i)
	{
		m_pxGameStates[i] = NULL;
	}
}

C_StateManager::~C_StateManager()
{
	m_pxInstance = NULL;
	
	if(m_pxGameStates)
	{
		for(int i = 0; i < m_uMaxGameStates; ++i)
		{
			if(m_pxGameStates[i])
			{
				delete m_pxGameStates[i];
				m_pxGameStates[i] = NULL;
			}
		}

		delete [] m_pxGameStates;
		m_pxGameStates = NULL;
	}
}

bool C_StateManager::FreeGameState(int uGameStateId)
{
	assert(uGameStateId >= 0 && uGameStateId < m_uMaxGameStates);
	assert(m_pxGameStates[uGameStateId] != NULL);

	if(m_pxGameStates[uGameStateId])
	{
		delete m_pxGameStates[uGameStateId];
		m_pxGameStates[uGameStateId] = NULL;
	}

	return true;
}

bool C_StateManager::CreateGameState(int uGameStateId, const int uMyHostId, const int uNumEntities, const int uNumClients)
{
	assert(uGameStateId >= 0 && uGameStateId < m_uMaxGameStates);
	assert(m_pxGameStates[uGameStateId] == NULL);

	m_pxGameStates[uGameStateId] = new C_GameState(uGameStateId, uMyHostId, uNumEntities, uNumClients);
	return true;
}

C_GameState* C_StateManager::GetGameState(const int i)
{
	if(i < 0 || i >= m_uMaxGameStates)
		return NULL;

	assert(m_pxGameStates);
	assert(m_pxGameStates[i]);
	return m_pxGameStates[i];
}

void C_StateManager::Update(bool bNetworkFrame)
{
	for(int i = 0; i < m_uMaxGameStates; ++i)
	{
		C_GameState* pxGameState = m_pxGameStates[i];
		if(!pxGameState)
			continue;
		
		pxGameState->Update(bNetworkFrame);			
	}
}

bool C_StateManager::GameStateSynchronised(int uGameStateId)
{
	if(!m_pxGameStates)
		return false;

	if(!m_pxGameStates[uGameStateId])
		return false;

	if(!m_pxGameStates[uGameStateId]->Synchronised())
		return false;

	return true;
}

bool C_StateManager::GameStatesSynchronised()
{
	for(int i = 0; i < m_uMaxGameStates; ++i)
	{
		if(!GameStateSynchronised(i))
			return false;		
	}

	return true;
}

int	C_StateManager::MyHostId()
{
	for(int i = 0; i < m_uMaxGameStates; ++i)
	{
		if(m_pxGameStates[i])
			return m_pxGameStates[i]->MyHostId();
	}
	assert(false);
	return -1;
}

bool C_StateManager::IsServer(int uGameState)
{
	C_GameState* pxGameState = GetGameState(uGameState);
	if(!pxGameState)
	{
		assert(false);
		return false;
	}

	return pxGameState->IsServer();
}

void C_StateManager::Render(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h)
{
	if(!m_pxGameStates)
		return;

	if(m_pxGameStates[netDebug::dbg_RenderGameState])
		m_pxGameStates[netDebug::dbg_RenderGameState]->Render(debugRenderPtr, x, y, w, h);
}