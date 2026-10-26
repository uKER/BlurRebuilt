#ifndef _INTROOUTROSPLINEAI_I_H
#define _INTROOUTROSPLINEAI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>


namespace airacingtech
{

	struct IntroOutroSetSplineParams
	{
		CObjectId	m_splineName;
	};

	struct IntroOutroSetSpeedParams
	{
		float m_speed;
	};

	struct IntroOutroSetTargetPointParams
	{
		Maths::HVec3	m_targetPoint;
	};

	BEGIN_INTERFACE_DEC(IIntroOutroSplineAI)

	public:
		virtual void Update() = 0;
		virtual float GetSteering() const  = 0;
		virtual float GetSpeed() const = 0;

	END_INTERFACE_DEC

}


#endif //_INTROOUTROSPLINEAI_I_H