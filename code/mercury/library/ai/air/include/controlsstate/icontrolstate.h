#ifndef _CONTROL_STATE_I_H
#define _CONTROL_STATE_I_H

#ifdef _MSC_VER
#pragma once
#define NONINITVTABLE __declspec(novtable)
#endif

#include <ComponentSystem/componentlib.h>
#include <controlsstate/controlstate.h>

namespace airacingtech
{

	BEGIN_INTERFACE_DEC(IControlState)
	
		virtual airacingtech::ControlState *GetState() = 0;
		virtual  void SetPreviousState()  = 0;
		virtual void PreUpdate() = 0;

	END_INTERFACE_DEC
}

#endif //_CONTROL_STATE_I_H
