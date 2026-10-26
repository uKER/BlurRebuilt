#ifndef _PID_CONTROLLER_I_H
#define _PID_CONTROLLER_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>

namespace airacingtech
{

	struct PIDControllerInitialiseParams
	{
		float					p;
		float					i;
		float					d;
		float					m_timeStep;
		int						m_numSlots;
	};

	BEGIN_INTERFACE_DEC(IPIDControllerAI)
	
	public:
	
		virtual void Record(float error, float timestep) = 0;
		virtual float GetOutput() = 0;
		virtual float GetTimeStep() const = 0;
		virtual void SetCoefficients(float p, float i, float d) = 0;
		
	END_INTERFACE_DEC
	
}

#endif //_PID_CONTROLLER_I_H
