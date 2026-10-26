#ifndef ACHIEVEMENT_DATA_H
#define ACHIEVEMENT_DATA_H

#include <xt/xtgame.h>

#include <GameLogic/GameData/GameDataDefs.h>
#include <GameLogic/GameData/MultiplayerData.h>
#include <Game\Achievements\AchievementEnums.h>
#include <ui\Graphics\UISpriteTextureParam.h>

class UI::C_TextureParam;
struct AmaxCity;

struct StickerData
{
	XtLocId								mDescription;
	XtLocId								mShortDescription;
	Game::BLUR_STICKER					mType;
	F32									mGoal; // e.g. "shunt __500__ cars" or (with mOneShot = true) "drive at __220__mph"
	bool								mOneShot; // if true, there is no "progress" to this achievement (e.g. hit 220mph)
	bool								mQuadTopLeft; // which quadrants of this achievement picture does this particular sticker occupy?
	bool								mQuadTopRight;
	bool								mQuadBottomLeft;
	bool								mQuadBottomRight;
};

struct AchievementData
{
	XtLocId								mName;
	Game::BLUR_ACHIEVEMENT				mType;
	Game::BLUR_ACHIEVEMENT_CATEGORY		mCategory;
	U8									mTextureIndex; // index along our texture sheet containing our icons
	bool								mSinglePlayer; // can we complete this in SP?
	bool								mMultiplayer; // can we complete this in MP?
	bool								mSplitScreen;
	bool								mDisableOnPS3;	// is this one excluded from the PS3 version?
	XtArray< StickerData >				mStickers;
};

struct MiscStickerData 
{
	XtHandle< AmaxRoute >			mRouteForCheckpointGoldWithoutDamage;

	XtHandle<MultiplayerChallenge>	mChallengeX;
	XtHandle<MultiplayerChallenge>	mChallengeY;

	F32								mMaxPhotoFinishWinTimeDifference;

	F32								mMaxTimeBetweenBargingGuyAndOffCliffScoringMessage;

	U8								mNumBoltsNeededToHitWhileAirbraking;

	F32								mMaxTimeForBombToStopIncomingAttack;
	F32								mMaxTimeForShieldPowerupToBeCloseRange;
	F32								mMaxTimeForPowerupHitAfterCriticalRepair;

	XtHandle< AmaxCity >			mSmokingOilfieldsCity;
	XtHandle< AmaxCity >			mGasStationCity;		
	XtHandle< AmaxCity >			mHollywoodHillsSignCity;
	XtHandle< AmaxCity >			mFireworksCity;

	F32								mMaxFireworksViewingAngle;
	XtHandle< AmaxVehicle >			mFireworksViewingVehicle;

	XtHandle< AmaxCity >			mClassAHighSpeedCity;
	XtHandle< AmaxCity >			mClassBHighSpeedCity;
	XtHandle< AmaxCity >			mClassCHighSpeedCity;
	XtHandle< AmaxCity >			mClassDHighSpeedCity;

	XtHandle< AmaxEvent >			mEventForShockLeonard;
	XtHandle< AmaxEvent >			mEventForSmashKhansRide;
	XtHandle< AmaxEvent >			mEventForNatalyaAvoidance;
	XtHandle< AmaxEvent >			mEventForCarverDamage;

	XtArray< XtHandle< AmaxCity > > mAllCitiesInTheGame;

	XtArray< XtHandle< Vehicle::XtPaintJob > > mAllPinkPaintJobs;

	S32								mSuccessfulPowerUpHitOnEveryCarTier;
	S32								mSuccessfulPowerUpHitOnEveryCarEvent;
	S32								mAvoidOwnShockTier;
	S32								mAvoidOwnShockEvent;

	F32								mTimeLobbedConsideredRecentlyComeToRest;

	//----------------------------------------------
};

struct AchievementsList : AmaxGameDataObj
{
	XtArray< XtHandle<AchievementData> >	mAchievements;
	XtHandle< MiscStickerData >			mMiscStickerData;
};

class C_AchievementData
{
public:
	void Init() { }

	void Update();
	MiscStickerData& GetMiscStickerData() {return *mAchievementsList[0]->mMiscStickerData.Object();}
	static C_AchievementData& Instance() {return s_instance;}
	AmaxGameData<AchievementsList*>		mAchievementsList;
private:
	static C_AchievementData s_instance;

	XtPack*	m_pAchievementPack;
};

#endif // ACHIEVEMENT_DATA_H
