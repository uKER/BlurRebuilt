// ***************************************************************
//  ScoreboardReaderByUsers   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Scoreboard row enumerator from a list of users.
// ***************************************************************

#ifndef SCOREBOARD_READER_USERS_H
#define SCOREBOARD_READER_USERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/ScoreboardStructures.h>
#include <Network/DemonwareTask.h>

#include <Network/DemonwareTask.h>

// forward declerations
namespace SysProfile { struct UserIdentifier; }

namespace Scoreboard
{
	// forward declerations
	struct RowData;

	/// Object used for reading column data from scoreboards for specific users ie: friends.
	/** On Xbox360 this object is capable of reading from more than 1 scoreboard with 1 server request.
	 *	When using demonware it's only possible to read from 1 scoreboard from 1 server request.
	 *	If a player does not have an entry on the requested scoreboard then no data will be returned for that player.
	 */
	class C_ReaderByUsers
	{
	public :
		explicit C_ReaderByUsers();
		virtual ~C_ReaderByUsers();

		/// Call once to allocate dynamic resources required to read from scoreboards.
		/**	@param maxUsersToQuery max users to query in one server call, 0 = use default MAX_FRIENDS+1
		 *	@return true if successful or false if not.
		 */
		bool Initialise(const U32 maxUserstoQuery = 0); 

		/// Set what users we wish to retrieve scoreboard data for.
		/**	@param pUsers pointer to a list of correctly configured SysProfile::UserIdentifier structures.
		 *	@param numUsers how many users does pUSers point to.
		 *	@return true if successfull of false if not.
		 */
		bool SetUsers(const SysProfile::UserIdentifier* pUsers, const U32 numUsers);

		/// Attempt to add a scoreboard so its data is read when requested.
		/**	@param scoreboardId unique identifier of the scoreboard to read.
		 *	@param columns list of column identifiers to retrieve from the scoreboard. When using demonware this is ignored as all columns will be returned in the query.
		 *	@param columnTypes variable types of the columns that will be returned in the same order as they are defined in the ColumnIdentifierList. This is ignored if not using demonware.
		 *	@param flushCache if true then the curent list of scoreboards to retrieve will be cleared before the new one is added.
		 *	@return true if successfull or false if not.
		 */
		bool AddScoreboard(const U32 scoreboardId, const ColumnIdentifierList& columns, const ColumnTypeList& columnTypes, const bool flushCache);

		/// Start Reading the requested scoreboards for the requested users.
		/**	@see Continue().
		 *	@param titleId Id of the title we wish to read scoreboards for.
		 *	@return true if read started successfully or false if not. if true then call ContinueRead() until asynchronous read is complete.
		 */
		bool Start(const U32 titleId);

		/// Continues the asynchronous procedure of reading scoreboard data from the servers.
		/**	@param errorOccurred check this once function returns true.
		 *	@return true when read operation has finished else false.
		 */
		bool Continue(bool& errorOccurred);

		/// Once the scoreboard data has been retrieved from the server we can extract the row data.
		/** @param scoreboardId unique identifier of scoreboard which rows will be extracted from.
		 *	@param pOutput pointer to a list of of Scoreboard::RowData objects to extract into.
		 *	@param maxRowsToExtract number of Scoreboard::RowData objects that pOutput points to.
		 *	@return the number of rows extracted.
		 */
		U32 ExtractRows(const U32 scoreboardId, RowData* pOutput, const U32 maxRowsToExtract);

		/// Once the scoreboard data has been retrieved from the server we can extract the row data for a specific user.
		/** @param user who's row we should try and return 
		 *	@param scoreboardId unique identifier of scoreboard which rows will be extracted from.
		 *	@param rowOut Scoreboard::RowData objects to write data into
		 *	@return true if the user was found in the server data, if false rowOut wil not be valid.
		 */
		bool ExtractRowForUser(const SysProfile::UserIdentifier& user,const U32 scoreboardId, RowData& rowOut);

		/// Call this function once you have finished accessing the results from a Start() request.
		/** Releases the dynamic results buffer.
		 */
		void StopAndCleanup(void);

		// maximum number of scoreboards we can request in 1 call to the server
		U32	 MaxScoreboardsToReadFrom() const { return m_MaxScoreboardsToReadFrom; }

	private:
		C_ReaderByUsers(const C_ReaderByUsers& original);			// private copy constructor
		C_ReaderByUsers& operator = (const C_ReaderByUsers rhs);	// private un coded assignment operator
		void CleanUp(void);

	private:
		bool	m_Initialised;
		bool	m_Reading;
		U32		m_MaxScoreboardsToReadFrom;		// maximum number of scoreboards we can request in 1 call to the server
		U32		m_MaxUsersToQuery;				// maximum users we can query in 1 call to the server.
		
		U32		m_NumUsersToQuery;				// how many users are we requesting data for.
		U32		m_NumScoreboardsToQuery;		// how many scoreboards will we be reading from.
	
		void*	m_pRequiredUsers;				// list of all users we are requesting data for
		void*	m_pRequriredScoreboards;		// list of all scoreboards we will bw querying.
		
	
#if defined(LIVE_ON_WINDOWS)
		U8*		m_pResultsBuffer;				// pointer to a buffer to hold data read from the scoreboards.
		void*	m_pAsyncTask;					// asynchronous task used when requesting data from server.	
#else
		bdRemoteTaskRef m_AsyncTask;
#endif
	};
}// end namespace


#endif


