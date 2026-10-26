#ifndef _PERK_NITRO_I_H
#define _PERK_NITRO_I_H

#ifdef _MSC_VER
#pragma once
#define NONINITVTABLE __declspec(novtable)
#endif

#include <ComponentSystem/componentlib.h>

namespace airacingtech
{
	BEGIN_INTERFACE_DEC(IPerkNitro)
	
		virtual void Evaluate() = 0;
		
	END_INTERFACE_DEC
}

#endif //_PERK_NITRO_I_H
