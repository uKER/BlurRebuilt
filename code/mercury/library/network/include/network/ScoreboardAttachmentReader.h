// ***************************************************************
//  ScoreboardAttachmentReader   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Reading game clips from a scoreboard entry.
// ***************************************************************

#ifndef SCOREBOARD_ATTACHMENT_READER_H
#define SCOREBOARD_ATTACHMENT_READER_H

#ifdef _MSC_VER
#pragma once
#endif

// forward declerations
namespace SysProfile { struct FullProfile; struct BaseProfile; }
namespace NetPlay { class C_PerUserServerStorageReader; }

namespace Scoreboard
{
	/// Read a scoreboard attachment file (game clip on xbox360)
	class C_AttachmentReader
	{
	public:

		/// Return values when downloading the attachment from a scoreboard.
		/** @see Continue()
		*/
		typedef enum 
		{
			eReadNotActive = 0,					/**< no read is currently active. */	
			eReadInProgress,					/**< still reading the file from the server. */	
			eReadFailed,						/**< failed to read the requested file. */	
			eReadSuccessful,					/**< read completed, buffer will contain downloaded data. */
			eReadFileDoesNotExist,				/**< the requested file to download does not exist on the server. */
			eReadDownloadBufferToSmall,			/**< the supplied download buffer was too small to fit the full attachment into. */
		}eScoreboardAttachmentReaderState;

		explicit C_AttachmentReader();
		virtual ~C_AttachmentReader();

		/// Attempt to start reading an attachment from the specified scoreboard for the specified user.
		/** This is an asynchronous task, if true is returned then call Continue() until completion
		 *	@see Continue().
		 *	@param localPlayer the local players who will be downloading the attachment.
		 *	@param attachmentOwner which user on the scoreboard owns this attachment.
		 *	@param scoreboardId which scoreboard is this attachment for.
		 *	@param pStorage pre allocated buffer to download attachment into.
		 *	@param storageSize size of bufer pointed to by pStorage.
		 *	@return true if the asynchronous read operation was started.
		 */
		bool Start(const SysProfile::FullProfile& localPlayer, const SysProfile::BaseProfile& attachmentOwner, const U32 scoreboardId, U8* pStorage, const U32 storageSize);

		/// Continues the asynchronous process of reading an attachment from the scoreboard.
		/**	@param numBytesDownloaded how many bytes has been downloaded once the download completes.
		 *	@return state of download.
		 */
		eScoreboardAttachmentReaderState Continue(U32& numBytesDownloaded);

		/// Stops any active attachment write and resets all internal data to default ready for a new write.
		/** @return true if successfull or false if not.
		 */
		bool StopAndCleanup(void);

	private:
		C_AttachmentReader(const C_AttachmentReader& original);			// private copy constructor
		C_AttachmentReader& operator = (const C_AttachmentReader rhs);	// private un coded assignment operator

		bool	m_Reading;						// have we started writing the data to the scoreboards.
	private:
#if defined(LIVE_ON_WINDOWS)
		U32		m_NumBytesDownloaded;			// how many bytes where actually downloaded.
		void*	m_pAsyncTask;
		XSTORAGE_FACILITY_INFO_GAME_CLIP	m_Info;
		XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS	m_Results;
#else
		NetPlay::C_PerUserServerStorageReader* m_pReader;
#endif
	};

}// end namespace

#endif
