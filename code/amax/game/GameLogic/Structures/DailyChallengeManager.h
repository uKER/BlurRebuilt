#ifndef DAILY_CHALLENGE_MANAGER_H
#define DAILY_CHALLENGE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

// Forward declarations
namespace Game
{
	class C_MultiplayerChallengeSystem;
	class MiscellaneousChallengeBlackboardData;
	class C_MultiplayerProfile;
}

namespace Game
{
	// Daily challenges are treated in the code just as every other (non daily) challenge. 
	// This class provides the 'Daily' behaviour. Time stamps are checked during this manager's
	// update and challenge progress is reset if necessary. 

	class C_DailyChallengeManager
	{
	public:
		C_DailyChallengeManager();

		void Update(float frameTime); // This checks the daily challenge date stamp and resets if they have expired

	private:

		// functionality // 
		void CacheLocalTimeZoneAdjustments(); 

		bool ProcessingValid(); 
		bool ConnectedToServer() const;

		void ProcessDailyChallenges();  
		void ResetAllDailyChallenges();
		
		U64	 ConvertUTCToLocal( U64 UTCTime );
		U32  GetCurrentDayStamp() const;
		U32  GetStoredDayStamp() const;
		void SetStoredDayStamp(const U32 dayStamp); 

		S32  GetStoredTimeZoneAdjustStamp() const;
		void SetStoredTimeZoneAdjustStamp(const S32 timeZoneAdjust); 

		// data //

		Game::C_MultiplayerProfile* mpCurrentMultiplayerProfile;
		float mTimeElapsedThisSession;    //Time elapsed in seconds
		U64 mSessionStartTimeStamp; //Time session started in seconds (obtained from online server)
		bool mTimeStampObtainedFromServer;
		S32  mLocalTimeAdjustmentInSecs;
	};

}// end namespace Game

#endif //#ifndef DAILY_CHALLENGE_MANAGER_H
