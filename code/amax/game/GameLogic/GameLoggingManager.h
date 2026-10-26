#ifndef GAME_LOGGING_MANAGER_H
#define GAME_LOGGING_MANAGER_H

#include <btl/Containers/containers.h>

#include <GameLogic\Structures\LevelStateConstants.h>
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>

namespace Game
{

const int AMAX_GAME_LOG_VERSION=7;

#pragma pack(push, 1)

typedef struct AmaxGameLogHeader
{
	U64 mCommunityGroupID;

	U32 mVersion;
	U32 mRaceMode;		//points / elim / race / desctruction
	U32 mCity;
	U32 mRoute;
	U32 mNumCars;
	U32 mLaps;

	S32 mTimeLimit;

}AmaxGameLogHeader;

typedef struct AmaxGameVehicleLog
{
	WCHAR mCharacterString[32];

	U64 mSystemID;

	float mHealthRemaining;
	float mDistanceTravelled;

	S32 mLoadOut1;
	S32 mLoadOut2;
	S32 mLoadOut3;

	S32 mRank;
	S32 mLegendLevel;

	U32 mCarID;
	U32 mManufacturerID;
	U32 mTotalFansGained;

	U32 mNumPowerups;
	U16 mPowerupsUsed[E_PICKUP_TYPE_MAX];
	U16 mPowerUpsHit[E_PICKUP_TYPE_MAX];

	U8 mStartPosition;
	U8 mFinalPosition;
	U8 mFinalState;

	U8 pad;

}AmaxGameVehicleLog;

typedef struct AmaxGameLog
{
	AmaxGameLogHeader mHeader;
	AmaxGameVehicleLog mVehicles[MAX_LEVELSTATE_CARS];	//purposely not making this a Btl as it's a pure binary upload
}AmaxGameLog;

#pragma pack(pop)

class C_GameLoggingManager
{
public:

	C_GameLoggingManager()
	{
	};

	void UploadGameLog();

	void DebugExportEnumNames();

	static C_GameLoggingManager& Instance(){return s_instance;};

private:

	static C_GameLoggingManager s_instance;

	AmaxGameLog mGameLog;
};

} // namespace Game

#endif // GAME_LOGGING_MANAGER_H