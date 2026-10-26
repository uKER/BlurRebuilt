#ifndef GAMEKEYBOARD_H
#define GAMEKEYBOARD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/SystemHeaders.h>

// yeah ... this isn't very nice. But the PS3 won't compile with Xbox enums!
#define MAGIC_KEYBOARD_OVERRIDE		0xa1b2c3d4

#ifdef PLATFORM_PS3
#include <sysutil/sysutil_oskdialog.h>
#endif

namespace UI
{

class C_DashboardInterface
{
public:
	C_DashboardInterface() 
		: mInitialised( false )
	{
	}

	~C_DashboardInterface()
	{
	}

public:
	typedef enum
	{
		E_SignInType_None,
		E_SignInType_LocalOnly,
		E_SignInType_ShowOnlineEnabled,
	} E_SignInType;

	void		Init();
	const bool	StartXenonKeyboardUI(const WCHAR *DefaultText,const WCHAR *TitleText,const WCHAR *DescText,const int NumChars, U64 uFlags = MAGIC_KEYBOARD_OVERRIDE );
	const bool	StartXenonSigninUI( const S32 numPlayers, const E_SignInType flags );
	const bool	StartXenonPlayersUI( );
	void		SetIndex(const S32 user)	{ mUserIndex = user; }
	WCHAR *		GetString()					{ return mResultText; }

	void		Update();


#ifdef PLATFORM_PS3
	void		SetKeyboardFinished()		{ mKeyboardFinished = true; }
	void		SetKeyboardCancelled()		{ mKeyboardCancelled = true; }
#endif

	static const S32	mResultTextLength = 512;

private:
	bool		StartPs3KeyboardUI( const WCHAR* pDefaultText, const WCHAR* pTitleText, S32 sNumCharacters );
	void		ValidateKeyboardString();

private:
	bool				mInitialised;

	S32					mUserIndex;
	WCHAR				mResultText[mResultTextLength];
	bool				mKeyboardActive;
#if defined(LIVE_ON_WINDOWS)
	XOVERLAPPED			mOverlapped;
#endif

#ifdef PLATFORM_PS3
	S32					mNumCharacters;
	uint16_t			mResultTextBuffer[ CELL_OSKDIALOG_STRING_SIZE + 1 ];
	bool				mKeyboardCancelled;
	bool				mKeyboardFinished;
#endif
};

extern C_DashboardInterface gDashboardInterface;

}; // namespace UI
#endif

