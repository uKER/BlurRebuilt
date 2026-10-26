#include "Stdafx.h"

#include "ui/Game/Profiles/ProfileInterface.h"
#include "ui/Game/SaveSystem/SaveSystem.h"

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
	DE_OPTION( UI::E_UiGameProfile_Error_CannotReadFile,		"CannotReadFile" )
	DE_OPTION( UI::E_UiGameProfile_Error_CannotWriteFile,		"CannotWriteFile" )
	DE_OPTION( UI::E_UiGameProfile_Error_CannotDeleteFile,		"CannotDeleteFile" )
	DE_OPTION( UI::E_UiGameProfile_Error_DeviceNotConnected,	"DeviceNotConnected" )
	DE_OPTION( UI::E_UiGameProfile_Error_NoFreeSpace,			"NoFreeSpace" )
	DE_OPTION( UI::E_UiGameProfile_Error_UserCancelled,			"UserCancelled" )
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
		//LOGERROR( UI::g_Log, DBSTR "Pad index ( %d ) out of range", sPadNo );
		return;
	}

	m_sPadNo = sPadNo;
	m_eErrorCode = E_UiGameProfile_Error_Okay;
	
	Start( 64 * 1024 );	// default stack size and normal priority
}

// -----------------
// C_AsyncSaveThread
// -----------------
void C_AsyncSaveThread::Enter( const S32 sPadNo )
{
	if ( sPadNo < 0 || sPadNo >= MAX_USERS )
	{
		//LOGERROR( UI::g_Log, DBSTR "Pad index ( %d ) out of range", sPadNo );
		return;
	}

	m_sPadNo = sPadNo;
	m_eErrorCode = E_UiGameProfile_Error_Okay;
	
	Start( 64 * 1024 );	// default stack size and normal priority
}

// -------------------
// C_AsyncDeleteThread
// -------------------
void C_AsyncDeleteThread::Enter( const S32 sPadNo )
{
	if ( sPadNo < 0 || sPadNo >= MAX_USERS )
	{
		//LOGERROR( UI::g_Log, DBSTR "Pad index ( %d ) out of range", sPadNo );
		return;
	}

	m_sPadNo = sPadNo;
	m_eErrorCode = E_UiGameProfile_Error_Okay;
	
	Start( 64 * 1024 );	// default stack size and normal priority
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
		//LOGERROR( g_Log, DBSTR "StartAsyncLoad - can't load while the load thread is still active" );
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
		//LOGERROR( g_Log, DBSTR "StartAsyncSave - can't save while the save thread is still active" );
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

bool C_SaveSystem::StartAsyncDelete( S32 sPadNo )
{
	bool bStarted = true;
  
	if ( !m_AsyncDeleteThread.IsRunning() )
	{
		m_AsyncDeleteThread.Enter( sPadNo );
	}
	else
	{
		//LOGERROR( g_Log, DBSTR "StartAsyncDelete - can't delete while the delete thread is still active" );
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

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
