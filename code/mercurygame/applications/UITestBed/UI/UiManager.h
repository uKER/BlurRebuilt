#ifndef UIMANAGER_H
#define UIMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/UIDefines.h>
#include <ui/UiInterface.h>

#include "FileSystem.h"

#include <Framework/Application/UserControls.h>

//#define LUA_READ_ONLY_FROM_HDD

enum E_UiButton
{
	E_UiButton_Pad_Up = Framework::E_Button_Count,
	E_UiButton_Pad_Down,
	E_UiButton_Pad_Left,
	E_UiButton_Pad_Right,
	E_UiButton_Confirm,
	E_UiButton_Cancel,
	E_UiButton_Option1,
	E_UiButton_Option2,
	E_UiButton_LeftTrigger,
	E_UiButton_RightTrigger,
	E_UiButton_LeftShoulder,
	E_UiButton_RightShoulder,
	E_UiButton_LeftThumb,
	E_UiButton_RightThumb,
	E_UiButton_Start,
	E_UiButton_Back,

	E_UiButton_PadOrAxis_Up,  
	E_UiButton_PadOrAxis_Down,	
	E_UiButton_PadOrAxis_Left,
	E_UiButton_PadOrAxis_Right,

	E_UiButton_Count,
};

enum E_UiAxis
{
	E_UiAxis_LeftStick_UD = Framework::E_Axis_Count,
	E_UiAxis_LeftStick_LR,

	E_UiAxis_RightStick_UD,
	E_UiAxis_RightStick_LR,

	E_UiAxis_Count,
};

namespace UI
{

//forward decl
class C_Simple2D::C_RenderTargetAlternateTexture;

// button types
enum E_ButtonType
{
	E_UIGameButtonType_ColourPicker = E_UIButtonType_EndMarker,
	E_UIGameButtonType_VinylScene,
	E_UIGameButtonType_VinylPreview,
	E_UIGameButtonType_VinylStripBar,
	
	E_ButtonType_TestList,

	E_ButtonType_Max,
};

enum E_GameSpriteType
{
	E_GameSpriteType_VinylList = E_SpriteType_Count,
	E_GameSpriteType_VinylShape,

	E_GameSpriteType_Count
};

class C_UiInterface : public I_UiInterface
{
public:
	C_UiInterface( )												{  }
	virtual ~C_UiInterface()										{  }

	const bool			IsWideScreen();
	void				PlaySound( const S32 sIndex, const F32 fValue1 = 0.0f, const F32 fValue2 = 0.0f );
	Sys::I_FileSystem*	GetFileSys();
	void				RegisterLibs( lua_State *pState );
	const bool			GetMousePos( F32& x, F32& y )				{ return false; }
	const bool			GetLeftMouseDown()							{ return false; }
	const bool			GetMouseScrollUp()							{ return false; }
	const bool			GetMouseScrollDown()						{ return false; }

	const bool			UseMouseInput( )							{ return false; }
	const bool			ExternalRender( const C_Sprite* pSprite );
	void				UseMouseInput( const bool bUseMouseInput )	{  }
	void				PollPlatformCallbacks()						{  }

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

	void							Process( const Maths::C_TimeS& frameTime );
	void							PreRender();
	void							Render();

	void							CreateTextureBundle( const bool bIngameBundle );
	void							DeleteTextureBundle( const bool bDeleteAll );

private:
	void							ProcessController();
	void							CreateGameGlobals( lua_State* pState );
	void							CreateGameLuaEnums( lua_State* pState );

private:
	C_TextureManager::C_ResourceIndex				m_SwitchBundle;
	C_UiInterface									m_UiInterface;
	C_Simple2D::C_RenderTargetAlternateTexture*		m_pRenderTarget0;
	Maths::HMat44									m_RT0Transform;
};

}; // namespace UI

#endif /* UIMANAGER_H */
