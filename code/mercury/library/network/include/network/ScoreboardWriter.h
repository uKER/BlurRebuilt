// ***************************************************************
//  ScoreboardWriter   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Write scores to online scoreboards.
// ***************************************************************

#ifndef SCOREBOARD_WRITER_H
#define SCOREBOARD_WRITER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/DemonwareTask.h>
#include <Network/ScoreboardStructures.h>

// forward declerations
namespace SysProfile { struct UserIdentifier; }
namespace NetPlay { class C_Session; }


namespace Scoreboard
{
	/// Object used for writing data to a collection of scoreboards.
	class C_Writer
	{
	public:
		explicit C_Writer();
		virtual ~C_Writer();

		/// Allocate Dynamic resources required to write to scoreboard.
		/**	@param maxScoreboardsToWrite maximum number of scoreboards we wish to write in 1 server call.
		 *	@param maxColumns maximum columns we can write in 1 call must have enough columns to service the number of scoreboards setup via maxScoreboardsToWrite. This is only referenced on Xbox360.
		 *	@return true if successfull or false if error occurred.
		 */
		bool Initialise(const U32 maxScoreboardsToWrite, const U32 maxColumns);

		/// Stops any active scoreboard write and resets all internal data to default ready for a new write.
		/**	This should be called before you attempt to Queue() any new scoreboards for writing to clear any old data that is still hanging around.
		 *	If the write is successful then this will be called automatically.
		 *	@return true if successfull or false if not.
		 */
		bool StopAndCleanup(void);

		/// Attempt to queue scoreboard data for writing.
		/**	@param scoreboardId id of the scoreboard where the data will be written.
		 *	@param rating players rating that will be used to order the player on the scoreboard. This will be ignored on xbox360 since one of the supplied columns will be configured to be the rating within XLAST.
		 *	@param writeType when using demonware this dictates what is is used to determine is the scores whould be written or not. On Xbox360 this is ignored sinec this is configured within XLAST. 
		 *	@param pColumnData pointer to some Scoreboard::ColumnData structures that should be configured with the colum data to write. The columns should be in the order they are defined within XLAST or Web-IMP.
		 *	@param numColumns number of Scoreboard::Data structures that pColumnData points to.
		 *	@return true if successfully or false if not.
		 */
		bool Queue(const U32 scoreboardId, const S64 rating, const eWriteType writeType, const ColumnData* pColumnData, const U32 numColumns);

		/// Attempt to start the asynchronous task of writing any queued data to the scoreboard server.
		/**	@see Continue().
		 *	@param user local or remote user identifier for who the queued scoreboard data will be written for.	
		 *	@param session current session that is required to be in the play state for the stats to be written correctly.
		 *	@return true if asynchronous write started or false if not.
		 */
		bool Start(const SysProfile::UserIdentifier& user, const NetPlay::C_Session& session);

		/// Continues the aysnchronous process of writing the queued data to the scoreboards.
		/**	@param errorOccurred once function returns true then check this value to see if data was successfully written to scoreboards or now.
		 *	@return true when write finished, or false if write still in progress.
		 */
		bool Continue(bool& errorOccurred);

		/// Returns true if we are currently writing.
		bool IsWriting(void) const {return m_Writing;}

	private:
		C_Writer(const C_Writer& original);			// private copy constructor
		C_Writer& operator = (const C_Writer rhs);	// private un coded assignment operator
		void CleanUp(void);

	private:
		bool	m_Initialised;					// has the dynamic memory been allocated.	
		bool	m_Writing;						// have we started writing the data to the scoreboards.
		U32		m_MaxScoreboards;				// maximum number of scoreboards we can write to using 1 Write() call
		U32		m_MaxColumns;					// maximum number of columns that the scoreboards will use when writing
		U32		m_NumScoreboardsToWrite;		// how many scoreboards have been setup ready for writing
		U32		m_NumColumnsToWrite;			// how many columns have been setup ready for writing
		void*	m_pScoreboardData;

#if defined(LIVE_ON_WINDOWS)
		void*	m_pColumnData;
		void*	m_pAsyncTask;
#else
		bdRemoteTaskRef m_AsyncTask;
#endif
	};

}// end namespace


#endif


