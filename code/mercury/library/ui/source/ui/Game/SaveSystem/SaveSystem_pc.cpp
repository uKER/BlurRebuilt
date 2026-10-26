#include "stdafx.h"

#include "ui/Game/Profiles/ProfileInterface.h"
#include "ui/Game/SaveSystem/SaveSystem.h"

#include "ui/UiInterface.h"
#include "ui/GameSaveSystem.h"
#include "ui/Graphics/Localisation/loc.h"

#include <system/misc/StringUtil.h>

#include <shlobj.h>		// [Blur2PC] T250: SHGetFolderPathA (save location)

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

// [Blur2PC] T250: retail Blur PC 1.2 (0x004fc4a0, used by all four save threads) keeps saves under
// "%APPDATA%\Bizarre Creations\Blur\" (created if missing) followed by the interface's save root, and builds each
// path with "%s%s\%s" (root, save root, file name). BLUR2PC_SAVE_ROOT replaces that folder for scripted runs so
// they never touch the player's saves; not in retail.
static void GetUserDataRoot( char* pRoot, const size_t uSize )
{
	if( const char* pOverride = getenv( "BLUR2PC_SAVE_ROOT" ) )
	{
		sprintf_s( pRoot, uSize, "%s\\", pOverride );
	}
	else
	{
		char appData[ MAX_PATH ] = { 0 };
		SHGetFolderPathA( NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, appData );
		sprintf_s( pRoot, uSize, "%s\\Bizarre Creations\\Blur\\", appData );
	}

	if ( !Sys::C_FileSystem::Instance().FileExists( pRoot ) )
		Sys::C_FileSystem::Instance().CreateDirectory( pRoot );
}

// -----------------
// C_AsyncLoadThread
// -----------------
void C_AsyncLoadThread::ThreadMain()
{
	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	if ( !pSaveInfoInterface )
		return;

	if ( ( pSaveInfoInterface->m_eType == E_UiSaveInfoType_Enumerate ) && ( pSaveInfoInterface->m_sEnumeration == -1 ) )
	{
		C_SaveSystem::Instance().m_sEnumerationCount = 0;	// reset

		Sys::C_DirectoryInfo dirInfo;

		// [Blur2PC] T250: retail 0x0075dc24
		char enumPath[ MAX_PATH ] = { 0 };
		GetUserDataRoot( enumPath, MAX_PATH );
		strcat_s( enumPath, MAX_PATH, pSaveInfoInterface->m_SaveRoot.Chars() );

		bool bSuccess = Sys::C_FileSystem::Instance().DirectoryInfo( enumPath, &dirInfo );

		if ( !bSuccess )
		{
			m_eErrorCode = 	E_UiGameProfile_Error_CannotEnumerateFiles;
		}
		else
		{
			for ( U32 i = 0; i < dirInfo.NumFiles(); i++ )
			{
				Sys::C_FileInfo& fileInfo = dirInfo.FileInfo( i );
				
				if ( fileInfo.m_FileType != Sys::E_FileType_File )
					continue;

				// check file types
				if ( Sys::ExtractFileExtension( fileInfo.m_FileName ) != Util::C_String( pSaveInfoInterface->m_EnumerationFileExtension.Chars() ) )
					continue;

				Sys::C_FilePath s = Sys::ExtractFilename( fileInfo.m_FileName );

				if ( s.Length() >= MAX_SAVE_FILENAME_SIZE )
					continue;

				// found one
				strcpy_s( C_SaveSystem::Instance().m_EnumeratedFileInfo[ C_SaveSystem::Instance().m_sEnumerationCount++ ].m_FileName, MAX_SAVE_FILENAME_SIZE, s.Chars() );
			}
		}
	}
	else
	{
		char loadPath[ MAX_PATH ] = { 0 };
		char userRoot[ MAX_PATH ] = { 0 };
		GetUserDataRoot( userRoot, MAX_PATH );	// [Blur2PC] T250: retail 0x0075de21

		S32 sNumFiles = ( pSaveInfoInterface->m_eType == E_UiSaveInfoType_Enumerate ) ? 1 : pSaveInfoInterface->m_FileInfo.Size();

		for ( S32 i = 0; i < sNumFiles; i++ )
		{
			if ( pSaveInfoInterface->m_eType == E_UiSaveInfoType_Enumerate )
				sprintf_s( loadPath, MAX_PATH, "%s%s\\%s", userRoot, pSaveInfoInterface->m_SaveRoot.Chars(), C_SaveSystem::Instance().m_EnumeratedFileInfo[ pSaveInfoInterface->m_sEnumeration ].m_FileName );
			else
				sprintf_s( loadPath, MAX_PATH, "%s%s\\%s", userRoot, pSaveInfoInterface->m_SaveRoot.Chars(), pSaveInfoInterface->m_FileInfo[ i ].m_FileName.Chars() );

			if ( !Sys::C_FileSystem::Instance().FileExists( loadPath ) )
			{
				m_eErrorCode = E_UiGameProfile_Error_FileDoesNotExist;
			}
			else
			{
				Sys::C_OpenParams params;
				params.m_IsCritical = false;

				// open save file
				Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( loadPath, Sys::E_FileAccess_Read, params );

				if ( !pFile )
				{
					// failed to open the file
					m_eErrorCode = E_UiGameProfile_Error_CannotReadFile;
				}
				else
				{
					const U32 uBufferSize = (U32)pFile->Length(); //pSaveInfoInterface->m_FileInfo[ i ].m_uFileSize;

					// read and process
					U8* pBuffer = new U8[ uBufferSize ];	// use fixed data size
					memset( pBuffer, 0xfd, uBufferSize );	// debug clear

					bool bReadSucceeded = pFile->Read( pBuffer, uBufferSize );

					if ( bReadSucceeded )		
						bReadSucceeded = pSaveInfoInterface->Load( m_sPadNo, pBuffer, uBufferSize, pSaveInfoInterface->m_FileInfo[ i ].m_sUserData );	// load in

					SAFE_DELETE_PTR( pFile );
					SAFE_DELETE_ARY( pBuffer );

					if ( !bReadSucceeded )	// failed to read in the variables - out-of-date file
						m_eErrorCode = E_UiGameProfile_Error_CannotReadFile;
					else
						m_eErrorCode = E_UiGameProfile_Error_Okay;
				}
			}

			// check for an optional file
			if ( ( m_eErrorCode == E_UiGameProfile_Error_FileDoesNotExist ) && pSaveInfoInterface->m_FileInfo[ i ].m_bOptional )
				m_eErrorCode = E_UiGameProfile_Error_Okay;

			if ( m_eErrorCode != E_UiGameProfile_Error_Okay )
				break;
		}
	}

	pSaveInfoInterface->LoadFinished();

#ifndef CFG_GOLD
	// force error code
	if ( g_SaveSystemForceErrorCode != E_UiGameProfile_Error_None )
	{
		if ( ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_Okay )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_FileDoesNotExist )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_CannotReadFile )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_CannotEnumerateFiles ) )
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

	if ( !pSaveInfoInterface )
		return;

	// TODO - free space checks?

	char savePath[ MAX_PATH ] = { 0 };
	char userRoot[ MAX_PATH ] = { 0 };
	GetUserDataRoot( userRoot, MAX_PATH );	// [Blur2PC] T250: retail 0x0075e0c0
	sprintf_s( savePath, MAX_PATH, "%s%s", userRoot, pSaveInfoInterface->m_SaveRoot.Chars() );

	// Create the directory if it doesnt exist.
	if ( !Sys::C_FileSystem::Instance().FileExists( savePath ) )
	{
		Sys::C_FileSystem::Instance().CreateDirectory( savePath );
	}
	else
	{
		for ( S32 i = 0; i < pSaveInfoInterface->m_FileInfo.Size(); i++ )
		{
			// open or create the save file.
			sprintf_s( savePath, MAX_PATH, "%s%s\\%s", userRoot, pSaveInfoInterface->m_SaveRoot.Chars(), pSaveInfoInterface->m_FileInfo[ i ].m_FileName.Chars() );

			Sys::C_OpenParams params;
			params.m_IsCritical = false;

			Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( savePath, Sys::E_FileAccess_Write, params );

			if ( !pFile )
			{
				// failed to open the file
				m_eErrorCode = E_UiGameProfile_Error_CannotWriteFile;
			}
			else
			{
				if ( pSaveInfoInterface->m_bCheckOverwrites )
				{
					m_eErrorCode = E_UiGameProfile_Error_FileExists;
				}
				else
				{
					const U32 uBufferSize = pSaveInfoInterface->m_FileInfo[ i ].m_uFileSize;

					// process and write
					U8* pBuffer = new U8[ uBufferSize ];	// use fixed data size
					memset( pBuffer, 0xfe, uBufferSize );	// debug clear

					// save out
					if ( pSaveInfoInterface->Save( m_sPadNo, pBuffer, uBufferSize, pSaveInfoInterface->m_FileInfo[ i ].m_sUserData ) )
					{
						if ( !pFile->Write( pBuffer, uBufferSize ) )
							m_eErrorCode = E_UiGameProfile_Error_CannotWriteFile;

						pFile->Flush();	
					}
					else
						m_eErrorCode = E_UiGameProfile_Error_CannotWriteFile;			

					SAFE_DELETE_PTR( pFile );
					SAFE_DELETE_ARY( pBuffer );
				}
			}

			if ( m_eErrorCode != E_UiGameProfile_Error_Okay )
				break;
		}
	}

	pSaveInfoInterface->SaveFinished();

#ifndef CFG_GOLD
	// force error code
	if ( g_SaveSystemForceErrorCode != E_UiGameProfile_Error_None )
	{
		if ( ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_Okay )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_FileExists )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_CannotWriteFile ) )
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

	if ( !pSaveInfoInterface )
		return;

	char loadPath[ MAX_PATH ] = { 0 };
	char userRoot[ MAX_PATH ] = { 0 };
	GetUserDataRoot( userRoot, MAX_PATH );	// [Blur2PC] T250: retail 0x0075e37f

	for ( S32 i = 0; i < pSaveInfoInterface->m_FileInfo.Size(); i++ )
	{
		sprintf_s( loadPath, MAX_PATH, "%s%s\\%s", userRoot, pSaveInfoInterface->m_SaveRoot.Chars(), pSaveInfoInterface->m_FileInfo[ i ].m_FileName.Chars() );

		// if save file exists, delete it.
		if ( Sys::C_FileSystem::Instance().FileExists( loadPath ) )
		{
			if ( Sys::C_FileSystem::Instance().DeleteFile( loadPath ) )
				m_eErrorCode = E_UiGameProfile_Error_Okay;
			else
				m_eErrorCode = E_UiGameProfile_Error_CannotDeleteFile;
		}

		if ( m_eErrorCode != E_UiGameProfile_Error_Okay )
			break;
	}

	pSaveInfoInterface->DeleteFinished();

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

// ------------
// C_SaveSystem
// ------------
S32 C_SaveSystem::GetCurrentSaveSize()
{
	return 0;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
