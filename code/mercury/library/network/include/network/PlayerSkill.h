// ***************************************************************
//  PlayerSkill   version:  1.0   ·  date: 05/22/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Defines structures containing player skill values used 
//			in ranked session. This is truesSkill on Xbox330 and 
//			bdPerformance when using demonwre.
// ***************************************************************

#ifndef NETWORK_PLAYER_SKILL
#define NETWORK_PLAYER_SKILL

#ifdef _MSC_VER
#pragma once
#endif

#include <SystemProfile/SystemProfile.h>

namespace NetPlay
{
	/// Wraps the platform dependent skill ranking systems.
	/** We get one of these structures back per player when we request the skill from the server.
	 *	@see C_PlayerSkillReader.
	 */
	struct PlayerSkill
	{
		PlayerSkill () : m_Skill(0) {}
		S64	m_Skill;		/**< ranges from -1000 to +1000 when using demonware. */
	};

	/// Wraps the data required to update the players skill value on the server.
	/** After each game the client ranks the players in order of performace, this data is passed to the server
	 *	so it can update the individual players skill values.
	 *	@see C_PlayerSkillWriter
	 */
	struct PlayerSkillSubmission
	{
		PlayerSkillSubmission() : m_Rating(0), m_Team(1) {}
		SysProfile::UserIdentifier	m_User;		/**< what player does this data relate to. */
		S32							m_Rating;	/**< game defined rating that dictates how well this player did, higher rating means player did better. */
		S32							m_Team;		/**< if non team game then give each player a unique team,  otherwise specific team identifier that all team members will use. */
	};
}

#endif //NETWORK_PLAYER_SKILL


