#include "stdafx.h"

#include <ui/UiInterface.h>
#include <ui/DashboardInterface.h>
#include <ui/Game/UIScreenControl.h>

#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/Gadgets/ButtonMessages.h"

namespace UI
{

#ifdef PLATFORM_PS3
static void sysutil_callback( uint64_t status, uint64_t param, void *userdata );
#endif

C_DashboardInterface gDashboardInterface;

void C_DashboardInterface::Init()
{
#ifdef PLATFORM_PS3
	int ret = cellSysutilRegisterCallback( 0, sysutil_callback, NULL );

	if ( ret != 0 )
	{
		BIZ_ASSERT_FAIL( "cellSysutilRegisterCallback - ret %d", ret );
		return;
	}
#endif

	mInitialised = true;
}

const bool C_DashboardInterface::StartXenonKeyboardUI(const WCHAR *DefaultText,const WCHAR *TitleText,const WCHAR *DescText,const int NumChars, U64 uFlags )
{
#if defined(LIVE_ON_WINDOWS)
	memset(&mOverlapped,0,sizeof(XOVERLAPPED));
#endif	

	mResultText[0] = 0;

	bool success = true;
#if defined(LIVE_ON_WINDOWS)
	// PR - defaulting to Kerbz's flags if it's not been overridden
	if ( uFlags == MAGIC_KEYBOARD_OVERRIDE )
		uFlags = VKBD_DEFAULT | VKBD_HIGHLIGHT_TEXT;

	DWORD error = XShowKeyboardUI(mUserIndex, (DWORD)uFlags, DefaultText,TitleText,DescText,mResultText,NumChars,&mOverlapped);
	success = (error == ERROR_IO_PENDING);
#endif

	mKeyboardActive = success;

#ifdef PLATFORM_PS3
	mKeyboardActive = StartPs3KeyboardUI( DefaultText, DescText, NumChars );
	success = mKeyboardActive;
#endif

	return success;
}

const bool C_DashboardInterface::StartXenonSigninUI( const S32 numPlayers, const E_SignInType type )
{
#if defined(LIVE_ON_WINDOWS)
	DWORD flags;
	switch(type)
	{
	default:
	case E_SignInType_None:
		flags = 0;
		break;
	case E_SignInType_LocalOnly:
		flags = XSSUI_FLAGS_LOCALSIGNINONLY;
		break;
	case E_SignInType_ShowOnlineEnabled:
		flags = XSSUI_FLAGS_SHOWONLYONLINEENABLED;
		break;
	}

	if(XShowSigninUI( numPlayers, flags ) != ERROR_SUCCESS)
	{
		return false;
	}
#endif

	return true;
}

const bool C_DashboardInterface::StartXenonPlayersUI( )
{
#if defined(LIVE_ON_WINDOWS)
	if(XShowPlayersUI(mUserIndex)  != ERROR_SUCCESS)
	{
		return false;
	}
#endif

	return true;
}

void C_DashboardInterface::Update()
{
	if(mKeyboardActive)
	{
#if defined(LIVE_ON_WINDOWS)
		if(XHasOverlappedIoCompleted(&mOverlapped))
		{
			if ( mOverlapped.dwExtendedError == ERROR_CANCELLED )
				_UI_MESSAGE( E_UiMsg_KeyboardCancelled );

			bool ok=true;

			/*
			if(mResultText[0] == 0x20)
				ok=false;
			*/

			if(wcslen(mResultText) == 0)
				ok=false;

			int a=0;
			int spaces=0;
			while(mResultText[a])
			{
				if(mResultText[a] == 0x20)
				{
					spaces++;
				}
				a++;
			}

			if(spaces == a)
				ok=false;

			ValidateKeyboardString();

			if ( ok )
			{
				_UI_MESSAGE( E_UiMsg_KeyboardFinished );
			}
			else
			{
				_UI_MESSAGE( E_UiMsg_KeyboardCancelled );
			}

			mKeyboardActive = false;
		}
#elif PLATFORM_PS3
		// nothing to do
#else
		// TODO
		_UI_MESSAGE( E_UiMsg_KeyboardFinished );
		mKeyboardActive = false;
#endif
		UiInterface()->PollPlatformCallbacks();
	}

#ifdef PLATFORM_PS3
	if ( mKeyboardFinished )
	{
		mKeyboardFinished = false;
		mKeyboardActive = false;

		if ( mKeyboardCancelled )
		{
			// force close
			cellOskDialogAbort();
			_UI_MESSAGE( E_UiMsg_KeyboardCancelled );
		}
		else
		{
			// get the string
			CellOskDialogCallbackReturnParam outputInfo;
			outputInfo.result = CELL_OSKDIALOG_INPUT_FIELD_RESULT_OK;
			outputInfo.numCharsResultString = CELL_OSKDIALOG_STRING_SIZE;
			outputInfo.pResultString = mResultTextBuffer;

			int ret = cellOskDialogUnloadAsync( &outputInfo );

			if ( ret != 0 )
				LOGERROR( g_Log, "cellOskDialogUnloadAsync() - ret %d", ret );
			else if  ( outputInfo.result == CELL_OSKDIALOG_INPUT_FIELD_RESULT_OK )
			{
				// copy over string
				for ( S32 i = 0; i < mNumCharacters; i++ )
					mResultText[ i ] = mResultTextBuffer[ i ];
				
				mResultText[ mNumCharacters ] = 0;

				ValidateKeyboardString();

				_UI_MESSAGE( E_UiMsg_KeyboardFinished );
			}
			else
			{
				_UI_MESSAGE( E_UiMsg_KeyboardCancelled );
			}
		}
	}
#endif
}

#ifdef PLATFORM_PS3
static void sysutil_callback( uint64_t status, uint64_t param, void *userdata )
{
	switch ( status )
	{
		case CELL_SYSUTIL_OSKDIALOG_FINISHED:
			gDashboardInterface.SetKeyboardFinished();
			break;

		case CELL_SYSUTIL_OSKDIALOG_INPUT_CANCELED:
			gDashboardInterface.SetKeyboardFinished();
			gDashboardInterface.SetKeyboardCancelled();
			break;

		default:
			break;
	}
}
#endif

bool C_DashboardInterface::StartPs3KeyboardUI( const WCHAR* pDefaultText, const WCHAR* pTitleText, S32 sNumCharacters )
{
	if ( !mInitialised )
		return false;

	// PS3 doesn't include the NULL character - just to be different
	sNumCharacters--;

#ifdef PLATFORM_PS3
	int ret = 0;

	// initial setup guff
	ret = cellOskDialogSetKeyLayoutOption( CELL_OSKDIALOG_10KEY_PANEL | CELL_OSKDIALOG_FULLKEY_PANEL );

	if ( ret != 0 )
	{
		LOGERROR( g_Log, "StartPs3KeyboardUI() - cellOskDialogSetKeyLayoutOption() - ret %d", ret );
		return false;
	}

	ret = cellOskDialogSetInitialKeyLayout( CELL_OSKDIALOG_INITIAL_PANEL_LAYOUT_SYSTEM );

	if ( ret != 0 )
	{
		LOGERROR( g_Log, "StartPs3KeyboardUI() - cellOskDialogSetInitialKeyLayout() - ret %d", ret );
		return false;
	}

	int32_t layoutMode = CELL_OSKDIALOG_LAYOUTMODE_X_ALIGN_CENTER | CELL_OSKDIALOG_LAYOUTMODE_Y_ALIGN_CENTER;
	ret = cellOskDialogSetLayoutMode( layoutMode );

	if ( ret != 0 )
	{
		LOGERROR( g_Log, "StartPs3KeyboardUI() - cellOskDialogSetLayoutMode() - ret %d", ret );
		return false;
	}

	CellOskDialogPoint controlPoint;
	controlPoint.x = 0.0f;
	controlPoint.y = 0.0f;

	CellOskDialogParam dialogParam;
	dialogParam.allowOskPanelFlg = CELL_OSKDIALOG_PANELMODE_ALPHABET;	// TODO - input flags for this
	dialogParam.firstViewPanel = CELL_OSKDIALOG_PANELMODE_ALPHABET;		// TODO - input flags for this
	dialogParam.controlPoint = controlPoint;
	dialogParam.prohibitFlgs = CELL_OSKDIALOG_NO_RETURN;	// TODO - input flags for this

	CellOskDialogInputFieldInfo inputFieldInfo;
	inputFieldInfo.message = (uint16_t*)pTitleText;
	inputFieldInfo.init_text = (uint16_t*)pDefaultText;
	inputFieldInfo.limit_length = sNumCharacters;
	
	// start the keyboard
	ret = cellOskDialogLoadAsync( SYS_MEMORY_CONTAINER_ID_INVALID, &dialogParam, &inputFieldInfo );

	if ( ret != 0 )
	{
		LOGERROR( g_Log, "StartPs3KeyboardUI() - cellOskDialogLoadAsync() - ret %d", ret );
		return false;
	}

	mKeyboardActive = true;
	mKeyboardCancelled = false;
	mKeyboardFinished = false;

	mNumCharacters = sNumCharacters;

	return true;
#endif

	return false;
}


void C_DashboardInterface::ValidateKeyboardString()
{
	for ( S32 i = 0; i < (S32)wcslen( mResultText ); i++ )
		if ( mResultText[ i ] == L'~' || mResultText[ i ] == L'%' )
			mResultText[ i ] = C_Font::s_NullCharacter;
}

}; // namespace UI
