// ***************************************************************
//  PerUserServerStorageReader   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Read user generated file(s) from the online servers. 
// ***************************************************************

#ifndef PER_USER_SERVER_STORAGE_READER_H
#define PER_USER_SERVER_STORAGE_READER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdLobby/bdStorage/bdFileData.h>
#include <bdLobby/bdStorage/bdFileInfo.h>
#include <Network/DemonwareTask.h>

// forward declerations
namespace SysProfile { struct FullProfile; struct UserIdentifier; }

namespace NetPlay
{
	/// Read a specific user file from the server.
	/** Each user can have a specific amount of data that is stored on the servers, this data is public so any user can query any other users files.
	 * Only players with full online accounts can have files on the servers, guests or blagged players won't. the game is in control of who it requests files for.
	 */
	class C_PerUserServerStorageReader
	{
	public:

		/// Return values when downloading a file from the servers.
		/** @see Continue()
		 */
		typedef enum 
		{
			eReadNotActive = 0,					/**< no read is currently active. */	
			eReadInProgress,					/**< still reading the file from the server. */	
			eReadFailed,						/**< failed to read the requested file. */	
			eReadSuccessful,					/**< read completed, buffer will contain downloaded data. */
			eReadFileDoesNotExist,				/**< the requested file to download does not exist on the server. */
			eReadDownloadBufferToSmall,			/**< the supplied download buffer was too small to fit the full file into. */
		}ePerUserStorageReaderState;

		explicit C_PerUserServerStorageReader(void); 
		virtual ~C_PerUserServerStorageReader(); 

		/// Start the process of reading a file from the server.
		/** Reading the file from the server is an asynchronous task, if this function returns true then call Continue() until completion.
		 *	Once you have received the file the data must be copied out before calling StopAndCleanup().
		 *	@param localPlayer the local players index wants to initiate the read file request.
		 *	@param fileOwner the user whos file will be returned if it exists.
		 *	@param fileName then name of the file that is to be returned.
		 *	@param pDownloadBuffer game allocated buffer to store file into, this buffer MUST stay valid for the complete download or until StopAndCleanUp() is called to cancel the download.
		 *	@param downloadBufferSize how big is the memory allocated pointed to by pDownloadBuffer. When using demonware this MUST be large enough to hold the complete file or memort traching may occur.  
		 *	@return true if the asynchronous read request was started.
		 */
		bool Start(const SysProfile::FullProfile& localPlayer, const SysProfile::UserIdentifier& fileOwner, const char* fileName, U8* pDownloadBuffer, const U32 downloadBufferSize);
	
		/// Continues the asynchronous task of reading a file from the servers.
		/** Must have successfully called Start() before calling this. 
		 *	@param downloadedSize once download has finished this will hold the number of bytes written into the download buffer.
		 *	@return state of current read.
		 */
		ePerUserStorageReaderState Continue(U32& downloadedSize);

		/// Stops any active read request and release dynamic resources.
		/** Once the file has been downloaded and the data has been copied out this function must be called to release the dynamic resources
		 *	allocated when downloading the file. 
		 */
		void StopAndCleanup(void);

	private:
		C_PerUserServerStorageReader(const C_PerUserServerStorageReader& original);				// private copy constructor
		C_PerUserServerStorageReader& operator = (const C_PerUserServerStorageReader rhs);		// private un coded assignment operator

		bool m_Reading;
		U8* m_pDownloadBuffer;
		U32	m_DownloadBufferSize;
		char m_FileName[MAX_PER_USER_FILE_NAME];
		
#if defined(LIVE_ON_WINDOWS)
		void *m_pAsyncTask;
		XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS	m_Results;
#elif defined(PLATFORM_PS3) || defined(PLATFORM_PC)
		bool			m_Searching;
		bdFileData		m_FileData;
		bdFileInfo		m_FileInfo;
		bdRemoteTaskRef	m_AsyncTask;
#endif
	};
}

#endif


