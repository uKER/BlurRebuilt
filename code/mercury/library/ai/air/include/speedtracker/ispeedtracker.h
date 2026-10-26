#ifndef _SPEEDTRACKER_AI_I_H
#define _SPEEDTRACKER_AI_I_H

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

	BEGIN_INTERFACE_DEC(ISpeedTracker)

	public:

		virtual void Update() = 0;
		virtual void CalculateFrictions(float &speed_Coeff, float &decel_co_eff) = 0;


	END_INTERFACE_DEC


}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

#endif //_SPEEDTRACKER_AI_I_H
