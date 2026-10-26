
#ifndef DynamicObjSUBSYSTEM_H
#define DynamicObjSUBSYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AppInterfaceLayer.h"


	  
class DynamicObjSubsystem : public Audio::Subsystem
{
public:

	enum DynamicObjSubsystemMessages
	{
		eSetCallback = 0,
		eTriggerExhaustAction
	
	};

	DynamicObjSubsystem();
	~DynamicObjSubsystem() { }

	bool		ProcessSystemMessage(const U32 u32MessageId);
	bool		ProcessCustomMessage(const U32 u32MessageId, U32 u32Data = 0, void* pData = 0);
	bool		SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)
	{
		return true;
	}
	void		Update();
	//{
	//}

private:
	Audio::EntityDataMemberHandle	m_hTriggerExhaustActionOut;
};

#endif // DynamicObjSUBSYSTEM_H
