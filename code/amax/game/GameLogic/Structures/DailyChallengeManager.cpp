#include "stdafx.h"
#include "GameLogic/Structures/DailyChallengeManager.h"

// Includes
#include <UI/Game/Multiplayer/MultiplayerDefines.h>
#include <GameLogic/Profile/MultiplayerProfile.h>
#include <GameLogic/MultiplayerChallengeSystem.h>
#include <GameLogic/Profile/GameProfileManager.h>
#include <system\time\Time.h>
#include <Game\Messaging\GameflowMessages.h>
#include <ui/Game/Gadgets/ButtonMessages.h>

// demonware includes
#include <bdLobby/bdTitleUtilities/bdTitleUtilities.h>
#include <Network/Core.h>
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>

// Ps3 specific includes
#ifdef PLATFORM_PS3
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_sysparam.h>
#endif //#ifdef PLATFORM_PS3

// Daily challenge overrides
Debug::Bool g_SimulateDailyChallengeNewDay("Multiplayer.Challenges.SimulateDailyChallengeNewDay", false ); 

namespace Game
{

///////////////////////////////////////////
//		C_DailyChallengeManager          //
///////////////////////////////////////////

C_DailyChallengeManager::C_DailyChallengeManager():mpCurrentMultiplayerProfile(NULL),mTimeElapsedThisSession(0.0f),mSessionStartTimeStamp(0),mTimeStampObtainedFromServer(false),mLocalTimeAdjustmentInSecs(0)
{
	// Cache any timezone + Daylightsavings adjustment values
	CacheLocalTimeZoneAdjustments();
}

/////////////////////////////////////////////////////////
// [ JBUNNER ] : BUG (37583)						   //
// Adjust Demonware time by users local time zone	   //
// As daily challenges resetting at incorrect times    //
/////////////////////////////////////////////////////////

void  C_DailyChallengeManager::CacheLocalTimeZoneAdjustments()
{
	// Server time stamp is in UTC. Need to convert to local time zone. 
	// Init the var && Calc the adjust
	mLocalTimeAdjustmentInSecs = 0; 
	S32 timeZoneAdjustInMins   = 0;

	// For 360... //

	// For ps3 //
#ifdef PLATFORM_PS3

	int  temp;
	bool timeZoneOk		   = (cellSysutilGetSystemParamInt( CELL_SYSUTIL_SYSTEMPARAM_ID_TIMEZONE, &temp ) == CELL_OK);

	if(timeZoneOk)
	{
		timeZoneAdjustInMins  += temp;
	}

	// [ JBUNNER ] : BUG (37583) - TODO - currently unable to obtain daylight savings adjust on PS3... does cellSysutilGetSystemParamInt already include this??

	// Currently in minutes so convert to seconds. Here adjust is from UTC to Timezone.. so dont neeed to -ve it. 
	mLocalTimeAdjustmentInSecs = timeZoneAdjustInMins * 60; 

#endif // #ifdef PLATFORM_PS3

	// Check that our time adjust is actually sensible (i.e < =/- 24 hours) // Kiribati is gmt+14 !!
	BIZ_ASSERT_MSG(abs(mLocalTimeAdjustmentInSecs) <= 86400, "C_DailyChallengeManager::CacheLocalTimeZoneAdjustments() < LocalTimeAdjustmentInSecs > =/- 24hours" );   //86400 == #secs in day
}

bool  C_DailyChallengeManager::ConnectedToServer() const
{
	if(RacerNet::C_NetServices::Instance().Router().GetTitleUtilities())
	{
		return true;
	}
	else
	{
		return false; 
	}
}

bool C_DailyChallengeManager::ProcessingValid()
{
	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile(); // Gets profile based on primary controller(can change)
	if(pGameProfile)
	{
		if(ConnectedToServer())
		{
			// Cache the profile we are going to check
			mpCurrentMultiplayerProfile = pGameProfile->GetMultiplayerProfile(); //pMultiplayerProfile;
			BIZ_ASSERT_MSG(mpCurrentMultiplayerProfile,"C_DailyChallengeManager::UpdateIsValid() - mpCurrentMultiplayerProfile is NULL");

			// Check if we have the server time stamp for the session start
			if(!mTimeStampObtainedFromServer)
			{
				// If we dont have it.. can we get it yet?	
				U64 serverTimeStamp_UTC = RacerNet::C_NetServices::Instance().GetServerTimeStamp(); 
				if(serverTimeStamp_UTC != 0)
				{
					U64 timeStampLocal = ConvertUTCToLocal(serverTimeStamp_UTC);

					// We have the time stamp, good to go. 
					mSessionStartTimeStamp = timeStampLocal;
					mTimeStampObtainedFromServer = true;

					// time elapsed should count from now onwards
					mTimeElapsedThisSession = 0.0f;
					return true;
				}
			}
			else
			{
				// We already have the time stamp from the server, good to go. 
				return true;
			}
		}
		else
		{
			// If failed for any reason (timed out/ could not connect etc)
			if(RacerNet::C_NetServices::Instance().GetServerTimeTaskFailed())
			{
				// Cache profile
				mpCurrentMultiplayerProfile = pGameProfile->GetMultiplayerProfile(); //pMultiplayerProfile;
				BIZ_ASSERT_MSG(mpCurrentMultiplayerProfile,"C_DailyChallengeManager::UpdateIsValid() - mpCurrentMultiplayerProfile is NULL");

				// If this has happened.. then we need to fall back on the local system clock (sigh)
				mSessionStartTimeStamp =  ConvertUTCToLocal(static_cast<U64>(Time::GetCurrentTime() / 1000000));

				return true;
			}
		}
	}
	
	// Cant update (either not online or no multiplayer profile selected)
	return false; 
}

U64 C_DailyChallengeManager::ConvertUTCToLocal( U64 UTCTime )
{
	// Do the adjust
	return UTCTime + mLocalTimeAdjustmentInSecs;
}

void C_DailyChallengeManager::Update(float frameTime)
{
	// Always increment the frame time
	mTimeElapsedThisSession += frameTime; 

	// If online and valid profile.. process the challenges. 
	if(ProcessingValid())
	{
		ProcessDailyChallenges(); 
	}
}

void C_DailyChallengeManager::ProcessDailyChallenges()
{	
	U32 prevDayStamp    = GetStoredDayStamp();
	U32 currentDayStamp = GetCurrentDayStamp();

	S32 prevTimeZoneStamp = GetStoredTimeZoneAdjustStamp(); 

	if((prevDayStamp != currentDayStamp) || 
	   (mLocalTimeAdjustmentInSecs < prevTimeZoneStamp) || // [JBunner] BUG: 37791 - if new timezone is further behind than previous, force reset as user attempts to obtain MORE time to complete daily challenges
		g_SimulateDailyChallengeNewDay )
	{
		// then it is a new day... reset all!
		ResetAllDailyChallenges();

		// Store the new dayStamp
		SetStoredDayStamp(currentDayStamp);

		// Store the latest TimeZone stamp
		SetStoredTimeZoneAdjustStamp(mLocalTimeAdjustmentInSecs); 

		//Disarm debug flag if necessary
		if(g_SimulateDailyChallengeNewDay)
		{
			g_SimulateDailyChallengeNewDay = false;
		}
	}
}


void C_DailyChallengeManager::ResetAllDailyChallenges()
{
	Game::C_MultiplayerChallengeSystem&	pChallengeSystem  = mpCurrentMultiplayerProfile->GetChallengeSystem();

	// For each DAILY challenge call reset
	// NOTE: since daily enums are mixed with regular enums, no nice way to do this...

	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_OVERTAKE ).Reset();				
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_PARTICIPANT ).Reset();			
	pChallengeSystem.GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_HARD_DAYS_WORK ).Reset();			
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_DEMOLITIONS_EXPERT ).Reset();	
	pChallengeSystem.GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_HAT_TRICK ).Reset();				
	pChallengeSystem.GetChallenge(UI::E_MULTIPLAYER_CHALLENGE_TYPE_STYLE ).Reset();					
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_FACE_PLANT ).Reset();				
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_PIED_PIPER ).Reset();				
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HEAD_IN_THE_CLOUDS ).Reset();	
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_HOARDER ).Reset();		
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CHAIN_REACTION ).Reset();			
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_AGGRESSIVE_PERSONALITY ).Reset();	
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_DOMINATOR ).Reset();				
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_DISTANCE ).Reset();				
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_CAR_CRAZY ).Reset();				
	pChallengeSystem.GetChallenge( UI::E_MULTIPLAYER_CHALLENGE_TYPE_WELL_TRAVELLED ).Reset();

	// Also reset the variables we use to track daily challenges
	mpCurrentMultiplayerProfile->GetChallengeBlackboard().mVehiclesUsedToday = 0;
	mpCurrentMultiplayerProfile->GetChallengeBlackboard().mCountriesVisitedTodayBitField = 0;
	mpCurrentMultiplayerProfile->GetChallengeBlackboard().mSecondsRemainderInFirstPosition = 0.0f;

	//Inform the UI
	_UI_MESSAGE( Game::E_GameflowMessage_DailyChallengesReset );	
}

U32 C_DailyChallengeManager::GetStoredDayStamp() const
{
	Game::MiscellaneousChallengeBlackboardData&	pChallengeDataBlackboard = mpCurrentMultiplayerProfile->GetChallengeBlackboard();
	return pChallengeDataBlackboard.mDailyChallengeDayStamp;
}

void C_DailyChallengeManager::SetStoredDayStamp(const U32 dayStamp) 
{
	Game::MiscellaneousChallengeBlackboardData&	pChallengeDataBlackboard = mpCurrentMultiplayerProfile->GetChallengeBlackboard();
	pChallengeDataBlackboard.mDailyChallengeDayStamp = dayStamp;
}

S32 C_DailyChallengeManager::GetStoredTimeZoneAdjustStamp() const
{
	Game::MiscellaneousChallengeBlackboardData& pChallengeDataBlackboard = mpCurrentMultiplayerProfile->GetChallengeBlackboard();
	return pChallengeDataBlackboard.mDailyChallengeTimeZoneAdjustStamp;
}

void C_DailyChallengeManager::SetStoredTimeZoneAdjustStamp(const S32 timeZoneAdjust) 
{
	Game::MiscellaneousChallengeBlackboardData&	pChallengeDataBlackboard = mpCurrentMultiplayerProfile->GetChallengeBlackboard();
	pChallengeDataBlackboard.mDailyChallengeTimeZoneAdjustStamp = timeZoneAdjust;
}

U32 C_DailyChallengeManager::GetCurrentDayStamp() const
{
	// This generates a safe 'actual current' time stamp based on the real server reading on server start up
	// Plus the elapsed session time. 

	U64 timeElapsedThisSession = (U64)(mTimeElapsedThisSession); // convert from floating point seconds to 'whole' seconds
	U64 currentTimeStamp = mSessionStartTimeStamp + timeElapsedThisSession; 

	const U64 secondsInDay = 86400;
	U32 wholeDaysElapsed = (U32)(currentTimeStamp / secondsInDay);

	return wholeDaysElapsed; 
}

}// end namespace Game
