#include "Stdafx.h"

#include "ui/Game/Profiles/ProfileInterface.h"
#include "ui/Game/SaveSystem/SaveSystem.h"
#include "ui/GameSaveSystem.h"

#include <system/misc/StringUtil.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

#ifndef CFG_GOLD
// debug enum to force a specific load/save error
DEBUG_ENUM( UI::E_UiGameProfile_Error )
	DE_OPTION( UI::E_UiGameProfile_Error_None,					"None" )
	DE_OPTION( UI::E_UiGameProfile_Error_Okay,					"Okay" )
	DE_OPTION( UI::E_UiGameProfile_Error_FileDoesNotExist,		"FileDoesNotExist" )
	DE_OPTION( UI::E_UiGameProfile_Error_FileExists,			"FileExists" )
	DE_OPTION( UI::E_UiGameProfile_Error_CannotEnumerateFiles,	"CannotEnumerateFiles" )
	DE_OPTION( UI::E_UiGameProfile_Error_CannotReadFile,		"CannotReadFile" )
	DE_OPTION( UI::E_UiGameProfile_Error_CannotWriteFile,		"CannotWriteFile" )
	DE_OPTION( UI::E_UiGameProfile_Error_CannotDeleteFile,		"CannotDeleteFile" )
	DE_OPTION( UI::E_UiGameProfile_Error_DeviceNotConnected,	"DeviceNotConnected" )
	DE_OPTION( UI::E_UiGameProfile_Error_NoFreeSpace,			"NoFreeSpace" )
	DE_OPTION( UI::E_UiGameProfile_Error_UserCancelled,			"UserCancelled" )
	DE_OPTION( UI::E_UiGameProfile_Error_DifferentOwner,		"DifferentOwner" )
END_DEBUG_ENUM

Debug::Enum< UI::E_UiGameProfile_Error >	g_SaveSystemForceErrorCode( "UI.Profiles.ForceErrorCode", UI::E_UiGameProfile_Error_None );
#endif

namespace UI
{

// -----------------
// C_AsyncLoadThread
// -----------------
void C_AsyncLoadThread::Enter( const S32 sPadNo )
{
	if ( sPadNo < 0 || sPadNo >= MAX_USERS )
	{
		LOGERROR( UI::g_Log, DBSTR "Pad index ( %d ) out of range", sPadNo );
		return;
	}

	m_sPadNo = sPadNo;
	m_eErrorCode = E_UiGameProfile_Error_Okay;
	
	Start( 64 * 1024, 0, "UI::C_AsyncLoadThread" ); // default stack size and normal priority
}

// -----------------
// C_AsyncSaveThread
// -----------------
void C_AsyncSaveThread::Enter( const S32 sPadNo )
{
	if ( sPadNo < 0 || sPadNo >= MAX_USERS )
	{
		LOGERROR( UI::g_Log, DBSTR "Pad index ( %d ) out of range", sPadNo );
		return;
	}

	m_sPadNo = sPadNo;
	m_eErrorCode = E_UiGameProfile_Error_Okay;
	
	Start( 64 * 1024, 0, "UI::C_AsyncSaveThread" ); // default stack size and normal priority
}

// -------------------
// C_AsyncDeleteThread
// -------------------
#ifdef PLATFORM_PS3
void C_AsyncDeleteThread::Enter( const S32 sPadNo, const bool bDeleteFromList )
#else
void C_AsyncDeleteThread::Enter( const S32 sPadNo )
#endif
{
	if ( sPadNo < 0 || sPadNo >= MAX_USERS )
	{
		LOGERROR( UI::g_Log, DBSTR "Pad index ( %d ) out of range", sPadNo );
		return;
	}

	m_sPadNo = sPadNo;
#ifdef PLATFORM_PS3
	m_bDeleteFromList = bDeleteFromList;
#endif
	m_eErrorCode = E_UiGameProfile_Error_Okay;
	
	Start( 64 * 1024, 0, "UI::C_AsyncDeleteThread" ); // default stack size and normal priority
}

// global instance of the save system
C_SaveSystem C_SaveSystem::m_sInstance;

// ------------
// C_SaveSystem
// ------------
bool C_SaveSystem::StartAsyncLoad( S32 sPadNo )
{
	bool bStarted = true;
  
	if ( !m_AsyncLoadThread.IsRunning() )
	{
		m_AsyncLoadThread.Enter( sPadNo );
	}
	else
	{
		LOGERROR( g_Log, DBSTR "StartAsyncLoad - can't load while the load thread is still active" );
		bStarted = false;
	}

	return bStarted;
}

bool C_SaveSystem::ContinueAsyncLoad()
{
	if ( !m_AsyncLoadThread.IsRunning() )
	{
		m_AsyncLoadThread.Join();	// wait for thread to terminate

#ifndef PLATFORM_PS3	// PS3 is moaning - linker error
		m_AsyncLoadThread.Close();	// close thread handle
#endif
		
		return true;
	}

	return false;
}

bool C_SaveSystem::StartAsyncSave( S32 sPadNo )
{
	bool bStarted = true;
  
	if ( !m_AsyncSaveThread.IsRunning() )
	{
		m_AsyncSaveThread.Enter( sPadNo );
	}
	else
	{
		LOGERROR( g_Log, DBSTR "StartAsyncSave - can't save while the save thread is still active" );
		bStarted = false;
	}

	return bStarted;
}

bool C_SaveSystem::ContinueAsyncSave()
{
	if ( !m_AsyncSaveThread.IsRunning() )
	{
		m_AsyncSaveThread.Join();	// wait for thread to terminate

#ifndef PLATFORM_PS3	// PS3 is moaning - linker error
		m_AsyncSaveThread.Close();	// close thread handle
#endif
		
		return true;
	}

	return false;
}

#ifdef PLATFORM_PS3
bool C_SaveSystem::StartAsyncDelete( S32 sPadNo, bool bDeleteFromList )
#else
bool C_SaveSystem::StartAsyncDelete( S32 sPadNo )
#endif
{
	bool bStarted = true;
  
	if ( !m_AsyncDeleteThread.IsRunning() )
	{
	#ifdef PLATFORM_PS3
		m_AsyncDeleteThread.Enter( sPadNo, bDeleteFromList );
	#else
		m_AsyncDeleteThread.Enter( sPadNo );
	#endif
	}
	else
	{
		LOGERROR( g_Log, DBSTR "StartAsyncDelete - can't delete while the delete thread is still active" );
		bStarted = false;
	}

	return bStarted;
}

bool C_SaveSystem::ContinueAsyncDelete()
{
	if ( !m_AsyncDeleteThread.IsRunning() )
	{
		m_AsyncDeleteThread.Join();	// wait for thread to terminate

#ifndef PLATFORM_PS3	// PS3 is moaning - linker error
		m_AsyncDeleteThread.Close();	// close thread handle
#endif
		
		return true;
	}

	return false;
}

S32 C_SaveSystem::GetEnumerationCount()
{
#if PLATFORM_PS3
	return 0;	// handled with sysutil
#elif PLATFORM_PC
	return m_sEnumerationCount;
#else
	return 0;
#endif
}

const WCHAR* C_SaveSystem::GetEnumerationName( const S32 sIndex )
{
	if ( sIndex < 0 || sIndex > ( GetEnumerationCount() - 1 ) )
		return L"";

#if PLATFORM_PS3
	return L"";	// handled with sysutil
#elif PLATFORM_PC
	static C_WideString<MAX_SAVE_FILENAME_SIZE> s;
	s.Format( L"%S", Sys::RemoveFileExtension( m_EnumeratedFileInfo[ sIndex ].m_FileName ).Chars() );
	return s.Chars();
#else
	return L"";
#endif
}

bool C_SaveSystem::IsValidEnumerationFileType( const S32 sIndex )
{
	if ( sIndex < 0 || sIndex > ( GetEnumerationCount() - 1 ) )
		return false;

	if ( !SaveInfoInterface() )
		return false;

#if PLATFORM_PS3
	return true;	// handled with sysutil
#elif PLATFORM_PC
	return ( Sys::ExtractFileExtension( Util::C_String( m_EnumeratedFileInfo[ sIndex ].m_FileName ) ) == SaveInfoInterface()->m_EnumerationFileExtension );
#else
	return false;
#endif
}

// -------------------
// C_SaveInfoInterface
// -------------------
void C_SaveInfoInterface::Reset()
{
	m_eType = E_UiSaveInfoType_Default;
	m_sEnumeration = -1;
	m_FileInfo.Clear();
#if PLATFORM_PC
	m_bCheckOverwrites = false;
	m_EnumerationFileExtension.Clear();
#endif
#ifdef PLATFORM_PS3
	m_TitleId.Clear();
	m_FilePath.Clear();
	m_uSizeIcon0 = 0;
	m_uSizeIcon1 = 0;
	m_uSizePic1 = 0;
	m_uSizeSnd0 = 0;
	m_ListName.Clear();
	m_pParamSFO_Title[ 0 ]		= 0;
	m_pParamSFO_Detail[ 0 ]		= 0;
	m_pSubtitle[ 0 ]			= 0;
	m_pListNewDataTitle[ 0 ]	= 0;
	memset( m_pSecureFieldId, 0, CELL_SAVEDATA_SECUREFILEID_SIZE );
#endif
}

#ifdef PLATFORM_PS3
static void UCS2stoUTF8s_ErrorCheck( const L10nResult& res )
{
	switch ( res )
	{
		case ConversionOK:
			break;
		case SRCIllegal:
			LOGERROR( g_Log, "UCS2stoUTF8s() - SRCIllegal - Target character string is corrupt" );
			break;
		case DSTExhausted:
			LOGERROR( g_Log, "UCS2stoUTF8s() - DSTExhausted - Buffer is too small for the conversion result" );
			break;
		case ConverterUnknown:
			LOGERROR( g_Log, "UCS2stoUTF8s() - ConverterUnknown - Specified converter is invalid" );
			break;
		default:
			break;
	}
}

void C_SaveInfoInterface::SetParamSFO_Title( const WCHAR* pParamSFO_Title )
{
	memset( m_pParamSFO_Title, 0, CELL_SAVEDATA_SYSP_TITLE_SIZE );

	WCHAR t[ CELL_SAVEDATA_SYSP_TITLE_SIZE ];
	memset( t, 0, CELL_SAVEDATA_SYSP_TITLE_SIZE );
	wcscpy( t, pParamSFO_Title );

	size_t wlen = wcslen( t );
	size_t len = CELL_SAVEDATA_SYSP_TITLE_SIZE;

	L10nResult res = UCS2stoUTF8s( (uint16_t*)t, &wlen, (uint8_t*)m_pParamSFO_Title, &len );
	UCS2stoUTF8s_ErrorCheck( res );
}

void C_SaveInfoInterface::SetParamSFO_Detail( const WCHAR* pParamSFO_Detail )
{
	memset( m_pParamSFO_Detail, 0, CELL_SAVEDATA_SYSP_DETAIL_SIZE );

	WCHAR t[ CELL_SAVEDATA_SYSP_DETAIL_SIZE ];
	memset( t, 0, CELL_SAVEDATA_SYSP_DETAIL_SIZE );
	wcscpy( t, pParamSFO_Detail );

	size_t wlen = wcslen( t );
	size_t len  = CELL_SAVEDATA_SYSP_DETAIL_SIZE;

	L10nResult res = UCS2stoUTF8s( (uint16_t*)t, &wlen, (uint8_t*)m_pParamSFO_Detail, &len );
	UCS2stoUTF8s_ErrorCheck( res );
}

void C_SaveInfoInterface::SetSubtitle( const WCHAR* pSubtitle, const bool bCopyInternal )
{
	if ( bCopyInternal )
		m_Subtitle = pSubtitle;

	memset( m_pSubtitle, 0, CELL_SAVEDATA_SYSP_TITLE_SIZE );

	WCHAR t[ CELL_SAVEDATA_SYSP_TITLE_SIZE ];
	memset( t, 0, CELL_SAVEDATA_SYSP_TITLE_SIZE );
	wcscpy( t, pSubtitle );

	size_t wlen = wcslen( t );
	size_t len = CELL_SAVEDATA_SYSP_TITLE_SIZE;

	L10nResult res = UCS2stoUTF8s( (uint16_t*)t, &wlen, (uint8_t*)m_pSubtitle, &len );
	UCS2stoUTF8s_ErrorCheck( res );
}

void C_SaveInfoInterface::SetListNewDataTitle( const WCHAR* pListNewDataTitle )
{
	memset( m_pListNewDataTitle, 0, CELL_SAVEDATA_SYSP_TITLE_SIZE );

	WCHAR t[ CELL_SAVEDATA_SYSP_TITLE_SIZE ];
	memset( t, 0, CELL_SAVEDATA_SYSP_TITLE_SIZE );
	wcscpy( t, pListNewDataTitle );

	size_t wlen = wcslen( t );
	size_t len = CELL_SAVEDATA_SYSP_TITLE_SIZE;

	L10nResult res = UCS2stoUTF8s( (uint16_t*)t, &wlen, (uint8_t*)m_pListNewDataTitle, &len );
	UCS2stoUTF8s_ErrorCheck( res );
}

void C_SaveInfoInterface::SetSecureFieldId( const char* pSecureFieldId )
{
	memset( m_pSecureFieldId, 0, CELL_SAVEDATA_SECUREFILEID_SIZE );

	size_t len = strlen( pSecureFieldId );

	if ( len > CELL_SAVEDATA_SECUREFILEID_SIZE )
	{
		LOGERROR( g_Log, "SetSecureFieldId() - string too long" );
		return;
	}

	memcpy( m_pSecureFieldId, pSecureFieldId, CELL_SAVEDATA_SECUREFILEID_SIZE );
	
	bool bFoundNull = false;

	// check for null characters - we're not allowed them
	for ( S32 i = 0; i < CELL_SAVEDATA_SECUREFILEID_SIZE; i++ )
	{
		if ( m_pSecureFieldId[ i ] == 0 )
		{
			m_pSecureFieldId[ i ] = '_';
			bFoundNull = true;
		}
	}

	if ( bFoundNull )
		LOGWARN( g_Log, "SetSecureFieldId() - contains null characters - these aren't allowed - I'm replacing them with underscores" );
}
#endif

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
