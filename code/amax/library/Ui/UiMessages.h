#ifndef UIMESSAGES_H
#define UIMESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/util/widestring.h"

#include <ui/Game/Gadgets/ButtonController.h>
#include <ui/Game/Gadgets/ButtonMessages.h>

#include "GameLogic/ActorStudioActors/ResourceTypes.h"
#include "gamelogic/racelogiccore/data/RLEnums.h"

#include "GameLogic/GameData/GameDataDefs.h"

#include "UI/Game/Multiplayer/MultiplayerDefines.h"

#include <Game/Achievements/AchievementEnums.h>

namespace UI
{

//
void RegisterGameMessageDebug();
//

enum E_UiGameMsg
{
	E_UiGameMsg_Start = E_UiMsg_Count,

	// Script based messages
	E_UiGameMsg_OnEnterScreen,		// Subscreen messages so that the screen can
	E_UiGameMsg_OnExitScreen,		// resume its original state.
	E_UiGameMsg_EnableToolBar,		// Turn the toolbar on/off.
	E_UiGameMsg_ChangeContactsList,	// Change the current viewing contacts list.
	E_UiGameMsg_NewsAdded,			// A new News item has been added.

	// DG - Render the UI into the AA layer after game has been drawn
	E_UiGameMsg_IngameRenderAA,

	// Buffers and resources need for frontend part of UI
	E_UiGameMsg_UiCreateResources,
	E_UiGameMsg_UiDestroyResources,

	// vehicle targeting messages
	E_UiGameMsg_VehicleInRangeMessage,
	E_UiGameMsg_PlayerInRangeWarningMessage,

	// vehicle hud direction messages
	E_UiGameMsg_DirectionBoxTrigger,

	// 'who hit who' with a perk
	E_UiGameMsg_HitByPerk,

	// scoring events
	E_UiGameMsg_ScoringEventChain,
	E_UiGameMsg_ScoringEventPerk,
	E_UiGameMsg_ScoringEventFans,
	E_UiGameMsg_ScoringEventMinor,

	E_UiGameMsg_SetLocalPlayerIndex,
	E_UiGameMsg_RaceStartsIn,

	E_UiGameMsg_ShufflePerks,
	E_UiGameMsg_StartedAPerk,
	E_UiGameMsg_PickedupAPerk,

	E_UiGameMsg_Dialogue,
	E_UiGameMsg_NewRacebookMessage,

	E_UiGameMsg_FanDemand,
	E_UiGameMsg_ShowFanScoring,

	E_UiGameMsg_ProgressMessage,
	E_UiGameMsg_BossDemandIncrement,

	E_UiGameMsg_PerkGifted,

	E_UiGameMsg_InviteReceived,
	E_UiGameMsg_LobbyConnectionLost,

	E_UiGameMsg_MpRankedUp,
	E_UiGameMsg_MpChallengeComplete,

	E_UiGameMsg_SpCheckpointPassed,
	E_UiGameMsg_SpCollectStopwatch,
	E_UiGameMsg_SpDestructionPointsAdded,
	E_UiGameMsg_SpDestructionBonusHitStart,
	E_UiGameMsg_SpDestructionBonusHitAdded,
	E_UiGameMsg_SpDestructionBonusHitEnd,
	E_UiGameMsg_SpFanDemandFansAdded,

	E_UiGameMsg_PrimeLastGasp,
	E_UiGameMsg_DisarmLastGasp,
	E_UiGameMsg_UsePickup,

	E_UiGameMsg_RefreshLobby,

	E_UiGameMsg_DisplayECMWarning,
	E_UiGameMsg_RemoveECMWarning,

	E_UiGameMsg_NewRivalSelected,

	E_UiGameMsg_UIAudioPreLoaded,
	E_UiGameMsg_UISceneReady,
	
	E_UiGameMsg_Max,
};

class C_UiVehicleInRangeMessage : public C_UiMessageDerived< C_UiVehicleInRangeMessage, E_UiGameMsg_VehicleInRangeMessage >
{
public:
	C_UiVehicleInRangeMessage( S8 sPlayerVehicleIndex, S8 sVehicleIndex ) 
		: m_sPlayerVehicleIndex( sPlayerVehicleIndex ), m_sVehicleIndex( sVehicleIndex )	{  }

public:
	S8		m_sPlayerVehicleIndex;
	S8		m_sVehicleIndex;
};

class C_UiNewRacebookMessage : public C_UiMessageDerived< C_UiVehicleInRangeMessage, E_UiGameMsg_NewRacebookMessage >
{
public:
	C_UiNewRacebookMessage( const bool isPrivate ) : m_IsPrivate( isPrivate ) { }
public:
	bool m_IsPrivate;
};

class C_UiPlayerInRangeWarningMessage : public C_UiMessageDerived< C_UiPlayerInRangeWarningMessage, E_UiGameMsg_PlayerInRangeWarningMessage >
{
public:
	C_UiPlayerInRangeWarningMessage( S8 sPlayerVehicleIndex, E_PICKUP_LOCK_ON eLockOnType ) 
		: m_sPlayerVehicleIndex( sPlayerVehicleIndex ), m_eLockOnType( eLockOnType ) {  }

public:
	S8					m_sPlayerVehicleIndex;
	E_PICKUP_LOCK_ON	m_eLockOnType;
};

class C_UiPlayerDirectionBoxTrigger : public C_UiMessageDerived< C_UiPlayerDirectionBoxTrigger, E_UiGameMsg_DirectionBoxTrigger >
{
public:
	C_UiPlayerDirectionBoxTrigger( S8 sVehicleIndex, bool bHasEntered, Game::E_TRIGGER_TYPE type ) : m_bHasEntered( bHasEntered ), m_sVehicleIndex( sVehicleIndex ), m_eType( type ) { }

public:
	S8						m_sVehicleIndex;
	bool					m_bHasEntered;
	Game::E_TRIGGER_TYPE	m_eType;
};

/*
class C_UiHitByPerkMsg : public C_UiMessageDerived< C_UiHitByPerkMsg, E_UiGameMsg_HitByPerk >
{
public:
	C_UiHitByPerkMsg( S8 sFromVehicleIndex, S8 sToVehicleIndex, E_PICKUP_TYPE eType ) : m_sFromVehicleIndex( sFromVehicleIndex ), m_sToVehicleIndex( sToVehicleIndex ), m_eType( eType ) { }

public:
	S8						m_sFromVehicleIndex;
	S8						m_sToVehicleIndex;
	E_PICKUP_TYPE			m_eType;
};
*/

class C_UiScoringEventMinor : public C_UiMessageDerived< C_UiScoringEventMinor, E_UiGameMsg_ScoringEventMinor >
{
public:
	C_UiScoringEventMinor( const WCHAR *str, const U8 vehicleIndex, const bool bWasMajor = false ) 
		: m_VehicleIndex( vehicleIndex )
		, m_bWasMajor( bWasMajor )
	{
		m_MinorString.Format(L"%s",str);
	}

public:
	U8						m_VehicleIndex;
	UI::C_WideString256		m_MinorString;
	bool					m_bWasMajor;
};

class C_UiScoringEventChain : public C_UiMessageDerived< C_UiScoringEventChain, E_UiGameMsg_ScoringEventChain >
{
public:
	C_UiScoringEventChain( const WCHAR *str, const S32 bonusFans, const U8 vehicleIndex )
		: m_BonusFans( bonusFans )
		, m_VehicleIndex( vehicleIndex )		
	{
		m_BonusString.Format(L"%s",str);
	}

public:
	U8						m_VehicleIndex;
	UI::C_WideString128		m_BonusString;
	S32						m_BonusFans;
};

class C_UiScoringEventPerk : public C_UiMessageDerived< C_UiScoringEventPerk, E_UiGameMsg_ScoringEventPerk >
{
public:
	C_UiScoringEventPerk( E_PICKUP_TYPE type, S8 vehicle_index, S8 firing_index, const WCHAR *str ) : 
		m_PickupType( type ),
		m_VehicleIndex( vehicle_index )
	{
		m_ScoreString.Format(L"%s",str);
		m_ScoreStringName.Clear();
	}

	// New FanMessage
	C_UiScoringEventPerk( const WCHAR *str, int comboNumber, int fansNumber, int fansTotal, S8 vehicle_index ) :
		m_ComboNumber( comboNumber ),
		m_FansNumber( fansNumber ),
		m_FansTotal( fansTotal ),
		m_VehicleIndex( vehicle_index )
	{
		m_ScoreString.Format(L"%s",str);
		m_ScoreStringName.Clear();
	}

	// New FanMessage with name
	C_UiScoringEventPerk( const WCHAR *str, const WCHAR *strname, int comboNumber, int fansNumber, int fansTotal, S8 vehicle_index ) :
		m_ComboNumber( comboNumber ),
		m_FansNumber( fansNumber ),
		m_FansTotal( fansTotal ),
		m_VehicleIndex( vehicle_index )
	{
		m_ScoreString.Format(L"%s",str);
		m_ScoreStringName.Format(L"%s",strname);
	}

public:
	E_PICKUP_TYPE			m_PickupType;
	S8						m_VehicleIndex;
	UI::C_WideString128		m_ScoreString;
	UI::C_WideString32		m_ScoreStringName;
	// New
	int						m_ComboNumber;
	int						m_FansNumber;
	int						m_FansTotal;
	
};

class C_UiScoringEventFans : public C_UiMessageDerived< C_UiScoringEventFans, E_UiGameMsg_ScoringEventFans >
{
public:
	C_UiScoringEventFans( const U32 fan_amount, const bool just_update, const U8 vehicleIndex ) 
		: m_FanAmount( fan_amount ), m_JustUpdate( just_update ), m_VehicleIndex( vehicleIndex )
	{
	}

public:
	bool	m_JustUpdate;
	U32		m_FanAmount;
	U8		m_VehicleIndex;
};

class C_UiProgressMessage : public C_UiMessageDerived< C_UiProgressMessage, E_UiGameMsg_ProgressMessage >
{
public:
	C_UiProgressMessage(const WCHAR *name, const WCHAR *desc, const F32 progress, const F32 goal, const U8 vehicleIndex, const bool oneShot = false, const Game::BLUR_ACHIEVEMENT_CATEGORY stickerCategory = Game::BLUR_ACHIEVEMENT_CATEGORY_NONE, const S8 stickerPart = -1 )
		: mProgress( progress ), mGoal( goal ), mOneShot( oneShot ), mVehicleIndex( vehicleIndex ), mStickerCategory( stickerCategory ), mStickerPart( stickerPart ), mChallengePack( E_ChallengePack_None )
	{
		mName.Format(L"%s", name);
		mDescription.Format(L"%s", desc);
	}

	C_UiProgressMessage(const WCHAR *name, const WCHAR *desc, const F32 progress, const F32 goal, const U8 vehicleIndex, const bool oneShot = false, const E_ChallengePack challengePack = E_ChallengePack_None )
		: mProgress( progress ), mGoal( goal ), mOneShot( oneShot ), mVehicleIndex( vehicleIndex ), mStickerCategory( Game::BLUR_ACHIEVEMENT_CATEGORY_NONE ), mStickerPart( -1 ), mChallengePack( challengePack )
	{
		mName.Format(L"%s", name);
		mDescription.Format(L"%s", desc);
	}

	UI::C_WideString128				mName;
	UI::C_WideString128				mDescription;
	F32								mProgress;
	F32								mGoal;
	bool							mOneShot;
	Game::BLUR_ACHIEVEMENT_CATEGORY	mStickerCategory;
	S8								mStickerPart;
	E_ChallengePack					mChallengePack;

	U8								mVehicleIndex;
};

class C_UiFanDemand : public C_UiMessageDerived< C_UiFanDemand, E_UiGameMsg_FanDemand >
{
public:
	C_UiFanDemand( const U32 fan_amount, const bool just_update, const U32 amount_left, const WCHAR *str, U8 vehicle_index, bool fan_run ) : 
				m_FanAmount( fan_amount ), 
				m_JustUpdate(just_update), 
				m_AmountLeft(amount_left),
				m_VehicleIndex(vehicle_index),
				m_FanRun(fan_run)
	{
		m_String.Format(L"%s",str);
	}

public:
	UI::C_WideString128				m_String;
	bool							m_JustUpdate;
	U32								m_AmountLeft;
	U32								m_FanAmount;
	U8								m_VehicleIndex;
	bool							m_FanRun;		
};

class C_UiShowFanScoring : public C_UiMessageDerived< C_UiShowFanScoring, E_UiGameMsg_ShowFanScoring >
{
public:
	C_UiShowFanScoring( const bool demands, const bool show ) : m_Demands( demands ), m_Show( show ) { }
public:
	bool			m_Demands;
	bool			m_Show;
};

class C_UiGameMsg_StartedAPerk : public C_UiMessageDerived< C_UiGameMsg_StartedAPerk, E_UiGameMsg_StartedAPerk >
{
public:
	C_UiGameMsg_StartedAPerk( S8 vehicle_index, E_PICKUP_TYPE type ) : mVehicleIndex( vehicle_index ), mType( type ) { }

public:
	S8						mVehicleIndex;
	E_PICKUP_TYPE			mType;
};

class C_UiGameMsg_PickedupAPerk : public C_UiMessageDerived< C_UiGameMsg_PickedupAPerk, E_UiGameMsg_PickedupAPerk >
{
public:
	C_UiGameMsg_PickedupAPerk( S8 vehicle_index, E_PICKUP_TYPE type ) : mVehicleIndex( vehicle_index ), mType( type ) { }

public:
	S8						mVehicleIndex;
	E_PICKUP_TYPE			mType;
};

class C_UiGameMsg_ShufflePerks : public C_UiMessageDerived< C_UiGameMsg_PickedupAPerk, E_UiGameMsg_ShufflePerks >
{
public:
	C_UiGameMsg_ShufflePerks( S8 vehicle_index, bool left ) : mVehicleIndex( vehicle_index ), mLeft( left ) { }

public:
	S8						mVehicleIndex;
	bool					mLeft;
};

class C_UiGameMsg_Dialogue : public C_UiMessageDerived< C_UiGameMsg_Dialogue, E_UiGameMsg_Dialogue >
{
public:
	C_UiGameMsg_Dialogue( const GAMEDATA_ID raceBookCharacterId, const C_TextStringID& locId )
		: m_RaceBookCharacterId( raceBookCharacterId )
		, m_LocId( locId )	{  }

public:
	GAMEDATA_ID			m_RaceBookCharacterId;
	C_TextStringID		m_LocId;
};

class C_UiGameMsg_IngameRenderAA : public C_UiMessageDerived< C_UiGameMsg_IngameRenderAA, E_UiGameMsg_IngameRenderAA >
{
public:
	C_UiGameMsg_IngameRenderAA( const bool postWorldRender ) : m_bPostWorldRender( postWorldRender ) {  }

public:
	bool				m_bPostWorldRender;
};

// perk gifted
class C_UiGameMsg_PerkGifted : public C_UiMessageDerived< C_UiGameMsg_PerkGifted, E_UiGameMsg_PerkGifted >
{
public:
	C_UiGameMsg_PerkGifted( const S32 sVehicleIndex, const E_MultiplayerVehicleAbilities eAbility, const S32 sSlot = -1 )
		: m_sVehicleIndex( sVehicleIndex )
		, m_eAbility( eAbility )
		, m_sSlot( sSlot )
	{
		//
	}

public:
	S32								m_sVehicleIndex;
	E_MultiplayerVehicleAbilities	m_eAbility;
	S32								m_sSlot;
};

};	// namespace UI

#endif /* UIMESSAGES_H */
