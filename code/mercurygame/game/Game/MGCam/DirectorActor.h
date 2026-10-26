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

namespace Cam2
{
	class C_Camera;
}

namespace MGCam
{

class C_DirectorActorCreate : public Actors::C_GameActorCreate
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
	E_DirectorMsg_ShowLevelCamera
};

typedef Actor::C_MessageActorValue<F32, E_DirectorMsg_ShowGameCamera>  C_ShowGameCameraMsg;
typedef Actor::C_MessageActorValue<F32, E_DirectorMsg_ShowLevelCamera> C_ShowLevelCameraMsg;

class C_CameraActorCreate;
class C_XtShowLevelCameraMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext) OVERRIDE;

	C_ShowLevelCameraMsg m_Msg;
	C_CameraActorCreate* m_CameraActor;
};

class C_XtShowGameCameraMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext) OVERRIDE;

	C_ShowGameCameraMsg m_Msg;
};

} // End of namespace

#endif // DIRECTORACTOR_H
