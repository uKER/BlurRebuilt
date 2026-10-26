

#ifndef UI_UI_GAME_DEFINES_H
#define UI_UI_GAME_DEFINES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui\UIDefines.h"

namespace UI
{

/*---------------------------------------------------------------------------------------
	Game specific gadget types.
---------------------------------------------------------------------------------------*/
enum E_UIGameButtonType
{
	E_UIGameButtonType_Lobby = E_UIButtonType_EndMarker,
	E_UIGameButtonType_AmaxGraphic,
	E_UIGameButtonType_AmaxSlider,
	E_UIGameButtonType_Carousel,

	E_UIGameButtonType_Damage,
	E_UIGameButtonType_Perks,
	E_UIGameButtonType_Perks2,
	E_UIGameButtonType_PerkMessage,
	E_UIGameButtonType_FanMessage,
	E_UIGameButtonType_RequirementsMessage,
	E_UIGameButtonType_HudMessages,
	E_UIGameButtonType_RivalHud,
	E_UIGameButtonType_CarNode,
	E_UIGameButtonType_LocatorNode,
	E_UIGameButtonType_CutoutText,
	E_UIGameButtonType_Targeting,
	E_UIGameButtonType_AiHud,
	E_UIGameButtonType_DirectionHud,
	E_UIGameButtonType_Shield,
	E_UIGameButtonType_HudDummy,
	E_UIGameButtonType_RankBar,
	E_UIGameButtonType_LoadoutController,
	E_UIGameButtonType_RaceLine,
	E_UIGameButtonType_RaceLineSplitscreen,
	E_UIGameButtonType_SpeedZone,
	E_UIGameButtonType_MultiplayerData,
	E_UIGameButtonType_MultiplayerSpeaking,
	E_UIGameButtonType_Checkpoint,
	E_UIGameButtonType_MinorScoreLog,
	E_UIGameButtonType_TeamTugOfWar,

	E_UIGameButtonType_VinylPreview,
	E_UIGameButtonType_VinylScene,
	E_UIGameButtonType_VinylStripBar,
	E_UIGameButtonType_ColourPicker,

	E_UIGameButtonType_FocalPoint,
	E_UIGameButtonType_AmaxLight,
	E_UIGameButtonType_LightPaint,
	E_UIGameButtonType_TabbedList,
	E_UIGameButtonType_TabNode,
	E_UIGameButtonType_PostProcessTexture,

	E_UIGameButtonType_Equaliser,
	E_UIGameButtonType_Checkbox,
	E_UIGameButtonType_SplicedText,
	E_UIGameButtonType_SpeakerSetup,
	E_UIGameButtonType_Chicklets,
	E_UIGameButtonType_ReinforceText,
	E_UIGameButtonType_MiniMap,
	E_UIGameButtonType_Toggle,
	E_UIGameButtonType_RacePreview,

	E_UIGameButtonType_EndMarker, // Incase more user types.
};

};


#endif // UI_UI_GAME_DEFINES_H
