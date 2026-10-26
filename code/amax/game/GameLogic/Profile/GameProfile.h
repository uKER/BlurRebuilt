#ifndef GAMEPROFILE_H
#define GAMEPROFILE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/Structures/LevelState.h>
#include <GameLogic/Profile/MultiplayerProfile.h>
#include <GameLogic/Profile/MultiplayerProfileStorage.h>
#include <GameLogic/GameData/GameEventData.h>

struct AmaxEvent;

namespace RaceBook
{
	class C_RaceBookProfile;
}

namespace Game
{

typedef Btl::Pair< XtId, S32 >			ResprayData;
typedef Btl::Map< XtId, ResprayData >	ResprayMap;

enum eSPVideoConfig
{
	VIDEO_GAME_MODE_RACE = 1,
	VIDEO_GAME_MODE_DESTRUCTION = 1 << 1,
	VIDEO_GAME_MODE_CHECKPOINT = 1 << 2,
	VIDEO_GAME_MODE_FAN = 1 << 3,
	VIDEO_GAME_MODE_ONEONONE = 1 << 4,
	VIDEO_TUTORIAL_1 = 1 << 5,
	VIDEO_TUTORIAL_2 = 1 << 6,
	VIDEO_FAN_PAR = 1 << 7,
	VIDEO_FAN_RUN = 1 << 8,
	VIDEO_FAN_DEMAND = 1 << 9,
	VIDEO_TIER_UNLOCKS = 1 << 10,
	VIDEO_BOSS_UNLOCKS = 1 << 11,
	VIDEO_BOSS_BEATEN = 1 << 12,
	VIDEO_END_OF_GAME = 1 << 13,
	VIDEO_END_PHONECALL = 1 << 14,
	VIDEO_GLOSSY_INTRO = 1 << 15,
	VIDEO_POWER_UPS = 1 << 16,
};

//enum GAME_DIFFICULTY
//{
//	GAME_DIFFICULTY_EASY,
//	GAME_DIFFICULTY_NORMAL,
//	GAME_DIFFICULTY_HARD,
//	GAME_DIFFICULTY_EXPERT,
//};

class ProfileOptions
{
public:
	void Init();
	void SendToLua( lua_State* pState );
	void SendDefaultsToLua( lua_State* pState );
	void GetFromLua( lua_State* pState );
#ifdef PLATFORM_PC
	void SendKeyboardToLua( lua_State* pState );
	void GetKeyboardFromLua( lua_State* pState );
	void InitKeyboard();
	//! [Blur2PC] T144: fills the audio quality bytes of options read from a 1373/1374 save (see GameProfile.cpp)
	void InsertAudioQualityIntoLegacyOptions();
	//! [Blur2PC] T250: reads the options of a save older than 1378 (Blur2PC's own 0x44/0x7c-byte layouts)
	void ReadLegacyOptions( Sys::BinaryDeserialiserAutoEndian& in, int version );
#endif

	U8 mGameDifficulty;
	//U8 mSteeringAssist;
	//U8 mBrakingAssist;
	U8 mAutoPickup;
	U8 mControlIndex;
	U8 mCameraType;
#ifdef PLATFORM_PC
	U8 mAudioQuality;		// [Blur2PC] T144: retail PC +0x04, "quality" (0 low, 1 medium, 2 high)
#endif
	U8 mAudioSfxVolume;
	U8 mAudioMusicType;
	U8 mAudioMusicVolume;
	U8 mAudioVoiceVolume;
	U8 mAudioMenuMusicVolume;
	U8 mAudioOutputType;
	U8 mAudioRoomSize;
	U8 mAudioDynamicRange;
	U8 mAudioMusicMode;
	U8 mAudiLFEeBoost;
	U8 mGammaIndex;
	F32 mAudioSpeakerAngleFL;
	F32 mAudioSpeakerAngleFR;
	F32 mAudioSpeakerAngleRR;
	F32 mAudioSpeakerAngleRL;

	//store defaults separately so we may retreive them
	U8 mDefaultGameDifficulty;
	//U8 mSteeringAssist;
	//U8 mBrakingAssist;
	U8 mDefaultAutoPickup;
	U8 mDefaultControlIndex;
	U8 mDefaultCameraType;
#ifdef PLATFORM_PC
	U8 mDefaultAudioQuality;	// [Blur2PC] T144: retail PC +0x24
#endif
	U8 mDefaultAudioSfxVolume;
	U8 mDefaultAudioMusicType;
	U8 mDefaultAudioMusicVolume;
	U8 mDefaultAudioVoiceVolume;
	U8 mDefaultAudioMenuMusicVolume;
	U8 mDefaultAudioOutputType;
	U8 mDefaultAudioRoomSize;
	U8 mDefaultAudioDynamicRange;
	U8 mDefaultAudioMusicMode;
	U8 mDefaultAudiLFEeBoost;
	U8 mDefaultGammaIndex;
	F32 mDefaultAudioSpeakerAngleFL;
	F32 mDefaultAudioSpeakerAngleFR;
	F32 mDefaultAudioSpeakerAngleRR;
	F32 mDefaultAudioSpeakerAngleRL;

	// Multiplayer options
	S8 mMatchingRule;

#ifdef PLATFORM_PC
	// [Blur2PC] T250: retail Blur PC (Blur.exe 1.2) saves ProfileOptions whole at 0x170 bytes, so every PC field
	// sits at its retail offset (static_asserts in GameProfile.cpp).
	// Online login strings, +0x41 and +0x61 (recovered names): retail reads them through two getters
	// (0x00b9ff00, 0x00b9ff40) that return NULL for an empty string; Lua Amax "OnlineConnectionPossible"
	// (0x00c16820) needs both, "Connect" (0x00c16880) passes both to the online service. Init empties them.
	char mOnlineLoginName[0x20];
	char mOnlineLoginPassword[0x63];

	// [Blur2PC] Retail Blur PC keyboard options (Blur.exe 1.2 ProfileOptions +0xc4..+0xf8 and +0x16d, defaults
	// from ProfileOptions::Init 0x00b9e790; saved with the profile). The keys are C_Keyboard enum values of the
	// custom layout, which retail's UI never offers; mKeyboardLayout is the layout in use (0 or 1).
	S8 mKeyboardIsCustom;
	S32 mKeyThrottle;
	S32 mKeyBrake;
	S32 mKeyEBrake;
	S32 mKeyFire;
	S32 mKeySwitchPerk;
	S32 mKeyDrop;
	S32 mKeyChangeView;
	S32 mKeyPause;
	S32 mKeyRearView;
	S32 mKeyPowerUpFwd;
	S32 mKeyPowerUpBwd;
	S32 mKeySteerLeft;
	S32 mKeySteerRight;
	// [Blur2PC] T250: +0xfc..+0x16c: no retail code reads or writes these bytes (searched for every offset as a
	// profile and an options displacement); they round-trip through the save unchanged. Unknown.
	U8 mRetailUnknownFC[0x71];
	S8 mKeyboardLayout;
#endif
private:
	enum E_AudioOutputType
	{
		E_AudioOutputType_Stereo = 0,
		E_AudioOutputType_Surround,
	};

	static E_AudioOutputType GetDefaultAudioOutputType();
};

struct RaceEventHistory
{
	U8 mDifficulty;
	int mFans;
	int mHighestCombo;
	int mFanDemandsComplete;
	U32 mVehicleUsed;
};

// ----------------------------------------------------------------

const int MAX_PROFILE_VOTE_BUFFER=100;

class C_GameProfile
{
public:
	C_GameProfile();

	void Init(int controller_index);
	void ReloadSorceryData();
	void UploadPlayerStats();
	void RetryingEvent();
	
	void SetSystemDefaults();

	U8 GetDifficulty();
	void SetDifficulty(U8 diff,bool just_for_this_race=false);
	void ClearRaceDifficulty();

	AmaxEvent* GetCurrentEvent() { return C_GameEventData::Instance().mEvents[(GAMEDATA_ID)mCurrentEventId]; };

	void ResprayAdd( const XtId amaxVehicleId, const XtId resprayId, const S32 sResprayIndex, const bool bMultiplayer = false );
	void ResprayRemove( const XtId amaxVehicleId, const bool bMultiplayer = false );
	XtId ResprayGet( const XtId amaxVehicleId, const bool bMultiplayer = false );

	void CacheDLCData();

	void Serialise( Sys::BinarySerialiserAutoEndian& stream );
	void Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	C_ProfileBase*					GetCurrentInGameProfile();
	C_MultiplayerProfile*			GetMultiplayerProfile()
									{
										return &mMultiplayerProfile;
									}

	C_MultiplayerProfile			mMultiplayerProfile;
	RaceBook::C_RaceBookProfile*	mRaceBookProfile;

	bool mRetryingEvent;
	bool mPassedEvent;
	bool mNeedsAIUpdate;
	bool mRaceBookStarted;
	bool mLoadNextRace;
	U32 mUnlockedCar;

	int mLastSavedRacePosition;
	int mPhotosSaved;
	int mChallengesUnlocked;

	S8	mControllerIndex;

	U8	mSeenHelpPopups;
	U8	mAppsLocked;

	U32 mCurrentRetryingEvent;
	U32 mNumberOfRetrys;

	F32	mSpTotalRaceTime;

	Btl::Set<XtId>			mRoutesWon;
	ProfileOptions			mOptions;
	bool					mInvertY;

	Btl::Set<U32>			mVehicleUnlocks;				// Used for cars unlocked with code.
	Btl::Set<U32>			mTempVehicleUnlocks;			// Used for cars unlocked as downloadable DLC. Do not save out.
	Btl::Set<XtId>			mVehiclesUsedInTierOne;			// Used for Boss Demands

	U32 mCurrentEventId;
	S32 mCurrentSPEventId;

	UI::C_WideString256 mTwitterUsername;
	UI::C_WideString256 mTwitterPassword;

	UI::C_WideString256 mFacebookUsername;
	UI::C_WideString256 mFacebookPassword;

	Btl::Map<U32, RaceEventHistory> mSinglePlayerRaceHistory;

	int mPhotoVoteIndex;
	Btl::Vector<U64, Btl::VectorParameters_Array<MAX_PROFILE_VOTE_BUFFER> > mPhotoVotes;
	int mEventVoteIndex;
	Btl::Vector<U64, Btl::VectorParameters_Array<MAX_PROFILE_VOTE_BUFFER> > mEventVotes;

	bool mUseThisRaceDifficulty;
	U8 mForThisRaceDifficulty;
	U8 mSelectingSPGame;

	Util::C_String128 mWebsiteLinkCode;

	// garage resprays
	ResprayMap	m_Resprays_SP;
	ResprayData	m_RespraysLastUsed_SP;

	// garage mod popup
	U8 mHasSeenGarageModPopup;

	//Game Mode Video Bitfield
	U32 mGameModeVideoBitfield;

	UI::E_MultiplayerVehicleAbilities mSpEquippedAbility;

	//Previously on blur DLC and news TMS ids
	U16	mLatestBlurNewsItemId;
	U16	mLatestDlcItemId;
	U16	mLatestCommunityItemId;

#ifdef PLATFORM_PC
	// [Blur2PC] T250: retail Blur PC profile +0x20bc, a set of DLC pack ids unlocked by code entry; saved after
	// the multiplayer profile, cleared by Init (0x00ba0580). Retail adds through Lua Amax "SetDLCUnlocked"
	// (0x00b89610, from SpDlcEntry.lua) and checks it when licensing DLC packs (0x004debd0).
	Btl::Set<U8> mDLCUnlocked;
	bool IsDLCUnlocked( U8 packId );		// retail 0x00b9fe20
	void SetDLCUnlocked( U8 packId );		// retail 0x00ba1850
#endif

	Game::E_MPS_Result DownloadMultiplayerProfile(const SysProfile::FullProfile& localPlayer, const SysProfile::UserIdentifier& fileOwner);
	bool IsDownloadMultiplayerProfileFinished(Game::E_MPS_Result& result);
	Game::E_MPS_Result ApplyMultiplayerProfile();

	Game::E_MPS_Result UploadMultiplayerProfile(const SysProfile::FullProfile& player);
	bool IsUploadMultiplayerProfileFinished(Game::E_MPS_Result& result);

	bool HasValidDownloadedMPProfile() const;
	void ResetDownloadedMPProfile();

	bool ShouldPlayVideo( eSPVideoConfig videoType );
	void SetVideoPlayed( eSPVideoConfig videoType );

	void LatestBlurNewsItemId(U16 id);
	void LatestDlcItemId(U16 id);
	void LatestCommunityItemId(U16 id);

	U16 LatestBlurNewsItemId();
	U16 LatestDlcItemId();
	U16 LatestCommunityItemId();

	bool SelectingSPGame();
	bool SelectingSPGame(bool spSelected);


private:
	C_MultiplayerProfileStorage m_multiplayerProfileStorage;
};


} // namespace Game

#endif // GAMEPROFILE_H
