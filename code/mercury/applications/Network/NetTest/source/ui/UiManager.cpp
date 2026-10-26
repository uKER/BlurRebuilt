#include "Stdafx.h"

#include "UiManager.h"
#include "OverloadNew.h"

#include <Display/Bundler/Bundler.h>

//#include "game.h"
//#include "GameScripting/LuaSys.h"
//

#include "netApp/netApp.h"
#include <Network/Friend.h>

#include <ui/DashboardInterface.h>
#include <ui/NetPanelGadget.h>
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
#include <ui/Graphics/Font/Font.h>
#include <ui/Graphics/Font/FontStyle.h>
#include <ui/Graphics/Font/UIPolygonFont.h>
#include <ui/Graphics/Movie.h>
////#include <ui/Graphics/PostProcess.h>
#include <ui/Graphics/UITextureManager.h>
#include <ui/Graphics/Localisation/Loc.h>
#include <ui/Graphics/SimpleTimer.h>
#include <ui/Graphics/UIParticle.h>
#include <ui/Illustrator/UIAdobeSceneManager.h>
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
#include "Game/GameDataFinder.h"
#include <ui/Game/Profiles/ProfileDataFinder.h>

// profiles
#include <ui/Game/Profiles/ProfileInterface.h>

// Gadget includes
//#include "Game/Gadgets/UIDynamicScene.h"
//#include "Game/Gadgets/UITestLeaderboardNEW.h"

// Gui
////#include "Game/Widgets/UIFrame.h"
//#include "Game/Widgets/UICanvas.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

#ifndef CFG_GOLD
Debug::Bool	g_debugUIRender("UI.Render", true);
Debug::Bool g_debugLUAAllowStepping("UI.Lua.AllowStepping", false);
extern Debug::Bool g_debugUseLUAFromHDD;
extern Debug::Bool g_UICameraDebugActive2D;
extern Debug::Float g_UICameraSpeedOffset2D;
extern Debug::Float g_UICameraSpeedScale2D;
#endif

#ifdef _DEBUG_POLYFONT_
Debug::Bool		g_UiPolygonFontDebugReset( "UI.PolygonFont.DebugReset", false );
Debug::Bool		g_UiPolygonFontDebugResetEveryFrame( "UI.PolygonFont.DebugResetEveryFrame", false );
Debug::String	g_UiPolygonFontFont( "UI.PolygonFont.Font", "Arial_Unicode_MS.pf" );
#endif // _DEBUG_POLYFONT_

// ***

void C_GameFileSystem_UpdateSystemMessages(){}

// ***
#ifdef PLATFORM_PS3
uint32_t _binary___Ui_Data_UI_bin_start = 0;
uint32_t _binary___Ui_Data_UI_bin_size = 0;
#endif

XT_LIBRARY( GameUI )
//	XT_REGISTER_TYPE( UI::UIDynamicScene )
//	XT_REGISTER_TYPE( UI::UIFrame )
//	XT_REGISTER_TYPE( UI::UICanvas )
//	XT_REGISTER_TYPE( UI::C_FrameStyle )
//	XT_REGISTER_TYPE( UI::C_CanvasStyle )
//	XT_REGISTER_TYPE( UI::C_HeaderStyle )
//	XT_REGISTER_TYPE( UI::C_ScrollBarStyle )
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
// global colour repository - TODO
//C_UiColourRepository gUIColourRepository;

C_UiInterface* g_pManager;

I_UiInterface* UiInterface()
{
	return g_pManager;
}

// ---
C_UiManager::C_UiManager( const S32 sInitParam )
{
	MEMORY_TRACKER_PUSH("ui");

	// Ouch!
	g_pManager = &m_UiInterface;

	// Lua sys
	g_pUILua = new C_UILua();
	assert( g_pUILua );

	C_ScreenControlManager::Instance().Add();		// Main
	C_ScreenControlManager::Instance().Add();		// Popups
	C_ScreenControlManager::Instance().Context(0);	// Default context is 'main'

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

	lua_State* pState = g_pUILua->GetState();

#ifndef _REMOVE_STRING_STORE_
	C_StringStore::Instance().Init();	
#endif
	gUITextureManager.LoadCommonTextures( "" );
	gSimple2D.Init( pState );

	const char* pUILayerNames[ 6 ] = 
	{
		"BG",
		"1",
		"2",
		"3",
		"4",
		"TOP",
	};
	const char* pUIPanelNames[ 6 ] = 
	{
		// -1 is _always_ the main panel
		"RT0",
		"RT1",
		"VP0",
		"VP1",
		"VP2",
		"VP3",
	};

	//gSimple2D.SetLayersAndPanels( pState, pUILayerNames, lengthof( pUILayerNames ), pUIPanelNames, lengthof( pUIPanelNames ) );

	gSimple2D.SetClipSize2D( Display::C_ViewSetup::GetDeviceCoords().Max().X(), Display::C_ViewSetup::GetDeviceCoords().Max().Y() );

	// alternate textures
	// movie
	gSimple2D.SetAlternateTexture( new C_MovieAlternateTexture( 0 ) );
	//render target
	m_pRenderTarget0 = new C_Simple2D::C_RenderTargetAlternateTexture( 0 );
	gSimple2D.SetAlternateTexture( m_pRenderTarget0 );

#ifdef _DEBUG_POLYFONT_
	gUIPolygonFont.Init();
	Util::C_String font = Util::C_String( "GAME:\\User\\Ui\\Data\\" )
	gUIPolygonFont.BuildFont( font.Chars(), true );
	gUIPolygonFont.BuildFont( font.Chars() );
#endif

	/// *** localiser MUST be initialised before the font library now - default height percentage depends on language *** ///

	const char* tables[] = 
	{ 
		"Xt\\UITestBed.loc",
		"Xt\\Multiplayer-en.loc"
	};

	// loc table MUST be loaded before intialising the localiser
	for(int i = 0; i < sizeof(tables) / sizeof(*tables); i ++)
	{
		UI::C_TextStringID::LocTable().Load( tables[i], NULL, i );
	}
	C_TextStringID::LocTable().CreateLuaEnums( pState );

	// setup the localiser - just english for now
	C_UILocaliser::Instance().Init( E_UILanguage_English, E_UIRegion_Europe );

	gFont.Initialise( "Ui\\Data\\ui_font_eur.dat" );

	#ifdef PLATFORM_PC
	gFont.Initialise( "z:\\devroot\\data\\uitestbed\\PC\\data\\image\\ui\\data\\ui_font_eur.dat" );
	#else
	gFont.Initialise( "GAME:\\Image\\Ui\\Data\\ui_font_eur.dat" );
	#endif

#ifndef _DEBUG_POLYFONT_
	#ifdef PLATFORM_PC
	//gFont.CreatePolygonFont( "z:\\devroot\\data\\uitestbed\\PC\\data\\user\\ui\\data\\Arial_Unicode_MS.pf" );
	#else
#if !defined (USER_DGILL)
	//gFont.CreatePolygonFont( "GAME:\\User\\Ui\\Data\\Verdana.pf", true, true, true, true, true );
#endif
	#endif
#endif
	
	gUIParticleMan.Init( 1 );

	// Must be done before buttons are registered
	gButtons.InitCreator( E_ButtonType_Max, true );

	// ***
	// Register button types here
	// ***


//	gButtons.Register<UIDynamicScene>();
//	gButtons.Register<UITestLeaderboardNEW>();
//	gButtons.Register<UIFrame>();
//	gButtons.Register<UICanvas>();
	gButtons.Register<NetPanelGadget>();


	gButtons.CreateLuaEnums( pState );

	CreateGameLuaEnums( pState );

	gUITextureManager.CreateLuaEnums( pState );

	gSimple2D.BuildLuaEnumsForAlternateTextures( pState );

	// load in sorcery definitions
	g_UIXt.LoadDefinitions( "Xt\\UI.bin" );

	// Load any fonts defined in sorcery
	UiXt::C_FontStyleList::Instance().LoadPolygonFonts();

	// font enums
	gFont.CreateLuaEnums( pState );
	C_FontStyleCreator::Instance().Initialise();
	C_FontStyleCreator::Instance().CreateLuaEnums( pState );

	// Run setup when all enums have been added
#ifndef CFG_GOLD
	g_pUILua->LoadCallSetupScript( sInitParam, g_debugUseLUAFromHDD );
#else
	g_pUILua->LoadCallSetupScript( sInitParam, false );
#endif

	gUIScreenControl.SetLuaScreenDir("Screens\\");
	gUIScreenControl.SetInitFunctionName( "Init" );
	gUIScreenControl.SetStartLoopName( "StartLoop" );
	gUIScreenControl.SetFrameUpdateName( "FrameUpdate" );
	gUIScreenControl.SetMessageUpdateName( "MessageUpdate" );
	gUIScreenControl.SetEndLoopName( "EndLoop" );
	gUIScreenControl.SetEndFunctionName( "End" );
	gUIScreenControl.Init();

	gUIPopupControl.SetLuaScreenDir("Popups\\");
	gUIPopupControl.SetInitFunctionName( "PopupInit" );
	gUIPopupControl.SetStartLoopName( "PopupStartLoop" );
	gUIPopupControl.SetFrameUpdateName( "PopupFrameUpdate" );
	gUIPopupControl.SetMessageUpdateName( "PopupMessageUpdate" );
	gUIPopupControl.SetEndLoopName( "PopupEndLoop" );
	gUIPopupControl.SetEndFunctionName( "PopupEnd" );

	// _after_ loading Lua scripts
	CreateGameGlobals( pState );

	gUIScreenControl.SetDefaultScreenName( "Blank.lua" );

	// init font styles - _after_ we load Lua common files in
	C_FontStyle::Instance().InitDefaultStyle( pState );

	// check if we've now got a font sequences LUA table loaded
	gFont.InitialiseSequences( pState );

	// Push loaded shapes into the lua enum table.
	C_AdobeSceneManager::Instance().CreateLuaEnums( pState );

	// hermite animation curves - MUST be after we load in from Sorcery
	g_UIHermiteCurves.CreateLuaEnums( pState );

	// add in the datafinders
	C_UIDataFinder::Instance().AddGameDataFinder( &C_GameDataFinder::Instance() );
	C_UIDataFinder::Instance().AddGameDataFinder( &C_ProfileDataFinder::Instance() );

	// initialise UI camera
	C_UICamera::Instance().Init();

#if defined (USER_PREID)
	// ***
	m_PolyTest.Init();
	// ***
#endif

	//gUIScreenControl.AddMessage("LOADED");

	MEMORY_TRACKER_POP("ui");
}

void C_UiManager::CreateTextureBundle( const bool bIngameBundle )
{
	#ifdef PLATFORM_PC
	gUITextureManager.Create( bIngameBundle, "z:\\devroot\\data\\uitestbed\\PC\\data\\image\\" );
	#else
	gUITextureManager.Create( bIngameBundle, "GAME:\\Image\\" ); 
	#endif

	gUITextureManager.DeferCreateLuaEnums();
}

void C_UiManager::DeleteTextureBundle( const bool bDeleteAll )
{
	gUITextureManager.Delete( bDeleteAll ); // Wipe them out; all of them
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
	// profile invite modes - TODO - this will be shared tech eventually
	lua_pushstring( pState, "AcceptGameInviteMode" );
	lua_newtable( pState );
		Lua::AddDataMember( pState, "Accept",	E_AcceptGameInviteMode_Accept );
		Lua::AddDataMember( pState, "Delay",	E_AcceptGameInviteMode_Delay );
		Lua::AddDataMember( pState, "Discard",	E_AcceptGameInviteMode_Discard );
	lua_rawset( pState, -3 );

	// TODO
	// colours
	//lua_pushstring(pState, "Colour");
	//lua_newtable(pState);
	//for (E_UiColour eColour = E_UiColour_Start; eColour < E_UiColour_Count; eColour = (E_UiColour)(eColour + 1))
	//	Lua::AddDataMember(pState, gUIColourNames[eColour], gUIColourRepository.GetColour(eColour).LUAString());
	//lua_rawset(pState, -3);
}

C_UiManager::~C_UiManager()
{
	g_pUILua->End();
	gUITextureManager.Delete( true ); // Delete all the textures bundles
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

#ifndef _REMOVE_STRING_STORE_
	C_StringStore::Instance().Exit();
#endif
	// Clean up loaded shapes.
	C_AdobeSceneManager::Instance().FlushData();

	C_UIDataFinder::Instance().Cleanup();

	C_FontStyle::Instance().Cleanup();

	g_pManager = NULL;
}

void C_UiManager::ProcessController( const Samples::C_Input& input )
{
	// sample pad input is different, we get told on a button down once held for the first time, then nothing else
	// - oh and only for pad 0?

}

void C_UiManager::Process( const Maths::C_TimeS& time, const Samples::C_Input& input )
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
		Util::C_String font = Util::C_String( "GAME:\\User\\Ui\\Data\\" ) + g_UiPolygonFontFont;
		gUIPolygonFont.BuildFont( font.Chars(), true );
		gUIPolygonFont.BuildFont( font.Chars() );		
	}
#endif // _DEBUG_POLYFONT_

	gUITextureManager.FlushCreateLuaEnums(g_pUILua->GetState());

	// update the camera
	C_UICamera::Instance().Update();

	theFETimer.Update( time );

	{
		ProcessController( input );
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
			gUITextureManager.Update();
		}

		{
			gUIScreenControl.Update();
			gUIPopupControl.Update();

			gButtons.ProcessDeferredMessages();
		}

		{
			for(U32 i = 0; i < MAX_MOVIE_SLOTS; i ++)
				gMovie[i].Update();
		}
	}

#ifndef CFG_GOLD
	gSimple2D.AddDebugHelpers();
#endif

	{
//		gButtons.Render();
//		gButtons.RenderPopups();
	}

	{
		gUIParticleMan.Render();
		gUIParticleMan.Update( time.Get() );
	}

	{
		/*
		UI::E_ManagerMsg msg = (UI::E_ManagerMsg)g_pUILua->Message();
		if(msg != UI::E_ManagerMsg_None)
		{
			// Clear the message
			g_pUILua->Message(UI::E_ManagerMsg_None);
			return msg;
		}
		*/
	}

	// Update sourcery data
	UiXt::C_ColourStyleList::Instance().FrameUpdate( g_pUILua->GetState() );
	UiXt::C_ScreenList::Instance().FrameUpdate();

	C_AdobeSceneManager::Instance().Update();

	m_PolyTest.Update();

#ifndef GFG_GOLD
	// update the LUA console
	g_pUILua->DoConsole();
#endif

	// resolve render targets
	if ( m_pRenderTarget0->BeginScene() )
	{
		if ( gSimple2D.ShouldRender( E_SpritePanel_RT0 ) )
		{
			gSimple2D.PreRender();

			for ( S32 i = 0; i < gSimple2D.LayerCount(); i++ )
				gSimple2D.Render( i, E_SpritePanel_RT0 );

			gSimple2D.PostRender();
		}

		m_pRenderTarget0->EndScene();
	}
}

void C_UiManager::Render( const bool bIsMasterLayer, const S32 sPlayerIndex )
{
	// ***
	m_PolyTest.Render();
#ifdef _DEBUG_POLYFONT_
#ifndef CFG_GOLD
	gUIPolygonFont.DebugRender();
#endif
#endif // _DEBUG_POLYFONT_
	// ***

	gSimple2D.PreRender();

	// 3D sprites live in panels

	// panel 0 has 3D stuff with the debug display camera
	const Display::C_ViewSettings3D& viewSettings = Display::Device()->GetViewSettings3D();
	Maths::HMat44 viewProjMatrix( Maths::Load< Maths::HMat44 >( &viewSettings.GetViewProjMatrix() ) );
	gSimple2D.SetTransformMatrix3D( viewProjMatrix );

	for ( S32 i = 0; i < gSimple2D.LayerCount(); i++ )
		gSimple2D.Render( i, E_SpritePanel_3D0 );

	// panel 1 has 3D stuff with the C_UICamera
	gSimple2D.SetTransformMatrix3D( C_UICamera::Instance().GetSettings3D().GetViewProjMatrix() );

	for ( S32 i = 0; i < gSimple2D.LayerCount(); i++ )
		gSimple2D.Render( i, E_SpritePanel_3D1 );

	// panel 2 has 2D stuff with the C_UICamera
	gSimple2D.SetTransformMatrix2D( C_UICamera::Instance().GetSettings2D().GetTransformMatrix() );

	for ( S32 i = 0; i < gSimple2D.LayerCount(); i++ )
		gSimple2D.Render( i, E_SpritePanel_2D1 );
	
	// back to 2D
	gSimple2D.SetTransformMatrix2D();

	// debug 2D camera movement
#ifndef CFG_GOLD
	if ( g_UICameraDebugActive2D )
	{
		C_UICameraSettings2D& settings2D = C_UICamera::Instance().GetSettings2D();

		Maths::HMat43 mat;
		mat.SetIdentity();

		// pivot about the middle
		Maths::HVec3 scalePivot( 320.0f - settings2D.OffsetX(), 240.0f - settings2D.OffsetY(), 0.0f );
		mat.Translation( mat.Translation() - scalePivot );
		mat = mat * Maths::HMat43( Maths::C_MatrixScale( Maths::HVec3( settings2D.ScaleX(), settings2D.ScaleY(), 1.0f ) ) );
		mat.Translation( mat.Translation() + scalePivot );

		// add on translation
		mat.Translation( mat.Translation() + Maths::HVec3( settings2D.OffsetX(), settings2D.OffsetY(), 0.0f ) );

		Maths::HMat44 transform( mat );
		Maths::HMat44 newTransform = transform * gSimple2D.GetTransformMatrix2D();

		gSimple2D.SetTransformMatrix2D( newTransform );
	}
#endif
	
	// render into master panel
	for ( S32 i = 0; i < gSimple2D.LayerCount(); i++ )
	{
		if ( bIsMasterLayer )
			gSimple2D.Render( i, E_SpritePanel_Main );
		else
		{
			// Render into the player viewports
			gSimple2D.Render( i, E_SpritePanel_VP0 + sPlayerIndex );	// TODO - make a new enum if we override panel count
		}
	}

	gSimple2D.PostRender();
}

LUALIB_API int luaopen_buttons(lua_State *pState);
LUALIB_API int luaopen_screen(lua_State *pState);
LUALIB_API int luaopen_vars(lua_State *pState);
LUALIB_API int luaopen_controller(lua_State *pState);
LUALIB_API int luaopen_system(lua_State *pState);
LUALIB_API int luaopen_hardware(lua_State *pState);
LUALIB_API int luaopen_profile( lua_State* pState );

LUALIB_API int luaopen_game_profile( lua_State* pState );

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
		// Game 
		{ "game_profile",	luaopen_game_profile },
		// End of list
        { NULL,				NULL } 
};

const bool C_UiInterface::IsWideScreen()
{
	return Display::Device()->GetWideScreen();
}

void C_UiInterface::PlaySound( const S32 sIndex )
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

Display::ResourceBundler::I_ResourceBundle* C_UiInterface::CreateResourceBundle()
{
	return new Display::ResourceBundler::C_ResourceBundleSingle();
}

} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
