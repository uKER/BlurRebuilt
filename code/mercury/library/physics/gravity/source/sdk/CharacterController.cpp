#include "StdAfx.h"
#include "gravity/sdk/charactercontroller.h"
#include "gravity/sdk/body.h"
#include "gravity/sdk/Scene.h"
#include "gravity/narrowphase/shapes.h"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/CapsuleRay.h"
#include "gravity/narrowphase/CapsuleRay.inl"
#include <system/perf.h>


namespace Gravity
{

	C_CharacterController::C_CharacterController(C_Body* pBody, const Maths::HF32 PCREF maxSpeed, const Maths::HF32 PCREF maxAcceleration) : m_pCharacterBody(pBody),
			m_MaxSpeed(maxSpeed), m_MaxAcceleration(maxAcceleration),
			m_TargetVelocity(Maths::Constants<Maths::HVec3>::Zero()), m_TargetAngularVelocity(Maths::Constants<Maths::HVec3>::Zero()),
			m_TargetPosition(Maths::Constants<Maths::HVec3>::Zero()), m_TargetAcceleration(Maths::Constants<Maths::HVec3>::Zero()),
			m_TargetAngularAcceleration(Maths::Constants<Maths::HVec3>::Zero()), m_ExternalForce(Maths::Constants<Maths::HVec3>::Zero()),
			m_JumpSpeed(Maths::Constants<Maths::HF32>::Zero()), m_OriginalMinPenetration(Maths::UNDEFINED), 
		m_OriginalRayLength(Maths::UNDEFINED), m_DeadZone(Maths::UNDEFINED)
	{
		m_pCollisionShape = (Collision::C_CharacterControllerShape*)pBody->GetShape();
		m_OriginalMinPenetration = m_pCollisionShape->m_Controller.m_MinPenetration;
		m_OriginalRayLength = m_pCollisionShape->m_Controller.m_RayLength;
		m_DeadZone = m_pCollisionShape->m_Controller.m_RayLength * Maths::Constants<Maths::HF32>::Half() * Maths::Constants<Maths::HF32>::Quarter();
		m_pCharacterBody->GetBody().m_IBodyInverse.AY(Maths::Reciprocal(m_pCharacterBody->GetBody().m_IBody.AY()));
		m_pCollisionShape->m_Controller.ResetContacts();
	}

	void C_CharacterController::SetRayCollisionFilter(Collision::C_CollisionFilter* pFilter)
	{
		if(m_pCharacterBody)
		{
			m_pCollisionShape->m_Controller.SetCollisionFilter(pFilter);
		}
	}


	void C_CharacterController::InitialiseConstroller(C_Body* pBody, const Maths::HF32 PCREF maxSpeed, const Maths::HF32 PCREF maxAcceleration)
	{
		m_pCharacterBody = pBody;
		m_MaxSpeed = maxSpeed;
		m_MaxAcceleration = maxAcceleration;
		m_pCollisionShape = (Collision::C_CharacterControllerShape*)pBody->GetShape();
		m_ControllerState = E_Normal;
		m_OriginalMinPenetration = m_pCollisionShape->m_Controller.m_MinPenetration;
		m_OriginalRayLength = m_pCollisionShape->m_Controller.m_RayLength;
		m_DeadZone = m_pCollisionShape->m_Controller.m_RayLength * Maths::Constants<Maths::HF32>::Half() * Maths::Constants<Maths::HF32>::Quarter();
		m_pCharacterBody->GetBody().m_IBodyInverse.AY(Maths::Reciprocal(m_pCharacterBody->GetBody().m_IBody.AY()));
		m_pCollisionShape->m_Controller.ResetContacts();
	}

	C_Body* C_CharacterController::GetBody() const
	{
		return m_pCharacterBody;
	}

	void C_CharacterController::SetBody(C_Body* pBody)
	{
		m_pCharacterBody = pBody;
		if(m_pCharacterBody)
			m_pCollisionShape = (Collision::C_CharacterControllerShape*)pBody->GetShape();
		else
			m_pCollisionShape = NULL;
	}

	void C_CharacterController::EnableClimbing()
	{
		m_ControllerState |= E_Climbing;
	}

	void C_CharacterController::DisableClimbing()
	{
		m_ControllerState &= ~E_Climbing;
	}

	Maths::HVec3 C_CharacterController::GetRelativeLinearVelocity() const
	{
		if(!m_pCharacterBody)
			return Maths::Constants<Maths::HVec3>::Zero();

		if(m_LastResult.m_pNode && m_LastResult.m_pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_FrameOfReference))
		{
			const SharedPhysics::C_RigidBody& referenceBody = *m_LastResult.m_pNode->m_pRigidBody;
			const Maths::HVec3 dir(m_pCharacterBody->Position() - referenceBody.CenterOfRotation());
			const Maths::HVec3 angVel(dir ^ referenceBody.m_AngularVelocity);
			const Maths::HVec3 pointVelocity(angVel + referenceBody.m_LinearVelocity);
			return m_pCharacterBody->LinearVelocity() - pointVelocity;
		}
		return m_pCharacterBody->LinearVelocity();
	}

	Maths::HVec3 C_CharacterController::GetRelativeAngularVelocity()
	{
		if(!m_pCharacterBody)
			return Maths::Constants<Maths::HVec3>::Zero();

		if(m_LastResult.m_pNode && m_LastResult.m_pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_FrameOfReference))
		{
			SharedPhysics::C_RigidBody& referenceBody = *m_LastResult.m_pNode->m_pRigidBody;
			const Maths::HVec3 angVel(referenceBody.m_AngularVelocity);
			return m_pCharacterBody->AngularVelocity() - angVel;
		}
		return m_pCharacterBody->AngularVelocity();
	}

	void C_CharacterController::SetTargetPosition(const Maths::HVec3 PCREF position, const Maths::C_Quaternion PCREF rotation)
	{
		m_ControllerState |= E_Override_Position;
		m_TargetPosition = position;
		m_TargetOrientation = rotation;
	}

	void C_CharacterController::SetTargetVelocity(const Maths::HVec3 PCREF velocity, const Maths::HVec3 PCREF angVel)
	{
		m_ControllerState |= E_Override_Velocity;
		m_TargetVelocity = velocity;
		m_TargetAngularVelocity = angVel;
	}

	void C_CharacterController::DisableRotationConstraint()
	{
		m_ControllerState |= E_NoRotationConstraint;
	}

	void C_CharacterController::EnableRotationConstraint()
	{
		m_ControllerState &= ~E_NoRotationConstraint;
	}

	void C_CharacterController::EnableFlyMode()
	{
		m_ControllerState |= E_FlyMode;
	}

	void C_CharacterController::DisableFlyMode()
	{
		m_ControllerState &= ~E_FlyMode;
	}


	void C_CharacterController::SetTargetAcceleration(const Maths::HVec3 PCREF acceleration, const Maths::HVec3 PCREF angAccel)
	{
		m_ControllerState |= E_Override_Acceleration;
		m_TargetAcceleration = acceleration;
		m_TargetAngularAcceleration = angAccel;
	}

	void C_CharacterController::AddForce(const Maths::HVec3 PCREF force)
	{
		m_ControllerState |= E_External_Force;
		m_ExternalForce = force;
	}

	void C_CharacterController::Teleport(const Maths::HVec3 PCREF position, const Maths::C_Quaternion PCREF orientation)
	{
		if(!m_pCharacterBody)
			return;

		m_pCharacterBody->Position(position);
		//[SteveP] Added this as teleport is using body.m_Position and this doesn't seem to get set?
		SharedPhysics::C_RigidBody& body = m_pCharacterBody->GetBody();
		body.m_Position = position;
		body.m_CenterOfMass = body.m_Position + body.m_CenterOfMassOffset * body.m_OrientationMatrix;

		m_pCharacterBody->Orientation(orientation);
		m_ControllerState |= E_Teleport;
	}

	Maths::HF32 C_CharacterController::TeleportWithCollision(const Maths::HVec3 PCREF position,
		const Maths::C_Quaternion PCREF orientation)
	{
		if(!m_pCharacterBody)
			return Maths::Constants<Maths::HF32>::Zero();

		using namespace Maths;

		Gravity::C_Scene* pScene = m_pCharacterBody->GetScene();
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		m_ControllerState |= E_Teleport;

		

		if(pScene)
		{
#if 1 //[SteveP] test. Just a capsule trace to ideal.. if hits then trace again perpendicular to hitnormal.
			m_pCharacterBody->Orientation(orientation);
			SharedPhysics::C_RigidBody& body = m_pCharacterBody->GetBody();

			Collision::C_Ray ray;

			const Maths::HF32	maxNormalDot(0.0001f);
			const Maths::HF32	radius( m_pCollisionShape->m_Controller.m_Capsule.Radius() );

			Maths::HVec3	dpos( position - body.m_Position );
			Maths::HF32		length(dpos.Length());
			C_Normal		dir( dpos* Reciprocal(length), Maths::C_Normal::UNIT_LENGTH_VECTOR );

			ray.InitialiseRay( body.m_Position, dir, length, &m_pCollisionShape->m_Filter, radius );
			//ray.SetDoubleSided(true);

			U32 data[1024];
			SharedPhysics::C_SafeStack<true> m_stack(data, 1024);
		
			pScene->DoRayTrace(ray, m_stack, false);
			Collision::C_RayTracingResult& result = ray.GetResult();

			if(result.Hit())
			{
				Maths::HF32			distance( Maths::Min(length, result.m_distance) );
				Maths::HVec3		hitPosition( body.m_Position  + dir*distance );

				//flipping the normal breaks regular collision if penetrating a small amount.
				//Maths::HVec3		hitNormal( (distance > Maths::HF32(0.0f)) ? result.TriangleNormal() : -result.TriangleNormal() );
				Maths::HVec3		hitNormal( result.TriangleNormal() );
			
				const Maths::HF32 dot( hitNormal | dir );

				//LOGWARN( g_Log, "collision dot %.2f", (F32)dot );

				if(dot < maxNormalDot )
				{
					const Maths::HF32	hitEpsilon( 0.001f );
					Maths::HF32			hitT( distance/length );
					
//					DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dLine(body.m_Position, hitPosition, RGBA(0, 255, 0, 255));
//					DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dCross(hitPosition, 0.01f, Maths::RGBA(255, 255, 255, 255));
//					DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dArrow(hitPosition, result.TriangleNormal().m_V, 0.1f, 0.01f, Maths::RGBA(255, 255, 255, 255));

					{	//another trace want to try and slide along perpendicular to the hitNormal
						dpos = position - hitPosition;
						dpos = dpos - hitNormal * (hitNormal | dpos );

						length = dpos.Length();
						dir = C_Normal( dpos * Reciprocal(length), Maths::C_Normal::UNIT_LENGTH_VECTOR );

						hitPosition += hitNormal * hitEpsilon;

						Maths::HVec3 newPosition( hitPosition + dpos );
//						DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dLine(hitPosition, newPosition, RGBA(0, 255, 0, 255));

					#if 1
						hitPosition = newPosition;

						ray.InitialiseRay( hitPosition, dir, length, &m_pCollisionShape->m_Filter, radius );
						pScene->DoRayTrace(ray, m_stack, false);

						if(result.Hit())
						{
							distance = Maths::Min(length, result.m_distance);

							//flipping the normal breaks regular collision if penetrating a small amount.
							//hitNormal = (result.m_distance >= Maths::HF32(0.0f)) ? result.TriangleNormal() : -result.TriangleNormal();
							hitNormal = result.TriangleNormal();

							if((hitNormal | dir) < maxNormalDot)
							{
								hitPosition = (hitPosition  + dir*distance) + hitNormal * hitEpsilon;

//								DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dCross(hitPosition, 0.01f, Maths::RGBA(255, 0, 255, 255));
//								DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dArrow(hitPosition, hitNormal, 0.1f, 0.01f, Maths::RGBA(255, 0, 255, 255));
							}
						}
					#endif
					}

					body.m_Position = hitPosition;
					body.m_CenterOfMass = body.m_Position + body.m_CenterOfMassOffset * body.m_OrientationMatrix;

					return hitT; //not accurate if second ray code is enabled, however currently result is not used.
				}
				//else
				//{
				//	This detection is not reliable, we'd need the hit point on the triangle to help guess a truely backward face.
				//#ifndef CFG_SPEED
				//	if(distance < 0.0f)
				//	{
				//		LOGWARN( g_Log, "CharacterController TeleportWithCollision() detected double sided collision mesh!\nPlease see debug rendered contact" );
	
				//		DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dPoint(result.HitPoint(), 0.01f, Maths::RGBA(255, 255, 255, 255));
				//		DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dCross(hitPosition, 0.02f, Maths::RGBA(255, 255, 255, 255));
				//		DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dArrow(hitPosition, result.TriangleNormal().m_V, 0.15f, 0.02f, Maths::RGBA(255, 255, 255, 255));
				//	}
				//#endif
				//}
			}	

			body.m_Position = position;
			body.m_CenterOfMass = body.m_Position + body.m_CenterOfMassOffset * body.m_OrientationMatrix;
#else
			//Original
			m_pCharacterBody->Orientation(orientation);
			Collision::C_Ray ray;
			SharedPhysics::C_RigidBody& body = m_pCharacterBody->GetBody();

			ray.m_bFatTrace = true;

			//[SteveP] only want the upper capsule to trace..
			//Collision::C_Capsule& capsule = m_pCollisionShape->m_Controller.m_EnclosingCapsule;
			Collision::C_Capsule& capsule = m_pCollisionShape->m_Controller.m_Capsule;

			const Maths::HVec3 d(position - body.m_Position);
			const Maths::HF32 length(d.Length());
			const HF32 recipLength=Reciprocal(length);
			const C_Normal dir(d*recipLength, Maths::C_Normal::UNIT_LENGTH_VECTOR);
			const HVec3 end(capsule.End());
			const HVec3 start(capsule.Start());
			const Maths::HF32 height(Maths::Abs((end - start).Y()*half));
			ray.InitialiseRay(body.m_Position, dir, length, &m_pCollisionShape->m_Filter, capsule.Radius() + height);
			//ray.InitialiseRay(capsule.Centre(), capsule.Centre() + dir * length, &m_pCollisionShape->m_Filter);
			ray.SetMaxCosAngle(-0.5f);

//			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(position, 0.25f, Maths::RGBA(255, 255, 0, 0));
//			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(0, 255, 0, 255));

			const U32 size = 1024;
			U32 data[size];
			SharedPhysics::C_SafeStack<true> m_stack(data, size);
		
			pScene->DoRayTrace(ray, m_stack, false);
			Collision::C_RayTracingResult& result = ray.GetResult();
			if(result.Hit())
			{
				const Maths::HVec3 extents(capsule.Radius(), height + capsule.Radius(), capsule.Radius());
				const Maths::HF32 dif(ray.m_Radius - (extents | Maths::Abs(ray.m_Direction.m_V)));
				const Maths::HF32 distance(Maths::Min(length, result.m_distance + dif));
				const Maths::HVec3 newPosition(body.m_Position  + dir*distance);
				body.m_Position = newPosition;
				body.m_CenterOfMass = body.m_Position + body.m_CenterOfMassOffset * body.m_OrientationMatrix;
				return distance/length;
			}	

			body.m_Position = position;
			body.m_CenterOfMass = body.m_Position + body.m_CenterOfMassOffset * body.m_OrientationMatrix;
#endif
			return Constants<HF32>::One();
		}
		return Constants<HF32>::Zero();
	}

	void C_CharacterController::Jump(const Maths::HF32 PCREF speed)
	{
		m_ControllerState |= E_Jumping;
		m_JumpSpeed = speed;

		// [Oscar] attempted workaround for a memory overwrite, looks like m_pCharacterBody could be pointing to deleted memory, m_pCharacterBody is NULL in this case
		if (m_pCharacterBody != NULL)
		{
			m_pCollisionShape->m_Controller.m_MinPenetration = Maths::Constants<Maths::HF32>::Zero();
		}
	}

	void C_CharacterController::ResetState()
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		U32 oldState = m_ControllerState;
		m_ControllerState = E_Normal;
		if(oldState & E_Jumping)
		{
			if(m_pCharacterBody)
			{
				const Maths::Scalar_Bool bJumping(m_pCharacterBody->GetBody().m_LinearVelocity.Y().GT(zero));
				m_pCollisionShape->m_Controller.m_MinPenetration = Maths::Select(bJumping, zero, m_OriginalMinPenetration);
				//m_pCollisionShape->m_Controller.m_RayLength = Maths::Select(bJumping, m_OriginalRayLength, m_OriginalRayLength);
				if(bJumping)
				{
					m_ControllerState |= E_Jumping;
				}
			}
		}
		if(oldState & E_Climbing || oldState & E_FlyMode)
		{
			m_pCollisionShape->m_Controller.m_MinPenetration = zero;
		}
		m_ControllerState |= (oldState & E_Teleport);
		m_ControllerState |= (oldState & E_FlyMode);
		m_ControllerState |= (oldState & E_NoRotationConstraint);
		m_ControllerState |= (oldState & E_Climbing);
			
	}
	
	void C_CharacterController::ResetMinPenetration()
	{
		DisableFlyMode();
		m_pCollisionShape->m_Controller.m_MinPenetration = m_OriginalMinPenetration;
	}

	bool C_CharacterController::GetOnGroundFlags()
	{
		return m_LastResult.m_pShape != NULL;
	}

	Collision::C_RayTracingResult& C_CharacterController::GetFeetCollisions()
	{
		return m_LastResult;
	}

	void C_CharacterController::SetFeetMaterial(U16 feetMaterial)
	{
		if (m_pCollisionShape)
			m_pCollisionShape->m_Controller.m_FeetMaterial = feetMaterial;
	}

	void C_CharacterController::Update(const Maths::HF32 PCREF dt)
	{
		if(!m_pCharacterBody)
			return;

		//PERF_EVENT("Gravity.CharacterController.Update");
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
		const Maths::HF32 invDt(dt.Reciprocal());
		//Not in contact, so no control over controller!

		Collision::C_RayTracingResult& result(m_pCollisionShape->m_Controller.m_Ray.m_result);
		m_LastResult = result;
		SharedPhysics::C_RigidBody& body = m_pCharacterBody->GetBody();
		Maths::HVec3 targetVelocity(m_TargetVelocity);
		Maths::HVec3 jumpVec(Maths::HVec3(zero, zero, zero));
		if(result.m_pShape != NULL)
		{
			jumpVec.Y(m_JumpSpeed);
			const Maths::HF32 speed(targetVelocity.Length());
			targetVelocity.Y(targetVelocity.Y() + Maths::Min(body.m_LinearVelocity.Y(), speed));
		}
		else
		{
			targetVelocity.Y(targetVelocity.Y() + body.m_LinearVelocity.Y());
			jumpVec.Y(jumpVec.Y());
		}
		
		Maths::HVec3 finalVelocity(body.m_LinearVelocity);
		Maths::HVec3 finalPosition(body.m_Position);
		Maths::HVec3 finalAngularVelocity(body.m_AngularVelocity);
		m_JumpSpeed = zero;
		if(m_ControllerState & E_Teleport)
		{
			finalVelocity = Maths::Constants<Maths::HVec3>::Zero();
			finalAngularVelocity = Maths::Constants<Maths::HVec3>::Zero();;
			m_ControllerState &= ~E_Teleport;
		}
		else if ( result.m_pShape != NULL || m_ControllerState & E_FlyMode)
		{

			Maths::HVec3 referenceLinearVelocity(Maths::Constants<Maths::HVec3>::Zero());
			Maths::HVec3 referenceAngularVelocity(Maths::Constants<Maths::HVec3>::Zero());
			if(result.m_pShape != NULL)
			{
				if(result.m_pNode != NULL && result.m_pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_FrameOfReference))
				{
					Gravity::C_Body* referenceBody = result.m_pNode->m_pBody;
					const Maths::HVec3 dir(result.HitPoint() - referenceBody->CenterOfRotation());
					referenceLinearVelocity = referenceBody->LinearVelocity() + (dir ^ referenceBody->AngularVelocity());
					referenceAngularVelocity = referenceBody->AngularVelocity();
				}

				if((result.m_pShape->GetShape() == Collision::E_TriangleMesh_Shape || 
				result.m_pShape->GetShape() == Collision::E_InstancedTriangleMesh_Shape))
				{
					//const Maths::HF32 scale(Maths::Constants<Maths::HF32>::Quarter());
					//const Maths::HVec3 norm(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY());
					//const Maths::HVec3 sphereCentre((m_pCollisionShape->m_Controller.m_Ray.m_start + 
					//	m_pCollisionShape->m_Controller.m_Ray.m_Direction * result.m_distance) + referenceLinearVelocity * dt);
					////Calculate ground plane...
					//const Maths::C_Normal groundNormal(result.TriangleNormal());
					//const Maths::HF32 d(sphereCentre | groundNormal);
					////Now find the closest point to final position on the plane...

					//const Maths::HF32 dis((groundNormal | finalPosition) - d);
					//const Maths::HVec3 closest(finalPosition - (groundNormal * dis));
					//const Maths::HVec3 characterCentre(closest + norm * (m_pCollisionShape->m_Controller.m_RayLength - m_pCollisionShape->m_Controller.m_Capsule.LocalEnd().Y()));
					//finalPosition.Y(characterCentre.Y());

					const Maths::HF32 scale(Maths::Constants<Maths::HF32>::Quarter());
					const Maths::HVec3 norm(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY());
					const Maths::HVec3 sphereCentre(m_pCollisionShape->m_Controller.m_Ray.m_start + 
						m_pCollisionShape->m_Controller.m_Ray.m_Direction * result.m_distance);
					const Maths::HVec3 finalPos(sphereCentre + norm * (m_pCollisionShape->m_Controller.m_RayLength - m_pCollisionShape->m_Controller.m_Capsule.LocalEnd().Y()));
					const Maths::HF32 penCorrection(finalPos.Y() - finalPosition.Y());
					finalPosition += norm * penCorrection * scale;
				}
			}
				
			if(m_ControllerState & E_Override_Position)
			{
				const Maths::HVec3 targetVelocity((m_TargetPosition - finalPosition) * invDt + referenceLinearVelocity);
				const Maths::C_Quaternion angVelQuat(Maths::NearestDifference(body.m_Orientation, m_TargetOrientation));
				const Maths::HVec3 normalisedImag(Maths::RotationAxis(angVelQuat));
				const Maths::HF32 angle(-Maths::RotationAngle(angVelQuat));
				const Maths::HVec3 angVelComp(normalisedImag * angle * invDt);
				const Maths::HVec3 targetAngularVelocity(angVelComp + referenceAngularVelocity);

				const Maths::HF32 targetSpeed(targetVelocity.Length());
				const Maths::HVec3 clampedVelocity(targetVelocity * (Maths::Min(m_MaxSpeed/targetSpeed, one)));
				Maths::HVec3 targetAcceleration((clampedVelocity - finalVelocity) * invDt);
				const Maths::HF32 targetAccelerationMagnitude(targetAcceleration.Length());
				const Maths::HVec3 clampedAcceleration(targetAcceleration * Maths::Min(m_MaxAcceleration/targetAccelerationMagnitude, one));
				finalVelocity += clampedAcceleration * dt;
				finalAngularVelocity = targetAngularVelocity;
			}
			else if (m_ControllerState & E_Override_Velocity)
			{
				const Maths::HF32 targetSpeed(targetVelocity.Length());
				const Maths::HVec3 clampedVelocity((targetVelocity * (Maths::Min(m_MaxSpeed/targetSpeed, one)) + referenceLinearVelocity));
				Maths::HVec3 targetAcceleration((clampedVelocity - finalVelocity) * invDt + referenceAngularVelocity);
				const Maths::HF32 targetAccelerationMagnitude(targetAcceleration.Length());
				const Maths::HVec3 clampedAcceleration(targetAcceleration * Maths::Min(m_MaxAcceleration/targetAccelerationMagnitude, one));
				finalVelocity += clampedAcceleration * dt;
				finalAngularVelocity = m_TargetAngularVelocity;
			}
			else if (m_ControllerState & E_Override_Acceleration)
			{
				const Maths::HVec3 targetAcceleration = m_TargetAcceleration;// - (negatedAcceleration * result.m_normal);
				const Maths::HF32 targetAccelerationMagnitude(targetAcceleration.Length());
				const Maths::HVec3 clampedAcceleration(targetAcceleration * Maths::Min(m_MaxAcceleration/targetAccelerationMagnitude, one));
				finalVelocity += clampedAcceleration * dt;
				finalAngularVelocity += m_TargetAngularVelocity * dt;
			}
		
			if (m_ControllerState & E_External_Force)
			{
				m_pCharacterBody->AddForce(m_ExternalForce);
			}

			if(m_ControllerState & E_NoRotationConstraint)
			{
				finalAngularVelocity = body.m_AngularVelocity;
			}
		}
		
		

		const U32 oldState(m_ControllerState);

		if(oldState & E_Jumping)
		{
			finalVelocity += jumpVec;			
		}
		finalVelocity += (finalPosition - body.m_Position) * invDt;
		body.StoreVelocitiesAndPosition(finalVelocity, finalAngularVelocity,
			body.m_Orientation, finalPosition);
		m_pCollisionShape->m_Controller.ResetContacts();
	}


	Maths::HVec3 C_CharacterController::Position() const
	{
		if(!m_pCharacterBody)
			return Maths::Constants<Maths::HVec3>::Zero();

		return m_pCharacterBody->Position();
	}

	bool C_CharacterController::operator < (const C_CharacterController& otherController) const
	{
		if(!m_pCharacterBody)
			return false;

		return (*m_pCharacterBody) < (*(otherController.m_pCharacterBody));
	}

	bool C_CharacterController::operator <= (const C_CharacterController& otherController) const
	{
		if(!m_pCharacterBody)
			return false;

		return (*m_pCharacterBody) <= *(otherController.m_pCharacterBody);
	}

	bool C_CharacterController::operator > (const C_CharacterController& otherController) const
	{
		if(!m_pCharacterBody)
			return false;

		return (*m_pCharacterBody) > *(otherController.m_pCharacterBody);
	}

	bool C_CharacterController::operator >= (const C_CharacterController& otherController) const
	{
		if(!m_pCharacterBody)
			return false;

		return (*m_pCharacterBody) >= *(otherController.m_pCharacterBody);
	}

	bool C_CharacterController::operator == (const C_CharacterController& otherController) const
	{
		if(!m_pCharacterBody)
			return false;

		return (*m_pCharacterBody) == *(otherController.m_pCharacterBody);
	}

	bool C_CharacterController::operator != (const C_CharacterController& otherController) const
	{
		if(!m_pCharacterBody)
			return false;

		return (*m_pCharacterBody) != *(otherController.m_pCharacterBody);
	}


};


