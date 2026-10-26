
#ifndef ListenerSubsystem_H
#define ListenerSubsystem_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AppInterfaceLayer.h"
#include <Game/GameActors/AudioActors.h>
	  
class ListenerSubsystem : public Audio::Subsystem
{
public:

	ListenerSubsystem();
	~ListenerSubsystem() { }

	bool		ProcessSystemMessage(const U32 u32MessageId);
	bool		ProcessCustomMessage(const U32 u32MessageId, U32 u32Data = 0, void* pData = 0);

	bool		SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)
	{
		return true;
	}
	void		Update();

private:
	AudioEntityHandle hEntity;
	//Game::Actors::C_ListenerAudioActor m_CameraActor;
	Actor::C_ActorHandle m_listenerHandle;
	Audio::XtEntityProxy* pXtEntity;
};

#endif // ListenerSubsystem_H
