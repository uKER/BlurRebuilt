#ifndef I_PHYSICSBODYCOMPONENT_H
#define I_PHYSICSBODYCOMPONENT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Gravity/narrowphase/CollisionFilter.h>
#include <Gravity/narrowphase/Primitive.h>
#include <shared_actors/ActorPhysics.h>
#include <shared_actors/Components/SharedComponents.h>

namespace Components
{

class I_PhysicsBody : public Actor::I_ActorComponent
{
public:
	virtual ~I_PhysicsBody() {}

public:
	// From I_ActorComponent
	static Component::C_ComponentInterfaceId	InterfaceType() {return E_IF_PhysicsBody;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return E_IF_PhysicsBody;}
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return Actor::I_ActorComponent::OnMessage(pMessage);}	

	// Interface Methods
	virtual const Gravity::C_Body* CollideBody() const=0;
	virtual bool IsBodyEnabled() const=0;
	virtual ActorPhysics::TBodyType BodyType() const=0;
	virtual Maths::HMat43 Matrix() const = 0;
	virtual Maths::HMat43 RenderMatrix() const=0;
	virtual Maths::HF32 Mass() const=0;
	virtual Maths::HF32 ReciprocalMass() const=0;
	virtual Maths::HVec3 LinearMomentum() const=0;
	virtual Maths::HVec3 LinearVelocity() const=0;
	virtual Maths::HVec3 AngularVelocity() const=0;
	virtual const Collision::C_CollisionFilter& CollisionFilter() const=0;
	virtual U32 CollisionGroup() const=0;
	virtual Collision::C_Sphere WorldBoundingSphere() const=0;
	virtual F32 CollisionInfiniteMassRatio() const=0;
};

} // namespace Components

#endif
