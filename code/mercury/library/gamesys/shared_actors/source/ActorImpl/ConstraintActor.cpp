
#include "SharedActorsPch.h"
#include "shared_actors/ActorImpl/ConstraintActor.h"
#include <misc_shared/Messaging/GameMessaging.h>
#include <shared_actors/SharedActorUtils.h>
#include <game_context/PhysicsContext.h>
#include <actor_system/ActorList.h>
#include <gravity/sdk/Scene.h>
#include <Gravity/sdk/ConstraintDescriptors.h>
#include <actor_system/ActorDebug.h>
#include <shared_actors/ActorMessages/ConstraintActorMessages.h>
#include <Gravity/solver2/Constraint.h>
#include <shared_actors/SharedActorMessages.h>

using namespace ActorPhysics;

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif


namespace Actors
{

C_ConstraintActor::~C_ConstraintActor()
{
	if (m_ConstraintHandle != KInvalidConstraintHandle)
	{
		ReleaseConstraint(m_ConstraintHandle);
		//C_GameWorld::GetInstance()->GetPhysicsWorld()->ReleaseConstraint(m_ConstraintHandle);
	}

	if (m_Spring)
		ReleaseSpring(m_Spring);
	m_Spring = NULL;
}


bool C_ConstraintActor::InitialiseImpl()
{
	if (!C_Actor::InitialiseImpl())
		return false;

	m_Matrix = GetCreationData()->GetCreationMatrix();

	return true;
}


bool C_ConstraintActor::CreateImpl()
{
	if (!C_Actor::CreateImpl())
		return false;

	m_Scene = GetContext<GameContext::I_PhysicsContext>()->GetPhysicsScene();

	const C_ConstraintActorCreate* create = GetCreationData();

	if (!create->ActorHandleA())
	{
		LOGERROR(Actors::g_Log, DBSTR"Constraint '%s' has NIL Actor A - requires valid actor", create->m_Name.string);
		return false;
	}

	// Create in update, as joined bodies are sure to exist then.
	AddSubscriber(GameMessaging::E_GameMsg_Update);

	return true;
}


//void C_ConstraintActor::ExternalDependenciesCreatedImpl()
//{
// 	if (!CreateConstraint())
// 	{
// 		LOGERROR(Actors::g_Log, DBSTR"Unable to create constraint actor '%s' - check connected actors!", GetActorName());
// 		Kill();
// 	}
//}


void C_ConstraintActor::Serialise(Sys::C_ContextualStream& stream) const
{
	if (!m_ConstraintHandle)
	{
		LOGERROR(Actors::g_Log, DBSTR"'%s': NULL constraint!", GetActorName());
		stream << m_Matrix;
		return;
	}

	// todo: is there a way of getting constraint's current world matrix from Gravity?

#if 0
	//Not used in Blur and this code has completely changed in the trunk because constraint
	//serialisation/deserialisation didn't work!
	switch (m_ConstraintHandle->ConstraintType())
	{
		case SharedPhysics::C_Constraint::E_Type_Fixed:
		{
			Solver2::C_FixedConstraint* constraint = static_cast<Solver2::C_FixedConstraint*>(m_ConstraintHandle);
			Maths::HMat43 mat(constraint->pNodeA->m_pBody->Orientation(), constraint->pNodeA->m_pBody->Position());
			const Maths::HVec3 pos = constraint->m_pSphericalConstraint->m_AnchorPoint1 * mat;
			mat.AW(pos);
			stream << mat;
		}	break;

		case SharedPhysics::C_Constraint::E_Type_Distance:
		{
			Solver2::C_DistanceConstraint* constraint = static_cast<Solver2::C_DistanceConstraint*>(m_ConstraintHandle);
			Maths::HMat43 mat(constraint->pNodeA->m_pBody->Orientation(), constraint->pNodeA->m_pBody->Position());
			const Maths::HVec3 pos = constraint->m_AnchorPoint1 * mat;
			mat.AW(pos);
			stream << mat;
		}	break;

		case SharedPhysics::C_Constraint::E_Type_Spherical:
		{
			Solver2::C_SphericalConstraint* constraint = static_cast<Solver2::C_SphericalConstraint*>(m_ConstraintHandle);
			Maths::HMat43 mat(constraint->pNodeA->m_pBody->Orientation(), constraint->pNodeA->m_pBody->Position());
			const Maths::HVec3 pos = constraint->m_AnchorPoint1 * mat;
			mat.AW(pos);
			stream << mat;
		}	break;

		default:
			stream << m_Matrix;
			break;
	}
#endif
}


void C_ConstraintActor::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_Matrix;
}


bool C_ConstraintActor::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch (pMessage->Type())
	{
		case GameMessaging::E_GameMsg_Update:
			if (CreateConstraint())
			{
				RemoveSubscriber(GameMessaging::E_GameMsg_Update);
			}
			else
			{
				LOGERROR(Actors::g_Log, DBSTR"Unable to create constraint actor '%s' - check connected actors!", GetActorName());
				Kill();
			}
			break;

		case ActorMsg::E_GameActorMsg_PhysicsConstraintDestroyed:
			m_ConstraintHandle = KInvalidConstraintHandle;
			OnBroken();
			break;

		case ActorMsg::E_ConstraintMsg_Destroy:
			ReleaseConstraint(m_ConstraintHandle);
			OnBroken();
			break;

		case ActorMsg::E_ConstraintMsg_ChangeBody:
		{
			const ActorMsg::C_ConstraintChangeBodyMsg* msg = Messaging::MessageCast<ActorMsg::C_ConstraintChangeBodyMsg>(pMessage);
			ChangeBody(msg->m_OldBody, msg->m_NewBody);
		}	break;

		default:
			return C_SharedActorBase::OnMessage(pMessage);
	}

	return true;
}


void C_ConstraintActor::OnBroken()
{
	BIZ_ASSERT(m_ConstraintHandle==KInvalidConstraintHandle);

	if (m_Spring)
	{
		ReleaseSpring(m_Spring);
		m_Spring = NULL;
	}

	GetCreationData()->m_OnBroken.Send(this);
	Kill();
}


// Changes the body of one side of the constraint.
void C_ConstraintActor::ChangeBody(const Gravity::C_Body* oldBody, const Gravity::C_Body* newBody)
{
	BIZ_ASSERT_MSG(oldBody && newBody && oldBody!=newBody && oldBody->GetScene()==newBody->GetScene(), "ConstraintActor::ChangeBody - body problem");
	BIZ_ASSERT_MSG(oldBody->GetShape() && newBody->GetShape(), "shape problem");

	if (m_ConstraintHandle->pNodeA->m_pBody == oldBody)
	{
		m_ConstraintHandle->pNodeA->m_pBody = const_cast<Gravity::C_Body*>(newBody);
	}
	else if (m_ConstraintHandle->pNodeB->m_pBody == oldBody)
	{
		m_ConstraintHandle->pNodeB->m_pBody = const_cast<Gravity::C_Body*>(newBody);
	}
	else
	{
		LOGERROR(Actors::g_Log, DBSTR"body problem");
		return;
	}

	const Maths::HVec3 diff(oldBody->ElementOffset() - newBody->ElementOffset());
	m_ConstraintHandle->ApplyBodyOffset(newBody, diff);
}


#ifndef CFG_GOLD
// Helper fn.
static void RangeCheck(F32 lo, F32 hi, S32& pm90, S32& pm180)
{
	if (lo>=-180.0f && hi<=180.0f)
		pm90++;
	if (lo>=-90.0f && hi<=90.0f)
		pm180++;
}
#endif


bool C_ConstraintActor::CreateConstraint()
{
	const Actors::C_ConstraintActorCreate* createPtr = GetCreationData();

	TCreationParams params;
	params.m_UserData = SharedActorUtils::HandleToUserData(GetHandle());
	params.m_ActorHandleA = createPtr->ActorHandleA();
	params.m_ActorHandleB = createPtr->ActorBIsNull() ? Actor::C_ActorHandle() : createPtr->ActorHandleB();
	params.m_WorldPos = m_Matrix.AW();

	switch (createPtr->ConstraintType())
	{
		case SharedPhysics::C_Constraint::E_Type_Fixed:
			m_ConstraintHandle = CreateFixedConstraint(params);
			break;

		case SharedPhysics::C_Constraint::E_Type_Distance:
		{
			const C_DistanceConstraint* distanceConstraint = static_cast<const C_DistanceConstraint*>(createPtr->ConstraintInfo());
			params.m_Extents.Set(0.0f, distanceConstraint->m_Distance);
			m_ConstraintHandle = CreateDistanceConstraint(params);
		}	break;

		case SharedPhysics::C_Constraint::E_Type_Spherical: // Ball & socket
		{
			const C_SphericalConstraint* sphericalConstraint = static_cast<const C_SphericalConstraint*>(createPtr->ConstraintInfo());
			params.m_CreateSpring = sphericalConstraint->m_CreateSpring;
			if (sphericalConstraint->m_Limit)
			{
#ifndef CFG_GOLD
				// Check limit validity.
				S32 plusMinus90Count = 0;
				S32 plusMinus180Count = 0;
				RangeCheck(sphericalConstraint->m_AxisLowLimitDegs.vx, sphericalConstraint->m_AxisHighLimitDegs.vx, plusMinus90Count, plusMinus180Count);
				RangeCheck(sphericalConstraint->m_AxisLowLimitDegs.vy, sphericalConstraint->m_AxisHighLimitDegs.vy, plusMinus90Count, plusMinus180Count);
				RangeCheck(sphericalConstraint->m_AxisLowLimitDegs.vz, sphericalConstraint->m_AxisHighLimitDegs.vz, plusMinus90Count, plusMinus180Count);
				if (plusMinus90Count < 1  ||  plusMinus180Count < 2)
				{
					LOGERROR(Actors::g_Log, DBSTR"Spherical constraint limits problem: Must have one val within +/-90 deg, and another within +/-180 degs.");
					return false;
				}
#endif
				params.m_AxisLimits.Set(Load(sphericalConstraint->m_AxisLowLimitDegs), Load(sphericalConstraint->m_AxisHighLimitDegs));
			}

			sphericalConstraint->m_BodyAMassScale > 0.0001f ? params.m_BodyAMassScale = sphericalConstraint->m_BodyAMassScale : params.m_BodyAMassScale = 1.0f;
			
			m_ConstraintHandle = CreateSphericalConstraint(params, &m_Spring);
			if (m_Spring)
			{
				m_Spring->m_RestLength = 1.0f;
				m_Spring->SetAngularCoefficient(sphericalConstraint->m_SpringCoeff, sphericalConstraint->m_SpringDamping);
				m_Spring->SetLinearCoefficient(0.0f, 0.0f);
			}
		}	break;

		case SharedPhysics::C_Constraint::E_Type_Prismatic: // Slider
		{
			const C_SliderConstraint* sliderConstraint = static_cast<const C_SliderConstraint*>(createPtr->ConstraintInfo());
			params.m_Axis = (sliderConstraint->m_Axis==Actors::E_AxisX ? m_Matrix.AX() : (sliderConstraint->m_Axis==Actors::E_AxisY ? m_Matrix.AY() : m_Matrix.AZ()));
			if (sliderConstraint->m_Extents.m_Limit && (sliderConstraint->m_Extents.m_Min >= sliderConstraint->m_Extents.m_Max))
			{
				LOGERROR(Actors::g_Log, DBSTR"Slider constraint extents problem: max (%.02f) must be larger than min (%.02f)", sliderConstraint->m_Extents.m_Max, sliderConstraint->m_Extents.m_Min);
				return false;
			}
			params.m_Extents = sliderConstraint->m_Extents;
			params.m_CreateSpring = sliderConstraint->m_CreateSpring;
			m_ConstraintHandle = CreateSliderConstraint(params, &m_Spring);
			if (m_Spring)
			{
				m_Spring->m_RestLength = sliderConstraint->m_RestLength;
				m_Spring->SetLinearCoefficient(sliderConstraint->m_SpringCoeff, sliderConstraint->m_SpringDamping);
				m_Spring->SetAngularCoefficient(0.0f, 0.0f);
			}
		}	break;

		case SharedPhysics::C_Constraint::E_Type_Revolute: // Hinge
		{
			const C_HingeConstraint* hingeConstraint = static_cast<const C_HingeConstraint*>(createPtr->ConstraintInfo());
			params.m_Axis = hingeConstraint->m_Axis==Actors::E_AxisX ? m_Matrix.AX() : (hingeConstraint->m_Axis==Actors::E_AxisZ ? m_Matrix.AZ() : m_Matrix.AY());
			if (hingeConstraint->m_Extents.m_Limit && (hingeConstraint->m_Extents.m_Min >= hingeConstraint->m_Extents.m_Max))
			{
				LOGERROR(Actors::g_Log, DBSTR"Hinge constraint extents problem: max (%.02f) must be larger than min (%.02f)", hingeConstraint->m_Extents.m_Max, hingeConstraint->m_Extents.m_Min);
				return false;
			}
			params.m_Extents = hingeConstraint->m_Extents;
			params.m_CreateSpring = hingeConstraint->m_CreateSpring;
			m_ConstraintHandle = CreateHingeConstraint(params, &m_Spring);
			if (m_Spring)
			{
				m_Spring->m_RestLength = 1.0f;
				m_Spring->SetAngularCoefficient(hingeConstraint->m_SpringCoeff, hingeConstraint->m_SpringDamping);
				m_Spring->SetLinearCoefficient(0.0f, 0.0f);
				//tbi in grav: m_Spring->SetAngleAndLimit(0.0f, Maths::DegToRad(hingeConstraint->m_AngularLimit));	// todo: zero ok for serialisation?
			}
		}	break;

		default:
			static S32 errCount = 0;
			if (errCount++ < 5)
				LOGWARN(Actors::g_Log, DBSTR"unhandled constraint type (%d)", createPtr->ConstraintType());
			break;
	}

	return (m_ConstraintHandle != KInvalidConstraintHandle);
}


bool C_ConstraintActor::ResolveConstraintBodies(Actor::C_ActorHandle actorHandleA, Actor::C_ActorHandle actorHandleB, Gravity::C_Body*& bodyA, Gravity::C_Body*& bodyB) const
{
	Gravity::C_Body* pBodyA = NULL;
	Gravity::C_Body* pBodyB = NULL;

	{
		pBodyA = const_cast<Gravity::C_Body*>(GetGravityBody(GetActorList()->GetActorPtr(actorHandleA)));

		if (actorHandleB == Actor::C_ActorHandle())
			pBodyB = m_Scene->GetStaticWorldBody(); //m_Scene->NumMeshBodies()>0 ? m_Scene->GetMeshBody(0) : NULL;//doesn't work: m_Scene->GetStaticWorldBody();
		else
			pBodyB = const_cast<Gravity::C_Body*>(GetGravityBody(GetActorList()->GetActorPtr(actorHandleB)));
	}

	if ((pBodyA != NULL) && (pBodyB != NULL))
	{
		if (pBodyA != pBodyB)
		{
			if (pBodyA->GetScene() && pBodyA->GetScene()==pBodyB->GetScene())
			{
				bodyA = pBodyA;
				bodyB = pBodyB;
				return true;
			}
			else
			{
				LOGERROR(Actors::g_Log, DBSTR"body scene problem");
			}
		}
		else
		{
			LOGERROR(Actors::g_Log, DBSTR"Constraint joined to same body");
		}
	}

	bodyA = NULL;
	bodyB = NULL;
	return false;
}


SharedPhysics::C_Spring* C_ConstraintActor::CreateSpring(Gravity::C_Body* bodyA, Gravity::C_Body* bodyB, const Maths::HVec3& worldPos, const Maths::HMat43& matA, const Maths::HMat43& matB)
{
	Gravity::SpringDescriptor desc;
	desc.DefineAnchorPoints(worldPos * matA.InverseRotTrans(), worldPos * matB.InverseRotTrans());
	//desc.DefineRestLength(restLength); //Should be here
	//desc.DefineSpringCoefficients(linearSpringCoefficient,linearDampingCoefficient,angularSpringCoefficient, angularDampingCoefficient);
	SharedPhysics::C_Spring* newSpring = desc.CreateSpring(m_Scene, bodyA, bodyB);
	m_Scene->AddSpring(newSpring);
	return newSpring;
}


void C_ConstraintActor::ReleaseSpring(SharedPhysics::C_Spring* spring)
{
	if (spring)
		m_Scene->RemoveSpring(spring);
}


TConstraintHandle C_ConstraintActor::CreateFixedConstraint(const TCreationParams& params)
{
	Gravity::C_Body* bodyA = NULL;
	Gravity::C_Body* bodyB = NULL;
	if (ResolveConstraintBodies(params.m_ActorHandleA, params.m_ActorHandleB, bodyA, bodyB))
	{
		const Maths::HMat43 matA(bodyA->Orientation(), bodyA->Position());
		const Maths::HMat43 matB(bodyB->Orientation(), bodyB->Position());
		Gravity::FixedJointDescriptor desc;
		desc.DefineAnchorPoints(params.m_WorldPos * matA.InverseRotTrans(), params.m_WorldPos * matB.InverseRotTrans());
		TConstraintHandle constraint = desc.CreateConstraint(*m_Scene->m_pGraphManager, bodyA, bodyB);
		constraint->SetBaumgarte(0.2f);
		constraint->SetUserData(params.m_UserData);
		m_Scene->AddDebugConstraint(constraint);
		return constraint;
	}

	//LOGWARN(Actors::g_Log, DBSTR"unable to create fixed constraint");
	return KInvalidConstraintHandle;
}


TConstraintHandle C_ConstraintActor::CreateDistanceConstraint(const TCreationParams& params)
{
	Gravity::C_Body* bodyA = NULL;
	Gravity::C_Body* bodyB = NULL;
	if (ResolveConstraintBodies(params.m_ActorHandleA, params.m_ActorHandleB, bodyA, bodyB))
	{
		const Maths::HMat43 matA(bodyA->Orientation(), bodyA->Position());
		const Maths::HMat43 matB(bodyB->Orientation(), bodyB->Position());
		Gravity::DistanceConstraintDescriptor desc;
		desc.DefineDistanceLimit(Maths::HF32(params.m_Extents.m_Max));
		desc.DefineAnchorPoints(params.m_WorldPos * matA.InverseRotTrans(), params.m_WorldPos * matB.InverseRotTrans());
		TConstraintHandle constraint = desc.CreateConstraint(*m_Scene->m_pGraphManager, bodyA, bodyB);
		constraint->SetUserData(params.m_UserData);
		m_Scene->AddDebugConstraint(constraint);
		return constraint;
	}

	//LOGWARN(Actors::g_Log, DBSTR"unable to create distance constraint");
	return KInvalidConstraintHandle;
}


TConstraintHandle C_ConstraintActor::CreateSphericalConstraint(const TCreationParams& params, SharedPhysics::C_Spring** spring)
{
	Gravity::C_Body* bodyA = NULL;
	Gravity::C_Body* bodyB = NULL;
	if (ResolveConstraintBodies(params.m_ActorHandleA, params.m_ActorHandleB, bodyA, bodyB))
	{
		const Maths::HMat43 matA(bodyA->Orientation(), bodyA->Position());
		const Maths::HMat43 matB(bodyB->Orientation(), bodyB->Position());

		TConstraintHandle constraint = KInvalidConstraintHandle;
		if (!params.m_AxisLimits.m_Limit)
		{
			Gravity::SphericalJointDescriptor desc;
			desc.DefineAnchorPoints(params.m_WorldPos * matA.InverseRotTrans(), params.m_WorldPos * matB.InverseRotTrans());
			constraint = desc.CreateConstraint(*m_Scene->m_pGraphManager, bodyA, bodyB);
		}
		else
		{
			Gravity::SphericalJointLimitDescriptor desc;
			desc.DefineAnchorPoints(params.m_WorldPos * matA.InverseRotTrans(), params.m_WorldPos * matB.InverseRotTrans());
			desc.DefineLimits(params.m_AxisLimits.m_Low, params.m_AxisLimits.m_High);
			constraint = desc.CreateConstraint(*m_Scene->m_pGraphManager, bodyA, bodyB);
		}

		constraint->SetBodyAMassScale(params.m_BodyAMassScale);
		constraint->SetUserData(params.m_UserData);
		m_Scene->AddDebugConstraint(constraint);

		// Create spring if wanted:
		if (params.m_CreateSpring && spring)
		{
			(*spring) = CreateSpring(bodyA, bodyB, params.m_WorldPos, matA, matB);
		}

		return constraint;
	}

	//LOGWARN(Actors::g_Log, DBSTR"unable to create spherical constraint");
	return KInvalidConstraintHandle;
}


TConstraintHandle C_ConstraintActor::CreateSliderConstraint(const TCreationParams& params, SharedPhysics::C_Spring** spring)
{
	Gravity::C_Body* bodyA = NULL;
	Gravity::C_Body* bodyB = NULL;
	if (ResolveConstraintBodies(params.m_ActorHandleA, params.m_ActorHandleB, bodyA, bodyB))
	{
		const Maths::HMat43 matA(bodyA->Orientation(), bodyA->Position());
		const Maths::HMat43 matB(bodyB->Orientation(), bodyB->Position());

		TConstraintHandle constraint = KInvalidConstraintHandle;
		if (!params.m_Extents.m_Limit)
		{
			Gravity::PrismaticJointDescriptor desc;
			desc.DefineJointAxis(params.m_Axis * matA.Rotation().InverseRotation(), params.m_Axis * matB.Rotation().InverseRotation());
			constraint = desc.CreateConstraint(*m_Scene->m_pGraphManager, bodyA, bodyB);
		}
		else
		{
			Gravity::PrismaticJointLimitDescriptor desc;
			desc.DefineJointAxis(params.m_Axis * matA.Rotation().InverseRotation(), params.m_Axis * matB.Rotation().InverseRotation());
			desc.DefineJointLimit(params.m_Extents.m_Min, params.m_Extents.m_Max);
			constraint = desc.CreateConstraint(*m_Scene->m_pGraphManager, bodyA, bodyB);
		}
		constraint->SetUserData(params.m_UserData);
		m_Scene->AddDebugConstraint(constraint);

		// Create spring if wanted:
		if (params.m_CreateSpring && spring)
			(*spring) = CreateSpring(bodyA, bodyB, matA.AW(), matA, matB);

		return constraint;
	}

	//LOGWARN(Actors::g_Log, DBSTR"unable to create slider constraint");
	return KInvalidConstraintHandle;
}


TConstraintHandle C_ConstraintActor::CreateHingeConstraint(const TCreationParams& params, SharedPhysics::C_Spring** spring)
{
	Gravity::C_Body* bodyA = NULL;
	Gravity::C_Body* bodyB = NULL;
	if (ResolveConstraintBodies(params.m_ActorHandleA, params.m_ActorHandleB, bodyA, bodyB))
	{
		const Maths::HMat43 matA(bodyA->Orientation(), bodyA->Position());
		const Maths::HMat43 matB(bodyB->Orientation(), bodyB->Position());
		TConstraintHandle constraint = KInvalidConstraintHandle;
		if (!params.m_Extents.m_Limit)
		{
			Gravity::RevoluteJointDescriptor desc;
			desc.DefineAnchorPoints(params.m_WorldPos * matA.InverseRotTrans(), params.m_WorldPos * matB.InverseRotTrans());
			desc.DefineJointAxis(params.m_Axis * matA.Rotation().InverseRotation(), params.m_Axis * matB.Rotation().InverseRotation());
			constraint = desc.CreateConstraint(*m_Scene->m_pGraphManager, bodyA, bodyB);
		}
		else
		{
			Gravity::RevoluteJointLimitDescriptor desc;
			desc.DefineAnchorPoints(params.m_WorldPos * matA.InverseRotTrans(), params.m_WorldPos * matB.InverseRotTrans());
			desc.DefineJointAxis(params.m_Axis * matA.Rotation().InverseRotation(), params.m_Axis * matB.Rotation().InverseRotation());
			desc.DefineJointLimit(params.m_Extents.m_Min, params.m_Extents.m_Max);
			constraint = desc.CreateConstraint(*m_Scene->m_pGraphManager, bodyA, bodyB);
		}
		constraint->SetUserData(params.m_UserData);
		m_Scene->AddDebugConstraint(constraint);

		// Create spring if wanted:
		if (params.m_CreateSpring && spring)
			(*spring) = CreateSpring(bodyA, bodyB, params.m_WorldPos, matA, matB);

		return constraint;
	}

	//LOGWARN(Actors::g_Log, DBSTR"unable to create hinge constraint");
	return KInvalidConstraintHandle;
}


//?
// TConstraintHandle C_ConstraintActor::CreateRotationalLimitConstraint(Actor::C_ActorHandle actorHandleA, Actor::C_ActorHandle actorHandleB, const Maths::HVec3& lowLimit, const Maths::HVec3& highLimit, void* userData)
// {
// 	Gravity::C_Body* bodyA = NULL;
// 	Gravity::C_Body* bodyB = NULL;
// 	if (ResolveConstraintBodies(actorHandleA, actorHandleB, bodyA, bodyB))
// 	{
// 		Gravity::RotationalLimitDescriptor desc;
// 
// 		desc.DefineLimits(lowLimit, highLimit);
// 		TConstraintHandle constraint = desc.CreateConstraint(*m_Scene->m_pGraphManager, bodyA, bodyB);
// 		constraint->SetUserData(userData);
// 		m_Scene->AddConstraint(constraint);
// 
// 		return constraint;
// 	}
// 
// 	return KInvalidConstraintHandle;
// }


void C_ConstraintActor::ReleaseConstraint(TConstraintHandle& constraintHandle)
{
	if (constraintHandle != KInvalidConstraintHandle)
		m_Scene->RemoveConstraint(constraintHandle);
	else
		LOGERROR(Actor::GetDebugLogGeneral(), DBSTR"invalid constraint handle");

	constraintHandle = KInvalidConstraintHandle;
}


// Updates the constraint if a shape changes in one of the constrained bodies.
void C_ConstraintActor::OnBodyShapeChanged(const Gravity::C_Body* body, const Maths::HVec3& offset)
{
	BIZ_ASSERT(body);

	if (!m_ConstraintHandle->ApplyBodyOffset(body, offset))
		LOGERROR(Actors::g_Log, DBSTR"constraint body problem");
}


} //namespace Actors

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif
