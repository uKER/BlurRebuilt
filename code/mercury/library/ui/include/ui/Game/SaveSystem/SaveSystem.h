#ifndef SAVESYSTEM_H
#define SAVESYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIDefines.h"
#include "ui/Util/WideString.h"

#include <Core/Threads/Thread.h>

#ifdef PLATFORM_PS3
#include <cell/cell_fs.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <cell/sysmodule.h>
#include <cell/l10n.h>

#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_savedata.h>
#endif

#ifndef CFG_GOLD
// debug enum to force a specific load/save error
extern Debug::Enum< UI::E_UiGameProfile_Error >	g_SaveSystemForceErrorCode;
#endif

#define _MAX_SAVE_ENUMERATIONS	200		// do we really want this many?

#ifdef PLATFORM_PS3
#define MAX_SAVE_ENUMERATIONS	( ( _MAX_SAVE_ENUMERATIONS < CELL_SAVEDATA_LISTITEM_MAX ) ? _MAX_SAVE_ENUMERATIONS : CELL_SAVEDATA_LISTITEM_MAX )
#else
#define MAX_SAVE_ENUMERATIONS	_MAX_SAVE_ENUMERATIONS
#endif

#define MAX_SAVE_FILENAME_SIZE	32

namespace UI
{

// -----------------
// C_AsyncLoadThread
// -----------------
class C_AsyncLoadThread : public MT::Thread
{
public:
	C_AsyncLoadThread() : m_sPadNo( 0 )			{  }
	~C_AsyncLoadThread()						{  }

public:
	void					Enter( const S32 sPadNo );								// starts off the thread
	E_UiGameProfile_Error	GetErrorCode() const									{ return m_eErrorCode; }
	void					SetErrorCode( const E_UiGameProfile_Error eErrorCode )	{ m_eErrorCode = eErrorCode; }
	
private:
	void					ThreadMain();

private:
	S32						m_sPadNo;
	E_UiGameProfile_Error	m_eErrorCode;
};

// -----------------
// C_AsyncSaveThread
// -----------------
class C_AsyncSaveThread : public MT::Thread
{
public:
	C_AsyncSaveThread() : m_sPadNo( 0 )			{  }
	~C_AsyncSaveThread()						{  }

public:
	void					Enter( const S32 sPadNo );								// starts off the thread
	E_UiGameProfile_Error	GetErrorCode() const									{ return m_eErrorCode; }
	void					SetErrorCode( const E_UiGameProfile_Error eErrorCode )	{ m_eErrorCode = eErrorCode; }
	
private:
	void					ThreadMain();

private:
	S32						m_sPadNo;
	E_UiGameProfile_Error	m_eErrorCode;
};

// -------------------
// C_AsyncDeleteThread
// -------------------
class C_AsyncDeleteThread : public MT::Thread
{
public:
	C_AsyncDeleteThread() : m_sPadNo( 0 )		{  }
	~C_AsyncDeleteThread()						{  }

public:
#ifdef PLATFORM_PS3
	void					Enter( const S32 sPadNo, const bool bDeleteFromList );	// starts off the thread
#else
	void					Enter( const S32 sPadNo );								// starts off the thread
#endif
	E_UiGameProfile_Error	GetErrorCode() const									{ return m_eErrorCode; }
	void					SetErrorCode( const E_UiGameProfile_Error eErrorCode )	{ m_eErrorCode = eErrorCode; }
	
private:
	void					ThreadMain();

private:
	S32						m_sPadNo;
#ifdef PLATFORM_PS3
	bool					m_bDeleteFromList;
#endif
	E_UiGameProfile_Error	m_eErrorCode;
};

#ifdef PLATFORM_PS3
enum E_SaveFileType
{
	E_SaveFileType_Icon0 = 0,
//	E_SaveFileType_Icon1,
	E_SaveFileType_Pic1,
//	E_SaveFileType_Snd0,

	E_SaveFileType_CustomData,	// can load/save multiple of these

	E_SaveFileType_Count,
};
#endif

// --------------
// C_SaveFileInfo
// --------------
#ifdef PLATFORM_PS3
#define SAVE_FILENAME_SIZE		CELL_SAVEDATA_FILENAME_SIZE		/* 13 */
#else
#define SAVE_FILENAME_SIZE		32	// ?
#endif

class C_SaveFileInfo
{
public:
	C_SaveFileInfo()
		: m_uFileSize( 0 )
		, m_bOptional( false )
		, m_sUserData( -1 )
	#ifdef PLATFORM_PS3
		, m_eFileType( E_SaveFileType_CustomData )
	#endif
	{
		//
	}

	C_SaveFileInfo(
		  const U32 uFileSize
		, const Util::C_FixedString< SAVE_FILENAME_SIZE >& fileName			// on PS3 this _MUST_ be 0-9, A_Z, -, _
		, const bool bOptional = false
		, const S32 sUserData = -1
	#ifdef PLATFORM_PS3
		, const E_SaveFileType eFileType = E_SaveFileType_CustomData
	#endif
	)
		: m_uFileSize( uFileSize )
		, m_FileName( fileName )
		, m_bOptional( bOptional )
		, m_sUserData( sUserData )
	#ifdef PLATFORM_PS3
		, m_eFileType( eFileType )
	#endif
	{
		//
	}

public:
	U32											m_uFileSize;		// 200 * 1024
	Util::C_FixedString< SAVE_FILENAME_SIZE >	m_FileName;			// "TEST_SAVE"
	bool										m_bOptional;
	S32											m_sUserData;
#ifdef PLATFORM_PS3
	E_SaveFileType								m_eFileType;
#endif
};

#ifdef PLATFORM_PS3
// -----------
// C_DataParam
// -----------
class C_DataParam
{
public:
	char							m_pDirName[ CELL_SAVEDATA_DIRNAME_SIZE ];
	char							m_pSubTitle[ CELL_SAVEDATA_SYSP_TITLE_SIZE ];
	char							m_pDetail[ CELL_SAVEDATA_SYSP_DETAIL_SIZE ];
	char							m_pListParam[ CELL_SAVEDATA_SYSP_LPARAM_SIZE ];
	Btl::Vector< C_SaveFileInfo >	m_FileList;		// this includes icon/sound files as well as the data files
};
#endif

#ifdef PLATFORM_PC
// --------------------
// C_EnumeratedFileInfo
// --------------------
class C_EnumeratedFileInfo
{
public:
	char	m_FileName[ MAX_SAVE_FILENAME_SIZE ];
};
#endif

enum E_UiSaveInfoType
{
	E_UiSaveInfoType_Default = 0,	// auto-save/load
	E_UiSaveInfoType_Enumerate,		// load from an enumerated list of saves

	E_UiSaveInfoType_Count,
};

// -------------------
// C_SaveInfoInterface
// -------------------
class C_SaveInfoInterface
{
public:
	C_SaveInfoInterface()			{ Reset(); }

public:
	E_UiSaveInfoType				m_eType;
	S32								m_sEnumeration;
	Btl::Vector< C_SaveFileInfo >	m_FileInfo;

#if PLATFORM_PC
public:
	bool							m_bCheckOverwrites;
	Util::C_String					m_SaveRoot;						// "SAVE"
	Util::C_String					m_EnumerationFileExtension;		// "replay"
#endif

#ifdef PLATFORM_PS3
public:
	Util::C_String					m_TitleId;												// "ABCD12345"
	Util::C_String					m_FilePath;												// "Image\\MemSavePS3"
	U32								m_uSizeIcon0;											// 30554	- REQUIRED	- TRC
	U32								m_uSizeIcon1;											// 1718272	- OPTIONAL	- 0
	U32								m_uSizePic1;											// 715488	- OPTIONAL	- 0
	U32								m_uSizeSnd0;											// 851224	- OPTIONAL	- 0
	Util::C_String					m_ListName;												// "REPLAY"					- for enumerations

public:
	void							SetParamSFO_Title( const WCHAR* pParamSFO_Title );
	void							SetParamSFO_Detail( const WCHAR* pParamSFO_Detail );
	void							SetSubtitle( const WCHAR* pSubtitle, const bool bCopyInternal = true );
	void							SetListNewDataTitle( const WCHAR* pListNewDataTitle );
	void							SetSecureFieldId( const char* pSecureFieldId );

	// these shouldn't be used directly
public:
	char							m_pParamSFO_Title[ CELL_SAVEDATA_SYSP_TITLE_SIZE ];		// utf8 - "Test"
	char							m_pParamSFO_Detail[ CELL_SAVEDATA_SYSP_DETAIL_SIZE ];	// utf8 - "Test Save Data Detail Info"
	C_WideString256					m_Subtitle;												// the raw subtitle - for enumerations for tag a number on to this at save time
	char							m_pSubtitle[ CELL_SAVEDATA_SYSP_TITLE_SIZE ];			// utf8 - "Save Game"
	char							m_pListNewDataTitle[ CELL_SAVEDATA_SYSP_TITLE_SIZE ];	// utf8 - "Create New Data"	- for enumerations
	unsigned char					m_pSecureFieldId[ CELL_SAVEDATA_SECUREFILEID_SIZE ];	// "WE <3 PS3 ... ??"
#endif

public:
	void							Reset();

	virtual bool					Load( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )	= 0;	// set from the buffer
	virtual bool					Save( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )	= 0;	// write into the buffer

	virtual void					LoadFinished()		{ }		// any extra stuff we may need to do regardless of error code
	virtual void					SaveFinished()		{ }		// any extra stuff we may need to do regardless of error code
	virtual void					DeleteFinished()	{ }		// any extra stuff we may need to do regardless of error code
};

// ------------
// C_SaveSystem
// ------------
class C_SaveSystem
{
public:
	C_SaveSystem()
		: m_pSaveInfoInterface( NULL )
#ifdef PLATFORM_PC
		, m_sEnumerationCount( 0 )
#endif
#ifdef PLATFORM_PS3
		, m_sFreeSpaceNeededKB( -1 )
#endif
	{
		//
	}

public:
	static C_SaveSystem&	Instance()						{ return m_sInstance; }

public:
	// interface
	C_SaveInfoInterface*	SaveInfoInterface()													{ BIZ_ASSERT_MSG( m_pSaveInfoInterface, "save won't work - but will report ok" ); return m_pSaveInfoInterface; }
	void					SaveInfoInterface( C_SaveInfoInterface* pSaveInfoInterface )		{ BIZ_ASSERT_MSG(   pSaveInfoInterface, "save won't work - but will report ok" ); m_pSaveInfoInterface = pSaveInfoInterface; }

	// loading
	bool					StartAsyncLoad( S32 sPadNo );
	bool					ContinueAsyncLoad();
	E_UiGameProfile_Error	GetLoadErrorCode()													{ return m_AsyncLoadThread.GetErrorCode(); }
	void					SetLoadErrorCode( const E_UiGameProfile_Error eErrorCode )			{ m_AsyncLoadThread.SetErrorCode( eErrorCode ); }

	// saving
	bool					StartAsyncSave( S32 sPadNo );
	bool					ContinueAsyncSave();
	E_UiGameProfile_Error	GetSaveErrorCode()													{ return m_AsyncSaveThread.GetErrorCode(); }
	void					SetSaveErrorCode( const E_UiGameProfile_Error eErrorCode )			{ m_AsyncSaveThread.SetErrorCode( eErrorCode ); }

	S32						GetCurrentSaveSize();

	// deleting
#ifdef PLATFORM_PS3
	bool					StartAsyncDelete( S32 sPadNo, bool bDeleteFromList );
#else
	bool					StartAsyncDelete( S32 sPadNo );
#endif
	bool					ContinueAsyncDelete();
	E_UiGameProfile_Error	GetDeleteErrorCode()												{ return m_AsyncDeleteThread.GetErrorCode(); }
	void					SetDeleteErrorCode( const E_UiGameProfile_Error eErrorCode )		{ m_AsyncDeleteThread.SetErrorCode( eErrorCode ); }

	// enumerating
	S32						GetEnumerationCount();
	const WCHAR*			GetEnumerationName( const S32 sIndex );
	bool					IsValidEnumerationFileType( const S32 sIndex );

#ifdef PLATFORM_PC
	S32						m_sEnumerationCount;
	C_EnumeratedFileInfo	m_EnumeratedFileInfo[ MAX_SAVE_ENUMERATIONS ];
#endif

	C_AsyncSaveThread&		GetAsyncSaveThread()												{ return m_AsyncSaveThread; }

#ifdef PLATFORM_PS3
	S32						FreeSpaceNeededKB()													{ return m_sFreeSpaceNeededKB; }
	void					FreeSpaceNeededKB( const S32 sFreeSpaceNeededKB )					{ m_sFreeSpaceNeededKB = sFreeSpaceNeededKB; }
#endif

private:
	static C_SaveSystem		m_sInstance;

private:
	C_AsyncLoadThread		m_AsyncLoadThread;		// loading thread
	C_AsyncSaveThread		m_AsyncSaveThread;		// saving thread
	C_AsyncDeleteThread		m_AsyncDeleteThread;	// deleting thread

	C_SaveInfoInterface*	m_pSaveInfoInterface;	// platform specific info needed for the save system

#ifdef PLATFORM_PS3
	S32						m_sFreeSpaceNeededKB;
#endif
};

}; //namespace UI

#endif /* SAVESYSTEM_H */
