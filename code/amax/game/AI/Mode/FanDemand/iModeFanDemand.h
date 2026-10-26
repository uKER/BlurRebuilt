#ifndef _MODE_FANDEMAND_AI_I_H
#define _MODE_FANDEMAND_AI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>


namespace airacingtech
{

	BEGIN_INTERFACE_DEC(IModeFanDemand)

	public:

		virtual void Update() = 0;


	END_INTERFACE_DEC


}


#endif //_MODE_FANDEMAND_AI_I_H