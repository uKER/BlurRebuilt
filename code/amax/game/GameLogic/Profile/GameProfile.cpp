#include "stdafx.h"

#include <GameLogic/Profile/GameProfile.h>
#include <Racebook\Profile\RaceBookProfile.h>
#include <Racebook\RaceBookManager.h>
#include <GameLogic\StickerManager.h>

#include <GameLogic\GameData\GameEventData.h>
#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>

#include <GameLogic\GameData\MultiplayerData.h>
#include <GameLogic\ContentServer\KeyArchiveManager.h>
#include <GameLogic/Profile/MultiplayerProfile.h>
#include <GameLogic/Profile/GameProfileManager.h>

#include <GameApp.h>
#include <GameLogic\Layers\RaceBookLayer.h>

#include <Game\GameUtil/ScopedSuspendResume.h>

#include "ui\UiManager.h"
#include "Ui/UiXtConfig.h"
#include "system/serialise/C_MemoryStream.h"
#include "Game/Game.h"

#ifdef PLATFORM_PC
#include <Input/KeyboardPC.h>	// [Blur2PC] keyboard options
#include <stddef.h>
#endif

Debug::Logger &g_GameProfileLogger( Debug::GetLogger("Amax.GameProfile") );
extern Debug::Bool g_ForceProfileRebuild;

const int GetSaveGameVersion();
#ifdef PLATFORM_PC
const int GetLoadingSaveGameVersion();	// [Blur2PC]
#endif

namespace Game
{

C_GameProfile::C_GameProfile():
	m_multiplayerProfileStorage(GetSaveGameVersion())
{
#ifdef PLATFORM_PC
	// [Blur2PC] retail's profiles are static objects, so the keyboard layout reads 0 until set; ProfileOptions::Init
	// (0x00b9e790) does not touch it
	mOptions.mKeyboardLayout = 0;
	// [Blur2PC] T250: likewise the option bytes nothing in retail touches stay zero
	memset( mOptions.mOnlineLoginName, 0, sizeof(mOptions.mOnlineLoginName) );
	memset( mOptions.mOnlineLoginPassword, 0, sizeof(mOptions.mOnlineLoginPassword) );
	memset( mOptions.mRetailUnknownFC, 0, sizeof(mOptions.mRetailUnknownFC) );
#endif
}

void C_GameProfile::Init(int controller_index)
{
	mRaceBookStarted=false;
	mCurrentEventId = 0;
	mCurrentSPEventId = -1;
	mPassedEvent=false;
	mNeedsAIUpdate=true;
	mRetryingEvent=false;
	mLoadNextRace=false;
	mUnlockedCar = 0;
	mLastSavedRacePosition = 1;
	mNumberOfRetrys = 0.f;

	mSeenHelpPopups = 0;
	mPhotosSaved = 0;
	mChallengesUnlocked = 0;
	mSeenHelpPopups = 0;

	mCurrentRetryingEvent = 0;
	mNumberOfRetrys = 0;

	mVehicleUnlocks.Clear();
	mTempVehicleUnlocks.Clear();
	mVehiclesUsedInTierOne.Clear();

	mRoutesWon.Clear();

	mMultiplayerProfile.Init();

	mControllerIndex=controller_index;

	// Apps that are initially locked
	mAppsLocked = 0;
	mOptions.Init();
	mInvertY = false;

	mTwitterUsername=L"";
	mTwitterPassword=L"";

	mFacebookUsername=L"";
	mFacebookPassword=L"";

	mWebsiteLinkCode="";
	
	mUseThisRaceDifficulty=false;
	mForThisRaceDifficulty=GAME_DIFFICULTY_NORMAL;

	mSpEquippedAbility = UI::E_MultiplayerVehicleAbilities_None;

	mSinglePlayerRaceHistory.Clear();

	mPhotoVoteIndex=0;
	mEventVoteIndex=0;
	for(int a=0;a<MAX_PROFILE_VOTE_BUFFER;a++)
	{
		mPhotoVotes[a]=0;
		mEventVotes[a]=0;
	}

	// garage resprays
	m_Resprays_SP.Clear();
	m_RespraysLastUsed_SP = ResprayData( 0, 0 );

	mHasSeenGarageModPopup = 0;

	// Game Mode Video Bitfield
	mGameModeVideoBitfield = 0;

	mLatestBlurNewsItemId = 0;
	mLatestDlcItemId = 0;
	mLatestCommunityItemId = 0;
	mSelectingSPGame = 1;
#ifdef PLATFORM_PC
	mDLCUnlocked.Clear();	// [Blur2PC] T250: retail 0x00ba0580
#endif

	ReloadSorceryData();
}

#ifdef PLATFORM_PC
// [Blur2PC] T250: retail 0x00b9fe20
bool C_GameProfile::IsDLCUnlocked( U8 packId )
{
	return mDLCUnlocked.Find( packId ) != mDLCUnlocked.End();
}

// [Blur2PC] T250: retail 0x00ba1850
void C_GameProfile::SetDLCUnlocked( U8 packId )
{
	mDLCUnlocked.Insert( packId );
}
#endif

void C_GameProfile::CacheDLCData()
{
	for( int i = 0; i < C_GameData::Instance().mCheatPacks.Number(); i++ )
	{
		AmaxCheatBase* cheatBase = C_GameData::Instance().mCheatPacks[i];
		if(cheatBase && cheatBase->GetType() == E_CHEAT_TYPE_VEHICLE && cheatBase->dlcLicensed)
		{
			AmaxCheatVehicle* pCheatVehicle = (AmaxCheatVehicle*)cheatBase;

			if(pCheatVehicle)
			{
				AmaxVehicle* pAmaxVeh = pCheatVehicle->mAmaxVehicle.Object();

				if(pAmaxVeh)
				{
					mTempVehicleUnlocks.Insert(pAmaxVeh->ID);
				}
			}
		}
	}
}

void C_GameProfile::ReloadSorceryData()
{
	MultiplayerConfig* multiplayerConfig=C_MultiplayerData::Instance().m_pConfig;
	m_multiplayerProfileStorage.SetPackVersion(multiplayerConfig->m_PackVersion);
}

void C_GameProfile::ClearRaceDifficulty()
{
	mUseThisRaceDifficulty=false;
	mForThisRaceDifficulty=GAME_DIFFICULTY_NORMAL;
}

U8 C_GameProfile::GetDifficulty()
{
	if(mUseThisRaceDifficulty)
		return mForThisRaceDifficulty;
	else
		return mOptions.mGameDifficulty;
}

void C_GameProfile::SetDifficulty(U8 diff, bool just_for_this_race)
{
	if(just_for_this_race)
	{
		mUseThisRaceDifficulty=true;
		mForThisRaceDifficulty=diff;
	}
	else
	{
		mOptions.mGameDifficulty=diff;
	}
}

ProfileOptions::E_AudioOutputType ProfileOptions::GetDefaultAudioOutputType()
{
#ifdef PLATFORM_PS3

	CellAudioOutState state;

	cellAudioOutGetState(0, 0, &state);

	if(state.soundMode.layout == CELL_AUDIO_OUT_SPEAKER_LAYOUT_2CH)
	{
		return E_AudioOutputType_Stereo;
	}
	else
	{
		return E_AudioOutputType_Surround;
	}
#endif

	
	return E_AudioOutputType_Stereo;
}

void ProfileOptions::Init()
{
	// Default options
	mDefaultGameDifficulty = GAME_DIFFICULTY_NORMAL;
	//mSteeringAssist = 0;
	//mBrakingAssist = 0;
	mDefaultAutoPickup = 0;
	mDefaultControlIndex = 0;
	mDefaultCameraType = 0;
#ifdef PLATFORM_PC
	mDefaultAudioQuality = 0;			// [Blur2PC] T144: retail ProfileOptions::Init (0x00b9e790)
#endif
	mDefaultAudioSfxVolume = 7;			// Scale 0 -> 9
	mDefaultAudioMusicType = 0;			// 0:Composed Soundtrack, 1:Licensed Soundtrack
	mDefaultAudioMusicVolume = 7;			// Scale 0 -> 9
	mDefaultAudioVoiceVolume = 7;			// Scale 0 -> 9
	mDefaultAudioMenuMusicVolume = 7;		// Scale 0 -> 9

	if (GetDefaultAudioOutputType()==E_AudioOutputType_Stereo)
	{
		mDefaultAudioOutputType = 0;			// 0:Stereo, 1:Surround
		mDefaultAudioDynamicRange = 0;			// 0:TV, 1:HomeTheatre
	}
	else
	{
		mDefaultAudioOutputType = 1;			// 0:Stereo, 1:Surround
		mDefaultAudioDynamicRange = 1;			// 0:TV, 1:HomeTheatre
	}

	mDefaultAudioRoomSize = 1;				// 0:Small, 1:Medium, 2:Large, 3:Auditorium
	mDefaultAudioMusicMode = 2;			// 0:Stereo, 1:Quad, 2:Surround
	mDefaultAudiLFEeBoost = 0;				// Scale 0 -> 9
	mDefaultGammaIndex = 5;				// Scale 0 -> 9
	mDefaultAudioSpeakerAngleFL = 240.0f;
	mDefaultAudioSpeakerAngleFR = 300.0f;
	mDefaultAudioSpeakerAngleRR = 20.0f;
	mDefaultAudioSpeakerAngleRL = 160.0f;

	//initialise to defaults
	mGameDifficulty			= mDefaultGameDifficulty;;
	//mSteeringAssist		= mSteeringAssist;
	//mBrakingAssist		= mBrakingAssist;
	mAutoPickup				= mDefaultAutoPickup;
	mControlIndex			= mDefaultControlIndex;
	mCameraType				= mDefaultCameraType;
#ifdef PLATFORM_PC
	mAudioQuality			= mDefaultAudioQuality;	// [Blur2PC] T144
#endif
	mAudioSfxVolume			= mDefaultAudioSfxVolume;
	mAudioMusicType			= mDefaultAudioMusicType;
	mAudioMusicVolume		= mDefaultAudioMusicVolume;
	mAudioVoiceVolume		= mDefaultAudioVoiceVolume;
	mAudioMenuMusicVolume	= mDefaultAudioMenuMusicVolume;
	mAudioOutputType		= mDefaultAudioOutputType;
	mAudioRoomSize			= mDefaultAudioRoomSize;
	mAudioDynamicRange		= mDefaultAudioDynamicRange;
	mAudioMusicMode			= mDefaultAudioMusicMode;
	mAudiLFEeBoost			= mDefaultAudiLFEeBoost;
	mGammaIndex				= mDefaultGammaIndex;
	mAudioSpeakerAngleFL	= mDefaultAudioSpeakerAngleFL;
	mAudioSpeakerAngleFR	= mDefaultAudioSpeakerAngleFR;
	mAudioSpeakerAngleRR	= mDefaultAudioSpeakerAngleRR;
	mAudioSpeakerAngleRL	= mDefaultAudioSpeakerAngleRL;

	mMatchingRule = RacerNet::eMatchingRuleAny;

#ifdef PLATFORM_PC
	// [Blur2PC] T250: retail 0x00b9e790 empties the online login strings
	mOnlineLoginName[0] = 0;
	mOnlineLoginPassword[0] = 0;
	InitKeyboard();
#endif
}

#ifdef PLATFORM_PC
// [Blur2PC] Retail Blur PC keyboard options (Blur.exe 1.2). Key names of the custom layout as the UI shows them
// (retail table 0x00eade48; the misspelt right bracket is retail's).
struct C_KeyName
{
	S32			m_Key;
	const char*	m_Name;
};

static const C_KeyName kKeyNames[] =
{
	{ 1, "CMN_1" }, { 2, "CMN_2" }, { 3, "CMN_3" }, { 4, "CMN_4" }, { 5, "CMN_5" }, { 6, "CMN_6" }, { 7, "CMN_7" },
	{ 8, "CMN_8" }, { 9, "CMN_9" }, { 10, "CMN_0" }, { 11, "CMN_KEYS_MINUS" }, { 12, "CMN_KEYS_PLUS" },
	{ 13, "CMN_KEYS_BACKSPACE" }, { 14, "CMN_KEYS_TAB" }, { 15, "CMN_Q" }, { 16, "CMN_W" }, { 17, "CMN_E" },
	{ 18, "CMN_R" }, { 19, "CMN_T" }, { 20, "CMN_Y" }, { 21, "CMN_U" }, { 22, "CMN_I" }, { 23, "CMN_O" }, { 24, "CMN_P" },
	{ 25, "CMN_KEYS_LSQR_BRACKET" }, { 26, "CMN_KEYS_RSQR_BRAQCKET" }, { 27, "CMN_KEYS_RETURN" }, { 28, "CMN_KEYS_LCTRL" },
	{ 29, "CMN_A" }, { 30, "CMN_S" }, { 31, "CMN_D" }, { 32, "CMN_F" }, { 33, "CMN_G" }, { 34, "CMN_H" }, { 35, "CMN_J" },
	{ 36, "CMN_K" }, { 37, "CMN_L" }, { 38, "CMN_KEYS_SEMI_COLON" }, { 39, "CMN_KEYS_APOSTROPHE" }, { 40, "CMN_KEYS_GRAVE" },
	{ 41, "CMN_KEYS_LSHIFT" }, { 42, "CMN_KEYS_BSLASH" }, { 43, "CMN_Z" }, { 44, "CMN_X" }, { 45, "CMN_C" }, { 46, "CMN_V" },
	{ 47, "CMN_B" }, { 48, "CMN_N" }, { 49, "CMN_M" }, { 50, "CMN_KEYS_COMMA" }, { 51, "CMN_KEYS_FULLSTOP" },
	{ 52, "CMN_KEYS_FSLASH" }, { 53, "CMN_KEYS_RSHIFT" }, { 54, "CMN_KEYS_NUMPAD_ASTERISK" }, { 55, "CMN_KEYS_LALT" },
	{ 56, "CMN_KEYS_SPACE" }, { 58, "CMN_KEYS_F1" }, { 59, "CMN_KEYS_F2" }, { 60, "CMN_KEYS_F3" }, { 61, "CMN_KEYS_F4" },
	{ 62, "CMN_KEYS_F5" }, { 63, "CMN_KEYS_F6" }, { 64, "CMN_KEYS_F7" }, { 65, "CMN_KEYS_F8" }, { 66, "CMN_KEYS_F9" },
	{ 67, "CMN_KEYS_F10" }, { 70, "CMN_KEYS_NUMPAD_7" }, { 71, "CMN_KEYS_NUMPAD_8" }, { 72, "CMN_KEYS_NUMPAD_9" },
	{ 73, "CMN_KEYS_NUMPAD_MINUS" }, { 74, "CMN_KEYS_NUMPAD_4" }, { 75, "CMN_KEYS_NUMPAD_5" }, { 76, "CMN_KEYS_NUMPAD_6" },
	{ 77, "CMN_KEYS_NUMPAD_PLUS" }, { 78, "CMN_KEYS_NUMPAD_1" }, { 79, "CMN_KEYS_NUMPAD_2" }, { 80, "CMN_KEYS_NUMPAD_3" },
	{ 81, "CMN_KEYS_NUMPAD_0" }, { 82, "CMN_KEYS_NUMPAD_POINT" }, { 84, "CMN_KEYS_F11" }, { 85, "CMN_KEYS_F12" },
	{ 105, "CMN_KEYS_NUMPAD_RETURN" }, { 106, "CMN_KEYS_RCTRL" }, { 115, "CMN_KEYS_NUMPAD_SLASH" }, { 117, "CMN_KEYS_RALT" },
	{ 119, "CMN_KEYS_HOME" }, { 120, "CMN_KEYS_UP_ARROW" }, { 121, "CMN_KEYS_PAGE_UP" }, { 122, "CMN_KEYS_LEFT_ARROW" },
	{ 123, "CMN_KEYS_RIGHT_ARROW" }, { 124, "CMN_KEYS_END" }, { 125, "CMN_KEYS_DOWN_ARROW" }, { 126, "CMN_KEYS_PAGE_DOWN" },
	{ 127, "CMN_KEYS_INSERT" }, { 128, "CMN_KEYS_DELETE" },
};

// [Blur2PC] T144: retail's layout (0x00b9e790, 0x00b9e8e0) puts the quality bytes in what was padding, so
// everything else keeps its offset
static_assert( offsetof(ProfileOptions, mAudioQuality) == 0x04, "PROFILE_OPTIONS_QUALITY_OFFSET" );
static_assert( offsetof(ProfileOptions, mGammaIndex) == 0x0f, "PROFILE_OPTIONS_GAMMA_OFFSET" );
static_assert( offsetof(ProfileOptions, mAudioSpeakerAngleFL) == 0x10, "PROFILE_OPTIONS_SPEAKER_OFFSET" );
static_assert( offsetof(ProfileOptions, mDefaultAudioQuality) == 0x24, "PROFILE_OPTIONS_DEFAULT_QUALITY_OFFSET" );
static_assert( offsetof(ProfileOptions, mDefaultAudioSpeakerAngleFL) == 0x30, "PROFILE_OPTIONS_DEFAULT_SPEAKER_OFFSET" );
// [Blur2PC] T250: retail saves the options whole (profile load 0x00ba43c0 reads 0x170 bytes)
static_assert( offsetof(ProfileOptions, mMatchingRule) == 0x40, "PROFILE_OPTIONS_MATCHING_RULE_OFFSET" );
static_assert( offsetof(ProfileOptions, mOnlineLoginName) == 0x41, "PROFILE_OPTIONS_LOGIN_NAME_OFFSET" );
static_assert( offsetof(ProfileOptions, mOnlineLoginPassword) == 0x61, "PROFILE_OPTIONS_LOGIN_PASSWORD_OFFSET" );
static_assert( offsetof(ProfileOptions, mKeyboardIsCustom) == 0xc4, "PROFILE_OPTIONS_KEYBOARD_CUSTOM_OFFSET" );
static_assert( offsetof(ProfileOptions, mKeyThrottle) == 0xc8, "PROFILE_OPTIONS_KEYS_OFFSET" );
static_assert( offsetof(ProfileOptions, mKeySteerRight) == 0xf8, "PROFILE_OPTIONS_LAST_KEY_OFFSET" );
static_assert( offsetof(ProfileOptions, mKeyboardLayout) == 0x16d, "PROFILE_OPTIONS_KEYBOARD_LAYOUT_OFFSET" );
static_assert( sizeof(ProfileOptions) == 0x170, "PROFILE_OPTIONS_SIZE" );

// [Blur2PC] T250: Blur2PC's own saves before 1378 kept the options at 0x7c bytes (1374/1375: keyboard custom flag
// at +0x41, keys at +0x44, layout at +0x78) or 0x44 bytes (1373: up to the matching rule); 1373/1374 also lack
// the audio quality byte
static const int kLegacyOptionsSize1373 = 0x44;
static const int kLegacyOptionsSize1374 = 0x7c;
static const int kLegacyOptionsKeyboardOffset = 0x41;
static const int kLegacyOptionsKeysOffset = 0x44;
static const int kLegacyOptionsLayoutOffset = 0x78;

void ProfileOptions::ReadLegacyOptions( Sys::BinaryDeserialiserAutoEndian& in, int version )
{
	U8 legacy[kLegacyOptionsSize1374];
	const int size = ( version == 1373 ) ? kLegacyOptionsSize1373 : kLegacyOptionsSize1374;
	in.Read( legacy, size );

	memcpy( this, legacy, offsetof(ProfileOptions, mOnlineLoginName) );
	mOnlineLoginName[0] = 0;
	mOnlineLoginPassword[0] = 0;
	if( version == 1373 )
	{
		InitKeyboard();
		mKeyboardLayout = 0;
	}
	else
	{
		mKeyboardIsCustom = (S8)legacy[kLegacyOptionsKeyboardOffset];
		memcpy( &mKeyThrottle, legacy + kLegacyOptionsKeysOffset, kLegacyOptionsLayoutOffset - kLegacyOptionsKeysOffset );
		mKeyboardLayout = (S8)legacy[kLegacyOptionsLayoutOffset];
	}

	if( version == 1373 || version == 1374 )
		InsertAudioQualityIntoLegacyOptions();
}

// [Blur2PC] T144: a 1373/1374 save has the bytes from the SFX volume to the gamma index one place lower, in
// both the options and their defaults; move them up and give quality its default
void ProfileOptions::InsertAudioQualityIntoLegacyOptions()
{
	U8* pu8Options = (U8*)this;

	memmove(pu8Options + offsetof(ProfileOptions, mAudioSfxVolume), pu8Options + offsetof(ProfileOptions, mAudioQuality),
		offsetof(ProfileOptions, mGammaIndex) - offsetof(ProfileOptions, mAudioQuality));
	memmove(pu8Options + offsetof(ProfileOptions, mDefaultAudioSfxVolume), pu8Options + offsetof(ProfileOptions, mDefaultAudioQuality),
		offsetof(ProfileOptions, mDefaultGammaIndex) - offsetof(ProfileOptions, mDefaultAudioQuality));

	mDefaultAudioQuality = 0;
	mAudioQuality = mDefaultAudioQuality;
}

// Retail ProfileOptions::Init (0x00b9e790): not custom, the custom keys start as layout 0
void ProfileOptions::InitKeyboard()
{
	mKeyboardIsCustom	= 0;
	mKeyThrottle		= Input::C_Keyboard::E_Button_Q;
	mKeyBrake			= Input::C_Keyboard::E_Button_A;
	mKeyEBrake			= Input::C_Keyboard::E_Button_Down;
	mKeyFire			= Input::C_Keyboard::E_Button_RControl;
	mKeySwitchPerk		= Input::C_Keyboard::E_Button_Up;
	mKeyDrop			= Input::C_Keyboard::E_Button_D;
	mKeyChangeView		= Input::C_Keyboard::E_Button_V;
	mKeyPause			= Input::C_Keyboard::E_Button_P;
	mKeyRearView		= Input::C_Keyboard::E_Button_Tab;
	mKeyPowerUpFwd		= Input::C_Keyboard::E_Button_W;
	mKeyPowerUpBwd		= Input::C_Keyboard::E_Button_S;
	mKeySteerLeft		= Input::C_Keyboard::E_Button_Left;
	mKeySteerRight		= Input::C_Keyboard::E_Button_Right;
}

// Retail 0x00b9ef70: IsCustom and the name of each key; a key with no name is left out
void ProfileOptions::SendKeyboardToLua( lua_State* pState )
{
	Lua::AddDataMember(pState, "IsCustom", (S32)mKeyboardIsCustom );

	for( U32 i = 0; i < lengthof( kKeyNames ); i++ )
	{
		const C_KeyName& key = kKeyNames[i];
		if( mKeyThrottle == key.m_Key )		Lua::AddDataMember(pState, "Throttle", key.m_Name );
		if( mKeyBrake == key.m_Key )		Lua::AddDataMember(pState, "Brake", key.m_Name );
		if( mKeyEBrake == key.m_Key )		Lua::AddDataMember(pState, "EBrake", key.m_Name );
		if( mKeyChangeView == key.m_Key )	Lua::AddDataMember(pState, "ChangeView", key.m_Name );
		if( mKeyRearView == key.m_Key )		Lua::AddDataMember(pState, "RearView", key.m_Name );
		if( mKeyPause == key.m_Key )		Lua::AddDataMember(pState, "Pause", key.m_Name );
		if( mKeySwitchPerk == key.m_Key )	Lua::AddDataMember(pState, "SwitchPerk", key.m_Name );
		if( mKeyFire == key.m_Key )			Lua::AddDataMember(pState, "Fire", key.m_Name );
		if( mKeyDrop == key.m_Key )			Lua::AddDataMember(pState, "Drop", key.m_Name );
		if( mKeyPowerUpFwd == key.m_Key )	Lua::AddDataMember(pState, "PowerUpFwd", key.m_Name );
		if( mKeyPowerUpBwd == key.m_Key )	Lua::AddDataMember(pState, "PowerUpBwd", key.m_Name );
		if( mKeySteerLeft == key.m_Key )	Lua::AddDataMember(pState, "SteerLeft", key.m_Name );
		if( mKeySteerRight == key.m_Key )	Lua::AddDataMember(pState, "SteerRight", key.m_Name );
	}
}

// Retail 0x00b9f170: numbers only; a missing or non-numeric field keeps its value
void ProfileOptions::GetKeyboardFromLua( lua_State* pState )
{
	Lua::GetDataMember(pState, "IsCustom", mKeyboardIsCustom, true );
	Lua::GetDataMember(pState, "Throttle", mKeyThrottle, true );
	Lua::GetDataMember(pState, "Brake", mKeyBrake, true );
	Lua::GetDataMember(pState, "EBrake", mKeyEBrake, true );
	Lua::GetDataMember(pState, "ChangeView", mKeyChangeView, true );
	Lua::GetDataMember(pState, "RearView", mKeyRearView, true );
	Lua::GetDataMember(pState, "Pause", mKeyPause, true );
	Lua::GetDataMember(pState, "SwitchPerk", mKeySwitchPerk, true );
	Lua::GetDataMember(pState, "Fire", mKeyFire, true );
	Lua::GetDataMember(pState, "Drop", mKeyDrop, true );
	Lua::GetDataMember(pState, "PowerUpFwd", mKeyPowerUpFwd, true );
	Lua::GetDataMember(pState, "PowerUpBwd", mKeyPowerUpBwd, true );
	Lua::GetDataMember(pState, "SteerLeft", mKeySteerLeft, true );
	Lua::GetDataMember(pState, "SteerRight", mKeySteerRight, true );
}
#endif

void ProfileOptions::SendToLua( lua_State* pState )
{
	Lua::AddDataMember(pState,"gamma", mGammaIndex );

#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC (0x00b9e8e0) adds the graphics settings, read live from the application (they
	// are stored in settings.xml, not in the profile)
	{
		S32 resolutionIndex = 0;
		S32 multisampleIndex = 0;
		bool windowed = true;
		bool vsync = true;
		C_GameApp::GetInstance()->GetGraphicsOptions(&resolutionIndex, &multisampleIndex, &windowed, &vsync);
		Lua::AddDataMember(pState,"resolution", resolutionIndex );
		Lua::AddDataMember(pState,"aa", multisampleIndex );
		Lua::AddDataMember(pState,"fullscreen", (U8)(windowed ? 0 : 1) );
		Lua::AddDataMember(pState,"vsync", (U8)(vsync ? 1 : 0) );
		Lua::AddDataMember(pState,"graphics_quality", (S32)C_GameApp::GetInstance()->GetGraphicalDetail() );
	}
#endif

	Lua::AddDataMember(pState,"difficulty", mGameDifficulty );
	//Lua::AddDataMember(pState,"steering_assist", mSteeringAssist );
	//Lua::AddDataMember(pState,"braking_assist", mBrakingAssist );
	Lua::AddDataMember(pState,"auto_pickup", mAutoPickup );

	Lua::AddDataMember(pState,"ctrl_index", mControlIndex );
	Lua::AddDataMember(pState,"camera_index", mCameraType );
#ifdef PLATFORM_PC
	Lua::AddDataMember(pState,"quality", mAudioQuality );	// [Blur2PC] T144: retail 0x00b9e8e0
#endif

	Lua::AddDataMember(pState,"fx_vol", mAudioSfxVolume );
	Lua::AddDataMember(pState,"music_type", mAudioMusicType );
	Lua::AddDataMember(pState,"music_vol", mAudioMusicVolume );
	Lua::AddDataMember(pState,"voice_vol", mAudioVoiceVolume );
	Lua::AddDataMember(pState,"menu_vol", mAudioMenuMusicVolume );

	Lua::AddDataMember(pState,"output", mAudioOutputType );
	Lua::AddDataMember(pState,"room", mAudioRoomSize );
	Lua::AddDataMember(pState,"dynamic", mAudioDynamicRange );
	Lua::AddDataMember(pState,"music_mode", mAudioMusicMode );
	Lua::AddDataMember(pState,"lfe",mAudiLFEeBoost );

	Lua::AddDataMember(pState,"speaker_fl", mAudioSpeakerAngleFL );
	Lua::AddDataMember(pState,"speaker_fr", mAudioSpeakerAngleFR );
	Lua::AddDataMember(pState,"speaker_rr", mAudioSpeakerAngleRR );
	Lua::AddDataMember(pState,"speaker_rl", mAudioSpeakerAngleRL );
}

void ProfileOptions::GetFromLua( lua_State* pState )
{
	Lua::GetDataMember(pState,"gamma", mGammaIndex, true );

#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC (0x00b9eae0) reads the graphics settings as bytes and applies them to the
	// application: graphical detail first, then resolution, multisample, full screen and vsync. Retail leaves
	// the bytes uninitialised when a key is missing; here a missing key keeps the current value.
	{
		S32 resolutionIndex = 0;
		S32 multisampleIndex = 0;
		bool windowed = true;
		bool vsync = true;
		C_GameApp::GetInstance()->GetGraphicsOptions(&resolutionIndex, &multisampleIndex, &windowed, &vsync);

		U8 graphicsQuality	= (U8)C_GameApp::GetInstance()->GetGraphicalDetail();
		U8 resolution		= (U8)resolutionIndex;
		U8 fullscreen		= windowed ? 0 : 1;
		U8 aa				= (U8)multisampleIndex;
		U8 vsyncByte		= vsync ? 1 : 0;

		Lua::GetDataMember(pState,"graphics_quality", graphicsQuality, true );
		Lua::GetDataMember(pState,"resolution", resolution, true );
		Lua::GetDataMember(pState,"fullscreen", fullscreen, true );
		Lua::GetDataMember(pState,"aa", aa, true );
		Lua::GetDataMember(pState,"vsync", vsyncByte, true );

		C_GameApp::GetInstance()->SetGraphicalDetail(graphicsQuality);
		C_GameApp::GetInstance()->ApplyGraphicsOptions(resolution, aa, fullscreen == 0, vsyncByte == 1);
	}
#endif

	Lua::GetDataMember(pState,"difficulty",mGameDifficulty);
	//Lua::GetDataMember(pState,"steering_assist", mSteeringAssist, true );
	//Lua::GetDataMember(pState,"braking_assist", mBrakingAssist, true );
	Lua::GetDataMember(pState,"auto_pickup", mAutoPickup, true );

	Lua::GetDataMember(pState,"ctrl_index", mControlIndex, true );
	Lua::GetDataMember(pState,"camera_index", mCameraType, true );
#ifdef PLATFORM_PC
	Lua::GetDataMember(pState,"quality", mAudioQuality, true );	// [Blur2PC] T144: retail 0x00b9eae0
#endif

	Lua::GetDataMember(pState,"fx_vol", mAudioSfxVolume, true );
	Lua::GetDataMember(pState,"music_type", mAudioMusicType, true );
	Lua::GetDataMember(pState,"music_vol", mAudioMusicVolume, true );
	Lua::GetDataMember(pState,"voice_vol", mAudioVoiceVolume, true );
	Lua::GetDataMember(pState,"menu_vol", mAudioMenuMusicVolume, true );

	Lua::GetDataMember(pState,"output", mAudioOutputType, true );
	Lua::GetDataMember(pState,"room", mAudioRoomSize, true );
	Lua::GetDataMember(pState,"dynamic", mAudioDynamicRange, true );
	Lua::GetDataMember(pState,"music_mode", mAudioMusicMode, true );
	Lua::GetDataMember(pState,"lfe",mAudiLFEeBoost, true );

	Lua::GetDataMember(pState,"speaker_fl", mAudioSpeakerAngleFL, true );
	Lua::GetDataMember(pState,"speaker_fr", mAudioSpeakerAngleFR, true );
	Lua::GetDataMember(pState,"speaker_rr", mAudioSpeakerAngleRR, true );
	Lua::GetDataMember(pState,"speaker_rl", mAudioSpeakerAngleRL, true );
}

void ProfileOptions::SendDefaultsToLua( lua_State* pState )
{
	Lua::AddDataMember(pState,"gamma", mDefaultGammaIndex );

	Lua::AddDataMember(pState,"difficulty", mDefaultGameDifficulty );
	//Lua::AddDataMember(pState,"steering_assist", mDefaultSteeringAssist );
	//Lua::AddDataMember(pState,"braking_assist", mDefaultBrakingAssist );
	Lua::AddDataMember(pState,"auto_pickup", mDefaultAutoPickup );

	Lua::AddDataMember(pState,"ctrl_index", mDefaultControlIndex );
	Lua::AddDataMember(pState,"camera_index", mDefaultCameraType );
#ifdef PLATFORM_PC
	Lua::AddDataMember(pState,"quality", mDefaultAudioQuality );	// [Blur2PC] T144: retail 0x00b9ed00
#endif

	Lua::AddDataMember(pState,"fx_vol", mDefaultAudioSfxVolume );
	Lua::AddDataMember(pState,"music_type", mDefaultAudioMusicType );
	Lua::AddDataMember(pState,"music_vol", mDefaultAudioMusicVolume );
	Lua::AddDataMember(pState,"voice_vol", mDefaultAudioVoiceVolume );
	Lua::AddDataMember(pState,"menu_vol", mDefaultAudioMenuMusicVolume );

	Lua::AddDataMember(pState,"output", mDefaultAudioOutputType );
	Lua::AddDataMember(pState,"room", mDefaultAudioRoomSize );
	Lua::AddDataMember(pState,"dynamic", mDefaultAudioDynamicRange );
	Lua::AddDataMember(pState,"music_mode", mDefaultAudioMusicMode );
	Lua::AddDataMember(pState,"lfe",mDefaultAudiLFEeBoost );

	Lua::AddDataMember(pState,"speaker_fl", mDefaultAudioSpeakerAngleFL );
	Lua::AddDataMember(pState,"speaker_fr", mDefaultAudioSpeakerAngleFR );
	Lua::AddDataMember(pState,"speaker_rr", mDefaultAudioSpeakerAngleRR );
	Lua::AddDataMember(pState,"speaker_rl", mDefaultAudioSpeakerAngleRL );
}

void C_GameProfile::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );

	out << mLastSavedRacePosition 
		<< mPhotosSaved 
		<< mChallengesUnlocked 
		<< mSeenHelpPopups 
		<< mAppsLocked 
		<< mCurrentRetryingEvent 
		<< mNumberOfRetrys
		<< mSpTotalRaceTime
		;

	out << mVehicleUnlocks.Size();
	for( Btl::Set<U32>::Iterator it = mVehicleUnlocks.Begin(); it != mVehicleUnlocks.End(); it++ )
		out << (U32)*it;

	out << mVehiclesUsedInTierOne.Size();
	for( Btl::Set<U32>::Iterator it = mVehiclesUsedInTierOne.Begin(); it != mVehiclesUsedInTierOne.End(); it++ )
		out << (U32)*it;

	out << mRoutesWon.Size();
	for( Btl::Set<XtId>::Iterator it = mRoutesWon.Begin(); it != mRoutesWon.End(); it++ )
		out << (U32)*it;

	out.Write( &mOptions, sizeof(ProfileOptions) );

	out << mSinglePlayerRaceHistory.Size();
	for( Btl::Map<U32, RaceEventHistory>::Iterator it = mSinglePlayerRaceHistory.Begin(); it != mSinglePlayerRaceHistory.End(); it++ )
	{
		out << it->GetKey() << it->GetValue().mDifficulty << it->GetValue().mFans << it->GetValue().mHighestCombo << it->GetValue().mFanDemandsComplete << it->GetValue().mVehicleUsed;
	}

	out << mPhotoVoteIndex;
	for(int a=0;a<MAX_PROFILE_VOTE_BUFFER;a++)
	{
		out << mPhotoVotes[a];
		out << mEventVotes[a];
	}

	out << mWebsiteLinkCode;

	// garage resprays
	out << m_Resprays_SP.Size();
	for ( ResprayMap::Iterator it = m_Resprays_SP.Begin(); it != m_Resprays_SP.End(); ++it )
		out << it->GetKey() << it->GetValue().first << it->GetValue().second;

	out << m_RespraysLastUsed_SP.first << m_RespraysLastUsed_SP.second;

	out << mHasSeenGarageModPopup;

	out << mGameModeVideoBitfield;

	//sticker progress
	out << Game::C_StickerManager::Instance();

	out << (S32)mSpEquippedAbility;

	//previously on blur
	out << mLatestBlurNewsItemId;
	out << mLatestDlcItemId;
	out << mLatestCommunityItemId;

	out << mSelectingSPGame;

	out << mMultiplayerProfile;  // Going to be thrown away - just for the purposes of PoB (previously on Blur)

#ifdef PLATFORM_PC
	// [Blur2PC] T250: retail 1378 (0x00ba2530)
	out << mDLCUnlocked.Size();
	for( Btl::Set<U8>::Iterator it = mDLCUnlocked.Begin(); it != mDLCUnlocked.End(); it++ )
		out << (U8)*it;
#endif
}

Game::E_MPS_Result C_GameProfile::UploadMultiplayerProfile(const SysProfile::FullProfile& player)
{
	static const int initialStreamSize=4*1024;
	Sys::C_MemoryStream stream(initialStreamSize);
	Sys::BinarySerialiserAutoEndian serial( stream );

	serial<<mMultiplayerProfile;

	return m_multiplayerProfileStorage.Upload(stream, player);
}

bool C_GameProfile::IsUploadMultiplayerProfileFinished(E_MPS_Result& result)
{
	F32 timeStep = C_GameApp::GetInstance()->GetTimeStep();
	bool finished=m_multiplayerProfileStorage.IsUploadFinished(timeStep);
	result=m_multiplayerProfileStorage.GetLastTransactionResult();
	return finished;
}

bool C_GameProfile::HasValidDownloadedMPProfile() const
{
#ifndef CFG_GOLD
	// need invalidate the profile that is already in memory
	if ( g_ForceProfileRebuild )
		return false;
#endif
	return m_multiplayerProfileStorage.HasValidDownloadedData();
}

void C_GameProfile::ResetDownloadedMPProfile()
{
	m_multiplayerProfileStorage.ResetDownloadedData();
}

void C_GameProfile::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );

	in	>> mLastSavedRacePosition 
		>> mPhotosSaved 
		>> mChallengesUnlocked 
		>> mSeenHelpPopups 
		>> mAppsLocked 
		>> mCurrentRetryingEvent 
		>> mNumberOfRetrys
		>> mSpTotalRaceTime
		;

	int numUnlocks;
	in >> numUnlocks;
	for( int i = 0; i < numUnlocks; i++ )
	{
		U32 vehId;
		in >> vehId;
		mVehicleUnlocks.Insert(vehId);
	}

	int numCars;
	in >> numCars;
	for( int i = 0; i < numCars; i++ )
	{
		XtId	id;
		in >> id;
		mVehiclesUsedInTierOne.Insert(id);
	}


	mRoutesWon.Clear();
	int numRoutes;
	in >> numRoutes;
	for( int i = 0; i < numRoutes; i++ )
	{
		XtId id;
		in >> id;
		mRoutesWon.Insert(id);
	}

#ifdef PLATFORM_PC
	// [Blur2PC] T250: Blur2PC saves before retail's 1378 have smaller options (see ReadLegacyOptions)
	if( GetLoadingSaveGameVersion() < 1378 )
		mOptions.ReadLegacyOptions( in, GetLoadingSaveGameVersion() );
	else
#endif
	in.Read( &mOptions, sizeof(ProfileOptions) );

	int numHistory;
	in >> numHistory;
	for( int i = 0; i < numHistory; i++ )
	{
		U32 eventID;
		RaceEventHistory rh;

		in >> eventID >> rh.mDifficulty >> rh.mFans >> rh.mHighestCombo >> rh.mFanDemandsComplete >> rh.mVehicleUsed;

		mSinglePlayerRaceHistory.Insert(eventID, rh);
	}

	in >> mPhotoVoteIndex;
	for(int a=0;a<MAX_PROFILE_VOTE_BUFFER;a++)
	{
		in >> mPhotoVotes[a];
		in >> mEventVotes[a];
	}

	in >> mWebsiteLinkCode;

	// garage resprays
	S32 resprayCount = 0;
	XtId amaxVehicleId = 0;
	XtId resprayId = 0;
	S32 resprayIndex = 0;

	in >> resprayCount;
	for ( S32 i = 0; i < resprayCount; i++ )
	{
		in >> amaxVehicleId >> resprayId >> resprayIndex;
		m_Resprays_SP.Insert( amaxVehicleId, ResprayData( resprayId, resprayIndex ) );
	}

	in >> m_RespraysLastUsed_SP.first >> m_RespraysLastUsed_SP.second;

	in >> mHasSeenGarageModPopup;

	in >> mGameModeVideoBitfield;

	//sticker progress
	in >> Game::C_StickerManager::Instance();

	S32 tempAbility;
	in >> tempAbility;
	mSpEquippedAbility = (UI::E_MultiplayerVehicleAbilities)tempAbility;

	in >> mLatestBlurNewsItemId;
	in >> mLatestDlcItemId;
	in >> mLatestCommunityItemId;

	in >> mSelectingSPGame;

	in >> mMultiplayerProfile; // Going to be thrown away - just for the purposes of PoB (previously on Blur)

#ifdef PLATFORM_PC
	// [Blur2PC] T250: retail 1378 (0x00ba43c0) ends the profile with the unlocked DLC packs; Init cleared them
	if( GetLoadingSaveGameVersion() >= 1378 )
	{
		int numDLC;
		in >> numDLC;
		for( int i = 0; i < numDLC; i++ )
		{
			U8 packId;
			in >> packId;
			mDLCUnlocked.Insert( packId );
		}
	}
#endif
}

Game::E_MPS_Result C_GameProfile::DownloadMultiplayerProfile(const SysProfile::FullProfile& localPlayer, const SysProfile::UserIdentifier& fileOwner)
{
	return m_multiplayerProfileStorage.Download(localPlayer, fileOwner);
}

bool C_GameProfile::IsDownloadMultiplayerProfileFinished(Game::E_MPS_Result& result)
{
	F32 timeStep = C_GameApp::GetInstance()->GetTimeStep();
	bool res=m_multiplayerProfileStorage.IsDownloadFinished(timeStep);
	result=m_multiplayerProfileStorage.GetLastTransactionResult();
	return res;
}

Game::E_MPS_Result C_GameProfile::ApplyMultiplayerProfile()
{
	BIZ_ASSERT(m_multiplayerProfileStorage.HasValidDownloadedData());

	Btl::Vector<U8> buffer;

	E_MPS_Result result=m_multiplayerProfileStorage.GetDownloaded(buffer);

	if (result==Game::E_MPS_Ok)
	{
		Sys::C_MemoryStream readStream(&buffer[0], buffer.Size());
		Sys::BinaryDeserialiserAutoEndian deserial( readStream );

		deserial>>mMultiplayerProfile;
	}
	return result;
}

void C_GameProfile::ResprayAdd( const XtId amaxVehicleId, const XtId resprayId, const S32 sResprayIndex, const bool bMultiplayer )
{
	ResprayMap& map = ( bMultiplayer ? mMultiplayerProfile.GetRespraysMP() : m_Resprays_SP );
	ResprayMap::InsertionReturnType r = map.Insert( amaxVehicleId, ResprayData( resprayId, sResprayIndex ) );

	if ( !r.inserted )
		r.iterator->GetValue() = ResprayData( resprayId, sResprayIndex );

	if ( bMultiplayer )
		mMultiplayerProfile.GetRespraysLastUsed() = ResprayData( resprayId, sResprayIndex );
	else
		m_RespraysLastUsed_SP = ResprayData( resprayId, sResprayIndex );
}

void C_GameProfile::ResprayRemove( const XtId amaxVehicleId, const bool bMultiplayer )
{
	ResprayMap& map = ( bMultiplayer ? mMultiplayerProfile.GetRespraysMP() : m_Resprays_SP );
	ResprayMap::Iterator it = map.Find( amaxVehicleId );

	if ( it != map.End() )
		map.Erase( it );
}

XtId C_GameProfile::ResprayGet( const XtId amaxVehicleId, const bool bMultiplayer )
{
	ResprayMap& map = ( bMultiplayer ? mMultiplayerProfile.GetRespraysMP() : m_Resprays_SP );
	ResprayMap::Iterator it = map.Find( amaxVehicleId );

	if ( it == map.End() )
		return 0;

	return ( it->GetValue().first );
}

void C_GameProfile::RetryingEvent()
{
	const Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

	mRetryingEvent=true;

	if(mCurrentRetryingEvent == levelState.mEvent)
	{
		mNumberOfRetrys++;
	}
	else
	{
		mCurrentRetryingEvent=levelState.mEvent;
		mNumberOfRetrys=1;
	}
}

C_ProfileBase* C_GameProfile::GetCurrentInGameProfile()
{
	switch(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode())
	{
	case E_GameMode_SinglePlayer:
		{
			if ( mRaceBookProfile )
				return mRaceBookProfile;
			else
			{
				static RaceBook::C_RaceBookProfile gDefaultRaceBookProfile;
				return &gDefaultRaceBookProfile;
			}
		}

	case E_GameMode_Online:
	case E_GameMode_SystemLink:
	case E_GameMode_SplitScreen:
		{
			return &mMultiplayerProfile;
		}
	}

	return NULL;
}

void C_GameProfile::SetSystemDefaults()
{

#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC (0x00b9f3a0) sets up keyboard and pad from the profile (0x00c2fe70)
	UI::SetupPlayerControls( mControllerIndex );
#else
	UI::C_UiXtConfig* pUiConfig = UI::C_UiXtConfig::Instance();
	if( pUiConfig )
	{
		pUiConfig->m_ControlConfigs[ (S32)mOptions.mControlIndex ].RemapGameControls( Input::GetGameInput(mControllerIndex), C_GameApp::GetInstance()->GetPad(mControllerIndex) );
	}
#endif
}

bool C_GameProfile::ShouldPlayVideo( eSPVideoConfig videoType )
{
	//Dont want to play the video if the bit has been set
	bool shouldPlayVideo = (videoType & mGameModeVideoBitfield) > 0 ? false : true;
	return shouldPlayVideo;
}

void C_GameProfile::SetVideoPlayed( eSPVideoConfig videoType )
{
	mGameModeVideoBitfield = (videoType | mGameModeVideoBitfield);
}

void C_GameProfile::LatestBlurNewsItemId(U16 id)
{
	mLatestBlurNewsItemId = id;
}

void C_GameProfile::LatestDlcItemId(U16 id)
{
	mLatestDlcItemId = id;
}

void C_GameProfile::LatestCommunityItemId(U16 id)
{
	mLatestCommunityItemId = id;
}

U16 C_GameProfile::LatestBlurNewsItemId()
{
	return mLatestBlurNewsItemId;
}

U16 C_GameProfile::LatestDlcItemId()
{
	return mLatestDlcItemId;
}

U16 C_GameProfile::LatestCommunityItemId()
{
	return mLatestCommunityItemId;
}

bool C_GameProfile::SelectingSPGame()
{
	return (bool)(mSelectingSPGame != 0);
}

bool C_GameProfile::SelectingSPGame(bool spSelected)
{
	if(SelectingSPGame() != spSelected)
	{
		mSelectingSPGame = (U8)spSelected;
		return true;
	}

	return false;
}

void C_GameProfile::UploadPlayerStats()
{
	if(Game::C_KeyArchiveManager::Instance().IsActive())
	{
		RaceBook::C_RaceBookProfile* prof = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mRaceBookProfile;
		Game::C_MultiplayerProfile* mult_prof = GetMultiplayerProfile();
		SP_SaveGame* sg = SP_GetCurrentSaveGame();
			
		bool gotDemonwareID=Game::C_KeyArchiveManager::Instance().SetUserIndex(mControllerIndex);	//need to get proper index

		BIZ_ASSERT_MSG(prof, "C_GameProfile::UploadPlayerStats() - RaceBookProfile not found. Tell somebody.");
		BIZ_ASSERT_MSG(mult_prof, "C_GameProfile::UploadPlayerStats() - MultiplayerProfile not found. Tell somebody.");
		BIZ_ASSERT_MSG(sg, "C_GameProfile::UploadPlayerStats() - Single Player Save not found. Tell somebody.");

		if(prof && mult_prof && sg && gotDemonwareID)
		{
			Game::C_KeyArchiveManager::Instance().ClearKeyArchiveToWrite();

			int complete=0;
			if(SP_CountMaxStars( sg ))
			{
				complete=((float)SP_CountTotalStars( sg )/(float)SP_CountMaxStars( sg )) * 100;
			}
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_COMPLETION_PERCENTAGE,						complete);

			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_NUMBER_OF_STARS,								SP_CountTotalStars( sg ));

			int bosses=0;
			for(int i=0;i<SP_GetTierCount(sg);i++)
			{
				SP_Tier *tier=SP_GetTier(sg,i);

				if(tier && tier->boss)
				{
					if (SP_IsBossDefeated(sg, tier->boss))
						bosses++;
				}
			}
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_BOSSES_COMPLETE,								bosses);

			for(int a=0;a<9;a++)
			{
				SP_Tier* pTier = SP_GetTier( SP_GetCurrentSaveGame(), a );
				if(pTier)
				{
					int complete=0;
					if(SP_CountMaxTierStars(sg, pTier))
					{
						complete=((float)SP_CountTotalTierStars(sg, pTier)/(float)SP_CountMaxTierStars(sg, pTier)) * 100;
					}
					Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite((Game::KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_TIER1_PERCENTAGE_COMPLETE+a),			complete);
				}
				else
				{
					Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite((Game::KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_TIER1_PERCENTAGE_COMPLETE+a),			0);
				}
			}

			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_TOTAL_FANS,									SP_GetFans( sg ));
			
			int count=0;
			for(int a=0;a<SP_GetTierCount(sg);a++)
			{
				SP_Tier *tier=SP_GetTier(sg,a);

				if(tier)
				{
					for(int b=0;b<tier->events.count;b++)
					{
						if(SP_GetEventState(sg, tier->events[b]) > kSP_Incomplete)
						{
							count++;
						}
					}
				}
			}
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_EVENTS_COMPLETED,							count);

			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_TIMES_WRECKED,								prof->mLifetimeStatistics.mTimesWrecked);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_TIMES_WRECKED_OTHERS,						prof->mLifetimeStatistics.mTimesWreckedOtherVehicle);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_SHOCK_FIRED,									prof->mLifetimeStatistics.mPickupsFired[E_PICKUP_TYPE_SHOCK]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_SHUNT_FIRED,									prof->mLifetimeStatistics.mPickupsFired[E_PICKUP_TYPE_SHUNT]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_BARGE_FIRED,									prof->mLifetimeStatistics.mPickupsFired[E_PICKUP_TYPE_BARGE]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_BOMB_FIRED,									prof->mLifetimeStatistics.mPickupsFired[E_PICKUP_TYPE_BOMB]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_BOLT_FIRED,									prof->mLifetimeStatistics.mPickupsFired[E_PICKUP_TYPE_BLINDFIRE]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_SHOCK_HIT,									prof->mLifetimeStatistics.mPickupsHit[E_PICKUP_TYPE_SHOCK]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_SHUNT_HIT,									prof->mLifetimeStatistics.mPickupsHit[E_PICKUP_TYPE_SHUNT]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_BARGE_HIT,									prof->mLifetimeStatistics.mPickupsHit[E_PICKUP_TYPE_BARGE]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_BOMB_HIT, 									prof->mLifetimeStatistics.mPickupsHit[E_PICKUP_TYPE_BOMB]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_BOLT_HIT, 									prof->mLifetimeStatistics.mPickupsHit[E_PICKUP_TYPE_BLINDFIRE]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_NITROUS_USED,								prof->mLifetimeStatistics.mPickupsFired[E_PICKUP_TYPE_NITROUS]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_REPAIR_USED, 								prof->mLifetimeStatistics.mTimesRepaired);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_SHIELD_USED, 								prof->mLifetimeStatistics.mTimesShielded);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_SINGLE_PLAYER_TOTAL_DISTANCE_TRAVELLED,					prof->mLifetimeStatistics.mDistanceDriven);
			
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_BLUR_EVENT_RATING,							mult_prof->GetMultiplayerStatistics().mNumRacePoints);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_RACES_WON,									mult_prof->GetMultiplayerStatistics().mNumEventsWon);
			
			float powerup_perc=0;
			if(mult_prof->GetMultiplayerStatistics().mTotalPickupsFired)
			{
				powerup_perc=((float)mult_prof->GetMultiplayerStatistics().mTotalPickupsHit / (float)mult_prof->GetMultiplayerStatistics().mTotalPickupsFired) * 100.0f;
			}
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_POWERUP_HIT_RATIO,							powerup_perc*10000);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_RACES_PLAYED,									mult_prof->GetMultiplayerStatistics().mNumEventsPlayed);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_TOTAL_FANS,									mult_prof->GetMultiplayerStatistics().mNumLifetimeFans);
			
			for(int index=0;index < 6;index++)
			{
				const MultiplayerChallengePack* pPack = C_MultiplayerData::Instance().m_pConfig->m_ChallengePacks[index];
				const Game::C_MultiplayerChallengeSystem& challengeSystem = mult_prof->GetChallengeSystem();

				if(pPack)
				{
					float a=challengeSystem.GetPackProgressCounter( pPack->mType ) / (float)pPack->mChallenges.count;
					int b=a*100;

					Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite((Game::KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_CHALLENGE_PACK_1_PERCENTAGE_COMPLETE+index),	b);
				}
				else
				{
					Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite((Game::KEY_ARCHIVE_INDEX_KEY)(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_CHALLENGE_PACK_1_PERCENTAGE_COMPLETE+index),	0);
				}
			}

			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_SHOCK_FIRED,									mult_prof->GetMultiplayerStatistics().mPickupsFired[E_PICKUP_TYPE_SHOCK]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_SHUNT_FIRED,									mult_prof->GetMultiplayerStatistics().mPickupsFired[E_PICKUP_TYPE_SHUNT]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_BARGE_FIRED,									mult_prof->GetMultiplayerStatistics().mPickupsFired[E_PICKUP_TYPE_BARGE]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_BOMB_FIRED,									mult_prof->GetMultiplayerStatistics().mPickupsFired[E_PICKUP_TYPE_BOMB]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_BLINDFIRE_FIRED,								mult_prof->GetMultiplayerStatistics().mPickupsFired[E_PICKUP_TYPE_BLINDFIRE]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_SHOCK_HIT,									mult_prof->GetMultiplayerStatistics().mPickupsHit[E_PICKUP_TYPE_SHOCK]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_SHUNT_HIT,									mult_prof->GetMultiplayerStatistics().mPickupsHit[E_PICKUP_TYPE_SHUNT]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_BARGE_HIT,									mult_prof->GetMultiplayerStatistics().mPickupsHit[E_PICKUP_TYPE_BARGE]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_BOMB_HIT,										mult_prof->GetMultiplayerStatistics().mPickupsHit[E_PICKUP_TYPE_BOMB]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_BLINDFIRE_HIT,								mult_prof->GetMultiplayerStatistics().mPickupsHit[E_PICKUP_TYPE_BLINDFIRE]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_NITROUS_USED,									mult_prof->GetMultiplayerStatistics().mPickupsFired[E_PICKUP_TYPE_NITROUS]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_REPAIR_USED,									mult_prof->GetMultiplayerStatistics().mNumTimesRepaired);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_SHIELD_USED,									mult_prof->GetMultiplayerStatistics().mNumTimesShield);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_TOTAL_DISTANCE_TRAVELLED,						(S64)mult_prof->GetMultiplayerStatistics().mDistanceDriven);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_TOTAL_PLAYTIME,								mult_prof->GetMultiplayerStatistics().mTotalPlaytime);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_TIMES_USED_CAR_CLASS_A, 						mult_prof->GetMultiplayerStatistics().mCarClassesUsed[AmaxCarClass_A]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_TIMES_USED_CAR_CLASS_B, 						mult_prof->GetMultiplayerStatistics().mCarClassesUsed[AmaxCarClass_B]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_TIMES_USED_CAR_CLASS_C, 						mult_prof->GetMultiplayerStatistics().mCarClassesUsed[AmaxCarClass_C]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_TIMES_USED_CAR_CLASS_D, 						mult_prof->GetMultiplayerStatistics().mCarClassesUsed[AmaxCarClass_D]);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_TIMES_WRECKED,								mult_prof->GetMultiplayerStatistics().mNumTimesWrecked);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_TIMES_WRECKED_OTHERS,							mult_prof->GetMultiplayerStatistics().mNumTimesWreckedOtherDrivers);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_AVERAGE_POSITION,								mult_prof->GetMultiplayerStatistics().mAveragePosition * 100);
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_PLAYER_RANK,									mult_prof->GetCurrentLevel());
			Game::C_KeyArchiveManager::Instance().AddKeyArchivePairToWrite(KEY_ARCHIVE_INDEX_KEY_MULTI_PLAYER_LEGEND_LEVEL,									mult_prof->GetLegendLevel());

			Game::C_KeyArchiveManager::Instance().WriteKeyArchive();
		}
	}
}

}; // namespace Game

Debug::Int		g_debugDifficulty("Cheats.Difficulty", 0, Debug::Range(0, 3));

static void SetDifficulty(void*)
{
	if(g_debugDifficulty >= 0 && g_debugDifficulty <= 3)
		Game::C_GameProfileManager::Instance().GetPrimaryProfile()->SetDifficulty(g_debugDifficulty);
}

#ifndef CFG_GOLD
static void UnlockAllApps(void*)
{
	if( !Game::C_GameProfileManager::Instance().GetPrimaryProfile() )
	{
		LOGWARN( g_GameProfileLogger, "Profile not available - do one!");
		return;
	}
	
	Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mAppsLocked = 0;

	// Cause UI to reload the screen
	UI::gUIPopupControl.Exit();
	UI::gUIScreenControl.SetNextScreen( UI::gUIScreenControl.GetCurrentScreen() );
}

Debug::Function g_debugSetDifficulty("Cheats.UnlockAllApps", UnlockAllApps, 0);
#endif

