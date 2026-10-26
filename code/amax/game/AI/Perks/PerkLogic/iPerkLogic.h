#ifndef _PERKLOGIC_AI_I_H
#define _PERKLOGIC_AI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>


namespace airacingtech
{

	BEGIN_INTERFACE_DEC(IPerkLogicAI)

	public:

		virtual void Update() = 0;
		virtual void PreUpdate() = 0;
		virtual void PostUpdate() = 0;

	
	END_INTERFACE_DEC


}


#endif //_PERKLOGIC_AI_I_H