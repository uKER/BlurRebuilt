// ***************************************************************
//  ScoreboardReaderByRank   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Scoreboard row enumerator from a rank or pivot.
// ***************************************************************

#ifndef SCOREBOARD_READER_RANK_H
#define SCOREBOARD_READER_RANK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/DemonwareTask.h>
#include <Network/ScoreboardStructures.h>

// forward declerations
namespace SysProfile { struct UserIdentifier; }

namespace Scoreboard
{
	// forward declerations
	struct RowData;

	/// Object used for reading column data from scoreboards from a specific rank or to pivot around a specific rank.
	/** On Xbox360 this object is capable of reading from more than 1 scoreboard with 1 server request.
	 *	When using demonware it's only possible to read from 1 scoreboard from 1 server request.
	 */
	class C_ReaderByRank
	{
	public :
		explicit C_ReaderByRank();
		virtual ~C_ReaderByRank();

		/// Call once to allocate dynamic resources required to read from scoreboards
		/**	@return true if successful or false if not.
		 */
		bool Initialise(void); 
  
		/// Attempt to add a scoreboard so its data is read when requested.
		/** @param scoreboardId unique identifier of the scoreboard to read.
		 *	@param columns list of column identifiers to retrieve from the scoreboard. When using demonware this is ignored as all columns will be returned in the query.
		 *	@param columnTypes variable types of the columns that will be returned in the same order as they are defined in the ColumnIdentifierList. This is ignored if not using demonware.
		 *	@param flushCache if true then the curreent cached list of scoreboards to retrieve is flushed before adding this new one.
		 *	@return true if successfull or false if not.
		 */
		bool AddScoreboard(const U32 scoreboardId, const ColumnIdentifierList& columns, const ColumnTypeList& columnTypes, const bool flushCache);

		/// Attempts to query the server for a set number of rows from the scoreboards already configured by AddScoreboard()
		/**	@see AddScoreboard().
		 *	@see Continue().
		 *	@param titleId Id of the title who's scoreboards are being queried.
		 *	@param startRank rank of wher the query should start, scoreboard ranks go from 1 to num entries.
		 *	@param maxRowsToRetrieve how many rows should we retrieve for each scoreboard.
		 *	@return true if the asynchronous server query was started or false if not.
		 */
		bool StartReadByRank(const U32 titleId, const U32 startRank, const U32 maxRowsToRetrieve);

		/// Attempts to query the server for a set number of rows from the scoreboards already configured by AddScoreboard().
		/** The rows that are returned are pivoted around the specific user.
		 *	@see AddScoreboard().
		 *	@see Continue().
		 *	@param titleId Id of the title who's scoreboards are being queried.
		 *	@param userPivot user to use as the pivot point within the scoreboard.
		 *	@param maxRowsToRetrieve How many rows should we retrieve for each scoreboard.
		 *	@return true if the asynchronous server query was started or false if not.
		 */
		bool StartReadByPivot(const U32 titleId, const SysProfile::UserIdentifier& userPivot, const U32 maxRowsToRetrieve);

		/// Continues the asynchronous procedure of reading scoreboard data from the servers.
		/** @param errorOccurred check this once function returns true.
		 *	@return true when read operation has finished else false.
		 */
		bool Continue(bool& errorOccurred);

		/// Once the scoreboard data has been retrieved from the server we can extract the row data.
		/** @param scoreboardId unique identifier of scoreboard which rows will be extracted from.
		 *	@param pOutput pointer to a list of of Scoreboard::RowData objects to extract into.
		 *	@param maxRowsToExtract number of Scoreboard::RowData objects that pOutput points to.
		 *	@param	scoreboardSize upon successfull exit this will hold the total number of rows on the specified scoreboard.
		 *	@return the number of rows extracted.
		 */
		U32 ExtractRows(const U32 scoreboardId, RowData* pOutput, const U32 maxRowsToExtract, U32& scoreboardSize);

		/// Call this function once you have finished with the returned scoreboard data.
		/** This will cause all cached scoreboard data to be released.
		 */
		void StopAndCleanup(void);

		bool IsReading() const { return m_Reading; }

		// maximum number of scoreboards we can request in 1 call to the server
		U32	 MaxScoreboardsToReadFrom() const { return m_MaxScoreboardsToReadFrom; }

	private:
		C_ReaderByRank(const C_ReaderByRank& original);			// private copy constructor
		C_ReaderByRank& operator = (const C_ReaderByRank rhs);	// private un coded assignment operator
		void CleanUp();

	private:
		bool	m_Initialised;
		bool	m_Reading;
		U32		m_MaxScoreboardsToReadFrom;		// maximum number of scoreboards we can request in 1 call to the server
		U32		m_NumScoreboardsToQuery;		// how many scoreboards are we querying in the server call..
		void*	m_pRequriredScoreboards;		// list of all scoreboards we will bw querying.

#if defined(LIVE_ON_WINDOWS)
		HANDLE	m_Handle;
		void*	m_pAsyncTask;					// asynchronous task used when requesting data from server.	
		U8*		m_pResultsBuffer;				// pointer to a buffer to hold data read from the scoreboards.
#else
		bdRemoteTaskRef m_AsyncTask;
#endif
	};
}// end namespace


#endif


