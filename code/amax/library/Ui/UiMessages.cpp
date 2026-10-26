#include "Stdafx.h"
#include "UiMessages.h"

// messages only need to be annotated if you intend to use them in Lua
XT_TYPE( UI::E_UiGameMsg, "UIGameMessage" )
	XT_OPTION( UI::E_UiGameMsg_OnEnterScreen,				"OnEnterScreen" )
	XT_OPTION( UI::E_UiGameMsg_OnExitScreen,				"OnExitScreen" )
	XT_OPTION( UI::E_UiGameMsg_ChangeContactsList,			"ChangeContactsList" )
	XT_OPTION( UI::E_UiGameMsg_SetLocalPlayerIndex,			"SetLocalPlayerIndex" )
	XT_OPTION( UI::E_UiGameMsg_Dialogue,					"Dialogue" )
	XT_OPTION( UI::E_UiGameMsg_NewRacebookMessage,			"NewRacebookMessage" )
	XT_OPTION( UI::E_UiGameMsg_RaceStartsIn,				"RaceStartsIn" )
	XT_OPTION( UI::E_UiGameMsg_MpRankedUp,					"MpRankedUp" )
	XT_OPTION( UI::E_UiGameMsg_MpChallengeComplete,			"MpChallengeComplete" )
	XT_OPTION( UI::E_UiGameMsg_SpCheckpointPassed,			"SpCheckpointPassed" )
	XT_OPTION( UI::E_UiGameMsg_SpCollectStopwatch,			"SpCollectStopwatch" )
	XT_OPTION( UI::E_UiGameMsg_SpDestructionPointsAdded,	"SpDestructionPointsAdded" )
	XT_OPTION( UI::E_UiGameMsg_SpDestructionBonusHitStart,	"SpDestructionBonusHitStart" )
	XT_OPTION( UI::E_UiGameMsg_SpDestructionBonusHitAdded,	"SpDestructionBonusHitAdded" )
	XT_OPTION( UI::E_UiGameMsg_SpDestructionBonusHitEnd,	"SpDestructionBonusHitEnd" )
	XT_OPTION( UI::E_UiGameMsg_SpFanDemandFansAdded,		"SpFanDemandFansAdded" )
	XT_OPTION( UI::E_UiGameMsg_RefreshLobby,				"RefreshLobby" )
	XT_OPTION( UI::E_UiGameMsg_PrimeLastGasp,				"PrimeLastGasp" )
	XT_OPTION( UI::E_UiGameMsg_DisarmLastGasp,				"DisarmLastGasp" )
	XT_OPTION( UI::E_UiGameMsg_UsePickup,					"UsePickup" )
	XT_OPTION( UI::E_UiGameMsg_DisplayECMWarning,           "DisplayECMWarning")
	XT_OPTION( UI::E_UiGameMsg_RemoveECMWarning,            "RemoveECMWarning")
	XT_OPTION( UI::E_UiGameMsg_NewRivalSelected,            "NewRivalSelected")
	XT_OPTION( UI::E_UiGameMsg_UIAudioPreLoaded,			"UIAudioPreLoaded")
	XT_OPTION( UI::E_UiGameMsg_UISceneReady,				"UISceneReady")
	
XT_END_TYPE

namespace UI
{

void RegisterGameMessageDebug()
{
#ifndef CFG_GOLD
	// register message debug info
	U32 uNumMessagesRegistered = 0;

	_UI_GAME_MESSAGE_DEBUG( OnEnterScreen );
	_UI_GAME_MESSAGE_DEBUG( OnExitScreen );
	_UI_GAME_MESSAGE_DEBUG( EnableToolBar );
	_UI_GAME_MESSAGE_DEBUG( ChangeContactsList );
	_UI_GAME_MESSAGE_DEBUG( NewsAdded );
	_UI_GAME_MESSAGE_DEBUG( IngameRenderAA );
	_UI_GAME_MESSAGE_DEBUG( UiCreateResources );
	_UI_GAME_MESSAGE_DEBUG( UiDestroyResources );
	_UI_GAME_MESSAGE_DEBUG( VehicleInRangeMessage );
	_UI_GAME_MESSAGE_DEBUG( PlayerInRangeWarningMessage );
	_UI_GAME_MESSAGE_DEBUG( DirectionBoxTrigger );
	_UI_GAME_MESSAGE_DEBUG( HitByPerk );
	_UI_GAME_MESSAGE_DEBUG( ScoringEventChain );
	_UI_GAME_MESSAGE_DEBUG( ScoringEventPerk );
	_UI_GAME_MESSAGE_DEBUG( ScoringEventFans );
	_UI_GAME_MESSAGE_DEBUG( ScoringEventMinor );
	_UI_GAME_MESSAGE_DEBUG( SetLocalPlayerIndex );
	_UI_GAME_MESSAGE_DEBUG( RaceStartsIn );
	_UI_GAME_MESSAGE_DEBUG( StartedAPerk );
	_UI_GAME_MESSAGE_DEBUG( PickedupAPerk );
	_UI_GAME_MESSAGE_DEBUG( ShufflePerks );
	_UI_GAME_MESSAGE_DEBUG( Dialogue );
	_UI_GAME_MESSAGE_DEBUG( FanDemand );
	_UI_GAME_MESSAGE_DEBUG( ProgressMessage );
	_UI_GAME_MESSAGE_DEBUG( BossDemandIncrement );
	_UI_GAME_MESSAGE_DEBUG( PerkGifted );
	_UI_GAME_MESSAGE_DEBUG( NewRacebookMessage );
	_UI_GAME_MESSAGE_DEBUG( ShowFanScoring );
	_UI_GAME_MESSAGE_DEBUG( InviteReceived );
	_UI_GAME_MESSAGE_DEBUG( LobbyConnectionLost );
	_UI_GAME_MESSAGE_DEBUG( MpRankedUp );
	_UI_GAME_MESSAGE_DEBUG( MpChallengeComplete );
	_UI_GAME_MESSAGE_DEBUG( SpCheckpointPassed );
	_UI_GAME_MESSAGE_DEBUG( SpCollectStopwatch );
	_UI_GAME_MESSAGE_DEBUG( SpDestructionPointsAdded );
	_UI_GAME_MESSAGE_DEBUG( SpDestructionBonusHitStart );
	_UI_GAME_MESSAGE_DEBUG( SpDestructionBonusHitAdded );
	_UI_GAME_MESSAGE_DEBUG( SpDestructionBonusHitEnd );
	_UI_GAME_MESSAGE_DEBUG( SpFanDemandFansAdded );
	_UI_GAME_MESSAGE_DEBUG( RefreshLobby );
	_UI_GAME_MESSAGE_DEBUG( PrimeLastGasp );
	_UI_GAME_MESSAGE_DEBUG( DisarmLastGasp );
	_UI_GAME_MESSAGE_DEBUG( UsePickup );
	_UI_GAME_MESSAGE_DEBUG( DisplayECMWarning );
	_UI_GAME_MESSAGE_DEBUG( RemoveECMWarning );
	_UI_GAME_MESSAGE_DEBUG( NewRivalSelected );
	_UI_GAME_MESSAGE_DEBUG( UIAudioPreLoaded );
	_UI_GAME_MESSAGE_DEBUG( UISceneReady );

	// if this asserts here then someone has changed E_UiGameMsg - you need to add some _UI_GAME_MESSAGE_DEBUG calls above
	BIZ_ASSERT( uNumMessagesRegistered == ( E_UiGameMsg_Max - ( E_UiGameMsg_Start + 1 ) ) );
#endif
}

};	// namespace UI
