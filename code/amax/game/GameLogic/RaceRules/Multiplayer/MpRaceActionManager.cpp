#include "stdafx.h"

#include <GameLogic\RaceRules\Multiplayer\MpRaceActionManager.h>
#include <GameApp.h>
#include <Game/Game.h>


namespace Game
{


void C_MpRaceActionManager::ClearActions()
{
	m_RaceActions.Clear();
}

void C_MpRaceActionManager::Update()
{
	F32 deltaTime = C_GameApp::GetInstance()->GetTimeStepRaw();

	for(int i = 0; i < m_RaceActions.Size(); i++)
	{
		m_RaceActions[i]->UpdateTimer(deltaTime);
	}
}

void C_MpRaceActionManager::AddRaceAction( C_MpRaceActionBase* p_Action )
{
	m_RaceActions.PushBack(p_Action);
}
} // namespace Game