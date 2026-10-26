#include "Stdafx.h"
#include "UiXtConfig.h"
#include "VideoConfig.h"

#if defined(PLATFORM_PC)
// [Blur2PC] UI::SetupPlayerControls
#include <GameApp.h>
#include <Game/GameControls/GameControls.h>
#include <GameLogic/Profile/GameProfileManager.h>
#include <ui/Game/Profiles/ProfileInterface.h>
#endif

#include <input\PadVirtual.h>
#if defined(PLATFORM_PC)
#include <Input/Pad360.h>
#endif

#if defined(PLATFORM_PS3)
#include <Input/PadPS3.h>
#endif

#if defined(PLATFORM_PC)
#include <Input/KeyboardPC.h>
#include <Input/Mouse.h>
#endif

XT_TYPE( Game::E_Button, "GameButton" )
	XT_OPTION( Game::E_Button_Throttle, "Throttle" )
	XT_OPTION( Game::E_Button_Brake, "Brake" )
	XT_OPTION( Game::E_Button_Handbrake, "Handbrake" )
	XT_OPTION( Game::E_Button_GearUp, "GearUp" )
	XT_OPTION( Game::E_Button_GearDown, "GearDown" )
	XT_OPTION( Game::E_Button_ChangeView, "ChangeView" )
	XT_OPTION( Game::E_Button_ResetVehicle, "ResetVehicle" )
	XT_OPTION( Game::E_Button_UsePickup, "UsePickup" )
	XT_OPTION( Game::E_Button_DiscardPickup, "DiscardPickup" )
	XT_OPTION( Game::E_Button_SwitchPickup, "SwitchPickup" )
	XT_OPTION( Game::E_Button_UseShield, "UseShield" )
	XT_OPTION( Game::E_Button_SwitchPickupAlt, "SwitchPickupAlt" )
	XT_OPTION( Game::E_Button_UseSecondTierPickup, "UseSecondTierPickup" )
	XT_OPTION( Game::E_Button_UsePickupAlt, "UsePickupAlt" )
	
	XT_OPTION( Game::E_Button_FireSlot1, "FireSlot1" )
	XT_OPTION( Game::E_Button_FireSlot2, "FireSlot2" )
	XT_OPTION( Game::E_Button_FireSlot3, "FireSlot3" )
	XT_OPTION( Game::E_Button_SwitchPickupLeft, "SwitchPickupLeft" )
	
	XT_OPTION( Game::E_Button_Count, "None" ) //  Flagged as no action
XT_END_TYPE

XT_TYPE( UI::C_ControlConfig, "UiXtControlConfig" )
	XT_FIELD( m_Name, "name" )
	XT_FIELD( m_GameFunctionA, "FunctionA" )
	XT_FIELD( m_GameFunctionB, "FunctionB" )
	XT_FIELD( m_GameFunctionX, "FunctionX" )
	XT_FIELD( m_GameFunctionY, "FunctionY" )
	XT_FIELD( m_GameFunctionLT, "FunctionLT" )
	XT_FIELD( m_GameFunctionRT, "FunctionRT" )
	XT_FIELD( m_GameFunctionLB, "FunctionLB" )
	XT_FIELD( m_GameFunctionRB, "FunctionRB" )
	XT_FIELD( m_GameFunctionLThumb, "FunctionLThumb" )
	XT_FIELD( m_GameFunctionRThumb, "FunctionRThumb" )
XT_END_TYPE

XT_TYPE( UI::C_UiXtBloomParameters, "UiXtBloomParameters" )
	XT_FIELD( m_DownsampleScale, "DownsampleScale" )
	XT_FIELD( m_BloomScale, "BloomScale" )
XT_END_TYPE

XT_TYPE( UI::C_UiCameraInfo, "UiXtCameraInfo" )
	XT_FIELD( m_ViewIndex, "ViewIndex" )
	XT_FIELD( m_Position, "Position" )
	XT_FIELD( m_LookAt, "LookAt" )
XT_END_TYPE

XT_TYPE( UI::C_UiXtColourPickerParameters, "UiXtColourPickerParameters" )
	XT_FIELD( m_sBoxSize, "box_size" )
	XT_FIELD( m_sSizeV, "size_v" )
	XT_FIELD( m_sSpin, "spin" )
XT_END_TYPE

XT_TYPE( UI::C_UiHudConfig_Pieces, "UiHudConfig_Pieces" )
	XT_FIELD( m_bCurrent, "Current" )
	XT_FIELD( m_CurrentOverrideId, "CurrentOverrideLocId" )
	XT_FIELD( m_bTarget, "Target" )
	XT_FIELD( m_bLapTimerOrTimeLimit, "LapTimerOrTimeLimit" )
	XT_FIELD( m_bLapCounter, "LapCounter" )
XT_END_TYPE

XT_TYPE( UI::C_UiHudConfig_SinglePlayer, "UiHudConfig_SinglePlayer" )
	XT_FIELD( m_eRaceRule, "RaceRule" )
	XT_FIELD( m_Pieces, "Pieces" )
XT_END_TYPE

XT_TYPE( UI::C_UiHudConfig_Multiplayer, "UiHudConfig_MultiPlayer" )
	XT_FIELD( m_eRaceType, "RaceType" )
	XT_FIELD( m_Pieces, "Pieces" )
XT_END_TYPE

XT_TYPE( UI::C_UiHudConfig, "UiHudConfig" )
	XT_FIELD( m_SinglePlayer, "SinglePlayer" )
	XT_FIELD( m_Multiplayer, "MultiPlayer" )
XT_END_TYPE

XT_TYPE( UI::C_UiGarageResprays, "UiGarageResprays" )
	XT_FIELD( m_Solid_A, "Solid_A" )
	XT_FIELD( m_Solid_B, "Solid_B" )
	XT_FIELD( m_Solid_C, "Solid_C" )
	XT_FIELD( m_Solid_Black, "Solid_Black" )
	XT_FIELD( m_Metallic, "Metallic" )
	XT_FIELD( m_Pearlecent, "Pearlecent" )
	XT_FIELD( m_Candy, "Candy" )
	XT_FIELD( m_Splitscreen, "Splitscreen" )
XT_END_TYPE

XT_TYPE( UI::C_UiHealthTextParamState, "UiHealthTextParamState" )
	XT_FIELD( m_fLimit, "Limit" )
	XT_FIELD( m_Options, "Options" )
XT_END_TYPE

XT_TYPE( UI::C_UiHealthTextParam, "UiHealthTextParam" )
	XT_FIELD( m_eHealthLevel, "HealthZone" )
	XT_FIELD( m_Params, "Params" )
XT_END_TYPE

XT_TYPE( UI::C_UiMirrorSmashParamUV, "UiMirrorSmashParamUV" )
	XT_FIELD( m_Position, "pos" )
	XT_FIELD( m_Size, "size" )
XT_END_TYPE

XT_TYPE( UI::C_UiMirrorSmashParam, "UiMirrorSmashParam" )
	XT_FIELD( m_fLimit, "Limit" )
	XT_FIELD( m_Left, "Left" )
	XT_FIELD( m_Right, "Right" )
XT_END_TYPE

XT_TYPE( UI::C_UiXtConfig, "UiXtConfig" )
	XT_CALLBACK( UI::C_UiXtConfig::SorceryCallback )
	XT_FIELD( m_GarageResprays, "GarageResprays" )
	XT_FIELD( m_FrontendBloomParameters, "FrontendBloomParameters" )
	XT_FIELD( m_IngameBloomParameters, "IngameBloomParameters" )
	XT_FIELD( m_ColourPickerParameters, "ColourPickerParameters" )
	XT_FIELD( m_PerkDisplayParameters, "PerkDisplayParameters" )
	XT_FIELD( m_HudConfig, "HudConfig" )
	XT_FIELD( m_HealthTextParams, "HealthTextParams" )
	XT_FIELD( m_MirrorSmashParams, "MirrorSmashParams" )
	XT_FIELD( m_CarPosition, "CarPosition" )
	XT_FIELD( m_CarOrientation, "CarOrientation" )
	XT_FIELD( m_CameraInfo, "CameraInfo" )
	XT_FIELD( m_VoiceMailDialogueRef, "VoiceMails" )
	XT_FIELD( m_ControlConfigs, "ControlConfigs" )
	XT_FIELD( m_CommonShapes, "CommonShapes" )
	XT_FIELD( m_FrontendShapes, "FrontendShapes" )
	XT_FIELD( m_MapShapes, "MapShapes" )
	XT_FIELD( m_PulseDefaultFreq, "PulseDefaultFreq" )
	XT_FIELD( m_PulseDriverHitDriverXFreq, "PulseHitDriverXFreq" )
	XT_FIELD( m_PulseDriverHitDriverXWaitTime, "PulseHitDriverXWaitTime" )
	XT_FIELD( m_PulseEarnXpFreq, "PulseEarnXpFreq" )
	XT_FIELD( m_PulseEarnXpWaitTime, "PulseXpWaitTime" )
	XT_FIELD( m_DefaultVehicle, "DefaultVehicle" )
XT_END_TYPE

// Video Config

VideoConfig* gUiVideoData;

void UiVideoConfigProc(XtEventType event, XtRecord* record, VideoConfig* data, XtCallbackContext* context)
{
	if (event == kXtEtConstruct)
	{
		gUiVideoData = data;
	}
	else if (event == kXtEtDestruct)
	{
		gUiVideoData = 0;
	}
}

XT_TYPE( GameModeVideo, "GameModeVideo")
	XT_FIELD(m_Video, "video" )
	XT_FIELD(m_CanSkip, "can_skip" )
	XT_FIELD(m_Fullscreen, "fullscreen" )
XT_END_TYPE

XT_TYPE( GameModeVideoConfig, "GameModeVideoConfig")
	XT_FIELD( m_GlossyIntro,"glossy_intro")
	XT_FIELD( m_RaceMode,"race_mode")
	XT_FIELD( m_DestructionMode,"destruction_mode")
	XT_FIELD( m_CheckpointMode,"checkpoint_mode")
	XT_FIELD( m_FanMode,"fan_mode")
	XT_FIELD( m_OneOnOne,"one_on_one")
	XT_FIELD( m_PowerUps,"power_ups")	
XT_END_TYPE

XT_TYPE( TutorialsVideoConfig, "TutorialsVideoConfig")
	XT_FIELD( m_Tutorial1,"tutorial_1")
	XT_FIELD( m_Tutorial2,"tutorial_2")
XT_END_TYPE

XT_TYPE( FanModeVideoConfig, "FanModeVideoConfig")
	XT_FIELD( m_FanPar,"fan_par")
	XT_FIELD( m_FanRun,"fan_run")
	XT_FIELD( m_FanDemand, "fan_demand")
XT_END_TYPE

XT_TYPE( EndOfGameVideoConfig, "EndOfGameVideoConfig")
	XT_FIELD( m_EndCredits,"end_credits")
	XT_FIELD( m_Phonecall,"phonecall")
XT_END_TYPE

XT_TYPE( VideoConfig, "VideoConfig")
	XT_CALLBACK(UiVideoConfigProc)
	XT_FIELD( m_GameModes,"game_modes")
	XT_FIELD( m_Tutorials,"tutorials")
	XT_FIELD( m_FanModes,"fan_modes")
	XT_FIELD( m_EndOfGame,"end_of_game")
	XT_FIELD( m_TierUnlock,"tier_unlock")
	XT_FIELD( m_BossUnlock,"boss_unlock")
	XT_FIELD( m_BossBeaten,"boss_beaten")
XT_END_TYPE	

XT_LIBRARY(VideoConfig)
	XT_REGISTER_TYPE(GameModeVideo)
	XT_REGISTER_TYPE(GameModeVideoConfig)
	XT_REGISTER_TYPE(TutorialsVideoConfig)
	XT_REGISTER_TYPE(FanModeVideoConfig)
	XT_REGISTER_TYPE(EndOfGameVideoConfig)
	XT_REGISTER_TYPE(VideoConfig)
XT_END_LIBRARY

namespace UI
{

void C_ControlConfig::RemapGameControls( Input::C_GameInput* pGameInput, Input::C_Device* pPad )
{
	if( !pGameInput || !pPad )
		return;

	// Remap
	for( U32 i = Framework::E_Button_Count; i < Game::E_Button_Count; i++ )
	{
		bool changeControl = true;

		// Don't change the photo mode controls:
		switch (i)
		{
			case Game::E_Button_PhotoRoll:
			case Game::E_Button_PhotoZoom:
			case Game::E_Button_PhotoRollReset:
			case Game::E_Button_PhotoZoomReset:
				changeControl = false;
			break;
		}

		if (changeControl)
			pGameInput->SetButtonInput( i, CreateButtonInput( pPad, i ) );
	}
}

Input::C_ButtonInput* C_ControlConfig::CreateButtonInput( Input::C_Device* pPad, const U32 buttonId ) const	// [Blur2PC] const
{
#if defined(PLATFORM_PS3)
	if( m_GameFunctionA == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_Cross );
	if( m_GameFunctionB == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_Circle );
	if( m_GameFunctionX == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_Square );
	if( m_GameFunctionY == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_Triangle );
	if( m_GameFunctionLT == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_L2 );
	if( m_GameFunctionRT == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_R2 );
	if( m_GameFunctionLB == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_L1 );
	if( m_GameFunctionRB == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_R1 );
	if( m_GameFunctionLThumb == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_L3 );
	if( m_GameFunctionRThumb == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_PadPS3::E_Button_R3 );
#elif defined(PLATFORM_PC)
	if( m_GameFunctionA == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_A );
	if( m_GameFunctionB == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_B );
	if( m_GameFunctionX == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_X );
	if( m_GameFunctionY == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_Y );
	if( m_GameFunctionLT == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_LeftTrigger );
	if( m_GameFunctionRT == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_RightTrigger );
	if( m_GameFunctionLB == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_LeftShoulder );
	if( m_GameFunctionRB == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_RightShoulder );
	if( m_GameFunctionLThumb == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_LeftThumb );
	if( m_GameFunctionRThumb == buttonId )
		return new Input::C_ButtonInputFromDevice(pPad, Input::C_Pad360::E_Button_RightThumb );
#endif
	// Clear the mapping for this function
	return NULL;
}

void C_ControlConfig::SendToLua( lua_State* pState )
{
	// Name of this config
	Lua::AddDataMember( pState, "name", m_Name.id );

	// Send the loc strings for each function
	const U32 start = Framework::E_Button_Count;

	static UI::C_TextStringID function_names[ (Game::E_Button_Count - start) + 1 ] =
	{
		UI::C_TextStringID( "CMN_OPT_CNT_THROTTLE" ),			// E_Button_Throttle
		UI::C_TextStringID( "CMN_OPT_CNT_BRAKE" ),				// E_Button_Brake,
		UI::C_TextStringID( "CMN_OPT_CNT_E_BRAKE" ),			// E_Button_Handbrake,
		UI::C_TextStringID( "CMN_OPT_CNT_GEAR_UP" ),			// E_Button_GearUp,
		UI::C_TextStringID( "CMN_OPT_CNT_GEAR_DOWN" ),			// E_Button_GearDown,
		UI::C_TextStringID( "CMN_OPT_CNT_CHANGE_VIEW" ),		// E_Button_ChangeView,
		UI::C_TextStringID( "CMN_OPT_CNT_RESET_VEHICLE" ),		// E_Button_ResetVehicle,
		
		UI::C_TextStringID( "CMN_OPT_CNT_FIRE_PERK" ),			// E_Button_UsePickup,
		UI::C_TextStringID( "CMN_OPT_CNT_DISCARD_PERK" ),		// E_Button_DiscardPickup,
		UI::C_TextStringID( "CMN_OPT_CNT_SWITCH_PERK" ),		// E_Button_SwitchPickup,
		UI::C_TextStringID( "CMN_OPT_CNT_SWITCH_PERK_LEFT" ),	// E_Button_SwitchPickupLeft
		UI::C_TextStringID( "CMN_OPT_CNT_SWITCH_PERK" ),		// E_Button_SwitchPickupAlt
		UI::C_TextStringID( "CMN_OPT_CNT_SHIELD" ),				// E_Button_UseShield,
		UI::C_TextStringID( "CMN_OPT_CNT_SWITCH_PERK_RIGHT" ),	// E_Button_UseSecondTierPickup,
		
		UI::C_TextStringID( "CMN_OPT_CNT_NONE" ),				// E_Button_Auto_Shuffle_Perks
		UI::C_TextStringID( "CMN_OPT_CNT_NONE" ),				// E_Button_Convert_To_Health
		UI::C_TextStringID( "CMN_OPT_CNT_NONE" ),				// E_Button_Clone_Selected_Perk
		UI::C_TextStringID( "CMN_OPT_CNT_NONE" ),				// E_Button_Fullhouse

		UI::C_TextStringID( "CMN_OPT_CNT_FIRE_PERK" ),			// E_Button_UsePickupAlt
		UI::C_TextStringID( "CMN_OPT_CNT_NONE" ),				// E_Button_PhotoRoll
		UI::C_TextStringID( "CMN_OPT_CNT_NONE" ),				// E_Button_PhotoZoom
		UI::C_TextStringID( "CMN_OPT_CNT_NONE" ),				// E_Button_PhotoRollReset
		UI::C_TextStringID( "CMN_OPT_CNT_NONE" ),				// E_Button_PhotoZoomReset

		UI::C_TextStringID( "CMN_OPT_CNT_FIRE_SLOT1" ),				// E_Button_FireSlot1
		UI::C_TextStringID( "CMN_OPT_CNT_FIRE_SLOT2" ),				// E_Button_FireSlot2
		UI::C_TextStringID( "CMN_OPT_CNT_FIRE_SLOT3" ),				// E_Button_FireSlot3
	};

	Lua::AddDataMember( pState, "ButtonsA", function_names[ m_GameFunctionA - start ].Value() );
	Lua::AddDataMember( pState, "ButtonsB", function_names[ m_GameFunctionB - start ].Value() );
	Lua::AddDataMember( pState, "ButtonsX", function_names[ m_GameFunctionX - start ].Value() );
	Lua::AddDataMember( pState, "ButtonsY", function_names[ m_GameFunctionY - start ].Value() );
	Lua::AddDataMember( pState, "ButtonsLT", function_names[ m_GameFunctionLT - start ].Value() );
	Lua::AddDataMember( pState, "ButtonsRT", function_names[ m_GameFunctionRT - start ].Value() );
	Lua::AddDataMember( pState, "ButtonsLB", function_names[ m_GameFunctionLB - start ].Value() );
	Lua::AddDataMember( pState, "ButtonsRB", function_names[ m_GameFunctionRB - start ].Value() );
	Lua::AddDataMember( pState, "ButtonsLThumb", function_names[ m_GameFunctionLThumb - start ].Value() );
	Lua::AddDataMember( pState, "ButtonsRThumb", function_names[ m_GameFunctionRThumb - start ].Value() );
}

// ------------------
// C_UiGarageResprays
// ------------------
XtHandle< Vehicle::XtPaintJob > C_UiGarageResprays::Find( const XtId id )
{
	// TODO - map it?

	if ( id == m_Solid_Black.Id() )
		return m_Solid_Black;

	for ( S32 i = 0; i < m_Solid_A.count; i++ )
		if ( id == m_Solid_A[ i ].Id() )
			return m_Solid_A[ i ];

	for ( S32 i = 0; i < m_Solid_B.count; i++ )
		if ( id == m_Solid_B[ i ].Id() )
			return m_Solid_B[ i ];

	for ( S32 i = 0; i < m_Solid_C.count; i++ )
		if ( id == m_Solid_C[ i ].Id() )
			return m_Solid_C[ i ];

	for ( S32 i = 0; i < m_Metallic.count; i++ )
		if ( id == m_Metallic[ i ].Id() )
			return m_Metallic[ i ];

	for ( S32 i = 0; i < m_Pearlecent.count; i++ )
		if ( id == m_Pearlecent[ i ].Id() )
			return m_Pearlecent[ i ];

	for ( S32 i = 0; i < m_Candy.count; i++ )
		if ( id == m_Candy[ i ].Id() )
			return m_Candy[ i ];

	for ( S32 i = 0; i < m_Splitscreen.count; i++ )
		if ( id == m_Splitscreen[ i ].Id() )
			return m_Splitscreen[ i ];

	return XtHandle< Vehicle::XtPaintJob >();
}

C_UiXtConfig* C_UiXtConfig::m_pInstance = NULL;

void C_UiXtConfig::SorceryCallback( XtEventType type, XtRecord* pRecord, C_UiXtConfig* pBase, XtCallbackContext* pContext )
{
	switch ( type )
	{
		case kXtEtConstruct:
			m_pInstance = pBase;
			m_pInstance->m_bDirty = true;
			break;

		case kXtEtDestruct:
			m_pInstance = NULL;
			break;

		default:
			break;
	}
}

#if defined(PLATFORM_PC)
// [Blur2PC] Retail Blur PC applies a player's controls from the profile (Blur.exe 1.2, 0x00c2fe70): the keyboard
// layout chosen in Options > Keyboard and the pad mapping of the profile's controller configuration, through
// SetupAmaxUserControls. Nothing is done while every pad drives the menus (AllowAllPadInput). Called where the
// snapshot remapped the pad (C_GameProfile::SetSystemDefaults, Amax.Options).
void SetupPlayerControls( const S32 controller )
{
	Game::C_GameProfile* pProfile = Game::C_GameProfileManager::Instance().GetProfile( controller );
	C_UiXtConfig* pUiConfig = C_UiXtConfig::Instance();
	if( !pProfile || !pUiConfig || C_ProfileInterface::Instance().AllowAllPadInput() )
		return;

	C_GameApp* pApp = C_GameApp::GetInstance();
	Game::SetupAmaxUserControlsPC(	Input::GetGameInput( controller ), pApp->GetPad( controller ), pApp->GetKeyboard( controller ), pApp->GetMouse( controller ),
									pProfile->mOptions.mKeyboardLayout, false, &pUiConfig->m_ControlConfigs[ (S32)pProfile->mOptions.mControlIndex ], 0 );
}
#endif

}; // namespace UI
