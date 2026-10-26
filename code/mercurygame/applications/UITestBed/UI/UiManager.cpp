#include "Stdafx.h"

#include "UiManager.h"

#include <Display/Bundler/Bundler.h>
#include <system/debug/BuildStamp.h>

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
#include <ui/Game/Gadgets/UIFriendsList.h>
#include <ui/Game/Gadgets/UIKeyboard.h>
#include <ui/Graphics/Font/Font.h>
#include <ui/Graphics/Font/FontStyle.h>
#include <ui/Graphics/Font/UIPolygonFont.h>
#include <ui/Graphics/Movie.h>
#include <ui/Graphics/UITextureManager.h>
#include <ui/Graphics/Localisation/Loc.h>
#include <ui/Graphics/SimpleTimer.h>
#include <ui/Graphics/UIParticle.h>
#include <ui/Illustrator/UIAdobeSceneManager.h>

// TEST : Amax livery editor stuff
#include "ui/Game/Gadgets/UIVinylScene.h"
#include "ui/Game/Gadgets/UIVinylPreview.h"
#include "ui/Game/Gadgets/UIVinylStripBar.h"
#include "ui/Game/Gadgets/UIColourPicker.h"
#include "ui/Game/Gadgets/UITestList.h"
#include "ui/Game/VinylEditorDataFinder.h"
#include "ui/Game/Gadgets/VinylManager.h"

// datafinders
#include "ui/Game/GameDataFinder.h"
#include <ui/Game/Profiles/ProfileDataFinder.h>
#include <ui/Game/Marketplace/MarketplaceDataFinder.h>

// profiles
#include <ui/Game/Profiles/ProfileInterface.h>

#ifdef PLATFORM_XENON
#include <xaudio2.h>
namespace Audio
{
	namespace Core
	{
		IXAudio2* g_pXAudio2;
	}
}
#endif

#define _RENDER_TARGET_

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

#ifndef CFG_GOLD
Debug::Bool g_debugLUAAllowStepping("UI.Lua.AllowStepping", false);
extern Debug::Bool g_debugUseLUAFromHDD;
extern Debug::Bool g_UICameraDebugActive2D;
extern Debug::Bool g_UICameraDebugActive3D;
#endif

#ifdef _DEBUG_POLYFONT_
Debug::Bool		g_UiPolygonFontDebugReset( "UI.PolygonFont.DebugReset", false );
Debug::Bool		g_UiPolygonFontDebugResetEveryFrame( "UI.PolygonFont.DebugResetEveryFrame", false );
Debug::String	g_UiPolygonFontFont( "UI.PolygonFont.Font", "Arial.pf" );
#endif // _DEBUG_POLYFONT_

// ***

void C_GameFileSystem_UpdateSystemMessages(){}

// ***
#ifdef WIN64
#define UIBIN_START			__binary_UITestBed_LUA_bin_start
#define UIBIN_END			__binary_UITestBed_LUA_bin_end
#else
#define UIBIN_START			_binary_UITestBed_LUA_bin_start
#define UIBIN_END			_binary_UITestBed_LUA_bin_end
#endif

extern "C" char UIBIN_START[], UIBIN_END[];

XT_LIBRARY( GameUI )
	XT_REGISTER_TYPE( UI::UITestList )
	// Vinyl
	XT_REGISTER_TYPE( UI::UIColourPicker )
	XT_REGISTER_TYPE( UI::UIVinylScene )
	XT_REGISTER_TYPE( UI::UIVinylPreview )
	XT_REGISTER_TYPE( UI::UIVinylStripBar )
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
// ***

namespace UI 
{

C_UiInterface* g_pManager;

I_UiInterface* UiInterface()
{
	return g_pManager;
}

// ---
C_UiManager::C_UiManager( const S32 sInitParam )
{
	MEMORY_TRACKER_PUSH("ui");

	const char *userName = Debug::GetBuildUser();

	const bool userDGill( 0 == stricmp( userName, "dgill" ) );
	const bool userWBrown( 0 == stricmp( userName, "wbrown" ) );
	const bool userPReid( 0 == stricmp( userName, "pereid" ) );

	// Ouch!
	g_pManager = &m_UiInterface;

	// Lua sys
	g_pUILua = new C_UILua();
	assert( g_pUILua );

	// reads in UIEnums.lua too
#if defined (PLATFORM_XENON)
	void *ptrToLuaScripts;
	unsigned long sectionSize;
	if (!XGetModuleSection(GetModuleHandle(NULL), "UIScript", &ptrToLuaScripts, &sectionSize))
	{
		LOGFATAL(g_Log, "LUA not embedded in EXE.");
	}
	g_pUILua->Init( true, "DontShip\\Ui\\Scripts\\", ptrToLuaScripts );
#else
	if( UIBIN_START == NULL )
	{
		LOGFATAL(g_Log, "LUA not embedded in EXE.");
	}
	g_pUILua->Init( true, "DontShip\\Ui\\Scripts\\", UIBIN_START );
#endif
	C_ScreenControlManager::Instance().Add();		// Main
	C_ScreenControlManager::Instance().Add();		// Popups

	// *** THIS SHOULDN'T BE HERE IN A PROPER GAME
	SysProfile::C_Manager::Instance().SetPrimaryPadNo( 0 );
	SysProfile::C_Manager::Instance().GetProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo(), C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() ) );
	// ***

	S32 sFriendsContext = C_ScreenControlManager::Instance().Count();
	C_UIFriends::Instance().Initialise( sFriendsContext, -1,/*(S32)E_UiMsg_CtrlButton_RightShoulder*/ "FriendsList.lua" );
	C_ScreenControlManager::Instance().Add();	// Friends List

	// intialise the screen control
	C_ScreenControl& f = C_ScreenControlManager::Instance().Get( sFriendsContext );
	f.SetLuaScreenDir( "Screens\\" );
	f.SetInitFunctionName( "FL_Init" );
	f.SetStartLoopName( "FL_StartLoop" );
	f.SetFrameUpdateName( "FL_FrameUpdate" );
	f.SetMessageUpdateName( "FL_MessageUpdate" );
	f.SetEndLoopName( "FL_EndLoop" );
	f.SetExitFunctionName( "FL_End" );

	S32 sKeyboardContext = C_ScreenControlManager::Instance().Count();
	C_UIKeyboard::Instance().Initialise( sKeyboardContext, "Keyboard.lua" );
	C_ScreenControlManager::Instance().Add();	// keyboard

	C_ScreenControl& k = C_ScreenControlManager::Instance().Get( sKeyboardContext );
	k.SetLuaScreenDir( "Screens\\" );
	k.SetInitFunctionName( "KB_Init" );
	k.SetStartLoopName( "KB_StartLoop" );
	k.SetFrameUpdateName( "KB_FrameUpdate" );
	k.SetMessageUpdateName( "KB_MessageUpdate" );
	k.SetEndLoopName( "KB_EndLoop" );
	k.SetExitFunctionName( "KB_End" );

	C_ScreenControlManager::Instance().Context(0);	// Default context is 'main'

	lua_State* pState = g_pUILua->GetState();

//	gUITextureManager.LoadCommonTextures( "" );
	gUITextureManager.LuaState( pState );
	gUITextureManager.AddResourceBundle( "Ui\\Data\\ui_common" );
	gUITextureManager.AddResourceBundle( "Ui\\Data\\ui_font_eur" );
	gUITextureManager.AddResourceBundle( "UI\\Data\\ui_game" );
	gUITextureManager.AddResourceBundle( "UI\\Data\\ui_frontend" );
	gUITextureManager.Update();
	
	m_SwitchBundle = C_TextureManager::m_InvalidResourceIndex;

	Display::Device()->AcquireThreadOwnership();

	gSimple2D.Init( pState, 5000, 200, 5000, 4 );
	gSimple2D.SetClipSize2D( Display::C_ViewSetup::GetDeviceCoords().Max().X(), Display::C_ViewSetup::GetDeviceCoords().Max().Y() );
	gSimple2D.SetZEnable( false );

	Display::Device()->ReleaseThreadOwnership();

	// alternate textures
	// movie
	gSimple2D.SetAlternateTexture( new C_MovieAlternateTexture( 0 ) );
	//render target
#ifdef _RENDER_TARGET_
	m_pRenderTarget0 = new C_Simple2D::C_RenderTargetAlternateTexture( 0, 512, 512 );
	gSimple2D.SetAlternateTexture( m_pRenderTarget0 );
#endif

#ifdef _DEBUG_POLYFONT_
	gUIPolygonFont.Init();
	Util::C_String font = Util::C_String( "DontShip\\Ui\\Data\\" ) + g_UiPolygonFontFont;
	gUIPolygonFont.BuildFont( font.Chars(), true );
	gUIPolygonFont.BuildFont( font.Chars() );
#endif

	/// *** localiser MUST be initialised before the font library now - default height percentage depends on language *** ///

	// loc table MUST be loaded before intialising the localiser
	UI::C_TextStringID::LocTable().Load( "gamedata\\xt\\UITestBed-en.loc", NULL, 0 );
	UI::C_TextStringID::LocTable().Load( "gamedata\\xt\\InfoLines-en.loc", NULL, 1 );
	UI::C_TextStringID::LocTable().Load( "gamedata\\xt\\Profile-en.loc", NULL, 2 );

	C_TextStringID::LocTable().CreateLuaEnums( pState );

	C_UILocaliser::Instance().DetermineLanguageAndRegion( 0 );
	C_UILocaliser::Instance().Init();

	gFont.Initialise( "Ui\\Data\\ui_font_eur.dat" );

#ifndef _DEBUG_POLYFONT_
	#ifdef PLATFORM_PC
	gFont.CreatePolygonFont( "dontship\\ui\\data\\Verdana.pf", true, false, false, false, false, false );
	#else

	if( userDGill )
		gFont.CreatePolygonFont( "dontship\\Ui\\Data\\Verdana.pf", true, true, false, false, false, false );
	
	#endif
#endif
	
	C_StringStore::Instance().Init();

	gUIParticleMan.Init( 1 );

	// Must be done before buttons are registered
	gButtons.InitCreator( E_ButtonType_Max, true );

	// ***
	// Register button types here
	// ***
	gButtons.Register<UITestList>();

	gButtons.Register<UIColourPicker>();
	gButtons.Register<UIVinylScene>();
	gButtons.Register<UIVinylPreview>();
	gButtons.Register<UIVinylStripBar>();

	gButtons.CreateLuaEnums( pState );

	CreateGameLuaEnums( pState );

//	gUITextureManager.CreateLuaEnums( pState );
	gUITextureManager.Update();

	gSimple2D.BuildLuaEnumsForAlternateTextures( pState );

	// load in sorcery definitions
	g_UIXt.LoadDefinitions( "gamedata\\xt\\UI.bin" );

	// Load any fonts defined in sorcery
	UiXt::C_FontStyleList::Instance().LoadPolygonFonts();

	// font enums
	gFont.CreateLuaEnums( pState );
	C_FontStyleCreator::Instance().Initialise();
	C_FontStyleCreator::Instance().CreateLuaEnums( pState );

	C_GadgetBase::C_Rect::ResetCulling();

	// Load all the test shapes in the directory.
	C_AdobeSceneManager::Instance().LoadScenes( "Ui\\Data\\Ipf\\" );

	// Run setup when all enums have been added
#ifndef CFG_GOLD
	g_pUILua->LoadCallSetupScript( sInitParam, g_debugUseLUAFromHDD );
#else
	g_pUILua->LoadCallSetupScript( sInitParam, false );
#endif

	// Add the Vinyl editor modes
	{
		lua_getglobal(pState, LUA_ENUM_TABLE_NAME);

		if( lua_type( pState, -1 ) == LUA_TTABLE )
		{
			lua_pushstring( pState, "VinylEditMode" );
			lua_newtable( pState );
				Lua::AddDataMember( pState, "None",			(S32)C_VinylManager::E_EditMode_None );
				Lua::AddDataMember( pState, "Translate",	(S32)C_VinylManager::E_EditMode_Translate );
				Lua::AddDataMember( pState, "Scale",		(S32)C_VinylManager::E_EditMode_Scale );
				Lua::AddDataMember( pState, "Rotate",		(S32)C_VinylManager::E_EditMode_Rotate );
				Lua::AddDataMember( pState, "Shear",		(S32)C_VinylManager::E_EditMode_Shear );
				Lua::AddDataMember( pState, "GradientRot",	(S32)C_VinylManager::E_EditMode_GradientRotate );
				Lua::AddDataMember( pState, "GradientTrans",(S32)C_VinylManager::E_EditMode_GradientTranslate );
				Lua::AddDataMember( pState, "GradientScale",(S32)C_VinylManager::E_EditMode_GradientScale );
			lua_rawset( pState, -3 );

			lua_pushstring( pState, "VinylSceneType" );
			lua_newtable( pState );
				Lua::AddDataMember( pState, "Avatar",		(S32)C_VinylManager::E_SceneType_Avatar );
				Lua::AddDataMember( pState, "Group",		(S32)C_VinylManager::E_SceneType_Group );
				Lua::AddDataMember( pState, "Livery",		(S32)C_VinylManager::E_SceneType_Livery );
			lua_rawset( pState, -3 );

			lua_pushstring( pState, "VinylList" );
			lua_newtable( pState );
				Lua::AddDataMember( pState, "Clipboard",	(S32)C_VinylManager::E_List_Clipboard );
				Lua::AddDataMember( pState, "ScratchPad",	(S32)C_VinylManager::E_List_ScratchPad );
				Lua::AddDataMember( pState, "Avatar",		(S32)C_VinylManager::E_List_Avatar );
				Lua::AddDataMember( pState, "Group",		(S32)C_VinylManager::E_List_Group );
				Lua::AddDataMember( pState, "LiveryTop",	(S32)C_VinylManager::E_List_LiveryTop );
				Lua::AddDataMember( pState, "LiveryLeft",	(S32)C_VinylManager::E_List_LiveryLeft );
				Lua::AddDataMember( pState, "LiveryRight",	(S32)C_VinylManager::E_List_LiveryRight );
				Lua::AddDataMember( pState, "LiveryFront",	(S32)C_VinylManager::E_List_LiveryFront );
				Lua::AddDataMember( pState, "LiveryBack",	(S32)C_VinylManager::E_List_LiveryBack );
			lua_rawset( pState, -3 );
		}
	}

	gUIScreenControl.SetLuaScreenDir("Screens\\");
	gUIScreenControl.SetInitFunctionName( "Init" );
	gUIScreenControl.SetPostInitFunctionName( "PostInit" );
	gUIScreenControl.SetStartLoopName( "StartLoop" );
	gUIScreenControl.SetFrameUpdateName( "FrameUpdate" );
	gUIScreenControl.SetMessageUpdateName( "MessageUpdate" );
	gUIScreenControl.SetEndLoopName( "EndLoop" );
	gUIScreenControl.SetExitFunctionName( "End" );
	gUIScreenControl.Init();

	gUIPopupControl.SetLuaScreenDir("Popups\\");
	gUIPopupControl.SetInitFunctionName( "PopupInit" );
	gUIPopupControl.SetPostInitFunctionName( "PopupPostInit" );
	gUIPopupControl.SetStartLoopName( "PopupStartLoop" );
	gUIPopupControl.SetFrameUpdateName( "PopupFrameUpdate" );
	gUIPopupControl.SetMessageUpdateName( "PopupMessageUpdate" );
	gUIPopupControl.SetEndLoopName( "PopupEndLoop" );
	gUIPopupControl.SetExitFunctionName( "PopupEnd" );

	// _after_ loading Lua scripts
	CreateGameGlobals( pState );

	gUIScreenControl.SetDefaultScreenName( "Blank.lua" );
/*
	if( userDGill )
	{
	//	C_AdobeSceneManager::Instance().LoadScene( "dontship\\Ui\\Data\\archer.ipf" );

	//	C_AdobeSceneManager::Instance().LoadScene( "dontship\\Ui\\Data\\Star.ipf" );
	//	C_AdobeSceneManager::Instance().LoadScene( "dontship\\Ui\\Data\\box.ipf" );

	//	for(U32 i = 0; i < C_UIAdobeSceneManager::Instance().GetNumberOfGradients(); i++)
	//		gSimple2D.GetGradient().Build( 1+i, *C_AdobeSceneManager::Instance().GetGradientPtr( i ) );
	}
	else
	if( userWBrown )
	{
		// Add extra shape that you want to load here.
		//C_AdobeSceneManager::Instance().LoadScenes( "Ui\\Ipf\\" );
		//C_AdobeSceneManager::Instance().LoadScene( "Ui\\Data\\Ipf\\icon_sheet.ipf" );
	}
	else
	if( userPReid )
	{
		//C_AdobeSceneManager::Instance().LoadScene( "dontship\\Ui\\Data\\Star.ipf" );
		//C_AdobeSceneManager::Instance().LoadScene( "dontship\\Ui\\Data\\Cross.ipf" );
		//C_AdobeSceneManager::Instance().LoadScene( "dontship\\Ui\\Data\\Spiral.ipf" );
	}
*/
	//C_AdobeSceneManager::Instance().BuildScenes();

	// init font styles - _after_ we load Lua common files in
	C_FontStyle::Instance().InitDefaultStyle( pState );

	// check if we've now got a font sequences LUA table loaded
	gFont.InitialiseSequences( pState );

	// Push loaded shapes into the lua enum table.
	C_AdobeSceneManager::Instance().CreateLuaEnums( pState );

	// add in the datafinders
	C_UIDataFinder::Instance().AddGameDataFinder( &C_GameDataFinder::Instance() );
	C_UIDataFinder::Instance().AddGameDataFinder( &C_ProfileDataFinder::Instance() );
	C_UIDataFinder::Instance().AddGameDataFinder( &C_MarketplaceDataFinder::Instance() );
	C_UIDataFinder::Instance().AddGameDataFinder( &C_VinylEditorDataFinder::Instance() );

	// initialise UI camera
	C_UICamera::Instance().Init();

	MEMORY_TRACKER_POP("ui");

	C_UIColourPicker::Instance().BuildColours( 11, 10, 5 );
}

void C_UiManager::CreateTextureBundle( const bool bIngameBundle )
{
	if( bIngameBundle )
	{
		gUITextureManager.RemoveResourceBundle( m_SwitchBundle );
		m_SwitchBundle = gUITextureManager.AddResourceBundle( "UI\\Data\\ui_game" );
	}
	else
	{
		gUITextureManager.RemoveResourceBundle( m_SwitchBundle );
		m_SwitchBundle = gUITextureManager.AddResourceBundle( "UI\\Data\\ui_frontend" );
	}

//	gUITextureManager.Create( bIngameBundle, "" ); 
//	gUITextureManager.DeferCreateLuaEnums();
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
	}
	// gUITextureManager.Delete( bDeleteAll );
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

	// UIEnums go here ...
}

C_UiManager::~C_UiManager()
{
	g_pUILua->End();
//	gUITextureManager.Delete( true ); // Delete all the textures bundles
	gUITextureManager.DestroyAll();

	C_TextStringID::LocTable().DestroyAll();
	gFont.Destroy();
	gButtons.ExitCreator();
	gSimple2D.Exit();
	g_UIXt.Cleanup();

	SAFE_DELETE_PTR( g_pUILua );
	SAFE_DELETE_PTR( m_pRenderTarget0 );

	for(U32 i = 0; i < MAX_MOVIE_SLOTS; i ++)
		gMovie[i].Destroy();

	gUIParticleMan.Exit();

	C_StringStore::Instance().Exit();

	// Clean up loaded shapes.
	C_AdobeSceneManager::Instance().FlushData();

	C_UIDataFinder::Instance().Cleanup();

	C_FontStyle::Instance().Cleanup();

	g_pManager = NULL;
}

void C_UiManager::ProcessController()
{
	const U32 delay = 25;
	const U32 rate = 5;

#ifndef CFG_GOLD
	{
		// *** DEBUG Button Presses *** //
		Input::C_GameInput* gameInputPtr = Input::GetGameInputManager()->GetGameInput( 0 );

		// reload screen - D+X
		if ( g_debugUseLUAFromHDD )
		{
			if ( gameInputPtr && gameInputPtr->GetButtonPressed( E_UiButton_Pad_Down ) )
			{
				if ( gameInputPtr->GetButtonPressedDebounced( E_UiButton_Option2 ) )
				{
					// PR - I'm calling this to set something before we reload
					Lua::Call( g_pUILua->GetState(), "DebugScreenReload" );

					gUIPopupControl.Exit();
					gUIScreenControl.SetNextScreen( gUIScreenControl.GetCurrentScreen() );
					return;
				}
			}
		}

		// reload loc - D+Y
		if ( g_debugUseLUAFromHDD )
		{
			if ( gameInputPtr && gameInputPtr->GetButtonPressed( E_UiButton_Pad_Down ) )
			{
				if ( gameInputPtr->GetButtonPressedDebounced( E_UiButton_Option1 ) )
				{
					C_TextStringID::LocTable().Reload();
					return;
				}
				else
					C_TextStringID::LocTable().TableValid( true );
			}
		}

		Input::C_GameInput* pInput = Input::GetGameInputManager()->GetGameInput( 0 );

		// debug 2D camera movement
		if ( pInput && g_UICameraDebugActive2D )
		{
			C_DebugCamUpdate2D i;
			i.lx = pInput->GetAxisSignedValue( E_UiAxis_LeftStick_LR );
			i.ly = pInput->GetAxisSignedValue( E_UiAxis_LeftStick_UD );
			i.lt = pInput->GetButtonValue( E_UiButton_LeftTrigger );
			i.rt = pInput->GetButtonValue( E_UiButton_RightTrigger );
			C_UICamera::Instance().DebugUpdate2D( i );
		}

		// debug 3D camera movement
		if ( pInput && g_UICameraDebugActive3D )
		{
			C_DebugCamUpdate3D i;
			i.lx = pInput->GetAxisSignedValue( E_UiAxis_LeftStick_LR );
			i.ly = pInput->GetAxisSignedValue( E_UiAxis_LeftStick_UD );
			i.rx = pInput->GetAxisSignedValue( E_UiAxis_RightStick_LR );
			i.ry = pInput->GetAxisSignedValue( E_UiAxis_RightStick_UD );
			i.lt = pInput->GetButtonValue( E_UiButton_LeftTrigger );
			i.rt = pInput->GetButtonValue( E_UiButton_RightTrigger );
			i.lb = pInput->GetButtonValue( E_UiButton_LeftShoulder );
			i.rb = pInput->GetButtonValue( E_UiButton_RightShoulder );
			C_UICamera::Instance().DebugUpdate3D( i );
		}
	}
#endif

	for(U32 pad = 0; pad < 4; pad++)
	{
#if defined( PLATFORM_PC )
		#pragma message("TODO: handle >1 pad on PC")
		if( pad > 0 )
			continue;
#endif
		// disable certain pads?
		if ( !C_ProfileInterface::Instance().AllowPadInput( pad ) )
			continue;
		
		Input::C_GameInput* gameInputPtr = Input::GetGameInputManager()->GetGameInput( pad );

		const S8 player_index = (S8)pad;
		const S32 sMask = C_ScreenControlManager::Instance().MessageMask();
		
		// check for unplugging of the controller
#if defined( PLATFORM_XENON )
		XINPUT_STATE inputState;
		if ( XInputGetState( (DWORD)player_index, &inputState ) == ERROR_DEVICE_NOT_CONNECTED )
#elif defined( PLATFORM_PS3_TODO )
		CellPadCapabilityInfo info;
		if ( cellPadGetCapabilityInfo( (S32)player_index, &info ) == CELL_PAD_ERROR_NO_DEVICE )
#else
		if ( 0 )
#endif
		{
			// only add in when we _know_ it'll be caught (or at least processed)
			if ( gUIScreenControl.GetScreenTime() == SCREEN_TIME_NORMAL )
				_UI_MESSAGE1_MASK( E_UiMsg_ControllerDisconnected, player_index, sMask );

			continue;
		}

		// only add messages when we _know_ they'll be processed
		if( gameInputPtr && gUIScreenControl.GetScreenTime() == SCREEN_TIME_NORMAL )
		{
			// up, down, left and right
			if ( gameInputPtr->GetButtonAutoRepeat( E_UiButton_PadOrAxis_Up, delay, rate ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Up, player_index, true, 1.0, 0.0, sMask );
			
			if ( gameInputPtr->GetButtonAutoRepeat( E_UiButton_PadOrAxis_Down, delay, rate ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Down, player_index, true, 1.0, 0.0, sMask );

			if ( gameInputPtr->GetButtonAutoRepeat( E_UiButton_PadOrAxis_Left, delay, rate ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Left, player_index, true, 1.0, 0.0, sMask );

			if ( gameInputPtr->GetButtonAutoRepeat( E_UiButton_PadOrAxis_Right, delay, rate ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Right, player_index, true, 1.0, 0.0, sMask );

			// left axis
			{
				F32 axis0 = gameInputPtr->GetAxisSignedValue( E_UiAxis_LeftStick_LR );
				F32 axis1 = gameInputPtr->GetAxisSignedValue( E_UiAxis_LeftStick_UD );

				if ( axis0 || axis1 )
				{
					_UI_MESSAGE_PAD( E_UiMsg_CtrlLeftStick_X, player_index, false, axis0, 0.0, sMask );
					_UI_MESSAGE_PAD( E_UiMsg_CtrlLeftStick_Y, player_index, false, axis1, 0.0, sMask );
				}
			}

			// right axis
			{
				F32 axis0 = gameInputPtr->GetAxisSignedValue( E_UiAxis_RightStick_LR );
				F32 axis1 = gameInputPtr->GetAxisSignedValue( E_UiAxis_RightStick_UD );	

				if ( axis0 || axis1 )
				{
					_UI_MESSAGE_PAD( E_UiMsg_CtrlRightStick_X, player_index, false, axis0, 0.0, sMask );
					_UI_MESSAGE_PAD( E_UiMsg_CtrlRightStick_Y, player_index, false, axis1, 0.0, sMask );
				}
			}

			// shoulders
			if ( gameInputPtr->GetButtonAutoRepeat( E_UiButton_LeftShoulder, delay, rate ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_LeftShoulder, player_index, true, 1.0, 0.0, sMask );

			if ( gameInputPtr->GetButtonAutoRepeat( E_UiButton_RightShoulder, delay, rate ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_RightShoulder, player_index, true, 1.0, 0.0, sMask );

			// triggers
			if ( gameInputPtr->GetButtonAutoRepeat( E_UiButton_LeftTrigger, delay, rate ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_LeftTrigger, player_index, true, 1.0, 0.0, sMask );

			if ( gameInputPtr->GetButtonAutoRepeat( E_UiButton_RightTrigger, delay, rate ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_RightTrigger, player_index, true, 1.0, 0.0, sMask );

			// thumbs
			if ( gameInputPtr->GetButtonPressedDebounced( E_UiButton_LeftThumb ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_LeftThumb, player_index, true, 1.0, 0.0, sMask );

			if ( gameInputPtr->GetButtonPressedDebounced( E_UiButton_RightThumb ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_RightThumb, player_index, true, 1.0, 0.0, sMask );
			
			// A, B, X and Y
			if ( gameInputPtr->GetButtonPressedDebounced( E_UiButton_Confirm ) )
			{
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_A, player_index, true, 1.0, 0.0, sMask );
				_UI_MESSAGE_PAD( E_UiMsg_MenuNext, player_index, true, 1.0, 0.0, sMask );
			}
			else
			if ( gameInputPtr->GetButtonPressedDebounced( E_UiButton_Cancel ) )
			{
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_B, player_index, true, 1.0, 0.0, sMask );
				_UI_MESSAGE_PAD( E_UiMsg_MenuBack, player_index, true, 1.0, 0.0, sMask );
			}
			else
			if ( gameInputPtr->GetButtonPressedDebounced( E_UiButton_Option1 ) )
			{
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Y, player_index, true, 1.0, 0.0, sMask );
			}
			else
			if ( gameInputPtr->GetButtonPressedDebounced( E_UiButton_Option2 ) )
			{
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_X, player_index, true, 1.0, 0.0, sMask );
			}

			// start and back
			if ( gameInputPtr->GetButtonPressedDebounced( E_UiButton_Start ) && !gameInputPtr->GetButtonPressed( E_UiButton_Back ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Start, player_index, true, 1.0, 0.0, sMask );

			if ( gameInputPtr->GetButtonPressedDebounced( E_UiButton_Back ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Back, player_index, true, 1.0, 0.0, sMask );

			if ( gameInputPtr->GetButtonPressed( E_UiButton_Back ) )
				_UI_MESSAGE_PAD( E_UiMsg_CtrlButton_Back, player_index, false, 1.0, 0.0, sMask );
		}
	}
}

void C_UiManager::Process( const Maths::C_TimeS& frameTime )
{
#ifdef _DEBUG_POLYFONT_
	if ( g_UiPolygonFontDebugReset || g_UiPolygonFontDebugResetEveryFrame )
	{
#ifdef PLATFORM_XENON
		Display::Device()->BlockUntilIdle();
#endif
		g_UiPolygonFontDebugReset = false;

		gUIPolygonFont.DestroyAll();
		// rebuild
		Util::C_String font = Util::C_String( "DontShip\\Ui\\Data\\" ) + g_UiPolygonFontFont;
		gUIPolygonFont.BuildFont( font.Chars(), true );
		gUIPolygonFont.BuildFont( font.Chars() );		
	}
#endif // _DEBUG_POLYFONT_

//	gUITextureManager.FlushCreateLuaEnums(g_pUILua->GetState());

	if( UiXt::C_FontStyleList::Instance().IsDirty() )
		UiXt::C_FontStyleList::Instance().FlushBind();

	// update the camera
	C_UICamera::Instance().Update();

	// update the friends list
	C_UIFriends::Instance().Update();

	theFETimer.Update( frameTime );

	{
		ProcessController();
	}

	{
		gSimple2D.StartFrame();
		gFont.StartFrame();
	}

	{
		{
			gDashboardInterface.Update();
			gGameSaveSystem.Update();
		}

		{
//			gUITextureManager.Update();
		}

		{
			for ( S32 i = 0; i < C_ScreenControlManager::Instance().Count(); i++ )
				C_ScreenControlManager::Instance().Get( i ).Update();

			gButtons.ProcessDeferredMessages();
		}

		{
			for(U32 i = 0; i < MAX_MOVIE_SLOTS; i ++)
				gMovie[i].Update();
		}
	}

	{
		gUIParticleMan.Render();
		gUIParticleMan.Update( frameTime.Get() );
	}
#ifndef CFG_GOLD
	gSimple2D.AddDebugHelpers();
#endif

	gSimple2D.BuildSpriteBatches();

	// Update sourcery data
	UiXt::C_ColourStyleList::Instance().FrameUpdate( g_pUILua->GetState() );
	UiXt::C_ScreenList::Instance().FrameUpdate();

	// Needs to be called after the gadgets frame update so that shapes can be reloaded on the fly.
	C_AdobeSceneManager::Instance().Update();

#ifndef GFG_GOLD
	// update the LUA console
	g_pUILua->DoConsole();
#endif
}

void C_UiManager::PreRender( )
{
#ifdef _RENDER_TARGET_
	// resolve render targets
	if ( m_pRenderTarget0->BeginScene() )
	{
		if ( gSimple2D.ShouldRender( E_SpritePanel_RT0 ) )
		{
			gSimple2D.SetTransformMatrix2D( C_VinylManager::Instance().Transform2D() );
			gSimple2D.PreRender();

			for ( U32 i = 0; i < gSimple2D.LayerCount(); i++ )
				gSimple2D.Render( i, E_SpritePanel_RT0 );

			gSimple2D.PostRender();
		}

		m_pRenderTarget0->EndScene();
	}
#endif
}

void C_UiManager::Render()
{
	// ***
#ifdef _DEBUG_POLYFONT_
#ifndef CFG_GOLD
	gUIPolygonFont.DebugRender();
#endif
#endif // _DEBUG_POLYFONT_
	// ***

	gSimple2D.PreRender();

	// 3D sprites live in panels

	// panel 1 has 3D stuff with the debug display camera
	const Display::C_ViewSettings3D& viewSettings = Display::Device()->GetViewSettings3D();
	Maths::HMat44 viewProjMatrix( Maths::Load< Maths::HMat44 >( &viewSettings.GetViewProjMatrix() ) );

	gSimple2D.SetTransformMatrix3D( 
		viewProjMatrix, 
		viewSettings.GetCameraPosition() 
	);

	for ( U32 i = 0; i < gSimple2D.LayerCount(); i++ )
		gSimple2D.Render( i, E_SpritePanel_3D0 );

	// panel 2 has 3D stuff with the C_UICamera
	gSimple2D.SetTransformMatrix3D( 
		C_UICamera::Instance().GetSettings3D().GetViewProjMatrix(),
		C_UICamera::Instance().GetSettings3D().EyePosition()
	);

	for ( U32 i = 0; i < gSimple2D.LayerCount(); i++ )
		gSimple2D.Render( i, E_SpritePanel_3D1 );

	// panel 3 has 2D stuff with the C_UICamera
	gSimple2D.SetTransformMatrix2D( C_UICamera::Instance().GetSettings2D().GetTransformMatrix() );

	for ( U32 i = 0; i < gSimple2D.LayerCount(); i++ )
		gSimple2D.Render( i, E_SpritePanel_2D1 );
	
	// back to 2D
	gSimple2D.SetTransformMatrix2D();

	// debug 2D camera movement
#ifndef CFG_GOLD
	if ( g_UICameraDebugActive2D )
		gSimple2D.SetTransformMatrix2D( C_UICamera::Instance().GetSettings2D().GetTransformMatrix() );
#endif
	
	// render into main panel
	for ( U32 i = 0; i < gSimple2D.LayerCount(); i++ )
		gSimple2D.Render( i, E_SpritePanel_Main );

	gSimple2D.PostRender();
}

LUALIB_API int luaopen_buttons(lua_State *pState);
LUALIB_API int luaopen_screen(lua_State *pState);
LUALIB_API int luaopen_vars(lua_State *pState);
LUALIB_API int luaopen_controller(lua_State *pState);
LUALIB_API int luaopen_system(lua_State *pState);
LUALIB_API int luaopen_hardware(lua_State *pState);
LUALIB_API int luaopen_profile( lua_State* pState );
LUALIB_API int luaopen_shape( lua_State* pState );
LUALIB_API int luaopen_marketplace( lua_State* pState );

LUALIB_API int luaopen_game_profile( lua_State* pState );
LUALIB_API int luaopen_vinyl_editor( lua_State* pState );
LUALIB_API int luaopen_debug2( lua_State* pState );

// Lua libraries that we are going to load.
static const luaL_reg lualibs[] = {
        { "base",			luaopen_base },
		{ "table",			luaopen_table },
		{ "string",			luaopen_string },
		{ "math",			luaopen_math },
		{ "debug",			luaopen_debug },
		// UI
		{ "buttons",		luaopen_buttons },
		{ "screen",			luaopen_screen },
		{ "pad",			luaopen_controller },
		{ "vars",			luaopen_vars },
		{ "hardware",		luaopen_hardware },
		{ "system",			luaopen_system },
		{ "profile",		luaopen_profile },
		{ "shape",			luaopen_shape },
		{ "marketplace",	luaopen_marketplace },
		// Game 
		{ "game_profile",	luaopen_game_profile },
		{ "vinyl_editor",	luaopen_vinyl_editor },

		{ "debug",			luaopen_debug2 },
		// End of list
        { NULL,				NULL } 
};

const bool C_UiInterface::IsWideScreen()
{
	return Display::Device()->GetWideScreen();
}

void C_UiInterface::PlaySound( const S32 sIndex, const F32 fValue1, const F32 fValue2 )
{
	//
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
	case E_GameSpriteType_VinylList :
		C_VinylManager::Instance().RenderList( pSprite );
		return true;

	case E_GameSpriteType_VinylShape :
		C_VinylManager::Instance().RenderShape( pSprite );
		return true;
	}

	return false;
}

Display::ResourceBundler::I_ResourceBundle* C_UiInterface::CreateResourceBundle()
{
	return new Display::ResourceBundler::C_ResourceBundleSingle();
}

} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
