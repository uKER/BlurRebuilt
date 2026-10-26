#ifndef _ACCELBRAKECONTROLLER_AI_I_H
#define _ACCELBRAKECONTROLLER_AI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>

namespace airacingtech
{

	BEGIN_INTERFACE_DEC(IAccelBrakeController)
	
	public:

		virtual void Update() = 0;
		virtual void SetControlsDirectly(bool d, float a, float b, float s, bool h) = 0 ;

		enum REV_STATE
		{
			REV_GENTLE = 0,
			REV_BLIP,
			REV_FULL,
			REV_NONE
		};

	END_INTERFACE_DEC

	
}

#endif //_ACCELBRAKECONTROLLER_AI_I_H
