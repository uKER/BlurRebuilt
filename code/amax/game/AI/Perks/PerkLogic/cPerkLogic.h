#ifndef _PERKLOGIC_AI_C_H
#define _PERKLOGIC_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ai/Perks/PerkLogic/iPerkLogic.h>
#include <ai/AI_AmaxSetup.h>

namespace airacingtech
{
	class IData;
	class IPerkSystem;
	class IGameServerTime;
	class IPerkSkillUsage;
	class ITrackDataAI;
	class RouteInfo;

	BEGIN_COMPONENT_DEC(CPerkLogicAI, IPerkLogicAI)

	public:

		void CachedInterfaces();
		void PreUpdate();
		void Update();
		void PostUpdate();
		void HandleTrigger();
		bool CheckForMines();
		bool CheckForShocks();
		int GetNearestPerkTypeInfront(int myIndex, E_PICKUP_TYPE pickup, int &routeIndex, float &racingOffset, float &zDist, float radius);
		int FindNearest(int Index, Maths::HVec3 pos, const airacingtech::RouteInfo *pRouteInfo);
	
		struct TargetPerk
		{
			int		m_nearPerk;
			int		m_nearPerkRoute;	
			float	m_TimeStamp;
			float	m_prefernceValue;
			int		m_PerkType;
			float	m_InitialDist;
		};


	private:

		IData				*m_pIData;
		IPerkSystem			*m_pIPerkSystem;
		IGameServerTime		*m_pIGameServerTime;
		IPerkSkillUsage		*m_pIPerkSkillUsage;
		float				m_lastReceived;
		ITrackDataAI		*m_pITrackData; 

		TargetPerk			m_TargetPerk;

	END_COMPONENT_DEC
}



#endif //_PERKLOGIC_AI_C_H