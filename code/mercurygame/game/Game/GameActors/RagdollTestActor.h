#ifndef GAME_RAGDOLL_TEST_ACTOR_H
#define GAME_RAGDOLL_TEST_ACTOR_H

//#ifdef _MSC_VER
//#pragma once
//#endif
//
//#include <actor_system/ActorSys.h>
//#include <Game/ActorDef/ActorTypes.h>
//#include <Game/ActorDef/GameActor.h>
//#include <shared_actors/XtActorBase.h>
//
//#include <Resource/Resource.h>
//#include <game_resource/ModelResource.h>
//#include <world_render/ModelInstance.h>
//#include <SceneGraph/SceneGraph.h>
//#include <game_resource/ModelData.h>
//
//#include <gravity/sdk/BodyHelper.h>
//#include <gravity/sdk/RagdollController.h>
//
//namespace Cam2 { class C_CharacterCamera; }
//namespace MGCam { class C_PhysicsChecks; }
//
//namespace Game
//{
//
//class C_GameWorld;
//
//namespace Actors
//{
//
//	
//class C_CharacterTestActorCreate : public ::Actors::C_PosActorCreate
//{
//public:
//	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_RagdollTest;}
//	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_RagdollTest;}
//
//	Util::C_String m_ModelFilename;
//};
//
//// Actor Info
//class C_RagdollTestActor : public C_GameActor
//{
//public:
//	C_RagdollTestActor();
//	virtual ~C_RagdollTestActor();
//	const Maths::HMat43 &			GetMatrix() const {return m_WorldMatrix;}
//	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_RagdollTest; }
//	static const Btl::String32			TypeName()	{ return "RagdollTest"; }
//	virtual bool MessageHandler(const Messaging::C_MessageBase *pMessage);
//	void CreateModel();
//	void DestroyModel();
//	void InitCamera();
//	void ExitCamera();
//
//	const U32 GetNumberBodies();
//	const Maths::HMat43 & GetBodyMatrix(const U32 bodyIndex);
//
//protected:
//	virtual bool InitialiseImpl();
//	virtual bool CreateImpl();
//
//private:
//	Maths::HMat43	m_WorldMatrix;
//
//	GameResource::C_ModelResource*		m_pModelResource;
//	WorldRender::C_ModelInstance*		m_pModelInstance;
//	const GameResource::C_ModelElement*	m_pModelElement;
//
//	Gravity::C_RagdollController		m_RagdollController;
//
//	Cam2::C_CharacterCamera* m_pCamera;
//	MGCam::C_PhysicsChecks* m_pPhysicsChecks;
//};
//
//} // namespace Actors
//
//
//} // namespace Game

#endif //  GAME_CHARACTER_TEST_ACTOR_H
