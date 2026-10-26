#ifndef _VEHICLELOGIC_AI_I_H
#define _VEHICLELOGIC_AI_I_H

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

	class LogicState;

	BEGIN_INTERFACE_DEC(IVehicleLogic)

	public:
		virtual LogicState *GetLogicStateBuffered() = 0;
		virtual LogicState *GetLogicState()  = 0;
		virtual LogicState *GetResetLogicState()= 0;
		virtual void StoreResetLogicState() = 0;
		virtual void ResetResetLogicState() = 0;
		virtual void Update() = 0;

	
	END_INTERFACE_DEC

}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

#endif //_VEHICLELOGIC_AI_I_H
