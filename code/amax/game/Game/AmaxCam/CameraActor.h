#ifndef CAMERAACTOR_H
#define CAMERAACTOR_H

#pragma warning (push, 3)
#include <shared_actors/XtActorBase.h>
#include <actor_system/Actor.h>
#include <Game/ActorDef/ActorTypes.h>
#include <shared_actors/Components/Creation/MovementComponentsCreate.h>
#pragma warning (pop)
#pragma warning (disable:4481) // nonstandard extension used: override specifier 'override'

namespace Cam2
{
	class C_Camera;
}

namespace AmaxCam
{

class C_CameraActorCreate : public Actors::C_PosActorCreate
{
public:
	const Actor::C_ActorTypeId		GetType() const		{ return Game::E_ActorTypeId_CameraActor; }

	Components::C_MovementCreateInfo*	m_pSorceryMovementComponentCreate;

	Component::I_ComponentCreator::C_DataRef	m_pMovementCreator;
	const Components::C_MovementCreateInfo*		GetMovementCreator() const {return static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData());}

	// from Actor::C_ActorCreationData
	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};

class C_CameraActor : public Actor::C_Actor
{
public:
	C_CameraActor();
	virtual ~C_CameraActor();
	
	static const Actor::C_ActorTypeId		TypeId()		{ return Game::E_ActorTypeId_CameraActor; }
	static const Btl::String32				TypeName()		{ return "CameraActor"; }
	
	bool OnMessage(const Messaging::C_MessageBase* message) OVERRIDE;
	
#ifndef CFG_GOLD
	void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const OVERRIDE;
#endif

	const C_CameraActorCreate*	GetCreationData() const {return static_cast<const C_CameraActorCreate*>(Actor::C_Actor::GetCreationData().GetData());}

	const Maths::HMat43& GetMatrix(void) const OVERRIDE;
	const Cam2::C_Camera* GetCamera() const { return m_Camera; }

protected:
	bool InitialiseImpl() OVERRIDE;
	bool CreateImpl() OVERRIDE;
	void KillImpl() OVERRIDE;

	Cam2::C_Camera* m_Camera;
	Maths::HMat43 m_Matrix;
};

}

#endif
