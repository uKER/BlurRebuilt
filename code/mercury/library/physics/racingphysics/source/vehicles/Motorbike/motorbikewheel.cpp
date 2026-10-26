#include "stdafx.h"
#include "racingphysics/vehicles/motorbike/motorbike.h"
//#include "racingphysics/collisions/collisionmesh.h"
#include "racingphysics/collisions/toruscollision.h"
#include "racingphysics/physics/random.h"

//using namespace Maths;

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize("", off) // DEBUG_RACING_PHYSICS
#endif

namespace RacingPhysics
{

Debug::Float g_bikeGroundOffset("RacingPhysics.Bikes.BikeGroundOffset", -0.01f, Debug::Range(-0.1f, 0.1f));
Debug::Float g_slipSpeed("RacingPhysics.Bikes.SlipSpeed", 10.f, Debug::Range(0.f, 100.f), Debug::Step(0.01f));
Debug::Float g_wheelRotationScale("RacingPhysics.Bikes.WheelRotationScale", 4.f, Debug::Range(0.f, 10.f), Debug::Step(0.01f));
Debug::Float g_tractionControlPower("RacingPhysics.Bikes.TractionControlPower", 4.f, Debug::Range(0.f, 10.f));

//Debug::Float g_tractionControlMaxSlip("RacingPhysics.Bikes.TractionControlMaxSlip", 10.f, Debug::Range(0.f, 20.f));
//Debug::Float g_brakingMaxSlip("RacingPhysics.Bikes.BrakingMaxSlip", 8.f, Debug::Range(0.f, 20.f));
//Debug::Float g_camberSlipAlignmentFactor("RacingPhysics.Bikes.CamberSlipAlignmentFactor", 0.2f, Debug::Range(0.f, 1.f));

/////////////////////////////////////////////////////////////////////////////

void MotorbikeWheel::Initialise(int index, Motorbike* pMotorbike, RigidBody* pRigidBody)
{
	m_pMotorbike = pMotorbike;
	m_pRigidBody = pRigidBody;

	Body().Initialise(0);

	m_index = index;

	SetMassVariables();

	// rear wheel is the drive wheel
	m_driven = (m_index == 1) ? true : false;

	m_frame.groundNormalForce.Initialise();
	m_frame.groundFriction.Initialise();
	m_frame.groundTorque.Initialise();
	m_frame.groundPosition.Initialise();
	m_frame.groundNormal.Initialise();
	m_frame.onGround = false;
	m_frame.slipAngle.Initialise();
	m_frame.wheelSpin.Initialise();
	m_frame.slipAngleWithCamber.Initialise();
	m_frame.slipScaleFactor.Initialise();
	m_frame.surfaceType = 0;
	m_frame.surfaceModifier = -1;
	m_frame.slipScaleFactorValid = true;

	m_debugSlipRecorder.Initialise();
}

void MotorbikeWheel::ClearStepVariables()
{
	memset(&m_step, 0xffffffff, sizeof(m_step));
}

void MotorbikeWheel::Shutdown()
{
	m_debugSlipRecorder.Shutdown();
}

void MotorbikeWheel::SetMassVariables()
{
	float mass = m_pMotorbike->WheelMass();
	float r2 = Dynamics().geometry.WheelRadius(m_index) * Dynamics().geometry.WheelRadius(m_index);

	// MI of hoop is m * r^2
	// Perpendicular Axes Rule => other two axes are half this.
	float MI_x, MI_yz;
	
	if (m_index == 0)
		MI_x = MI_yz = mass * r2 * Dynamics().mass.wheelMIScaleFront;
	else
		MI_x = MI_yz = mass * r2 * Dynamics().mass.wheelMIScaleRear; // hack

	Body().SetMassVariables(mass, Vector3R(MI_x, MI_yz, MI_yz), Point3R(0.f, 0.f, 0.f));

	m_inertia = MI_x;
}

void MotorbikeWheel::GetGroundIntersection(CollisionCache::Contact& contact, const Point3R& position, const Normal3R& normal) const
{
	float offset = 0.f;

	/*if (m_frame.surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_GRASS 
		|| m_frame.surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_GRAVEL
		|| m_frame.surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_STICKY) 
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

		offset = filteredNoise
					* s_vibrationScale
					* Min((float)Magnitude(Body().Velocity()), s_maxSpeed);
	}*/

	TorusCollision collision(position - Constants<Normal3R>::Y() * (Dynamics().suspension.tyreSpringMovement - offset), 
									normal,
									Dynamics().geometry.WheelRadius(m_index) - Dynamics().geometry.TubeRadius(m_index),
									Dynamics().geometry.TubeRadius(m_index));

	//contact = collision.GetDeepestCollision(*m_pMotorbike->CollisionCachePtr(), 0);
}

void MotorbikeWheel::PutWheelOnGround()
{
	float depth = 0;

	{
		CollisionCache::Contact contact;
		GetGroundIntersection(contact, Body().Position(), Body().GetAxis().AxisX());
		
		if (contact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
		{
			depth = contact.intersectionDistance;
			m_step.groundContactPosition = contact.intersectionPoint - contact.intersectionNormal * depth;
			m_step.groundNormal = contact.intersectionNormal;

			if (m_pMotorbike->StepIndex() == 0) // first step this frame
			{
				m_frame.surfaceType = 0;//CollisionMeshSurfaceFlags(contact.surfaceFlags).Surface();
				m_frame.onGround = true;
			}

			if (contact.intersectionPoint.Y() > -0.1f)
				int breakhere = 0;
		}
	}

	if (depth > 0.f)
	{
		if (depth > 0.1)
			int breakhere = 0;

		m_step.onGround = true;
		m_step.groundPosition = m_step.groundContactPosition + m_step.groundNormal * depth;
		m_step.groundZ = Normalised(Body().GetAxis().AxisX() ^ m_step.groundNormal);
		m_step.groundX = MakeNormal(m_step.groundNormal ^ m_step.groundZ);
		m_step.bodyGroundZ = Normalised(m_pMotorbike->Body().GetAxis().AxisX() ^ m_step.groundNormal);
		m_step.bodyGroundX = MakeNormal(m_step.groundNormal ^ m_step.bodyGroundZ);

		m_frame.groundPosition.Accumulate(MakeVector(m_step.groundPosition) * AccumulationFactor());
		m_frame.groundNormal.Accumulate(MakeVector(m_step.groundNormal) * AccumulationFactor());

		CalculateSlip();

		if (m_step.groundNormal.Y() < 0.5f)
			int breakhere = 0;

		RP_ASSERT(Magnitude(m_step.groundPosition - Body().Position()) < Radius() * 2.f);
	}
	else
	{
		m_step.onGround = false;
		m_frame.onGround = false;
		m_frame.surfaceType = 0;
		m_frame.surfaceModifier = -1;
		m_frame.slipScaleFactor.Accumulate(1.f * AccumulationFactor());
	}
}

// To improve high-speed stability, the wheels rotate at a lower rate than they should.
float MotorbikeWheel::RotationScale() const
{
	float angularVelocity = Body().AngularVelocity() | Body().GetAxis().AxisX();

	if (Abs(angularVelocity) > g_slipSpeed)
	{
		float vw = (Abs(angularVelocity) - g_slipSpeed) * Radius();
		float vb = Abs(m_pMotorbike->Speed_GlobalXZ());
		float vmax = (200.f * PhysicsConstants::MPH);
		return 1.f + g_wheelRotationScale * (vw * vb) / (vmax * vmax);
	}

	return 1.f;
}

//float g_brakeRollReduction = 4.f;
//float g_maxRearBrakeSlip = 8.f;
//float g_maxFrontBrakeSlip = 2.f;
//float g_frontBrakeLeanFactor = 10.f;
float g_rollFactor1 = 2.f;
float g_rollFactor2 = 2.f;
float g_rollRateFactor1 = 0.1f;
float g_rollRateFactor2 = 0.1f;
float g_steerRotationFactor = 0.25f;

void MotorbikeWheel::CalculateSlip()
{
	if (OnGround_Step())
	{
		//Vector3R v = Body().PointVelocity(m_groundContactPosition);
		Normal3R ax = Body().GetAxis().AxisX();

		Maths::Store(ax | GroundNormal_Step(), &m_step.camberAngle);

		int bi = (m_index == 0) ? m_pMotorbike->ForkIndex() : m_pMotorbike->BodyIndex();
		Vector3R wheelVelocity = m_pMotorbike->GetRigidBody(bi).PointVelocity(m_step.groundContactPosition);

		float pz = wheelVelocity | GroundZ_Step();
		float px = wheelVelocity | GroundX_Step();

		float angularVelocity = Body().AngularVelocity() | ax;
		float slipSpeedFactor = Dynamics().tyres.slipSpeed;

		Vector3R d = (Body().GetAxis().Position() - m_step.groundContactPosition);
		d -= ax * (d | ax);
		float effectiveRadius = Magnitude(d);

		float w = angularVelocity * effectiveRadius * RotationScale();

		m_step.wheelSpin = (w - pz) / (Abs(pz) + slipSpeedFactor);
		m_step.slipAngle = px / (Abs(pz) + g_slipSpeed);
		m_step.slipFactor = Abs(pz) + g_slipSpeed;

		m_frame.wheelSpin.Accumulate(m_step.wheelSpin * AccumulationFactor());
		m_frame.slipAngle.Accumulate(m_step.slipAngle * AccumulationFactor());

		float slipScaleFactor = SlipScaleFactor();
		RP_ASSERT(slipScaleFactor >= 0.f && slipScaleFactor <= 1.f);
		m_frame.slipScaleFactor.Accumulate(slipScaleFactor * AccumulationFactor());

		if (!m_frame.slipScaleFactorValid)
		{
			m_frame.slipScaleFactorValid = true;
			m_frame.slipScaleFactor.SetValue(slipScaleFactor);
		}

		RP_ASSERT(m_frame.slipScaleFactor.GetAccumulatedValue() <= 1.f);

		float steerAngle = (m_pMotorbike->Fork().GetAxis().AxisZ() | m_pMotorbike->Body().GetAxis().AxisX());
		float steerRotationRate = (m_pMotorbike->Fork().AngularVelocity() - m_pMotorbike->Body().AngularVelocity()) | m_pMotorbike->Fork().GetAxis().AxisY();

		// camber force
		{
			m_step.slipAngleWithCamber = SlipAngle_Step()
										+ tan(CamberAngle_Step()) 
										* CamberForceScale() 
										* m_frame.slipScaleFactor.GetValue()
										* TransverseTyreSlip() 
										* Min(Abs(angularVelocity) * 0.1f, 1.f)
										* Min(Abs(m_pMotorbike->Speed_GlobalXZ()) * 0.05f, 1.f)
										/ Max(TransverseGripScale(), 1.f);

			m_frame.slipAngleWithCamber.Accumulate(m_step.slipAngleWithCamber * AccumulationFactor());
		}

		// static friction
		{
			m_step.staticFrictionX = 0.f;
			m_step.staticFrictionZ = 0.f;

			float s = Sqrt(pz * pz + px * px);
			float sMax = Dynamics().tyres.dynamicFrictionSpeed;

			if (s < Dynamics().tyres.dynamicFrictionSpeed)
			{
				float speedFactor = 1.f - s / Dynamics().tyres.dynamicFrictionSpeed;
				float spin = WheelSpin_Step() / LongitudinalTyreSlip();
				float spinFactor = Max(1.f - Abs(spin), 0.f);
				float steerFactor = Max(1.f - Abs(steerAngle) * 2.f, 0.f);
				//float rollFactor = Clamped(1.f - Abs(CamberAngle_Step()) * 2.f, 0.f, 1.f);
				m_step.staticFrictionX = speedFactor * spinFactor * steerFactor;// * rollFactor;

				if (m_index == 1)
					m_step.staticFrictionZ = speedFactor * spinFactor * steerFactor * m_pMotorbike->RearBrake();
				else
					m_step.staticFrictionZ = speedFactor * spinFactor * steerFactor * m_pMotorbike->FullBrakes();
			}
		}

		// traction control and ABS
		{
			float maxSinRoll = m_pMotorbike->MaxSinRoll_Step() * Dynamics().steering.rollFraction;
			float sinRoll = m_pMotorbike->SinRoll_Step();
			float steerSmooth = m_pMotorbike->SteerSmooth();
			float r = steerSmooth * maxSinRoll - sinRoll;
			float dr = m_pMotorbike->RollRate_Step();
			float rearBrake = m_pMotorbike->RearBrake();
			float throttle = m_pMotorbike->Throttle();
			float forwardSpeed = Max(m_pMotorbike->Speed_GlobalXZ(), 0.f);
			float speed = Magnitude(m_pMotorbike->Body().Velocity());
			float spin = WheelSpin_Step() / LongitudinalTyreSlip();
			float slip = SlipAngleWithCamber_Step() / TransverseTyreSlip();
			float fullBrakes = m_pMotorbike->FullBrakes();
			int gear = m_pMotorbike->Engine().Gear();

			// traction control - allows wheel spin while steering for slides
			if (Driven() && gear > 0)
			{
				float f = 1.f;//Dynamics().gears.gearRatios[gear - 1] / Dynamics().gears.gearRatios[0];

				// reduced if grip needed for steering
				float gripFraction = Clamped(1.05f + (r * g_rollFactor1 + dr * g_rollRateFactor1) * sinRoll, 0.f, 1.f);
				//float gripFraction = Clamped(1.f - Abs(r + dr * g_rollRateFactor) / 2.f, 0.f, 1.f);
				f *= gripFraction * gripFraction;

				// increased when steering and accelerating
				//float speedFactor = Max(1.f - speed / (15.f * PhysicsConstants::MPH), 0.f);
				//f *= 1.f + 10.f * Dynamics().engine.tractionControlMaxSpin * AddSmooth(Abs(steerSmooth), fullBrakes) * speedFactor;

				// reduced at large slip angles
				float maxSlip = Dynamics().engine.tractionControlMaxSlip;
				float maxSpin = Dynamics().engine.tractionControlMaxSpin;
				float speedFactor = Clamped((speed - 1.f) / (Dynamics().engine.wheelSpinMaxSpeed_Mph * PhysicsConstants::MPH), 0.f, 1.f);
				float leanFactor = Clamped(-m_pMotorbike->LeanZ() * 2.f * (1.f - fullBrakes) * (1.f - rearBrake), 0.f, 1.f);
				float slipFactor = Max(1.f - Abs(slip) * (1.f - Abs(steerSmooth)) / 2.f, 0.f);
				float burnoutFactor = (1.f - speedFactor) * (1.f - leanFactor) * slipFactor;

				float target = (Dynamics().engine.limiterRPM * PhysicsConstants::RPM / m_pMotorbike->Engine().GearRatio(1)) * effectiveRadius 
								/ (slipSpeedFactor * LongitudinalTyreSlip());

				target = Maths::Lerp(target, 10.f, Abs(steerSmooth));

				f *= Maths::Lerp(1.f, maxSpin, throttle * speedFactor * Min(Abs(slip) * 2.f / maxSpin, 1.f));
				f *= Maths::Lerp(1.f, target, burnoutFactor * burnoutFactor);

				//f *= 1.f - Min(Abs(steerAngle + steerRotationRate * 0.25f), 1.f) * speedFactor;

				//f *= Clamped(1.f - (Abs(slip) - maxSlip) / maxSlip, 0.f, 2.f);
				f *= 1.f - Min(Abs(slip) / maxSlip, 1.f);

				m_step.tractionControlAV = (-f * LongitudinalTyreSlip() * (Abs(pz) + slipSpeedFactor) + pz) / (effectiveRadius * RotationScale());
			}

			// braking
			{
				float f = 1.f;
				//float rollRateFactor = Lerp(0.25f, 1.f, rearBrake);

				if (m_index == 0) //front brake
				{
					// reduce if the bike wants to stand up
					float gripFraction = Clamped(1.f + (r + dr * 0.25f) * Sign(sinRoll), 0.f, 1.f);
					f *= gripFraction * gripFraction;

					f *= 1.f - Clamped((Abs(steerAngle + steerRotationRate * 0.15f) - 0.5f) * 2.f, 0.f, 1.f);

					// reduce braking if slipping
					// reduced at large slip angles
					float maxSlip = Dynamics().brakes.frontBrakeMaxSlip;
					//f *= Clamped(1.f - (Abs(slip) - maxFrontBrakeSlip) / maxFrontBrakeSlip, 0.f, 1.f);
					f *= 1.f - Min(Abs(slip) / maxSlip, 1.f);
				}
				else // rear brake
				{
					// reduced at large slip angles
					float maxSlip = Maths::Lerp(2.f, Dynamics().brakes.rearBrakeMaxSlip, rearBrake * Min(speed / 20.f, 1.f));
					float maxSpin = Dynamics().brakes.rearBrakeMaxSpin * 2.f / (2.f + fullBrakes);

					f *= Maths::Lerp(1.f, maxSpin, rearBrake * Min(Abs(slip) / maxSlip, 1.f));
					f *= 1.f - Min(Abs(slip) / maxSlip, 1.f);

					//f *= 1.f - Min(Abs(steerAngle + steerRotationRate * g_steerRotationFactor), 1.f);

					//if (Abs(slip) > maxSlip)
					//	int breakhere = 0;
				}

				f *= Clamped(1.f + (r * g_rollFactor2 + dr * g_rollRateFactor2) * sinRoll, 0.f, 1.f);

				m_step.brakingAV = (f * LongitudinalTyreSlip() * (Abs(pz) + slipSpeedFactor) + pz) / (effectiveRadius * RotationScale());
			}
		}
	}
}

float MotorbikeWheel::TransverseTyreSlip() const
{
	return Dynamics().tyres.transverseTyreSlipScale * Dynamics().tyres.TyreSlipScale(m_index);
}

float MotorbikeWheel::LongitudinalTyreSlip() const
{
	return Dynamics().tyres.longitudinalTyreSlipScale * Dynamics().tyres.TyreSlipScale(m_index);
}

float MotorbikeWheel::GetTyreForceScale(float slip1, float slip2, float maxSlip) const
{
	float slip = Sqrt(slip1 * slip1 + slip2 * slip2);

	if (slip > 0.001f)
		return (Dynamics().tyres.grip.Evaluate(Min(slip, maxSlip)) * SurfaceGripScale() * Dynamics().tyres.GripScale(m_index)) / slip;
	else 
		return 1.f;
}

float MotorbikeWheel::SlipScaleFactor() const
{
	if (m_index == 0)
	{
		return 1.f;
	}
	else
	{
		float combinedSlip = (WheelSpin_Step() > 0.f) 
							? (Dynamics().tyres.combinedSlipBackwards * m_pMotorbike->RearBrake())
							: Dynamics().tyres.combinedSlipForwards_Mph.Evaluate(m_pMotorbike->Speed_GlobalXZ() / PhysicsConstants::MPH, true);
		
		float spin = Abs(WheelSpin_Step() / LongitudinalTyreSlip());

		return 1.f / (1.f + spin * combinedSlip * Abs(CamberAngle_Step()));
	}
}

//float g_slipAlignmentRear = 1.f;

void MotorbikeWheel::CalculateDynamicFrictionForce(Vector3R& force, Vector3R& torque)
{
	if (OnGround_Step())
	{
		FloatVR fmax = m_frame.groundNormalForce.GetValue() | GroundNormal_Step();

		FloatVR spin = WheelSpin_Step() / LongitudinalTyreSlip();
		FloatVR slip = SlipAngleWithCamber_Step() / TransverseTyreSlip();
		FloatVR slipScaleFactor = m_frame.slipScaleFactor.GetValue();

		if (WheelSpin_Step() > 0.f) // braking
		{
			FloatVR scaleX = GetTyreForceScale(slip * slipScaleFactor, spin, 1.f);
			FloatVR scaleZ = GetTyreForceScale(spin, slip * slipScaleFactor, Max(FloatVR(1.f), Abs(spin)));

			force = fmax * 
					(-GroundX_Step() * Dynamics().tyres.transverseGripScale * scaleX * slip * slipScaleFactor
					+ GroundZ_Step() * Dynamics().tyres.backwardsGripScale * scaleZ * spin);
		}
		else
		{
			FloatVR scaleX = GetTyreForceScale(slip * slipScaleFactor, spin, 1.f);
			FloatVR scaleZ = GetTyreForceScale(spin, slip * slipScaleFactor, Max(FloatVR(1.f), Abs(spin)));

			force = fmax * 
					(-GroundX_Step() * Dynamics().tyres.transverseGripScale * scaleX * slip * slipScaleFactor
					+ GroundZ_Step() * Dynamics().tyres.forwardsGripScale * scaleZ * spin);
		}

		//Clear(&force);

		// Torque from: 
		// 1. Self aligning moment, proportional to slip angle.
		// 2. Twisting moment, proportional to camber.
		// Both effects come from tyre distortion resulting from rolling friction, 
		// so require angular velocity and low wheelspin
		if (m_index == 0)
		{
			FloatVR speed = Abs(m_pMotorbike->Speed_GlobalXZ());
			FloatVR clamp = 1.f;//Lerp(1.f, 0.2f, Clamped(speed / 30.f, 0.f, 1.f));
			FloatVR slip = ClampedAbs(SlipAngle_Step() * slipScaleFactor / TransverseTyreSlip(), clamp);

			FloatVR av = speed / Radius();
			FloatVR slipAlignment = Dynamics().tyres.slipAlignment_Av.Evaluate(Maths::Abs(av), true);
			slipAlignment *= Maths::Lerp(1.f, Dynamics().tyres.camberSlipAlignmentFactor, Min(Abs(CamberAngle_Step()) / 0.75f, 1.f));
			float angularVelocity = Body().AngularVelocity() | Body().GetAxis().AxisX();

			FloatVR f = -Min(Abs(angularVelocity) * 0.1f, 1.f) * Clamped(1.f - Abs(WheelSpin_Step()) / LongitudinalTyreSlip(), 0.f, 1.f);
			FloatVR w = (1.f + m_pMotorbike->DownforceFactor_Step()) * m_pMotorbike->Mass() * PhysicsConstants::GRAVITY;
			w = Max(fmax, w * 0.25f);

			Normal3R ny = MakeNormal(GroundZ_Step() ^ Body().GetAxis().AxisX());

			torque = ny * (f * w * slip * slipAlignment);
			torque -= GroundNormal_Step() * (f * w) * CamberAngle_Step() * Dynamics().tyres.camberAlignment * slipScaleFactor;
		}
		else
			Clear(&torque);
	}
	else
	{
		Clear(&force);
		Clear(&torque);
	}
}

void MotorbikeWheel::BeginIntegrate()
{
	SetMassVariables();

	m_frame.groundNormalForce.Begin();
	m_frame.groundFriction.Begin();
	m_frame.groundTorque.Begin();
	m_frame.groundPosition.Begin();
	m_frame.groundNormal.Begin();
	//m_frame.onGround = true;
	m_frame.slipAngle.Begin();
	m_frame.wheelSpin.Begin();
	m_frame.slipAngleWithCamber.Begin();
	m_frame.slipScaleFactor.Begin();

	// check for water
	{
		m_frame.surfaceModifier = 0;//Surfaces::SURFACE_MODIFIER_UNKNOWN;

		/*if (gRaceState.mTrackSurface == GAME_TRACKSURFACE_DRY)
			m_frame.surfaceModifier = Surfaces::SURFACE_MODIFIER_UNKNOWN;
		else if (gRaceState.mTrackSurface == GAME_TRACKSURFACE_WET)
			m_frame.surfaceModifier = Surfaces::SURFACE_MODIFIER_WET;
		else if (gRaceState.mTrackSurface == GAME_TRACKSURFACE_SNOW)
			m_frame.surfaceModifier = Surfaces::SURFACE_MODIFIER_SNOW;
		else if (gRaceState.mTrackSurface == GAME_TRACKSURFACE_VERY_WET)
			m_frame.surfaceModifier = Surfaces::SURFACE_MODIFIER_VERY_WET;
		else
			RP_ASSERT(!"Unknown track surface type");

		float radius = Radius();

		Vector3R margin(radius, radius, radius);
		Point3R centre = Body().GetAxis().Position();

		Point3R min = centre - margin;
		Point3R max = centre + margin;

		Btl::AutoPointer<CollisionCache> pCache;
		Allocate(pCache);
		CollisionMesh::Instance().BuildCollisionCache(&*pCache, min, max, COLLISION_FLAG_NOT_SOLID|COLLISION_FLAG_NOT_WALL);

		SphereCollision collision(centre, radius);
		CollisionCache::Contact contact = collision.GetFirstCollision(*pCache, 0);

		if (contact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
		{
			CollisionMeshSurfaceFlags surfaceFlags = ((CollisionMeshSurfaceFlags) contact.surfaceFlags);

			if (surfaceFlags.water == CollisionMeshSurfaceFlags::WATER_SHALLOW && gRaceState.mPuddles == GAME_PUDDLES_WATER)
				m_frame.surfaceModifier = Surfaces::SURFACE_MODIFIER_SHALLOW_PUDDLE;
			else if (surfaceFlags.water == CollisionMeshSurfaceFlags::WATER_DEEP && gRaceState.mPuddles == GAME_PUDDLES_WATER)
				m_frame.surfaceModifier = Surfaces::SURFACE_MODIFIER_DEEP_PUDDLE;
			else if (surfaceFlags.ice && gRaceState.mPuddles == GAME_PUDDLES_ICE)
				m_frame.surfaceModifier = Surfaces::SURFACE_MODIFIER_ICE;
		}*/
	}

	m_frame.surfaceGripScale = 1.f;

	/*if (m_frame.surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_GRASS) 
		m_frame.surfaceGripScale *= Surfaces::GetSlidingFriction(Surfaces::SURFACE_TYPE_GRASS, Surfaces::SURFACE_TYPE_BIKE_WHEEL, 0.f);
	else if (m_frame.surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_GRAVEL) 
		m_frame.surfaceGripScale *= Surfaces::GetSlidingFriction(Surfaces::SURFACE_TYPE_GRAVEL, Surfaces::SURFACE_TYPE_BIKE_WHEEL, 0.f);
	else if (m_frame.surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_STICKY) 
		m_frame.surfaceGripScale *= Surfaces::GetSlidingFriction(Surfaces::SURFACE_TYPE_STICKY, Surfaces::SURFACE_TYPE_BIKE_WHEEL, 0.f);
	else if (m_pMotorbike->GetControlType() != RacingPhysics::Vehicle::CONTROL_TYPE_AI)
		m_frame.surfaceGripScale *= Surfaces::GetCitySlidingFriction(Surfaces::SURFACE_TYPE_TARMAC, m_pMotorbike->m_cityIndex);

	if (m_frame.surfaceModifier != Surfaces::SURFACE_MODIFIER_UNKNOWN)
		m_frame.surfaceGripScale *= Surfaces::GetSlidingFriction(m_frame.surfaceModifier, Surfaces::SURFACE_TYPE_BIKE_WHEEL, Abs(m_pMotorbike->Speed_GlobalXZ()));*/
}

float g_groundForceSmoothTime = 0.5f;

void MotorbikeWheel::EndIntegrate()
{
	{
		m_frame.groundNormalForce.End();
		m_frame.groundFriction.End();
		m_frame.groundTorque.End();
		m_frame.groundPosition.End();
		m_frame.groundNormal.End();
		m_frame.slipAngle.End();
		m_frame.wheelSpin.End();
		m_frame.slipAngleWithCamber.End();

		if (m_frame.wheelSpin.GetValue() < 0.f)
			m_frame.slipScaleFactor.End(m_pMotorbike->TimeStep(), Dynamics().tyres.slipSmoothTimeInForwards, Dynamics().tyres.slipSmoothTimeOutForwards);
		else
			m_frame.slipScaleFactor.End(m_pMotorbike->TimeStep(), Dynamics().tyres.slipSmoothTimeInBackwards, Dynamics().tyres.slipSmoothTimeOutBackwards);
		
		RP_ASSERT(m_frame.slipScaleFactor.GetValue() <= 1.f);
		//m_frame.slipScaleFactor.

		m_frame.smoothGroundForce = ExponentialSmoothing(m_frame.groundNormalForce.GetValue(), m_frame.smoothGroundForce, m_pMotorbike->TimeStep(), g_groundForceSmoothTime);
	}

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
			p = Position() - m_pMotorbike->Body().GetAxis().Global(Vector3R(0.f, Radius(), 0.f));
			s = 0;
		}

		m_debugSlipRecorder.Update(p, s, OnGround());
	}
}

//////////////////////////////////////////////////////////////////////

void MotorbikeEngine::Initialise(Motorbike* pMotorbike, Rotor* pRigidBody)
{
	m_pMotorbike = pMotorbike;
	m_pRotor = pRigidBody;

	m_pRotor->Initialise();
	m_pRotor->SetMassVariables(Dynamics().engine.engineInertia);

	m_powerCut = false;
	m_currentGear = 1;
	m_gearDest = 1;
	m_clutchEngaged = 0.f;
	m_gearChangeClutch = 0.f;
	m_gearChange = false;
	m_gearChangeClutchTime = 0.f;

	m_tractionControl.Initialise();
	m_adjustedThrottle.Initialise();
}

void MotorbikeEngine::BeginIntegrate()
{
	m_pRotor->SetMassVariables(Dynamics().engine.engineInertia);

	float rpm = RPM();

	// rev limiter
	float powerCutFraction = m_currentGear < Dynamics().gears.topGear ? Dynamics().engine.powerCutFraction : 1.f;

	if (!m_powerCut && rpm > Dynamics().engine.limiterRPM)
		m_powerCut = true;
	else if (m_powerCut && rpm < Dynamics().engine.limiterRPM * powerCutFraction)
		m_powerCut = false;

	bool hasTraction = false;
	
	if (m_pMotorbike->Wheel(1).OnGround())
	{
		if (m_pMotorbike->Wheel(1).WheelSpin() > -m_pMotorbike->Wheel(1).LongitudinalTyreSlip())
			hasTraction = true;

		// change gear
		if (m_pMotorbike->AutoGears())
		{
			if (m_gearDest == m_currentGear)
			{
				if (m_gearDest < Dynamics().gears.topGear 
					&& m_powerCut 
					&& hasTraction
					&& m_clutchEngaged > 0.75f)
					m_gearDest++;
				else
				{
					float wheelRPM = m_pMotorbike->Wheel(1).AngularVelocity1D() 
									* m_pMotorbike->Wheel(1).RotationScale();

					Normal3R groundZ = Normalised(m_pMotorbike->Wheel(1).Body().GetAxis().AxisX() ^ m_pMotorbike->Wheel(1).GroundNormal());
					float speed = Abs(m_pMotorbike->Body().Velocity() | groundZ);
					float groundRPM = speed / m_pMotorbike->Wheel(1).Radius();

					float r = Dynamics().engine.limiterRPM * Dynamics().gears.gearDownRPMFraction * PhysicsConstants::RPM;

					while (m_gearDest > 1
							&& (m_gearDest > 2 || speed < Dynamics().gears.maxSpeedForChangeToFirst * PhysicsConstants::MPH)
							&& groundRPM * GearRatio(m_gearDest - 1) < r 
							&& wheelRPM * GearRatio(m_gearDest - 1) < r)
						m_gearDest--;
				}
			}
		}

		// manual
		{
			if (m_pMotorbike->CheckControlsPressed())
			{
				if (m_gearDest < Dynamics().gears.topGear && m_pMotorbike->Controls().gearUp)
					m_gearDest++;
				else if (m_gearDest > 1 && m_pMotorbike->Controls().gearDown)
					m_gearDest--;
			}
		}
	}

	if (m_gearDest != m_currentGear)
	{
		if (m_clutchEngaged > 0.f)
			m_gearChangeClutchTime = Dynamics().engine.clutchDelay;

		m_gearChange = true;
		m_clutchEngaged -= m_pMotorbike->TimeStep() / Dynamics().engine.clutchSmoothTime;

		if (m_clutchEngaged <= 0.f)
		{
			m_clutchEngaged = 0.f;
			m_gearChangeClutch = 0.f;
			m_gearChangeClutchTime -= m_pMotorbike->TimeStep();

			if (m_gearChangeClutchTime <= 0.f)
			{
				m_gearChangeClutchTime = 0.f;
				m_currentGear = m_gearDest;
				m_pMotorbike->m_feedback.HandleGearChange(1.f);
			}
		}
	}
	else
	{
		//m_clutchEngaged = ExponentialSmoothing(1.f, m_clutchEngaged, m_pMotorbike->TimeStep(), Dynamics().engine.clutchSmoothTime);
		float cp = m_pMotorbike->TimeStep() / Dynamics().engine.clutchSmoothTime;
		m_clutchEngaged = Min(m_clutchEngaged + cp, 1.f);
		m_gearChangeClutch = Min(m_gearChangeClutch + cp, 1.f);
		float forwardSpeed = Max(m_pMotorbike->Speed_GlobalXZ(), 0.f);

		if (m_gearChangeClutch == 1.f)
			m_gearChange = false;

		float minSpeed = m_pMotorbike->Wheel(1).Radius() * Dynamics().engine.idleRPM / (GearRatio(1) / PhysicsConstants::RPM);
				
		if (forwardSpeed < minSpeed)
		{
			//m_clutchEngaged = Lerp(0.1f, 1.f, forwardSpeed / minSpeed);

			if (rpm < Dynamics().engine.idleRPM * 1.1f)
				m_clutchEngaged = 0.f;
		}
		
		float speedFactor = Clamped((forwardSpeed - 1.f) / (10.f * PhysicsConstants::MPH), 0.f, 1.f);
		//m_clutchEngaged = Min(m_clutchEngaged, 1.f - m_pMotorbike->FullBrakes() * (1.f - m_pMotorbike->Throttle()) * speedFactor);
		m_clutchEngaged = Min(m_clutchEngaged, 1.f - m_pMotorbike->RearBrake() * (1.f - speedFactor));

		if (m_currentGear == 1)
		{
			float throttle = m_pMotorbike->Throttle();
			float clutchFraction = Dynamics().engine.clutchRPMFraction;
			float startRPM = Maths::Lerp(Dynamics().engine.idleRPM * 1.1f, Dynamics().engine.limiterRPM, clutchFraction * throttle);
			float endRPM = Min(startRPM * (1.1f + throttle), Dynamics().engine.limiterRPM);

			float f = (rpm - startRPM) / (endRPM - startRPM);
			f = Clamped(f, 0.f, 1.f);
			m_clutchEngaged = Min(m_clutchEngaged, AddSmooth(f, 1.f - throttle));
		}
	}

	m_tractionControl.Begin();
	m_adjustedThrottle.Begin();
}

void MotorbikeEngine::EndIntegrate()
{
	float s_tractionControlSmoothTime = 0.1f;
	m_tractionControl.End(m_pMotorbike->TimeStep(), s_tractionControlSmoothTime);
	m_adjustedThrottle.End();
}

float MotorbikeEngine::RPM() const
{
	Normal3R x = m_pMotorbike->Body().GetAxis().AxisX();
	Vector3R w = Body().AngularVelocity();

	float rpm = Abs(x | w) / PhysicsConstants::RPM;
	return rpm;
}

float MotorbikeEngine::AngularVelocity1D() const
{
	Normal3R x = m_pMotorbike->Body().GetAxis().AxisX();
	Vector3R w = Body().AngularVelocity() - m_pMotorbike->Body().AngularVelocity();
	return -(x | w);
}

float MotorbikeWheel::AngularVelocity1D() const 
{
	Vector3R av;
	
	if (m_index == 0)
		av = Body().AngularVelocity() - m_pMotorbike->Fork().AngularVelocity();
	else
		av = Body().AngularVelocity() - m_pMotorbike->Body().AngularVelocity();

	float w = -(av | Body().GetAxis().AxisX());
	return w;
}

float MotorbikeEngine::GearRatio(int gear) const
{
	if (gear < 1) return 0.f;
	return Dynamics().gears.finalRatio * Dynamics().gears.gearRatios[gear - 1];
}

float g_compression = 0.0f;
float g_maxCompression = 100.f;

void MotorbikeEngine::EvaluateControlForces(ArticulatedBody::Derivative& derivative, float maxTotalEngineTorque, float loadTorque, float clutchPowerCut)
{
	RP_ASSERT(maxTotalEngineTorque >= 0.f);

	float engineTorque = 0.f;

	Normal3R x = m_pMotorbike->Body().GetAxis().AxisX();
	Vector3R w = Body().AngularVelocity();

	float av = (x | w);
	float throttle = m_pMotorbike->Throttle();

	float rpm = RPM();
	float maxRPM = Dynamics().engine.engineTorque_Rpm.x1;

	// idle and limiter
	if (m_powerCut || m_currentGear < 0) 
		throttle = 0.f;
	else if (rpm < Dynamics().engine.idleRPM * 1.1f) 
	{
		float s = Dynamics().engine.engineInertia * (maxRPM * PhysicsConstants::RPM);
		float engineBrakingAtIdle = Dynamics().engine.engineBraking_RpmFraction.Evaluate(Dynamics().engine.idleRPM / maxRPM, false) * s;
		float peakTorque = Dynamics().engine.engineTorque_Rpm.yScale;
		float idleThrottle = engineBrakingAtIdle / (peakTorque + engineBrakingAtIdle);
		throttle = Max(throttle, Clamped(1.f - rpm / Dynamics().engine.idleRPM + idleThrottle, 0.f, 1.f));
	}

	RP_ASSERT(throttle >= 0.f && throttle <= 1.f);

	throttle *= 1.f - clutchPowerCut * (m_gearChange ? 1.f : 0.f);
	//maxTotalEngineTorque *= 1.f - clutchPowerCut * (m_gearChange ? 1.f : ClutchEngaged());

	// engine braking
	{
		float d = Clamped(av * Dynamics().engine.engineInertia / m_pMotorbike->TimeStep(), -1.f, 1.f);
		float s = Dynamics().engine.engineInertia * (maxRPM * PhysicsConstants::RPM);
		engineTorque = Dynamics().engine.engineBraking_RpmFraction.Evaluate(rpm / maxRPM, false) * d * s;	
		engineTorque *= (1.f - throttle);
	}

	if (loadTorque > 0)
		engineTorque -= Min(loadTorque * g_compression, g_maxCompression) * (1.f - throttle);

	// engine torque
	float t = Dynamics().engine.engineTorque_Rpm.Evaluate(rpm);

	// limit torque according to speed
	float power = t * Abs(av);
	float tmax = m_pMotorbike->Mass() * PhysicsConstants::GRAVITY * 3.f * (Magnitude(m_pMotorbike->Body().Velocity()) + 4.f) / (Abs(av) + 1.f);
	t = Maths::Lerp(Min(t, tmax), t, AddSmooth(m_pMotorbike->FullBrakes(), m_pMotorbike->RearBrake()));

	float tractionControl = (t > maxTotalEngineTorque) ? ((t - maxTotalEngineTorque) / t) : 0.f;
	tractionControl = Maths::Lerp(0.f, tractionControl, ClutchEngaged());
	if (Gear() > 1) tractionControl = Maths::Lerp(tractionControl, 1.f, clutchPowerCut * (m_gearChange ? 1.f : ClutchEngaged()));
	if (m_powerCut) tractionControl = 1.f;
	m_tractionControl.Accumulate(tractionControl * m_pMotorbike->AccumulationFactor());
	throttle *= (1.f - tractionControl);
	m_adjustedThrottle.Accumulate(throttle * m_pMotorbike->AccumulationFactor());
	engineTorque += t * throttle;

	float power_Hp = engineTorque * Abs(av) / PhysicsConstants::HORSEPOWER;

	Vector3R torque = x * engineTorque;
	m_pMotorbike->ApplyTorque(derivative, m_pMotorbike->BodyIndex(), m_pMotorbike->EngineIndex(), torque);
}

}

