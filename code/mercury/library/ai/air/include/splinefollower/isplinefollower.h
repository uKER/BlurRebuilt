#ifndef _SPLINEFOLLOWER_AI_I_H
#define _SPLINEFOLLOWER_AI_I_H

#ifdef _MSC_VER
#pragma once
#define NONINITVTABLE __declspec(novtable)
#endif

#include <ComponentSystem/componentlib.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

namespace airacingtech
{

	class ITrackDataAI;
	class IData;
	class IPIDControllerAI;
	class IVehicleState;
	class VehicleState;
	class ISplineTracker;
	class RouteChangeMSG;
	class IGameServerTime;
	class ISplineAI;

	struct SteeringControllerInitialiseParams
	{
		bool	m_UseVelocity;
		float	m_Blend;
		float	m_ForwardsDist;
		float	m_AngVelSteering;
		float	m_AlignedSteering;
		float	m_SteeringRate;
		float	m_SteeringTimeStep;
		float	m_LerpValue;
		float	m_DelayValue;
		float	m_SmoothTimeStep;

		bool	m_UseOffset;
		float	m_OffsetValue;

		// below are not to be copied.....
		Btl::Vector<Maths::HVec3, Btl::VectorParameters_Static< 32 > >  m_ctrlPoints;
		Btl::Vector<Maths::HVec3, Btl::VectorParameters_Static< 100 > >  m_samplesDebug; // might be too big ???
		//Btl::Vector<Maths::HVec3>  m_samplesDebug; // debug only....
		int		m_nearestIndex;
		int		m_furthestIndex;
		float	m_steering;
		float	m_angle;
		float	m_LastSteering;
		float	m_offset;

		Smoother<Maths::HVec3>	 *m_pHeadingSmoother;

		IGameServerTime			*m_pIGameServerTime;
		float					m_startRouteChangeTime;
		float					m_startRouteChangeTimeThreshold;


		Maths::HVec3			m_constraintPosition;
		Maths::HVec3			m_constraintVelocity;
		Maths::HVec3			m_constraintAngVelocity;
		float					m_startConstraint;
		bool					m_shouldMove;
		bool					m_endIntroOutroSpline;


		ITrackDataAI		*m_pTrackData;
		IData				*m_pIData;
		IPIDControllerAI	*m_pIPIDController;
		IVehicleState		*m_pIVehicleState;
		VehicleState		*m_pVehState;
		ISplineTracker		*m_pSplineTracker;




		float				m_distToSpline;
		float				m_trackAngle;
		float				m_startRaceTime;


		Maths::HVec3		m_targetSplinePoint[2];
		bool				m_validTarget;
		int					m_reachedOffsetLimit;
	};


	BEGIN_INTERFACE_DEC(ISplineFollower)

	public:

		virtual void Update() = 0;
		virtual const Btl::Vector<Maths::HVec3, Btl::VectorParameters_Static< 100 > > &GetDebugSpline() const = 0;

		virtual int GetNearestIndex() const = 0;
		virtual int GetFurthestIndex() const = 0;
		virtual float GetSteering() const = 0;
		virtual float GetOffset() const = 0;
		virtual void SetOffset(float offset) = 0;
		virtual Maths::HVec3 &GetConstraintPosition() = 0;
		virtual Maths::HVec3 &GetConstraintAngVelocity() = 0;
		virtual Maths::HVec3 &GetConstraintVelocity() = 0;
		virtual bool ShouldMove() = 0;
		virtual float GetAngle() const = 0;
		virtual float GetTrackAngle() const = 0;

		virtual void Update(class ISplineAI *pISpline) = 0;
		virtual bool IsAtEndIntroOutroSpline() = 0;

		virtual void UpdateTargetPoint(Maths::HVec3 point) = 0;
		virtual int GetOffsetLimit() = 0;

	END_INTERFACE_DEC


}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

#endif //_SPLINEFOLLOWER_AI_I_H
