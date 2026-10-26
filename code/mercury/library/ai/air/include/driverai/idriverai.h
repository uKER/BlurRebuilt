#ifndef _DRIVER_AI_I_H
#define _DRIVER_AI_I_H

#include <ComponentSystem/componentlib.h>

namespace airacingtech
{

	BEGIN_INTERFACE_DEC(IDriverAI)
	
	public:
	
		virtual void Update() = 0;
		virtual void PreUpdate() = 0;
		virtual void PostUpdate() = 0;

	END_INTERFACE_DEC

	
}

#endif //_DRIVER_AI_I_H
