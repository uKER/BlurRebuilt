#ifndef UIXTCONFIG_H
#define UIXTCONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Device.h>
#include <input\GameInput.h>

#include "ui/Game/Gadgets/HUD/UIPerks2.h"
#include "Ui/Game/Multiplayer/MultiplayerDefines.h"

#include "GameLogic/RaceRules/RaceRuleBase.h"
#include "GameLogic/GameData/GameData.h"
#include "Game/GameControls/GameControls.h"

#include <vehicle/db/VehicleXt.h>

namespace UI
{

// ---------------------
// C_UiXtBloomParameters
// ---------------------
class C_UiXtBloomParameters
{
public:
	XtArray< XtF32 >	m_DownsampleScale;
	XtArray< XtF32 >	m_BloomScale;
};

// ----------------------------
// C_UiXtColourPickerParameters
// ----------------------------
class C_UiXtColourPickerParameters
{
public:
	S32		m_sBoxSize;
	S32		m_sSizeV;
	S32		m_sSpin;
};

// --------------------
// C_UiHudConfig_Pieces
// --------------------
class C_UiHudConfig_Pieces
{
public:
	bool		m_bCurrent;
	XtLocId		m_CurrentOverrideId;
	bool		m_bTarget;
	bool		m_bLapTimerOrTimeLimit;
	bool		m_bLapCounter;
};

// --------------------------
// C_UiHudConfig_SinglePlayer
// --------------------------
class C_UiHudConfig_SinglePlayer
{
public:
	Game::RaceModeRuleType		m_eRaceRule;
	C_UiHudConfig_Pieces		m_Pieces;
};

// -------------------------
// C_UiHudConfig_MultiPlayer
// -------------------------
class C_UiHudConfig_Multiplayer
{
public:
	E_RaceType					m_eRaceType;
	C_UiHudConfig_Pieces		m_Pieces;
};

// -------------
// C_UiHudConfig
// -------------
class C_UiHudConfig
{
public:
	XtArray< C_UiHudConfig_SinglePlayer >	m_SinglePlayer;
	XtArray< C_UiHudConfig_Multiplayer >	m_Multiplayer;
};

// ------------
// C_UiCameraInfo
// ------------
class C_UiCameraInfo
{
public:
	XtU32		m_ViewIndex;
	XtVec3		m_Position;
	XtVec3		m_LookAt;
};

class C_ControlConfig
{
public:
	void					SendToLua( lua_State* pState );
	void					RemapGameControls( Input::C_GameInput* pGameInput, Input::C_Device* pPad );

#if !defined(PLATFORM_PC)
private:
#endif
	// [Blur2PC] public and const on PC: retail's SetupAmaxUserControls (Game::SetupAmaxUserControlsPC) asks it
	// for each game button's pad input
	Input::C_ButtonInput*	CreateButtonInput( Input::C_Device* pPad, const U32 buttonId ) const;

public:
	XtLocId					m_Name;

	Game::E_Button			m_GameFunctionA;
	Game::E_Button			m_GameFunctionB;
	Game::E_Button			m_GameFunctionX;
	Game::E_Button			m_GameFunctionY;
	Game::E_Button			m_GameFunctionLT;
	Game::E_Button			m_GameFunctionRT;
	Game::E_Button			m_GameFunctionLB;
	Game::E_Button			m_GameFunctionRB;
	Game::E_Button			m_GameFunctionLThumb;
	Game::E_Button			m_GameFunctionRThumb;
	Game::E_Button			m_GameFunctionLStickX;
	Game::E_Button			m_GameFunctionLStickY;
	Game::E_Button			m_GameFunctionRStickX;
	Game::E_Button			m_GameFunctionRStickY;
};

// ------------------
// C_UiGarageResprays
// ------------------
class C_UiGarageResprays
{
public:
	XtArray< XtHandle< Vehicle::XtPaintJob > >		m_Solid_A;
	XtArray< XtHandle< Vehicle::XtPaintJob > >		m_Solid_B;
	XtArray< XtHandle< Vehicle::XtPaintJob > >		m_Solid_C;
	XtHandle< Vehicle::XtPaintJob >					m_Solid_Black;
	XtArray< XtHandle< Vehicle::XtPaintJob > >		m_Metallic;
	XtArray< XtHandle< Vehicle::XtPaintJob > >		m_Pearlecent;
	XtArray< XtHandle< Vehicle::XtPaintJob > >		m_Candy;
	XtArray< XtHandle< Vehicle::XtPaintJob > >		m_Splitscreen;

public:
	XtHandle< Vehicle::XtPaintJob >					Find( const XtId id );
};

// ------------------------
// C_UiHealthTextParamState
// ------------------------
class C_UiHealthTextParamState
{
public:
	F32						m_fLimit;
	XtArray< XtLocId >		m_Options;
};

// -------------------
// C_UiHealthTextParam
// -------------------
class C_UiHealthTextParam
{
public:
	E_HEALTH_LEVEL							m_eHealthLevel;
	XtArray< C_UiHealthTextParamState >		m_Params;
};

// ----------------------
// C_UiMirrorSmashParamUV
// ----------------------
class C_UiMirrorSmashParamUV
{
public:
	UI::C_TextureParam::C_Vector2UV		m_Position;
	UI::C_TextureParam::C_Vector2UV		m_Size;
};

// --------------------
// C_UiMirrorSmashParam
// --------------------
class C_UiMirrorSmashParam
{
public:
	F32							m_fLimit;
	C_UiMirrorSmashParamUV		m_Left;
	C_UiMirrorSmashParamUV		m_Right;
};

// ------------
// C_UiXtConfig
// ------------
class C_UiXtConfig
{
public:
	C_UiXtConfig() 
		: m_bDirty( false )
	{  }

public:
	bool								m_bDirty;

	C_UiGarageResprays					m_GarageResprays;
	C_UiXtBloomParameters				m_FrontendBloomParameters;
	C_UiXtBloomParameters				m_IngameBloomParameters;
	C_UiXtColourPickerParameters		m_ColourPickerParameters;
	C_PerkDisplayParameters				m_PerkDisplayParameters;
	C_UiHudConfig						m_HudConfig;
	XtArray< C_UiHealthTextParam >		m_HealthTextParams;
	XtArray< C_UiMirrorSmashParam >		m_MirrorSmashParams;
	XtVec3								m_CarPosition;
	XtF32								m_CarOrientation;
	XtArray<C_UiCameraInfo>				m_CameraInfo;
	XtHandle<AmaxInGameDialogue>		m_VoiceMailDialogueRef;
	XtArray<C_ControlConfig>			m_ControlConfigs;
	XtArray<XtSz8>						m_CommonShapes;
	XtArray<XtSz8>						m_FrontendShapes;
	XtArray<XtSz8>						m_MapShapes;
	// Race rule pulse config
	XtF32								m_PulseDefaultFreq;
	XtF32								m_PulseDriverHitDriverXFreq;	
	XtF32								m_PulseDriverHitDriverXWaitTime;
	XtF32								m_PulseEarnXpFreq;
	XtF32								m_PulseEarnXpWaitTime;

	C_UISpriteColours					m_TeamAColour;
	C_UISpriteColours					m_TeamBColour;

	XtHandle<AmaxVehicle>				m_DefaultVehicle;

public:
	static C_UiXtConfig*				Instance()		{ return m_pInstance; }
	static void							SorceryCallback( XtEventType type, XtRecord* pRecord, C_UiXtConfig* pBase, XtCallbackContext* pContext );
private:
	static C_UiXtConfig*				m_pInstance;
};

#if defined(PLATFORM_PC)
// [Blur2PC] retail Blur PC 0x00c2fe70: apply the controller's profile (keyboard layout, pad configuration) to its game input
void SetupPlayerControls( const S32 controller );
#endif

}; // namespace UI

#endif /* UIXTCONFIG_H */
