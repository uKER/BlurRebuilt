#ifndef _PERK_GAMEBRAIN_I_H
#define _PERK_GAMEBRAIN_I_H

#ifdef _MSC_VER
#pragma once
#define NONINITVTABLE __declspec(novtable)
#endif

#include <ComponentSystem/componentlib.h>

namespace airacingtech
{
	BEGIN_INTERFACE_DEC(IGameBrain)
	

		virtual void Update() = 0;
		virtual void PreUpdate() = 0;
		virtual void PostUpdate() = 0;

		
	END_INTERFACE_DEC
}

#endif //_PERK_GAMEBRAIN_I_H
