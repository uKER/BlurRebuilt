#ifndef UIMANAGER_H
#define UIMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/UIDefines.h>
#include <ui/UiInterface.h>

#include "FileSystem.h"

// ***
#include "PolyTest.h"
// ***

#include "Samples/System/Input.h"

#define LUA_READ_ONLY_FROM_HDD

namespace UI
{

//forward decl
class C_Simple2D::C_RenderTargetAlternateTexture;

// button types
enum E_ButtonType
{
	E_ButtonType_DynamicScene = E_UIButtonType_EndMarker,
	E_ButtonType_LeaderboardTest,
	E_ShooterButtonType_NetPanel,
	E_ButtonType_Canvas,

	E_ButtonType_Max,
};

class C_UiInterface : public I_UiInterface
{
public:
	C_UiInterface( )												{  }
	virtual ~C_UiInterface()										{  }

	const bool			IsWideScreen();
	void				PlaySound( const S32 index );
	Sys::I_FileSystem*	GetFileSys()								{ return &g_UiFileSystem; }
	void				RegisterLibs( lua_State *pState );
	const bool			GetMousePos( F32& x, F32& y )				{ return false; }
	const bool			GetLeftMouseDown()							{ return false; }
	const bool			GetMouseScrollUp()							{ return false; }
	const bool			GetMouseScrollDown()						{ return false; }

	const bool			UseMouseInput( )							{ return false; }
	void				UseMouseInput( const bool bUseMouseInput )	{  }

	Display::ResourceBundler::I_ResourceBundle*		CreateResourceBundle();

	void							RenameMovieFor50Hz( const char* pOriginalFileName, char* pNewFileName, const S32 sBufferSize )			{  }
	void							RenameMovieForSmallNTSC( const char* pOriginalFileName, char* pNewFileName, const S32 sBufferSize )		{  }

private :
	//
};

// -- 
class C_UiManager
{
public:
	C_UiManager( const S32 sInitParam );
	~C_UiManager( );

	void							Process( const Maths::C_TimeS& time, const Samples::C_Input& input );
	void							Render( const bool bIsMasterLayer = true, const S32 sPlayerIndex = -1 );

	void							CreateTextureBundle( const bool bIngameBundle );
	void							DeleteTextureBundle( const bool bDeleteAll );

private:
	void							ProcessController( const Samples::C_Input& input );
	void							CreateGameGlobals( lua_State* pState );
	void							CreateGameLuaEnums( lua_State* pState );

private:
	C_UiInterface									m_UiInterface;
	C_Simple2D::C_RenderTargetAlternateTexture*		m_pRenderTarget0;

	// ***
	C_PolyTest										m_PolyTest;
	// ***
};

// TODO
//extern C_UiColourRepository gUIColourRepository;

} // namespace UI
#endif
