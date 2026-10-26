// ***************************************************************
//  PlayerSkillReader   version:  1.0   ·  date: 05/22/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Reads the skill of a list of players from the platform 
//			servers. Usefull while matchmaking in ranked games.
// ***************************************************************

#ifndef NETWORK_PLAYER_SKILL_READER_H
#define NETWORK_PLAYER_SKILL_READER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/DemonwareTask.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

// forward declerations
namespace SysProfile { struct UserIdentifier; }
namespace Scoreboard { class C_ReaderByUsers; }
namespace NetPlay { struct PlayerSkill; class C_DemonwareTask; }
class bdPerformanceValue;

namespace NetPlay
{
	/// Object used for reading the ranked skill of a list of players.
	/** This works on all platforms, on Xbox360 it will get the trueskill, when
	 *	using demonware it will get the bdPerformance of the players.
	 */
	class C_PlayerSkillReader
	{
	public :
		explicit C_PlayerSkillReader();
		virtual ~C_PlayerSkillReader();

		/// Call once to allocate dynamic resources required to read from scoreboards.
		/**	@param maxUsersToQuery how many players do we with to get skill data for in server request.
		 *	@return true if successful or false if not.
		 */
		bool Initialise(const U32 maxUsersToQuery); 

		/// Attempt to request a list of players skill from the platform servers. 
		/**	@param pUsers pointer to a list of correctly configured SysProfile::UserIdentifier structures who we wish to get the skill for.
		 *	@param numUsers how many users does pUSers point to.
		 *	@param gameMode which game mode should we query the players skill from. for Xbox360 we should pass the scoreboard Id for the required game mode.
		 *	On Demonware this parameter should match against what was written via C_PlayerSkillWriter::Start().
		 *	@return true if successfull of false if not.
		 */
		bool Start(const SysProfile::UserIdentifier* pUsers, const U32 numUsers, const U32 gameMode);

		/// Continues the asynchronous procedure of reading players skill data from the servers.
		/**	@see Start()
		 *	@param errorOccurred check this once function returns true.
		 *	@return true when read operation has finished else false.
		 */
		bool Continue(bool& errorOccurred);

		/// Are we currently in the process of reading from the servers.
		bool IsReading(void) const {return m_Reading;}

		/// After requesting data from servers get access to an individual players skill data.
		/** It's up to the game to make sure they only get results for players that were passed into the 
		 *	Start() function otherwise the skill data returned will be default.
		 *	@param user what player do you with to get from the server results. This user MUST have been 
		 *	included in the players passed into the Start() function.
		 *	@param skill will contain the players skill rating.
		 *	@return true if the skill rating is valid or false if not.
		 */
		bool GetSkillFromResults(const SysProfile::UserIdentifier& user, PlayerSkill& skill);

		/// Call this function once you have finished accessing the results from a Start() request.
		/** Releases the dynamic results buffer.
		 */
		void StopAndCleanup(void);

	private:
		C_PlayerSkillReader(const C_PlayerSkillReader& original);			// private copy constructor
		C_PlayerSkillReader& operator = (const C_PlayerSkillReader rhs);	// private un coded assignment operator
		void CleanUp(void);

	private:
		bool	m_Initialised;
		bool	m_Reading;
		bool	m_ResultsValid;
		U32		m_MaxUsersToQuery;
		U32		m_GameModeId;

		bdRemoteTaskRef m_AsyncTask;
		bdEntityID* m_pEntities;	
		bdPerformanceValue* m_pResults;
		U32	m_NumResults;
	};
}

#endif //NETWORK_PLAYER_SKILL_READER_H


