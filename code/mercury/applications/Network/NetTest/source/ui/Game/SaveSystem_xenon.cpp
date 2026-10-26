#include "stdafx.h"

#include "ui/Game/Profiles/ProfileInterface.h"
#include "ui/Game/SaveSystem/SaveSystem.h"

#include "ui/UiInterface.h"
#include "ui/GameSaveSystem.h"
#include "ui/Graphics/Localisation/loc.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

// -----------------
// C_AsyncLoadThread
// -----------------
void C_AsyncLoadThread::ThreadMain()
{
	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	gGameSaveSystem.SetUserIndex( C_ProfileInterface::Instance().GetPadProfile( m_sPadNo ).m_UserIndex );
	gGameSaveSystem.SetSaveRoot( pSaveInfoInterface->SaveRoot() );

	bool bSuccess = gGameSaveSystem.LoadSave( pSaveInfoInterface->FileName(), pSaveInfoInterface->DisplayName() );

	// store away the device id
	C_ProfileInterface::Instance().DeviceId( m_sPadNo, gGameSaveSystem.mDeviceID );

	if ( !bSuccess )
	{
		// failed for some reason - check the error code and close
		S32 sError = gGameSaveSystem.GetLastError();

		if ( sError == ERROR_FILE_CORRUPT )
			m_eErrorCode = E_UiGameProfile_Error_CannotReadFile;
		else if ( sError == ERROR_DEVICE_NOT_CONNECTED )
			m_eErrorCode = E_UiGameProfile_Error_DeviceNotConnected;
		else
			m_eErrorCode = E_UiGameProfile_Error_FileDoesNotExist;

		gGameSaveSystem.CloseSave();
	}
	else
	{
		Sys::I_File* pFile = UiInterface()->GetFileSys()->Open( pSaveInfoInterface->InternalName(), Sys::E_FileAccess_Read );

		if ( !pFile )
		{
			// failed to open the file
			gGameSaveSystem.CloseSave();
			m_eErrorCode = E_UiGameProfile_Error_CannotReadFile;
		}
		else
		{
			const U32 uBufferSize = pSaveInfoInterface->BufferSize();

			// read and process
			U8* pBuffer = new U8[ uBufferSize ];	// use fixed data size
			memset( pBuffer, 0xfd, uBufferSize );	// debug clear

			pFile->Read( pBuffer, uBufferSize );

			// load in
			bool bReadSucceeded = pSaveInfoInterface->Load( m_sPadNo, pBuffer, uBufferSize );

			SAFE_DELETE_PTR( pFile );
			SAFE_DELETE_ARY( pBuffer );

			gGameSaveSystem.CloseSave();

			if ( !bReadSucceeded )	// failed to read in the variables - out-of-date file
				m_eErrorCode = E_UiGameProfile_Error_CannotReadFile;
			else
				m_eErrorCode = E_UiGameProfile_Error_Okay;
		}
	}

	// do a last check to see if people are daft enough to pull the MU!
	if( XContentGetDeviceState( C_ProfileInterface::Instance().DeviceId( m_sPadNo ), NULL ) == ERROR_DEVICE_NOT_CONNECTED )
		m_eErrorCode = E_UiGameProfile_Error_DeviceNotConnected;

#ifndef CFG_GOLD
	// force error code
	if ( g_SaveSystemForceErrorCode != E_UiGameProfile_Error_None )
	{
		if ( ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_Okay )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_FileDoesNotExist )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_CannotReadFile )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_DeviceNotConnected ) )
		{
			m_eErrorCode = g_SaveSystemForceErrorCode;
		}
	}
#endif
}

// -----------------
// C_AsyncSaveThread
// -----------------
void C_AsyncSaveThread::ThreadMain()
{
	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	gGameSaveSystem.SetUserIndex( C_ProfileInterface::Instance().GetPadProfile( m_sPadNo ).m_UserIndex );
	gGameSaveSystem.SetSaveRoot( pSaveInfoInterface->SaveRoot() );

	// set the device id
	gGameSaveSystem.mDeviceID = C_ProfileInterface::Instance().DeviceId( m_sPadNo );

	S32 sError = gGameSaveSystem.CreateSave( pSaveInfoInterface->FileName(), pSaveInfoInterface->DisplayName(), pSaveInfoInterface->Roamable(), pSaveInfoInterface->CheckOverwrites() );

	if ( sError != GAMESAVE_SYSTEM_OK )
	{
		// failed for some reason - check the error code and close
		if ( sError == GAMESAVE_SYSTEM_DEVICE_DISCONNECTED )
			m_eErrorCode = E_UiGameProfile_Error_DeviceNotConnected;
		if ( sError == GAMESAVE_SYSTEM_FILE_EXISTS )
			m_eErrorCode = E_UiGameProfile_Error_FileExists;
		else
			m_eErrorCode = E_UiGameProfile_Error_CannotWriteFile;

		gGameSaveSystem.CloseSave();
	}
	else
	{
		bool bWriteSucceeded = false;

		Sys::I_File* pFile = UiInterface()->GetFileSys()->Open( pSaveInfoInterface->InternalName(), Sys::E_FileAccess_Write );

		if ( !pFile )
		{
			// failed to open the file
			gGameSaveSystem.CloseSave();
			m_eErrorCode = E_UiGameProfile_Error_CannotWriteFile;
		}
		else
		{
			const U32 uBufferSize = pSaveInfoInterface->BufferSize();

			// process and write
			U8* pBuffer = new U8[ uBufferSize ];	// use fixed data size
			memset( pBuffer, 0xfe, uBufferSize );	// debug clear

			// save out
			if ( pSaveInfoInterface->Save( m_sPadNo, pBuffer, uBufferSize ) )
			{
				pFile->Write( pBuffer, uBufferSize );
				pFile->Flush();	
			}
			else
				m_eErrorCode = E_UiGameProfile_Error_CannotWriteFile;			

			SAFE_DELETE_PTR( pFile );
			SAFE_DELETE_ARY( pBuffer );

			gGameSaveSystem.Flush();
			gGameSaveSystem.CloseSave();
		}
	}

	// do a last check to see if people are daft enough to pull the MU!
	if( XContentGetDeviceState( C_ProfileInterface::Instance().DeviceId( m_sPadNo ), NULL ) == ERROR_DEVICE_NOT_CONNECTED )
		m_eErrorCode = E_UiGameProfile_Error_DeviceNotConnected;

#ifndef CFG_GOLD
	// force error code
	if ( g_SaveSystemForceErrorCode != E_UiGameProfile_Error_None )
	{
		if ( ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_Okay )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_FileExists )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_CannotWriteFile )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_DeviceNotConnected ) )
		{
			m_eErrorCode = g_SaveSystemForceErrorCode;
		}
	}
#endif
}

// -------------------
// C_AsyncDeleteThread
// -------------------
void C_AsyncDeleteThread::ThreadMain()
{
	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	gGameSaveSystem.SetUserIndex( C_ProfileInterface::Instance().GetPadProfile( m_sPadNo ).m_UserIndex );
	gGameSaveSystem.SetSaveRoot( pSaveInfoInterface->SaveRoot() );

	// set the device id
	gGameSaveSystem.mDeviceID = C_ProfileInterface::Instance().DeviceId( m_sPadNo );

	if ( gGameSaveSystem.DeleteSave( pSaveInfoInterface->FileName(), pSaveInfoInterface->DisplayName() ) )
		m_eErrorCode = E_UiGameProfile_Error_Okay;
	else
		m_eErrorCode = E_UiGameProfile_Error_CannotDeleteFile;

	gGameSaveSystem.CloseSave();	

#ifndef CFG_GOLD
	// force error code
	if ( g_SaveSystemForceErrorCode != E_UiGameProfile_Error_None )
	{
		if ( ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_Okay )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_CannotDeleteFile ) )
		{
			m_eErrorCode = g_SaveSystemForceErrorCode;
		}
	}
#endif
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
