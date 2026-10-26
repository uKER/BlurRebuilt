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

// helper function
bool FileAllocLoad( const char* pFilePath, U8*& pBuffer, U32& uFileSize )
{
	Sys::C_OpenParams params;
	params.m_IsCritical = false;

	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( pFilePath, Sys::E_FileAccess_Read, params );

	if ( !pFile )
	{
		LOGERROR( g_Log, "Memory save content file %s not found.", pFilePath );
		return false;
	}

	uFileSize = pFile->Length();

	// alloc file buffer
	pBuffer = new U8[ uFileSize ];

	if ( !pBuffer )
	{
		LOGERROR( g_Log, "Memory save content file %s out of memory allocating %d bytes.", pFilePath, uFileSize );
		return false;
	}

	// read file into buffer
	if ( !pFile->Read( pBuffer, uFileSize ) )
	{
		LOGERROR( g_Log, "Failed to read file %s", pFilePath );
		SAFE_DELETE_PTR( pFile );
		return false;
	}
	
	// done
	SAFE_DELETE_PTR( pFile );

	return true;
}

// globals

#if CELL_SDK_VERSION < 0x250000
#define _SAVE_USE_MEM_CONTAINER_
#endif

//
static bool						g_bInitialised = false;

#ifdef _SAVE_USE_MEM_CONTAINER_
static sys_memory_container_t	g_MemoryContainer;
static S32						g_MemoryContainerSizeAuto = 5 * 1024 * 1024; //3 * 1024 * 1024;	// needs to be 5MB to support deleting
#endif

static Btl::Vector< U8* >		g_Buffers;				// save/load buffer for each file - alloced/freed each time we save/load
static U8*						g_pFileBuffer = NULL;	// used for processing non-profile files for saving out

static char						g_ListNewDataTitle[ CELL_SAVEDATA_SYSP_TITLE_SIZE ];	// title for new data icon
static CellSaveDataListNewData	g_ListNewData;
static CellSaveDataNewDataIcon	g_ListNewDataIcon;
static char						g_ListNewDataDirectory[ CELL_SAVEDATA_SYSP_TITLE_SIZE ];
//

#define SIZE_KB( x )			( ( x + 1023 ) / 1024 )

static S32 g_SaveOperationStep = -1;		// nothing
static S32 g_SaveOperationStepCustom = -1;	// nothing
static S32 g_LoadOperationStep = -1;		// nothing

// note file sizes are changed to proper ones once we start
static Btl::Vector< C_SaveFileInfo, Btl::VectorParameters_Static< 4 > > g_FileList;

// note DIRNAME and SUBTITLE and PARAMSFO_DETAIL are changed to proper ones once we start
// - and the file list is filled out
static C_DataParam g_DataParam = 
{
	"DIRNAME", "SUBTITLE", "PARAMSFO_DETAIL", "CONFIG", /* m_FileList */
};

bool Initialise()
{
	// set things from the SaveInfoInterface
	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	if ( !pSaveInfoInterface )
		return false;

	// directory name
	if ( pSaveInfoInterface->m_eType == E_UiSaveInfoType_Enumerate )
		sprintf( g_DataParam.m_pDirName, "%s-LIST-%s-", pSaveInfoInterface->m_TitleId.Chars(), pSaveInfoInterface->m_ListName.Chars() );
	else
		sprintf( g_DataParam.m_pDirName, "%s-AUTO-", pSaveInfoInterface->m_TitleId.Chars() );

	sprintf( g_DataParam.m_pSubTitle, pSaveInfoInterface->m_pSubtitle );				// subtitle
	sprintf( g_DataParam.m_pDetail, pSaveInfoInterface->m_pParamSFO_Detail );		// detail info

	if ( !g_bInitialised )
	{
		// setup system files
		g_FileList.PushBack( C_SaveFileInfo( 0, "ICON0.PNG", false, -1, E_SaveFileType_Icon0 ) );
//		g_FileList.PushBack( C_SaveFileInfo( 0, "ICON1.PAM", false, -1, E_SaveFileType_Icon1 ) );
		g_FileList.PushBack( C_SaveFileInfo( 0, "PIC1.PNG",  false, -1, E_SaveFileType_Pic1  ) );
//		g_FileList.PushBack( C_SaveFileInfo( 0, "SND0.AT3",  false, -1, E_SaveFileType_Snd0  ) );
	}

	// file sizes
	g_FileList[ E_SaveFileType_Icon0 ].m_uFileSize = pSaveInfoInterface->m_uSizeIcon0;
//	g_FileList[ E_SaveFileType_Icon1 ].m_uFileSize = pSaveInfoInterface->m_uSizeIcon1;
	g_FileList[ E_SaveFileType_Pic1  ].m_uFileSize = pSaveInfoInterface->m_uSizePic1;
//	g_FileList[ E_SaveFileType_Snd0  ].m_uFileSize = pSaveInfoInterface->m_uSizeSnd0;

	// file size check - TRC
	if ( ( pSaveInfoInterface->m_uSizeIcon1 + pSaveInfoInterface->m_uSizeSnd0 ) > ( 3 * 1024 * 1024 ) )
	{
		LOGERROR( g_Log, DBSTR "m_uSizeIcon1 + m_uSizeSnd0 _must_ be less than 3MB - not using them ..." );

//		g_FileList[ E_SaveFileType_Icon1 ].m_uFileSize = 0;
//		g_FileList[ E_SaveFileType_Snd0  ].m_uFileSize = 0;
	}

	// build the file list
	g_DataParam.m_FileList.Clear();

	// system files
	for ( S32 i = 0; i < g_FileList.Size(); i++ )
		if ( g_FileList[ i ].m_uFileSize > 0 )
			g_DataParam.m_FileList.PushBack( g_FileList[ i ] );

	// custom data files
	for ( S32 i = 0; i < pSaveInfoInterface->m_FileInfo.Size(); i++ )
		g_DataParam.m_FileList.PushBack( pSaveInfoInterface->m_FileInfo[ i ] );

	if ( !g_bInitialised )
	{
	#ifdef _SAVE_USE_MEM_CONTAINER_
		// create memory container
		if ( BizContainerCreate( &g_MemoryContainer, g_MemoryContainerSizeAuto ) != 0 )
			LOGFATAL( g_Log, "Initialise - cannot create PS3 save system memory container!" );
		else
	#endif
			g_bInitialised = true;
	}

	return g_bInitialised;
}

// load callbacks
void DataListLoadCallback( CellSaveDataCBResult* pResult, CellSaveDataListGet* pGet, CellSaveDataListSet* pSet )
{
	if ( pGet->dirNum > pGet->dirListNum )
	{
		pResult->result = CELL_SAVEDATA_CBRESULT_ERR_FAILURE;	// wtf - how did this happen then?
		return;
	}

	pSet->fixedList = pGet->dirList;

	if ( pGet->dirListNum > CELL_SAVEDATA_LISTITEM_MAX )
		pSet->fixedListNum = CELL_SAVEDATA_LISTITEM_MAX;
	else
		pSet->fixedListNum = pGet->dirListNum;

	pSet->focusPosition = CELL_SAVEDATA_FOCUSPOS_LISTHEAD;
	pSet->focusDirName = NULL;
	pSet->reserved = NULL;	// have to set this to NULL

	pSet->newData = NULL;

	pResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void DataStatusLoadCallback( CellSaveDataCBResult* pResult, CellSaveDataStatGet* pGet, CellSaveDataStatSet* pSet )
{
	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	// TODO - this needs to be an option in the interface
	if ( ( pSaveInfoInterface->m_eType == E_UiSaveInfoType_Default ) && ( ( pGet->bind & CELL_SAVEDATA_BINDSTAT_ERR_NOOWNER ) || ( pGet->bind & CELL_SAVEDATA_BINDSTAT_ERR_OWNER ) ) )
	{
		// save was created either with no owner info or we've loaded a save created by another user
		pResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		C_SaveSystem::Instance().SetLoadErrorCode( E_UiGameProfile_Error_DifferentOwner );
		return;
	}

	// check for new data
	if ( pGet->isNewData == CELL_SAVEDATA_ISNEWDATA_YES )
	{
		// no file exists
		LOGINFO( g_Log, "DataStatusLoadCallback: No Data" );
		C_SaveSystem::Instance().SetLoadErrorCode( E_UiGameProfile_Error_FileDoesNotExist );
		pResult->result = CELL_SAVEDATA_CBRESULT_ERR_NODATA;
		return;
	}
	else
	{
		if ( pGet->fileListNum < pGet->fileNum )
		{
			pResult->result = CELL_SAVEDATA_CBRESULT_ERR_FAILURE;	// wtf - how did this happen then?
			return;
		}
	}

	// carry on
	pSet->reCreateMode = CELL_SAVEDATA_RECREATE_NO_NOBROKEN;
	pSet->setParam = NULL;

	pResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

	// try and find our files
	for ( S32 i = 0; i < pSaveInfoInterface->m_FileInfo.Size(); i++ )
	{
		S32 j = 0;

		for ( j = 0; j < pGet->fileListNum; j++ )
			if ( !strcmp( pSaveInfoInterface->m_FileInfo[ i ].m_FileName.Chars(), pGet->fileList[ j ].fileName ) )
				break;

		if ( j < pGet->fileListNum )
		{
			// file found - alloc load buffer
			S32 sFileSize = pGet->fileList[ j ].st_size;
			U8* pSaveFileBuffer = new U8[ sFileSize ];

			if ( !pSaveFileBuffer )
			{
				LOGFATAL( g_Log, "C_AsyncLoadThread::ThreadMain() - pSaveFileBuffer - cannot allocate!" );
				pResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
				C_SaveSystem::Instance().SetLoadErrorCode( E_UiGameProfile_Error_CannotReadFile );
				return;
			}
			else
			{
				memset( pSaveFileBuffer, 0xfe, sFileSize );	// debug clear
				g_Buffers[ i ] = pSaveFileBuffer;
				pSaveInfoInterface->m_FileInfo[ i ].m_uFileSize = (U32)sFileSize;
			}
		}
		else
		{
			// file not found
			if ( !pSaveInfoInterface->m_FileInfo[ i ].m_bOptional )
			{
				// required file - save is bust
				pResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
				C_SaveSystem::Instance().SetLoadErrorCode( E_UiGameProfile_Error_CannotReadFile );
				return;
			}
		}
	}

	g_LoadOperationStep = 0;
}

void FileOperationLoadCallback( CellSaveDataCBResult* pResult, CellSaveDataFileGet* pGet, CellSaveDataFileSet* pSet )
{
	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	// ok to go unless we set further down
	pResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

	// find the buffer to load into
	while ( ( g_LoadOperationStep < pSaveInfoInterface->m_FileInfo.Size() ) && !g_Buffers[ g_LoadOperationStep ] )
		g_LoadOperationStep++;

	if ( g_LoadOperationStep < pSaveInfoInterface->m_FileInfo.Size() )
	{
		// load in the custom data
		C_SaveFileInfo& saveFileInfo = pSaveInfoInterface->m_FileInfo[ g_LoadOperationStep ];
		
		pSet->fileOperation = CELL_SAVEDATA_FILEOP_READ;
		pSet->reserved = NULL;	// have to set this to NULL
		pSet->fileType = CELL_SAVEDATA_FILETYPE_SECUREFILE;
		memcpy( pSet->secureFileId, pSaveInfoInterface->m_pSecureFieldId, CELL_SAVEDATA_SECUREFILEID_SIZE );
		pSet->fileName = saveFileInfo.m_FileName.NonConstChars();
		pSet->fileOffset = 0;
		pSet->fileSize = saveFileInfo.m_uFileSize;
		pSet->fileBufSize = saveFileInfo.m_uFileSize;
		pSet->fileBuf = g_Buffers[ g_LoadOperationStep ];
	}
	else
	{
		// done
		pResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
	}

	pResult->progressBarInc = 100 / pSaveInfoInterface->m_FileInfo.Size();
	g_LoadOperationStep++;
}

// -----------------
// C_AsyncLoadThread
// -----------------
void C_AsyncLoadThread::ThreadMain()
{
	if ( !Initialise() )
		return;

	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	if ( g_Buffers.Size() != 0 )
	{
		LOGERROR( g_Log, "C_AsyncLoadThread::ThreadMain - PS3 buffers already exists - this shouldn't happen!" );

		for ( S32 i = 0; i < g_Buffers.Size(); i++ )
			SAFE_DELETE_ARY( g_Buffers[ i ] );

		g_Buffers.Clear();
	}
	
	// the load buffers are allocated in the callback once we've determined whether the file exists or not
	for ( S32 i = 0; i < pSaveInfoInterface->m_FileInfo.Size(); i++ )
		g_Buffers.PushBack( (U8*)NULL );

	int sErrorCode = CELL_SAVEDATA_RET_OK;

	if ( pSaveInfoInterface->m_eType == E_UiSaveInfoType_Enumerate )
	{
		// load from a list
		CellSaveDataSetList setList;
		setList.sortType = CELL_SAVEDATA_SORTTYPE_MODIFIEDTIME; // CELL_SAVEDATA_SORTTYPE_SUBTITLE;
		setList.sortOrder = CELL_SAVEDATA_SORTORDER_DESCENT; // CELL_SAVEDATA_SORTORDER_ASCENT;
		setList.dirNamePrefix = g_DataParam.m_pDirName;
		setList.reserved = NULL;	// have to set this to NULL

		CellSaveDataSetBuf setBuf;
		setBuf.dirListMax = MAX_SAVE_ENUMERATIONS;
		setBuf.fileListMax = g_DataParam.m_FileList.Size();
		memset( setBuf.reserved, 0, sizeof( setBuf.reserved ) );	// have to set this to 0
		setBuf.bufSize = Maths::Max<U32>( setBuf.dirListMax * sizeof( CellSaveDataDirList ), setBuf.fileListMax * sizeof( CellSaveDataFileStat ) );

		// alloc buffer
		U8* pBuffer = new U8[ setBuf.bufSize ];

		if ( !pBuffer )
		{
			LOGFATAL( g_Log, "C_AsyncSaveThread::ThreadMain() - pBuffer - cannot allocate!" );
		}
		else
		{
			setBuf.buf = pBuffer;

			// set off the load
			sErrorCode = cellSaveDataListLoad2(
				CELL_SAVEDATA_VERSION_CURRENT,
				&setList,
				&setBuf,
				DataListLoadCallback,
				DataStatusLoadCallback,
				FileOperationLoadCallback,
			#ifdef _SAVE_USE_MEM_CONTAINER_
				g_MemoryContainer,
			#else
				SYS_MEMORY_CONTAINER_ID_INVALID,
			#endif
				0	// no userdata - yet
			);

			// cleanup
			SAFE_DELETE_ARY( pBuffer );
		}
	}
	else
	{
		// it's the fixed auto-save
		CellSaveDataSetBuf setBuf = { 0 };
		setBuf.dirListMax = 1;
		setBuf.fileListMax = g_DataParam.m_FileList.Size();
		setBuf.bufSize = Maths::Max<U32>( setBuf.dirListMax * sizeof( CellSaveDataDirList ), setBuf.fileListMax * sizeof( CellSaveDataFileStat ) );

		// alloc list of FileStats
		U8* pFileStatBuffer = new U8[ setBuf.bufSize ];

		if ( !pFileStatBuffer )
		{
			LOGFATAL( g_Log, "C_AsyncLoadThread::ThreadMain() - pFileStatBuffer - cannot allocate!" );
		}
		else
		{
			bool retry;
			setBuf.buf = pFileStatBuffer;
			do
			{
				retry = false;
				// set off the load
				sErrorCode = cellSaveDataAutoLoad2(
					CELL_SAVEDATA_VERSION_CURRENT,
					g_DataParam.m_pDirName,
					CELL_SAVEDATA_ERRDIALOG_NONE,
					&setBuf,
					DataStatusLoadCallback,
					FileOperationLoadCallback,
				#ifdef _SAVE_USE_MEM_CONTAINER_
					g_MemoryContainer,
				#else
					SYS_MEMORY_CONTAINER_ID_INVALID,
				#endif
					0	// no userdata - yet
				);
				
				// check the error code
				switch ( sErrorCode )
				{
					case CELL_SAVEDATA_RET_OK:
						retry = false;
						break;
					case CELL_SAVEDATA_ERROR_BUSY:
						LOGWARN( g_Log, "Error : Load - Busy - this should never happen!- but it does(retrying..)" );
						retry = true;
						sys_timer_usleep( 1000 * 250 );
						break;
					default:
						retry = false;
						break;
				}
			}
			while( retry );
			

			// cleanup
			SAFE_DELETE_ARY( pFileStatBuffer );
		}
	}

	// load in
	if ( sErrorCode == CELL_SAVEDATA_RET_OK )
	{
		for ( S32 i = 0; i < pSaveInfoInterface->m_FileInfo.Size(); i++ )
		{
			if ( g_Buffers[ i ] )
			{
				if ( !pSaveInfoInterface->Load( m_sPadNo, g_Buffers[ i ], pSaveInfoInterface->m_FileInfo[ i ].m_uFileSize, pSaveInfoInterface->m_FileInfo[ i ].m_sUserData ) )
				{
					m_eErrorCode = E_UiGameProfile_Error_CannotReadFile;	// failed to read in the variables - out-of-date file
					break;
				}
			}
		}
	}

	// check the error code
	switch ( sErrorCode )
	{
		case CELL_SAVEDATA_RET_OK:
			LOGINFO( g_Log, "Load - OK" );
			break;
		case CELL_SAVEDATA_RET_CANCEL:
			LOGINFO( g_Log, "Load - user cancelled" );
			m_eErrorCode = E_UiGameProfile_Error_UserCancelled;
			break;
		case CELL_SAVEDATA_ERROR_CBRESULT:
			LOGERROR( g_Log, "Load - Callback Error" );
			break;
		case CELL_SAVEDATA_ERROR_ACCESS_ERROR:
			LOGERROR( g_Log, "Error : Load - Access Error" );
			m_eErrorCode = E_UiGameProfile_Error_CannotReadFile;
			break;
		case CELL_SAVEDATA_ERROR_INTERNAL:
			LOGERROR( g_Log, "Error : Load - Internal Error" );
			m_eErrorCode = E_UiGameProfile_Error_CannotReadFile;
			break;
		case CELL_SAVEDATA_ERROR_PARAM:
			LOGFATAL( g_Log, "Error : Load - Parameter Error - should never happen!" );
			break;
		case CELL_SAVEDATA_ERROR_NOSPACE:
			LOGFATAL( g_Log, "Error : Load - No Free Space - this is a TRC fail for this error to happen here!" );
			break;
		case CELL_SAVEDATA_ERROR_BROKEN:
			LOGERROR( g_Log, "Error : Load - Corrupted Data" );
			m_eErrorCode = E_UiGameProfile_Error_CannotReadFile;
			break;
		case CELL_SAVEDATA_ERROR_FAILURE:
			LOGERROR( g_Log, "Error : Load - Failure" );
			m_eErrorCode = E_UiGameProfile_Error_CannotReadFile;
			break;
		case CELL_SAVEDATA_ERROR_BUSY:
			LOGFATAL( g_Log, "Error : Load - Busy - this should never happen!" );
			break;
		default:
			LOGWARN( g_Log, "Error : Load - unhandled return value" );
			break;
	}

	// free the buffers
	for ( S32 i = 0; i < g_Buffers.Size(); i++ )
		SAFE_DELETE_ARY( g_Buffers[ i ] );

	g_Buffers.Clear();

	pSaveInfoInterface->LoadFinished();

#ifndef CFG_GOLD
	// force error code
	if ( g_SaveSystemForceErrorCode != E_UiGameProfile_Error_None )
	{
		if ( ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_Okay )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_FileDoesNotExist )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_CannotReadFile )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_UserCancelled )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_DifferentOwner ) )
		{
			m_eErrorCode = g_SaveSystemForceErrorCode;
		}
	}
#endif
}

// save callbacks
void DataListSaveCallback( CellSaveDataCBResult* pResult, CellSaveDataListGet* pGet, CellSaveDataListSet* pSet )
{
	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	strcpy( g_ListNewDataTitle, pSaveInfoInterface->m_pListNewDataTitle );

	memset( &g_ListNewData, 0, sizeof( CellSaveDataListNewData ) );
	memset( &g_ListNewDataIcon, 0, sizeof( CellSaveDataNewDataIcon ) );

	// free file buffer if already exists
	SAFE_DELETE_ARY( g_pFileBuffer );

	if ( pGet->dirNum > pGet->dirListNum )
	{
		pResult->result = CELL_SAVEDATA_CBRESULT_ERR_FAILURE;	// wtf - how did this happen then?
		return;
	}

	pSet->fixedList = pGet->dirList;

	if ( pGet->dirListNum > CELL_SAVEDATA_LISTITEM_MAX )
		pSet->fixedListNum = CELL_SAVEDATA_LISTITEM_MAX;
	else
		pSet->fixedListNum = pGet->dirListNum;

	pSet->focusPosition = CELL_SAVEDATA_FOCUSPOS_LISTHEAD;
	pSet->focusDirName = NULL;
	pSet->reserved = NULL;	// have to set this to NULL

	if ( pGet->dirListNum < MAX_SAVE_ENUMERATIONS )
	{
		// find a new directory name that doesn't already exist
		// -- TODO - make this bit better?
		S32 sNewNum = -1;

		for ( S32 i = 0; i < MAX_SAVE_ENUMERATIONS; i++ )
		{
			bool bFound = false;

			for ( S32 j = 0; j < pGet->dirListNum; j++ )
			{
				S32 sNum = -1;
				sscanf( pGet->dirList[ j ].dirName + strlen( g_DataParam.m_pDirName ), "%d", &sNum );

				if ( sNum == i )
				{
					bFound = true;
					break;
				}
			}

			if ( !bFound )
			{
				sNewNum = i;
				break;
			}
		}

		if ( sNewNum != -1 )
		{
			// set the new subtitle with the enumeration number
			C_WideString256 subtitle;
			subtitle.Format( L"%d. ", sNewNum + 1 );
			subtitle.Add( pSaveInfoInterface->m_Subtitle );

			pSaveInfoInterface->SetSubtitle( subtitle.Chars(), false );				// convert to utf8 again
			sprintf( g_DataParam.m_pSubTitle, pSaveInfoInterface->m_pSubtitle );	// copy back into the data param

			pSet->newData = &g_ListNewData;

			// new directory
			sprintf( g_ListNewDataDirectory, "%s%d", g_DataParam.m_pDirName, sNewNum );

			g_ListNewData.dirName = g_ListNewDataDirectory;
			g_ListNewData.iconPosition = CELL_SAVEDATA_ICONPOS_HEAD; // CELL_SAVEDATA_ICONPOS_TAIL;
			g_ListNewData.reserved = NULL;	// have to set this to NULL

			// load in the new data icon - use Icon0
			char filePath[ CELL_FS_MAX_FS_PATH_LENGTH ];

			Util::C_String dir = pSaveInfoInterface->m_FilePath.Chars();
			Util::C_String filename = Util::C_String( g_DataParam.m_FileList[ E_SaveFileType_Icon0 ].m_FileName ).Lower();
			sprintf( filePath, "%s\\%s", dir.Chars(), filename.Chars() );

			U32 uFileSize = 0;

			if ( !FileAllocLoad( filePath, g_pFileBuffer, uFileSize ) )
				pSet->newData = NULL;	// hide new data icon
			else
			{
				g_ListNewDataIcon.title = g_ListNewDataTitle;
				g_ListNewDataIcon.iconBuf = g_pFileBuffer;
				g_ListNewDataIcon.iconBufSize = uFileSize;
				g_ListNewDataIcon.reserved = NULL;	// have to set this to NULL

				g_ListNewData.icon = &g_ListNewDataIcon;
			}
		}
		else
		{
			LOGERROR( g_Log, "DataListSaveCallback - couldn't find a new directory name - shouldn't happen!" );
			pSet->newData = NULL;	// hide new data icon
		}
	}
	else
	{
		pSet->newData = NULL;	// hide new data icon
	}

	pResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void DataStatusSaveCallback( CellSaveDataCBResult* pResult, CellSaveDataStatGet* pGet, CellSaveDataStatSet* pSet )
{
	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	pSet->reCreateMode  = CELL_SAVEDATA_RECREATE_NO;	// directly overwrite - will corrupt the save if the user switches off whilst saving
	//pSet->reCreateMode |= CELL_SAVEDATA_DISABLE_TROPHY_OWNERSHIP_CHECK;	// DO NOT SHIP THIS - trophies can be unlocked from another users save

	pSet->setParam = &pGet->getParam;

	if ( pGet->isNewData == CELL_SAVEDATA_ISNEWDATA_NO )
	{
		// data already exists
		// - start from the first custom data - no need to re-write out icons/sound
		g_SaveOperationStepCustom = 0;

		for ( S32 i = 0; i < g_DataParam.m_FileList.Size(); i++ )
		{
			if ( g_DataParam.m_FileList[ i ].m_eFileType == E_SaveFileType_CustomData )
			{
				g_SaveOperationStep = i;
				break;
			}
		}

		// check HDD space
		S32 sDiffSizeKB = 0;

		for ( S32 i = 0; i < pSaveInfoInterface->m_FileInfo.Size(); i++ )
		{
			// try and find the file
			S32 j = 0;

			for ( j = 0; j < pGet->fileListNum; j++ )
				if ( !strcmp( pSaveInfoInterface->m_FileInfo[ i ].m_FileName.Chars(), pGet->fileList[ j ].fileName ) )
					break;

			if ( j < pGet->fileListNum )
			{
				// file found - diff sizes
				sDiffSizeKB += SIZE_KB( pSaveInfoInterface->m_FileInfo[ i ].m_uFileSize ) - SIZE_KB( pGet->fileList[ j ].st_size );
			}
			else
			{
				// new file
				sDiffSizeKB += SIZE_KB( pSaveInfoInterface->m_FileInfo[ i ].m_uFileSize );
			}
		}

		S32 sNeedSizeKB = pGet->hddFreeSizeKB - sDiffSizeKB;

		if ( sNeedSizeKB < 0 )
		{
			// no free space
			sNeedSizeKB = Maths::Abs( sNeedSizeKB );
			pResult->errNeedSizeKB = sNeedSizeKB;
			pResult->result = CELL_SAVEDATA_CBRESULT_ERR_NOSPACE;
			LOGINFO( g_Log, "DataStatusSaveCallback: No Free Space" );
			C_SaveSystem::Instance().SetSaveErrorCode( E_UiGameProfile_Error_NoFreeSpace );
			C_SaveSystem::Instance().FreeSpaceNeededKB( sNeedSizeKB );
			return;
		}
	}
	else
	{
		// no save present - create a new one
		strcpy( pSet->setParam->title, pSaveInfoInterface->m_pParamSFO_Title );
		pSet->setParam->attribute = CELL_SAVEDATA_ATTR_NORMAL;
		//pSet->setParam->parentalLevel = PARENTAL_LEVEL;	// this isn't here anymore?!

		strcpy( pSet->setParam->subTitle, g_DataParam.m_pSubTitle );
		strcpy( pSet->setParam->detail, g_DataParam.m_pDetail );
		strcpy( pSet->setParam->listParam, g_DataParam.m_pListParam );

		memset( pSet->setParam->reserved,	0, sizeof( pSet->setParam->reserved ) );	// have to set this to 0
		memset( pSet->setParam->reserved2,	0, sizeof( pSet->setParam->reserved2 ) );	// have to set this to 0
		
		// check HDD space
		S32 sNewSizeKB = 0;

		for ( S32 i = 0; i < g_DataParam.m_FileList.Size(); i++ )
			sNewSizeKB += SIZE_KB( g_DataParam.m_FileList[ i ].m_uFileSize );

		S32 sNeedSizeKB = pGet->hddFreeSizeKB - sNewSizeKB;

		if ( sNeedSizeKB < 0 )
		{
			// no free space
			sNeedSizeKB = Maths::Abs( sNeedSizeKB );
			pResult->errNeedSizeKB = sNeedSizeKB;
			pResult->result = CELL_SAVEDATA_CBRESULT_ERR_NOSPACE;
			LOGINFO( g_Log, "DataStatusSaveCallback: No Free Space" );
			C_SaveSystem::Instance().SetSaveErrorCode( E_UiGameProfile_Error_NoFreeSpace );
			C_SaveSystem::Instance().FreeSpaceNeededKB( sNeedSizeKB );
			return;
		}

		// ok to save out
		g_SaveOperationStep = 0;
		g_SaveOperationStepCustom = 0;
	}

	pResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void FileOperationSaveCallback( CellSaveDataCBResult* pResult, CellSaveDataFileGet* pGet, CellSaveDataFileSet* pSet )
{
	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	char filePath[ CELL_FS_MAX_FS_PATH_LENGTH ];

	// everything is ok unless we set an error further down
	pResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

	// free file buffer if already exists
	SAFE_DELETE_ARY( g_pFileBuffer );

	if ( g_SaveOperationStep >= g_DataParam.m_FileList.Size() )
	{
		// we're done
		pResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
	}
	else
	{
		if ( g_DataParam.m_FileList[ g_SaveOperationStep ].m_eFileType == E_SaveFileType_CustomData )
		{
			// save out custom data
			pSet->fileOperation = CELL_SAVEDATA_FILEOP_WRITE;
			pSet->fileName = g_DataParam.m_FileList[ g_SaveOperationStep ].m_FileName.NonConstChars();
			pSet->fileSize = g_DataParam.m_FileList[ g_SaveOperationStep ].m_uFileSize;
			pSet->fileBufSize = g_DataParam.m_FileList[ g_SaveOperationStep ].m_uFileSize;
			pSet->fileBuf = g_Buffers[ g_SaveOperationStepCustom ];
			//pSet->fileType = CELL_SAVEDATA_FILETYPE_NORMALFILE;
			pSet->fileType = CELL_SAVEDATA_FILETYPE_SECUREFILE;
			memcpy( pSet->secureFileId, pSaveInfoInterface->m_pSecureFieldId, CELL_SAVEDATA_SECUREFILEID_SIZE );

			g_SaveOperationStepCustom++;
		}
		else
		{
			// save out one of the extra files
			U32 uFileSize = 0;

			// load the file into g_FileBuffer
			Util::C_String dir = pSaveInfoInterface->m_FilePath.Chars();
			Util::C_String filename = Util::C_String( g_DataParam.m_FileList[ g_SaveOperationStep ].m_FileName ).Lower();
			sprintf( filePath, "%s\\%s", dir.Chars(), filename.Chars() );

			if ( !FileAllocLoad( filePath, g_pFileBuffer, uFileSize ) )
			{
				// oh noes
				pResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
				C_SaveSystem::Instance().SetLoadErrorCode( E_UiGameProfile_Error_CannotWriteFile );
				return;
			}

			// save out
			pSet->fileOperation = CELL_SAVEDATA_FILEOP_WRITE;
			pSet->fileBuf = g_pFileBuffer;
			pSet->fileBufSize = uFileSize;
			pSet->fileSize = uFileSize;

			// set the file type
			switch ( g_DataParam.m_FileList[ g_SaveOperationStep ].m_eFileType )
			{
				case E_SaveFileType_Icon0:
					pSet->fileType = CELL_SAVEDATA_FILETYPE_CONTENT_ICON0;
					break;
//				case E_SaveFileType_Icon1:
//					pSet->fileType = CELL_SAVEDATA_FILETYPE_CONTENT_ICON1;
//					break;
				case E_SaveFileType_Pic1:
					pSet->fileType = CELL_SAVEDATA_FILETYPE_CONTENT_PIC1;
					break;
//				case E_SaveFileType_Snd0:
//					pSet->fileType = CELL_SAVEDATA_FILETYPE_CONTENT_SND0;
//					break;
				default:
					LOGFATAL( g_Log, "Shouldn't have gotten here!" );
					break;
			}
		}
	}

	// next file
	pResult->progressBarInc = 100 / g_DataParam.m_FileList.Size();
	g_SaveOperationStep++;
}

// -----------------
// C_AsyncSaveThread
// -----------------
void C_AsyncSaveThread::ThreadMain()
{
	if ( !Initialise() )
		return;

	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	if ( g_Buffers.Size() != 0 )
	{
		LOGERROR( g_Log, "C_AsyncSaveThread::ThreadMain - PS3 buffers already exists - this shouldn't happen!" );

		for ( S32 i = 0; i < g_Buffers.Size(); i++ )
			SAFE_DELETE_ARY( g_Buffers[ i ] );

		g_Buffers.Clear();
	}

	// alloc buffers and save out
	bool bSuccess = true;
			
	for ( S32 i = 0; i < pSaveInfoInterface->m_FileInfo.Size(); i++ )
	{
		S32 sFileSize = pSaveInfoInterface->m_FileInfo[ i ].m_uFileSize;
		U8* pSaveFileBuffer = new U8[ sFileSize ];

		if ( !pSaveFileBuffer )
		{
			bSuccess = false;
			LOGFATAL( g_Log, "C_AsyncSaveThread::ThreadMain() - pSaveFileBuffer - cannot allocate!" );
			break;
		}

		memset( pSaveFileBuffer, 0xfe, sFileSize );	// debug clear

		g_Buffers.PushBack( pSaveFileBuffer );

		if ( !pSaveInfoInterface->Save( m_sPadNo, g_Buffers[ i ], sFileSize, pSaveInfoInterface->m_FileInfo[ i ].m_sUserData ) )
		{
			bSuccess = false;
			break;
		}
	}

	if ( bSuccess )
	{
		int sErrorCode = CELL_SAVEDATA_RET_OK;

		if ( pSaveInfoInterface->m_eType == E_UiSaveInfoType_Enumerate )
		{
			// save to a list
			CellSaveDataSetList setList;
			setList.sortType = CELL_SAVEDATA_SORTTYPE_MODIFIEDTIME; // CELL_SAVEDATA_SORTTYPE_SUBTITLE;
			setList.sortOrder = CELL_SAVEDATA_SORTORDER_DESCENT; // CELL_SAVEDATA_SORTORDER_ASCENT;
			setList.dirNamePrefix = g_DataParam.m_pDirName;
			setList.reserved = NULL;	// have to set this to NULL

			CellSaveDataSetBuf setBuf;
			setBuf.dirListMax = MAX_SAVE_ENUMERATIONS;
			setBuf.fileListMax = g_DataParam.m_FileList.Size();
			memset( setBuf.reserved, 0, sizeof( setBuf.reserved ) );	// have to set this to 0
			setBuf.bufSize = Maths::Max<U32>( setBuf.dirListMax * sizeof( CellSaveDataDirList ), setBuf.fileListMax * sizeof( CellSaveDataFileStat ) );

			// alloc buffer
			U8* pBuffer = new U8[ setBuf.bufSize ];

			if ( !pBuffer )
			{
				LOGFATAL( g_Log, "C_AsyncSaveThread::ThreadMain() - pBuffer - cannot allocate!" );
			}
			else
			{
				setBuf.buf = pBuffer;

				// set off the save
				sErrorCode = cellSaveDataListSave2(
					CELL_SAVEDATA_VERSION_CURRENT,
					&setList,
					&setBuf,
					DataListSaveCallback,
					DataStatusSaveCallback,
					FileOperationSaveCallback,
				#ifdef _SAVE_USE_MEM_CONTAINER_
					g_MemoryContainer,
				#else
					SYS_MEMORY_CONTAINER_ID_INVALID,
				#endif
					0	// no userdata - yet
				);

				// cleanup
				SAFE_DELETE_ARY( pBuffer );
			}
		}
		else
		{
			// it's the fixed auto-save
			CellSaveDataSetBuf setBuf = { 0 };
			setBuf.dirListMax = 1;
			setBuf.fileListMax = g_DataParam.m_FileList.Size();
			setBuf.bufSize = Maths::Max<U32>( setBuf.dirListMax * sizeof( CellSaveDataDirList ), setBuf.fileListMax * sizeof( CellSaveDataFileStat ) );
			
			// alloc list of FileStats
			U8* pFileStatBuffer = new U8[ setBuf.bufSize ];

			if ( !pFileStatBuffer )
			{
				LOGFATAL( g_Log, "C_AsyncSaveThread::ThreadMain() - pFileStatBuffer - cannot allocate!" );
			}
			else
			{
				setBuf.buf = pFileStatBuffer;

				// set off the save
				sErrorCode = cellSaveDataAutoSave2(
					CELL_SAVEDATA_VERSION_CURRENT,
					g_DataParam.m_pDirName,
					CELL_SAVEDATA_ERRDIALOG_NONE,
					&setBuf,
					DataStatusSaveCallback,
					FileOperationSaveCallback,
				#ifdef _SAVE_USE_MEM_CONTAINER_
					g_MemoryContainer,
				#else
					SYS_MEMORY_CONTAINER_ID_INVALID,
				#endif
					0	// no userdata - yet
				);

				// cleanup
				SAFE_DELETE_ARY( pFileStatBuffer );
			}
		}

		// check the error code
		switch ( sErrorCode )
		{
			case CELL_SAVEDATA_RET_OK:
				LOGINFO( g_Log, "Save - OK" );
				break;
			case CELL_SAVEDATA_RET_CANCEL:
				LOGINFO( g_Log, "Save - user cancelled" );
				m_eErrorCode = E_UiGameProfile_Error_UserCancelled;
				break;
			case CELL_SAVEDATA_ERROR_CBRESULT:
				LOGERROR( g_Log, "Save - Callback Error" );
				break;
			case CELL_SAVEDATA_ERROR_ACCESS_ERROR:
				LOGERROR( g_Log, "Error : Save - Access Error" );
				m_eErrorCode = E_UiGameProfile_Error_CannotReadFile;
				break;
			case CELL_SAVEDATA_ERROR_INTERNAL:
				LOGERROR( g_Log, "Error : Save - Internal Error" );
				m_eErrorCode = E_UiGameProfile_Error_CannotWriteFile;
				break;
			case CELL_SAVEDATA_ERROR_PARAM:
				LOGFATAL( g_Log, "Error : Save - Parameter Error - should never happen!" );
				break;
			case CELL_SAVEDATA_ERROR_NOSPACE:
				LOGFATAL( g_Log, "Error : Save - No Free Space - this is a TRC fail for this error to happen here!" );
				break;
			case CELL_SAVEDATA_ERROR_BROKEN:
				LOGERROR( g_Log, "Error : Save - Corrupted Data" );
				m_eErrorCode = E_UiGameProfile_Error_CannotWriteFile;
				break;
			case CELL_SAVEDATA_ERROR_FAILURE:
				LOGERROR( g_Log, "Error : Save - Failure" );
				m_eErrorCode = E_UiGameProfile_Error_CannotWriteFile;
				break;
			case CELL_SAVEDATA_ERROR_BUSY:
				LOGFATAL( g_Log, "Error : Save - Busy - this should never happen!" );
				break;
			default:
				LOGWARN( g_Log, "Error : Save - unhandled return value" );
				break;
		}
	}
	else
		m_eErrorCode = E_UiGameProfile_Error_CannotWriteFile;

	// free the buffers
	for ( S32 i = 0; i < g_Buffers.Size(); i++ )
		SAFE_DELETE_ARY( g_Buffers[ i ] );

	g_Buffers.Clear();

	pSaveInfoInterface->SaveFinished();

#ifndef CFG_GOLD
	// force error code
	if ( g_SaveSystemForceErrorCode != E_UiGameProfile_Error_None )
	{
		if ( ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_Okay )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_CannotWriteFile )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_NoFreeSpace )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_UserCancelled ) )
		{
			m_eErrorCode = g_SaveSystemForceErrorCode;
		}
	}
#endif
}

// delete callbacks
void DeleteFixedCallback( CellSaveDataCBResult* pResult, CellSaveDataListGet* pGet, CellSaveDataFixedSet* pSet )
{
	if ( pGet->dirListNum == 0 )
	{
		pResult->result = CELL_SAVEDATA_CBRESULT_ERR_NODATA;
		return;
	}

	pSet->dirName = pGet->dirList[ 0 ].dirName;
	pSet->newIcon = NULL;
	pSet->option = CELL_SAVEDATA_OPTION_NONE;

	pResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void DeleteDoneCallback( CellSaveDataCBResult* pResult, CellSaveDataDoneGet* pGet )
{
	pResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
}

// -------------------
// C_AsyncDeleteThread
// -------------------
void C_AsyncDeleteThread::ThreadMain()
{
	if ( !Initialise() )
		return;

	C_SaveInfoInterface* pSaveInfoInterface = C_SaveSystem::Instance().SaveInfoInterface();

	if ( m_bDeleteFromList )
	{
	#ifdef _SAVE_USE_MEM_CONTAINER_
		cellSaveDataDelete2( g_MemoryContainer );
	#else
		cellSaveDataDelete2( SYS_MEMORY_CONTAINER_ID_INVALID );
	#endif
	}
	else
	{
		// it's delete the fixed auto-save
		// directory name for the autosave data
		char directoryName[ CELL_SAVEDATA_DIRNAME_SIZE ];
		sprintf( directoryName, "%s-AUTO-", pSaveInfoInterface->m_TitleId.Chars() );

		CellSaveDataSetList setList;
		setList.sortType = CELL_SAVEDATA_SORTTYPE_MODIFIEDTIME;
		setList.sortOrder = CELL_SAVEDATA_SORTORDER_DESCENT;
		setList.dirNamePrefix = directoryName;
		setList.reserved = NULL;

		CellSaveDataSetBuf setBuf;
		setBuf.dirListMax = 1; //CELL_SAVEDATA_DIRLIST_MAX - 1;
		setBuf.fileListMax = 0;
		memset( setBuf.reserved, 0, sizeof( setBuf.reserved ) );	// have to set this to 0
		setBuf.bufSize = setBuf.dirListMax * sizeof( CellSaveDataDirList );

		// alloc list of FileStats
		U8* pDirListBuffer = new U8[ setBuf.bufSize ];

		if ( !pDirListBuffer )
		{
			LOGFATAL( g_Log, "C_SaveSystem::DeleteSave() - pDirListBuffer - cannot allocate!" );
		}
		else
		{
			setBuf.buf = pDirListBuffer;

			int sErrorCode = cellSaveDataFixedDelete(
				&setList,
				&setBuf,
				DeleteFixedCallback,
				DeleteDoneCallback,
			#ifdef _SAVE_USE_MEM_CONTAINER_
				g_MemoryContainer,
			#else
				SYS_MEMORY_CONTAINER_ID_INVALID,
			#endif
				0	// no userdata
			);

			// check the error code
			switch ( sErrorCode )
			{
				case CELL_SAVEDATA_RET_OK:
					LOGINFO( g_Log, "delete - OK" );
					m_eErrorCode = E_UiGameProfile_Error_Okay;
					break;
				case CELL_SAVEDATA_RET_CANCEL:
					LOGINFO( g_Log, "delete - user cancelled" );
					m_eErrorCode = E_UiGameProfile_Error_UserCancelled;
					break;
				case CELL_SAVEDATA_ERROR_CBRESULT:
					LOGERROR( g_Log, "Error: delete - Callback Error" );
					m_eErrorCode = E_UiGameProfile_Error_CannotDeleteFile;
					break;
				case CELL_SAVEDATA_ERROR_ACCESS_ERROR:
					LOGERROR( g_Log, "Error : delete - Access Error" );
					m_eErrorCode = E_UiGameProfile_Error_CannotDeleteFile;
					break;
				case CELL_SAVEDATA_ERROR_INTERNAL:
					LOGERROR( g_Log, "Error : delete - Internal Error" );
					m_eErrorCode = E_UiGameProfile_Error_CannotDeleteFile;
					break;
				case CELL_SAVEDATA_ERROR_PARAM:
					LOGFATAL( g_Log, "Error : delete - Parameter Error - should never happen!" );
					break;
				case CELL_SAVEDATA_ERROR_BUSY:
					LOGFATAL( g_Log, "Error : delete - Busy - this should never happen!" );
					break;
				default:
					LOGWARN( g_Log, "Error : delete - unhandled return value" );
					break;
			}

			// cleanup
			SAFE_DELETE_ARY( pDirListBuffer );
		}
	}

	pSaveInfoInterface->DeleteFinished();

#ifndef CFG_GOLD
	// force error code
	if ( g_SaveSystemForceErrorCode != E_UiGameProfile_Error_None )
	{
		if ( ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_Okay )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_CannotDeleteFile )
			|| ( g_SaveSystemForceErrorCode == E_UiGameProfile_Error_UserCancelled ) )
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
