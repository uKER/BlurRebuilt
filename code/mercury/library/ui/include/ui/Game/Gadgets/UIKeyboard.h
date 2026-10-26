#ifndef UIKEYBOARD_H
#define UIKEYBOARD_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Util/WideString.h"

namespace UI
{

// ------------
// C_UIKeyboard
// ------------
class C_UIKeyboard
{
public:
	C_UIKeyboard()
		: m_bInitialised( false )
		, m_bDisplayed( false )
		, m_bHighlight( false )
		, m_sContext( -1 )
		, m_sRestoreContext( 0 )
	{
		//
	}
public:
	void					Initialise( const S32 sContext, const char* pScriptName = "Keyboard.lua" );

	void					Start( const S32 sMaxCharacters = -1, const WCHAR* pString = NULL, const bool bHighlight = false );
	void					Cancel( const C_WideString256& string = C_WideString256() );

	C_WideString256&		GetString()				{ return m_String; }
	S32						GetMaxCharacters()		{ return m_sMaxCharacters; }
	bool					GetHighlight()			{ return m_bHighlight; }

public:
	static C_UIKeyboard&	Instance()				{ return m_sInstance; }

private:
	static C_UIKeyboard		m_sInstance;

private:
	bool					m_bInitialised;
	bool					m_bDisplayed;
	bool					m_bHighlight;

	S32						m_sContext;
	S32						m_sRestoreContext;

	C_WideString256			m_String;

	static const U32		m_uMaxScriptNameLength = 32;
	char					m_pScriptName[ m_uMaxScriptNameLength ];

	static const U32		m_uMaxCharacterDefault = 16;
	S32						m_sMaxCharacters;
};

// ----------
// UIKeyboard
// ----------
class UIKeyboard : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_Keyboard, "KEYBOARD" )

public:
	void 	Init();
	void 	Setup( lua_State* pState );
	bool 	MessageHandler( const Messaging::C_MessageBase* pMessage );
	void 	Render();

private:
	WCHAR	GetCharacter( const S32 sRow, const S32 sCol );

private:
	S32					m_sRow;
	S32					m_sCol;
	S32					m_sPosition;
	C_WideString256		m_String;
	bool				m_bCaps;
	F32					m_fCharacterSize;

	WCHAR				w[ 2 ];

	bool				m_bHighlight;
};

};	// namespace UI

#endif /* UIKEYBOARD_H */
