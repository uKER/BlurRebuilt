#include "stdafx.h"
#include "racingphysics/vehicles/car/car.h"
//#include "racingphysics/collisions/collisionmesh.h"
#include "racingphysics/collisions/sweptsphere.h"
#include "racingphysics/collisions/spherecollision.h"
#include "racingphysics/physics/random.h"
#include "racingphysics/physics/physicstimer.h"

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off)
#endif

namespace RacingPhysics
{

void CarWheel::InitialiseConstants(int index, Car* pCar, Rotor* pRotor)
{
	m_pCar = pCar;
	m_pRotor = pRotor;

	m_wheelIndex = index;

	m_signZ = (m_wheelIndex < 2) ? -1.f : 1.f;
	m_signX = (m_wheelIndex % 2) ? -1.f : 1.f;
}

#if !PLATFORM_SPU

void CarWheel::Initialise()
{
	m_dd.frame.groundNormalForce.Initialise();
	m_dd.frame.groundFriction.Initialise();
	m_dd.frame.groundTorque.Initialise();
	m_dd.frame.groundPosition.Initialise();
	m_dd.frame.groundNormal.Initialise();
	m_dd.frame.groundVelocity.Initialise();
	Clear(&m_dd.frame.smoothGroundForce);
	m_dd.frame.onGround = true;
	m_dd.frame.suspensionDistance.Initialise();
	m_dd.frame.slipAngle.Initialise();
	m_dd.frame.wheelSpin.Initialise();
	m_dd.frame.surfaceFlags = 0;
	m_dd.frame.surfaceModifier = 0;
	m_dd.frame.userData = 0;
	Clear(&m_dd.frame.restPosition);
	Clear(&m_dd.frame.wheelPosition);
	m_dd.frame.surfaceGripScale = 1.f;
	m_dd.frame.surfaceRoughnessScale = 1.f;
	m_dd.frame.surfaceMIScale = 1.f;
	m_dd.frame.angularVelocity1D = 0.f;

	m_debugSlipRecorder.Initialise();

	CacheAxis();
}

void CarWheel::Shutdown()
{
	m_debugSlipRecorder.Shutdown();
}

#endif

FloatVR CarWheel::ForwardsGripScaleInGear(S32 gear) const
{
	if (gear < 1 || TyreDynamics().forwardsGripScaleInGear.count < gear)
		return 1.f;
	else
		return TyreDynamics().forwardsGripScaleInGear[gear - 1];
}

FloatVR CarWheel::ForwardsGripScaleDry() const 
{
	FloatVR f =	TyreDynamics().grip.yScale * 
				TyreDynamics().forwardsGripScale * 
				m_pCar->PerformanceModifiers().forwardGripScale * 
				m_pCar->PerformanceModifiers().wheelModifiers[m_wheelIndex].gripScale *
				ForwardsGripScaleInGear(m_pCar->Transmission().Gear());

	return f;
}

bool CarWheel::Validate() const
{
	RP_ASSERT(GetRotor().GetRotationMatrix().DebugTest());
	RP_ASSERT(Abs(GetRotor().AngularVelocity().Y()) < 10000.f);
	return true;
}

Point3R CarWheel::CalculateRestPosition(const Axis& carBody) const
{
	FloatVR wb = Dynamics().geometry.wheelBase;
	FloatVR tw = (m_wheelIndex < 2) ? Dynamics().geometry.trackWidthFront : Dynamics().geometry.trackWidthRear;

	Vector3R offset = carBody.Global(m_dd.offset);
	return carBody.Position() + offset
			+ VEHICLE_REVERSE_Z_SIGN carBody.AxisZ() * wb * 0.5f * m_signZ
			+ VEHICLE_REVERSE_Z_SIGN carBody.AxisX() * tw * 0.5f * m_signX;
}

Normal3R CarWheel::CalculateSuspensionAxis(const Axis& carBody) const
{
	return Rotated(carBody.AxisY(), VEHICLE_REVERSE_Z_SIGN carBody.AxisX(), FloatVR(SuspensionDynamics().casterAngle));
}

void CarWheel::GetCollisionBoundingBox(Point3R& min, Point3R& max) const
{
	FloatVR s = Radius() + SuspensionDynamics().movementDown;
	Point3R p = CalculateRestPosition(m_pCar->Body().GetAxis());
	Point3R min2 = p - Vector3R(s, s, s);
	Point3R max2 = p + Vector3R(s, s + Radius(), s);

	min = Min(min, min2);
	max = Max(max, max2);
}

void CarWheel::SetCarAxis(const Axis& axis, float steeringAngle)
{
	m_pRotor->SetRotation(axis);
	m_pRotor->Rotate(CalculateSuspensionAxis(axis) * steeringAngle);

	m_dd.frame.wheelPosition = CalculateRestPosition(m_pCar->Body().GetAxis()) + CalculateSuspensionAxis(m_pCar->Body().GetAxis()) * m_dd.frame.suspensionDistance.GetValue();
}

void CarWheel::SetOnGround(bool b)
{
	m_dd.frame.onGround = b;
}

void CarWheel::BeginIntegrate()
{
	m_dd.frame.groundNormalForce.Begin();
	m_dd.frame.groundFriction.Begin();
	m_dd.frame.groundTorque.Begin();
	m_dd.frame.groundPosition.Begin();
	m_dd.frame.groundNormal.Begin();
	m_dd.frame.groundVelocity.Begin();
	m_dd.frame.suspensionDistance.Begin();
	m_dd.frame.wheelSpin.Begin();
	m_dd.frame.slipAngle.Begin();
	
	{
		m_dd.frame.surfaceModifier = 0;
		m_dd.frame.surfaceModifierDepth = 0.f;
		m_dd.frame.userData = 0;

		CollisionCache::Contact contact;
		FloatVR offset = CollisionRadius() * 2.f;
		GetGroundIntersection(contact, GetWheelPosition(), CalculateSuspensionAxis(m_pCar->Body().GetAxis()), offset, &m_pCar->WheelOverlayFilter(),
			&m_PuddleCachedTriangle);

		if (contact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
		{
			m_dd.frame.surfaceModifier = contact.surfaceFlags;
			m_dd.frame.userData = contact.userData;
			m_dd.frame.surfaceModifierDepth = offset - contact.intersectionDistance;
		}
	}

	if (m_dd.frame.surfaceModifier == 0)
	{
		m_dd.frame.surfaceGripScale = Surfaces::GetGripScale(m_dd.frame.surfaceFlags, 0, Magnitude(m_dd.frame.groundVelocity.GetValue()));
		if (m_wheelIndex > 1) m_dd.frame.surfaceGripScale *= Surfaces::GetRearGripScale(m_dd.frame.surfaceFlags, 0);
		m_dd.frame.surfaceRoughnessScale = Surfaces::GetRoughnessFactor(m_dd.frame.surfaceFlags, 0);
		m_dd.frame.surfaceRollingFriction = Surfaces::GetRollingFriction(m_dd.frame.surfaceFlags, 0) * m_pCar->PerformanceModifiers().surfaceRollingFrictionScale;
		m_dd.frame.surfaceMIScale = Surfaces::GetMIScale(m_dd.frame.surfaceFlags, 0);
	}
	else
	{
		float f = Maths::Clamped(m_dd.frame.surfaceModifierDepth / Radius(), 0.f, 1.f);
		m_dd.frame.surfaceGripScale = Surfaces::GetGripScale(m_dd.frame.surfaceFlags, m_dd.frame.surfaceModifier, f, 0, Magnitude(m_dd.frame.groundVelocity.GetValue()));
		if (m_wheelIndex > 1) m_dd.frame.surfaceGripScale *= Surfaces::GetRearGripScale(m_dd.frame.surfaceFlags, m_dd.frame.surfaceModifier, f, 0);
		m_dd.frame.surfaceRoughnessScale = Surfaces::GetRoughnessFactor(m_dd.frame.surfaceFlags, m_dd.frame.surfaceModifier, f, 0);
		m_dd.frame.surfaceRollingFriction = Surfaces::GetRollingFriction(m_dd.frame.surfaceFlags, m_dd.frame.surfaceModifier, f, 0);
		m_dd.frame.surfaceMIScale = Surfaces::GetMIScale(m_dd.frame.surfaceFlags, m_dd.frame.surfaceModifier, f, 0);
	}
}

void CarWheel::EndIntegrate()
{
	m_dd.frame.groundNormalForce.End();
	m_dd.frame.groundFriction.End();
	m_dd.frame.groundTorque.End();
	m_dd.frame.groundPosition.End();
	m_dd.frame.groundNormal.End();
	m_dd.frame.groundVelocity.End();
	m_dd.frame.wheelSpin.End();
	m_dd.frame.slipAngle.End();
	m_dd.frame.groundNormal.SetValue(MakeVector(Normalised(m_dd.frame.groundNormal.GetAccumulatedValue())));

	if (!OnGround())
		m_dd.frame.suspensionDistance.SetValue(Maths::Lerp(m_dd.frame.suspensionDistance.GetValue(), -SuspensionDynamics().movementDown, m_pCar->TimeStep()));
	else
		m_dd.frame.suspensionDistance.End();

	m_dd.frame.wheelPosition = CalculateRestPosition(m_pCar->Body().GetAxis()) + CalculateSuspensionAxis(m_pCar->Body().GetAxis()) * m_dd.frame.suspensionDistance.GetValue();

	float smoothTime = 0.5f;
	m_dd.frame.smoothGroundForce = ExponentialSmoothing(m_dd.frame.groundNormalForce.GetValue(), m_dd.frame.smoothGroundForce, m_pCar->TimeStep(), smoothTime);

	m_dd.frame.angularVelocity1D = CalculateAngularVelocity1D();
}

#if !PLATFORM_SPU

void CarWheel::EndFrame()
{
	// slip recorder
	{
		Point3R p;
		float s;
		
		if (OnGround())
		{
			p = GroundPosition();
			s = Maths::Sqrt(SlipAngle() * SlipAngle() / (LongitudinalTyreSlip() * LongitudinalTyreSlip())
								+ WheelSpin() * WheelSpin() / (TransverseTyreSlip() * TransverseTyreSlip()));
		}
		else
		{
			p = GetWheelPosition() - m_pCar->Body().GetAxis().Global(Vector3R(0.f, Radius(), 0.f));
			s = 0;
		}

		m_debugSlipRecorder.Update(p, s, OnGround());
	}
}

#endif

void CarWheel::ClearStepVariables()
{
	memset(&m_step, 0xffffffff, sizeof(m_step));
}

void CarWheel::SetMassVariables()
{
	float mi = (m_wheelIndex < 2) ? Dynamics().mass.frontWheelMI : Dynamics().mass.rearWheelMI;

	m_pRotor->SetMassVariables(mi);

	m_dd.driven = Dynamics().transmission.driveType == CarDynamicsXt::DRIVE_TYPE_FOURWHEEL
		|| ((m_wheelIndex < 2) && Dynamics().transmission.driveType == CarDynamicsXt::DRIVE_TYPE_FRONTWHEEL)
		|| ((m_wheelIndex > 1) && Dynamics().transmission.driveType == CarDynamicsXt::DRIVE_TYPE_REARWHEEL);
}

void CarWheel::PutWheelOnGround()
{
	m_step.angularVelocity1D = CalculateAngularVelocity1D();
	RP_ASSERT(_finite(m_step.angularVelocity1D));
	m_step.suspensionAxis = CalculateSuspensionAxis(m_pCar->Body().GetAxis());
	m_step.restPosition = CalculateRestPosition(m_pCar->Body().GetAxis());
	m_step.staticLoad = CalculateStaticLoad();
	m_step.steeringAngle = CalculateSteeringAngle();

	CollisionCache::Contact contact;
	FloatVR offset =  CollisionRadius() + SuspensionDynamics().movementUp;
	GetGroundIntersection(contact, RestPosition_Step(), SuspensionAxis_Step(), offset, &m_pCar->WheelGroundFilter(), &m_CachedTriangle);
	
	if (contact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
	{
		m_step.onGround = true;
		m_step.groundNormal = contact.intersectionNormal;
		m_step.groundPosition = contact.intersectionPoint;
		m_step.suspensionDistance = offset - contact.intersectionDistance;
		m_step.wheelPosition = RestPosition_Step() + SuspensionAxis_Step() * m_step.suspensionDistance;
		m_step.groundZ = Normalised(GetRotationMatrix().AxisX() ^ m_step.groundNormal);
		m_step.groundX = MakeNormal(m_step.groundNormal ^ m_step.groundZ);

		float toe = ((m_wheelIndex % 2) ? -1.f : 1.f) * SuspensionDynamics().toeAngle + GetAckermanSteeringCorrection(m_step.steeringAngle);
		m_step.groundZ = Rotated(m_step.groundZ, SuspensionAxis_Step(), FloatVR(toe));
		m_step.groundX = Rotated(m_step.groundX, SuspensionAxis_Step(), FloatVR(toe));

		//Vector3R groundVelocity(0.f, 0.f, 20.f);
		//groundVelocity += Constants<Normal3R>::Y() * (float) (Maths::Cos(PhysicsTimer::Instance().PhysicsGetTimer()) * 4.f);
		//if (m_step.groundPosition.z > 0.f) m_step.groundVelocity = groundVelocity;
		//else
		
		m_step.groundVelocity = contact.groundVelocity;

		m_dd.frame.suspensionDistance.Accumulate(m_step.suspensionDistance * AccumulationFactor());
		m_dd.frame.groundPosition.Accumulate(MakeVector(m_step.groundPosition) * AccumulationFactor());
		m_dd.frame.groundNormal.Accumulate(MakeVector(m_step.groundNormal) * AccumulationFactor());
		m_dd.frame.groundVelocity.Accumulate(m_step.groundVelocity * AccumulationFactor());

		if (m_pCar->StepIndex() == 0) // first step this frame
		{
			m_dd.frame.surfaceFlags = contact.surfaceFlags;
			m_dd.frame.userData = contact.userData;
			m_dd.frame.onGround = true;
		}

		CalculateSlip();
	}
	else
	{
		m_step.onGround = false;
		m_dd.frame.onGround = false;
		m_dd.frame.surfaceFlags = 0;
		m_dd.frame.surfaceModifier = 0;
		m_dd.frame.userData = 0;
	}
}

float CarWheel::CalculateSteeringAngle() const
{
	if (m_wheelIndex < 2)
	{
		Normal3R bodyZ = m_pCar->GetAxis().AxisZ();
		Normal3R suspensionAxis = CalculateSuspensionAxis(m_pCar->Body().GetAxis());
		Normal3R wheelX = GetRotationMatrix().AxisX();
		Normal3R wheelZ = Normalised(wheelX ^ suspensionAxis);

		return Maths::ArcSin(wheelZ | (suspensionAxis ^ bodyZ));
	}
	else
		return 0.f;
}

float CarWheel::GetAckermanSteeringCorrection(float heading) const
{
	if (Abs(heading) > 0.01f)
	{
		float speed = Magnitude(m_pCar->GetVelocity());
		float l = Dynamics().geometry.wheelBase;
		float w = Dynamics().geometry.trackWidthFront * Dynamics().steering.ackerman;
		float r = l / Maths::Tan(heading);
		float s = Maths::ArcTan(l / (r - ((m_wheelIndex % 2) ? 1.f : -1.f) * w / 2.f));
		float speedFactor = Clamped(1.f - ((speed - 5.f) / 5.f), 0.f, 1.f);
		return (s - heading) * speedFactor;
	}
	else
		return 0.f;
}

void CarWheel::CalculateSlip()
{
	if (OnGround_Step())
	{
		Normal3R ax = GetRotationMatrix().AxisX();
		Maths::Store(ax | GroundNormal_Step(), &m_step.camberAngle);

		Vector3R wheelVelocity = m_pCar->Body().PointVelocity(m_step.groundPosition) - m_step.groundVelocity;

		FloatVR pz = wheelVelocity | GroundZ_Step();
		FloatVR px = wheelVelocity | GroundX_Step();

		FloatVR angularVelocity = VEHICLE_FORWARD_Z_SIGN AngularVelocity1D_Step();
		FloatVR slipSpeedFactor = FloatVR(TyreDynamics().slipSpeed);

		FloatVR w = angularVelocity * TyreDynamics().radius;

		FloatVR wheelSpinFactor = 1.f / (Abs(pz) + TyreDynamics().slipSpeed);
		m_step.wheelSpinFactor = wheelSpinFactor;
		m_step.wheelSpin = VEHICLE_REVERSE_Z_SIGN (w - pz) * wheelSpinFactor;
		m_step.slipAngle = VEHICLE_REVERSE_Z_SIGN px * wheelSpinFactor;

		m_dd.frame.wheelSpin.Accumulate(m_step.wheelSpin * AccumulationFactor());
		m_dd.frame.slipAngle.Accumulate(m_step.slipAngle * AccumulationFactor());

		// static friction
		{
			m_step.staticFrictionX = 0.f;
			m_step.staticFrictionZ = 0.f;

			FloatVR s = Sqrt(pz * pz + px * px);
			FloatVR sMax = TyreDynamics().dynamicFrictionSpeed;

			if (s < sMax)
			{
				FloatVR speedFactor = 1.f - s / sMax;
				FloatVR spin = WheelSpin_Step() / LongitudinalTyreSlip();
				FloatVR spinFactor = Max(1.f - Abs(spin), FloatVR(0.f));
				m_step.staticFrictionX = Max(speedFactor, FloatVR(0.f)) * spinFactor;

				float brake = m_pCar->Brake();
				if (Driven()) brake *= (1.f - m_pCar->Throttle());
				float handbrake = 0.f;
				if (m_wheelIndex > 1) handbrake = m_pCar->Handbrake();

				m_step.staticFrictionZ = speedFactor * spinFactor * AddSmooth(brake, handbrake);
			}
		}
	}
}

float GetGroundOffset(const Point3R& position, float speed)
{
	float s_vibrationScale = 0.005f;
	float s_noisePositionScale = 25.f;
	float s_maxSpeed = 20.f;

	float fx = Abs(position.X()) * s_noisePositionScale;
	float fz = Abs(position.Z()) * s_noisePositionScale;

	int x = int(fx);
	int z = int(fz);

	float frx = fx - x;
	float frz = fz - z;

	float filteredNoise = (Noise(x) * (1.f - frx) +  Noise(x + 1) * frx)
						* (Noise(z) * (1.f - frz) +  Noise(z + 1) * frz);

	float offset = filteredNoise
				* s_vibrationScale
				* Min(speed, s_maxSpeed);

	return offset;
}

void CarWheel::ResetTriangleCache()
{
	m_CachedTriangle.m_bHit = false;
	m_PuddleCachedTriangle.m_bHit = false;
}

void CarWheel::GetGroundIntersection(CollisionCache::Contact& contact, const Point3R& restPosition, const Normal3R& suspensionDirection, FloatVR heightOffset,
									 const Collision::C_CollisionFilter* filter, Collision::C_SingleTriangleCache* pCache)
{
	float radius = CollisionRadius();
	
	if (radius == 0.f)
	{
		contact.collisionType = CollisionCache::COLLISION_TYPE_NONE;
		return;
	}
	Spu::MemBaseMarker marker;

	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	float offset = GetGroundOffset(restPosition, (float)Magnitude(m_pCar->Body().Velocity()) * m_pCar->FrameTime() / PhysicsTimer::Instance().PhysicsUpdateTime());
	offset *= m_dd.frame.surfaceRoughnessScale;

	Vector3R bump = Constants<Normal3R>::Y() * offset;
	
	Point3R startPosition = restPosition + bump + suspensionDirection * heightOffset;
	float maxDistance = heightOffset + SuspensionDynamics().movementDown;

	{
		//PERF_EVENT("RacingPhysics.CarWheel.GetGroundIntersection");

		Collision::C_Ray ray;
		ray.InitialiseRay(	startPosition,
							-suspensionDirection,
							maxDistance,
							filter,
							radius);
		ray.SetMaxCosAngle(-Maths::Constants<Maths::HF32>::Quarter());

		m_pCar->GetLineCollisionCache().Trace(ray, pCache);

		contact.collisionType = CollisionCache::COLLISION_TYPE_NONE;

		if (ray.m_result.m_pShape != NULL)
		{
			
			//KierS - knocked out because this may be causing cars to get beached if the ground is slightly above the wheel
			/*if(ray.m_result.m_distance >= Maths::Constants<Maths::HF32>::Zero() 
				&& (ray.m_result.m_normal | -suspensionDirection) < Maths::Constants<Maths::HF32>::Zero())*/
			if((ray.m_result.m_normal | -suspensionDirection) < Maths::Constants<Maths::HF32>::Zero()) 
			{
				//Kier - Adding on a small value because rounding issues can become aparent with raytrace
				//functions at distances far away from the centre of the level
				RP_ASSERT(ray.m_result.m_distance <= (maxDistance + 0.001f));
				contact.collisionType = CollisionCache::COLLISION_TYPE_TRIANGLE_SURFACE;
				contact.intersectionDistance = Maths::Max(ray.m_result.m_distance, zero);
				contact.intersectionNormal = ray.m_result.m_normal;
				//contact.intersectionNormal = Maths::C_Normal(ray.m_end - ray.m_result.m_contact);
				contact.intersectionPoint = ray.m_result.m_contact;
				contact.surfaceFlags = ray.m_result.m_EffectMaterialIndex;
				contact.userData = ray.m_result.GetUserData();
			}

			//Kier - commented this out for Blur - they don't have platforms to drive on, so avoid the DMA!
#if 0
			if(ray.m_result.m_pNode)
			{
				SharedPhysics::C_RigidBody* pRigidBody = (SharedPhysics::C_RigidBody*)(((U8*)ray.m_result.m_pNode) + sizeof(SharedPhysics::C_GraphNode));
				//SharedPhysics::C_GraphNode* pNode = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(ray.m_result.m_pNode, sizeof(SharedPhysics::C_GraphNode));
				//SharedPhysics::C_RigidBody* pRigid = (SharedPhysics::C_RigidBody*)Spu::FetchAlloc(pNode->m_pRigidBody, sizeof(SharedPhysics::C_RigidBody));
				SharedPhysics::C_RigidBody* pRigid = (SharedPhysics::C_RigidBody*)Spu::FetchAlloc(pRigidBody, sizeof(SharedPhysics::C_RigidBody));
				contact.groundVelocity = pRigid->m_LinearVelocity + ((ray.m_result.m_contact - pRigid->CenterOfRotation()) ^ pRigid->m_AngularVelocity);
			}
			else
#endif
			{
				contact.groundVelocity = Maths::Constants<Maths::HVec3>::Zero();
			}

		}
	}
}

// To improve high-speed stability, the wheels rotate at a lower rate than they should.
FloatVR CarWheel::RotationScale() const
{
	FloatVR angularVelocity = GetRotor().AngularVelocity() | GetDriveAxis();

	if (Abs(angularVelocity) > 10.f)
	{
		static float s_wheelRotationScale = 4.f;

		FloatVR vw = (Abs(angularVelocity) - 10.f) * Radius();
		FloatVR vb = Abs(m_pCar->GroundSpeed_LocalZ());
		FloatVR vmax = (200.f * PhysicsConstants::MPH);
		return 1.f + s_wheelRotationScale * (vw * vb) / (vmax * vmax);
	}

	return 1.f;
}

FloatVR CarWheel::CalculateAngularVelocity1D() const 
{
	Vector3R av = GetRotor().AngularVelocity() - m_pCar->Body().AngularVelocity();
	FloatVR w = -(av | GetDriveAxis());
	return w * RotationScale();
}

FloatVR CarWheel::GetTyreForceScale(FloatVR slip, FloatVR maxSlip) const
{
	if (slip > 0.001f)
		return (TyreDynamics().grip.Evaluate(Min(slip, maxSlip)) * SurfaceGripScale()) / slip;
	else 
		return 1.f;
}

float CarWheel::CalculateStaticLoad() const
{
	float staticLoad = Dynamics().mass.mass * 0.25f * PhysicsConstants::GRAVITY * (1.f + m_pCar->CalculateDownforceFactor());
	return staticLoad;
}

FloatVR CarWheel::SlipAlignment() const
{
	FloatVR av = AngularVelocity1D_Step();
	FloatVR slipAlignment = TyreDynamics().slipAlignment_Av.Evaluate(Maths::Abs(av), true);	
	FloatVR f = Clamped(av * 0.03f, FloatVR(0.f), FloatVR(1.f));
	return f * slipAlignment;
}

float CarWheel::GetTyreSaturation(float s) const
{
	float limit = TyreDynamics().grip.dataPoints[2].y;
	float c = TyreDynamics().grip.dataPoints[2].x - 1.f;

	float s2 = s * s;
	float s4 = s * s * s * s;

	float f1 = Maths::Select_LT(s, 1.f, 
								1.5f * (s2 + s) / (s4 + s + 1.f), 
								0.f);
	float sr = s - 1.f;
	float sr2 = sr * sr;

	float f2 = Maths::Select_GE(s, 1.f, 
								1.f - (1.f - limit) * sr2 / (sr2 + c), 
								0.f);

	return f1 + f2;
}

void CarWheel::CalculateDynamicFrictionForce(Vector3R& force, Vector3R& torque) const
{
	RP_ASSERT(OnGround_Step());

	float spin = WheelSpin_Step() / LongitudinalTyreSlip();
	float fmax = m_dd.frame.groundNormalForce.GetValue() | GroundNormal_Step();
	float gripScaleZ = (spin > 0.f) ? BackwardsGripScale() : ForwardsGripScale();

	float slip = SlipAngle_Step() / TransverseTyreSlip();

	float s = Maths::Sqrt(slip * slip + spin * spin);
	float f = GetTyreSaturation(s);

	float fx = VEHICLE_REVERSE_Z_SIGN Maths::Select_NE(s, 0.f, f * (slip / s), 0.f);
	float fz = VEHICLE_REVERSE_Z_SIGN Maths::Select_NE(s, 0.f, f * (spin / s), 0.f);

	force = fmax * (-GroundX_Step() * TransverseGripScale() * fx 
					+ GroundZ_Step() * gripScaleZ * fz);

	if (m_wheelIndex < 2)
	{
		Normal3R ny = MakeNormal(GroundZ_Step() ^ GetRotationMatrix().AxisX());
		float a =  ClampedAbs(slip, 1.f) * SlipAlignment() / Max(s, 1.f);
		torque = ny * (-fmax * a);
	}
	else
		Clear(&torque);
}

void CarWheel::GetDmaData(void** pDmaData)
{
	pDmaData[0] = &m_dd;
}

void CarWheel::SpuFetch(const void* pDmaData)
{
	Spu::Fetch(&m_dd, pDmaData, sizeof(DmaData));
}

void CarWheel::SpuStore(void* pDmaData) const
{
	Spu::Store(pDmaData, &m_dd, sizeof(DmaData));
}

}
