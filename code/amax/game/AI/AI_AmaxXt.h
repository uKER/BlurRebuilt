#ifndef AI_AMAX_XT_H
#define AI_AMAX_XT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/xtgame.h>
#include <AI_AmaxData.h>

class AIAmaxXt
{
public:

	static AIAmaxXt& Instance() {return m_Instance;}

	AmaxGameData<AIXt*>				m_AItemplates;
	AmaxGameData<AISteeringXt *>	m_AISteeringParams;

	AmaxGameData<AISkillzXt *>		m_AISkillz;
	AmaxGameData<AIPerkPrefsXt *>	m_AIPerkPrefs;
	AmaxGameData<AIPerkSkillzXt *>	m_AIPerkSkillz;

	

	inline AIXt* GetAIPersonalityData(U32 index)
	{
		
		AIXt *pData = m_AItemplates[(GAMEDATA_ID)index];
		assert(pData);
		return pData;
	}

	inline AISteeringXt* GetAISteeringData(U32 index)
	{
		AISteeringXt *pData = m_AISteeringParams[(GAMEDATA_ID)index];
		assert(pData);
		return pData;
	}


	inline AISkillzXt* GetAISkillzData(U32 index)
	{
		AISkillzXt *pData = m_AISkillz[(GAMEDATA_ID)index];
		assert(pData);
		return pData;
	}

	inline AIPerkPrefsXt* GetAIPerkPrefsData(U32 index)
	{
		AIPerkPrefsXt *pData = m_AIPerkPrefs[(GAMEDATA_ID)index];
		assert(pData);
		return pData;
	}

	inline AIPerkSkillzXt* GetAIPerkSkillzData(U32 index)
	{
		AIPerkSkillzXt *pData = m_AIPerkSkillz[(GAMEDATA_ID)index];
		assert(pData);
		return pData;
	}


	inline S32 FindAISkillzID(U32 index)
	{
		AISkillzXt *pData = m_AISkillz.FindIDFromIndex(index);

		BIZ_ASSERT_MSG(pData, "Sorcery Data Problem : Could not find record for AISkillz with index:%d", index);
		
		return pData->ID;
	}

	inline  S32 FindPerkPrefsID(U32 index)
	{ 
		AIPerkPrefsXt *pData = m_AIPerkPrefs.FindIDFromIndex(index);

		BIZ_ASSERT_MSG(pData, "Sorcery Data Problem : Could not find record for PerkPrefs with index:%d", index);

		return pData->ID;
	}

	inline  S32 FindAIPerkSkillzID(U32 index)
	{ 
		AIPerkSkillzXt *pData = m_AIPerkSkillz.FindIDFromIndex(index);

		BIZ_ASSERT_MSG(pData, "Sorcery Data Problem : Could not find record for AIPerkSkillz with index:%d", index);

		return pData->ID;
	}


private:
	static AIAmaxXt m_Instance;

	
};
	



#endif // AI_AMAX_XT_H
