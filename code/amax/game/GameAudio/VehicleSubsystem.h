
#ifndef VEHICLESUBSYSTEM_H
#define VEHICLESUBSYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AppInterfaceLayer.h"


	  
class VehicleSubsystem : public Audio::Subsystem
{
public:

	enum VehicleSubsystemMessages
	{
		eSetCallback = 0,
		eTriggerExhaustAction
	
	};

	VehicleSubsystem();
	~VehicleSubsystem() { }

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
	Audio::EntityDataMemberHandle	m_hTriggerExhaustActionOut;
};

#endif // VEHICLESUBSYSTEM_H
