// ***************************************************************
//  ScoreboardAttachmentWriter   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Writing game clips to a scoreboard entry.
// ***************************************************************

#ifndef SCOREBOARD_ATTACHMENT_WRITER_H
#define SCOREBOARD_ATTACHMENT_WRITER_H

#ifdef _MSC_VER
#pragma once
#endif

// forward declerations
namespace NetPlay { class C_PerUserServerStorageWriter; }
namespace SysProfile { struct FullProfile; }


namespace Scoreboard
{
	/// Object used for writing an attachment (game clip on xbox360) to a scoreboard entry.
	class C_AttachmentWriter
	{
	public:
		/// Return values when uploading an attachment to the scoreboard..
		/** @see Continue()
		*/
		typedef enum 
		{
			eWriteNotActive = 0,				/**< no write is currently active. */	
			eWriteInProgress,					/**< still writing the file to the server. */	
			eWriteFailed,						/**< failed to write the requested file. */	
			eWriteSuccessful,					/**< write completed */
		}eScoreboardAttachmentWriterState;

		explicit C_AttachmentWriter();
		virtual ~C_AttachmentWriter();

		/// Attempt to start writing an attachment to a specified scoreboard for the specified user.
		/**	@param localPlayer local user who is to have their attachment written to the scoreboard.
		 *	@param scoreboardId which scoreboard is this attachment for.
		 *	@param scoreRating the same rating value that was written when the score was uploaded, it MUST match !
		 *	@param pAttachmentData pointer to the data to upload, this MUST stay resident until the asynchronous write completes.
		 *	@param attachmentSize size in bytes to upload.
		 */
		bool Start(const SysProfile::FullProfile& localPlayer, const U32 scoreboardId, const S64 scoreRating, U8* pAttachmentData, const U32 attachmentSize);
			
		/// Continues the asynchronous process of writing an attachment to the scoreboard.
		/**	@return state of wripe operation.
		 */
		eScoreboardAttachmentWriterState Continue(void);

		/// Stops any active attachment write and resets all internal data to default ready for a new write.
		/** @return true if successfull or false if not.
		 */
		bool StopAndCleanup(void);

	private:
		C_AttachmentWriter(const C_AttachmentWriter& original);			// private copy constructor
		C_AttachmentWriter& operator = (const C_AttachmentWriter rhs);	// private un coded assignment operator

	private:
		bool	m_Writing;						// have we started writing the data to the scoreboards.
#if defined(LIVE_ON_WINDOWS)
		void*								m_pAsyncTask;
		XSTORAGE_FACILITY_INFO_GAME_CLIP	m_Info;
#else
		NetPlay::C_PerUserServerStorageWriter* m_pWriter;
#endif
	};

}// end namespace


#endif


