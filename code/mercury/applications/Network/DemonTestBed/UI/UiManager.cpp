#include "Stdafx.h"

#include "UiManager.h"
#include "OverloadNew.h"

#include <Display/Bundler/Bundler.h>

//#include "game.h"
//#include "GameScripting/LuaSys.h"
//

#include <ui/DashboardInterface.h>
#include <ui/GameSaveSystem.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
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
#include "Game/Gadgets/UIDynamicScene.h"
#include "Game/Gadgets/UITestLeaderboardNEW.h"

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
	XT_REGISTER_TYPE( UI::UIDynamicScene )
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
}

void C_UiManager::CreateTextureBundle( const bool bIngameBundle )
{
}

void C_UiManager::DeleteTextureBundle( const bool bDeleteAll )
{
}

void C_UiManager::CreateGameGlobals( lua_State* pState )
{
}

void C_UiManager::CreateGameLuaEnums( lua_State* pState )
{
}

C_UiManager::~C_UiManager()
{
}

void C_UiManager::ProcessController( const Samples::C_Input& input )
{
}

void C_UiManager::Render( const bool bIsMasterLayer, const S32 sPlayerIndex )
{
	gSimple2D.PostRender();
}



const bool C_UiInterface::IsWideScreen()
{
	return true;
}

void C_UiInterface::PlaySound( const S32 sIndex )
{
	//
}

// Open the LUA libraries
void C_UiInterface::RegisterLibs( lua_State *pState )
{
}

Display::ResourceBundler::I_ResourceBundle* C_UiInterface::CreateResourceBundle()
{
	return NULL;
}

} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
