#ifndef SAVESYSTEM_H
#define SAVESYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIDefines.h"
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
	void					Enter( const S32 sPadNo );								// starts off the thread
	E_UiGameProfile_Error	GetErrorCode() const									{ return m_eErrorCode; }
	void					SetErrorCode( const E_UiGameProfile_Error eErrorCode )	{ m_eErrorCode = eErrorCode; }
	
private:
	void					ThreadMain();

private:
	S32						m_sPadNo;
	E_UiGameProfile_Error	m_eErrorCode;
};

#ifdef PLATFORM_PS3
// -----------
// C_FileParam
// -----------
class C_FileParam
{
public:
	char	m_pFileName[ CELL_SAVEDATA_FILENAME_SIZE + 16 ];
	S32		m_sFileSize;	
	char	m_pPadding[ 3 ];
};

// -----------
// C_DataParam
// -----------
class C_DataParam
{
public:
	char			m_pDirName[ CELL_SAVEDATA_DIRNAME_SIZE ];
	char			m_pSubTitle[ CELL_SAVEDATA_SYSP_TITLE_SIZE ];
	char			m_pDetail[ CELL_SAVEDATA_SYSP_DETAIL_SIZE ];
	char			m_pListParam[ CELL_SAVEDATA_SYSP_LPARAM_SIZE ];
	C_FileParam*	m_pFileList;
};
#endif

// -------------------
// C_SaveInfoInterface
// -------------------
class C_SaveInfoInterface
{
#ifdef PLATFORM_XENON
public:
	virtual const char*		SaveRoot()			= 0;	// "SAVE"
	virtual const char*		InternalName()		= 0;	// "SAVE:\\savegame"
	virtual const char*		FileName()			= 0;	// "testprofile.save"
	virtual const WCHAR*	DisplayName()		= 0;	// UI::C_TextStringID( "IDS_PROFILE_SAVE_GAME_NAME" ).String()
	virtual const bool		Roamable()			= 0;	// true
	virtual const bool		CheckOverwrites()	= 0;	// false
#elif PLATFORM_PS3
public:
	virtual const char*		TitleID()			= 0;	// "ABCD12345"
	virtual const char*		ParamSFO_Title()	= 0;	// "Test"
	virtual const char*		ParamSFO_Detail()	= 0;	// "Test Save Data Detail Info"
	virtual const char*		Subtitle()			= 0;	// "Save Game"
	virtual Util::C_String	FilePath()			= 0;	// "Image\\MemSavePS3"
	virtual const S32		SizeIcon0()			= 0;	// 30554
	virtual const S32		SizeIcon1()			= 0;	// 1718272
	virtual const S32		SizePic1()			= 0;	// 715488
	virtual const S32		SizeSnd0()			= 0;	// 851224
	virtual const char*		SecureFieldId()		= 0;	// sizeof( CELL_SAVEDATA_SECUREFILEID_SIZE ) - e.g. "WE <3 PS3 ... ??"
#elif PLATFORM_PC
	// TODO
#endif

public:
	virtual const U32		BufferSize()		= 0;	// 200 * 1024

	virtual bool			Load( const S32 sPadNo, U8* pBuffer, const U32 uDataSize )	= 0;	// set from the buffer
	virtual bool			Save( const S32 sPadNo, U8* pBuffer, const U32 uDataSize )	= 0;	// write into the buffer
};

// ------------
// C_SaveSystem
// ------------
class C_SaveSystem
{
public:
	C_SaveSystem() : m_pSaveInfoInterface( NULL )			{  }

public:
	static C_SaveSystem&	Instance()						{ return m_sInstance; }

public:
	// interface
	C_SaveInfoInterface*	SaveInfoInterface()													{ assert( m_pSaveInfoInterface ); return m_pSaveInfoInterface; }
	void					SaveInfoInterface( C_SaveInfoInterface* pSaveInfoInterface )		{ assert( pSaveInfoInterface ); m_pSaveInfoInterface = pSaveInfoInterface; }

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

	// deleting
	bool					StartAsyncDelete( S32 sPadNo );
	bool					ContinueAsyncDelete();
	E_UiGameProfile_Error	GetDeleteErrorCode()												{ return m_AsyncDeleteThread.GetErrorCode(); }
	void					SetDeleteErrorCode( const E_UiGameProfile_Error eErrorCode )		{ m_AsyncDeleteThread.SetErrorCode( eErrorCode ); }

private:
	static C_SaveSystem		m_sInstance;

private:
	C_AsyncLoadThread		m_AsyncLoadThread;		// loading thread
	C_AsyncSaveThread		m_AsyncSaveThread;		// saving thread
	C_AsyncDeleteThread		m_AsyncDeleteThread;	// deleting thread

	C_SaveInfoInterface*	m_pSaveInfoInterface;	// platform specific info needed for the save system
};

}; //namespace UI

#endif /* SAVESYSTEM_H */
