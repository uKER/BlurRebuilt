#ifndef GAME_CHARACTER_TEST_ACTOR_H
#define GAME_CHARACTER_TEST_ACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/sdk/CharacterController.h>

namespace Cam2 { class C_CharacterCamera; }
namespace MGCam { class C_PhysicsChecks; }
namespace GameResource { class C_ModelResource; class C_ModelElement; }
namespace WorldRender { class C_ModelInstance; }

namespace Game
{

class C_GameWorld;

namespace Actors
{

	
class C_CharacterTestActorCreate : public ::Actors::C_PosActorCreate
{
public:
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_CharacterTest;}
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_CharacterTest;}

	Util::C_String m_ModelFilename;
};

// Actor Info
class C_CharacterTestActor : public C_GameActor
{
public:
	C_CharacterTestActor();
	virtual ~C_CharacterTestActor();
	const Maths::HMat43 &			GetMatrix() const {return m_WorldMatrix;}
	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_CharacterTest; }
	static const Btl::String32			TypeName()	{ return "CharacterTest"; }
	virtual bool OnMessage(const Messaging::C_MessageBase *pMessage);
	void CreateModel();
	void DestroyModel();
	void InitCamera();
	void ExitCamera();

protected:
	virtual bool InitialiseImpl();
	virtual bool CreateImpl();

private:
	Maths::HMat43	m_WorldMatrix;

	GameResource::C_ModelResource*		m_pModelResource;
	WorldRender::C_ModelInstance*		m_pModelInstance;
	const GameResource::C_ModelElement*	m_pModelElement;

	Gravity::C_Body* m_pBody;
	Gravity::C_CharacterController		m_CharacterController;

	Cam2::C_CharacterCamera* m_pCamera;
	MGCam::C_PhysicsChecks* m_pPhysicsChecks;
};

} // namespace Actors


} // namespace Game

#endif //  GAME_CHARACTER_TEST_ACTOR_H
