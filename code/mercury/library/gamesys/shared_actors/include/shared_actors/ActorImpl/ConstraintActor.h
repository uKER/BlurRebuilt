#ifndef _CONSTRAINT_ACTOR_H_
#define _CONSTRAINT_ACTOR_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorBase.h>
#include <shared_actors/ActorCreation/ConstraintActorCreate.h>
#include <shared_actors/ActorPhysics.h>


namespace Gravity
{
	class C_Scene;
}

namespace SharedPhysics
{
	class C_Spring;
}


namespace Actors
{


class C_ConstraintActor : public C_SharedActorBase
{
public:
	struct TAxisLimits
	{
		TAxisLimits() : m_Limit(false) {}
		void Set(const Maths::HVec3& low, const Maths::HVec3& high) {m_Limit=true; m_Low=low; m_High=high;}
		bool m_Limit;
		Maths::HVec3 m_Low;
		Maths::HVec3 m_High;
	};

	// General purpose creation params class.
	class TCreationParams
	{
	public:
		TCreationParams() : m_UserData(NULL), m_CreateSpring(false) {}
	public:
		Actor::C_ActorHandle m_ActorHandleA;
		Actor::C_ActorHandle m_ActorHandleB;
		Maths::HVec3 m_WorldPos;
		Maths::HVec3 m_Axis;
		Actors::TBoolExtents m_Extents;
		TAxisLimits m_AxisLimits;
		void* m_UserData;
		bool m_CreateSpring;
		F32	m_BodyAMassScale;
	};

public:
	C_ConstraintActor() : m_ConstraintHandle(ActorPhysics::KInvalidConstraintHandle), m_Spring(NULL) {}
	~C_ConstraintActor();

	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_Constraint; }
	static const Actor::C_ActorTypeName		TypeName()	{ return "ConstraintActor"; }

	const C_ConstraintActorCreate*	GetCreationData() const {return static_cast<const C_ConstraintActorCreate*>(Actor::C_Actor::GetCreationData().GetData());}

	bool							OnMessage(const Messaging::C_MessageBase *pMessage );

public:
	// From C_Actor
	void SerialiseForSave(Sys::C_ContextualStream& stream) const {Actor::C_Actor::SerialiseForSave(stream); Serialise(stream);}
	void DeserialiseForSave(Sys::C_ContextualStream& stream) {Actor::C_Actor::DeserialiseForSave(stream); Deserialise(stream);}
	void SerialiseForSync(Sys::C_ContextualStream& stream) const {Actor::C_Actor::SerialiseForSync(stream); Serialise(stream);}
	void DeserialiseForSync(Sys::C_ContextualStream& stream) {Actor::C_Actor::DeserialiseForSync(stream); Deserialise(stream);}

protected:
	// From C_Actor
	bool InitialiseImpl();
	bool CreateImpl();
	//void ExternalDependenciesCreatedImpl();

	bool ResolveConstraintBodies(Actor::C_ActorHandle actorHandleA, Actor::C_ActorHandle actorHandleB, Gravity::C_Body*& bodyA, Gravity::C_Body*& bodyB) const;
	ActorPhysics::TConstraintHandle CreateFixedConstraint(const TCreationParams& params);
	ActorPhysics::TConstraintHandle CreateDistanceConstraint(const TCreationParams& params);
	ActorPhysics::TConstraintHandle CreateSphericalConstraint(const TCreationParams& params, SharedPhysics::C_Spring** spring=NULL);
	ActorPhysics::TConstraintHandle CreateSliderConstraint(const TCreationParams& params, SharedPhysics::C_Spring** spring=NULL);
	ActorPhysics::TConstraintHandle CreateHingeConstraint(const TCreationParams& params, SharedPhysics::C_Spring** spring=NULL);

	SharedPhysics::C_Spring* CreateSpring(Gravity::C_Body* bodyA, Gravity::C_Body* bodyB, const Maths::HVec3& worldPos, const Maths::HMat43& matA, const Maths::HMat43& matB);
	void ReleaseSpring(SharedPhysics::C_Spring* spring);
	void ReleaseConstraint(ActorPhysics::TConstraintHandle& constraintHandle);
	void OnBroken();
	void ChangeBody(const Gravity::C_Body* oldBody, const Gravity::C_Body* newBody);

	void OnBodyShapeChanged(const Gravity::C_Body* body, const Maths::HVec3& offset);

private:
	bool CreateConstraint();
	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

protected:
	Maths::HMat43 m_Matrix;
	Gravity::C_Scene* m_Scene;	// Cached from actor context.
	ActorPhysics::TConstraintHandle m_ConstraintHandle;
	SharedPhysics::C_Spring* m_Spring;
};


class C_DynamicFixedConstraintActor : public C_ConstraintActor
{
public:
	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_FixedConstraint; }
	static const Actor::C_ActorTypeName	TypeName()	{ return "FixedConstraintActor"; }
};


class C_DynamicDistanceConstraintActor : public C_ConstraintActor
{
public:
	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_DistanceConstraint; }
	static const Actor::C_ActorTypeName	TypeName()	{ return "DistanceConstraintActor"; }
};


class C_DynamicSphericalConstraintActor : public C_ConstraintActor
{
public:
	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_SphericalConstraint; }
	static const Actor::C_ActorTypeName	TypeName()	{ return "SphericalConstraintActor"; }
};


class C_DynamicSliderConstraintActor : public C_ConstraintActor
{
public:
	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_SliderConstraint; }
	static const Actor::C_ActorTypeName	TypeName()	{ return "SliderConstraintActor"; }
};


class C_DynamicHingeConstraintActor : public C_ConstraintActor
{
public:
	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_HingeConstraint; }
	static const Actor::C_ActorTypeName	TypeName()	{ return "HingeConstraintActor"; }
};


}//namespace Actors

#endif
