#ifndef _AVOIDANCE_BASIC_C_H
#define _AVOIDANCE_BASIC_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <avoidance/iavoidance.h>

namespace airacingtech
{
	class IData;
	class RouteChangeMSG;
	class IGameServerTime;

	


	BEGIN_COMPONENT_DEC(CAvoidanceBasic, IAvoidance)

	public:

		void CachedInterfaces();
		void PreUpdate();
		void Update();

		enum AVOIDANCE_SIDE_SWIPE_STATE
		{
			AVOIDANCE_NORMAL,
			AVOIDANCE_SIDEWIPE_START,
			AVOIDANCE_SIDEWIPE_END,
		};

		int GetOverallSituation() const {return m_overallSituation;}

		AvoidCar *GetAvoidCar(int whichOne) 
		{
			AvoidCar *pAvoidCar = 0;

			switch(whichOne)
			{
			case 0:
				{
					pAvoidCar = &m_InfrontDirectly;
				}
				break;

			case 1:
				{
					pAvoidCar = &m_InfrontLeft;
				}
				break;

			case 2:
				{
					pAvoidCar = &m_Infront;
				}
				break;

			case 4:
				{
					pAvoidCar = &m_InfrontRight;
				}
				break;

			case 8:
				{
					pAvoidCar = &m_AlongsideLeft;
				}
				break;

			case 16:
				{
					pAvoidCar = &m_AlongsideRight;
				}
				break;

			case 32:
				{
					pAvoidCar = &m_Behind;
				}
				break;


			default:
				break;
			}

			return pAvoidCar;
		}
	

	private:
		IData						*m_pIData;
		IGameServerTime				*m_pIGameServerTime;
		Btl::Vector<IData *>		m_AIlist;
		AvoidCar				m_InfrontDirectly; // not relative to the spline...
		AvoidCar				m_Infront;
		AvoidCar				m_InfrontLeft;
		AvoidCar				m_InfrontRight;
		AvoidCar				m_AlongsideLeft;
		AvoidCar				m_AlongsideRight;
		AvoidCar				m_Behind;


		float			m_actualSplineXDist;
		float			m_TargetOffset;
		float			m_CurrentTargetOffset;
		bool			m_returnToLine;
		float			m_returnToLineSeconds;
		float			m_leftDistance;
		float			m_rightDistance;
		float			m_startRouteChangeTime;
		float			m_returnToOffset;// Racing line is 0.0f;
		float			m_RandomOffsetTime;


		int				m_overallSituation;
		int				m_usedSituation;

		AVOIDANCE_SIDE_SWIPE_STATE	m_sideSwipeState;
		float						m_sideSwipeTime;
		float						m_resetSideSwipeTime;

		void SetAhead(AvoidCar &avoidV);
		void SetAlongside(AvoidCar &avoidV);
		void SetBehind(AvoidCar &avoidV);

		void AvoidVehicles();
		void AvoidanceAction(int situation);
		void AvoidInfront(AvoidCar *pAvoidCar);
		void AvoidInfrontCorner(AvoidCar *pAvoidCar);
		void AvoidInfrontStraight(AvoidCar *pAvoidCar);
		void AvoidInfrontLeft(AvoidCar *pAvoidCar);
		void AvoidInfrontRight(AvoidCar *pAvoidCar);
		void AvoidAlongsideLeft(AvoidCar *pAvoidCar);
		void AvoidAlongsideRight(AvoidCar *pAvoidCar);

		void TurnRight(AvoidCar *pAvoidCar, float xMetres);
		void TurnLeft(AvoidCar *pAvoidCar, float xMetres);

		void SetTargetOffset(float targetoffset, float currentTargetDist, float secs);
		void ReturnToLine();
		void ChangedRoute(RouteChangeMSG *pData);
		float Prediction(AvoidCar *pAvoid, float tTime, float radiusA, float radiusB);

	END_COMPONENT_DEC

}

#endif //_AVOIDANCE_BASIC_C_H
