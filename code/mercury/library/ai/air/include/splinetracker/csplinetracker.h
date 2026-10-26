#ifndef _SPLINETRACKER_AI_C_H
#define _SPLINETRACKER_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <splinetracker/isplinetracker.h>
#include <utilityai/KdTree.h>
#include <trackdata/trackdata.h>


namespace airacingtech
{
	class AITrackDataRoutes;
	class IGameServerTime;
	class RouteInfo;
	class ITrackDataAI;
	class IData;

	BEGIN_COMPONENT_DEC(CSplineTracker, ISplineTracker)

	public:

		
		inline ResetInfo *GetResetInfo() {return &m_LastValidInfo;}
	
		ResetInfo			m_LastValidInfo;

		void Update();

		void StoreValidInfoForReset(RouteInfo *pRouteInfo);


		inline const RouteInfo *GetRouteInfo(int index) const 
		{
			assert(index >=0 && index < m_RouteInfo.Size());
			return &m_RouteInfo[index];
		}

		inline const Btl::Vector<RouteInfo> &GetRouteInfo() const {return m_RouteInfo;};
		inline const RouteInfo *GetCurrentRoute() const {return m_pCurrentRoute;}
		inline const int GetCurrentLane() const {return m_CurrentLane;}
		inline const float GetCurrentOffset() const {return m_currentOffset;}

		inline const RouteInfo *GetLogicRoute() const {return m_pRouteInfoLogic;}
		inline const RouteInfo *GetLogicInter1() const {return m_pInter1Route;}
		inline const RouteInfo *GetLogicInter2() const {return m_pInter2Route;}
		inline const RouteInfo *GetLogicStart() const {return m_pStartRoute;}
		inline const RouteInfo *GetLogicFinish() const {return m_pFinishRoute;}

		void UpdateMultiRoutes();


		inline bool InStartZone(){return m_pStartRoute->m_in;}
		inline bool InInter1(){return m_pInter1Route->m_in;}
		inline bool InInter2(){return m_pInter2Route->m_in;}
		inline bool InFinish(){return m_pFinishRoute->m_in;}

	private:
		CSplineTracker(const CSplineTracker& other);
		CSplineTracker& operator = (const CSplineTracker& other);

		ITrackDataAI	*m_pTrackData;
		IData			*m_pIData;
		IGameServerTime	*m_pIGameServerTime;
		RouteInfo		*m_pRouteInfoLogic;

		RouteInfo		*m_pStartRoute;
		RouteInfo		*m_pInter1Route;
		RouteInfo		*m_pInter2Route;
		RouteInfo		*m_pFinishRoute;

	
		void ChangeRoute(RouteInfo *pRouteInfo,  bool leaving);
		void ChangeRouteUsingJunctions(RouteInfo *pRouteInfo,  bool leaving);
		int DecideLane(RouteInfo *pRouteInfo);
		int NearestLane(RouteInfo *pRouteInfo);
		//void DistanceToSplines(RouteInfo *pRouteInfo);
		void RecordDistanceToSplines();
		void WorkOutOffsetToRacingSpline(RouteInfo *pRouteInfo);
		void WorkOutPositionAroundTheSpline(RouteInfo *pRouteInfo);
		
		void Setup();
		void CachedInterfaces();
		void FindNearestIndexFromRoute(AITrackDataRoutes *pRoute, int &index, float &paramDist);
		void FindNearestIndexFromRouteCached(AITrackDataRoutes *pRoute, int &index, int routeIndex, float &paramDist);
		void LogicMessages(RouteInfo *pRouteInfo, int entering);

		void UpdateMultiRoutesOpt();
		

		inline int FindRouteByName(CHash findName)
		{
			AITrackDataRoutes *pRoute = 0;
			for(int i=0; i<m_RouteInfo.Size(); i++)
			{
				pRoute = m_RouteInfo[i].m_pRoute;
				if (pRoute->m_Name == findName)
				{
					return i;
				}
			}
			return -1;
		}

		// Temporary......
		inline int FindRouteByPointer(AITrackDataRoutes *pSearch)
		{
			assert(pSearch);
			for(int i=0; i<m_RouteInfo.Size(); i++)
			{
				AITrackDataRoutes *pRoute = m_RouteInfo[i].m_pRoute;
				if (pRoute == pSearch)
				{
					return i;
				}
			}
			return -1;
		}

		inline bool IsInRoute(int RouteIndex)
		{
			assert(RouteIndex >= 0 && RouteIndex < m_RouteInfo.Size());
			return m_RouteInfo[RouteIndex].m_in;
		}

		inline bool IsInRoute(const RouteInfo *pRouteInfo, int &RouteIndex)
		{
			assert(pRouteInfo);
			bool inRoute = false;	

			for(int i=0; i<m_RouteInfo.Size(); i++)
			{
				if (m_RouteInfo[i].m_in == true)
				{
					if (m_RouteInfo[i].m_pRoute == pRouteInfo->m_pRoute)
					{
						RouteIndex = i;
						inRoute = true;
						break;
					}
				}
			}

			return inRoute;
		}

		inline void GetValidPosDir(Maths::HVec3 &pos, Maths::HVec3 &dir, float &speed)
		{
			pos = m_LastValidInfo.m_position;//m_validPosition;
			dir = m_LastValidInfo.m_SplineDirection;//m_validSplineDirection;
			speed = m_LastValidInfo.m_speed;
		}

		void StoreLastValidPositionAndDirection(RouteInfo *pRouteInfo);
		
		Maths::HVec3 m_validPosition;
		Maths::HVec3 m_validSplineDirection;
		Maths::HVec3 m_validSplinePosition;



		Btl::Vector<int>		m_splineIndexList;
		Btl::Vector<RouteInfo>	m_RouteInfo;
		Btl::Vector<RouteInfo>	m_RouteInfoBuffered;
		int						m_CurrentLane;
		RouteInfo				*m_pCurrentRoute;
		RouteInfo				*m_PreviousRoute;
		float					m_currentOffset;
		bool					m_distribute_cpu_load;
		float					m_currentTime;
		float					m_lastTime;
		

	END_COMPONENT_DEC



}

#endif //_SPLINETRACKER_AI_C_H
