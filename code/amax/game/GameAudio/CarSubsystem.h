#ifndef _CAR_SUBSYSTEM_H_
#define _CAR_SUBSYSTEM_H_

#ifdef _MSC_VER
#pragma once
#endif


#include "AppInterfaceLayer.h"


class CarSubsystem : public Audio::Subsystem
{
public:

	CarSubsystem() { }
	~CarSubsystem() { }

	bool		SendMessage(const U32 u32MessageId)
	{
		return true;
	}
	bool		SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)
	{
		return true;
	}
	void		Update()
	{
	}
};

#endif	//	_CAR_SUBSYSTEM_H_
