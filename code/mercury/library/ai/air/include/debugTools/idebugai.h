#ifndef _DEBUG_AI_I_H
#define _DEBUG_AI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>

namespace airacingtech
{

	BEGIN_INTERFACE_DEC(IDebugAI)
	
	public:
		
		virtual void Update() = 0;

	END_INTERFACE_DEC
	
}

#endif //_DEBUG_AI_I_H
