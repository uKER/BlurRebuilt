// ***************************************************************
//  PerUserServerStorageWriter   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Write user generated file(s) to the platform holders servers. 
// ***************************************************************

#ifndef PER_USER_SERVER_STORAGE_WRITER_H
#define PER_USER_SERVER_STORAGE_WRITER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/DemonwareTask.h>

// forward declerations
namespace SysProfile { struct FullProfile; }

namespace NetPlay
{
	/// Write a specific user file to the server.
	/** Each user can have a specific amount of data that is stored on the servers. It's only possible to write files that you own, you can't update your friends user files
	 *	If the file that is to be written already exists it wwill be overwritten.
	 */
	class C_PerUserServerStorageWriter
	{
	public:

		/// Return values when uploading a file to the servers.
		/** @see Continue()
		*/
		typedef enum 
		{
			eWriteNotActive = 0,				/**< no write is currently active. */	
			eWriteInProgress,					/**< still writing the file to the server. */	
			eWriteFailed,						/**< failed to write the requested file. */	
			eWriteSuccessful,					/**< write completed */
		}ePerUserStorageWriterState;

		explicit C_PerUserServerStorageWriter(void); 
		virtual ~C_PerUserServerStorageWriter(); 

		/// Start the process of writing a users file to to the server.
		/** Writing the file to the server is an asynchronous task, if this function returns true then call Continue() until completion.
		 *	@param localPlayer the local players who this file will belong to when it's written to the server.
		 *	@param fileName then name of the file that will be created / updated on the server.
		 *	@param pUploadBuffer game allocated buffer that should contain the data to be uploaded, this buffer MUST stay valid for the complete upload or until StopAndCleanUp() is called to cancel the upload.
		 *	@param uploadFileSize how big is the file that is to be uploaded.  
		 *	@return true if the asynchronous write request was started.
		 */
		bool Start(const SysProfile::FullProfile& localPlayer, const char* fileName, U8* pUploadBuffer, const U32 uploadFileSize);

		/// Continues the asynchronous task of writing a file to the servers.
		/** Must have successfully called Start() before calling this. 
		 *	@return state of current read.
		 */
		ePerUserStorageWriterState Continue(void);

		/// Stops any active write request and release dynamic resources.
		/** This will be called internally but the game can call it during a write to cancel the operation.
		 */
		void StopAndCleanup(void);

	private:
		C_PerUserServerStorageWriter(const C_PerUserServerStorageWriter& original);				// private copy constructor
		C_PerUserServerStorageWriter& operator = (const C_PerUserServerStorageWriter rhs);		// private un coded assignment operator

		bool m_Writing;
		U8* m_pUploadBuffer;
		U32	m_UploadFileSize;
		char m_FileName[MAX_PER_USER_FILE_NAME];
		
#if defined(LIVE_ON_WINDOWS)
		void *m_pAsyncTask;
#elif defined(PLATFORM_PS3) || defined(PLATFORM_PC)
		bdRemoteTaskRef	m_AsyncTask;
#endif
	};
}

#endif


