#ifndef _RUBBERBANDINGAI_I_H
#define _RUBBERBANDINGAI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>


namespace airacingtech
{

	class AIEventBalancingData;
	class AIEventSegmentData;

	class RubberBandingIndividualBalancingParams
	{
	public:

		RubberBandingIndividualBalancingParams(){};
		~RubberBandingIndividualBalancingParams(){};
		
		int		m_To;
		float	m_Distance;
		float	m_TrackSpeedPercentageMax;
		float   m_TrackSpeedPercentageMin;
		bool	m_Enable;

		float	m_CatchUpRateMin;
		float	m_CatchUpRateMax;
		float	m_CatchDistanceMin;
		float	m_CatchDistanceMax;

		float	m_SlowDownRateMin;
		float	m_SlowDownRateMax;
		float	m_SlowDownDistanceMin;
		float	m_SlowDownDistanceMax;

		int		m_MistakeAttempts;
		int		m_BlockAttempts;
		int		m_HitPlayerAttempts;

	};

	class  RubberBandingInitialiseParams
	{

	public:
		RubberBandingInitialiseParams(){};
		~RubberBandingInitialiseParams(){};

		Btl::Vector<RubberBandingIndividualBalancingParams *>	m_RaceSegments;

	};


	BEGIN_INTERFACE_DEC(IRubberbandingAI)

	public:

		virtual void Update() = 0;
		virtual float GetFzCatchup()= 0;
		virtual float GetFzSlowDown()= 0;
	
		virtual float GetDirectTargetSpeed() const = 0;
		virtual void SetDirectTargetSpeed(float v) = 0;

		virtual AIEventBalancingData *GetEventBalancingData() = 0;
		virtual AIEventSegmentData *GetEventBalancingSegmentData() = 0;

		virtual void	SetDriverIndexOverride(int index) = 0;
	
	END_INTERFACE_DEC

}


#endif //_RUBBERBANDINGAI_I_H