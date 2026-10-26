#ifndef DIRECTORACTOR_H
#define DIRECTORACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#pragma warning (push, 3)
#include <actor_system/Actor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtSharedActorMessages.h>
#include <Game/ActorDef/ActorTypes.h>
#pragma warning (pop)
#pragma warning (disable:4481) // nonstandard extension used: override specifier 'override'

#include "CameraSystem.h"

namespace Cam2
{
	class C_Camera;
}

namespace AmaxCam
{

class C_DirectorActorCreate : public ::Actors::C_GameActorCreate	// [Blur2PC] qualified: Game::Actors is visible too
{
public:
	const Actor::C_ActorTypeId GetType() const { return Game::E_ActorTypeId_DirectorActor; }
};

class C_DirectorActor : public Actor::C_Actor
{
public:
	C_DirectorActor();
	virtual ~C_DirectorActor();

	static const Actor::C_ActorTypeId		TypeId()		{ return Game::E_ActorTypeId_DirectorActor; }
	static const Btl::String32				TypeName()		{ return "DirectorActor"; }

	bool OnMessage(const Messaging::C_MessageBase *pMessage) OVERRIDE;

protected:
	bool InitialiseImpl() OVERRIDE;
	bool CreateImpl() OVERRIDE; // from Actor::C_Actor

private:
	const Cam2::C_Camera* HandleToCamera(const Actor::C_ActorHandle& handle);
};

enum E_DirectorMsg
{
	E_DirectorMsg_HiBits = FOURCC_HI20('DRCT'),
	
	E_DirectorMsg_ShowGameCamera,
	E_DirectorMsg_ShowLevelCamera,
	E_DirectorMsg_ShowEffect,

	E_DirectorMsg_End, 
	E_DirectorMsg_Begin = E_DirectorMsg_HiBits + 1, 
	E_DirectorMsg_Count = E_DirectorMsg_End - E_DirectorMsg_Begin
};

enum E_WhichFeed
{
	E_WhichFeed_Both = 0,
	E_WhichFeed_Current,
	E_WhichFeed_Future,
};

class C_ShowGameCameraMsg : public Messaging::C_MessageDerived<C_ShowGameCameraMsg, E_DirectorMsg_ShowGameCamera>
{
public:
	C_ShowGameCameraMsg()
	{
		m_CameraType = E_GameCam_None;
		m_ViewportId = 0;
		m_WhichFeed = E_WhichFeed_Both;
	}

	E_GameCam m_CameraType;
	S32 m_ViewportId;
	E_WhichFeed m_WhichFeed;
};

class C_ShowLevelCameraMsg : public Messaging::C_MessageDerived<C_ShowLevelCameraMsg, E_DirectorMsg_ShowLevelCamera>
{
public:
	C_ShowLevelCameraMsg()
	{
		m_ViewportId = 0;
		m_WhichFeed = E_WhichFeed_Both;
	}

	Actor::C_ActorHandle m_CameraActor;
	S32 m_ViewportId;
	E_WhichFeed m_WhichFeed;
};

// Showing effects like black and white:
enum E_Effect
{
	E_Effect_Normal,
	E_Effect_BlackAndWhite,
};

class C_ShowEffectMsg : public Messaging::C_MessageDerived<C_ShowEffectMsg, E_DirectorMsg_ShowEffect>
{
public:
	E_Effect m_EffectType;
};

class C_XtShowEffectMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext) OVERRIDE;

	C_ShowEffectMsg m_Msg;
	E_Effect m_EffectType;
};

} // End of namespace

#endif // DIRECTORACTOR_H
