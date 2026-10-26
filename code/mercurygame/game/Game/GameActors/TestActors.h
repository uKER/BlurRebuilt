#ifndef GAME_TEST_ACTORS_H
#define GAME_TEST_ACTORS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>

#include <resource_utils/ResourceUtilsDeclare.h>
#include <shared_actors/XtActorBase.h>

namespace Game
{
	class C_GameWorld;
}

namespace Actors
{
	class C_SimplePhysicsBodyUserData;
	class C_VisibleDetect;
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
	class C_EnvironmentSampler;
}

namespace Game
{

namespace Actors
{

/// Base class for ModelTestActor creation data.
class C_ModelTestActorCreate : public ::Actors::C_PosActorCreate
{
public:
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_ModelTest;}
	virtual const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_ModelTest;}

	virtual Util::C_String GetModelFilename() const = 0;

};

/// Runtime creation data for ModelTestActor.
class C_ModelTestActorCreateProcedural : public C_ModelTestActorCreate
{
public:
	Util::C_String m_ModelFilename;

	virtual Util::C_String GetModelFilename() const; // from C_ModelTestActorCreate

	// serialisation is needed for runtime-created data
	virtual void Serialise(Sys::C_ContextualStream& stream) const;
	virtual void Deserialise(Sys::C_ContextualStream& stream);

};

/// Xt annotated creation data for ModelTestActor.
class C_ModelTestActorCreateXt : public C_ModelTestActorCreate
{
public:
	// data members are filled by ActorStudio
	XtHandle<ResourceUtils::C_GameModel> m_GameModel;

	virtual Util::C_String GetModelFilename() const; // from C_ModelTestActorCreate

	// serialisation is not needed for ActorStudio data, we use a UID in the Sorcery database

};

// Actor Info
class C_ModelTestActor : public C_GameActor
{
public:
	C_ModelTestActor();
	virtual ~C_ModelTestActor();
	const Maths::HMat43 &			GetMatrix() const {return m_WorldMatrix;}
	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_ModelTest; }
	static const Btl::String32			TypeName()	{ return "ModelTest"; }
	virtual bool OnMessage(const Messaging::C_MessageBase *pMessage);
	void CreateModel();
	void DestroyModel();

	const C_ModelTestActorCreate* GetCreationData() const {return static_cast<const C_ModelTestActorCreate*>(C_GameActor::GetCreationData().GetData());}

#ifndef CFG_GOLD
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const; // from Actor::C_Actor
#endif

protected:
	virtual bool InitialiseImpl();
	virtual bool CreateImpl();

private:
	Maths::HMat43	m_WorldMatrix;
	Maths::C_Sphere m_LocalBoundingSphere;

	GameResource::C_ModelResource*		m_pModelResource;
	WorldRender::C_ModelInstance*		m_pModelInstance;
	const GameResource::C_ModelElement*	m_pModelElement;

	Gravity::C_Body*						m_pBody;
	::Actors::C_SimplePhysicsBodyUserData*	m_pPhysicsBodyUserData;
	WorldRender::C_EnvironmentSampler*		m_pEnvSampler;
	::Actors::C_VisibleDetect				*m_pVisDetect;
};

} // namespace Actors


} // namespace Game

#endif //  GAME_ACTOR_TYPES_H
