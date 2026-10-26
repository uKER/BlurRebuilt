// ***************************************************************
//  PlayerSkillWriter   version:  1.0   ·  date: 05/27/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Updates the server after a game so the skill of the players
//			can be updated based on their preformance in the game.
//			This used trueSkill on xbo360 and bdPerformcnce on other 
//			platforms.
// ***************************************************************

#ifndef NETWORK_PLAYER_SKILL_WTITER_H
#define NETWORK_PLAYER_SKILL_WTITER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <Network/DemonwareTask.h>

// forward declerations
namespace SysProfile { struct UserIdentifier; }
namespace Scoreboard { class C_ScoreboardWriter; }
namespace NetPlay { struct PlayerSkillSubmission; class C_DemonwareTask; class C_Session; class C_AsyncTask; }
class bdPerformanceValue;

namespace NetPlay
{
	/// Object used for informing the servers after a game how individual players performed.
	/** Uses trueSkill on xbox360 and bdPerformance on othre platforms.
	 *	Xbox360		: Ranked games - All players submit player performances.
					: Standard games - Only host sumbits player performances.
	 *	
	 *	PC - PS3	: Ranked games - Only original host posts player performances. if session migrates the don't post performances.
	 *				: Standard games - No one posts player performances as it's not used in this mode.
	 *
	 *	On all platforms we must wait for the player performances to finish writing before we end the online session otherwise the server
	 *	may not have all the data required. On xbox360 this will may cause the arbitration to fail and the performances to be discarded. 
	 */
	class C_PlayerSkillWriter
	{
	public :
		explicit C_PlayerSkillWriter();
		virtual ~C_PlayerSkillWriter();

		/// Call once to allocate dynamic resources required to write the skill data to the servers.
		/**	@param maxUsersToWrite how many players will we be updating skill for.
		 *	@return true if successful or false if not.
		 */
		bool Initialise(const U32 maxUsersToWrite); 

		/// Attempt to write a list of player performace restlts back the server for evaluation. 
		/**	@param session the current online session, NOTE : This session must stay valid until the player skill write has finished,
		 *	or the came calls StopAndClear().
		 *	@param pSkillData pointer to a list of correctly configured PlayerSkillSubmission structures who we wish to write.
		 *	@param numUsers how many users does pSKillData point to.
		 *	@param gameMode which game mode should we upate the players skill for. On xbox360 this is ignored since the trueskill is updated using the current
		 *		   system deed gamemode. 
		 *	@return true if successfull of false if not.
		 */
		bool Start(const C_Session& session, const PlayerSkillSubmission* pSkillData, const U32 numUsers, const U32 gameMode);

		/// Continues the asynchronous procedure of writing player performance skill data to the servers.
		/**	@see Start()
		 *	@param errorOccurred check this once function returns true.
		 *	@return true when read operation has finished else false.
		 */
		bool Continue(bool& errorOccurred);

		/// Are we currently in the process of writing to the servers.
		bool IsWriting(void) const {return m_Writing;}

		/// Stops any current server write task.
		void StopAndCleanup(void);

	private:
		C_PlayerSkillWriter(const C_PlayerSkillWriter& original);			// private copy constructor
		C_PlayerSkillWriter& operator = (const C_PlayerSkillWriter rhs);	// private un coded assignment operator
		void CleanUp(void);

	private:
		bool	m_Initialised;
		bool	m_Writing;
		U32		m_MaxUsersToWrite;
		U32		m_GameModeId;
		U32		m_CurrentIndex;

		bdRemoteTaskRef m_AsyncTask;
		bdPerformanceValue* m_pWriteCache;
	};
}

#endif //NETWORK_PLAYER_SKILL_WTITER_H



