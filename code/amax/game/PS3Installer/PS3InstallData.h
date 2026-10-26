#ifndef _PS3_INSTALL_DATA_H_
#define _PS3_INSTALL_DATA_H_

extern Debug::Bool g_GameFileSystemCacheLevels;


#define USE_NEW_THINGY_MAJIG


// Copy files to the Game Data area of the HDD.
// Be able to Swap Blu-Ray Usage to HDD Usage Transparently
// Copy File data when idle/paused?
#include <System/Serialise/I_FileSystem.h>
#include <sysutil/sysutil_gamecontent.h>

class C_PS3InstallDataItem
{
public:
	C_PS3InstallDataItem( Sys::I_FileSystem* fileSystem, Btl::String sourceFilename, Btl::String destFileName, bool remap, bool isGameData, bool systemFile )
	{
		mFileSystem = fileSystem;
		mSourceFilename = sourceFilename;
		mDestFileName.SetFormatted( destFileName );
		mRemapFile = remap;
		mSystemFile = systemFile;
		mCopyPendingFlag = false;
		mCopyInProgressFlag = false;
		mIsGameData = isGameData;
		mSourceFileSize = 0;
		mDestFileSize = 0;
		mSourceFileTime = 0;
		mDestFileTime = 0;
	}
	Sys::I_FileSystem*	mFileSystem;
	Sys::C_FilePath		mSourceFilename;
	Sys::C_FilePath		mDestFileName;
	S64					mSourceFileSize;
	S64					mDestFileSize;
	Time::C_Time		mSourceFileTime;
	Time::C_Time		mDestFileTime;

	bool				mRemapFile;
	bool				mCopyPendingFlag;
	bool				mCopyInProgressFlag;
	bool				mIsGameData;
	bool				mSystemFile;
	void				SetCopyPendingFlag( bool value ) { mCopyPendingFlag = value; }
	void				SetCopyInProgressFlag( bool value ) { mCopyInProgressFlag = value; }
	void				SetSourceFileSize( S64 size ) { mSourceFileSize = size; }
	void				SetDestFileSize( S64 size ) { mDestFileSize = size; }
	void				SetSourceFileTime( Time::C_Time time ) { mSourceFileTime = time; }
	void				SetDestFileTime( Time::C_Time time ) { mDestFileTime = time; }
};

namespace LogicalFs
{
	class C_PakFs;
}

class C_PS3InstallData
{
public:
	enum eDIALOGACTION
	{
		eDIALOG_ACTION_UNANSWERED,
		eDIALOG_ACTION_ERROR,
	};

	enum eGAMESTATE
	{
		eGameState_UnInitialised,
		eGameState_CheckForSpace,
		eGameState_CheckForSpaceWaiting,
		eGameState_NoHDDSpace,
		eGameState_DeleteSpace,
		eGameState_ValidateHDDContents,
		eGameState_ValidateHDDContentsWaiting,
		eGameState_CopyStage1,
		eGameState_CopyStage2,
		eGameState_CopyStage3,
		eGameState_Idle,
		eGameState_Delay,
	};

	enum eCURRENTCOPY
	{
		eCurrentCopy_Idle,
		eCurrentCopy_IntroMovie,
		eCurrentCopy_AttractMovie,
		eCurrentCopy_MainData,
	};
private:

	bool				m_InitialisedOk;
	bool				m_FinishedCopying;
//	CellGameDataStatGet mCellGameDataStatGet;
	MT::Event			m_SignalStart;
	Btl::Vector< C_PS3InstallDataItem > m_FileCopyList;
	sys_ppu_thread_t	copyingThreadHandle;

	bool				m_Initialised;
	int					m_DialogAction;


	eGAMESTATE			mGameState;
	eGAMESTATE			mAfterDelayState;
	eCURRENTCOPY		mCurrentCopy;
	U32					mDelayCount;
	bool				mDiscEjected;
	bool				mDiscIsIn;
	MT::Mutex			mGameDataCopyMutex;

	bool				m_CopyInProgress;
	volatile bool		mFinishedWaitingForSpaceThread;
	volatile bool		mFinishedWaitingForContentsThread;
	volatile bool		mHasCheckedForFreeSpace;
	volatile bool		mNotEnoughFreeSpace;
	volatile bool		mHasCheckedForFreeSpaceUI;
	S64					mHddFreeSpace;
	S64					mRequiredHDDSpaceKB;
	S64					mRequiredSizeMB;
	S64					mRequiredSaveDataKB;
	S64					mRequiredGameDataKB;
	S64					mCurrentSaveGameSizeKB;
	S64					mCurrentInstalledSizeKB;
	volatile S64		mCopyProgressBytes;
						
//	CellDiscGameSystemFileParam	mBootDiscInfo;

	bool				mBootedFromDisc;
	bool				mUiInitialised;
	bool				WaitForHDDChecks();
	bool				mbInstallationComplete;
	char				mDataContentInfoPath[ CELL_GAME_PATH_MAX ];
	char				mDataUsrdirPath[ CELL_GAME_PATH_MAX ];
	LogicalFs::C_PakFs*	mpPakFs;
	U32					mThrottleUs;
	bool				mAbort;
	bool				mShowInstallMessage;
	
	void				RemoveUnusedLanguageFiles( const char *usedLang );
	void				UninstallFolder( Sys::I_FileSystem* fileSystem, Btl::String sourcefolder, Btl::String destfolder );
	bool				CopyIcons();
	void				CreateGameData();
#ifdef USE_NEW_THINGY_MAJIG
//	LogicalFs::C_PakFs*	mpPakFsCompressed;
	LogicalFs::C_PakFs*	mpPakFsRaw;
	LogicalFs::C_PakFs*	mpPakFsCompressedLoc;
	LogicalFs::C_PakFs*	mpPakFsRawLoc;
#endif

	volatile	bool	mSuspendInstall;
	bool				mXMBActive;

private:
	void SetInitialised( bool ok );
	bool InitialisedOk();
	bool HasFinishedCopying();
//	void SetGameDataStatGet( CellGameDataStatGet* get );
//	CellGameDataStatGet* GetHddInfo() { return &mCellGameDataStatGet; }
	void SignalStartCopy();
	void SignalStartWait();
	bool AddFileToList( Sys::I_FileSystem* fileSystem, const char *filename, const char *destfname, bool remapFile = false, bool isGameData = true, bool systemFile = false );
	bool AddFileToCopy( const char *filename );
	bool StartCopyThread( bool createJoinable );
	bool WaitForCopyToComplete();
	void ForceDirectories( const char *DirectoryPath );

	void CopyLevelPaks();
	U64  GetGameDataSize( const Sys::C_FilePath& filePath );
	bool CheckFiles( Sys::I_FileSystem* sourcefileSystem, const char* sourceFileName , const char* destFileName );
	void AddAllFilesToList();
	void CreateNewData();
	void SetDiscEjected( bool set )							{ mDiscEjected = set;						}
	void SetCurrentSaveGameSizeKB( S64 value )				{ mCurrentSaveGameSizeKB = value;			}
	void SetIsBootedFromDisc( bool set )					{ mBootedFromDisc = set;					}
	eCURRENTCOPY	GetCurrentCopyStatus()					{ return mCurrentCopy; 						}
	bool			IsDiscEjected()							{ return mDiscEjected; 						}
	eGAMESTATE		GetGameState()							{ return mGameState;						}
	S64				GetRequiredSaveDataKB()					{ return mRequiredSaveDataKB;				}
	bool 			GetFinishedWaitingForSpaceThread()		{ return mFinishedWaitingForSpaceThread;	}
	bool 			GetFinishedWaitingForContentsThread()	{ return mFinishedWaitingForContentsThread;	}
	bool 			GetNotEnoughFreeSpace()					{ return mNotEnoughFreeSpace;				}
	bool			GetIsBootedFromDisc()					{ return mBootedFromDisc;					}
	void SetInstallingMessage( bool set )					{ mShowInstallMessage = set;				}
	char* GetDataContentInfoPath()							{ return mDataContentInfoPath;				}
	void AddFolderToList( Sys::I_FileSystem* fileSystem, Btl::String sourcefolder, Btl::String destfolder );
	S64 GetGameDataCopyProgressBytes() const;
	S64 mTotalNrKBytesToCopy;
	bool InstallFolder( Sys::I_FileSystem* fileSystem, const Sys::C_FilePath& path_parameter );
	bool EarlyCheckForDiskFull();

public:
	C_PS3InstallData();
	bool Initialise();

	bool ReleaseResources();
	static C_PS3InstallData& Instance();
	S64	GetRequiredHDDSpaceKB();
	bool IsCopyInProgress();
	bool ShowInstallingMessage();
	bool CopyAttractMovie();
	bool CopyMainInstallData();
	void IncrementGameDataCopyProgressBytes(S64 bytes);
	void SetThrottle( U32 throttle );
	void Update();
	//0..1
	float GetProgressTotal();
	void SetFinishedCopying( bool value = true );
	const Btl::Vector< C_PS3InstallDataItem >& GetFileList() { return m_FileCopyList; }
	C_PS3InstallDataItem* GetFileListItem(U32 index)		{ return &m_FileCopyList[index]; }
	bool CopyFile( Sys::I_FileSystem* fileSystem, const char *sourceName, const char *destName );
	void SetAbort( bool abort )								{ C_PS3InstallData::Instance().WaitForSystemFiles(); mAbort = true; }
	void SetUiInitialised()									{ mUiInitialised = true;					}
	char* GetDataUsrdirPath()								{ return mDataUsrdirPath;					}
	bool IsInstallationComplete()							{ return !IsCopyInProgress();				}
	bool GetAbort()											{ return mAbort; }
	U32 GetThrottle()										{ return mThrottleUs; }
	MT::Mutex&		GetGameDataCopyMutex()					{ return mGameDataCopyMutex;				}
	void SetCopyInProgress( bool set )						{ m_CopyInProgress = set;					}
	void SetCurrentCopyStatus( eCURRENTCOPY status )		{ mCurrentCopy = status;					}
	void SetGameState( eGAMESTATE state )					{ mGameState = state;						}
	S64	 GetHddFreeSpaceKB()								{ return mHddFreeSpace;						}
	void SetHddFreeSpaceKB( S64 value )						{ mHddFreeSpace = value;					}
	S64	 GetCurrentInstalledSizeKB()						{ return mCurrentInstalledSizeKB;			}
	U64  GetGameDataRequiredSize();
	void SetRequiredGameDataKB( S64 value )					{ mRequiredGameDataKB	= value; 			}  
	void SetRequiredHDDSpaceKB( S64 value )					{ mRequiredHDDSpaceKB	= value; 			}
	void SetRequiredSizeMB( S64 value )						{ mRequiredSizeMB		= value; 			}
	void SetRequiredSaveDataKB( S64 value )					{ mRequiredSaveDataKB	= value; 			}
	void SetNotEnoughFreeSpace( bool set )					{ mNotEnoughFreeSpace = set;				}
	void SetHasCheckedForFreeSpace( bool set )				{ mHasCheckedForFreeSpace = set;			}
	void SetFinishedWaitingForSpaceThread( bool set )		{ mFinishedWaitingForSpaceThread = set;		}
	S64	 GetRequiredGameDataKB()							{ return mRequiredGameDataKB;				}
	void SetFinishedWaitingForContentsThread( bool set )	{ mFinishedWaitingForContentsThread = set;	}
	S64	 GetRequiredSizeMB()								{ return mRequiredSizeMB;					}
	U64  GetCurrentSaveGameSize();
	void VerifyContentsOfGameData();
	bool GetHasCheckedForFreeSpace()						{ return mHasCheckedForFreeSpace;			}
	bool GetHasCheckedForFreeSpaceUI()						{ return mHasCheckedForFreeSpaceUI;			}
	S64	GetCurrentSaveGameSizeKB()							{ return mCurrentSaveGameSizeKB;			}
	void WaitForSystemFiles();
	void SuspendInstall()									{ mSuspendInstall = true;					}
	void ResumeInstall()									{ mSuspendInstall = false;					}
	bool IsSuspended()										{ return mSuspendInstall;					}
	void SignalInstallComplete( bool set )					{ mbInstallationComplete = set;				}
	bool SafeToOpenFile()									
	{
		bool result = ((mHasCheckedForFreeSpaceUI) && (mbInstallationComplete));
		return result; 
	}
	void SetXMBActiveState( bool set )						{ mXMBActive = set;					}
	bool GetXMBActiveState()								{ return mXMBActive;				}
};

extern C_PS3InstallData gGameData;

#endif
