// ***************************************************************
//  PlayerSkillCombiner   version:  1.0   ·  date: 06/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Combines multiple player skill values into a single 
//			skill value that represents a group of players.
// ***************************************************************

#ifndef NETWORK_PLAYER_SKILL_COMBINER_H
#define NETWORK_PLAYER_SKILL_COMBINER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Network/SystemHeaders.h>
#include <Network/Network.h>
#include <Network/PlayerSkill.h>

namespace NetPlay
{
	/// Combine a bunch of player skill into a single skill that represents the whole group.
	/** @param maxPeers set this to the maximum number of skills you will ever pass to the Combine() method.
	 */
	template <U32 maxPeers> 
	class C_PlayerSkillCombiner
	{
	public:
		C_PlayerSkillCombiner();
		virtual ~C_PlayerSkillCombiner();  

		/// Combine all supplied skill values into one aggerate skill.
		/** @param pSkills pointer to a list of skills to combine.
		 *	@param numSkills how many skills will we be combining. must be less or equal to maxPeers when the template was created.
		 *	@return combined skill. if an error occured then this will be a defaulted skill value instead.
		 */
		PlayerSkill Combine(const PlayerSkill* pSkills, const U32 numSkills);
	};

	// constructor
	template <U32 maxPeers> 
	C_PlayerSkillCombiner<maxPeers>::C_PlayerSkillCombiner()
	{}


	// destructor
	template <U32 maxPeers>
	C_PlayerSkillCombiner<maxPeers>::~C_PlayerSkillCombiner()
	{}

	template <U32 maxPeers>
	PlayerSkill C_PlayerSkillCombiner<maxPeers>::Combine(const PlayerSkill* pSkills, const U32 numSkills)
	{
		if (0 == numSkills || NULL == pSkills)
		{
			PlayerSkill defaultSkill; 
			LOGERROR(g_Log, "C_PlayerSkillCombiner<maxPeers>::Combine() - Invalid parameters..");
			return defaultSkill; // return default skill
		}

		if (numSkills > maxPeers)
		{
			PlayerSkill defaultSkill; 
			LOGERROR(g_Log, "C_PlayerSkillCombiner<maxPeers>::Combine() - Increase maxPeers when you create template, supplied more skill pointers than we can use internally.");
			return defaultSkill; // return default skill
		}

		PlayerSkill out; // default skill..

		// demonware simply take average of all skills..
		S64 combined = 0;
		for (U32 i=0; i<numSkills; i++)
			combined += pSkills[i].m_Skill;

		out.m_Skill = static_cast<S64>(combined / numSkills);

		return out;
	}
} // end namespace NetPlay
#endif //#ifndef NETWORK_PLAYER_SKILL_COMBINER_H
