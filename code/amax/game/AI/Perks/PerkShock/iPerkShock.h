#ifndef _PERK_SHOCK_I_H
#define _PERK_SHOCK_I_H

#ifdef _MSC_VER
#pragma once
#define NONINITVTABLE __declspec(novtable)
#endif

#include <ComponentSystem/componentlib.h>

namespace airacingtech
{
	BEGIN_INTERFACE_DEC(IPerkShock)
	
		virtual void	Evaluate() = 0;
		virtual float	GetEvaluationValue() = 0;
		
		
	END_INTERFACE_DEC
}

#endif //_PERK_SHOCK_I_H
