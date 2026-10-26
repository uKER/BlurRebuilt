#include "stdafx.h"

#include <Game/Game.h>

#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <GameLogic/RaceModes/RaceModeDefines.h>
#include <GameLogic/RaceModes/RaceModeStreetRace.h>
#include <GameLogic/RaceModes/RaceModeFriendDemandStreetRace.h>

#include <GameLogic\FriendsChallenges\FriendChallenges.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <Game\Messaging\GameflowMessages.h>

namespace Game
{

void C_RaceModeFriendDemandStreetRace::Initialise(Game::C_LevelState *pLevelState)
{
	C_RaceModeStreetRace::Initialise(pLevelState);

	SetInternalNameAndID("FriendDemand_StreetRace",E_RaceMode_FriendDemand_StreetRace);
}

void C_RaceModeFriendDemandStreetRace::Update()
{
	if(!mRaceInProgress)
		return;

	//work out if we're passing / failing or failed completely
	const FriendChallenge *fc=C_FriendChallenges::Instance().GetAttemptingFriendDemand();






	C_RaceModeStreetRace::Update();
}


void C_RaceModeFriendDemandStreetRace::StartLevel()
{
	C_RaceModeStreetRace::StartLevel();
}

void C_RaceModeFriendDemandStreetRace::StartRace()
{
	C_RaceModeStreetRace::StartRace();
}

void C_RaceModeFriendDemandStreetRace::UpdateHUD()
{
	if(!mRaceInProgress)
		return;

	/*
	const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(0);

	//work out if we're passing / failing or failed completely
	FriendChallenge *fc=C_FriendChallenges::Instance().GetAttemptingFriendDemand();
	int index=C_FriendChallenges::Instance().GetAttemptingFriendDemandIndex();

	UI::C_WideString128 heading;
	heading.Format(L"%s's \"%s\"", fc->mChallenger, fc->mName);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(68.0f, 158.0f), heading.AsString(), Maths::RGBA(0xff,0xff,0x00,0xff), 24.0f);

	//medal to beat
	UI::C_WideString128 medal;
	if(fc->mMedal == kSP_Gold)
		medal.Format(L"Get Gold Medal");
	else if(fc->mMedal == kSP_Silver)
		medal.Format(L"Get Silver Medal or Better");
	else if(fc->mMedal == kSP_Bronze)
		medal.Format(L"Get Bronze Medal or Better");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(68.0f, 178.0f), medal.AsString(), Maths::RGBA(0xff,0xff,0xff,0xff), 20.0f);

	UI::C_WideString128 medal1;
	int position = pLogicActor->GetPositionInfo().mRacePosition;
	if (position <= 1)
		medal1.Format(L"Current Medal: Gold");
	else if (position <= 2)
		medal1.Format(L"Current Medal: Silver");
	else if (position <= 3)
		medal1.Format(L"Current Medal: Bronze");
	else
		medal1.Format(L"Current Medal: None");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(84.0f, 178.0f + 24.f), medal1.AsString(), Maths::RGBA(0xff,0xff,0xff,0xff), 20.0f);

	//fans to beat
	UI::C_WideString128 fans;
	fans.Format(L"Beat %d Fans", fc->mU32Criteria);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(68.0f, 178.0f + 48.f), fans.AsString(), Maths::RGBA(0xff,0xff,0xff,0xff), 20.0f);

	UI::C_WideString16 result;
	FRIEND_CHALLENGE_RESULT_TYPE res=C_FriendChallenges::Instance().AttemptingCriteriaResult();
	if(res == FRIEND_CHALLENGE_RESULT_TYPE_PASSED)
		result.Format(L"Passed");
	if(res == FRIEND_CHALLENGE_RESULT_TYPE_PASSING)
		result.Format(L"Passing");
	if(res == FRIEND_CHALLENGE_RESULT_TYPE_FAILED)
		result.Format(L"Failed");
	if(res == FRIEND_CHALLENGE_RESULT_TYPE_FAILING)
		result.Format(L"Failing");

	const VehicleScoringInfo& scoring = pLogicActor->GetScoring();
	UI::C_WideString128 fans1;
	fans1.Format(L"Current Fans: %d (%s)", scoring.mFinalScoreForUI, result.Chars() );
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(84.0f, 178.0f + 68.f), fans1.AsString(), Maths::RGBA(0xff,0xff,0xff,0xff), 20.0f);

	//param 1
	UI::C_WideString128 param1;
	UI::C_WideString128 param1_1(C_FriendChallenges::Instance().GetParamTitleString(0, index, NULL));
	UI::C_WideString128 param1_2(C_FriendChallenges::Instance().GetParamSwitchString(0, index, NULL));
	UI::C_WideString128 param1_3(C_FriendChallenges::Instance().GetParamVariableString(0, index, NULL));
	param1.Format(L"%s %s %s", param1_1.Chars(), param1_2.Chars(), param1_3.Chars());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(68.0f, 178.0f + 92.f), param1.AsString(), Maths::RGBA(0xff,0xff,0xff,0xff), 20.0f);


	UI::C_WideString64 param2;

	res=C_FriendChallenges::Instance().AttemptingParamResult(&param2, true);

	if(res == FRIEND_CHALLENGE_RESULT_TYPE_PASSED)
		result.Format(L" (Passed)");
	if(res == FRIEND_CHALLENGE_RESULT_TYPE_PASSING)
		result.Format(L" (Passing)");
	if(res == FRIEND_CHALLENGE_RESULT_TYPE_FAILED)
		result.Format(L" (Failed)");
	if(res == FRIEND_CHALLENGE_RESULT_TYPE_FAILING)
		result.Format(L" (Failing)");

	param2.Add(result.Chars());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(84.0f, 178.0f + 112.f), param2.AsString(), Maths::RGBA(0xff,0xff,0xff,0xff), 20.0f);
	*/

	C_RaceModeStreetRace::UpdateHUD();
}

void C_RaceModeFriendDemandStreetRace::SetupResults()
{
	Game::C_LevelResult& results=C_Game::GetInstance()->GetLevelResult();
	Game::C_LevelState& state=C_Game::GetInstance()->GetLevelState();
	
	FRIEND_CHALLENGE_RESULT_TYPE medal_res=C_FriendChallenges::Instance().AttemptingMedalResult();
	FRIEND_CHALLENGE_RESULT_TYPE criteria_res=C_FriendChallenges::Instance().AttemptingCriteriaResult();
	FRIEND_CHALLENGE_RESULT_TYPE param_res=C_FriendChallenges::Instance().AttemptingParamResult();

	if(medal_res >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING && criteria_res >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING && param_res >= FRIEND_CHALLENGE_RESULT_TYPE_PASSING)
	{
		results.mFriendDemandComplete=1;

		if(C_FriendChallenges::Instance().GetAttemptingFriendDemand()->mAttempts == 1)
			results.mFriendDemandComplete=2;
	}
	else
	{
		results.mFriendDemandComplete=0;
	}

	C_FriendChallenges::Instance().FinishedAttempt(results.mFriendDemandComplete);
	C_RaceModeStreetRace::SetupResults();
}

bool C_RaceModeFriendDemandStreetRace::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	if(!ShouldHandleMessages())
		return false;

	/*
	//warning C4065: switch statement contains 'default' but no 'case' labels
	switch( pMessage->Type() )
	{
		default:
			break;
	}
	*/

	return C_RaceModeStreetRace::MessageHandler(pMessage);
}

} // namespace Game
