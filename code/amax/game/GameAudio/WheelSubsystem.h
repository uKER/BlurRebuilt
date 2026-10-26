
#ifndef WHEELSUBSYSTEM_H
#define WHEELSUBSYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AppInterfaceLayer.h"


	  
class WheelSubsystem : public Audio::Subsystem
{
public:

	enum WheelSubsystemMessages
	{
		eSetCallback = 0,
		eFLWheelDry_AudioResumed,
		eFRWheelDry_AudioResumed , 
		eRLWheelDry_AudioResumed , 
		eRRWheelDry_AudioResumed , 
		eFLWheelOffRoad_AudioResumed,
		eFRWheelOffRoad_AudioResumed,
		eRLWheelOffRoad_AudioResumed,
		eRRWheelOffRoad_AudioResumed,
		eFLWheelDry_AudioStopped , 
		eFRWheelDry_AudioStopped , 
		eRLWheelDry_AudioStopped , 
		eRRWheelDry_AudioStopped , 
		eFLWheelOffRoad_AudioStopped,
		eFRWheelOffRoad_AudioStopped,
		eRLWheelOffRoad_AudioStopped,
		eRRWheelOffRoad_AudioStopped,
		eAIWheelOffRoad_AudioStopped,
		eAIWheelOffRoad_AudioResumed,
		eAIWheelDry_AudioStopped ,
		eAIWheelDry_AudioResumed

		//eTriggerExhaustAction
	
	};

	WheelSubsystem();
	~WheelSubsystem() { }

	bool		ProcessSystemMessage(const U32 u32MessageId);
	bool		ProcessCustomMessage(const U32 u32MessageId, U32 u32Data = 0, void* pData = 0);
	bool		SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)
	{
		return true;
	}
	void		Update()
	{
	}

private:
	//Audio::EntityDataMemberHandle	m_hTriggerExhaustActionOut;
	bool m_AllowWheelSuspending;
};

#endif // WheelSUBSYSTEM_H
