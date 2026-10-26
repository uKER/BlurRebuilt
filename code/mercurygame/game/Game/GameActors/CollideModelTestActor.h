// Collide Model Test Actor
// ------------------------

// This is copied from Paul's original test model actor but is designed to test new components
// which are to be used for Bond09 & shared in Mercury

#ifndef COLLIDEMODEL_TESTACTOR_H
#define COLLIDEMODEL_TESTACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>

#include <resource_utils/ResourceUtilsDeclare.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

namespace Game
{
	class C_GameWorld;
}

namespace GameResource
{
	class C_ModelElement;
	class C_ModelResource;
}

namespace Gravity
{
	class C_Body;
}

namespace WorldRender
{
	class C_ModelInstance;
}

namespace Game
{

namespace Actors
{
#if 0
/// Base class for ModelTestActor creation data.
class C_CollideModelTestActorCreate : public ::Actors::C_PosActorCreate
{
public:
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_CollideModelTest;}
	virtual const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_CollideModelTest;}

	virtual Util::C_String GetModelFilename() const = 0;

};

/// Runtime creation data for ModelTestActor.
class C_CollideModelTestActorCreateDynamic : public C_ModelTestActorCreate
{
public:
	Util::C_String m_ModelFilename;

	virtual Util::C_String GetModelFilename() const; // from C_ModelTestActorCreate

	// serialisation is needed for runtime-created data
	virtual void Serialise(Sys::C_ContextualStream& stream) const;
	virtual void Deserialise(Sys::C_ContextualStream& stream);

};
#endif
/// Xt annotated creation data for ModelTestActor.
class C_CollideModelTestActorCreate : public ::Actors::C_PosActorCreate
{
public:
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_CollideModelTest;}
	virtual const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_CollideModelTest;}

	// data members are filled by ActorStudio
	XtHandle<ResourceUtils::C_GameModel> m_GameModel;

	//virtual Util::C_String GetModelFilename() const; // from C_ModelTestActorCreate

	// serialisation is not needed for ActorStudio data, we use a UID in the Sorcery database

};

// Actor Info
class C_CollideModelTestActor : public C_GameActor
{
public:
	C_CollideModelTestActor();
	virtual ~C_CollideModelTestActor();
	const Maths::HMat43 &			GetMatrix() const {return m_WorldMatrix;}
	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_CollideModelTest; }
	static const Btl::String32			TypeName()	{ return "CollideModelTest"; }
	virtual bool OnMessage(const Messaging::C_MessageBase *pMessage);

	const C_CollideModelTestActorCreate* GetCreationData() const {return static_cast<const C_CollideModelTestActorCreate*>(C_GameActor::GetCreationData().GetData());}

protected:
	virtual bool InitialiseImpl();
	virtual bool CreateImpl();

private:
	Maths::HMat43	m_WorldMatrix;
};

/// Xt annotated creation data for Platform Actor.
class C_PlatformActorCreate : public ::Actors::C_PosActorCreate
{
public:
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_CollideModelTest;}
	virtual const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_CollideModelTest;}

	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);

	Component::I_ComponentCreator::C_DataRef	m_pMovementCreator;
// 	const Components::C_MovementCreateInfo*		GetMovementCreator() const {return static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData());}
// 	Components::C_MovementCreateInfo*			GetMovementCreator() {return const_cast<Components::C_MovementCreateInfo*>(static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData()));}

	// data members are filled by ActorStudio
	XtSz8	m_PlatformName;
	Components::C_MovementCreateInfo*	m_pSorceryMovementComponentCreate;
};

// Actor Info
class C_PlatformActor : public C_GameActor
{
public:
	C_PlatformActor();
	virtual ~C_PlatformActor();
	const Maths::HMat43 &			GetMatrix() const {return m_WorldMatrix;}
	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_Platform; }
	static const Btl::String32			TypeName()	{ return "Platform"; }
	virtual bool OnMessage(const Messaging::C_MessageBase *pMessage);

	const C_PlatformActorCreate* GetCreationData() const {return static_cast<const C_PlatformActorCreate*>(C_GameActor::GetCreationData().GetData());}

protected:
	virtual bool InitialiseImpl();
	virtual bool CreateImpl();

private:
	Maths::HMat43	m_WorldMatrix;
};

} // namespace Actors


} // namespace Game

#endif //  GAME_ACTOR_TYPES_H
