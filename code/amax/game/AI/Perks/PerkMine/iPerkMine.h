#ifndef _PERK_MINE_I_H
#define _PERK_MINE_I_H

#ifdef _MSC_VER
#pragma once
#define NONINITVTABLE __declspec(novtable)
#endif

#include <ComponentSystem/componentlib.h>

namespace airacingtech
{
	struct TargetEvaluation;

	BEGIN_INTERFACE_DEC(IPerkMine)
	
		virtual void				Evaluate() = 0;

		virtual float				GetDropEvaluation() = 0;
		virtual TargetEvaluation*	GetVehicleEvaluation(int idx) = 0;
		virtual TargetEvaluation*	GetProjectileEvaluation(int idx) = 0;
		virtual int					GetNumProjectiles() = 0;

	END_INTERFACE_DEC
}

#endif //_PERK_MINE_I_H
