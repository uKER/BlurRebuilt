
#ifndef VOSUBSYSTEM_H
#define VOSUBSYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AppInterfaceLayer.h"
	  
class VOSubsystem : public Audio::Subsystem
{
public:
		enum Message
	{
		E_Msg_HiBits = FOURCC_HI20('VOSS'),

		eMoveActionIn,
		eToggleActionIn,
		eForwardsActionIn,
		eBackActionIn,
		eIn_horizontalActionIn,
		eIn_verticalActionIn,
		eOutActionIn,
		eLinkActionIn,
		eStartActionIn,
		eStopActionIn
	};

	VOSubsystem();
	~VOSubsystem() { }

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
	AudioEntityHandle m_hEntity;
};

#endif // VOSUBSYSTEM_H
