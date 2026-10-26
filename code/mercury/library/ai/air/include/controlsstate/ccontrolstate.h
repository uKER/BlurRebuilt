#ifndef _CONTROL_STATE_C_H
#define _CONTROL_STATE_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <controlsstate/icontrolstate.h>




namespace airacingtech
{
	BEGIN_COMPONENT_DEC(CControlState, IControlState)
	
		inline airacingtech::ControlState *GetState() {return &m_State;}
		inline  void SetPreviousState() {m_StatePrevious = m_State;}

		void PreUpdate();
	private:

		airacingtech::ControlState	m_State;
		airacingtech::ControlState	m_StatePrevious;

	END_COMPONENT_DEC
}




#endif //_CONTROL_STATE_C_H
