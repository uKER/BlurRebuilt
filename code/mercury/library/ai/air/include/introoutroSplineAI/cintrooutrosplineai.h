#ifndef _INTROOUTROSPLINEAI_C_H
#define _INTROOUTROSPLINEAI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <introoutrosplineai/iintrooutrosplineai.h>

namespace airacingtech
{
	
	class ISplineAI;
	class IData; 

	BEGIN_COMPONENT_DEC(CIntroOutroSplineAI, IIntroOutroSplineAI)

	public:

		enum STATE
		{
			STATE_NONE = 0,
			STATE_SPLINE_FOLLOWING,
			STATE_FOLLOW_TARGET_POINT
		};

		void Update();
		float GetSteering() const ;
		float GetSpeed() const {return m_speed;}
		
	private:

		float				m_steering;
		float				m_speed;
		Maths::HVec3		m_targetPoint;
		class ISplineAI		*m_pISpline;
		class IData         *m_pIData; 

		STATE				m_state;

		void CachedInterfaces();


	
	END_COMPONENT_DEC
}


#endif //_INTROOUTROSPLINEAI_C_H