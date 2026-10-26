#ifndef _RECOVERYAI_I_H
#define _RECOVERYAI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>


namespace airacingtech
{
	
	BEGIN_INTERFACE_DEC(IRecoveryAI)

	public:
		virtual void Update() = 0;
		virtual void SetState(int state)= 0;
		virtual ResetInfo *GetResetInfo() = 0;
		virtual int GetRuleFired() = 0;
		virtual void SetLastPollTime(float t) = 0;
		virtual float GetLastPollTime() = 0;
		

	END_INTERFACE_DEC

}


#endif //_RECOVERYAI_I_H