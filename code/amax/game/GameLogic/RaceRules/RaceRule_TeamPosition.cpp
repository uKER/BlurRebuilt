#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic\RaceRules\RaceRule_TeamPosition.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/GameComponents.h>

#include <ui/Game/UILocaliser.h>
#include <AI\AI_Amax.h>

namespace Game
{

	bool RaceModeRule_TeamPosition::IsTriggered(int vehicleIndex)
	{
		Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		airacingtech::AIEventSegmentData* myBalancingData = AIAmax::AmaxAIR::Instance().GetEventBalancingSegmentData(vehicleIndex);

		if(!myBalancingData)
			return false;

		mWinning[vehicleIndex] = false;

		int teamPoints = mTeamPoints[myBalancingData->m_Group];
		for( int i = 0; i < pLayer->GetNumVehicles(); i++ )
		{
			if( !myBalancingData || myBalancingData->m_Group == i)
				continue;

			if( teamPoints < mTeamPoints[i] )
				return false;
		}

		mWinning[vehicleIndex] = true;

		return true;
	}

	void RaceModeRule_TeamPosition::PreCache()
	{
		Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		int numVehicles = pLayer->GetNumVehicles();

		for( int i = 0; i < numVehicles; i++ )
			mTeamPoints[i] = 0;

		for( int i = 0; i < numVehicles; i++ )
		{
			int score = numVehicles - pLayer->GetLogicVehicleActor(i)->GetPositionInfo().mRacePosition;
			airacingtech::AIEventSegmentData* balancingData = AIAmax::AmaxAIR::Instance().GetEventBalancingSegmentData(i);

			if( balancingData && balancingData->m_Group >= 0 && balancingData->m_Group < 20 )
				mTeamPoints[balancingData->m_Group] += (score + 1);
		}
	}

	void RaceModeRule_TeamPosition::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
	{
		switch(stringType)
		{
		case RACE_MODE_RULE_STRING_TYPE_RESULTS:
		case RACE_MODE_RULE_STRING_TYPE_HUD_TARGET:
			{
				static UI::C_TextStringID string("RR_TeamVictory_HUD_Current");
				str.Format(string.String());
			}
			break;

		case RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT:
			{
				static UI::C_TextStringID stringWin("RR_TeamVictory_HUD_Current_Win");
				static UI::C_TextStringID stringLose("RR_TeamVictory_HUD_Current_Lose");

				str.Format(mWinning[idx] ? stringWin.String(): stringLose.String());
			}
			break;
		}
	}

} // namespace Game