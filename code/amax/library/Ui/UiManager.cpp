#include "Stdafx.h"
#if PLATFORM_PC
#include "game/Gadgets/Hud/UIPerks2.h"	// [Blur2PC] car panel delta
#include "game/Gadgets/Hud/UIAiHud.h"	// [Blur2PC] AI badges at the cars' render pose
#endif

#ifdef PLATFORM_PS3
#define AVOID_ONE_POINT_SEVEN_SECOND_UI_BLOCK
#endif

#include <system/time/scopedtimer.h>

#include <GameSystems.h>

#include "UiManager.h"
#include "UiXtConfig.h"

#include <Camera2/Camera.h>
#include <Display/Bundler/Bundler.h>
#include <display2/utils/Utils.h>
#include <system/memory/MemoryTracker.h>

//#include "game.h"
//#include "GameScripting/LuaSys.h"
//

#include <Game/GameControls/GameControls.h>
#include <Game/Game.h>

#include <ui/DashboardInterface.h>
#include <ui/GameSaveSystem.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Game/Gadgets/ButtonController.h>
#include <ui/Game/StringStore.h>
#include <ui/Game/UIScreenControl.h>
#include <ui/Game/UILocaliser.h>
#include <ui/Game/UILua.h>
#include <ui/Game/UIMaths.h>
#include <ui/Game/UIXt.h>
#include <ui/Game/Gadgets/UICamera.h>
#include <ui/Game/Gadgets/UICredits.h>
#include <ui/Game/Gadgets/UILegal.h>
#include <ui/Game/Gadgets/UIToggle.h>
#include <ui/Graphics/Font/Font.h>
#include <ui/Graphics/Font/FontStyle.h>
#include <ui/Graphics/Font/UIPolygonFont.h>
#include <ui/Graphics/Movie.h>
#include <ui/Graphics/UITextureManager.h>
#include <ui/Graphics/Localisation/Loc.h>
#include <ui/Graphics/SimpleTimer.h>
#include <ui/Graphics/UIParticle.h>
#include <ui/Illustrator/UIAdobeSceneManager.h>
#include <ui/UiMessages.h>
#include <ui/Game/UiXt.h>
#include <ui/Game/Profiles/ProfileInterface.h>
#include <ui/Game/Profiles/ProfileInterfaceMessages.h>
#include <ui/Game/Profiles/GamerPictureManager.h>
#include <SystemProfile/SystemProfileManager.h>
#include <SystemProfile/GameInvite.h>
#include <Network/Presence.h>
#include <Network/Securekey.h>
#include <Network/Core/NetRichPresence.h>
#include <Ui/LightPaintSystem.h>

#include "../Game/GameWorld/GameWorld.h"
#include "Game/Messaging/GameflowMessages.h"
#include "GameLogic\ActorStudioActors\DynamicObjectMessages.h"
#include "Game\Messaging\VehicleActorMessages.h"
#include "GameLogic\Messaging\GameLogicMessages.h"

#include "Network/Core/NetServices.h"
#include "Network/Router/NetRouterBase.h"
#include "Network/Core/NetRace.h"
#include "Network/Core/NetParty.h"
#include <bdCore/bdCore.h>

#include <GameLogic/ActorStudioActors/CutSceneMessages.h>
#include <GameLogic/FriendsChallenges/FriendChallenges.h>
#include <GameLogic/RaceRules/RaceRulesManager.h>
#include <Gamelogic/SocialNetworking/SocialNetworkManager.h>

#include <GameApp.h>

#include <input/Device.h>
#include <input/Keyboard.h>	// [Blur2PC] identify the keyboard join key in retail split-screen
#include <framework/application/ApplicationBase.h>

#include "Ui/SoundEffects.h"
#include "Ui/Game/Multiplayer/MultiplayerDefines.h"
#include "Ui/Game/Multiplayer/MultiplayerHelpers.h"

#include "Game/GameActors/Components/Messages/VehicleLogicMessages.h"

#include "RaceBook/Data/RaceBookData.h"
#include "UiHelpers.h"

#include "AppPhotoCapture.h"

#include "Advertising/Advertising.h"
#include "Game/GameActors/Components/Messages/AudioMessages.h"
#include "GameAudio/UISubsystem.h"
#include "GameAudio/GameAudioLayer.h"

#include <Graphics/HDR.h>
#include <Network\Core\NetScoreboards.h>

#include <Game/Achievements/AchievementEnums.h>
#include "RaceBook/RaceBookManager.h"

//#include <ui/Graphics/UILoading.h>
//#include <ui/Graphics/UIMiniMap.h>
//#include <ui/Graphics/UITextureLoad.h>
//
//#include "Graphics/Interface_MultiTexture.h"
//#include "Graphics/Interface_PixelLight.h"
//#include "Graphics/Interface_FontEffects.h"
//#include "Graphics/Interface_NormalMapOffset.h"

//#include "Graphics/Simple3d.h"

//#include "Graphics/ModelManager.h"
//#include "Game/Playlists.h"
//
//#include <Common/Memory/MemHeap.h>
//#include <common/memory/memheap.inl>
//#include "Memory/OverloadNew.h"
//
//#include "ui/Shaders/Constants.h"
//#include "Camera/Camera.h"
//
//#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
//#include "Audio\UiSection.h"
//#endif
//
//#ifdef PLATFORM_PS3
//#include "Game/PS3SaveSystem.h"
//#include <cell/pad.h>
//#include <sysutil/sysutil_common.h>
//#include <sysutil/sysutil_sysparam.h>
//#endif

// datafinders
#include "GameDataFinder.h"
#include "HudDataFinder.h"
#include "MpDataFinder.h"
#include "SpDataFinder.h"
#include "LobbyDataFinder.h"
#include "LanDataFinder.h"
#include "MatchingDataFinder.h"
#include <ui/Game/Profiles/ProfileDataFinder.h>

// profiles
#include <ui/Game/Profiles/ProfileInterface.h>

/*---------------------------------------------------------------------------------------
	Game Gadget includes
---------------------------------------------------------------------------------------*/
#include "Game/Gadgets/UIAmaxLight.h"
#include "Game/Gadgets/UILightPaint.h"
#include "Game/Gadgets/UICarousel.h"
#include "Game/Gadgets/UILobby.h"
#include "Game/Gadgets/UIAmaxGraphic.h"
#include "Game/Gadgets/UIAmaxSlider.h"
#include "Game/Gadgets/UIFocalPoint.h"
#include "Game/Gadgets/UIPostProcessTexture.h"
#include "Game/Gadgets/UICheckbox.h"
#include "Game/Gadgets/UISpeakerSetup.h"
#include "Game/Gadgets/UIChicklets.h"
#include "Game/Gadgets/UIReinforceText.h"
#include "Game/Gadgets/UILocatorNode.h"
#include "Game/Gadgets/UICarNode.h"
#include "Game/Gadgets/UIRacePreview.h"
#include "Game/Gadgets/Hud/UIDamage.h"
#include "Game/Gadgets/Hud/UIFanMessage.h"
#include "Game/Gadgets/Hud/UIRequirementsMessage.h"
#include "Game/Gadgets/Hud/UILoadoutController.h"
#include "Game/Gadgets/Hud/UIMiniMap.h"
#include "Game/Gadgets/Hud/UIPerks.h"
#include "Game/Gadgets/Hud/UIPerks2.h"
#include "Game/Gadgets/Hud/UIHudDummy.h"
#include "Game/Gadgets/Hud/UIRankBar.h"
#include "Game/Gadgets/Hud/UITargeting.h"
#include "Game/Gadgets/Hud/UIAiHud.h"
#include "Game/Gadgets/Hud/UIShield.h"
#include "Game/Gadgets/Hud/UIRaceLine.h"
#include "Game/Gadgets/Hud/UIRaceLineSplitscreen.h"
#include "Game/Gadgets/Hud/UIMultiplayerData.h"
#include "Game/Gadgets/Hud/UIMultiplayerSpeaking.h"
#include "Game/Gadgets/Hud/UICheckpoint.h"
#include "Game/Gadgets/Hud/UIMinorScoreLog.h"
#include "Game/Gadgets/Hud/UITeamTugOfWar.h"
#include "Game/Gadgets/Vinyl/UIColourPicker.h"

#include "SoundEffects.h"
#include <GameLogic/Layers/RaceLogicLayer.h>	// [Blur2PC] automation race state
#include <GameLogic/Layers/RaceBookLayer.h>	// [Blur2PC]
#include <system/debug/Blur2PCAutomation.h>	// [Blur2PC]
#include "Ui/UiLayer.h"

#include "Game/AmaxCam/CameraSwitcher.h"

#include "GameLogic/GameData/LocData.h"

#ifndef CFG_GOLD
#include <Automation/GameDataLogging.h>
#endif

#ifdef PLATFORM_PS3
#include <PS3Installer/PS3InstallData.h>
#endif


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

Debug::Bool g_UiRunDeferred("UI.MultiThreaded", false);

Debug::Float g_UiDebugAudioEqDamp( "UI.Render.AudioEqDamp", 25.0f, Debug::Range( 0.0f, 100.0f ) );

namespace UI { void DeferredUpdate(Scheduler::Task *pTask); }

JQ_DECLARE_PPU_MODULE(UiUpdateModule, UI::DeferredUpdate);

#ifndef CFG_GOLD
DEBUG_ENUM(UI::C_UiManager::E_LaunchMode)
	DE_OPTION(UI::C_UiManager::E_LaunchMode_Master,			"Master")		// Master version			: GOLD
	DE_OPTION(UI::C_UiManager::E_LaunchMode_Final,			"Final")		// Final version			: All features and final game structure (Denug)
	DE_OPTION(UI::C_UiManager::E_LaunchMode_Debug,			"Debug")		// Dev'						: Developer Debug : Boot into debug screen
	DE_OPTION(UI::C_UiManager::E_LaunchMode_DebugRaceBook,	"DebugRb")		// Dev Frontend				: RaceBook Debug : Don't play first race
	DE_OPTION(UI::C_UiManager::E_LaunchMode_Demo,			"Demo")			// LIVE + PSN + Coverdisk	: (TBC)
	DE_OPTION(UI::C_UiManager::E_LaunchMode_Automation,		"Automation")		// Debug
END_DEBUG_ENUM

DEBUG_ENUM(UI::E_UILanguage)
	DE_OPTION(UI::E_UILanguage_Other,		"None")
	DE_OPTION(UI::E_UILanguage_English,		"English")
	DE_OPTION(UI::E_UILanguage_French,		"French")
	DE_OPTION(UI::E_UILanguage_German,		"German")
	DE_OPTION(UI::E_UILanguage_Italian,		"Italian")
	DE_OPTION(UI::E_UILanguage_Spanish,		"Spanish")
#if defined (PLATFORM_PC)
	DE_OPTION(UI::E_UILanguage_Russian,		"Russian")
	DE_OPTION(UI::E_UILanguage_Polish,		"Polish")
#else
	DE_OPTION(UI::E_UILanguage_Japanese,	"Japanese")
#endif
END_DEBUG_ENUM

// Debug hook
static UI::C_ScreenControl*	gpUiScreen = NULL;
static UI::C_ScreenControl*	gpUiPopup = NULL;

extern Debug::Bool g_debugUseLUAFromHDD;
extern Debug::Bool g_UICameraDebugActive2D;
extern Debug::Bool g_UICameraDebugActive3D;

Debug::Float g_UiLightOffset( "UI.LightOffset", 2.0f, Debug::Range( -20.0f, 20.0f ) );

Debug::Bool g_Net_ShowBandwidthUsage("Network.Core.ShowBandwidthUsage", false);

Debug::Enum<UI::E_UILanguage> g_UiDebugOverideLanguage("UI.LanguageOveride", UI::E_UILanguage_Other, Debug::Persist() );
Debug::Enum<UI::C_UiManager::E_LaunchMode>	g_UiDebugLaunchMode("Ui.NewLaunchMode", UI::C_UiManager::E_LaunchMode_Master, Debug::Persist() );

Debug::Bool	g_UiDebugReloadLocs("UI.Localisation.EnableLocReload", false, Debug::Persist() );

Debug::Bool g_debugUpdateUICamera("UI.UpdateCamera", false);
Debug::Int g_debugUICameraIndex("UI.CameraIndex", 0);

Debug::Bool g_UiDebugLogAudioTriggers( "UI.DebugLogAudioTriggers", false, Debug::Persist() );
Debug::Bool g_UiProcessController("UI.ProcessController", true);

Debug::Bool g_UiShouldRenderPanel_2D( "UI.Panel.2D", true );
Debug::Bool g_UiShouldRenderPanel_2DAA( "UI.Panel.2DAA", true );
Debug::Bool g_UiShouldRenderPanel_3DAA_WORLD( "UI.Panel.3DAA_WORLD", true );
Debug::Bool g_UiShouldRenderPanel_3D_WORLD( "UI.Panel.3D_WORLD", true );
Debug::Bool g_UiShouldRenderPanel_3DAA_LIGHT( "UI.Panel.3DAA_LIGHT", true );
Debug::Bool g_UiShouldRenderPanel_2DBG( "UI.Panel.2DBG", true );
Debug::Bool g_UiShouldRenderPanel_3DAA_0( "UI.Panel.3DAA_0", true );
Debug::Bool g_UiShouldRenderPanel_3DAA_1( "UI.Panel.3DAA_1", true );
Debug::Bool g_UiShouldRenderPanel_3D( "UI.Panel.3D", true );
Debug::Bool g_UiShouldRenderPanel_VP0( "UI.Panel.VP0", true );
Debug::Bool g_UiShouldRenderPanel_VP1( "UI.Panel.VP1", true );
Debug::Bool g_UiShouldRenderPanel_VP2( "UI.Panel.VP2", true );
Debug::Bool g_UiShouldRenderPanel_VP3( "UI.Panel.VP3", true );
Debug::Bool g_UiShouldRenderPanel_2DIG_A( "UI.Panel.2D_A", true );
Debug::Bool g_UiShouldRenderPanel_2DIG_B( "UI.Panel.2D_B", true );
#endif

// ***

void C_GameFileSystem_UpdateSystemMessages(){}

// ***
#if 0 // [Blur2PC][x64] was #ifdef WIN64: the blobs are C definitions (luabundle.py, gen_cmake.py) named _binary_* on every arch
#define UIBIN_START			__binary_Amax_LUA_bin_start
#define UIBIN_END			__binary_Amax_LUA_bin_end
#else
#define UIBIN_START			_binary_Amax_LUA_bin_start
#define UIBIN_END			_binary_Amax_LUA_bin_end
#endif

#ifdef PLATFORM_PS3
namespace SonyNetworkPlatform
{
	extern const SceNpCommunicationId amax_network_platform_communication_id;
}
#endif // PLATFORM_PS3

extern "C" char UIBIN_START[], UIBIN_END[];

/*---------------------------------------------------------------------------------------
	Register gadget types
---------------------------------------------------------------------------------------*/
XT_TYPE( UI::E_UiSoundEffect, "UISoundEffect" )
	// frontend sounds
	XT_OPTION( UI::E_UiSoundEffect_PressStartActionIn,					"Start" )								// when you are asked to press start at the beginning of the game
	XT_OPTION( UI::E_UiSoundEffect_Forwards,							"Forward" )								// Accept � going through into another window/screen
	XT_OPTION( UI::E_UiSoundEffect_Back,								"Backward" )							// Cancel � going back to a previous screen
	XT_OPTION( UI::E_UiSoundEffect_Error,								"Error" )								// When the user has made an error or you can�t move forward
	XT_OPTION( UI::E_UiSoundEffect_LeftActionIn,						"Left" )								// Choosing to go left on the screen
	XT_OPTION( UI::E_UiSoundEffect_RightActionIn,						"Right" )								// Choosing to go right on the screen
	XT_OPTION( UI::E_UiSoundEffect_Move,								"Move" )								// Moving up and down
	XT_OPTION( UI::E_UiSoundEffect_Toggle,								"Toggle" )								// selecting between options or values
	XT_OPTION( UI::E_UiSoundEffect_ModShopAttachActionIn,				"ModShopAttach" )						// attaching a mod
	XT_OPTION( UI::E_UiSoundEffect_ModShopRemoveActionIn,				"ModShopRemove" )					//X	// removing a mod
	XT_OPTION( UI::E_UiSoundEffect_GarageSwitchCarActionIn,				"GarageSwitchCar" )						// switching car choice
	XT_OPTION( UI::E_UiSoundEffect_GarageSwitchCarLockedActionIn,		"GarageSwitchCarLocked" )				// switching car choice to a locked car
	XT_OPTION( UI::E_UiSoundEffect_GarageSelectActionIn,				"GarageSelect" )						// select garage
	XT_OPTION( UI::E_UiSoundEffect_GarageEnterActionIn,					"GarageEnter" )							// entering garage
	XT_OPTION( UI::E_UiSoundEffect_GarageExitActionIn,					"GarageExit" )							// exiting garage
	XT_OPTION( UI::E_UiSoundEffect_GarageReSprayActionIn,				"GarageReSpray" )						// respraying car
	XT_OPTION( UI::E_UiSoundEffect_GarageColourPickerMoveActionIn,		"GarageColourPickerMove" )				// moving around the colour chooser
	XT_OPTION( UI::E_UiSoundEffect_GarageFinishStyleSelect01ActionIn,	"GarageFinishStyleSelect01" )			// Solid left & right movement onto it
	XT_OPTION( UI::E_UiSoundEffect_GarageFinishStyleSelect02ActionIn,	"GarageFinishStyleSelect02" )			// metallic left & right movement onto it
	XT_OPTION( UI::E_UiSoundEffect_GarageFinishStyleSelect03ActionIn,	"GarageFinishStyleSelect03" )			// Pearlescent left & right movement onto it
	XT_OPTION( UI::E_UiSoundEffect_GarageFinishStyleSelect04ActionIn,	"GarageFinishStyleSelect04" )			// Candy left & right movement onto it
	XT_OPTION( UI::E_UiSoundEffect_GarageSelectVehicleActionIn,			"GarageSelectVehicle" )					// When selecting your vehicle
	XT_OPTION( UI::E_UiSoundEffect_LobbyJoinActionIn,					"LobbyJoin" )							// joining the lobby
	XT_OPTION( UI::E_UiSoundEffect_LobbyLeaveActionIn,					"LobbyLeave" )							// leaving the lobby
	XT_OPTION( UI::E_UiSoundEffect_LobbyCountDown10ActionIn,			"LobbyCountDown10" )					// any digit countdown before last 3
	XT_OPTION( UI::E_UiSoundEffect_LobbyCountDown03ActionIn,			"LobbyCountDown03" )					// last 3 digits of countdown
	XT_OPTION( UI::E_UiSoundEffect_LobbyVoteCountActionIn,				"LobbyVoteCount" )						// The vote counter adding one
	XT_OPTION( UI::E_UiSoundEffect_ChallengeNewItemActionIn,			"ChallengeNewItem" )				//X	// a new challenge item
	XT_OPTION( UI::E_UiSoundEffect_EnrouteToNextEventActionIn,			"EnrouteToNextEvent" )					// When the enroute to next event screen is shown
	XT_OPTION( UI::E_UiSoundEffect_TabsOpen,							"TabsOpen" )							// when all the square tabs spread
	XT_OPTION( UI::E_UiSoundEffect_GraphicTextBoxActionIn,				"GraphicTextBox" )						// popup
	XT_OPTION( UI::E_UiSoundEffect_GraphicForwardActionIn,				"GraphicForward" )						// graphics motion in, like on the options screen enter
	XT_OPTION( UI::E_UiSoundEffect_GraphicBackwardActionIn,				"GraphicBackward" )						// graphics motion out, like on the options screen exit
	XT_OPTION( UI::E_UiSoundEffect_GraphicPowerButtonActionIn,			"GraphicPowerButton" )					// this needs a hook when it and the title "blur" appear and pulses
	XT_OPTION( UI::E_UiSoundEffect_GraphicMenuTopActionIn,				"GraphicMenuTop" )						// in MP, there is a blue bar that enters and fills the top of the screen, just before the �TabsOpen� sound plays
	XT_OPTION( UI::E_UiSoundEffect_GraphicStartFlyForwardActionIn,		"GraphicStartFlyForward" )				// start button screen fly forward
	XT_OPTION( UI::E_UiSoundEffect_GraphicStartFlyBackwardActionIn,		"GraphicStartFlyBackward" )				// fly back to start button screen
	XT_OPTION( UI::E_UiSoundEffect_TutorialSplashScreen01ActionIn,		"TutorialSplashScreen01" )
	XT_OPTION( UI::E_UiSoundEffect_TutorialSplashScreen02ActionIn,		"TutorialSplashScreen02" )
	XT_OPTION( UI::E_UiSoundEffect_TutorialSplashScreen03ActionIn,		"TutorialSplashScreen03" )
	XT_OPTION( UI::E_UiSoundEffect_PressStartButtonScreenActionIn,		"PressStartButtonScreen" )				// on start screen startup

	// hud and ingame sounds
	XT_OPTION( UI::E_UiSoundEffect_RaceGoActionIn,						"RaceStart" )							// Triggered at start of race � �Go!� essentially
	XT_OPTION( UI::E_UiSoundEffect_RaceStartCountdownActionIn,			"RaceStartCountdown" )					// 3-2-1 intro countdown
	XT_OPTION( UI::E_UiSoundEffect_LockHardActionIn,					"LockHard" )							// hardlock on player
	XT_OPTION( UI::E_UiSoundEffect_LockSoftActionIn,					"LockSoft" )						//X	// softlockonplayer
	XT_OPTION( UI::E_UiSoundEffect_LockHardAIActionIn,					"LockHardAI" )							// hardlockon AI
	XT_OPTION( UI::E_UiSoundEffect_LockSoftAIActionIn,					"LockSoftAI" )						//X	// softlockonAI
	XT_OPTION( UI::E_UiSoundEffect_HealthLowActionIn,					"HealthLow" )							// When player health is low
	XT_OPTION( UI::E_UiSoundEffect_ModUse01ActionIn,					"ModUse01" )							// use mod ingame
	XT_OPTION( UI::E_UiSoundEffect_ModUse02ActionIn,					"ModUse02" )							// use mod ingame
	XT_OPTION( UI::E_UiSoundEffect_ModUse03ActionIn,					"ModUse03" )							// use mod ingame
	XT_OPTION( UI::E_UiSoundEffect_ModDropActionIn,						"ModDrop" )							//X	// dropping a mod
	XT_OPTION( UI::E_UiSoundEffect_BankChainActionIn,					"BankChain" )							// Chain amount being banked
	XT_OPTION( UI::E_UiSoundEffect_ChallengeCompleteActionIn,			"ChallengeComplete" )					// Completion of a challenge
	XT_OPTION( UI::E_UiSoundEffect_RankUpActionIn,						"RankUp" )								// Player ranks up
	XT_OPTION( UI::E_UiSoundEffect_FansActionIn,						"Fans" )								// This is for fans chaining so +2 +5 +80 etc
	XT_OPTION( UI::E_UiSoundEffect_ChainMultiplierActionIn,				"ChainMultiplier" )						// When it goes from 1-2-3-4-5
	XT_OPTION( UI::E_UiSoundEffect_CountDownActionIn,					"CountDown" )							// Timer countdown
	XT_OPTION( UI::E_UiSoundEffect_CountDown03ActionIn,					"CountDown03" )							// Timer countdown - last 3 seconds
	XT_OPTION( UI::E_UiSoundEffect_BankFanActionIn,						"BankFan" )								// Number of fans being banked
	XT_OPTION( UI::E_UiSoundEffect_MessageOtherActionIn,				"MessageOther" )						// when you get told you�ve shunted/barged/drifted/360
	XT_OPTION( UI::E_UiSoundEffect_PerkSwitchActionIn,					"PerkSwitch" )						//X	// switch perk sound
	XT_OPTION( UI::E_UiSoundEffect_EliminationCountDown10ActionIn,		"EliminationCountDown10" )			//X	// First 7 out of 10 seconds
	XT_OPTION( UI::E_UiSoundEffect_EliminationCountDown03ActionIn,		"EliminationCountDown03" )			//X	// Last 3 seconds
	XT_OPTION( UI::E_UiSoundEffect_FansCountUpActionIn,					"FansCountUp" )						//X	// Fans counting up
	XT_OPTION( UI::E_UiSoundEffect_FansTotalSmallActionIn,				"FansTotalSmall" )					//X	// Small total of fans
	XT_OPTION( UI::E_UiSoundEffect_FansTotalMediumActionIn,				"FansTotalMedium" )					//X	// medium total of fans
	XT_OPTION( UI::E_UiSoundEffect_FansTotalLargeActionIn,				"FansTotalLarge" )					//X	// large total of fans
	XT_OPTION( UI::E_UiSoundEffect_RankCountUpActionIn,					"RankCountUp" )						//X	// rank count up for every fan?
	XT_OPTION( UI::E_UiSoundEffect_RankNextActionIn,					"RankNext" )						//X	// reaching next rank sound from rank 2-3
	XT_OPTION( UI::E_UiSoundEffect_PlayerFirstActionIn,					"PlayerFirst" )						//X	// Player first position
	XT_OPTION( UI::E_UiSoundEffect_PlayerSecondActionIn,				"PlayerSecond" )					//X	// Player second position
	XT_OPTION( UI::E_UiSoundEffect_PlayerThirdActionIn,					"PlayerThird" )						//X	// Player 3rd position
	XT_OPTION( UI::E_UiSoundEffect_MotorMashCountActionIn,				"MotorMashCount" )						// For kill count number
	XT_OPTION( UI::E_UiSoundEffect_MotorMashLightImpactActionIn,		"MotorMashLightImpact" )				// For motormash
	XT_OPTION( UI::E_UiSoundEffect_MotorMashHeavyImpactActionIn,		"MotorMashHeavyImpact" )				// For motormash
	XT_OPTION( UI::E_UiSoundEffect_MotorMashWreckedActionIn,			"MotorMashWrecked" )					// For motormash
	XT_OPTION( UI::E_UiSoundEffect_WreckedActionIn,						"WreckedActionIn" )						// For kill count number	
	XT_OPTION( UI::E_UiSoundEffect_MPFocusOnWinnerActionIn,				"MPFocusOnWinner" )						// when the camera zooms onto the winning player (from the start of the sequence)
	XT_OPTION( UI::E_UiSoundEffect_MPPositionInActionIn,				"MPPositionIn" )						// the white text telling the player their position (from the text just about appearing)
	XT_OPTION( UI::E_UiSoundEffect_MPNumberGlowActionIn,				"MPNumberGlow" )						// For the 1, 2 or 3 numbers that come across the screen (form the swirly-ness just about appearing)
	XT_OPTION( UI::E_UiSoundEffect_MPFinishInActionIn,					"MPFinishIn" )							// when �Finish� zooms to the front of the screen (from start of sequence)
	XT_OPTION( UI::E_UiSoundEffect_MPRankAwayActionIn,					"MPRankAway" )							// 
	XT_OPTION( UI::E_UiSoundEffect_MPRankInActionIn,					"MPRankIn" )							// 
	XT_OPTION( UI::E_UiSoundEffect_GiftHealthActionIn,					"GiftHealth" )							// when a Mod gives you health
	XT_OPTION( UI::E_UiSoundEffect_GiftPowerUpActionIn,					"GiftPowerUp" )							// when a Mod gives you a perk
	XT_OPTION( UI::E_UiSoundEffect_IronFistActivateActionIn,			"IronFist" )							// iron fist mod is activated
	XT_OPTION( UI::E_UiSoundEffect_DiscardPickupActionIn,				"DiscardPickup" )						// press Y and drop pickup
	XT_OPTION( UI::E_UiSoundEffect_DiscardDecoyPickupActionIn,			"DiscardDecoyPickup" )					// drop decoy
	XT_OPTION( UI::E_UiSoundEffect_PreviouslyOnBlurActionIn,			"PreviouslyOnBlur" )					// PoB
	XT_OPTION( UI::E_UiSoundEffect_DestructionPassActionIn,				"DestructionPass" )						// SP destruction mode pass
	XT_OPTION( UI::E_UiSoundEffect_DestructionFailActionIn,				"DestructionFail" )						// SP destruction mode fail
	XT_OPTION( UI::E_UiSoundEffect_DestructionWreckedActionIn,			"DestructionWrecked" )					// SP destruction mode wreck
	XT_OPTION( UI::E_UiSoundEffect_DestructionWreckLongshotActionIn,	"DestructionWreckLongshot" )			// SP destruction mode longshot wreck
	XT_OPTION( UI::E_UiSoundEffect_PickupsFireErrorActionIn,			"PickupsFireErrorActionIn" )			// error sound when you attempt to fire a pickup but are not allowed
	XT_OPTION( UI::E_UiSoundEffect_LastGaspInitiatedActionIn,			"LastGaspInitiatedActionIn" )			// last gasp timer has statred
	XT_OPTION( UI::E_UiSoundEffect_ScramblerActivatedActionIn,			"ScramblerActivatedActionIn" )			// you've been scrambled
	XT_OPTION( UI::E_UiSoundEffect_ScramblerDeactivatedActionIn,		"ScramblerDeactivatedActionIn" )		// you can fire powerUps again
	XT_OPTION( UI::E_UiSoundEffect_PassedCheckpointActionIn,			"PassedCheckpoint" )					// 
	XT_OPTION( UI::E_UiSoundEffect_PassedStartLineActionIn,				"PassedStartLine" )						// 
	XT_OPTION( UI::E_UiSoundEffect_PassedFanRunCheckpointActionIn,		"PassedFanRunCheckpoint" )				// 
	XT_OPTION( UI::E_UiSoundEffect_StopwatchCollectedActionIn,			"StopwatchCollected" )					// 
	XT_OPTION( UI::E_UiSoundEffect_MedalIncreaseActionIn,				"MedalIncrease" )						// light glow when getting a light
	XT_OPTION( UI::E_UiSoundEffect_MedalDecreaseActionIn,				"MedalDecrease" )						// going down a light level all at once
	XT_OPTION( UI::E_UiSoundEffect_MedalInThreatActionIn,				"MedalInThreat" )						// every frame when light in checkpoint is draining
	XT_OPTION( UI::E_UiSoundEffect_LapIncrementedActionIn,				"LapIncremented" )						// 
	XT_OPTION( UI::E_UiSoundEffect_LastLapActionIn,						"LastLap" )								// 
	XT_OPTION( UI::E_UiSoundEffect_PerksGoActionIn,						"PerksGo" )								// 
	XT_OPTION( UI::E_UiSoundEffect_PerksCountdownActionIn,				"PerksCountdown" )						// 
	XT_OPTION( UI::E_UiSoundEffect_RaceCountdownActionIn,				"RaceCountdown" )						// 
	XT_OPTION( UI::E_UiSoundEffect_FanDemandPickupActionIn,				"FanDemandPickup" )						// 
	XT_OPTION( UI::E_UiSoundEffect_FanDemandCountdownActionIn,			"FanDemandCountdown" )					// 
	XT_OPTION( UI::E_UiSoundEffect_FanDemandSuccessActionIn,			"FanDemandSuccess" )					// 
	XT_OPTION( UI::E_UiSoundEffect_FanDemandFailActionIn,				"FanDemandFail" )						// 
	XT_OPTION( UI::E_UiSoundEffect_FansSmallCheerActionIn,				"FansSmallCheer" )						// 
	XT_OPTION( UI::E_UiSoundEffect_FansMidCheerActionIn,				"FansMidCheer" )						// 
	XT_OPTION( UI::E_UiSoundEffect_FansLargeCheerActionIn,				"FansLargeCheer" )						// 
	XT_OPTION( UI::E_UiSoundEffect_PhotoTakenActionIn,					"PhotoTaken" )							// 
	XT_OPTION( UI::E_UiSoundEffect_FansCountingActionIn,				"FansCounting" )						// 
	XT_OPTION( UI::E_UiSoundEffect_Route66ActionIn,						"USA_Amboy_1" )							// 
	XT_OPTION( UI::E_UiSoundEffect_TumbleweedAlleyActionIn,				"USA_Amboy_3" )							// Badlands Traverse?
	XT_OPTION( UI::E_UiSoundEffect_PasseigDeGraciaActionIn,				"SPA_Barcelona_Gracia_1" )				// 
	XT_OPTION( UI::E_UiSoundEffect_ElCarmelHeightsActionIn,				"SPA_Barcelona_Gracia_2" )				// 
	XT_OPTION( UI::E_UiSoundEffect_CatalanClimbActionIn,				"SPA_Barcelona_Gracia_5" )				// 
	XT_OPTION( UI::E_UiSoundEffect_CityBreachActionIn,					"SPA_Barcelona_Oval_1" )				// 
	XT_OPTION( UI::E_UiSoundEffect_SpeedwayActionIn,					"SPA_Barcelona_Oval_2" )				// 
	XT_OPTION( UI::E_UiSoundEffect_PromenadeLoopActionIn,				"UK_Brighton_1" )						// 
	XT_OPTION( UI::E_UiSoundEffect_SeafrontStripActionIn,				"UK_Brighton_2" )						// 
	XT_OPTION( UI::E_UiSoundEffect_CoastalCruiseActionIn,				"UK_Brighton_3" )						// 
	XT_OPTION( UI::E_UiSoundEffect_PacificReachActionIn,				"USA_LA_LongBeachDocks_2" )				// 
	XT_OPTION( UI::E_UiSoundEffect_CargoRunActionIn,					"USA_LA_LongBeachDocks_3" )				// 
	XT_OPTION( UI::E_UiSoundEffect_TheMoneyRunActionIn,					"USA_LA_Downtown_1" )					// 
	XT_OPTION( UI::E_UiSoundEffect_HighRiseRingActionIn,				"USA_LA_Downtown_4" )					// 
	XT_OPTION( UI::E_UiSoundEffect_HarborFreewayActionIn,				"USA_LA_Downtown_6" )					// 
	XT_OPTION( UI::E_UiSoundEffect_ShoreditchSweepSoundActionIn,		"UK_London_Hackney_1" )					// 
	XT_OPTION( UI::E_UiSoundEffect_UrbanBeltActionIn,					"UK_London_Hackney_2" )					// 
	XT_OPTION( UI::E_UiSoundEffect_CentralSprintActionIn,				"UK_London_Hackney_4" )					// 
	XT_OPTION( UI::E_UiSoundEffect_HollywoodRiftActionIn,				"USA_LA_Observatory_1" )				// 
	XT_OPTION( UI::E_UiSoundEffect_DowntownVistaActionIn,				"USA_LA_Observatory_2" )				// 
	XT_OPTION( UI::E_UiSoundEffect_ConcreteBasinActionIn,				"USA_LA_River_3" )						// 
	XT_OPTION( UI::E_UiSoundEffect_StormdrainSurgeActionIn,				"USA_LA_River_6" )						// 
	XT_OPTION( UI::E_UiSoundEffect_AscentActionIn,						"JPN_MHaruna_1" )						// Descent?
	XT_OPTION( UI::E_UiSoundEffect_ManhattanViewActionIn,				"USA_NY_Dumbo_2" )						// 
	XT_OPTION( UI::E_UiSoundEffect_BrooklynTourActionIn,				"USA_NY_Dumbo_3" )						// 
	XT_OPTION( UI::E_UiSoundEffect_RussianSteppesActionIn,				"USA_SanFran_RussianHill_1" )			// 
	XT_OPTION( UI::E_UiSoundEffect_GoldenGateRushActionIn,				"USA_SanFran_Sausalito_1" )				// 
	XT_OPTION( UI::E_UiSoundEffect_BayAreaTourActionIn,					"USA_SanFran_Sausalito_2" )				// 
	XT_OPTION( UI::E_UiSoundEffect_BayShoreRouteActionIn,				"JPN_Tokyo_Shutoko_2" )					// 
	XT_OPTION( UI::E_UiSoundEffect_WangansenActionIn,					"JPN_Tokyo_Shutoko_4" )					// 
	XT_OPTION( UI::E_UiSoundEffect_ShannonActionIn,						"UnlockBoss1" )							// 
	XT_OPTION( UI::E_UiSoundEffect_KhanActionIn,						"UnlockBoss2" )							// 
	XT_OPTION( UI::E_UiSoundEffect_RhymerActionIn,						"UnlockBoss3" )							// 
	XT_OPTION( UI::E_UiSoundEffect_DraykeActionIn,						"UnlockBoss4" )							// 
	XT_OPTION( UI::E_UiSoundEffect_NatalyaActionIn,						"UnlockBoss5" )							// 
	XT_OPTION( UI::E_UiSoundEffect_TalonActionIn,						"UnlockBoss6" )							// 
	XT_OPTION( UI::E_UiSoundEffect_TwinsActionIn,						"UnlockBoss7" )							// 
	XT_OPTION( UI::E_UiSoundEffect_CarverActionIn,						"UnlockBoss8" )							// 
	XT_OPTION( UI::E_UiSoundEffect_EvilShannonActionIn,					"UnlockBoss9" )							// 
	XT_OPTION( UI::E_UiSoundEffect_PreviouslyRewardActionIn,			"PreviouslyReward" )					// 
	XT_OPTION( UI::E_UiSoundEffect_PreviouslyOutActionIn,				"PreviouslyOut" )						// 
	XT_OPTION( UI::E_UiSoundEffect_PreviouslySparksActionIn,			"PreviouslySparks" )					// 
	XT_OPTION( UI::E_UiSoundEffect_RaceTargetFailedActionIn,			"RaceTargetFailed" )					// 
	XT_OPTION( UI::E_UiSoundEffect_RaceTargetPassActionIn,				"RaceTargetPass" )						// 
	XT_OPTION( UI::E_UiSoundEffect_UpActionIn,							"Up" )									// 
	XT_OPTION( UI::E_UiSoundEffect_DownActionIn,						"Down" )								// 
	XT_OPTION( UI::E_UiSoundEffect_CarouselLeftActionIn,				"CarouselLeft" )						// 
	XT_OPTION( UI::E_UiSoundEffect_CarouselRightActionIn,				"CarouselRight" )						// 
	XT_OPTION( UI::E_UiSoundEffect_StickersPageActionIn,				"StickersPage" )						// 
	XT_OPTION( UI::E_UiSoundEffect_EndOfRacePauseActionIn,				"EndOfRacePause" )						// 	
	XT_OPTION( UI::E_UiSoundEffect_EndOfRaceUnPauseActionIn,			"EndOfRaceUnPause" )					// 	
	XT_OPTION( UI::E_UiSoundEffect_TabsCloseActionIn,					"TabsClose" )							// 
	XT_OPTION( UI::E_UiSoundEffect_FilterActionIn,						"Filter" )								// 
	XT_OPTION( UI::E_UiSoundEffect_FriendVs1ActionIn,					"FriendVs1" )							// 
	XT_OPTION( UI::E_UiSoundEffect_FriendVs2ActionIn,					"FriendVs2" )							// 
	XT_OPTION( UI::E_UiSoundEffect_FriendVs3ActionIn,					"FriendVs3" )							// 
	XT_OPTION( UI::E_UiSoundEffect_FriendTaleOfTheTapeActionIn,			"FriendTaleOfTheTape" )					// 
	XT_OPTION( UI::E_UiSoundEffect_BossDemandIncrementActionIn,			"BossDemandIncrement" )					// 
	XT_OPTION( UI::E_UiSoundEffect_BossDemandCompleteActionIn,			"BossDemandComplete" )					// 
	XT_OPTION( UI::E_UiSoundEffect_BossUnlockedActionIn,				"BossUnlocked" )						// 
	XT_OPTION( UI::E_UiSoundEffect_ChooseActionIn,						"Choose" )								// 
	XT_OPTION( UI::E_UiSoundEffect_CascadeActionIn,						"Cascade" )								// 
	XT_OPTION( UI::E_UiSoundEffect_BossRaceWinActionIn,					"BossRaceWin" )							// 
	XT_OPTION( UI::E_UiSoundEffect_BossRaceLoseActionIn,				"BossRaceLose" )						// 
	XT_OPTION( UI::E_UiSoundEffect_LoadingScreenNavActionIn,			"LoadingScreenNav" )					// 
	XT_OPTION( UI::E_UiSoundEffect_ApplyBossModActionIn,				"ApplyBossMod" )						// 
	XT_OPTION( UI::E_UiSoundEffect_HudProgressSlideActionIn,			"HudProgressSlide" )					// 
	XT_OPTION( UI::E_UiSoundEffect_HudFanComboEndActionIn,				"HudFanComboEnd" )						// 
	XT_OPTION( UI::E_UiSoundEffect_HudPositionIncrementActionIn,		"HudPositionIncrement" )				// 
	XT_OPTION( UI::E_UiSoundEffect_HudHealthLossActionIn,				"HudHealthLoss" )						// 
	XT_OPTION( UI::E_UiSoundEffect_HudFanComboActiveActionIn,			"HudFanComboActive" )					// 
	XT_OPTION( UI::E_UiSoundEffect_PostRacePassedLightTrailActionIn,	"PostRacePassedLightTrail" )			// 
	XT_OPTION( UI::E_UiSoundEffect_PostRacePassedActionIn,				"PostRacePassed" )						// 
	XT_OPTION( UI::E_UiSoundEffect_PostRaceFailedActionIn,				"PostRaceFailed" )						// 
	XT_OPTION( UI::E_UiSoundEffect_PostRaceBestActionIn,				"PostRaceBest" )						// 
	XT_OPTION( UI::E_UiSoundEffect_PostRaceNotBestActionIn,				"PostRaceNotBest" )						// 
	XT_OPTION( UI::E_UiSoundEffect_PostRaceGoalSpawnActionIn,			"PostRaceGoalSpawn" )					// 
	XT_OPTION( UI::E_UiSoundEffect_PostRaceLeaderboardSpawnActionIn,	"PostRaceLeaderboardSpawn" )			// 
	XT_OPTION( UI::E_UiSoundEffect_PostRaceNewUnlocksActionIn,			"PostRaceNewUnlocks" )					// 
	XT_OPTION( UI::E_UiSoundEffect_PreviouslyWhooshActionIn,			"PreviouslyWhoosh" )					// 
	XT_OPTION( UI::E_UiSoundEffect_PreviouslyCounterActionIn,			"PreviouslyCounter" )					// 
	XT_OPTION( UI::E_UiSoundEffect_SkipVistaActionIn,					"SkipVista" )							// 

	// ----------------------------------------------------------------------------------------------------------
	// NOTE - please add any new ones that aren't currently hooked up below here until the UI team have done them
	// ----------------------------------------------------------------------------------------------------------
	XT_OPTION( UI::E_UiSoundEffect_GraphicCityActionIn,					"GraphicCity" )						//X	// when entering Messages screen, the city the player is in stamps in below Fans
	XT_OPTION( UI::E_UiSoundEffect_GraphicPictureBoxActionIn,			"GraphicPictureBox" )				//X	// often paired with the text box, a picture or movie box appears on the left side
	XT_OPTION( UI::E_UiSoundEffect_UnlockContentActionIn,				"UnlockContent" )					//X	// when a new item is unlocked after a race
	XT_OPTION( UI::E_UiSoundEffect_MPPlayerFirstActionIn,				"MPPlayerFirst" )					//X	// 
	XT_OPTION( UI::E_UiSoundEffect_MPFinalScoreActionIn,				"MPFinalScore" )					//X	// 
	XT_OPTION( UI::E_UiSoundEffect_GamePausedQuitActionIn,				"GamePausedQuit" )					//X	// 
XT_END_TYPE

XT_LIBRARY( GameUI )
	XT_REGISTER_TYPE( UI::E_UiGameMsg )
	XT_REGISTER_TYPE( UI::E_UiSoundEffect )

	XT_REGISTER_TYPE( UI::UIAmaxLight )
	XT_REGISTER_TYPE( UI::UILightPaint )
	XT_REGISTER_TYPE( UI::UICarousel )
	XT_REGISTER_TYPE( UI::UILobby )
	XT_REGISTER_TYPE( UI::UIPostProcessTexture )
	XT_REGISTER_TYPE( UI::UIDamage )
	XT_REGISTER_TYPE( UI::C_UiDamageSlotParam )
	XT_REGISTER_TYPE( UI::UIFanMessage )
	XT_REGISTER_TYPE( UI::UIRequirementsMessage )
	XT_REGISTER_TYPE( UI::UILoadoutController )
	XT_REGISTER_TYPE( UI::UIMiniMap )
	XT_REGISTER_TYPE( UI::UIPerks )
	XT_REGISTER_TYPE( UI::UIPerks2 )
	XT_REGISTER_TYPE( UI::C_UiPerkSlotEffectParam )
	XT_REGISTER_TYPE( UI::C_PerkDisplayParameters )
	XT_REGISTER_TYPE( UI::C_UIPerkInfo )
	XT_REGISTER_TYPE( UI::UIHudDummy )
	XT_REGISTER_TYPE( UI::UIRankBar )
	XT_REGISTER_TYPE( UI::UIRacePreview )
	XT_REGISTER_TYPE( UI::UIAmaxGraphic )
	XT_REGISTER_TYPE( UI::UIAmaxSlider )
	XT_REGISTER_TYPE( UI::UICheckbox )
	XT_REGISTER_TYPE( UI::UIFocalPoint )
	XT_REGISTER_TYPE( UI::UITargeting )
	XT_REGISTER_TYPE( UI::UIAiHud )
	XT_REGISTER_TYPE( UI::UIShield )
	XT_REGISTER_TYPE( UI::UIRaceLine )
	XT_REGISTER_TYPE( UI::UIRaceLineSplitscreen )
	XT_REGISTER_TYPE( UI::UISpeakerSetup )
	XT_REGISTER_TYPE( UI::UIChicklets )
	XT_REGISTER_TYPE( UI::UIReinforceText )
	XT_REGISTER_TYPE( UI::UILocatorNode )
	XT_REGISTER_TYPE( UI::UICarNode )
	XT_REGISTER_TYPE( UI::UIMultiplayerData )
	XT_REGISTER_TYPE( UI::UIMultiplayerSpeaking )
	XT_REGISTER_TYPE( UI::UIColourPicker )
	XT_REGISTER_TYPE( UI::UICheckpoint )
	XT_REGISTER_TYPE( UI::UIMinorScoreLog )
	XT_REGISTER_TYPE( UI::UIToggle )
	XT_REGISTER_TYPE( UI::UITeamTugOfWar )

	XT_REGISTER_TYPE( UI::C_UiCameraInfo )
	XT_REGISTER_TYPE( UI::C_UiXtBloomParameters )
	XT_REGISTER_TYPE( UI::C_UiXtColourPickerParameters )
	XT_REGISTER_TYPE( UI::C_UiHudConfig_Pieces )
	XT_REGISTER_TYPE( UI::C_UiHudConfig_SinglePlayer )
	XT_REGISTER_TYPE( UI::C_UiHudConfig_Multiplayer )
	XT_REGISTER_TYPE( UI::C_UiHudConfig )
	XT_REGISTER_TYPE( UI::C_UiHealthTextParamState )
	XT_REGISTER_TYPE( UI::C_UiHealthTextParam )
	XT_REGISTER_TYPE( UI::C_UiMirrorSmashParamUV )
	XT_REGISTER_TYPE( UI::C_UiMirrorSmashParam )
	XT_REGISTER_TYPE( UI::C_UiGarageResprays )
	XT_REGISTER_TYPE( UI::C_UiXtConfig )

	XT_REGISTER_TYPE( UI::C_ControlConfig )
	XT_REGISTER_TYPE( Game::E_Button )
XT_END_LIBRARY

namespace Lua
{
void OutputString(const char *str)
{
	if(str == NULL)
		return;

	Debug::ConsoleLn("Lua", str);
}
};

namespace Game
{
LUALIB_API int luaopen_amax(lua_State *pState);
LUALIB_API int luaopen_gamedata(lua_State *pState);
LUALIB_API int luaopen_net_services(lua_State *pState);
LUALIB_API int luaopen_net_party(lua_State *pState);
LUALIB_API int luaopen_net_race(lua_State *pState);
LUALIB_API int luaopen_game_profile(lua_State *pState);
LUALIB_API int luaopen_lsp(lua_State *pState);
LUALIB_API int luaopen_friend_demand(lua_State *pState);
};

// ***

namespace UI 
{

extern bool g_Blur2PCSystemQuitGame;	// [Blur2PC] Lua_System.cpp (T146)

// global colour repository - TODO
//C_UiColourRepository gUIColourRepository;

C_UiInterface* g_pManager;

I_UiInterface* UiInterface()
{
	return g_pManager;
}

// Interface for Simple2D to display the mirror 
class C_MirrorAlternateTexture : public C_Simple2D::I_AlternateTexture
{
public:
	C_MirrorAlternateTexture() : m_pGameWorld( NULL ), m_Name( "MIRROR" ) { }

	/// Return handle to the texture data
	UITextureHandle				Handle( )
	{ 
		if( !m_pGameWorld )
			return NULL;
		if( !m_pGameWorld->GetMirrorTexture() )
			return NULL;

		return &m_pGameWorld->GetMirrorTexture()->GetTexture(); 
	}

	/// Name of the alternate texture
	const Util::C_String32&		Name( )	{ return m_Name; }

public:
	Game::C_GameWorld*					m_pGameWorld;
	Util::C_String32					m_Name;
};

// Interface for Simple2D to display the mirror 
class C_PhotoModeAlternateTexture : public C_Simple2D::I_AlternateTexture
{
public:
	C_PhotoModeAlternateTexture() : m_Name( "PHOTOMODE" ) { }

	/// Return handle to the texture data
	UITextureHandle				Handle( )
	{ 
		return PhotoMode::AppPhotoCapture().GetTexture();
	}

	/// Name of the alternate texture
	const Util::C_String32&		Name( )	{ return m_Name; }

public:
	Util::C_String32					m_Name;
};

#ifdef PLATFORM_PC
// [Blur2PC] T256: the source Xbox atlas shares the PC font sequence coordinates.
// Embed it in the executable so prompt switching needs no modified retail data.
class C_ControllerGlyphTexture : public C_Simple2D::I_AlternateTexture
{
public:
	C_ControllerGlyphTexture() : m_Name("CONTROLLER_GLYPHS")
	{
		HMODULE module = GetModuleHandle(NULL);
		HRSRC resource = FindResourceA(module, "BLUR2PC_CONTROLLER_GLYPHS", MAKEINTRESOURCEA(10));
		if (!resource) return;
		const U8* data = static_cast<const U8*>(LockResource(LoadResource(module, resource)));
		const U32 size = SizeofResource(module, resource);
		// The embedded asset is an uncompressed BGRA TGA, without a colour map.
		if (!data || size < 18 || data[1] != 0 || data[2] != 2 || data[16] != 32) return;
		const U32 width = data[12] | (data[13] << 8);
		const U32 height = data[14] | (data[15] << 8);
		const U32 offset = 18 + data[0];
		if (width != 512 || height != 512 || size < offset + width * height * 4) return;
		if (!Display::Device()->CreateTexture2D(width, height, 1, Display::E_Format_Linear,
			Display::E_Format_A8R8G8B8, &m_Texture)) return;
		Display::C_LockedRect rect = m_Texture.Lock(0);
		if (!rect.pBits)
		{
			Display::Device()->DestroyTexture2D(&m_Texture);
			return;
		}
		for (U32 y = 0; y < height; ++y)
		{
			const U32 sourceY = (data[17] & 0x20) ? y : height - 1 - y;
			memcpy(rect.pBits + y * rect.pitch, data + offset + sourceY * width * 4, width * 4);
		}
		m_Texture.Unlock(0);
	}
	~C_ControllerGlyphTexture()
	{
		if (m_Texture.IsCreated()) Display::Device()->DestroyTexture2D(&m_Texture);
	}
	UITextureHandle Handle() { return m_Texture.IsCreated() ? &m_Texture : NULL; }
	const Util::C_String32& Name() { return m_Name; }
	C_TextureHandleIndex m_Index;
private:
	Util::C_String32 m_Name;
	Display::C_Texture2D m_Texture;
};
#endif

class C_AlternateTexture : public C_Simple2D::I_AlternateTexture 
{
public:
	C_AlternateTexture( const char* name ) : 
		m_Name( name ), m_pTexture( NULL )
	{
	}

	/// Set the texture pointer
	void								Set( Display::C_Texture2D* texture )
	{ 
		m_pTexture = texture;
	}

	/// Return handle to the texture data
	UITextureHandle						Handle( )
	{
		return m_pTexture;
	}

	/// Name of the alternate texture
	virtual const Util::C_String32&		Name( )		
	{ 
		return m_Name;
	}

private:
	Display::C_Texture2D*		m_pTexture;
	Util::C_String32			m_Name;
};

// Interface for Simple2D to display the mirror glass
class C_MirrorGlassAlternateTexture : public C_Simple2D::I_AlternateTexture
{
public:
	C_MirrorGlassAlternateTexture() : m_Name( "MIRROR_GLASS" ) { }

	/// Return handle to the texture data
	UITextureHandle				Handle( )
	{ 
		if ( HighDynamicRangeExists() )
		{
			Display::C_Texture2D& tex = GetHighDynamicRange()->GetOverlayTexture();

			if ( tex.IsCreated() )
				return &tex;
		}

		return NULL;
	}

	/// Name of the alternate texture
	const Util::C_String32&		Name( )	{ return m_Name; }

public:
	Util::C_String32					m_Name;
};

// ---
C_UiManager::C_UiManager( const U32 sInitParam ) : 
	m_InitParam( (E_LaunchMode)sInitParam ),
	m_Blur2PCQuitGame( false ),	// [Blur2PC] T146
	m_bNormalNextButtonMapping( true ),
	m_fInviteShowTime( 3.0f ),
	m_fInviteTimer( 0.0f ),
	m_sActiveControllerDisconnected( -1 ),
	m_fDisconnectShowTime( 0.5f ),
	m_fDisconnectTimer( 0.0f ),
	m_pMirrorAlternateTexture( NULL ),
	m_pPhotoModeAlternateTexture( NULL ),
	m_pIgRenderBufferAlternateTexture( NULL ),
	m_pMirrorGlassAlternateTexture( NULL ),
#ifdef PLATFORM_PC
	m_pControllerGlyphTexture( NULL ), // [Blur2PC] T256
#endif
	m_pCurrentRouteName( NULL ),
	m_PlatformConnectionState( false ),
	m_eCarClassSP( AmaxCarClass_ANY )
#ifdef PLATFORM_PS3
	,
	m_renderBlackRectangle(false),
	m_systemMenuState(false),
	m_systemOverlayState(false)
#endif
{
	Time::C_ScopedTimer st("C_UiManager::C_UiManager()");
#ifndef AVOID_ONE_POINT_SEVEN_SECOND_UI_BLOCK
	Display::Device()->AcquireThreadOwnership();
#endif

	MEMORY_TRACKER_PUSH("ui");

	Util::C_String128 temp;
	
#ifdef PLATFORM_PS3
	// check for button swapping on PS3 - MUST be done before we create the game enums
	// -- default to X
	S32 sNormalNextButtonMapping = CELL_SYSUTIL_ENTER_BUTTON_ASSIGN_CROSS;

	if ( cellSysutilGetSystemParamInt( CELL_SYSUTIL_SYSTEMPARAM_ID_ENTER_BUTTON_ASSIGN, &sNormalNextButtonMapping ) < 0 )
		sNormalNextButtonMapping = CELL_SYSUTIL_ENTER_BUTTON_ASSIGN_CROSS;	// default on error

	m_bNormalNextButtonMapping = ( sNormalNextButtonMapping == CELL_SYSUTIL_ENTER_BUTTON_ASSIGN_CROSS );
#endif

	// Ouch!
	g_pManager = &m_UiInterface;

	RegisterGameMessageDebug();

	/*---------------------------------------------------------------------------------------
		Load and Create adobe polygon files.
	---------------------------------------------------------------------------------------*/
	// Load all the test shapes in the directory.
//	C_AdobeSceneManager::Instance().LoadScenes( "Ui\\Data\\ipf\\" );

	// Lua sys
	g_pUILua = new C_UILua();
	assert( g_pUILua );

	m_UiColourStyle = "Orange";
	C_UISpriteColours::SetStyle( m_UiColourStyle );

	// reads in UIEnums.lua too
	if( UIBIN_START == NULL )
	{
		LOGFATAL(g_Log, "LUA not embedded in EXE.");
	}
	g_pUILua->Init( true, "DontShip\\Ui\\Scripts\\", UIBIN_START );
	// NOTE : Make sure these are added in the same order as the E_Context enum
	const S32 sMainContext = C_ScreenControlManager::Instance().Count();
	C_ScreenControlManager::Instance().Add();		// E_Context_Main
	C_ScreenControlManager::Instance().Get( sMainContext ).BlockInputToMeDefault( false );
	C_ScreenControlManager::Instance().Context( sMainContext );	// Default context is 'main'

	const S32 sPopupContext = C_ScreenControlManager::Instance().Count();
	C_ScreenControlManager::Instance().Add();		// E_Context_Popup
	C_ScreenControlManager::Instance().Get( sPopupContext ).BlockInputToMeDefault( true );

	const S32 sLoadSaveContext = C_ScreenControlManager::Instance().Count();
	C_ScreenControlManager::Instance().Add();		// E_Context_LoadSave
	C_ScreenControlManager::Instance().Get( sLoadSaveContext ).BlockInputToMeDefault( false );

	const S32 sFriendsContext = C_ScreenControlManager::Instance().Count();
	C_ScreenControlManager::Instance().Add();		// E_Context_Friends
	C_ScreenControlManager::Instance().Get( sFriendsContext ).BlockInputToMeDefault( true );

	const S32 sBlurContext = C_ScreenControlManager::Instance().Count();
	C_ScreenControlManager::Instance().Add();		// E_Context_Blurb
	C_ScreenControlManager::Instance().Get( sBlurContext ).BlockInputToMeDefault( true );

	const S32 sCarouselAppContext = C_ScreenControlManager::Instance().Count();		
	C_ScreenControlManager::Instance().Add();		// E_Context_CarouselApp
	C_ScreenControlManager::Instance().Get( sCarouselAppContext ).BlockInputToMeDefault( false );

	const S32 sSubscreen0 = C_ScreenControlManager::Instance().Count();		
	C_ScreenControlManager::Instance().Add();		// E_Context_Subscreen0
	C_ScreenControlManager::Instance().Get( sSubscreen0 ).BlockInputToMeDefault( false );

	const S32 sSubscreen1 = C_ScreenControlManager::Instance().Count();		
	C_ScreenControlManager::Instance().Add();		// E_Context_Subscreen1
	C_ScreenControlManager::Instance().Get( sSubscreen1 ).BlockInputToMeDefault( false );

	const S32 sSubscreen2 = C_ScreenControlManager::Instance().Count();		
	C_ScreenControlManager::Instance().Add();		// E_Context_Subscreen2
	C_ScreenControlManager::Instance().Get( sSubscreen2 ).BlockInputToMeDefault( false );

	BIZ_ASSERT( C_ScreenControlManager::Instance().Count() == E_Context_Count );

	C_UIFriends::Instance().Initialise( sFriendsContext, E_UiMsg_CtrlButton_Back, "FriendsList.lua" );
	C_UIFriends::Instance().SetInterface( this );
	
	lua_State* pState = g_pUILua->GetState();

#ifndef _REMOVE_STRING_STORE_
	C_StringStore::Instance().Init();	
#endif
	gSimple2D.Init( pState );

	//C_AdobeSceneManager::Instance().BuildScenesAsync();
	S32 sSupportedLanguages = 
		  LOC_MASK( E_UILanguage_English )
		| LOC_MASK( E_UILanguage_French )
		| LOC_MASK( E_UILanguage_Italian )
		| LOC_MASK( E_UILanguage_German )
		| LOC_MASK( E_UILanguage_Spanish )
#ifdef PLATFORM_PC
		| LOC_MASK( E_UILanguage_Polish )
		| LOC_MASK( E_UILanguage_Russian )
#endif
//		| LOC_MASK( E_UILanguage_Japanese )
	;

	UI::C_UILocaliser::Instance().DetermineLanguageAndRegion( sSupportedLanguages, UI::E_UILanguage_English );

#ifndef CFG_GOLD
	// Overide the language
	if( g_UiDebugOverideLanguage != E_UILanguage_Other )
	{
		UI::C_UILocaliser::Instance().DetermineLanguageAndRegion( LOC_MASK( (S32)g_UiDebugOverideLanguage ), g_UiDebugOverideLanguage );
	}
	else
#endif
	{
		UI::C_UILocaliser::Instance().DetermineLanguageAndRegion( sSupportedLanguages, UI::E_UILanguage_English );
	}

	gUITextureManager.LuaState( pState );
	{
		Time::C_ScopedTimer st("bundles");

		// Common
		gUITextureManager.AddResourceBundle( "Ui\\Data\\ui_common" );

		// Load font texture bundles
#if defined (PLATFORM_PC)
		if( C_UILocaliser::Instance().Language() == E_UILanguage_Russian )
			gUITextureManager.AddResourceBundle( "Ui\\Data\\ui_font_cy" );	// Cyrillic
		else
		if( C_UILocaliser::Instance().Language() == E_UILanguage_Polish )
			gUITextureManager.AddResourceBundle( "Ui\\Data\\ui_font_ba" );	// Baltic
		else
			gUITextureManager.AddResourceBundle( "Ui\\Data\\ui_font_eur" );	// Latin
#else
		gUITextureManager.AddResourceBundle( "Ui\\Data\\ui_font_eur" );		// Latin
		gUITextureManager.AddResourceBundle( "Ui\\Data\\ui_font_ea" );		// Japanese additional
#endif

		gUITextureManager.Update();
	}

	m_SwitchBundle = C_TextureManager::m_InvalidResourceIndex;
	m_Ps3InstallBundle = C_TextureManager::m_InvalidResourceIndex;


	const char* pUILayerNames[ 7 ] = 
	{
		"BACKGROUND",
		"LAYER_1",
		"LAYER_2",
		"LAYER_3",
		"LAYER_4",
		"LAYER_5",
		"TOP",
	};
	const char* pUIPanelNames[ E_GameSpritePanel_Count ] = 
	{
		"_2D",
		"_2DAA",
		"_3DAA_WORLD",
		"_3DAA_LIGHT",
		"_2DBG",
		"_3DAA_0", 
		"_3DAA_1",
		"_3D",
		"_3D_WORLD",
		"_VP0",
		"_VP1",
		"_VP2",
		"_VP3",
		"_2D_A",
		"_2DBGAA",
		"_DUMMY1",
		"_2D_B",
		"_RT0",
		"_RT1",
		"_RT2",
		"_RT3",
#if PLATFORM_PC
		"_VP0_CAR",	// [Blur2PC]
#endif
	};

	m_FrontendShapesLoaded = false;

	gSimple2D.SetLayersAndPanels( pState, pUILayerNames, lengthof( pUILayerNames ), pUIPanelNames, lengthof( pUIPanelNames ) );
	gSimple2D.AddPanelUsage3D( E_GameSpritePanel_3DAA_LIGHT );
	gSimple2D.AddPanelUsage3D( E_GameSpritePanel_3DAA_WORLD );
	gSimple2D.AddPanelUsage3D( E_GameSpritePanel_3DAA_0 );
	gSimple2D.AddPanelUsage3D( E_GameSpritePanel_3DAA_1 );
	gSimple2D.AddPanelUsage3D( E_GameSpritePanel_3D );
	gSimple2D.AddPanelUsage3D( E_GameSpritePanel_3D_WORLD );
	gSimple2D.AddPanelUsage3D( E_GameSpritePanel_VP0 );
	gSimple2D.AddPanelUsage3D( E_GameSpritePanel_VP1 );
	gSimple2D.AddPanelUsage3D( E_GameSpritePanel_VP2 );
	gSimple2D.AddPanelUsage3D( E_GameSpritePanel_VP3 );
#if PLATFORM_PC
	gSimple2D.AddPanelUsage3D( E_GameSpritePanel_VP0_CAR );	// [Blur2PC]
#endif

#ifdef PLATFORM_PC
	// [Blur2PC] retail UI init (0x00c3c68d): the 2D screen width follows the back buffer's aspect ratio
	{
		const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
		gSimple2D.SetScreenWidth( (F32)backBuffer.GetWidth() * ( UI_VSCREEN_SIZEY / (F32)backBuffer.GetHeight() ) );
	}
#endif
	gSimple2D.SetClipSize2D( Display::C_ViewSetup::GetDeviceCoords().Max().X(), Display::C_ViewSetup::GetDeviceCoords().Max().Y() );

	// Set up the world light
	C_Renderer::C_Light& light	= gSimple2D.GetLight( E_GameSpritePanel_3DAA_WORLD );
	light.m_ColourTo			= Maths::HVec4( 0.0f, 0.0f, 0.0f, 0.1f );
	light.m_FallOff				= Maths::HVec4( 3.0f, 0.0f, 0.0f, 0.0f );

	// alternate textures
#ifdef PLATFORM_PC
	// [Blur2PC] Keep fixed split-screen join hints on the original PC atlas.
	m_pControllerGlyphTexture = new C_ControllerGlyphTexture();
	if (m_pControllerGlyphTexture->Handle())
		m_pControllerGlyphTexture->m_Index = gSimple2D.SetAlternateTexture(m_pControllerGlyphTexture);
#endif
	m_pMirrorAlternateTexture = new C_MirrorAlternateTexture();
	gSimple2D.SetAlternateTexture( m_pMirrorAlternateTexture );

	m_pPhotoModeAlternateTexture = new C_PhotoModeAlternateTexture();
	gSimple2D.SetAlternateTexture( m_pPhotoModeAlternateTexture );

	m_pRenderBufferAlternateTexture = new C_AlternateTexture( "RENDER_BUFFER" );
	gSimple2D.SetAlternateTexture( m_pRenderBufferAlternateTexture );

	for( U32 i = 0; i < m_MaxLocalPlayers; i++ )
	{
		temp.Printf( "MINIMAP_RT%d", i ); 

		m_pMiniMapRenderTargetAlternateTexture[ i ] = new C_AlternateTexture( temp.Chars() );;
		gSimple2D.SetAlternateTexture( m_pMiniMapRenderTargetAlternateTexture[ i ] );
	}
	
	for( U32 i = 0; i < m_MovieSlots; i++ )
	{
		m_pMovieAlternateTexture[ i ] = new C_MovieAlternateTexture( i );
		gSimple2D.SetAlternateTexture( m_pMovieAlternateTexture[ i ] );
	}

	m_pIgRenderBufferAlternateTexture = new C_AlternateTexture( "IG_RENDER_BUFFER" );
	gSimple2D.SetAlternateTexture( m_pIgRenderBufferAlternateTexture );

	m_pLoadingAlternateTexture[0] = new C_AlternateTexture( "LOADING_TEXTURE" );
	gSimple2D.SetAlternateTexture( m_pLoadingAlternateTexture[0] );

	m_pLoadingAlternateTexture[1] = new C_AlternateTexture( "LOADING_TEXTURE_2" ); // Grr
	gSimple2D.SetAlternateTexture( m_pLoadingAlternateTexture[1] );

	m_pMirrorGlassAlternateTexture = new C_MirrorGlassAlternateTexture();
	gSimple2D.SetAlternateTexture( m_pMirrorGlassAlternateTexture );

	// allocate enough room for gamer pictures - 20 for online lobby
	C_ProfileInterface::Instance().GamerPictureManager()->Create( 4, 19, gUITextureManager.GetTexturePtrByName( "default_gamerpic" ) );

#ifdef PLATFORM_PS3
	C_ProfileInterface::Instance().GamerPictureManager()->SetTitleID( SonyNetworkPlatform::amax_network_platform_communication_id );
#endif //PLATFORM_PS3

	gFont.Initialise( "\\Ui\\Data\\ui_font_eur.dat" );

#if defined (PLATFORM_PC)
	if( C_UILocaliser::Instance().Language() == E_UILanguage_Russian )
		gFont.Initialise( "\\Ui\\Data\\ui_font_cy.dat" );					// Cyrillic
	else
	if( C_UILocaliser::Instance().Language() == E_UILanguage_Polish )
		gFont.Initialise( "\\Ui\\Data\\ui_font_ba.dat" );					// Baltic
	else
		gFont.Initialise( "\\Ui\\Data\\ui_font_eur.dat" );					// Latin
#else
	gFont.Initialise( "\\Ui\\Data\\ui_font_eur.dat" );						// Latin
	gFont.InitialiseCommonAdditionalFont( "Ui\\Data\\ui_font_ea.dat" );		// Japanese additional
#endif


	gUIParticleMan.Init( 0 );

	/*---------------------------------------------------------------------------------------
		Register button types here
	---------------------------------------------------------------------------------------*/
	// Must be done before buttons are registered
	gButtons.InitCreator( E_UIGameButtonType_EndMarker, true );
	gButtons.Register<UI::UIAmaxLight>();
	gButtons.Register<UI::UILightPaint>();
	gButtons.Register<UI::UICarousel>();
	gButtons.Register<UI::UILobby>();
	gButtons.Register<UI::UIPostProcessTexture>();
	gButtons.Register<UI::UIDamage>();
	gButtons.Register<UI::UIFanMessage>();
	gButtons.Register<UI::UIRequirementsMessage>();
	gButtons.Register<UI::UILoadoutController>();
	gButtons.Register<UI::UIMiniMap>();
	gButtons.Register<UI::UIPerks>();
	gButtons.Register<UI::UIPerks2>();
	gButtons.Register<UI::UIHudDummy>();
	gButtons.Register<UI::UIRankBar>();
	gButtons.Register<UI::UIRacePreview>();
	gButtons.Register<UI::UIAmaxGraphic>();
	gButtons.Register<UI::UIAmaxSlider>();
	gButtons.Register<UI::UICheckbox>();
	gButtons.Register<UI::UIFocalPoint>();
	gButtons.Register<UI::UITargeting>();
	gButtons.Register<UI::UIAiHud>();
	gButtons.Register<UI::UIColourPicker>();
	gButtons.Register<UI::UIShield>();
	gButtons.Register<UI::UIRaceLine>();
	gButtons.Register<UI::UIRaceLineSplitscreen>();
	gButtons.Register<UI::UISpeakerSetup>();
	gButtons.Register<UI::UIChicklets>();
	gButtons.Register<UI::UIReinforceText>();
	gButtons.Register<UI::UILocatorNode>();
	gButtons.Register<UI::UICarNode>();
	gButtons.Register<UI::UIMultiplayerData>();
	gButtons.Register<UI::UIMultiplayerSpeaking>();
	gButtons.Register<UI::UICheckpoint>();
	gButtons.Register<UI::UIMinorScoreLog>();
	gButtons.Register<UI::UIToggle>();
	gButtons.Register<UI::UITeamTugOfWar>();

	// Create lua enums.
	gButtons.CreateLuaEnums( pState );

	CreateGameLuaEnums( pState );

	// network enums.
	RacerNet::C_NetServices::Instance().CreateLuaEnums(pState, LUA_ENUM_TABLE_NAME);

	gSimple2D.BuildLuaEnumsForAlternateTextures( pState );

	// load in sorcery definitions
	MEMORY_TRACKER_PUSH("ui.xt");
	g_UIXt.LoadDefinitions( "gamedata\\Xt\\UI.bin", C_GameApp::GetInstance()->GetGlobalXtDatabase() );
	MEMORY_TRACKER_POP("ui.xt");

	// Load any fonts defined in sorcery
	UiXt::C_FontStyleList::Instance().LoadPolygonFonts();

	// font enums
	gFont.CreateLuaEnums( pState );
	C_FontStyleCreator::Instance().Initialise();
	C_FontStyleCreator::Instance().CreateLuaEnums( pState );

	C_GadgetBase::C_Rect::ResetCulling();

	// Main context
	gUIScreenControl.SetLuaScreenDir("Screens\\");
	gUIScreenControl.SetInitFunctionName( "Init_Wrapper" );
	gUIScreenControl.SetPostInitFunctionName( "PostInit_Wrapper" );
	gUIScreenControl.SetStartLoopName( "StartLoop_Wrapper" );
	gUIScreenControl.SetFrameUpdateName( "FrameUpdate_Wrapper" );
	gUIScreenControl.SetMessageUpdateName( "MessageUpdate_Wrapper" );
	gUIScreenControl.SetEnterEndName( "EnterEnd_Wrapper" );
	gUIScreenControl.SetEndLoopName( "EndLoop_Wrapper" );
	gUIScreenControl.SetExitFunctionName( "End_Wrapper" );

	gUIScreenControl.SetDefaultScreenName( "Blank.lua" );

	// Popup context
	gUIPopupControl.SetLuaScreenDir("Popups\\");
	gUIPopupControl.SetInitFunctionName( "PopupInit" );
	gUIPopupControl.SetPostInitFunctionName( "PopupPostInit" );
	gUIPopupControl.SetStartLoopName( "PopupStartLoop" );
	gUIPopupControl.SetFrameUpdateName( "PopupFrameUpdate" );
	gUIPopupControl.SetMessageUpdateName( "PopupMessageUpdate" );
	gUIPopupControl.SetEnterEndName( "PopupEnterEnd" );
	gUIPopupControl.SetEndLoopName( "PopupEndLoop" );
	gUIPopupControl.SetExitFunctionName( "PopupEnd" );

#ifndef CFG_GOLD
	gpUiScreen = &gUIScreenControl;
	gpUiPopup = &gUIPopupControl;
#endif

	// RaceWorld : Sub screen contexts
	for( U32 i = E_Context_CarouselApp; i <= E_Context_Subscreen2; i++)
	{
		C_ScreenControl& sub = C_ScreenControlManager::Instance().Get( i );

		sub.SetLuaScreenDir("Screens\\");
		sub.SetInitFunctionName( "Init_Wrapper" );
		sub.SetPostInitFunctionName( "PostInit_Wrapper" );
		sub.SetStartLoopName( "StartLoop_Wrapper" );
		sub.SetFrameUpdateName( "FrameUpdate_Wrapper" );
		sub.SetMessageUpdateName( "MessageUpdate_Wrapper" );
		sub.SetEnterEndName( "EnterEnd_Wrapper" );
		sub.SetEndLoopName( "EndLoop_Wrapper" );
		sub.SetExitFunctionName( "End_Wrapper" );

//		sub.Init();
	}

	// friends list context
	C_ScreenControl& f = C_ScreenControlManager::Instance().Get( sFriendsContext );
	f.SetLuaScreenDir( "Screens\\" );
	f.SetInitFunctionName( "FL_Init" );
	f.SetPostInitFunctionName( "FL_PostInit" );
	f.SetStartLoopName( "FL_StartLoop" );
	f.SetFrameUpdateName( "FL_FrameUpdate" );
	f.SetMessageUpdateName( "FL_MessageUpdate" );
	f.SetEnterEndName( "FL_EnterEnd" );
	f.SetEndLoopName( "FL_EndLoop" );
	f.SetExitFunctionName( "FL_End" );

	// Load save context
	C_ScreenControl& ls = C_ScreenControlManager::Instance().Get( sLoadSaveContext );
	ls.SetLuaScreenDir( "Screens\\" );
	ls.SetInitFunctionName( "Init_Wrapper" );
	ls.SetPostInitFunctionName( "PostInit_Wrapper" );
	ls.SetStartLoopName( "StartLoop_Wrapper" );
	ls.SetFrameUpdateName( "FrameUpdate_Wrapper" );
	ls.SetMessageUpdateName( "MessageUpdate_Wrapper" );
	ls.SetEnterEndName( "EnterEnd_Wrapper" );
	ls.SetEndLoopName( "EndLoop_Wrapper" );
	ls.SetExitFunctionName( "End_Wrapper" );

	// Blurb social network context
	C_ScreenControl& blurb = C_ScreenControlManager::Instance().Get( sBlurContext );
	blurb.SetLuaScreenDir( "Screens\\" );
	blurb.SetInitFunctionName( "Init_Wrapper" );
	blurb.SetPostInitFunctionName( "PostInit_Wrapper" );
	blurb.SetStartLoopName( "StartLoop_Wrapper" );
	blurb.SetFrameUpdateName( "FrameUpdate_Wrapper" );
	blurb.SetMessageUpdateName( "MessageUpdate_Wrapper" );
	blurb.SetEnterEndName( "EnterEnd_Wrapper" );
	blurb.SetEndLoopName( "EndLoop_Wrapper" );
	blurb.SetExitFunctionName( "End_Wrapper" );

	// add in the datafinders
	C_UIDataFinder::Instance().AddGameDataFinder( &C_HudDataFinder::Instance() );
	C_UIDataFinder::Instance().AddGameDataFinder( &C_SpDataFinder::Instance() );
	C_UIDataFinder::Instance().AddGameDataFinder( &C_MpDataFinder::Instance() );
	C_UIDataFinder::Instance().AddGameDataFinder( &C_GameDataFinder::Instance() );
	C_UIDataFinder::Instance().AddGameDataFinder( &C_ProfileDataFinder::Instance() );
	C_UIDataFinder::Instance().AddGameDataFinder( &C_LobbyDataFinder::Instance() );
	C_UIDataFinder::Instance().AddGameDataFinder( &C_LanDataFinder::Instance() );
	C_UIDataFinder::Instance().AddGameDataFinder( &C_MatchingDataFinder::Instance() );

	// initialise UI camera
	C_UICamera::Instance().Init();

	// Add messagess to UI message subscriber
	for( S32 i = 0; i < C_ScreenControlManager::Instance().Count( ); i++ )
	{
		C_ScreenControl& sc = C_ScreenControlManager::Instance().Get( i );

		// Game flow messages
		for( S32 j = Game::E_GameflowMessage_StartScreenDone; j < Game::E_GameflowMessage_End; j++ )
			sc.AddLuaSubscriber( j );
		
		if( i != 0 )
		{
			sc.AddLuaSubscriber( E_UiGameMsg_NewRacebookMessage );
			sc.AddLuaSubscriber( E_UiGameMsg_Dialogue );
			sc.AddLuaSubscriber( E_UiGameMsg_RefreshLobby );
			sc.AddLuaSubscriber( E_UiGameMsg_UISceneReady );
			continue;
		}
			
		// UI Messages
		for( S32 j = UI::E_UiGameMsg_Start; j < UI::E_UiGameMsg_Max; j++ )
			sc.AddLuaSubscriber( j );
		/*
		sc.AddLuaSubscriber( E_VehicleLogicMsg_ResetWarningMsg );
		sc.AddLuaSubscriber( E_VehicleLogicMsg_FiredPerk );
		sc.AddLuaSubscriber( E_VehicleLogicMsg_HitByPerk );
		*/

		// cut scene messages
		sc.AddLuaSubscriber( ActorMsg::E_CutSceneMsg_Start_Cutscene );
		sc.AddLuaSubscriber( ActorMsg::E_CutSceneMsg_Start_ScriptedCamera );
		sc.AddLuaSubscriber( ActorMsg::E_CutSceneMsg_Start_RestartScriptedCamera );
		sc.AddLuaSubscriber( ActorMsg::E_CutSceneMsg_Start_PassedCutscene );
		sc.AddLuaSubscriber( ActorMsg::E_CutSceneMsg_Start_PassedScriptedCamera );
		sc.AddLuaSubscriber( ActorMsg::E_CutSceneMsg_Start_FailedScriptedCamera );
		sc.AddLuaSubscriber( ActorMsg::E_CutSceneMsg_Finished );
		sc.AddLuaSubscriber( ActorMsg::E_CutSceneMsg_Start_GarageCamera );
		sc.AddLuaSubscriber( ActorMsg::E_CutSceneMsg_Start_GarageCameraReverse );

		// Misc
		sc.AddLuaSubscriber( E_VehicleLogicMsg_HitByPerk );
		sc.AddLuaSubscriber( Game::E_GameLogicMessage_LapIncremented );
		sc.AddLuaSubscriber( Game::E_VehicleMessage_Overtake );
		sc.AddLuaSubscriber( Game::E_GameflowMessage_VehicleFinished );
		sc.AddLuaSubscriber( Game::E_VehicleMessage_Eliminated );
		sc.AddLuaSubscriber( Game::E_GameflowMessage_Wrecked );
		sc.AddLuaSubscriber( Game::E_GameflowMessage_Respawn );
		sc.AddLuaSubscriber( Game::E_GameflowMessage_IdlePlayerKicked );
		sc.AddLuaSubscriber( Game::E_GameflowMessage_DailyChallengesReset );
		sc.AddLuaSubscriber( Game::E_GameflowMessage_FriendsUpdated );	

		sc.AddLuaSubscriber( E_UiGameMsg_RemoveECMWarning );
		sc.AddLuaSubscriber( E_UiGameMsg_DisplayECMWarning );
		sc.AddLuaSubscriber( E_ProfMsg_ServiceConnectionLost );
		sc.AddLuaSubscriber( E_ProfMsg_ServiceConnectionEstablished );
		sc.AddLuaSubscriber( E_UiGameMsg_NewRivalSelected );
		sc.AddLuaSubscriber( ActorMsg::E_DynamicObjMsg_FinishedAnimatedCamera );
	}

	C_ProfileInterface::Instance().Initialise();

	// ------------------
	// game flow messages
	// ------------------
	SetSubscriberSystem( &Game::C_Game::GetInstance()->GetSubscriberSystem() );
		AddSubscriber( Game::E_GameflowMessage_StartRaceLoad );
		AddSubscriber( ActorMsg::E_DynamicObjMsg_StartAnimatedCamera );
		AddSubscriber( ActorMsg::E_DynamicObjMsg_BlendAnimatedCamera );
		AddSubscriber( ActorMsg::E_DynamicObjMsg_AutoSelectAnimatedCamera );

	// -----------
	// UI messages
	// -----------
	SetSubscriberSystem( &gButtons.GetSubscriberSystem() );
		AddSubscriber( E_VehicleLogicMsg_FiredPerk );

		// ----------------
		// profile messages
		// ----------------
		AddSubscriber( E_ProfMsg_SignedIn );
		AddSubscriber( E_ProfMsg_SignedOut );

		AddSubscriber( E_ProfMsg_GameInvitePending );
		AddSubscriber( E_ProfMsg_MuteInfoChanged );
		AddSubscriber( E_ProfMsg_ServiceConnectionLost );
		AddSubscriber( E_ProfMsg_ServiceConnectionEstablished );
		AddSubscriber( E_ProfMsg_ServiceConnectionLostXLSP );
		AddSubscriber( E_ProfMsg_LobbyConnectionLost );
		AddSubscriber( E_ProfMsg_NetworkCableRemoved );

		AddSubscriber( E_UiGameMsg_InviteReceived );
		AddSubscriber( E_UiGameMsg_LobbyConnectionLost );

		// cut scene messages
		AddSubscriber( ActorMsg::E_CutSceneMsg_Start_Cutscene );
		AddSubscriber( ActorMsg::E_CutSceneMsg_Start_ScriptedCamera );
		AddSubscriber( ActorMsg::E_CutSceneMsg_Start_RestartScriptedCamera );
		AddSubscriber( ActorMsg::E_CutSceneMsg_Start_PassedCutscene );
		AddSubscriber( ActorMsg::E_CutSceneMsg_Start_PassedScriptedCamera );
		AddSubscriber( ActorMsg::E_CutSceneMsg_Start_FailedScriptedCamera );
		AddSubscriber( ActorMsg::E_CutSceneMsg_Start_GarageCamera );
		AddSubscriber( ActorMsg::E_CutSceneMsg_Start_GarageCameraReverse );
	//gUIScreenControl.AddMessage("LOADED");

	// Hook the advertising system into the UI message system
	m_pAdvertisingMessageController = new Advertising::MessageController(&gButtons.GetSubscriberSystem());

	// initialise audio eq
	for ( S32 i = 0; i < m_AudioEq.Size(); i++ )
		m_AudioEq[ i ] = 0.0f;

	for( S32 i = 0; i < m_MaskXTime.Size(); i++ )
		m_MaskXTime[ i ] = 0;

	for( S32 i = 0; i < m_MaskYTime.Size(); i++ )
		m_MaskYTime[ i ] = 0;

	gDashboardInterface.Init();

#ifdef PLATFORM_PS3
	C_PS3InstallData::Instance().SetUiInitialised();
#endif

	// Render matrix for RT0
	const F32 transformUI[ 4 ] =
	{
		1.0f / 512.0f,	// scale x
		1.0f / 512.0f,	// scale y
		0.5f,			// translate x
		0.5f,			// translate y
	};
	gSimple2D.CalcTransform2D( transformUI, 1.0f, 1.0f, 0.0f, 15000.0f, m_TransformRT0 );

	MEMORY_TRACKER_POP("ui");

#ifndef AVOID_ONE_POINT_SEVEN_SECOND_UI_BLOCK
	Display::Device()->ReleaseThreadOwnership();
#endif
}

void C_UiManager::LoadingTextureClone( Display::C_Texture* src, const U32 index )
{
	Display::C_Texture* pTex = (Display::C_Texture*)m_pLoadingAlternateTexture[ index ]->Handle();
	if( pTex )
	{
		// Destroy old
		Display::Device()->Destroy( pTex );
		m_pLoadingAlternateTexture[ index ]->Set( NULL );
	}

	if( !Display2::CloneTexture2D( src, &m_LoadingTexture[ index ] ) )
		return;

	m_pLoadingAlternateTexture[ index ]->Set( &m_LoadingTexture[ index ] );
}

void C_UiManager::PostXtInit()
{
	// Run setup when all enums have been added
#ifndef CFG_GOLD
	g_pUILua->LoadCallSetupScript( (U32)m_InitParam, g_debugUseLUAFromHDD );
#else
	g_pUILua->LoadCallSetupScript( (U32)m_InitParam, false );
#endif
	lua_State* pState = g_pUILua->GetState();

	// _after_ loading Lua scripts
	CreateGameGlobals( pState );

	// check if we've now got a font sequences LUA table loaded
	gFont.InitialiseSequences( pState );

	// init font styles - _after_ we load Lua common files in
	C_FontStyle::Instance().InitDefaultStyle( pState );

	// Load the platform specific controller shape
#if defined(PLATFORM_PS3)
	C_AdobeSceneManager::Instance().LoadScene( "Ui\\Data\\ipf\\contoller_ps3.ipf" );
#else
	C_AdobeSceneManager::Instance().LoadScene( "Ui\\Data\\ipf\\controller_360.ipf" );
#endif
	// Load the common shapes
	C_UiXtConfig* pConfig = C_UiXtConfig::Instance();
	if( pConfig )
	{
		// Commmon shapes
		Util::C_String128 filename;
		for( XtI32 i = 0; i < pConfig->m_CommonShapes.count; i++ )
		{
			filename.Printf( "Ui\\Data\\ipf\\%s.ipf", pConfig->m_CommonShapes[ i ].Text() );
			C_AdobeSceneManager::Instance().LoadScene( filename.Chars() );
		}
		// Load the frontend shapes
		LoadFrontendShapes();
	}

	// Set default camera
	UICredits::C_Text::Instance().Load( "Gamedata\\Ui\\credits.txt" );
	UILegal::C_Text::Instance().Load( "Gamedata\\Ui\\legal.txt" );
//	gUIScreenControl.Init();
}

void C_UiManager::LoadFrontendShapes()
{
	if( m_FrontendShapesLoaded )
		return;

	C_UiXtConfig* pConfig = C_UiXtConfig::Instance();
	if( !pConfig )
		return;

	Util::C_String128 filename;
	for( XtI32 i = 0; i < pConfig->m_FrontendShapes.count; i++ )
	{
		if ( strcmp( pConfig->m_FrontendShapes[ i ].Text(), GetVehicleSheet( m_eCarClassSP ) ) == 0 )
			continue;	// we've already got this car class sheet loaded

		filename.Printf( "Ui\\Data\\ipf\\%s.ipf", pConfig->m_FrontendShapes[ i ].Text() );
		C_AdobeSceneManager::Instance().LoadScene( filename.Chars() );
	}

	// Load routes
	for( XtI32 i = 0; i < pConfig->m_MapShapes.count; i++ )
	{
		// Don't throw away the route we're about to go and race
		if( m_pCurrentRouteName && stricmp( m_pCurrentRouteName, pConfig->m_MapShapes[ i ].Text() ) == 0 )
			continue;

		filename.Printf( "Ui\\Data\\ipf\\MiniMap\\%s_ui.ipf", pConfig->m_MapShapes[ i ].Text() );
		C_AdobeSceneManager::Instance().LoadScene( filename.Chars() );

		filename.Printf( "Ui\\Data\\ipf\\MiniMap\\%s_game.ipf", pConfig->m_MapShapes[ i ].Text() );
		C_AdobeSceneManager::Instance().LoadScene( filename.Chars() );
	}
	m_pCurrentRouteName = NULL;

	m_FrontendShapesLoaded = true;
	m_eCarClassSP = AmaxCarClass_ANY;	// reset this for next race
}

void C_UiManager::FreeFrontendShapes()
{
	if( !m_FrontendShapesLoaded )
		return;

	C_UiXtConfig* pConfig = C_UiXtConfig::Instance();
	if( !pConfig )
		return;

	// in single player - don't nuke the car icons sheet for the class that we're currently in
	Game::C_RaceBookLayer* pRaceBookLayer = GetRaceBookLayer();

	if ( pRaceBookLayer && pRaceBookLayer->GetGameModePtr() && ( pRaceBookLayer->GetGameModePtr()->GetID() == Game::E_GameMode_SinglePlayer ) )
	{
		// get the last car we used (set from garage)
		XtId vehicleId = SP_GameGetLastVehicle( SP_GetCurrentSaveGame() );
		AmaxVehicle* pAmaxVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)vehicleId ];

		if ( pAmaxVehicle )
			m_eCarClassSP = pAmaxVehicle->carClass;
	}

	for( XtI32 i = 0; i < pConfig->m_FrontendShapes.count; i++ )
		if ( strcmp( pConfig->m_FrontendShapes[ i ].Text(), GetVehicleSheet( m_eCarClassSP ) ) != 0 )	// remove all but the car class we want to keep
			C_AdobeSceneManager::Instance().RemoveScene( pConfig->m_FrontendShapes[ i ].Text() );

	// Free routes
	AmaxRoute *pRoute  = C_GameData::Instance().mRoutes[(GAMEDATA_ID)UI::GetLevelState().mRoute];
	if( pRoute && pRoute->city )
		m_pCurrentRouteName = pRoute->city->tag.Text();

	Util::C_String128 filename;
	for( XtI32 i = 0; i < pConfig->m_MapShapes.count; i++ )
	{
		if( m_pCurrentRouteName && stricmp( m_pCurrentRouteName, pConfig->m_MapShapes[ i ].Text() ) == 0 )
			continue;

		filename.Printf( "%s_ui", pConfig->m_MapShapes[ i ].Text() );
		C_AdobeSceneManager::Instance().RemoveScene( filename.Chars() );

		filename.Printf( "%s_game", pConfig->m_MapShapes[ i ].Text() );
		C_AdobeSceneManager::Instance().RemoveScene( filename.Chars() );
	}

	m_FrontendShapesLoaded = false;
}

void C_UiManager::FreeCommonShapes()
{
	C_UiXtConfig* pConfig = C_UiXtConfig::Instance();
	if( !pConfig )
		return;

	for( XtI32 i = 0; i < pConfig->m_CommonShapes.count; i++ )
		C_AdobeSceneManager::Instance().RemoveScene( pConfig->m_CommonShapes[i].Text() );

	for( XtI32 i = 0; i < pConfig->m_MapShapes.count; i++ )
		C_AdobeSceneManager::Instance().RemoveScene( pConfig->m_MapShapes[i].Text() );
}

void C_UiManager::SetGameWorld( Game::C_GameWorld* gw )
{
	m_pMirrorAlternateTexture->m_pGameWorld = gw;

	// Incase going back to the UI
	if ( gw )
	{
		C_GameDataFinder::Instance().InitMessaging();
	}
}

void C_UiManager::SetRenderBuffer( Display::C_Texture2D* texture )
{
	m_pRenderBufferAlternateTexture->Set( texture );
}

void C_UiManager::SetMiniMapRenderTarget( Display::C_Texture2D* texture, const U32 localPlayerIndex )
{
	m_pMiniMapRenderTargetAlternateTexture[ localPlayerIndex ]->Set( texture );
}

void C_UiManager::SetIgRenderTarget( Display::C_Texture2D* texture )
{
	m_pIgRenderBufferAlternateTexture->Set( texture );
}

void C_UiManager::LoadFrontendTextureBundle( const bool initialLoad )
{
	// can only create if we've already deleted the switch
	if( m_SwitchBundle != C_TextureManager::m_InvalidResourceIndex )
		return;

	m_SwitchBundle = gUITextureManager.AddResourceBundle( "UI\\Data\\ui_frontend" );

#if defined (PLATFORM_PS3)
	if( initialLoad )
		m_Ps3InstallBundle = gUITextureManager.AddResourceBundle( "UI\\Data\\ui_ps3_install" );
#endif
}

void C_UiManager::LoadGameTextureBundle()
{
	// can only create if we've already deleted the switch
	if( m_SwitchBundle != C_TextureManager::m_InvalidResourceIndex )
		return;

	m_SwitchBundle = gUITextureManager.AddResourceBundle( "UI\\Data\\ui_game" );
}

void C_UiManager::DeleteTextureBundle( const bool bDeleteAll )
{
	if( bDeleteAll )
	{
		gUITextureManager.DestroyAll();
	}
	else
	{
		gUITextureManager.RemoveResourceBundle( m_SwitchBundle );
		m_SwitchBundle = C_TextureManager::m_InvalidResourceIndex;

#if defined (PLATFORM_PS3)
		if( m_Ps3InstallBundle != C_TextureManager::m_InvalidResourceIndex )
		{
			gUITextureManager.RemoveResourceBundle( m_Ps3InstallBundle );
			m_Ps3InstallBundle = C_TextureManager::m_InvalidResourceIndex;
		}
#endif
	}
}

void C_UiManager::LoadingTextureDestroy( const U32 index )
{
	Display::C_Texture* pTex = (Display::C_Texture*)m_pLoadingAlternateTexture[ index ]->Handle();
	if( !pTex )
		return;

	Display::Device()->Destroy( pTex );
}

void C_UiManager::CreateGameGlobals( lua_State* pState )
{
	lua_getglobal( pState, LUA_GLOBAL_TABLE_NAME );

	if ( lua_isnil( pState, -1 ) )
	{
		LOGFATAL( g_Log, DBSTR "%s - table not found", LUA_GLOBAL_TABLE_NAME );
		lua_pop( pState, 1 );
		return;
	}

	// UIGlobals go here ...
	Lua::AddDataMember( pState, "CurrentRegion",	UI::C_UILocaliser::Instance().Region() );
	Lua::AddDataMember( pState, "CurrentLanguage",	UI::C_UILocaliser::Instance().Language() );

#ifndef CFG_GOLD
	//Log language and region
	AmaxDataLogging::LogRegionLanguageUsed(UI::C_UILocaliser::Instance().Region(), UI::C_UILocaliser::Instance().Language() );
#endif


	Lua::AddDataMember( pState, "ResetTimerAfterWrecked", 	UI::GetRLC()->GetRLSystem()->mDamageSystem->mParameters->mResetTimerAfterWrecked );

	lua_pop( pState, 1 );
}

void C_UiManager::CreateGameLuaEnums( lua_State* pState )
{
	lua_getglobal( pState, LUA_ENUM_TABLE_NAME );

	if ( lua_isnil( pState, -1 ) )
	{
		LOGFATAL( g_Log, DBSTR "%s - table not found", LUA_ENUM_TABLE_NAME );
		lua_pop( pState, 1 );
		return;
	}

	// normal next button mapping
	Lua::AddDataMember( pState, "NormalNextButtonMapping", m_bNormalNextButtonMapping );

#ifdef PLATFORM_PC
	// [Blur2PC] Retail UIEnums.Device.Keyboard (Blur.exe 1.2, 0x00c325a0).
	lua_pushstring( pState, "Device" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Keyboard", 0 );
	lua_rawset( pState, -3 );
#endif

	// Launch codes 3...2...1... blast off Amax Yeah!
	lua_pushstring(pState, "LaunchMode");
	lua_newtable(pState);
		Lua::AddDataMember(pState, "Master",	E_LaunchMode_Master );
		Lua::AddDataMember(pState, "Final",		E_LaunchMode_Final );
		Lua::AddDataMember(pState, "Debug",		E_LaunchMode_Debug );
		Lua::AddDataMember(pState, "DebugRb",	E_LaunchMode_DebugRaceBook );
		Lua::AddDataMember(pState, "Demo",		E_LaunchMode_Demo );
		Lua::AddDataMember(pState, "Automation",E_LaunchMode_Automation );
	lua_rawset(pState,-3);

	// Raceworld carousel branches
	lua_pushstring(pState, "RwBranch");
	lua_newtable(pState);
		Lua::AddDataMember(pState, "Races",			E_RwCarouselBranch_Races );
		Lua::AddDataMember(pState, "Garage",		E_RwCarouselBranch_Garage );
		Lua::AddDataMember(pState, "Quit",			E_RwCarouselBranch_Quit );
		Lua::AddDataMember(pState, "Help",			E_RwCarouselBranch_Help );
		Lua::AddDataMember(pState, "Photos",		E_RwCarouselBranch_Photos );
		Lua::AddDataMember(pState, "Blurb",			E_RwCarouselBranch_Blurb );
		Lua::AddDataMember(pState, "Stickers",		E_RwCarouselBranch_Stickers );
		Lua::AddDataMember(pState, "Leaderboards",	E_RwCarouselBranch_Leaderboards );
		Lua::AddDataMember(pState, "Options",		E_RwCarouselBranch_Options );
	lua_rawset(pState,-3);

	lua_pushstring(pState, "RwMpBranch");
	lua_newtable(pState);
		Lua::AddDataMember(pState, "Online",		E_RwMpCarouselBranch_Online );
		Lua::AddDataMember(pState, "Splitscreen",	E_RwMpCarouselBranch_Splitscreen );
		Lua::AddDataMember(pState, "Options",		E_RwMpCarouselBranch_Options );
		Lua::AddDataMember(pState, "Lan",			E_RwMpCarouselBranch_Lan );
		Lua::AddDataMember(pState, "RaceHistory",	E_RwMpCarouselBranch_RaceHistory );
		Lua::AddDataMember(pState, "Help",			E_RwMpCarouselBranch_Help );
		Lua::AddDataMember(pState, "Stickers",		E_RwMpCarouselBranch_Stickers );
		Lua::AddDataMember(pState, "Quit",			E_RwMpCarouselBranch_Quit );
	lua_rawset(pState,-3);

	// Raceworld ingame carousel branches
	lua_pushstring(pState, "RwGameBranch");
	lua_newtable(pState);
		Lua::AddDataMember(pState, "Paused",		E_RwGameCarouselBranch_Paused );
		Lua::AddDataMember(pState, "Retry",			E_RwGameCarouselBranch_Retry );
		Lua::AddDataMember(pState, "Options",		E_RwGameCarouselBranch_Options );
		Lua::AddDataMember(pState, "Help",			E_RwCarouselBranch_Help );
		Lua::AddDataMember(pState, "Stats",			E_RwGameCarouselBranch_Stats );
		Lua::AddDataMember(pState, "Goals",			E_RwGameCarouselBranch_Goals );
		Lua::AddDataMember(pState, "Photo",			E_RwGameCarouselBranch_PhotoMode );
		Lua::AddDataMember(pState, "Quit",			E_RwGameCarouselBranch_Quit );
		Lua::AddDataMember(pState, "ReturnToLobby",	E_RwGameCarouselBranch_ReturnToLobby );
	lua_rawset(pState,-3);

	lua_pushstring(pState, "Context");
	lua_newtable(pState);
		Lua::AddDataMember(pState, "Main",				E_Context_Main );
		Lua::AddDataMember(pState, "Popup",				E_Context_Popup );
		Lua::AddDataMember(pState, "LoadSave",			E_Context_LoadSave );
		Lua::AddDataMember(pState, "Friends",			E_Context_Friends );
		Lua::AddDataMember(pState, "Blurb",				E_Context_Blurb );
		Lua::AddDataMember(pState, "CarouselApp",		E_Context_CarouselApp );
		Lua::AddDataMember(pState, "Subscreen0",		E_Context_Subscreen0 );
		Lua::AddDataMember(pState, "Subscreen1",		E_Context_Subscreen1 );
		Lua::AddDataMember(pState, "Subscreen2",		E_Context_Subscreen2 );

		Lua::AddDataMember(pState, "SpWrecked",			E_Context_Subscreen0 ); // Shared with subscreen 0
	lua_rawset(pState,-3);

	UiXt::CreateLuaTableFromEnumType( pState, "UIGameMessage", "GameMessage" );

	UiXt::CreateLuaTableFromEnumType( pState, "AnimatedCameraSequenceType" );

	lua_pushstring( pState, "CutScene" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Start",					ActorMsg::E_CutSceneMsg_Start_Cutscene );
		Lua::AddDataMember( pState, "StartScriptedCamera",		ActorMsg::E_CutSceneMsg_Start_ScriptedCamera );
		Lua::AddDataMember( pState, "RestartScriptedCamera",	ActorMsg::E_CutSceneMsg_Start_RestartScriptedCamera );
		Lua::AddDataMember( pState, "Passed",					ActorMsg::E_CutSceneMsg_Start_PassedCutscene );
		Lua::AddDataMember( pState, "PassedScriptedCamera", 	ActorMsg::E_CutSceneMsg_Start_PassedScriptedCamera );
		Lua::AddDataMember( pState, "FailedScriptedCamera", 	ActorMsg::E_CutSceneMsg_Start_FailedScriptedCamera );
		Lua::AddDataMember( pState, "Finished",					ActorMsg::E_CutSceneMsg_Finished );
		Lua::AddDataMember( pState, "StartGarageCamera",		ActorMsg::E_CutSceneMsg_Start_GarageCamera );
		Lua::AddDataMember( pState, "StartGarageCameraReverse",	ActorMsg::E_CutSceneMsg_Start_GarageCameraReverse );
	lua_rawset( pState, -3 );

	lua_pushstring(pState, "MiscMessage");
	lua_newtable(pState);
		Lua::AddDataMember(pState, "LapIncremented", Game::E_GameLogicMessage_LapIncremented );
		Lua::AddDataMember(pState, "Overtake", Game::E_VehicleMessage_Overtake );
		Lua::AddDataMember(pState, "VehicleEliminated", Game::E_VehicleMessage_Eliminated );
		Lua::AddDataMember(pState, "IdlePlayerKicked", Game::E_GameflowMessage_IdlePlayerKicked );
		Lua::AddDataMember(pState, "DailyChallengesReset", Game::E_GameflowMessage_DailyChallengesReset );
		Lua::AddDataMember(pState, "FriendsUpdated", Game::E_GameflowMessage_FriendsUpdated );
		Lua::AddDataMember(pState, "FinishedAnimatedCamera", ActorMsg::E_DynamicObjMsg_FinishedAnimatedCamera );
		Lua::AddDataMember(pState, "ServiceConnectionLost", E_ProfMsg_ServiceConnectionLost );
		Lua::AddDataMember(pState, "ServiceConnectionEstablished",E_ProfMsg_ServiceConnectionEstablished );
	lua_rawset(pState,-3);

	UiXt::CreateLuaTableFromEnumType( pState, "UISoundEffect", "SoundEffect" );
	UiXt::CreateLuaTableFromEnumType( pState, "AmaxCarClass", "VehicleClass" );
	UiXt::CreateLuaTableFromEnumType( pState, "AmaxCarUiStyle", "VehicleUiStyle" );
	UiXt::CreateLuaTableFromEnumType( pState, "AmaxVehicleStyleType", "VehicleStyle" );

	lua_pushstring( pState, "ScoringType" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "None",				AMAX_SCORING_EVENT_NONE );
		Lua::AddDataMember( pState, "Shunt",			AMAX_SCORING_EVENT_SHUNT );
		Lua::AddDataMember( pState, "Shock",			AMAX_SCORING_EVENT_SHOCK );
		Lua::AddDataMember( pState, "Barge",			AMAX_SCORING_EVENT_BARGE );
		Lua::AddDataMember( pState, "NitroOvertake",	AMAX_SCORING_EVENT_NITROUS_OVERTAKE );
		Lua::AddDataMember( pState, "Mine",				AMAX_SCORING_EVENT_BOMB );
	lua_rawset( pState, -3 );

	// Vehicle logic messages that are sent to lua
	lua_pushstring( pState, "VLMessage" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "HitByPerk", E_VehicleLogicMsg_HitByPerk );
	lua_rawset( pState, -3 );

	UiXt::CreateLuaTableFromEnumType( pState, "IconOverideType" );
	UiXt::CreateLuaTableFromEnumType( pState, "UiCircuitType" );

	lua_pushstring( pState, "SPScoreboardTypes" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "SPRacesScoreboard", RacerNet::eSPRacesScoreboard);
		Lua::AddDataMember( pState, "SPFanDemandsScoreboard", RacerNet::eSPFanDemandsScoreboard );
		Lua::AddDataMember( pState, "SPCheckpointsScoreboard", RacerNet::eSPCheckpointsScoreboard );
		Lua::AddDataMember( pState, "SPDestructionScoreboard", RacerNet::eSPDestructionScoreboard );
		Lua::AddDataMember( pState, "SPOneOnOneScoreboard", RacerNet::eSP1on1Scoreboard );
	lua_rawset( pState, -3 );

	// Add multiplayer enums
	CreateMpLuaEnums( pState );

	lua_pushstring( pState, "Palette" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Solid", E_Palette_Solid );
		Lua::AddDataMember( pState, "Metallic", E_Palette_Metallic );
		Lua::AddDataMember( pState, "Pearlecent", E_Palette_Pearlecent );
		Lua::AddDataMember( pState, "Candy", E_Palette_Candy );
	lua_rawset( pState, -3 );

	// friend demands
	lua_pushstring( pState, "FriendDemandType" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "ShuntsHit",				Game::FRIEND_CHALLENGE_PARAM_TYPE_SHUNTS_HIT );
		Lua::AddDataMember( pState, "BargesHit",				Game::FRIEND_CHALLENGE_PARAM_TYPE_BARGES_HIT );
		Lua::AddDataMember( pState, "MinesHit",					Game::FRIEND_CHALLENGE_PARAM_TYPE_MINES_HIT );
		Lua::AddDataMember( pState, "BlindfiresHit",			Game::FRIEND_CHALLENGE_PARAM_TYPE_BLIND_FIRES_HIT );
		Lua::AddDataMember( pState, "NitrousUsed",				Game::FRIEND_CHALLENGE_PARAM_TYPE_NITROUS_USED	 );
		Lua::AddDataMember( pState, "FanRunScore",				Game::FRIEND_CHALLENGE_PARAM_TYPE_FAN_RUN_SCORE );
		Lua::AddDataMember( pState, "RaceTime",					Game::FRIEND_CHALLENGE_PARAM_TYPE_RACE_TIME	 );
		Lua::AddDataMember( pState, "FansGained",				Game::FRIEND_CHALLENGE_PARAM_TYPE_FANS_GAINED );
		Lua::AddDataMember( pState, "BoltAccuracy",				Game::FRIEND_CHALLENGE_PARAM_TYPE_BOLT_ACCURACY );
		Lua::AddDataMember( pState, "TotalWrecked",				Game::FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_WRECKED );
		Lua::AddDataMember( pState, "BlueWrecked",				Game::FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_BLUE_WRECKED );
		Lua::AddDataMember( pState, "RedWrecked",				Game::FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_RED_WRECKED );
		Lua::AddDataMember( pState, "YellowWrecked",			Game::FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_YELLOW_WRECKED );
		Lua::AddDataMember( pState, "GreenWrecked",				Game::FRIEND_CHALLENGE_PARAM_TYPE_TOTAL_GREEN_WRECKED );
		Lua::AddDataMember( pState, "StopwatchesUSed",			Game::FRIEND_CHALLENGE_PARAM_TYPE_STOPWATCHES_USED );
		Lua::AddDataMember( pState, "PickupAccuracy",			Game::FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_ACCURACY );
		Lua::AddDataMember( pState, "PickupsHit",				Game::FRIEND_CHALLENGE_PARAM_TYPE_PICKUP_HITS );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "FriendDemandModifer" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "NotUsed",					Game::FRIEND_CHALLENGE_MODIFIER_NOT_USED );
		Lua::AddDataMember( pState, "LessThan",					Game::FRIEND_CHALLENGE_MODIFIER_LESS_THAN );
		Lua::AddDataMember( pState, "GreaterThan",				Game::FRIEND_CHALLENGE_MODIFIER_GREATER_THAN );
		Lua::AddDataMember( pState, "Equals",					Game::FRIEND_CHALLENGE_MODIFIER_EQUALS );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "FriendDemandParamUse" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "None",						Game::FRIEND_CHALLENGE_PARAM_USE_NONE );
		Lua::AddDataMember( pState, "LessThanMoreThan",			Game::FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN );
		Lua::AddDataMember( pState, "LessThanMoreThanEquals",	Game::FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_MORE_THAN_EQUALS );
		Lua::AddDataMember( pState, "EqualsMoreThan",			Game::FRIEND_CHALLENGE_PARAM_USE_EQUALS_MORE_THAN );
		Lua::AddDataMember( pState, "LessThanEquals",			Game::FRIEND_CHALLENGE_PARAM_USE_LESS_THAN_EQUALS );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "FriendDemandSort" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "New",		Game::FRIEND_CHALLENGE_SORT_TYPE_NEW );
		Lua::AddDataMember( pState, "Active",	Game::FRIEND_CHALLENGE_SORT_TYPE_ACTIVE );
		Lua::AddDataMember( pState, "Alpha",	Game::FRIEND_CHALLENGE_SORT_TYPE_ALPHA );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "FriendDemandMessageType" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Local",	Game::AMAX_SOCIAL_NETWORK_BLURB_TYPE_NORMAL );
		Lua::AddDataMember( pState, "Server",	Game::AMAX_SOCIAL_NETWORK_BLURB_TYPE_SERVER_MESSAGE );
	lua_rawset( pState, -3 );

	//Only the ones being used for the sake of not having a long list of unused enums.
	lua_pushstring( pState, "StickerType" );
	lua_newtable( pState);
		Lua::AddDataMember( pState, "CustomiseALoadout",			Game::BLUR_STICKER_CUSTOMISE_A_LOADOUT );
		Lua::AddDataMember( pState, "UnlockShannon",				Game::BLUR_STICKER_UNLOCK_SHANNON );
		Lua::AddDataMember( pState, "BeatShannon",					Game::BLUR_STICKER_BEAT_SHANNON );
		Lua::AddDataMember( pState, "BeatKhan",						Game::BLUR_STICKER_BEAT_KHAN );
		Lua::AddDataMember( pState, "BeatRhymer",					Game::BLUR_STICKER_BEAT_RHYMER );
		Lua::AddDataMember( pState, "BeatDrake",					Game::BLUR_STICKER_BEAT_DRAKE );
		Lua::AddDataMember( pState, "BeatNatalya",					Game::BLUR_STICKER_BEAT_NATALYA );
		Lua::AddDataMember( pState, "BeatTalon",					Game::BLUR_STICKER_BEAT_TALON );
		Lua::AddDataMember( pState, "BeatTwins",					Game::BLUR_STICKER_BEAT_AYUMI_HARUMI );
		Lua::AddDataMember( pState, "BeatCarver",					Game::BLUR_STICKER_BEAT_CARVER );
		Lua::AddDataMember( pState, "BeatShannonAgain",				Game::BLUR_STICKER_BEAT_SHANNON_AGAIN );
		Lua::AddDataMember( pState,	"AcceptFriendDemand",			Game::BLUR_STICKER_ACCEPT_FRIEND_DEMAND );
		Lua::AddDataMember( pState, "FirstFriendDemand",			Game::BLUR_STICKER_FIRST_FRIEND_DEMAND );
		Lua::AddDataMember( pState, "BeatFriendDemand",				Game::BLUR_STICKER_BEAT_FRIEND_DEMAND );
		Lua::AddDataMember( pState, "SendChallengeBack",			Game::BLUR_STICKER_SEND_CHALLENGE_BACK );
		Lua::AddDataMember( pState, "ShareFirstExperience",			Game::BLUR_STICKER_SHARE_FIRST_EXPERIENCE );
		Lua::AddDataMember( pState, "LeaderboardChallengeFriend",	Game::BLUR_STICKER_MAXI_LEADERBOARD_FRIEND );
		Lua::AddDataMember( pState, "ResprayFirstCar",				Game::BLUR_STICKER_RESPRAY_A_CAR );
		Lua::AddDataMember( pState, "ResprayWithUnlocked",			Game::BLUR_STICKER_RESPRAY_WITH_UNLOCKED );
	lua_rawset( pState, -3 );

	//Sp video config stuff
	lua_pushstring( pState, "SPVideoConfig" );
	lua_newtable( pState);
		Lua::AddDataMember( pState, "VIDEO_GAME_MODE_RACE", Game::VIDEO_GAME_MODE_RACE);
		Lua::AddDataMember( pState, "VIDEO_GAME_MODE_DESTRUCTION", Game::VIDEO_GAME_MODE_DESTRUCTION );
		Lua::AddDataMember( pState, "VIDEO_GAME_MODE_CHECKPOINT", Game::VIDEO_GAME_MODE_CHECKPOINT );
		Lua::AddDataMember( pState, "VIDEO_GAME_MODE_FAN", Game::VIDEO_GAME_MODE_FAN );
		Lua::AddDataMember( pState, "VIDEO_GAME_MODE_ONEONONE", Game::VIDEO_GAME_MODE_ONEONONE );
		Lua::AddDataMember( pState, "VIDEO_TUTORIAL_1", Game::VIDEO_TUTORIAL_1 );
		Lua::AddDataMember( pState, "VIDEO_TUTORIAL_2", Game::VIDEO_TUTORIAL_2 );
		Lua::AddDataMember( pState, "VIDEO_FAN_PAR", Game::VIDEO_FAN_PAR );
		Lua::AddDataMember( pState, "VIDEO_FAN_RUN", Game::VIDEO_FAN_RUN );
		Lua::AddDataMember( pState, "VIDEO_FAN_DEMAND", Game::VIDEO_FAN_DEMAND );
		Lua::AddDataMember( pState, "VIDEO_TIER_UNLOCKS", Game::VIDEO_TIER_UNLOCKS );
		Lua::AddDataMember( pState, "VIDEO_BOSS_UNLOCKS", Game::VIDEO_BOSS_UNLOCKS );
		Lua::AddDataMember( pState, "VIDEO_BOSS_BEATEN", Game::VIDEO_BOSS_BEATEN );
		Lua::AddDataMember( pState, "VIDEO_END_OF_GAME", Game::VIDEO_END_OF_GAME );
		Lua::AddDataMember( pState, "VIDEO_END_PHONECALL", Game::VIDEO_END_PHONECALL );
		Lua::AddDataMember( pState, "VIDEO_GLOSSY_INTRO", Game::VIDEO_GLOSSY_INTRO );
		Lua::AddDataMember( pState, "VIDEO_POWER_UPS", Game::VIDEO_POWER_UPS );
	lua_rawset( pState, -3 );

	UiXt::CreateLuaTableFromEnumType( pState, "GameDifficulty", "Difficulty" );

	lua_pop( pState, 1 );
}

C_UiManager::~C_UiManager()
{
	delete m_pAdvertisingMessageController;

	RemoveAllSubscribers();

	C_ProfileInterface::Instance().GamerPictureManager()->Destroy();

	g_pUILua->End();
	gUITextureManager.DestroyAll(); // Delete all the textures bundles
	C_TextStringID::LocTable().DestroyAll();
	gFont.Destroy();
	gButtons.ExitCreator();
	gSimple2D.Exit();

	g_UIXt.Cleanup();

	for(U32 i = 0; i < MAX_MOVIE_SLOTS; i ++)
		gMovie[i].Destroy();

	gUIParticleMan.Exit();

	SAFE_DELETE_PTR( g_pUILua );

#ifndef _REMOVE_STRING_STORE_
	C_StringStore::Instance().Exit();
#endif
	// Clean up loaded shapes.
	C_AdobeSceneManager::Instance().FlushData();

	C_UIDataFinder::Instance().Cleanup();

	C_FontStyle::Instance().Cleanup();

	for( U32 i = 0; i < m_NumLoadingTextures; i++ )
	{
		LoadingTextureDestroy( i );
		SAFE_DELETE_PTR( m_pLoadingAlternateTexture[ i ] );
	}

	for( U32 i = 0; i < m_MaxLocalPlayers; i++ )
		SAFE_DELETE_PTR( m_pMiniMapRenderTargetAlternateTexture[ i ] );

	for( U32 i = 0; i < m_MovieSlots; i++ )
		SAFE_DELETE_PTR( m_pMovieAlternateTexture[ i ] );

	SAFE_DELETE_PTR( m_pMirrorAlternateTexture );
	SAFE_DELETE_PTR( m_pPhotoModeAlternateTexture );
	SAFE_DELETE_PTR( m_pRenderBufferAlternateTexture );
	SAFE_DELETE_PTR( m_pIgRenderBufferAlternateTexture );
	SAFE_DELETE_PTR( m_pMirrorGlassAlternateTexture );
#ifdef PLATFORM_PC
	// [Blur2PC] T256
	gFont.SetInputGlyphTexture(C_TextureHandleIndex());
	SAFE_DELETE_PTR(m_pControllerGlyphTexture);
#endif

	g_pManager = NULL;
}

void C_UiManager::ProcessController()
{
#ifdef PLATFORM_PC
	// [Blur2PC] Only players allowed to navigate this menu may select its prompts.
	// Use the same profile/lock gate as navigation below, including all-pad start/join screens.
	Input::C_DeviceManager* devices = Input::C_DeviceManager::GetInstance();
	U32 glyphUserMask = 0;
	for (S32 i = 0; i < m_MaxLocalPlayers; ++i)
		if (C_ProfileInterface::Instance().AllowPadInput(i)) glyphUserMask |= 1u << i;
	gFont.SetInputGlyphTexture(devices && m_pControllerGlyphTexture &&
		devices->GetLastActiveDeviceType(glyphUserMask) >= Input::E_Device_Pad360 ?
		m_pControllerGlyphTexture->m_Index : C_TextureHandleIndex());
#endif
	// reset every frame
	m_sActiveControllerDisconnected = -1;

	// If the camera switcher menu is being shown, don't send input to the UI:
	if (AmaxCam::Switcher().UsingInput())
		return;

	if ( !Input::GetGameInputManager() )
		return;

	const U16 delay = 20;
	const U16 repeatTime = 3;

	// Get context mask
	S32 sMask = C_ScreenControlManager::Instance().InputMessageMask();

	// [Blur2PC] Retail PC processes all four slots (0x00c348c0).
	for ( S32 i = 0; i < m_MaxLocalPlayers; i++ )
	{
		// disable certain pads?
		if ( !C_ProfileInterface::Instance().AllowPadInput( i ) )
			continue;
		
		Input::C_GameInput* pInput = Input::GetGameInputManager()->GetGameInput( i );

		if ( !pInput )
			continue;

		const S8 sPadIndex = (S8)i;

#if defined(PLATFORM_PS3)
		bool padDisconnected = false;
		if(sPadIndex < Framework::E_UserInputIndex_Count)
		{
			if(!Framework::C_ApplicationBase::GetInstance()->GetPad( sPadIndex )->IsConnected())
				padDisconnected = true;
		}
#elif defined (PLATFORM_PC)
		bool padDisconnected = false;
		if(sPadIndex < Framework::E_UserInputIndex_Count)
		{
			bool PadConnect = Framework::C_ApplicationBase::GetInstance()->GetPad( sPadIndex )->IsConnected();
			bool KeyboardConnect = Framework::C_ApplicationBase::GetInstance()->GetKeyboard( sPadIndex )->IsConnected();
			if(	!PadConnect && !KeyboardConnect)
			{
				padDisconnected = true;
			}
		}
#endif

//		// check for unplugging of the controller
//#if defined( PLATFORM_XENON )
//		XINPUT_STATE inputState;
//		if ( XInputGetState( (DWORD)sPadIndex, &inputState ) == ERROR_DEVICE_NOT_CONNECTED )
//			padDisconnected = true;
//#elif defined( PLATFORM_PS3 ) && CELL_SDK_VERSION < 0x250000
//		CellPadCapabilityInfo info;
//		if ( cellPadGetCapabilityInfo( (S32)sPadIndex, &info ) == CELL_PAD_ERROR_NO_DEVICE )
//			padDisconnected = true;
//#endif

		if(padDisconnected)
		{
			if ( ( SysProfile::C_Manager::Instance().GetPrimaryPadNo() != -1 ) && !C_ProfileInterface::Instance().AllowAllPadInput() )
				m_sActiveControllerDisconnected = i;

			// only add in when we _know_ it'll be caught (or at least processed)
			if ( gUIScreenControl.GetScreenTime() == SCREEN_TIME_NORMAL )
			{
				_UI_MESSAGE1_MASK( E_UiMsg_ControllerDisconnected, sPadIndex, sMask );
//				UI::C_UiMessageBase message( E_UiMsg_ControllerDisconnected, sPadIndex );
//				UI::gButtons.GetDeferredUiMessageQueue().BroadcastMessage( &message, sMask );
			}
			continue;
		}

		// only add messages when we _know_ they'll be processed
		if ( gUIScreenControl.GetScreenTime() == SCREEN_TIME_NORMAL )
		{
			if( m_MaskXTime[ i ] )
				m_MaskXTime[ i ]--;

			if( m_MaskYTime[ i ] )
				m_MaskYTime[ i ]--;

			bool u = pInput->GetButtonAutoRepeat( Game::E_UiButton_PadOrAxis_Up, delay, repeatTime );
			bool d = pInput->GetButtonAutoRepeat( Game::E_UiButton_PadOrAxis_Down, delay, repeatTime );
			bool l = pInput->GetButtonAutoRepeat( Game::E_UiButton_PadOrAxis_Left, delay, repeatTime );
			bool r = pInput->GetButtonAutoRepeat( Game::E_UiButton_PadOrAxis_Right, delay, repeatTime );

			if( m_MaskYTime[ i ] )
			{
				u = false;
				d = false;
			}

			if( m_MaskXTime[ i ] )
			{
				l = false;
				r = false;
			}

			// U, D, L, R, A, B, X, Y, RS, LS  - exclusive
			if ( u )
			{
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Up, sPadIndex, true, 1.0, 0, sMask );
				m_MaskXTime[ i ] = 3;
			}
			else
			if ( d )
			{
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Down, sPadIndex, true, 1.0, 0, sMask );
				m_MaskXTime[ i ] = 3;
			}
			else			
			if ( l )
			{
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Left, sPadIndex, true, 1.0, 0, sMask );
				m_MaskYTime[ i ] = 3;
			}
			else
			if ( r )
			{
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Right, sPadIndex, true, 1.0, 0, sMask );
				m_MaskYTime[ i ] = 3;
			}
			else
			if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_Confirm ) )
			{
				F64 inputDevice = 0.0;
#ifdef PLATFORM_PC
				// [Blur2PC] Retail PC (0x00c348c0) passes the type of the device that pressed the button
				// (keyboard 0, mouse 1, pad 2) with Confirm, Cancel, X and Y; Lua reads it as arg4.
				inputDevice = (F64)pInput->GetButtonDevice( Game::E_UiButton_Confirm );
#endif
				if ( m_bNormalNextButtonMapping )
					_UI_MESSAGE_PAD( E_UiMsg_MenuNext, sPadIndex, true, 1.0, inputDevice, sMask );
				else
					_UI_MESSAGE_PAD( E_UiMsg_MenuBack, sPadIndex, true, 1.0, inputDevice, sMask );
			}
			else
			if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_Cancel ) )
			{
#ifdef PLATFORM_PC
				const F64 inputDevice = (F64)pInput->GetButtonDevice( Game::E_UiButton_Cancel );	// [Blur2PC] retail 0x00c348c0
#else
				const F64 inputDevice = 0.0;
#endif
				if ( m_bNormalNextButtonMapping )
					_UI_MESSAGE_PAD( E_UiMsg_MenuBack, sPadIndex, true, 1.0, inputDevice, sMask );
				else
					_UI_MESSAGE_PAD( E_UiMsg_MenuNext, sPadIndex, true, 1.0, inputDevice, sMask );
			}
			else
			if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_Option1 ) )
			{
#ifdef PLATFORM_PC
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_X, sPadIndex, true, 1.0, (F64)pInput->GetButtonDevice( Game::E_UiButton_Option1 ), sMask );	// [Blur2PC] retail 0x00c348c0
#else
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_X, sPadIndex, true, 1.0, 0, sMask );
#endif
			}
			else
			if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_Option2 ) )
			{
#ifdef PLATFORM_PC
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Y, sPadIndex, true, 1.0, (F64)pInput->GetButtonDevice( Game::E_UiButton_Option2 ), sMask );	// [Blur2PC] retail 0x00c348c0
#else
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Y, sPadIndex, true, 1.0, 0, sMask );
#endif
			}
			else
			if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_LeftShoulder ) )
			{
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_LeftShoulder, sPadIndex, true, 1.0, 0, sMask );
			}
			else
			if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_RightShoulder ) )
			{
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_RightShoulder, sPadIndex, true, 1.0, 0, sMask );
			}
			
			// start and back
			#if !defined(CFG_GOLD)
			bool debugMenuActivatePressed = false;

			if(pInput->GetButtonPressed( Framework::E_Button_DebugMenuActivate ))
				debugMenuActivatePressed = true;

			if(!debugMenuActivatePressed)
			#endif
			{
				if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_Start ) )
					_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Start, sPadIndex, true, 1.0, 0, sMask );		

				if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_Back ) )
					_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Back, sPadIndex, true, 1.0, 0, sMask );		

				if ( pInput->GetButtonPressed( Game::E_UiButton_Back ) )
					_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Back, sPadIndex, false, 1.0, 0, sMask );
			}

			// triggers
			if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_LeftTrigger ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_LeftTrigger, sPadIndex, true, 1.0, 0, sMask );
				
			if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_RightTrigger ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_RightTrigger, sPadIndex, true, 1.0, 0, sMask );

			// thumbs
			if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_LeftThumb ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_LeftThumb, sPadIndex, true, 1.0, 0.0, sMask );

			if ( pInput->GetButtonPressedDebounced( Game::E_UiButton_RightThumb ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_RightThumb, sPadIndex, true, 1.0, 0.0, sMask );

			// sticks
			if ( pInput->GetAxisValue( Game::E_UiAxis_LeftStick_LR ) != 0.5f )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlLeftStick_X, sPadIndex, false, ( ( pInput->GetAxisValue( Game::E_UiAxis_LeftStick_LR ) - 0.5f ) * 2.0f ), 0, sMask );

			if ( pInput->GetAxisValue( Game::E_UiAxis_LeftStick_UD ) != 0.5f )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlLeftStick_Y, sPadIndex, false, ( ( pInput->GetAxisValue( Game::E_UiAxis_LeftStick_UD ) - 0.5f ) * 2.0f ), 0, sMask );

			if ( pInput->GetAxisValue( Game::E_UiAxis_RightStick_LR ) != 0.5f )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlRightStick_X, sPadIndex, false, ( ( pInput->GetAxisValue( Game::E_UiAxis_RightStick_LR ) - 0.5f ) * 2.0f ), 0, sMask );

			if ( pInput->GetAxisValue( Game::E_UiAxis_RightStick_UD ) != 0.5f )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlRightStick_Y, sPadIndex, false, ( ( pInput->GetAxisValue( Game::E_UiAxis_RightStick_UD ) - 0.5f ) * 2.0f ), 0, sMask );
		}
	}	

#ifndef CFG_GOLD
	// debug stuff
	Input::C_GameInput& input = *Input::GetGameInputManager()->GetGameInput( 0 );

	// Only allow screen reload on down + x and if loadfromHD is on
	if ( g_debugUseLUAFromHDD && input.GetButtonPressed( Game::E_UiButton_Pad_Down ) && input.GetButtonPressedDebounced( Game::E_UiButton_Option1 ) )
	{
		// Clean up subscreens
		for( S32 i = 0; i < C_ScreenControlManager::Instance().Count(); i++ )
		{
			// Everything 'but' the main context
			if( i == E_Context_Main )
				continue;

			C_ScreenControlManager::Instance().Get( i ).SetEndLoop();
			C_ScreenControlManager::Instance().Get( i ).SetEnd();

			C_ScreenControlManager::Instance().Get( i ).Exit();
		}

		// Relaunch screen on the main context
		C_ScreenControlManager::Instance().Get( E_Context_Main ).SetNextScreen( C_ScreenControlManager::Instance().Get( E_Context_Main ).GetCurrentScreen() );
		return;
	}
	if ( g_UiDebugReloadLocs && input.GetButtonPressed( Game::E_UiButton_Pad_Down ) && input.GetButtonPressedDebounced( Game::E_UiButton_Option2 ) )
	{
		// reload loc
		C_TextStringID::LocTable().Reload();
	}

	// debug 2D camera movement
	if ( g_UICameraDebugActive2D )
	{
		C_DebugCamUpdate2D i;
		i.lx = ( input.GetAxisValue( Game::E_UiAxis_LeftStick_LR ) - 0.5f ) * 2.0f;
		i.ly = ( input.GetAxisValue( Game::E_UiAxis_LeftStick_UD ) - 0.5f ) * 2.0f;
		i.lt = input.GetButtonValue( Game::E_UiButton_LeftTrigger );
		i.rt = input.GetButtonValue( Game::E_UiButton_RightTrigger );
		C_UICamera::Instance().DebugUpdate2D( i );
	}

	// debug 3D camera movement
	if ( g_UICameraDebugActive3D )
	{
		C_DebugCamUpdate3D i;
		i.lx = ( input.GetAxisValue( Game::E_UiAxis_LeftStick_LR ) - 0.5f ) * 2.0f;
		i.ly = ( input.GetAxisValue( Game::E_UiAxis_LeftStick_UD ) - 0.5f ) * 2.0f;
		i.rx = ( input.GetAxisValue( Game::E_UiAxis_RightStick_LR ) - 0.5f ) * 2.0f;
		i.ry = ( input.GetAxisValue( Game::E_UiAxis_RightStick_UD ) - 0.5f ) * 2.0f;
		i.lt = input.GetButtonValue( Game::E_UiButton_LeftTrigger );
		i.rt = input.GetButtonValue( Game::E_UiButton_RightTrigger );
		C_UICamera::Instance().DebugUpdate3D( i );
	}
#endif
}

void C_UiManager::SetSchedulerGroup( int iGroup)
{
	m_SchedulerGroup = iGroup;
}

#if defined(PLATFORM_PC)
// [Blur2PC] Automation scripts (system/debug/Blur2PCAutomation.h): Lua helpers, `lua` lines, `wait`
// conditions and screen-change logging. Runs only while BLUR2PC_AUTOMATION names a script.
namespace
{
	int Blur2PCLua_RaceState( lua_State* pState )
	{
		static const char* const s_names[] = { "None", "Loading", "Intro", "Starting", "Racing", "Finished", "Outro", "Quitting", "Restarting", "GoingToNextEvent" };
		const char* name = "None";
		Game::C_RaceBookLayer* pRaceBookLayer = GetRaceBookLayer();
		Game::C_RaceLogicLayer* pRaceLogic = pRaceBookLayer ? pRaceBookLayer->GetRaceLogicLayer() : NULL;
		if ( pRaceLogic )
		{
			const int state = (int)pRaceLogic->GetRaceState();
			if ( state >= 0 && state < (int)( sizeof( s_names ) / sizeof( s_names[0] ) ) )
				name = s_names[state];
		}
		lua_pushstring( pState, name );
		return 1;
	}

	int Blur2PCLua_Tap( lua_State* pState )
	{
		const char* name = lua_tostring( pState, 1 );
		const int dik = name ? Blur2PCAutomation::KeyFromName( name ) : -1;
		if ( dik < 0 )
			return luaL_error( pState, "Blur2PC.Tap: unknown key %s", name ? name : "(nil)" );
		lua_pushboolean( pState, Blur2PCAutomation::QueueTap( dik ) );
		return 1;
	}

	int Blur2PCLua_Log( lua_State* pState )
	{
		const char* text = lua_tostring( pState, 1 );
		Blur2PCAutomation::LogF( "lua: %s", text ? text : "(nil)" );
		return 0;
	}

	int Blur2PCLua_Run( lua_State* pState )
	{
		const char* name = lua_tostring( pState, 1 );
		if ( !name )
			return luaL_error( pState, "Blur2PC.Run(file): file name expected" );
		char path[MAX_PATH];
		const bool absolute = ( name[0] && name[1] == ':' ) || name[0] == '\\' || name[0] == '/';
		_snprintf_s( path, MAX_PATH, _TRUNCATE, "%s%s", absolute ? "" : Blur2PCAutomation::ScriptDir(), name );
		FILE* f = fopen( path, "rb" );
		if ( !f )
			return luaL_error( pState, "Blur2PC.Run: cannot open %s", path );
		fseek( f, 0, SEEK_END );
		const long size = ftell( f );
		fseek( f, 0, SEEK_SET );
		char* buffer = (char*)malloc( size > 0 ? size : 1 );
		const size_t got = fread( buffer, 1, size > 0 ? size : 0, f );
		fclose( f );
		char chunkName[MAX_PATH + 1];
		_snprintf_s( chunkName, sizeof( chunkName ), _TRUNCATE, "@%s", name );
		const int status = luaL_loadbuffer( pState, buffer, got, chunkName );
		free( buffer );
		if ( status != 0 )
			return lua_error( pState );
		const int base = lua_gettop( pState ) - 1;
		lua_call( pState, 0, LUA_MULTRET );
		return lua_gettop( pState ) - base;
	}

	// Runs automation Lua the way C_ScreenControl runs a main-context screen's callbacks: context 0
	// selected, and the screen's globals (GUI, SCUI) bound by GUIBank's SetContextTable.
	class C_Blur2PCMainContextScope
	{
	public:
		C_Blur2PCMainContextScope( lua_State* pState ) : m_pState( pState ), m_Saved( C_ScreenControlManager::Instance().Context() ), m_Bound( false )
		{
			C_ScreenControlManager::Instance().Context( 0 );
			lua_getglobal( m_pState, "SetContextTable" );
			if ( lua_isfunction( m_pState, -1 ) )
			{
				lua_pushnumber( m_pState, 0 );
				if ( lua_pcall( m_pState, 1, 1, 0 ) == 0 )
					m_Bound = lua_istable( m_pState, -1 ) ? true : false;
			}
			lua_pop( m_pState, 1 );
		}
		~C_Blur2PCMainContextScope()
		{
			if ( m_Bound )
			{
				lua_getglobal( m_pState, "ResetContextTable" );
				if ( lua_pcall( m_pState, 0, 0, 0 ) != 0 )
					lua_pop( m_pState, 1 );
			}
			C_ScreenControlManager::Instance().Context( m_Saved );
		}
	private:
		lua_State*	m_pState;
		S32			m_Saved;
		bool		m_Bound;
	};

	void Blur2PCAutomationUiUpdate()
	{
		if ( !Blur2PCAutomation::IsActive() || !g_pUILua || !g_pUILua->GetState() )
			return;
		lua_State* pState = g_pUILua->GetState();

		lua_getglobal( pState, "Blur2PC" );
		const bool registered = !lua_isnil( pState, -1 );
		lua_pop( pState, 1 );
		if ( !registered )
		{
			static const luaL_Reg s_lib[] = { { "RaceState", Blur2PCLua_RaceState }, { "Tap", Blur2PCLua_Tap }, { "Log", Blur2PCLua_Log }, { "Run", Blur2PCLua_Run }, { NULL, NULL } };
			luaL_register( pState, "Blur2PC", s_lib );
			lua_pop( pState, 1 );
		}

		// screen changes in every context
		static char s_screens[8][128];
		const S32 contexts = C_ScreenControlManager::Instance().Count() < 8 ? C_ScreenControlManager::Instance().Count() : 8;
		for ( S32 i = 0; i < contexts; ++i )
		{
			const char* screen = C_ScreenControlManager::Instance().Get( i ).GetCurrentScreen();
			if ( !screen ) screen = "";
			if ( strcmp( screen, s_screens[i] ) != 0 )
			{
				strncpy_s( s_screens[i], screen, _TRUNCATE );
				Blur2PCAutomation::LogF( "screen %d: %s", i, screen[0] ? screen : "(none)" );
			}
		}

		if ( const char* code = Blur2PCAutomation::TakeLuaRequest() )
		{
			C_Blur2PCMainContextScope scope( pState );
			Blur2PCAutomation::LogF( "lua %s", code );
			if ( luaL_loadbuffer( pState, code, strlen( code ), "=automation lua" ) != 0 || lua_pcall( pState, 0, 0, 0 ) != 0 )
			{
				Blur2PCAutomation::LogF( "lua error: %s", lua_tostring( pState, -1 ) );
				lua_pop( pState, 1 );
			}
		}

		if ( const char* condition = Blur2PCAutomation::WaitCondition() )
		{
			C_Blur2PCMainContextScope scope( pState );
			static char s_lastError[512];
			char chunk[Blur2PCAutomation::k_MaxText + 16];
			_snprintf_s( chunk, sizeof( chunk ), _TRUNCATE, "return (%s)", condition );
			if ( luaL_loadbuffer( pState, chunk, strlen( chunk ), "=automation wait" ) != 0 || lua_pcall( pState, 0, 1, 0 ) != 0 )
			{
				const char* error = lua_tostring( pState, -1 );
				if ( error && strcmp( error, s_lastError ) != 0 )
				{
					strncpy_s( s_lastError, error, _TRUNCATE );
					Blur2PCAutomation::LogF( "wait error: %s", error );
				}
			}
			else if ( lua_toboolean( pState, -1 ) )
			{
				Blur2PCAutomation::WaitMet();
			}
			lua_pop( pState, 1 );
		}
	}
}
#endif

void C_UiManager::Process( const Maths::C_TimeS& time )
{
	gUITextureManager.Update();

	if( UiXt::C_FontStyleList::Instance().IsDirty() )
		UiXt::C_FontStyleList::Instance().FlushBind();

	// update the friends list
	C_UIFriends::Instance().Update();

	theFETimer.Update( time );

#ifndef CFG_GOLD
	if(g_UiProcessController)
#endif
	{
		ProcessController();
	}

	{
		gSimple2D.StartFrame();
		//gSimple3D.StartFrame();
		gFont.StartFrame();
	}

	{
		{
			gDashboardInterface.Update();
			gGameSaveSystem.Update();
		}

		{
#ifndef CFG_GOLD
			C_UILua::C_StackCheck c = g_pUILua->CreateStackCheck();
#endif
			for( S32 i = 0; i < C_ScreenControlManager::Instance().Count( ); i++ )
				C_ScreenControlManager::Instance().Get( i ).Update();

#ifdef PLATFORM_PS3
			RenderBlackRectangle();
#endif

			// invites
			if ( m_fInviteTimer > 0.0f )
			{
				F32 fAlpha = Maths::Clamp01( m_fInviteTimer / m_fInviteShowTime );

				C_UIColour black = 0xF0000000;
				C_UIColour white = 0xFFFFFFFF;
				black.A01( fAlpha );
				white.A01( fAlpha );

				static F32 fInviteBoxHeight = 72.0f;
				static F32 fInviteTextHeight = 20.0f;
				static F32 fTextY = 36.0f;

				gSimple2D.AddSprite( gSimple2D.TopLayer(), Maths::HVec3( 320.0f, fInviteBoxHeight, 0.0f ), 
					gSimple2D.ScreenWidth() + 2, fInviteBoxHeight + 2,
					black, 0, E_SpriteType_Sprite, C_RenderState::Instance() );
				gSimple2D.AddJustification( E_Justify_BottomCentre );
				gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );

				gFont.DrawText( gSimple2D.TopLayer(), Maths::HVec3( 320.0f, fTextY, 0.0f ), E_Justify_TopCentre, 
					C_TextStringID( "PRO_FRIENDS_NEW_INVITE" ).String(), white, fInviteTextHeight, C_FontStyle::Instance(), 0.0f, 0 );

				m_fInviteTimer -= time.Get();
			}

		#ifdef PLATFORM_PS3
			HandleControllerDisconnects( time.Get() );
		#endif

#if !defined (CFG_GOLD)
			if (g_Net_ShowBandwidthUsage)
			{
				C_WideString64 str;
				F32 out, in;
				RacerNet::C_NetServices::Instance().DebugGetBandwidthUsage(in, out);
				str.Format( L" In : %.2f kbps", in*8);
				gFont.DrawDebugText( gSimple2D.TopLayer() , Maths::HVec3( gSimple2D.ScreenLeft()+50, 370.0f, 0.0f ), E_Justify_MiddleLeft, str, 0xFF0000E0, 14.0f );
				str.Format( L" Out : %.2f kbps", out*8);
				gFont.DrawDebugText( gSimple2D.TopLayer(), Maths::HVec3( gSimple2D.ScreenLeft()+50, 385.0f, 0.0f ), E_Justify_MiddleLeft, str, 0xFF0000E0, 14.0f );
			}
#endif
//			const U64 ticks = Time::GetPerformanceCounter();
			gButtons.ProcessDeferredMessages();

//			LOGWARN( g_Log, "%d", Time::GetPerformanceCounter() - ticks );
		}

		{
			for(U32 i = 0; i < MAX_MOVIE_SLOTS; i ++)
				gMovie[i].Update();
		}
	}

	{
		gUIParticleMan.Render();
		gUIParticleMan.Update( time.Get() );
	}


	LightPaint_Update( time.Get() );

	// update the camera
	C_UICamera::Instance().Update();

#ifndef CFG_GOLD
	gSimple2D.AddDebugHelpers();
#endif
	// Update sorcery data
	UiXt::C_ColourStyleList::Instance().FrameUpdate( g_pUILua->GetState() );
	UiXt::C_ScreenList::Instance().FrameUpdate();

	C_AdobeSceneManager::Instance().Update();

#ifndef GFG_GOLD
	// update the LUA console
	g_pUILua->DoConsole();
#endif

#if defined(PLATFORM_PC)
	Blur2PCAutomationUiUpdate();	// [Blur2PC]
#endif

	// update profile stuff
	RacerNet::C_NetServices& ns =  RacerNet::C_NetServices::Instance();
	ns.EnterLock();
	C_ProfileInterface::Instance().Update();
	ns.Exitlock();

	if ( ns.IsInitialised() )
	{
		// Check 
		const bool state = ns.Router().IsOnlineMode();
		
		// If we was online but we are now offline then inform the user that we have lost the connection
		if ( m_PlatformConnectionState && !state )
		{
		}
		m_PlatformConnectionState = state;
	}

	if (!g_UiRunDeferred)
	{
		DeferredUpdate();
	}
	else
		Scheduler::AddTask(UiUpdateModule, this, 0, m_SchedulerGroup);

	Game::C_FriendChallenges::Instance().Update();

	// [Blur2PC] retail Blur PC (C_UiManager::Process, 0x00c373b0, T146): a UISystem.SystemQuitGame request
	// is latched here and acted on by C_GameApp::OnUpdate
	if( g_Blur2PCSystemQuitGame )
		m_Blur2PCQuitGame = true;
}

#ifdef PLATFORM_PS3
void C_UiManager::HandleControllerDisconnects( F32 fFrameTime )
{
	if ( ( m_sActiveControllerDisconnected != -1 ) && ( m_fDisconnectTimer < m_fDisconnectShowTime ) )
		m_fDisconnectTimer = Maths::Clamp( m_fDisconnectTimer + fFrameTime, 0.0f, m_fDisconnectShowTime );
	else
	if ( ( m_sActiveControllerDisconnected == -1 ) && ( m_fDisconnectTimer != 0.0f ) )
		m_fDisconnectTimer = Maths::Clamp( m_fDisconnectTimer - fFrameTime, 0.0f, m_fDisconnectShowTime );

	if ( m_fDisconnectTimer == 0.0f )
		return;

	C_UIColour black = 0xff000000;
	C_UIColour white = 0xffffffff;

	static F32 fBoxOffset = 60.0f;
	static F32 fTextHeight = 20.0f;
	static F32 fTextOffset = 0.0f;

	F32 y = 482.0f - ( fBoxOffset * ( m_fDisconnectTimer / m_fDisconnectShowTime ) );

	gSimple2D.AddSprite( gSimple2D.TopLayer(), Maths::HVec3( 320.0f, y, 0.0f ), 
		gSimple2D.ScreenWidth() + 2, fBoxOffset + 2,
		black, 0, E_SpriteType_Sprite, C_RenderState::Instance() );
	gSimple2D.AddJustification( E_Justify_TopCentre );
	gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );

	if ( m_sActiveControllerDisconnected != -1 )
	{
		C_WideString128 buffer;
		buffer.Format( C_TextStringID( "PRO_CONTROLLER_DISCONNECTED" ).String(), m_sActiveControllerDisconnected + 1 );

		gFont.DrawText( gSimple2D.TopLayer(), Maths::HVec3( 320.0f, y + fTextOffset, 0.0f ), E_Justify_TopCentre, buffer, white, fTextHeight, C_FontStyle::Instance(), 0.0f, 0 );
	}
}
#endif

void DeferredUpdate(Scheduler::Task *pTask)
{
	C_UiManager *p = (C_UiManager*)pTask->Input;
	p->DeferredUpdate();
}

void C_UiManager::DeferredUpdate()
{
	PERF_EVENT("UI.DeferredUpdate");
#if 0 
	UI::C_UiMessageBase< E_UiMsg_Render > message;

	for( S32 i = 0; i < C_ScreenControlManager::Instance().Count( ); i++ )
		if( C_ScreenControlManager::Instance().Get( i ).DoRender() )
			gButtons.BroadcastMessageNow( C_ScreenControlManager::Instance().Get( i ).GetMessageMask(), &message );
#endif

	// Build the sprite batches
	gSimple2D.BuildSpriteBatches();

	// set audio eq values
	Audio::Subsystem* pSystem = Audio::InterfaceController::Get().GetSubsystem( Game::C_GameAudioLayer::eUISubsystem );

	if ( pSystem )
	{
		UISubsystem* pUiSystem = (UISubsystem*)pSystem;
		AudioEqValues& values = pUiSystem->GetEqValues();

		for ( S32 i = 0; i < values.Size(); i++ )
		{
			F32 fNewValue = values[ i ];

			if ( g_UiDebugAudioEqDamp == -1.0f )
				m_AudioEq[ i ] = fNewValue;
			else
			{
				if ( fNewValue > m_AudioEq[ i ] )
					m_AudioEq[ i ] = fNewValue;
				else
				{
					m_AudioEq[ i ] -= ( g_UiDebugAudioEqDamp * theFETimer.FrameTime() );

					if ( m_AudioEq[ i ] < 0.0f )
						m_AudioEq[ i ] = 0.0f;
				}
			}
		}

		UI::gSimple2D.GetRenderer().SetAudioEqValues( 
			m_AudioEq[ 0 ], m_AudioEq[ 1 ], m_AudioEq[ 2 ], m_AudioEq[ 3 ], 
			m_AudioEq[ 4 ], m_AudioEq[ 5 ], m_AudioEq[ 6 ], m_AudioEq[ 7 ] 
		);
	}
	else
		UI::gSimple2D.GetRenderer().SetAudioEqValues();
}

void C_UiManager::PreRender()
{
	// Store up panel view matrix
	for ( S32 i = 0; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); i++ )
	{
		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView( i );
		const Display::C_ViewSettings3D& settings3D = view.m_View.GetViewSettings3D();

		// E_GameSpritePanel_VPn
		gSimple2D.SetTransformMatrixByPanel( E_GameSpritePanel_VP0 + i, settings3D.GetWorldToDevice() );
#if PLATFORM_PC
		if( i == 0 )
		{	// [Blur2PC] render interpolation: the sprites in this panel were placed at the car's update pose
			// (the UI builds its sprite lists during the game update); move them to the pose the car is drawn at
			Maths::HMat44 carPanel = settings3D.GetWorldToDevice();
			Maths::HMat43 delta;
			if( UIPerks2::Blur2PCCarPanelDelta( delta ) )
				carPanel = Maths::HMat44( delta ) * carPanel;
			gSimple2D.SetTransformMatrixByPanel( E_GameSpritePanel_VP0_CAR, carPanel );
			UIAiHud::Blur2PCPlaceCarSprites();	// other sprites that follow a car (AI badges, countdown), likewise
		}
#endif

		if( i == 0 )
		{			
			// E_GameSpritePanel_3DAA_WORLD
#ifdef PLATFORM_PC
			// [Blur2PC] PC always uses the camera-scale path. Its projections already
			// follow the viewport aspect, so frontend and in-game both use unit scale.
			if( false )
#else
			if( gSimple2D.WideScreen() )
#endif
			{
				gSimple2D.SetTransformMatrixByPanel( E_GameSpritePanel_3DAA_WORLD, settings3D.GetWorldToDevice() );
			}
			else
			{
				// The 4:3 squish
				const F32 fScale = C_UICamera::Instance().GetSettings3D( 0 ).Scale();
				const Maths::HMat44 sm( Maths::HMat33( Maths::C_MatrixScale( fScale, fScale, 1.0f ) ) );

				gSimple2D.SetTransformMatrixByPanel( E_GameSpritePanel_3DAA_WORLD, (settings3D.GetWorldToCamera() * sm) * settings3D.GetCameraToDevice() );
			}
			// Copy into E_GameSpritePanel_3D_WORLD
			gSimple2D.SetTransformMatrixByPanel( E_GameSpritePanel_3D_WORLD, gSimple2D.GetTransformMatrixByPanel( E_GameSpritePanel_3DAA_WORLD ) );
		}
	}
	
	LightPaint_Render();
}

void C_UiManager::Render2DBG( )
{
	// _2DBG
#ifndef CFG_GOLD
	if( g_UiShouldRenderPanel_2DBG )
#endif
	if( !gSimple2D.ShouldRender( E_GameSpritePanel_2DBG ) )
		return;

	// For things rendered *before* the postprocess stuff gets composited
	gSimple2D.PreRender();
	gSimple2D.SetTransformMatrix2D();

#ifndef CFG_GOLD
	if ( g_UICameraDebugActive2D )
		gSimple2D.SetTransformMatrix2D( C_UICamera::Instance().GetSettings2D( 0 ).GetTransformMatrix() );
#endif

	gSimple2D.RenderPanel( E_GameSpritePanel_2DBG );

	gSimple2D.PostRender();
}

void C_UiManager::RenderIngameAA( const bool postWorldRender )
{
	if( C_GameApp::GetInstance()->LoadingScreenEnabledState() )
		return;

	const PanelIndex panel( postWorldRender ? E_GameSpritePanel_2DIG_B : E_GameSpritePanel_2DIG_A );

	// _2DIG_x
#ifndef CFG_GOLD
	if( (panel == E_GameSpritePanel_2DIG_A && g_UiShouldRenderPanel_2DIG_A == false) || 
		(panel == E_GameSpritePanel_2DIG_B && g_UiShouldRenderPanel_2DIG_B == false) )
		return;
#endif

	if( !gSimple2D.ShouldRender( panel ) )
		return;

	gSimple2D.PreRender();
	gSimple2D.SetTransformMatrix2D( C_UICamera::Instance().GetSettings2D( 0 ).GetTransformMatrix() );
	gSimple2D.SetZEnable( false );

	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Red | Display::E_ColourWriteEnable_Green | Display::E_ColourWriteEnable_Blue);

	gSimple2D.RenderPanel( panel );

	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);

	gSimple2D.PostRender();
}

void C_UiManager::RenderRT( const U32 viewportIndex )
{
	// resolve render targets
	gSimple2D.SetTransformMatrix2D( m_TransformRT0 );
	gSimple2D.PreRender();

	gSimple2D.RenderPanel( E_GameSpritePanel_RT0 + viewportIndex );

	gSimple2D.PostRender();
}

void C_UiManager::RenderAA( )
{
	PERF_EVENT("UI.RenderAA");

	gSimple2D.PreRender();

	// render in player viewports (3D game camera)
	gSimple2D.SetZEnable( false );

	if( gSimple2D.ShouldRender( E_GameSpritePanel_2DBGAA ) )
	{
		gSimple2D.SetTransformMatrix2D();

		// debug 2D camera movement
#ifndef CFG_GOLD
		if ( g_UICameraDebugActive2D )
			gSimple2D.SetTransformMatrix2D( C_UICamera::Instance().GetSettings2D( 0 ).GetTransformMatrix() );
#endif
		gSimple2D.RenderPanel( E_GameSpritePanel_2DBGAA );
	}

	if ( Framework::C_ApplicationBase::GetInstance() && ( Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() > 0 ) )
	{
		Display::C_Viewport viewport = Display::Device()->GetViewport();

		// ratio between this rendertarget and the fullscreen one
		U32 uWidth = Display::Device()->GetBackBuffer().GetWidth();
		U32 uHeight = Display::Device()->GetBackBuffer().GetHeight();

		F32 fRatioX = viewport.GetW() / (F32)uWidth;
		F32 fRatioY = viewport.GetH() / (F32)uHeight;

		for ( S32 i = 0; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); i++ )
		{
			const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView( i );
			const Display::C_ViewSettings3D& settings3D = view.m_View.GetViewSettings3D();

			Display::C_Viewport viewportMe = view.m_View.GetViewport();
			viewportMe.m_X = (U32)( viewportMe.m_X * fRatioX );
			viewportMe.m_Y = (U32)( viewportMe.m_Y * fRatioY );
			viewportMe.m_W = (U32)( viewportMe.m_W * fRatioX );
			viewportMe.m_H = (U32)( viewportMe.m_H * fRatioY );

			Display::Device()->SetViewport( viewportMe );

			// _VPx
#ifndef CFG_GOLD
			if( (i == 0 && g_UiShouldRenderPanel_VP0) ||
				(i == 1 && g_UiShouldRenderPanel_VP1) ||
				(i == 2 && g_UiShouldRenderPanel_VP2) ||
				(i == 3 && g_UiShouldRenderPanel_VP3) )
#endif
			if( gSimple2D.ShouldRender( E_GameSpritePanel_VP0 + i ) )
			{
				gSimple2D.SetTransformMatrix3D( gSimple2D.GetTransformMatrixByPanel( E_GameSpritePanel_VP0 + i ), settings3D.GetCameraPosition() );
				gSimple2D.RenderPanel( E_GameSpritePanel_VP0 + i );
			}
#if PLATFORM_PC
			if( i == 0 && gSimple2D.ShouldRender( E_GameSpritePanel_VP0_CAR ) )
			{	// [Blur2PC] see PreRender
				gSimple2D.SetTransformMatrix3D( gSimple2D.GetTransformMatrixByPanel( E_GameSpritePanel_VP0_CAR ), settings3D.GetCameraPosition() );
				gSimple2D.RenderPanel( E_GameSpritePanel_VP0_CAR );
			}
#endif

			// _3DAA_WORLD
#ifndef CFG_GOLD
			if( g_UiShouldRenderPanel_3DAA_WORLD )
#endif
			if( gSimple2D.ShouldRender( E_GameSpritePanel_3DAA_WORLD ) && i == 0 )
			{
				C_Renderer::C_Light& light = gSimple2D.GetLight( E_GameSpritePanel_3DAA_WORLD );

				Maths::HMat43 mat;

#ifndef CFG_GOLD
				Maths::HVec3 offset( 0.0f, 0.0f, (F32)g_UiLightOffset );
#else
				Maths::HVec3 offset( 0.0f, 0.0f, 2.0f );
#endif

				light.m_Dir = settings3D.GetCameraToWorld().AZ();

				offset = offset * settings3D.GetCameraToWorld().Rotation();
				light.m_Pos = settings3D.GetCameraPosition() + offset;

				gSimple2D.SetTransformMatrix3D( gSimple2D.GetTransformMatrixByPanel( E_GameSpritePanel_3DAA_WORLD ), settings3D.GetCameraPosition() );
				gSimple2D.RenderPanel( E_GameSpritePanel_3DAA_WORLD );
			}
		}

		Display::Device()->SetViewport( viewport );
	}

	// _3DAA_Light
#ifndef CFG_GOLD
	if( g_UiShouldRenderPanel_3DAA_LIGHT )
#endif
	if( gSimple2D.ShouldRender( E_GameSpritePanel_3DAA_LIGHT ) )
	{
		gSimple2D.SetTransformMatrix3D( C_UICamera::Instance().GetSettings3D( 0 ).GetViewProjMatrix(), C_UICamera::Instance().GetSettings3D( 0 ).EyePosition() );
		gSimple2D.RenderPanel( E_GameSpritePanel_3DAA_LIGHT );
	}

	// _3DAA_0
#ifndef CFG_GOLD
	if( g_UiShouldRenderPanel_3DAA_0 )
#endif
	if( gSimple2D.ShouldRender( E_GameSpritePanel_3DAA_0 ) )
	{
		gSimple2D.SetTransformMatrix3D( C_UICamera::Instance().GetSettings3D( 0 ).GetViewProjMatrix(), C_UICamera::Instance().GetSettings3D( 0 ).EyePosition() );
		gSimple2D.RenderPanel( E_GameSpritePanel_3DAA_0 );
	}

	// _3DAA_1
#ifndef CFG_GOLD
	if( g_UiShouldRenderPanel_3DAA_1 )
#endif
	if( gSimple2D.ShouldRender( E_GameSpritePanel_3DAA_1 ) )
	{
		gSimple2D.SetTransformMatrix3D( C_UICamera::Instance().GetSettings3D( 1 ).GetViewProjMatrix(), C_UICamera::Instance().GetSettings3D( 1 ).EyePosition() );
		gSimple2D.RenderPanel( E_GameSpritePanel_3DAA_1 );
	}

	// _2DAA
#ifndef CFG_GOLD
	if( g_UiShouldRenderPanel_2DAA )
#endif
	if( gSimple2D.ShouldRender( E_GameSpritePanel_2DAA ) )
	{
		gSimple2D.SetTransformMatrix2D();

#ifndef CFG_GOLD
		// debug 2D camera movement
		if ( g_UICameraDebugActive2D )
			gSimple2D.SetTransformMatrix2D( C_UICamera::Instance().GetSettings2D( 0 ).GetTransformMatrix() );
#endif
		gSimple2D.RenderPanel( E_GameSpritePanel_2DAA );
	}

	gSimple2D.PostRender();
}

void C_UiManager::RenderNonAA( )
{
	gSimple2D.PreRender();
	gSimple2D.SetZEnable( false );

#ifndef CFG_GOLD
	if( g_UiShouldRenderPanel_3D_WORLD )
#endif
	if( gSimple2D.ShouldRender( E_GameSpritePanel_3D_WORLD ) )
	{
		const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView( 0 );
		const Display::C_ViewSettings3D& settings3D = view.m_View.GetViewSettings3D();

		// Copy settings
		gSimple2D.GetLight( E_GameSpritePanel_3D_WORLD ) = gSimple2D.GetLight( E_GameSpritePanel_3DAA_WORLD );

		gSimple2D.SetTransformMatrix3D( gSimple2D.GetTransformMatrixByPanel( E_GameSpritePanel_3D_WORLD ), settings3D.GetCameraPosition() );
		gSimple2D.RenderPanel( E_GameSpritePanel_3D_WORLD );
	}


	// _3D
#ifndef CFG_GOLD
	if( g_UiShouldRenderPanel_3D )
#endif
	if( gSimple2D.ShouldRender( E_GameSpritePanel_3D ) )
	{
		// Basic 3D
		gSimple2D.SetTransformMatrix3D( C_UICamera::Instance().GetSettings3D( 0 ).GetViewProjMatrix(), C_UICamera::Instance().GetSettings3D( 0 ).EyePosition() );
		gSimple2D.RenderPanel( E_GameSpritePanel_3D );
	}

	// _2D
#ifndef CFG_GOLD
	if( g_UiShouldRenderPanel_2D )
#endif
	if( gSimple2D.ShouldRender( E_GameSpritePanel_2D ) )
	{
		// Basic 2D
		gSimple2D.SetTransformMatrix2D();

#ifndef CFG_GOLD
		// debug 2D camera movement
		if ( g_UICameraDebugActive2D )
			gSimple2D.SetTransformMatrix2D( C_UICamera::Instance().GetSettings2D( 0 ).GetTransformMatrix() );
#endif
		gSimple2D.RenderPanel( E_GameSpritePanel_2D );
	}

	gSimple2D.PostRender();
}

bool C_UiManager::CanInvite( const U32 uIndex )
{
	SysProfile::C_Manager& sysProfileMgr = SysProfile::C_Manager::Instance();
	if (sysProfileMgr.ServiceConnectionState() == SysProfile::C_Manager::eServiceStateConnected)
	{
		// is the friend in a state where er can invites them..
		bool friendStateOk = false;
		const NetPlay::eFriendState fs = C_UIFriends::Instance().GetFriendState(uIndex);
		if (NetPlay::eFriendOnline  == fs)
			friendStateOk = true; // may be different on each platform.

		if (friendStateOk)
		{
			if (!sysProfileMgr.SendingGameInvite())
			{
				// are any of our network sessions in a state where we can invite ?
				RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
				if (ns.IsInitialised())
				{
					// TODO -> do we need to check if the player is actually in our session and not allow invite sending then ?
					NetPlay::SessionSecurity invData;
					if (ns.Race().CanInvite(invData))
						return true;
					else if (ns.Party().CanInvite(invData))
						return true;
				}
			}
		}
	}
	return false;
}

bool C_UiManager::CanJoinSessionInProgress( const U32 uIndex )
{
	SysProfile::C_Manager& sysProfileMgr = SysProfile::C_Manager::Instance();
	if (sysProfileMgr.ServiceConnectionState() == SysProfile::C_Manager::eServiceStateConnected)
	{
		const SysProfile::BaseProfile& profile = C_UIFriends::Instance().GetFriendProfile(uIndex);
		const SysProfile::GameInvite& inv = C_UIFriends::Instance().GetFriendInvite(uIndex);
		const NetPlay::RichPresence& presence = C_UIFriends::Instance().GetFriendPresence(uIndex);
		
		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			if (inv.m_Active)
			{
				const bdSecurityID& secId = inv.m_Id;
				// if we are not already in this session the we can join it
				if (!ns.Party().InSession(secId) && !ns.Race().InSession(secId))
					return true;
			}
			else if (presence.m_JoinInProgressPossible)
			{
				if (!ns.Party().InSession(presence.m_OnlineSessionId) && !ns.Race().InSession(presence.m_OnlineSessionId))
					return true;
			}
		}
	}
	return false; 
}

void C_UiManager::SendInvite( const U32 uIndex )
{
	const SysProfile::BaseProfile& profile = C_UIFriends::Instance().GetFriendProfile(uIndex);
	SysProfile::C_Manager& sysProfileMgr = SysProfile::C_Manager::Instance();
	if (sysProfileMgr.ServiceConnectionState() == SysProfile::C_Manager::eServiceStateConnected)
	{
		if (!sysProfileMgr.SendingGameInvite())
		{
			RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
			if (ns.IsInitialised())
			{
				const S32 pad = C_UIFriends::Instance().GetPadNo();
				if (pad > -1)
				{
					SysProfile::UserIdentifier user = profile;

					bool sent = false;
					NetPlay::SessionSecurity invData;
					if (ns.Race().CanInvite(invData))
					{
						sysProfileMgr.StartSendGameInvite(pad, &user, 1, invData);
						sent = true;
					}
					else if (ns.Party().CanInvite(invData))
					{
						sysProfileMgr.StartSendGameInvite(pad, &user, 1, invData);
						sent = true;
					}

					// Send a timeline message
					if ( sent )
					{
						C_UiTimelineMessage msg( "invite_sent", true, 0.0f );
						gButtons.BroadcastMessageNow( 0xffffffff, &msg );
					}
				}	
			}
		}
	}
}

void C_UiManager::JoinSessionInProgress( const U32 uIndex )
{
	SysProfile::C_Manager& sysProfileMgr = SysProfile::C_Manager::Instance();
	if (sysProfileMgr.ServiceConnectionState() == SysProfile::C_Manager::eServiceStateConnected)
	{
		const SysProfile::BaseProfile& profile = C_UIFriends::Instance().GetFriendProfile(uIndex);
		SysProfile::GameInvite& inv = C_UIFriends::Instance().GetFriendInvite(uIndex);
		const NetPlay::RichPresence& presence = C_UIFriends::Instance().GetFriendPresence(uIndex);

		RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
		if (ns.IsInitialised())
		{
			if (inv.m_Active)
			{
				// removes game invite from the presence data once accepted on this platform..
				const S32 pad = C_UIFriends::Instance().GetPadNo();
				sysProfileMgr.ProcessGameInvite(pad, profile, true);
				ns.AcceptGameInvite(inv);

				// clear invite from enumerated data, incase the enumerator is not updating the presence each frame.
				inv.m_Active = false;
			}
			else
			{
				// create a game invite structure from the presence data.
				SysProfile::GameInvite hackedInv;
				hackedInv.m_Active = true;

				hackedInv.m_Id = presence.m_OnlineSessionId;
				ns.AcceptGameInvite(hackedInv);
			}
		}
	}
}

void C_UiManager::BuildPresenceString( const U32 uIndex, C_WideString64& string )
{
	string.Clear();

	// game defined rich presence is only valid when player is online and playing the same game.
	if (C_UIFriends::Instance().GetFriendState(uIndex) != NetPlay::eFriendOnline)
		return;
	const NetPlay::RichPresence& presence = C_UIFriends::Instance().GetFriendPresence(uIndex);

	switch(presence.m_Index)
	{
	case RacerNet::eRichPresence_Idle:
		{
			C_TextStringID loc( "RP_IDLE" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_SignedIn:
		{
			C_TextStringID loc( "RP_INITIAL_MENUS" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_Stickerbook:
		{
			C_TextStringID loc( "RP_STICKERBOOK" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_Leaderboards:
		{
			C_TextStringID loc( "RP_LEADERBOARDS" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_Help:
		{
			C_TextStringID loc( "RP_HELP" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_Showroom:
		{
			C_TextStringID loc( "RP_SHOWROOM" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_SpMain:
		{
			C_TextStringID loc( "RP_SP" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_Photos:
		{
			C_TextStringID loc( "RP_PHOTOS" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_FriendChallenges:
		{
			C_TextStringID loc( "RP_FRIEND_CHALLENGES" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_SpCareer:
		{
			C_TextStringID loc( "RP_CAREER" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_Sp_Choosing:
		{
			C_TextStringID loc( "RP_SP_CHOOSING" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresencePS3_SpBoss:
		{
			C_TextStringID loc( "RP_SP_BOSS" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresencePS3_SpRace:
		{
			C_TextStringID loc( "RP_SP_RACE" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresencePS3_SpDestruction:
		{
			C_TextStringID loc( "RP_SP_DESTRUCTION" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresencePS3_SpCheckpoint:
		{
			C_TextStringID loc( "RP_SP_CHECKPOINT" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresencePS3_FC_Challenge:
		{
			C_TextStringID loc( "RP_ATTEMPTING_CHALLENGE" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresencePS3_RC_ReChallenge:
		{
			C_TextStringID loc( "RP_ATTEMPTING_RECHALLENGE" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_SplitScreen:
		{
			C_TextStringID loc( "RP_SPLIT_SCREEN" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_LAN:
		{
			C_TextStringID loc( "RP_LAN" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_MpMain:
		{
			C_TextStringID loc( "RP_MP" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_RaceHistory:
		{
			C_TextStringID loc( "RP_RACE_HISTORY" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_OnlineMain:
		{
			C_TextStringID loc( "RP_ONLINE_PLATFORM" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_ModShop:
		{
			C_TextStringID loc( "RP_MODSHOP" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_Challenges:
		{
			C_TextStringID loc( "RP_MP_CHALLENGES" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresence_MpLobby:
		{
			C_TextStringID loc( "RP_LOBBY_PLATFORM" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresencePS3_OnlineRacing:
		{
			C_TextStringID loc( "RP_MP_RACE_PLATFORM" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresencePS3_OnlineMM:
		{
			C_TextStringID loc( "RP_MP_MM_PLATFORM" );
			if (loc.IsValid() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	case RacerNet::eRichPresencePS3_OnlinePrivate:
		{
			C_TextStringID loc( "RP_PRIVATE_GAME_PLATFORM" );
			if (loc.IsValid() && loc.IsLocString() && loc.IsLocString())
				string.Format(L"%s", loc.String());
			break;
		}
	}
}

void C_UiManager::RefreshLocalNetPlayerName()
{
#ifdef PLATFORM_PS3
	// [WillB] #37847 - The player has a online and offline user name on the PS3 so it needs to be reflected in the UI when it changes.
	const S32 sIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
	const SysProfile::FullProfile& padProfile = C_ProfileInterface::Instance().GetPadProfile( sIndex );

	RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();
	if ( ns.IsInitialised() )
		ns.RefreshLocalPlayerName( padProfile.m_Name );
#endif // PLATFORM_PS3
}

bool C_UiManager::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !pMessage )
		return false;

	RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();

	switch ( pMessage->Type() )
	{
		case Game::E_GameflowMessage_StartRaceLoad:
		{
			// reset some stuff
			C_GameDataFinder::Instance().Init();
			C_HudDataFinder::Instance().Init();
			break;
		}

		case E_ProfMsg_SignedIn:
			Lua::Call( g_pUILua->GetState(), "ProfileSignedInOut", "bi", true, ((const C_ProfMsg_SignedIn*)pMessage)->m_Pad );
			break;
		case E_ProfMsg_SignedOut:
			Lua::Call( g_pUILua->GetState(), "ProfileSignedInOut", "bi", false, ((const C_ProfMsg_SignedOut*)pMessage)->m_Pad );
			break;

		case E_ProfMsg_ServiceConnectionLostXLSP:
			Lua::Call( g_pUILua->GetState(), "XLSPConnectionLost" );
			break;

		case E_ProfMsg_ServiceConnectionEstablished:
			RefreshLocalNetPlayerName();
			break;

		case E_ProfMsg_ServiceConnectionLost:
			RefreshLocalNetPlayerName();

			Lua::Call( g_pUILua->GetState(), "ServiceConnectionLost" );
			break;

		case E_ProfMsg_LobbyConnectionLost:
			Lua::Call( g_pUILua->GetState(), "LobbyConnectionLost" );
			break;

		case E_ProfMsg_NetworkCableRemoved:
			Lua::Call( g_pUILua->GetState(), "NetworkCableRemoved" );
			break;

		case E_ProfMsg_GameInvitePending:
			if (ns.IsInitialised())
			{
				const C_ProfMsg_GameInvitePending* pMessageInvite = (const C_ProfMsg_GameInvitePending*)pMessage;
				ns.AcceptGameInvite(pMessageInvite->m_Invite);
			}
			break;

		case E_ProfMsg_MuteInfoChanged:
			if (ns.IsInitialised())
				ns.CommunicatorMuteInformationChanged();
			break;

		case E_UiGameMsg_InviteReceived:
			m_fInviteTimer = m_fInviteShowTime;
			break;

		// If we happen to loose connection to all peers in the lobby due to bad connection, then disconnect player from the game with a happy message
		case E_UiGameMsg_LobbyConnectionLost: 
			C_ProfileInterface::Instance().DisconnectFromLobby();
			break;

		case ActorMsg::E_DynamicObjMsg_StartAnimatedCamera:
		case ActorMsg::E_DynamicObjMsg_AutoSelectAnimatedCamera:
		case ActorMsg::E_DynamicObjMsg_BlendAnimatedCamera:
			// Relay camera messages from UI to actor system
			if ( GetActorList() )
				GetActorList()->GetSubscriberSystem().BroadcastMessage( pMessage );
			break;

		// cut scene messages
		case ActorMsg::E_CutSceneMsg_Start_Cutscene:
		case ActorMsg::E_CutSceneMsg_Start_ScriptedCamera:
		case ActorMsg::E_CutSceneMsg_Start_RestartScriptedCamera:
		case ActorMsg::E_CutSceneMsg_Start_PassedCutscene:
		case ActorMsg::E_CutSceneMsg_Start_PassedScriptedCamera:
		case ActorMsg::E_CutSceneMsg_Start_FailedScriptedCamera:
		case ActorMsg::E_CutSceneMsg_Start_GarageCamera:
		case ActorMsg::E_CutSceneMsg_Start_GarageCameraReverse:
		{
			// forward to actor list
			if ( GetActorList() )
				GetActorList()->GetSubscriberSystem().BroadcastMessage( pMessage );
			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( pMessage );
			break;
		}
	}

	return true;
}

#ifdef PLATFORM_PS3
void C_UiManager::RenderBlackRectangle()
{
	bool newSystemMenuState=C_GameApp::GetInstance()->m_SystemMenuActive;
	bool newSystemOverlayState=SysProfile::C_Manager::Instance().ShowingSystemOverlay();

	if (newSystemMenuState&&!m_systemMenuState)
	{
		m_renderBlackRectangle=true;
	}

	if (newSystemOverlayState&&!m_systemOverlayState)
	{
		m_renderBlackRectangle=true;
	}

	if (!newSystemMenuState&&m_systemMenuState)
	{
		m_renderBlackRectangle=false;
	}

	if (!newSystemOverlayState&&m_systemOverlayState)
	{
		m_renderBlackRectangle=false;
	}

	m_systemMenuState=newSystemMenuState;
	m_systemOverlayState=newSystemOverlayState;

	if (m_renderBlackRectangle )
	{
		gSimple2D.AddSprite( gSimple2D.TopLayer(), Maths::HVec3( 320.0f, 240.0f, 0.0f ), gSimple2D.ScreenWidth() + 2, 482.0f,
			0xF0000000, 0, E_SpriteType_Sprite, C_RenderState::Instance() );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
	}
}
#endif // PLATFORM_PS3

const U32 C_UiInterface::PushMessageLuaParams(lua_State* pState, const Messaging::C_MessageBase *pMessage )
{
	switch( pMessage->Type() )
	{
		case ActorMsg::E_DynamicObjMsg_FinishedAnimatedCamera:
		{
			const ActorMsg::C_FinishedAnimatedCameraMsg* pMsg = Messaging::MessageCast< ActorMsg::C_FinishedAnimatedCameraMsg >( pMessage );

			if( !pMsg )
			{
				LOGERROR(g_Log, "UIManager: Messaging::MessageCast< ActorMsg::C_FinishedAnimatedCameraMsg >( pMessage )");
				return 0;
			}
			
			lua_pushinteger(pState, pMsg->m_Viewport);
			lua_pushboolean(pState, pMsg->m_FutureFeed);
			return 2;
		}

		case E_VehicleLogicMsg_HitByPerk :
		{
			const C_VehicleLogicMsg_HitByPerkMsg* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >( pMessage );

			if( !pMsg )
				return 0;

			// Check to see if local firing
			if( GetLevelState().mCars[ pMsg->mFiringVehicleIndex ].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER )
			{
				// Get avatar index of 'other' player
				int avatarID = 0;
				RaceBook::C_RaceBookProfile* pOther = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)GetLevelState().mCars[ pMsg->mVehicleIndex ].mCharacterID);
				if( pOther )
					avatarID = pOther->mAvatarID;

				lua_pushboolean( pState, true );
				lua_pushinteger( pState, pMsg->mFiringVehicleIndex );
				lua_pushinteger( pState, avatarID );
				lua_pushinteger( pState, pMsg->mPickupType );
				return 4;
			}
			// Else see if local has been hit
			if( GetLevelState().mCars[ pMsg->mVehicleIndex ].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER )
			{
				// Get avatar index of 'other' player
				int avatarID = 0;
				RaceBook::C_RaceBookProfile* pOther = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)GetLevelState().mCars[ pMsg->mFiringVehicleIndex ].mCharacterID);
				if( pOther )
					avatarID = pOther->mAvatarID;

				lua_pushboolean( pState, false );
				lua_pushinteger( pState, pMsg->mVehicleIndex );
				lua_pushinteger( pState, avatarID );
				lua_pushinteger( pState, pMsg->mPickupType );
				return 4;
			}
			break;
		}
		
		case E_UiGameMsg_Dialogue:
		{
			const C_UiGameMsg_Dialogue* pMsg = Messaging::MessageCast<C_UiGameMsg_Dialogue>( pMessage );
			
			RaceBookCharacter* pCharacter = C_RaceBookData::Instance().mCharacters[ (GAMEDATA_ID)pMsg->m_RaceBookCharacterId ];

			if ( !pCharacter )
				return 0;

			lua_pushinteger( pState, (U32)pCharacter->avatarID );
			lua_pushinteger( pState, pMsg->m_LocId.Value() );
			return 2;
		}

		case Game::E_GameLogicMessage_LapIncremented:
		{
			const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>( pMessage );
			if( !pMsg )
				return 0;

			lua_pushinteger( pState, GetLocalPlayerIndex( pMsg->mVehicleIndex ) );
			return 1;
		}

		case Game::E_VehicleMessage_Overtake:
		{
			const Game::Actors::C_VehicleOvertakeMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleOvertakeMsg>( pMessage );
			if( !pMsg )
				return 0;

			lua_pushinteger( pState, GetLocalPlayerIndex( pMsg->mOvertakerVehicleIndex ) );
			return 1;
		}

		case Game::E_GameflowMessage_VehicleFinished:
		{
			S32 sViewportIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
			lua_pushnumber( pState, sViewportIndex );
			return 1;
		}

		case Game::E_VehicleMessage_Eliminated:
		{
			S32 sViewportIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
			lua_pushnumber( pState, sViewportIndex );
			return 1;
		}

		case Game::E_GameflowMessage_Wrecked:
		case Game::E_GameflowMessage_Respawn:
		{
			S32 sViewportIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
			lua_pushnumber( pState, sViewportIndex );
			return 1;
		}

		case E_UiGameMsg_DisplayECMWarning:
		{
			S32 sViewportIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
			lua_pushnumber( pState, sViewportIndex );
			return 1;
		}
		case E_UiGameMsg_RemoveECMWarning:
		{
			S32 sViewportIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
			lua_pushnumber( pState, sViewportIndex );
			return 1;
		}
		case E_UiGameMsg_NewRivalSelected:
		{
			S32 sViewportIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
			lua_pushnumber( pState, sViewportIndex );
			return 1;
		}
		case E_UiGameMsg_MpRankedUp:
		{
			const C_UiMessageData* pMessageD = Messaging::MessageCast< C_UiMessageData >( pMessage );

			lua_pushnumber( pState, pMessageD->m_Args[ 0 ] );
			lua_pushnumber( pState, pMessageD->m_Args[ 1 ] );
			lua_pushnumber( pState, pMessageD->m_Args[ 2 ] );
			lua_pushnumber( pState, pMessageD->m_Args[ 3 ] );

			return 4;
		}

		case E_UiGameMsg_MpChallengeComplete:
		{
			const C_UiMessageData* pMessageD = Messaging::MessageCast< C_UiMessageData >( pMessage );

			lua_pushnumber( pState, pMessageD->m_Args[ 0 ] );
			lua_pushnumber( pState, pMessageD->m_Args[ 1 ] );

			return 2;
		}

		case E_UiGameMsg_SpDestructionPointsAdded:
		{
			bool bIsBonus = ( Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ] != 0.0f );
			lua_pushboolean( pState, bIsBonus );
			return 1;
		}

		default:
			break;
	}

	return 0;
}

LUALIB_API int luaopen_debug2(lua_State *pState);
LUALIB_API int luaopen_buttons(lua_State *pState);
LUALIB_API int luaopen_screen(lua_State *pState);
LUALIB_API int luaopen_controller(lua_State *pState);
LUALIB_API int luaopen_system(lua_State *pState);
LUALIB_API int luaopen_hardware(lua_State *pState);
LUALIB_API int luaopen_profile( lua_State* pState );
LUALIB_API int luaopen_shape( lua_State* pState );
LUALIB_API int luaopen_multiplayer( lua_State* pState );

LUALIB_API int luaopen_sp(lua_State *pState);

// Lua libraries that we are going to load.
static const luaL_reg lualibs[] = 
{
        { "base",			luaopen_base },
		{ "table",			luaopen_table },
		{ "string",			luaopen_string },
		{ "math",			luaopen_math },
		{ "debug",			luaopen_debug },
		// Bizarre debug stuff (like memory tracker)
		{ "debug2",			luaopen_debug2 },
		// UI
		{ "buttons",		luaopen_buttons },
		{ "screen",			luaopen_screen },
		{ "pad",			luaopen_controller },
		{ "hardware",		luaopen_hardware },
		{ "system",			luaopen_system },
		{ "profile",		luaopen_profile },
		{ "shape",			luaopen_shape },
		{ "multiplayer",	luaopen_multiplayer },
		// GameLogic
		{ "amax",			Game::luaopen_amax },
		{ "gamedata",		Game::luaopen_gamedata },
		{ "netservices",	Game::luaopen_net_services },
		{ "netparty",		Game::luaopen_net_party },
		{ "netrace",		Game::luaopen_net_race },
		{ "game_profile",	Game::luaopen_game_profile },
		{ "lsp",			Game::luaopen_lsp },

		{ "frienddemand",	Game::luaopen_friend_demand },

		// Single player
		{ "sp",				luaopen_sp },

		// End of list
		{ NULL,				NULL } 
};

const bool C_UiInterface::IsWideScreen()
{
	return Display::Device()->GetWideScreen();
}

void C_UiInterface::PlaySound( const S32 sIndex, const F32 fValue1, const F32 fValue2 )
{
	if( sIndex == -1 )
		return;

	C_UiSoundEffectMessage msg( sIndex, fValue1, fValue2 );
	Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &msg );

#ifndef CFG_GOLD
	if ( g_UiDebugLogAudioTriggers )
		LOGINFO( UI::g_Log, "C_UiInterface::PlaySound - %s - %d", UiXt::GetEnumNameFromType( "UISoundEffect", sIndex ), sIndex );
#endif
}


const S32 C_UiInterface::GetSoundIndex( const E_BaseSoundEffect eEffect )
{
	switch( eEffect )
	{
		case E_BaseSoundEffect_Move:		return E_UiSoundEffect_Move;			break;
		case E_BaseSoundEffect_Forwards:	return E_UiSoundEffect_Forwards;		break;
		case E_BaseSoundEffect_Backwards:	return E_UiSoundEffect_Back;			break;
		case E_BaseSoundEffect_Error:		return E_UiSoundEffect_Error;			break;
		case E_BaseSoundEffect_Toggle:		return E_UiSoundEffect_Toggle;			break;
		case E_BaseSoundEffect_Up:			return E_UiSoundEffect_UpActionIn;		break;
		case E_BaseSoundEffect_Down:		return E_UiSoundEffect_DownActionIn;	break;
		case E_BaseSoundEffect_Left:		return E_UiSoundEffect_LeftActionIn;	break;
		case E_BaseSoundEffect_Right:		return E_UiSoundEffect_RightActionIn;	break;
		default:							return 0;								break;
	}

	return 0;
}


// Open the LUA libraries
void C_UiInterface::RegisterLibs( lua_State *pState )
{
	const luaL_reg *lib;

	for (lib = lualibs; lib->func != NULL; lib++) 
	{
		lib->func(pState);           // Open the library 
		
		// Flush the stack, by setting the top to 0, in order to
		// ignore any result given by the library load function.
		lua_settop(pState, 0);
    }
}

const bool C_UiInterface::ExternalRender( const C_Sprite* pSprite )
{
	switch( pSprite->m_Type )
	{
	case E_GameSpriteType_LightPaint:
		pSprite->m_Callback( pSprite );
		return true;
	}

	return false;
}

Display::ResourceBundler::I_ResourceBundle* C_UiInterface::CreateResourceBundle()
{
	return new Display::ResourceBundler::C_ResourceBundleSingle();
}

void C_UiInterface::PollPlatformCallbacks()
{
	C_GameApp::GetInstance()->PollPlatformCallbacks();
}

} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
