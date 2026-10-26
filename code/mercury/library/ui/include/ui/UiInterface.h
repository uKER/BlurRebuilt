#ifndef UIINTERFACE_H
#define UIINTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

extern "C" 
{
	#include <lua.h>
}

#include "system/serialise.h"

#include "ui/Game/Gadgets/BaseUIVariableSelect.h"
#include "ui/Game/Gadgets/Buttons.h"

namespace UI
{

enum E_BaseSoundEffect
{
	E_BaseSoundEffect_Move = 0,
	E_BaseSoundEffect_Forwards,
	E_BaseSoundEffect_Backwards,
	E_BaseSoundEffect_Error,
	E_BaseSoundEffect_Toggle,
	E_BaseSoundEffect_Up,
	E_BaseSoundEffect_Down,
	E_BaseSoundEffect_Left,
	E_BaseSoundEffect_Right,

	E_BaseSoundEffect_Count,
};


class I_UiInterface
{
public:
	virtual ~I_UiInterface() {}
	virtual const bool			IsWideScreen() = 0;
	virtual void				PlaySound( const S32 sIndex, const F32 fValue1 = 0.0f, const F32 fValue2 = 0.0f ) = 0;
	virtual void				RegisterLibs( lua_State *pState ) = 0;
	virtual const bool			GetMousePos( F32& x, F32& y ) = 0;
	virtual const bool			GetLeftMouseDown() = 0;
	virtual const bool			UseMouseInput( ) = 0;
	virtual void				UseMouseInput( const bool is_using ) = 0;
	virtual const bool			GetMouseScrollUp() = 0;
	virtual const bool			GetMouseScrollDown() = 0;

	virtual const S32			GetSoundIndex( const E_BaseSoundEffect eEffect )									{ return -1; }
	virtual const bool			ExternalRender( const C_Sprite* sprite )											{ return false; }
	virtual const U32			PushMessageLuaParams( lua_State* pState, const Messaging::C_MessageBase *pMessage )	{ return 0; }

	//virtual void				ColumnRender(const UIScoreboardColumn& column, const F32 x, const F32 y, const U8 layer, const C_UISpriteColours& col, const E_Justify justify, const S32 font, const F32 drawsizey, const S32 panel, const S32 b) = 0;

	virtual Display::ResourceBundler::I_ResourceBundle*	CreateResourceBundle() = 0;

	virtual void				RenameMovieFor50Hz( const char* pOriginalFileName, char* pNewFileName, const S32 sBufferSize ) = 0;
	virtual void				RenameMovieForSmallNTSC( const char* pOriginalFileName, char* pNewFileName, const S32 sBufferSize ) = 0;

	/*
	virtual void				PauseGame( const bool pause ) = 0;
	virtual void				AudioStartMusic( const U32 index ) = 0;
	virtual void				AudioPlaySound( const U32 index ) = 0;
	virtual void				AudioSendMessage( const U32 event ) = 0;
	virtual void				AudioStopMusic() = 0;
	*/

	virtual void				PollPlatformCallbacks() = 0;
};

extern I_UiInterface* UiInterface();

}; // namespace UI

#endif // UIINTERFACE_H
