#include "SharedActorsPch.h"
#include <shared_actors/ActorPhysics.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorList.h>
#include <Gravity/sdk/Body.h>
#include <shared_actors/ActorCreation/ConstraintActorCreate.h>
#include <shared_actors/Components/Interfaces/I_PhysicsBodyComponents.h>

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

namespace ActorPhysics
{

Debug::Bool gLogForces("ActorDebug.Physics.LogForces", false);


TBodyType BodyType(const Gravity::C_Body* body)
{
	if (body->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject))
		return ActorPhysics::E_BodyType_Animated;
	else if (body->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static))
		return ActorPhysics::E_BodyType_Static;
	return ActorPhysics::E_BodyType_Dynamic;
}


void TBuoyancyCoeffs::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_Coeff;
	stream << m_MaxCoeff;
	stream << m_LinearDragCoeff;
	stream << m_AngularDragCoeff;
}

void TBuoyancyCoeffs::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_Coeff;
	stream >> m_MaxCoeff;
	stream >> m_LinearDragCoeff;
	stream >> m_AngularDragCoeff;
}


const Components::I_PhysicsBody* GetPBC(const Actor::C_Actor* pActor)
{
	return (pActor == NULL) ? NULL : pActor->QueryInterface<Components::I_PhysicsBody>();
}

const Gravity::C_Body* GetGravityBody(const Actor::C_Actor* pActor)
{
	const Components::I_PhysicsBody* pPbc = GetPBC(pActor);
	return (pPbc == NULL) ? NULL : pPbc->CollideBody();
}


Actor::C_ActorHandle CreateConstraint(const Actors::C_ConstraintTypeBase* constraintBase, const Maths::HMat43& matrix, const Actor::C_Actor& actorA, const Actor::C_ActorHandle& actorHandleB)
{
	Actor::C_ActorList& actorList = *actorA.GetActorList();

	switch (constraintBase->ConstraintType())
	{
		case SharedPhysics::C_Constraint::E_Type_Fixed:
		{
			Actor::C_ActorCreationDataFactoryHelper<Actors::C_DynamicFixedConstraintActorCreate> pCreate;
			Maths::Store(matrix, &pCreate->m_DynWorldMatrix);
			pCreate->m_ActorHandleA = actorA.GetHandle();
			pCreate->m_ActorHandleB = actorHandleB;
			return actorList.Spawn(pCreate.Get());
		}

		case SharedPhysics::C_Constraint::E_Type_Spherical:
		{
			const Actors::C_SphericalConstraint* sphereConstraint = static_cast<const Actors::C_SphericalConstraint*>(constraintBase);
			Actor::C_ActorCreationDataFactoryHelper<Actors::C_DynamicSphericalConstraintActorCreate> pCreate;
			Maths::Store(matrix, &pCreate->m_DynWorldMatrix);
			pCreate->m_ActorHandleA = actorA.GetHandle();
			pCreate->m_ActorHandleB = actorHandleB;
			pCreate->m_Constraint = *sphereConstraint;
			return actorList.Spawn(pCreate.Get());
		}

		case SharedPhysics::C_Constraint::E_Type_Distance:
		{
			const Actors::C_DistanceConstraint* distConstraint = static_cast<const Actors::C_DistanceConstraint*>(constraintBase);
			Actor::C_ActorCreationDataFactoryHelper<Actors::C_DynamicDistanceConstraintActorCreate> pCreate;
			Maths::Store(matrix, &pCreate->m_DynWorldMatrix);
			pCreate->m_ActorHandleA = actorA.GetHandle();
			pCreate->m_ActorHandleB = actorHandleB;
			pCreate->m_Constraint = *distConstraint;
			return actorList.Spawn(pCreate.Get());
		}

		case SharedPhysics::C_Constraint::E_Type_Prismatic: // Slider
		{
			const Actors::C_SliderConstraint* sliderConstraint = static_cast<const Actors::C_SliderConstraint*>(constraintBase);
			Actor::C_ActorCreationDataFactoryHelper<Actors::C_DynamicSliderConstraintActorCreate> pCreate;
			Maths::Store(matrix, &pCreate->m_DynWorldMatrix);
			pCreate->m_ActorHandleA = actorA.GetHandle();
			pCreate->m_ActorHandleB = actorHandleB;
			pCreate->m_Constraint = *sliderConstraint;
			return actorList.Spawn(pCreate.Get());
		}

		case SharedPhysics::C_Constraint::E_Type_Revolute: // Hinge
		{
			const Actors::C_HingeConstraint* hingeConstraint = static_cast<const Actors::C_HingeConstraint*>(constraintBase);
			Actor::C_ActorCreationDataFactoryHelper<Actors::C_DynamicHingeConstraintActorCreate> pCreate;
			Maths::Store(matrix, &pCreate->m_DynWorldMatrix);
			pCreate->m_ActorHandleA = actorA.GetHandle();
			pCreate->m_ActorHandleB = actorHandleB;
			pCreate->m_Constraint = *hingeConstraint;
			return actorList.Spawn(pCreate.Get());
		}

		default:
			LOGERROR(Actors::g_Log, DBSTR"unhandled constraint type %d", constraintBase->ConstraintType());
			break;
	}

	return Actor::C_ActorHandle();
}


Btl::String Stringify(const Maths::HVec3& vec)
{
	return Btl::String::Format("[X:%.02f, Y:%.02f, Z:%.02f Len:%.02f]", (F32)vec.X(), (F32)vec.Y(), (F32)vec.Z(), (F32)vec.Length());
}

} // namespace ActorPhysics

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif
