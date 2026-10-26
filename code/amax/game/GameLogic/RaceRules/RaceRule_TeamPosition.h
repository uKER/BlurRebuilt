#ifndef RACE_RULE_TEAM_POSITION_H
#define RACE_RULE_TEAM_POSITION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\Layers\RaceLogicLayer.h>
#include <GameLogic\RaceRules\RaceRuleBase.h>

namespace Game
{

	class RaceModeRule_TeamPosition : public RaceModeRuleBase
	{
	public:
		RaceModeRule_TeamPosition() 
		{
			for( int i = 0; i < Game::MAX_RACELOGIC_VEHICLES; i++ )
			{
				mTeamPoints[i] = 0;
				mWinning[i] = false;
			}
		};

		bool IsTriggered(int vehicleIndex);
		RaceModeRuleType GetType() { return E_RaceRule_TeamPosition; };
		virtual void PreCache();

		void GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);

	private:
		Btl::Vector<int, Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES> > mTeamPoints;
		Btl::Vector<int, Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES> > mWinning;
	};

} // namespace Game

#endif // RACE_RULE_TEAM_POSITION_H
