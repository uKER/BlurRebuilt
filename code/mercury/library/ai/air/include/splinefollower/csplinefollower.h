#ifndef _SPLINEFOLLOWER_AI_C_H
#define _SPLINEFOLLOWER_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif



#include <splinefollower/isplinefollower.h>

namespace airacingtech
{
	

	BEGIN_COMPONENT_DEC(CSplineFollower, ISplineFollower)

	public:

		void Update();
		
		inline const Btl::Vector<Maths::HVec3, Btl::VectorParameters_Static< 100 > > 	&GetDebugSpline() const
		{
			return m_state.m_samplesDebug;
		}

		inline int GetNearestIndex() const {return m_state.m_nearestIndex;}
		inline int GetFurthestIndex() const {return m_state.m_furthestIndex;}
		inline float GetSteering() const {return m_state.m_steering;}
		inline float GetOffset() const {return m_state.m_offset;}
		inline void SetOffset(float offset){m_state.m_offset = offset;}
		inline Maths::HVec3 &GetConstraintPosition(){return m_state.m_constraintPosition;}
		inline Maths::HVec3 &GetConstraintAngVelocity(){return m_state.m_constraintAngVelocity;}
		inline Maths::HVec3 &GetConstraintVelocity(){return m_state.m_constraintVelocity;}
		inline bool ShouldMove(){return m_state.m_shouldMove;}


		void Update(class ISplineAI *pISpline);
		inline bool IsAtEndIntroOutroSpline(){return m_state.m_endIntroOutroSpline;}

		inline float GetAngle() const {return m_state.m_angle;}
		inline float GetTrackAngle() const {return m_state.m_trackAngle;}

		void UpdateTargetPoint(Maths::HVec3 point);

		int GetOffsetLimit()
		{
			return m_state.m_reachedOffsetLimit;
		}

	private:

		void CachedInterfaces();
		void BuildCubicSpline();
		void BuildCubicSpline2();
		void BuildCubicSpline3();
		int TrackSpline(Maths::HVec3& position);
		int TrackSpline(Maths::HVec3& position, class ISplineAI *pISpline, int nearIndex);
		
		void FollowSpline(Maths::HVec3& pos, Maths::HVec3& heading, Maths::HVec3& p1, Maths::HVec3& p2);
		void ChangedRoute(RouteChangeMSG *pData);
		void CopyData(SteeringControllerInitialiseParams *pData);

		SteeringControllerInitialiseParams m_state;

		bool m_rollingStart;


		
	
	END_COMPONENT_DEC



}

#endif //_SPLINEFOLLOWER_AI_C_H
