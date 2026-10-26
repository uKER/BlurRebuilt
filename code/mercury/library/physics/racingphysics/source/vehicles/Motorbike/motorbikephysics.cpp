#include "stdafx.h"
#include "racingphysics/vehicles/motorbike/motorbike.h"
//#include "racingphysics/collisions/collisionmesh.h"
#include "racingphysics/collisions/raycast.h"
#include "racingphysics/physics/surfaces.h"
//#include "racingphysics/physics/gametimer.h"
#include "racingphysics/physics/physicstimer.h"
#include "racingphysics/collisions/infiniteplane.h"
//#include "MGSTech/Vince/Vince.h"

// [Rog] I feel bad putting this in, but until there's a nice callback system this will do
//#include "Game\GameLogic\RaceState.h"

//using namespace Maths;

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize("", off) // DEBUG_RACING_PHYSICS
#endif

namespace RacingPhysics
{

Debug::Bool g_bikeRiderPresent("RacingPhysics.Bikes.RiderPresent", true);
//Debug::Float g_lowSpeedLean("RacingPhysics.Bikes.LowSpeedLean", 0.35f, Debug::Range(0.f, 1.f));
//Debug::Float g_slideLeanScale("RacingPhysics.Bikes.SlideLeanScale", 0.f, Debug::Range(0.f, 1.f));
Debug::Float g_kneeRollTolerance("RacingPhysics.Bikes.KneeRollTolerance", 0.00f, Debug::Range(0.f, 1.f));
//Debug::Float g_steeringRollTorqueOffset("RacingPhysics.Bikes.SteeringRollTorqueOffset", -0.32f, Debug::Range(-1.f, 1.f));
Debug::Float g_maxSteeringTorque("RacingPhysics.Bikes.MaxSteeringTorque", 2000.f, Debug::Range(0.f, 10000.f));
Debug::Float g_steeringPower("RacingPhysics.Bikes.SteeringPower", 1.f, Debug::Range(0.f, 10.f));
Debug::Bool g_noLean("RacingPhysics.Bikes.NoLeanOnOddNumberedBikes", false);

EXTERN_DEBUG_FLOAT( g_debugThrust );

///////////////////////////////////////////////////////////////////////

void Motorbike::Initialise(const VehicleInitialiseInfo& initialiseInfo)
{
	SetFilename(initialiseInfo.filename.Chars());

	bool changed;
	m_pSettings = MotorbikeDynamicsXt::LoadDynamicsXt(GetFilename(), changed);
	m_dynamicsChangedCounter = 0;
	RP_ASSERT(m_pSettings && "Bike dynamics not found");

	Vehicle::Initialise(&m_collisionBody, Vehicle::VEHICLE_TYPE_BIKE);
	ArticulatedBody::Initialise(5, 1);

	Point3R centre(0.f, 0.f, 0.f);
	Vector3R size(0.25f, 1.2f, Dynamics().geometry.wheelBase * 0.5f
							+ Dynamics().geometry.WheelRadius(0) + 0.05f);

	//m_collisionObject.InitialiseBike(&m_collisionBody.GetAxis(), centre, size,
	//									Point3R(0.f, 0.f, Dynamics().geometry.wheelBase * 0.5f),
	//									Point3R(0.f, 0.f, -Dynamics().geometry.wheelBase * 0.5f),
	//									Dynamics().geometry.WheelRadius(0));

	for (int i = 0; i < m_wheels.Size(); i++)
		Wheel(i).Initialise(i, this, &GetRigidBody(1 + i));

	m_engine.Initialise(this, &GetRotor(0));

	//m_collisionBody.Initialise(&m_collisionObject);

	SetMassVariables();

	m_footForce.Initialise();
	m_footFriction.Initialise();
	m_airFrictionForce.Initialise();
	m_steeringTorque.Initialise();

	m_controls.Clear();

	m_feedback.Initialise(this);

	m_steer = 0.f;
	m_steerReduced = 0.f;
	m_steerSmooth = 0.f;
	m_steerSmoothReduced = 0.f;
	m_rollFactor = 0.f;
	m_checkControlsPressed = false;
	m_riderPresent = true;
	m_fullBrakes = 0.f;
	m_rearBrake = 0.f;
	m_frontBrake = 0.f;
	m_leanX = 0.f;
	m_leanZ = 0.f;
	m_lowSpeedLean = 0.f;
	m_throttle = 0.f;
	m_reverse = false;
	m_reverseThrottle = 0.f;
	m_previousSteering = 0.f;

	m_leftCollisionTime = 0.f;
	m_rightCollisionTime = 0.f;

	m_frame.rollCorrection = 0.f;
	m_frame.bodyOnGround = false;
	m_frame.air = true;
	m_frame.smoothedGroundNormal = Constants<Normal3R>::Y();
	m_frame.sinRoll.Initialise();
	m_frame.maxSinRoll = 0.f;
	m_frame.dr.Initialise();
	m_frame.downforceFactor = 0.f;

	Axis a;
	a.SetIdentityMatrix();
	a.SetPosition(Constants<Point3R>::Origin());
	SetAxis(a);

	SetControlType(initialiseInfo.controlType);
	SetControllerPort(initialiseInfo.controllerPort);
	//SetCollisionsEnabled(initialiseInfo.vehicleToVehicleCollisions);
	SetAutoGears(initialiseInfo.autoGears);  

	//m_bodySurfaceType = (initialiseInfo.controlType == CONTROL_TYPE_AI) ? 
	//					Surfaces::SURFACE_TYPE_BIKE_BODY_AI : Surfaces::SURFACE_TYPE_BIKE_BODY;

	ClearStepVariables();

	m_networkState.networkStateValid = false;
	m_predictedNetworkState = m_networkState;
	m_networkState.axis.SetPosition(Constants<Point3R>::Origin());
}

bool Motorbike::Validate() const
{
	RP_ASSERT(Body().GetAxis().DebugTest());
	RP_ASSERT(Magnitude(Body().Velocity()) < 1000.f);
	RP_ASSERT(Magnitude(Body().AngularVelocity()) < 250.f);
	return true;
}

bool Motorbike::RiderPresent() const 
{
	return m_riderPresent && g_bikeRiderPresent;
}

void Motorbike::SetRiderPresent(bool b)
{
	m_riderPresent = b;
}

void Motorbike::Shutdown()
{
	//m_collisionObject.Shutdown();

	for (int wi = 0; wi < m_wheels.Size(); wi++)
		Wheel(wi).Shutdown();
}

Debug::Float g_lowSpeedIntegration1Bike("RacingPhysics.Bikes.Integration.EulerSpeed", 0.2f, Debug::Range(0.f, 100.f));
Debug::Float  g_lowSpeedIntegration2Bike("RacingPhysics.Bikes.Integration.MidpointSpeed", 20.f, Debug::Range(0.f, 100.f));

void Motorbike::GetIntegrationType(int& integrationType, int& nSteps)
{
	float v = Magnitude(GetVelocity());

	// change integration type at low speed
	if (integrationType == 2)
	{
		if (v < g_lowSpeedIntegration1Bike)
		{
			nSteps *= 4;
			integrationType = 0; // euler
		}
		else if (v < g_lowSpeedIntegration2Bike)
		{
			nSteps *= 2;
			integrationType = 1; // midpoint
		}
	}
}

void Motorbike::SetMassVariables()
{
	float bodyMass = Dynamics().mass.kerbMass * (1.f - Dynamics().mass.forkMassFraction) - 2.f * WheelMass();
	Body().SetMassVariables(bodyMass, 
							Vector3R(Dynamics().mass.MIScaleX, bodyMass * Dynamics().mass.MIScaleY, bodyMass * Dynamics().mass.MIScaleZ),
							Point3R(0.f, Dynamics().geometry.cofGOffsetY, Dynamics().geometry.cofGOffsetZ));
	
	float forkMass = Dynamics().mass.kerbMass * Dynamics().mass.forkMassFraction;
	float forkMI = forkMass * Dynamics().mass.forkMIScale;
	Fork().SetMassVariables(forkMass, Vector3R(forkMI, forkMI, forkMI), Point3R(0.f, 1.f, 0.f));
	
	float riderMass = Dynamics().rider.riderMass;
	Rider().SetMassVariables(riderMass, Vector3R(riderMass, riderMass, riderMass), Point3R(0.f, 0.f, 0.f));

	ArticulatedBody::CalculateMass();

	// collision body
	{
		Point3R cofG = Body().GetAxis().Local(ArticulatedBody::CofG());
		float totalMass = Mass();
		
		float s_xScale = 1.f;
		float s_yScale = 1.f;
		float s_zScale = 1.f;

		if (!RiderPresent())
			cofG = Point3R(0.f, 0.2f, 0.f);

		m_collisionBody.SetMassVariables(	totalMass, 
											Vector3R(s_xScale * Dynamics().mass.MIScaleX, 
													 s_yScale * Dynamics().mass.MIScaleY, 
													 s_zScale * Dynamics().mass.MIScaleZ), 
											cofG);
	}
}

float Motorbike::ForkOffset() const
{
	float a1 = Dynamics().geometry.casterAngle_Degrees * PhysicsConstants::DEGREES;
	float a2 = (90.f - Dynamics().geometry.casterAngle_Degrees) * PhysicsConstants::DEGREES;
	float offset = sin(a1) * (Dynamics().geometry.WheelRadius(0) - Dynamics().geometry.trail * tan(a2));

	//float offset = -(Dynamics().geometry.trail - Dynamics().geometry.wheelRadius[0] * tan(a));
	return offset;
}

Point3R Motorbike::RestingWheelPosition_Local(int i, float offsetY) const
{
	if (i == 0)
		return Point3R(0.f, offsetY, -ForkOffset());
	else
		return Point3R(0.f, offsetY, Dynamics().geometry.wheelBase * 0.5f);
}

Point3R Motorbike::RestingWheelPosition_Global(int i) const
{
	if (i == 0)
		return Fork().GetAxis().Global(Point3R(0.f, 0.f, -ForkOffset()));
	else
		return Body().GetAxis().Global(Point3R(0.f, 0.f, Dynamics().geometry.wheelBase * 0.5f));
}

Normal3R Motorbike::ForkAxis_Local() const
{
	float s = sin(Dynamics().geometry.casterAngle_Degrees * PhysicsConstants::DEGREES);
	float c = sqrt(1.f - s * s);

	return MakeNormal(Vector3R(0.f, c, s));
}

Point3R Motorbike::ForkPosition_Local() const
{
	Point3R p = Point3R(0.f, 0.f, -Dynamics().geometry.wheelBase * 0.5f);
	float s = sin(Dynamics().geometry.casterAngle_Degrees * PhysicsConstants::DEGREES);
	float c = sqrt(1.f - s * s);
	
	p += Vector3R(0.f, -s, c) * ForkOffset();

	return p;
}

Point3R Motorbike::RiderPosition_Local() const
{
	Point3R p = Point3R(0.f, Dynamics().rider.riderCofGOffsetY, Dynamics().rider.riderCofGOffsetZ);
	return p;
}

void Motorbike::SetAxis(const Axis& a, bool overwriteOldAxis)
{
	SetAxis(a, 0.f, 0.f, 0.f);
	CopyStateToCollisionBody();
	if (overwriteOldAxis)
		m_oldAxis = a;
}

void Motorbike::SetAxis(const Axis& a, float steeringAngle, float suspensionOffset0, float suspensionOffset1)
{
	RP_ASSERT(a.DebugTest());
	
	Axis forkAxis, frontWheelAxis, rearWheelAxis;
	CalculateAxes(a, steeringAngle, suspensionOffset0, suspensionOffset1, forkAxis, frontWheelAxis, rearWheelAxis);

	Body().SetAxis(a);
	Fork().SetAxis(forkAxis);
	Wheel(0).Body().SetAxis(frontWheelAxis);
	Wheel(1).Body().SetAxis(rearWheelAxis);

	Rider().SetPosition(a.Global(RiderPosition_Local()));
}

void Motorbike::CopyStateToCollisionBody()
{
	m_collisionBody.SetAxis(Body().GetAxis());
	m_collisionBody.SetVelocity(Body().PointVelocity(m_collisionBody.CofG()));
	m_collisionBody.SetAngularVelocity(Body().AngularVelocity());
	//CopyStateFromCollisionBody();
}

void Motorbike::CopyStateFromCollisionBody()
{
	float eav = Engine().AngularVelocity1D();
	float wav0 = Wheel(0).AngularVelocity1D();
	float wav1 = Wheel(1).AngularVelocity1D();
	float fav = (Fork().AngularVelocity() - Body().AngularVelocity()) | Fork().GetAxis().AxisY();

	float s0 = SuspensionDistance(0);
	float s1 = SuspensionDistance(1);
	float af = asin(Fork().GetAxis().AxisZ() | Body().GetAxis().AxisX());

	float vyw0 = (Wheel(0).Body().Velocity() - Body().PointVelocity(Wheel(0).Body().CofG())) | Fork().GetAxis().AxisY();
	float vyw1 = (Wheel(1).Body().Velocity() - Body().PointVelocity(Wheel(1).Body().CofG())) | Body().GetAxis().AxisY();

	SetAxis(m_collisionBody.GetAxis(), af, s0, s1);

	Vector3R vb = m_collisionBody.PointVelocity(Body().CofG());
	Vector3R vf = m_collisionBody.PointVelocity(Fork().CofG());
	Vector3R vw0 = m_collisionBody.PointVelocity(Wheel(0).Body().CofG()) + Fork().GetAxis().AxisY() * vyw0;
	Vector3R vw1 = m_collisionBody.PointVelocity(Wheel(1).Body().CofG()) + Body().GetAxis().AxisY() * vyw1;

	Body().SetVelocity(vb);
	Fork().SetVelocity(vf);
	Wheel(0).Body().SetVelocity(vw0);
	Wheel(1).Body().SetVelocity(vw1);

	Vector3R favn = m_collisionBody.AngularVelocity() + Fork().GetAxis().AxisY() * fav;
	Vector3R eavn = m_collisionBody.AngularVelocity() - Body().GetAxis().AxisX() * eav;
	Vector3R wav0n = m_collisionBody.AngularVelocity() - Wheel(0).Body().GetAxis().AxisX() * wav0 + Fork().GetAxis().AxisY() * fav;
	Vector3R wav1n = m_collisionBody.AngularVelocity() - Wheel(1).Body().GetAxis().AxisX() * wav1;

	Body().SetAngularVelocity(m_collisionBody.AngularVelocity());
	Fork().SetAngularVelocity(favn);
	Engine().Body().SetAngularVelocity(eavn);
	Wheel(0).Body().SetAngularVelocity(wav0n);
	Wheel(1).Body().SetAngularVelocity(wav1n);

	RP_ASSERT(Validate());
}

void Motorbike::CalculateAxes(const Axis& body, float steeringAngle, float suspensionOffset0, float suspensionOffset1,
						Axis& forkAxis, Axis& frontWheelAxis, Axis& rearWheelAxis) const
{
	// fork
	forkAxis.SetPosition(body.Global(ForkPosition_Local()));
	forkAxis.SetRotation(Body().GetAxis());
	forkAxis.Rotate(Body().GetAxis().AxisX() * Dynamics().geometry.casterAngle_Degrees * PhysicsConstants::DEGREES);
	forkAxis.Rotate(Fork().GetAxis().AxisY() * steeringAngle);
	
	// wheels
	frontWheelAxis.SetPosition(forkAxis.Global(RestingWheelPosition_Local(0, suspensionOffset0)));
	rearWheelAxis.SetPosition(body.Global(RestingWheelPosition_Local(1, suspensionOffset1)));

	frontWheelAxis.SetRotation(forkAxis);
	rearWheelAxis.SetRotation(body);
}

void Motorbike::SetPosition(const Point3R& p)
{
	Axis(a) = Body().GetAxis();
	a.SetPosition(p);
	SetAxis(a);
}

void Motorbike::Stop()
{
	Vehicle::Stop();
	ArticulatedBody::Stop();
	m_collisionBody.Stop();
}

void Motorbike::Reset() 
{
	Reset(Body().GetAxis().AxisZ(), Body().GetAxis().Position());
}

void Motorbike::Reset(const Normal3R& direction, const Point3R& position)
{
	if (g_bikeRiderPresent)
	{
		Axis a;
		a.SetPosition(position + Constants<Normal3R>::Y());
		a.LookAlong(direction, Constants<Normal3R>::Y());
		SetAxis(a);
		PutOnGround();
		Stop();

		m_frame.bodyOnGround = false;
	}
}

// Calculate the maximum amount of roll that the bike should need for cornering
// given its speed and grip.
float Motorbike::CalculateMaxSinRoll(float speed, float gripScale, float slideFactor)
{
	float lowSpeedTurnRadiusScale = Dynamics().steering.lowSpeedTurnRadiusScale;//Lerp(Dynamics().steering.lowSpeedTurnRadiusScale, 10, slideFactor);
	
	float f = Min(gripScale, speed * speed / (Dynamics().geometry.wheelBase * lowSpeedTurnRadiusScale * PhysicsConstants::GRAVITY));
	float rmax = f / sqrt(1.f + f * f);

	// correction for rider
	float offset = Dynamics().rider.riderMovement * Dynamics().rider.riderLeanX * Dynamics().rider.riderMass / (Dynamics().mass.kerbMass + Dynamics().rider.riderMass);
	float h = (Dynamics().geometry.cofGOffsetY + Dynamics().geometry.WheelRadius(0));
	float dr = offset / h;

	// correction for tyre width
	float t = Dynamics().geometry.TubeRadius(0);
	float dt = rmax * t / (h - t);

	float maxSinRoll = Max(rmax /*- dr*/ + dt, 0.f);

	return Clamped(maxSinRoll, 0.01f, 0.9f);
}

float Motorbike::CalculateDownforceFactor() const
{
	return Dynamics().mass.constantDownforce + Dynamics().mass.downforce * Abs(Speed_GlobalXZ()) / (100.f * PhysicsConstants::MPH);
}

void Motorbike::BeginFrame(float largestSpeed)
{
	RP_ASSERT(Validate());

	CopyStateToCollisionBody();
	Vehicle::BeginFrame(largestSpeed);
	NetworkJumpIfNeeded();

	// in case of network jump
	//UpdateCollisionCache(largestSpeed);

	m_oldAxis = Body().GetAxis();
}

void Motorbike::BeginIntegrate()
{
	ClearStepVariables();
	SetMassVariables();
	CopyStateToCollisionBody();

	m_footForce.Begin();
	m_footFriction.Begin();
	m_airFrictionForce.Begin();
	m_steeringTorque.Begin();

	m_frame.dr.Begin();
	m_frame.sinRoll.Begin();

	m_leftCollisionTime = Max(m_leftCollisionTime - ArticulatedBody::TimeStep(), 0.f);
	m_rightCollisionTime = Max(m_rightCollisionTime - ArticulatedBody::TimeStep(), 0.f);

	for (int wi = 0; wi < m_wheels.Size(); wi++)
		Wheel(wi).BeginIntegrate();

	Engine().BeginIntegrate();

	// steering input
	{
		// raw steering input
		//m_steer = m_controls.steer * Abs(m_controls.steer);
		m_steer = Abs(pow(m_controls.steer, g_steeringPower)) * Sign(m_controls.steer);

		//if (g_steerOverride != 0.f)
		//	m_steer = g_steerOverride;

		m_steer = Maths::Sin(m_steer * Maths::PiBy2);

		if (m_leftCollisionTime > 0.f) 
			m_steer = Clamped(m_steer, 0.f, 1.f);
		if (m_rightCollisionTime > 0.f) 
			m_steer = Clamped(m_steer, -1.f, 0.f);

		float gripScale = (Wheel(0).TransverseGripScale() + Wheel(1).TransverseGripScale()) * 0.5f;
		m_frame.maxSinRoll = CalculateMaxSinRoll(Magnitude(Body().Velocity()), gripScale, 0.f);
		m_frame.downforceFactor = CalculateDownforceFactor();

		// find maximum required roll for cornering
		// and reduce steer input once past this point
		{
			float tanRoll, sinPitch, sinYaw;
			Body().GetAxis().GetRollPitchYaw_Trig(tanRoll, sinPitch, sinYaw);
			float sinRoll = tanRoll / sqrt(1.f + tanRoll * tanRoll);

			float rmax = MaxSinRoll() * 0.9f;

			m_rollFactor = Clamped(rmax * 100.f, 0.f, 1.f);

			if (m_steer * sinRoll >= 0.f)
				if (rmax > 0.f)
					m_rollFactor = 1.f - pow(Min(Abs(sinRoll) / rmax, 1.f), Dynamics().steering.leanSteerReductionPower);
		}

		// reduced by roll factor
		m_steerReduced = m_steer * m_rollFactor;

		// get the smoothing factor
		float smoothTime = Dynamics().steering.steerSmoothTime;
		if (Controls().isWheel)
			smoothTime = 0.01f;

		float f = exp(-TimeStep() / smoothTime);

		// smoothed steering (for steering torque)
		m_steerSmooth = m_steerSmooth * f + m_steer * (1.f - f);
		RP_ASSERT(Maths::Finite(m_steerSmooth));
		m_steerSmoothReduced = m_steerSmooth * m_rollFactor;

		// smoothed roll rate for steering calculations
		//m_rollRateSmooth = m_rollRateSmooth * f - (Body().AngularVelocity() ^ Body().GetAxis().AxisX()).Y() * (1.f - f);
	}

	// throttle and braking input
	{
		float throttle, reverseThrottle;
		bool reverse;

		if ((Speed_GlobalXZ() < Dynamics().feet.maxSpeedForReverse)
			&& (m_controls.reverse || (AutoReverse() && m_controls.fullBrakes > 0.1f && m_controls.throttle < 0.1f)))
			reverse = true;
		else 
			reverse = false;

		{
			m_rearBrake = Clamped(m_rearBrake + (m_controls.rearBrake - 0.5f) * 2.f * TimeStep() /  Dynamics().brakes.rearBrakeSmoothTime, 0.f, 1.f);
		}

		{
			if (AutoGears() && m_controls.reverse && Speed_GlobalXZ() < Dynamics().feet.maxSpeedForReverse)
				throttle = 0.f;
			else if (AutoReverse() && reverse)
				throttle = 0.f;
			else
				throttle = m_controls.throttle;

			if (AutoGears() && m_controls.reverse && Speed_GlobalXZ() < Dynamics().feet.maxSpeedForReverse)
				reverseThrottle = 1.f;
			else if (AutoReverse() && reverse)
				reverseThrottle = m_controls.fullBrakes;
			else
				reverseThrottle = 0.f;

			reverseThrottle *= 1.f - m_rearBrake;
			throttle *= 1.f - m_rearBrake * Engine().ClutchEngaged();

			float f = exp(-TimeStep() / Dynamics().engine.throttleSmoothTime);

			// smoothed throttle
			m_reverse = reverse;
			m_throttle = m_throttle * f + throttle * (1.f - f);
			m_reverseThrottle = m_reverseThrottle * f + reverseThrottle * (1.f - f);
		}
			
		{
			float brakes;

			if (AutoReverse() && reverse)
				brakes = m_controls.throttle;
			else
				brakes = m_controls.fullBrakes;

			// brake to stop the bike rolling backwards
			if (reverseThrottle == 0.f && Speed_GlobalXZ() < 0.f && GetControlType() != CONTROL_TYPE_NETWORK)
				m_rearBrake = Clamped(1.f - throttle * 10.f, m_rearBrake, 1.f);

			//brakes = AddSmooth(brakes, m_controls.rearBrake * g_fullBrakeSlideFraction);

			// smoothed brakes
			//float f = exp(-TimeStep() / Dynamics().brakes.fullBrakesSmoothTime);
			//m_fullBrakes = m_fullBrakes * f + brakes * (1.f - f);

			m_fullBrakes = Clamped(m_fullBrakes + (brakes - m_fullBrakes) * TimeStep() /  Dynamics().brakes.fullBrakesSmoothTime, 0.f, 1.f);
		}

		// low speed lean
		{
			float lowSpeedLean = m_steerSmooth * Throttle() * (1.f - AddSmooth(FullBrakes(), AddSmooth(FrontBrake(), RearBrake())));
			float f = exp(-TimeStep() / 0.25f);
			m_lowSpeedLean = m_lowSpeedLean * f + lowSpeedLean * (1.f - f);

			//if (g_targetSpeed_MPH > 0.f)
			//	m_throttle = Clamped((g_targetSpeed_MPH * PhysicsConstants::MPH - Speed_GlobalXZ()) * 0.1f + 0.6f, 0.f, 1.f);
		}
	}

	// lean
	{
		if (g_noLean && VehicleIndex() % 2 == 1)
			m_controls.lean = 0.f;

		// lean backwards for better braking
		float brakeLean = FullBrakes() * (1.f - Throttle()) * Clamped(Speed_LocalZ() / (30.f * PhysicsConstants::MPH), 0.f, 1.f);

		m_leanZ = Maths::Lerp(-brakeLean, m_controls.lean, Abs(m_controls.lean));//ClampedAbs(m_controls.lean - brakeLean, 1.f);
		m_leanX = m_steerSmoothReduced * (1.f - brakeLean);
	}

	m_dynamicsChangedCounter = Max(m_dynamicsChangedCounter - 1, 0);

	//if (Abs(Body().AngularVelocity().Y()) > g_angularVelocityTest)
	//	int breakhere = 0;

	if (!RiderPresent())
	{
		m_throttle = 0.f;
	}
}

void Motorbike::EndFrame(const Maths::C_TimeS PCREF time)
{
	RP_ASSERT(Validate());

	Vehicle::EndFrame(time);

	/*{
		static Vehicle::CompressedNetworkState cns;

		if (0)
		{
			GetNetworkState(&cns);
			GetNetworkState(m_networkState);
		}

		if (0)
		{
			ProcessNetworkState(cns);
			NetworkJump(true);
			extern Debug::Bool g_updatePhysics;
			g_updatePhysics = false;
		}
	}*/

	RP_ASSERT(Validate());
}

// after update state
void Motorbike::EndIntegrate()
{
	RP_ASSERT(Validate());

	m_footForce.End();
	m_footFriction.End();
	m_airFrictionForce.End();
	m_steeringTorque.End();

	m_frame.dr.End();
	m_frame.sinRoll.End();

	Engine().EndIntegrate();

	for (int wi = 0; wi < m_wheels.Size(); wi++)
		Wheel(wi).EndIntegrate();

	m_checkControlsPressed = false;

	m_frame.air = !(Wheel(0).OnGround() || Wheel(1).OnGround());

	if (Wheel(0).OnGround() || Wheel(1).OnGround())
	{
		Vector3R averageNormal = Constants<Vector3R>::Null();
		if (Wheel(0).OnGround()) averageNormal += Wheel(0).GroundNormal();
		if (Wheel(1).OnGround()) averageNormal += Wheel(1).GroundNormal();

		float f = 0.9f;
		m_frame.smoothedGroundNormal = Normalised(Normalised(averageNormal) * (1.f - f) + m_frame.smoothedGroundNormal * f);
	}

	CopyStateToCollisionBody();

	RP_ASSERT(Validate());
}

void Motorbike::Integrate(float time) 
{
	//PixEvent pix(0xffff0000, "Motorbike::Integrate");
	ArticulatedBody::Integrate(time);
}

void Motorbike::EvaluateTyreForces(Derivative& derivative)
{
	for (int wi = 0; wi < m_wheels.Size(); wi++)
	{
		if (Wheel(wi).OnGround_Step())
		{
			Vector3R force, torque;
			Wheel(wi).CalculateDynamicFrictionForce(force, torque);

			Wheel(wi).m_frame.groundFriction.Accumulate(force * AccumulationFactor());
			Wheel(wi).m_frame.groundTorque.Accumulate(torque * AccumulationFactor());

			ApplyExternalForce(derivative, WheelBodyIndex(wi), 
								force, Wheel(wi).GroundContactPosition_Step());
			ApplyExternalTorque(derivative, WheelBodyIndex(wi), torque);
		}
	}
}

// Equivalent mass including wheel inertia
float Motorbike::MassCorrectionFactor() const
{
	float m = Mass();
	return (m + WheelMass() * (Dynamics().mass.wheelMIScaleFront + Dynamics().mass.wheelMIScaleRear)) / m;
}

void Motorbike::EvaluateControlForces(Derivative& derivative)
{
	Point3R cofg_Global = CofG();
	Point3R cofg_Local = Body().GetAxis().Local(CofG());//CorrectedCofGLocal();

	float footForceSize = Magnitude(m_footForce.GetValue());

	// Max torque such that the bike will not flip over.
	float maxTotalFlipBrakingTorque = (Mass() * PhysicsConstants::GRAVITY * (1.f + DownforceFactor_Step()) - footForceSize)
									* Dynamics().geometry.WheelRadius(0)
									* (Dynamics().geometry.wheelBase * 0.5f + cofg_Local.Z() - Dynamics().suspension.maxSuspensionMovement) 
									/ (cofg_Local.Y() + Dynamics().geometry.WheelRadius(0) + Dynamics().suspension.maxSuspensionMovement);

	float maxFlipEngineTorque = (Mass() * PhysicsConstants::GRAVITY * (1.f + DownforceFactor_Step()) - footForceSize)
									* Dynamics().geometry.WheelRadius(0)
									* (Dynamics().geometry.wheelBase * 0.5f - cofg_Local.Z()) 
									/ (cofg_Local.Y() + Dynamics().geometry.WheelRadius(0) + Dynamics().suspension.maxSuspensionMovement);

	maxTotalFlipBrakingTorque = Max(maxTotalFlipBrakingTorque, 0.f);
	maxFlipEngineTorque = Max(maxFlipEngineTorque, 0.f);

	float rollFraction = Dynamics().steering.rollFraction;

	if (Wheel(1).OnGround_Step())
	{
		float spin = Abs(Wheel(1).WheelSpin_Step() / Wheel(1).LongitudinalTyreSlip());
		float f = Clamped(Abs(spin) - 0.5f, 0.f, 1.f);
		f *= (1.f - FullBrakes());

		rollFraction *= Maths::Lerp(1.f, Dynamics().steering.slidingRoll, f);
	}

	// limit engine torque
	float maxEngineTorque = 10000.f;

	FloatVR tractionControlBraking = 0.f;

	{
		// traction control
		if (Dynamics().engine.tractionControl 
			&& Wheel(0).OnGround_Step()
			&& Wheel(1).OnGround_Step()
			&& Engine().Gear() > 0)
		{
			Vector3R newAngularMomentum = Wheel(1).Body().AngularMomentum() + derivative.parts[WheelBodyIndex(1)].torque * TimeStep();
			Vector3R newAngularVelocity = Wheel(1).Body().InverseMIGlobal() * newAngularMomentum;
			float w = newAngularVelocity | Wheel(1).Body().GetAxis().AxisX();
			float wRequired = Wheel(1).TractionControlAV_Step();
			maxEngineTorque = -(wRequired - w) * Wheel(1).Inertia() / TimeStep();

			if (maxEngineTorque < 0.f)
				tractionControlBraking = -maxEngineTorque;
		}

		if (Dynamics().engine.antiFlip
			|| m_controls.lean >= 0.f) // keep front wheel on ground
		{
			maxEngineTorque = Min(maxFlipEngineTorque, maxEngineTorque);

			if (m_controls.lean >= 0.f && !Wheel(0).OnGround_Step())
				maxEngineTorque = 0.f;
		}
		else if (Wheel(1).OnGround_Step() && !Wheel(0).OnGround_Step()) // allow wheelie
		{
			float d = -Body().GetAxis().AxisZ().Y();
			float v = Body().AngularVelocity() | Wheel(1).Body().GetAxis().AxisX();

			float liftRatio = Dynamics().engine.liftRatio;
			liftRatio *= Clamped(-m_controls.lean * Throttle(), 0.f, 1.f);
			liftRatio = Clamped(liftRatio, 0.15f, 1.f);
			//float rpmFraction = Engine().RPM() * 0.5f / Dynamics().engine.limiterRPM;

			float f = (d - liftRatio + v * Dynamics().engine.wheelieDamping);
			maxEngineTorque = maxFlipEngineTorque * (1.f - d) * Clamped(1.f - f, 0.f, 2.f);
		}

		maxEngineTorque = Max(maxEngineTorque, 0.f);
	}

	float engineTorque = 0.f;
	float clutchPowerCut = 0.f;

	for (int wi = 0; wi < m_wheels.Size(); wi++)
	{
		int bi = (wi == 0) ? ForkIndex() : BodyIndex();

		if (Wheel(wi).Driven() && Engine().Gear() != 0)
		{
			Normal3R ex = Body().GetAxis().AxisX();
			Vector3R ew = Engine().Body().AngularVelocity();

			float eav = (ex | ew);

			Normal3R wx = Wheel(wi).Body().GetAxis().AxisX();
			Vector3R ww = Wheel(wi).Body().AngularVelocity() * Wheel(wi).RotationScale();

			float wav = (wx | ww) * Engine().GearRatio();

			float f = (15000.f / Dynamics().engine.limiterRPM) * (Dynamics().engine.engineTorque_Rpm.yScale / 100.f);
			float x = (eav - wav) * Dynamics().engine.clutchSpring * f;
			float t = x * Engine().ClutchEngaged() * Engine().ClutchEngaged();

			t = ClampedAbs(t, Abs(eav * Dynamics().engine.engineInertia / TimeStep()));
			t = Clamped(t, -maxEngineTorque / Engine().GearRatio(), maxTotalFlipBrakingTorque / Engine().GearRatio());

			// slipper clutch
			if (Wheel(wi).OnGround_Step())
			{
				Normal3R x = Wheel(wi).Body().GetAxis().AxisX();
						
				Vector3R newAngularMomentum = Wheel(wi).Body().AngularMomentum() + derivative.parts[WheelBodyIndex(wi)].torque * TimeStep();
				Vector3R newAngularVelocity = Wheel(wi).Body().InverseMIGlobal() * newAngularMomentum;
				float w = newAngularVelocity | x;

				float wRequired = Wheel(wi).BrakingAV_Step();
				float maxTorque = (wRequired - w) * Wheel(wi).Inertia() / TimeStep();

				t = Min(t, Max(maxTorque, 0.f));
			}

			// stop engine racing ahead of clutch
			{
				float wav = (wx | ww) * Engine().GearRatio(Engine().NextGear());
				float x = -(eav - wav) / (Dynamics().engine.limiterRPM * PhysicsConstants::RPM);
				float xmax = 0.1f;
				clutchPowerCut = Clamped(2.f * (x - xmax) / xmax, 0.f, 1.f);

				if (clutchPowerCut > 0.f)
					int breakhere = 0;
			}

			Vector3R torque = ex * t;
			engineTorque += t * Engine().GearRatio();

			ApplyTorque(derivative, WheelBodyIndex(wi), BodyIndex(), torque * Engine().GearRatio());
			ApplyTorque(derivative, BodyIndex(), EngineIndex(), torque);
		}
	}

	Engine().EvaluateControlForces(derivative, maxEngineTorque / Abs(Engine().GearRatio()), engineTorque, clutchPowerCut);

	if (Wheel(1).OnGround_Step())
	{
		float maxTorque = (Wheel(1).GroundForce() | Wheel(1).GroundNormal_Step()) * Wheel(1).Radius();
		maxTotalFlipBrakingTorque -= Clamped(engineTorque, 0.f, maxTorque);
		maxTotalFlipBrakingTorque = Max(maxTotalFlipBrakingTorque, 0.f);
	}

	if (RiderPresent())
	{
		for (int wi = 0; wi < m_wheels.Size(); wi++)
		{
			int bi = (wi == 0) ? ForkIndex() : BodyIndex();

			// braking
			{
				float brake = FullBrakes();

				if (wi == 1) 
				{
					brake *= 1.f - Throttle();
					brake = AddSmooth(brake, RearBrake());
					//brake = AddSmooth(brake, tractionControlBraking);
				}
				else
				{
					brake = AddSmooth(brake, FrontBrake());
				}

				// no rear braking in corner
				//if (wi == 1) brake *= 1.f - Abs(m_steerSmooth) * (1.f - Dynamics().brakes.rearBrakingInCorner) * (1.f - RearBrake());// * (1.f - tractionControlBraking);

				if (brake > 0.001f || tractionControlBraking > 0.f)
				{
					// No braking if wheel leaves the ground
					if (Wheel(wi).OnGround_Step() && Magnitude2(Wheel(wi).GroundForce()) > 0.f)
					{
						float torqueScale = (Wheel(wi).GroundForce() | Wheel(wi).GroundNormal_Step()) * Wheel(wi).BackwardsGripScale() * Wheel(wi).Radius();
						torqueScale = Max(torqueScale, 0.f);
						float maxGripBrakingTorque = torqueScale;// * 2.f;// * gripFraction;
						float maxBrakingTorque = maxGripBrakingTorque;

						Normal3R x = Wheel(wi).Body().GetAxis().AxisX();
						
						Vector3R newAngularMomentum = Wheel(wi).Body().AngularMomentum() + derivative.parts[WheelBodyIndex(wi)].torque * TimeStep();
						Vector3R newAngularVelocity = Wheel(wi).Body().InverseMIGlobal() * newAngularMomentum;
						FloatVR w = newAngularVelocity | x;

						// anti-lock
						{
							float wRequired = Wheel(wi).BrakingAV_Step();
							float maxTorque = (wRequired - w) * Wheel(wi).Inertia() / TimeStep();

							maxBrakingTorque = Min(maxBrakingTorque, Max(maxTorque, 0.f));
						}

						float s_maxendoLean = 0.2f;

						// anti-flip
						if (wi == 0 && w < 0.f)
						{
							if ((FullBrakes() > 0.1f || FrontBrake() > 0.1f) &&
								(Dynamics().brakes.antiFlip 
								|| Abs(SinRoll_Step()) > s_maxendoLean 
								|| m_controls.lean < 0.25f)
								) // keep rear wheel on ground
							{
								float f = Magnitude(Wheel(wi).GroundForce()) / (Magnitude(Wheel(0).GroundForce()) + Magnitude(Wheel(1).GroundForce()));
						
								float maxFlipBrakingTorque = maxTotalFlipBrakingTorque * Clamped((1.f - f) * 10.f, 0.f, 1.f);
								
								float s_brakeDamping = 2.f;

								float v = Body().AngularVelocity() | Wheel(1).Body().GetAxis().AxisX();

								maxFlipBrakingTorque *= Clamped(1.f + v * s_brakeDamping, 0.f, 2.f);
								
								maxBrakingTorque = Min(maxBrakingTorque, maxFlipBrakingTorque);

								if (!Wheel(1).OnGround_Step())
									maxBrakingTorque = 0.f;
							}
							else if (Wheel(0).OnGround_Step()) // let rear wheel go off ground slightly
							{
								float d = (Wheel(0).GroundNormal_Step() | Body().GetAxis().AxisZ());
								float v = Body().AngularVelocity() | Wheel(1).Body().GetAxis().AxisX();

								float liftRatio = Dynamics().brakes.liftRatio;
								liftRatio *= Clamped(m_controls.lean * FullBrakes(), 0.f, 1.f);

								float f = d / liftRatio - v * Dynamics().brakes.endoDamping;

								float maxFlipBrakingTorque = maxTotalFlipBrakingTorque * 2.f * Clamped(1.f - f, 0.f, 1.f);
								maxBrakingTorque = Min(maxGripBrakingTorque, maxFlipBrakingTorque);
							}
						}

						RP_ASSERT(maxBrakingTorque >= 0.f);

						FloatVR t = (maxBrakingTorque * brake * Dynamics().brakes.BrakeForce(wi));

						if (wi == 1)
							t = Max(tractionControlBraking, t);
			
						t = ClampedAbs(-w / TimeStep(), t / Wheel(wi).Inertia()) * Wheel(wi).Inertia();

						ApplyTorque(derivative, WheelBodyIndex(wi), bi, t * x);

						{
							Vector3R newAngularMomentum = Wheel(wi).Body().AngularMomentum() + derivative.parts[WheelBodyIndex(wi)].torque * TimeStep();
							Vector3R newAngularVelocity = Wheel(wi).Body().InverseMIGlobal() * newAngularMomentum;
							float nw = newAngularVelocity | x;
							int breakhere = 0;
						}
					}
				}
			}
		}

		// steering
		if (Wheel(0).OnGround_Step() && Wheel(1).OnGround_Step())
		{
			// speed dependent steer torque
			float t = Dynamics().steering.steerTorque_Mph.Evaluate(Speed_LocalZ() / PhysicsConstants::MPH);

			t *= 2.f;

			{
				float f = Magnitude(Wheel(0).GroundForce()) / (0.5f * Mass() * (1.f + DownforceFactor_Step()) * PhysicsConstants::GRAVITY);

				t *= Clamped(f, 0.f, 1.f);
			}

			// change torque with roll angle
			{
				float f = Abs(SinRoll_Step()) / MaxSinRoll_Step() * Dynamics().steering.rollFraction;
				f = Clamped(f, 0.f, 1.f);
				float rollTorqueScale = 1.f * (1.f - f) + Dynamics().steering.maxRollTorqueFraction * f;
				t *= Clamped(rollTorqueScale, 0.f, 1.f);
			}

			if (Abs(SinRoll_Step()) > MaxSinRoll_Step() * rollFraction)
				int breakhere = 0;

			// roll feedback
			if (Speed_GlobalXZ() > 0.1f)
			{
				float r = m_steerSmooth * MaxSinRoll_Step() * rollFraction - SinRoll_Step();
				float dr = RollRate_Step() + m_steerSmoothReduced * Dynamics().steering.rollFeedbackRate;

				float f = 0.f;

				{
					float rollTorque = 0.f;
					
					if (Wheel(1).OnGround_Step())
					{
						float spin = Abs(Wheel(1).WheelSpin_Step() / Wheel(1).LongitudinalTyreSlip());
						float slip = Abs(Wheel(1).SlipAngle_Step() / Wheel(1).TransverseTyreSlip());
						rollTorque = Clamped(Abs(spin) - 0.5f, 0.f, 1.f);
					}

					//float speedFactor = 15.f / (Abs(Speed_GlobalXZ()) + 15.f);
					//static float s_rollOffset = 0.05f;

					float dr = -RollRate_Step();
					float r = SinRoll_Step();
					float max = MaxSinRoll_Step() * rollFraction;
					float k = r + dr * Dynamics().feet.kneeForceDamping;
					float s = Sign(k);
					k = -s * Max(Abs(k) - max, 0.f) ;
					k *= Abs((r * r) / (max * max + 0.01f));

					//k -= SinRoll_Step() * Abs(SinRoll_Step()) * s_rollTorque;

					k *= rollTorque;
					k *= Dynamics().steering.rollTorque;
					//k *= speedFactor;
					k *= (1.f - FullBrakes());
					f += k;
				}

				{
					dr = ClampedAbs(dr, 4.f);
					r = ClampedAbs(r, 0.5f);

					f += dr * Dynamics().steering.feedbackTorqueDamping;
					f += r * Dynamics().steering.feedbackTorque;
				}

				{
					// Steering torque based on slip affects:
					// 1. High speed, leaning back up.
					// 2. Transition from braking to accelerating.
					float maxSlip = Wheel(0).TransverseTyreSlip();
					float slip = Wheel(0).SlipAngleWithCamber_Step();
					
					if (Abs(slip) > maxSlip)
					{
						float s_slipSteeringTorque = 2.f;
						float scale = (slip - maxSlip * Sign(slip)) / maxSlip;
						f = scale * s_slipSteeringTorque;
					}
					//else
					//{
					//	f = Clamped(f, -1.f, 1.f);
					//	f = f + slip / maxSlip;
					//}
				}

				f *= t;

				if (Abs(f) > g_maxSteeringTorque)
					int breakhere = 0;

				f = ClampedAbs(f, (float) g_maxSteeringTorque);

				Vector3R torque = Fork().GetAxis().AxisY() * f;
				ApplyTorque(derivative, BodyIndex(), ForkIndex(), torque);

				m_steeringTorque.Accumulate(torque * AccumulationFactor());
			}
		}
		//else if (Wheel(0).OnGround_Step())
		//{
		//	static float s_endoTorque = 100.f;
		//	Vector3R torque = Fork().GetAxis().AxisY() * m_steer * s_endoTorque;
		//	ApplyTorque(derivative, BodyIndex(), ForkIndex(), torque);
		//}

		// steer spring
		//if (Wheel(1).OnGround_Step())
		{
			//float frontFactor = Magnitude(Wheel(0).GroundForce()) / (Magnitude(Wheel(0).GroundForce()) + Magnitude(Wheel(1).GroundForce()));
				
			float s_mph = Abs(Speed_LocalZ()) / PhysicsConstants::MPH;
			//float steer = m_steerSmooth * 0.8f * (1.f - Min(s_mph / Dynamics().steering.steerSpringTorque_Mph.x1, 1.f));
			float steer = Maths::Lerp(m_steerSmooth * 0.8f, 0.1f, 
								Clamped((s_mph - Dynamics().steering.steerSpringTorque_Mph.x0) 
											/ (Dynamics().steering.steerSpringTorque_Mph.x1 - Dynamics().steering.steerSpringTorque_Mph.x0), 
												0.f, 1.f));

			if (!Wheel(0).OnGround_Step() || !Wheel(1).OnGround_Step())
			{
				s_mph = 0.f;
				steer = 0.f;
			}

			float a = (Fork().GetAxis().AxisZ() | Body().GetAxis().AxisX());
			float aAdjusted = ClampedAbs(a - steer, 0.25f);
			float v = (Fork().AngularVelocity() - Body().AngularVelocity()) | Fork().GetAxis().AxisY();
			float steerLimit = Wheel(0).TransverseTyreSlip() * Dynamics().steering.steerSpringLimit;
			float aReduced = 0.f; if (Abs(a) > steerLimit) aReduced = a - steerLimit * Sign(a);
			aReduced = ClampedAbs(aReduced, 0.25f);
			float steerSpring1 = Dynamics().steering.steerSpringTorque_Mph.Evaluate(s_mph, true);

			float t = aAdjusted * steerSpring1 + aReduced * Dynamics().steering.largeAngleSteerSpring;
			float damping = (Speed_LocalZ() < 0.f ? Dynamics().steering.steerDampingReverse : Dynamics().steering.steerDamping);
			t += v * damping;

			Vector3R torque = Fork().GetAxis().AxisY() * t;
			ApplyTorque(derivative, BodyIndex(), ForkIndex(), torque);

			m_steeringTorque.Accumulate(torque * AccumulationFactor());
		}

		// lean
		if (Wheel(1).OnGround_Step())
		{
			float f = Dynamics().rider.riderMass * PhysicsConstants::GRAVITY;
			float leanX = LeanX(); // * (1.f - RearBrake() * 2.f)

			/*if (Wheel(0).OnGround_Step() && !Wheel(1).OnGround_Step())
			{
				float x = (Rider().Position() - Body().Position()) | Body().GetAxis().AxisX();
				float dx = (Rider().Velocity() - Body().Velocity()) | Body().GetAxis().AxisX();
				float r = SinRoll_Step();
				float dr = RollRate_Step();
				float max = Dynamics().rider.riderLeanX * Dynamics().rider.riderMovement;

				static float s_autoLeanScale = 3.f;
				static float s_autoLeanDamping = -0.5f;

				float target = (r) * s_autoLeanScale + (dr + dx) * s_autoLeanDamping;
				leanX = (-target) / max;

				leanX = Clamped(leanX, -1.f, 1.f);
			}*/

			Vector3R force = Body().GetAxis().AxisX() * f * Dynamics().rider.riderLeanX * leanX
							+ Body().GetAxis().AxisZ() * f * Dynamics().rider.riderLeanZ * LeanZ();

			ApplyForce(derivative, BodyIndex(), RiderIndex(), force, Rider().Position());
		}

		// Feet
		if (!Air() 
			&& Body().GetAxis().AxisY().Y() > 0.f
			&& !BodyOnGround_Step())
			//&& -Body().GetAxis().AxisZ().Y() < 0.25f)
		{
			float footForce = 0.f;
			float kneeForce = 0.f;
			float maxSpeed = Dynamics().feet.footForceSpeed_Mph;
			float speed = Abs(Speed_GlobalXZ());//Magnitude(Velocity());
			float speedFactor = Max(1.f - speed / (maxSpeed * PhysicsConstants::MPH), 0.f);
			float brakes = FullBrakes();//AddSmooth(AddSmooth(FullBrakes(), FrontBrake()), RearBrake());
			float leanSpeedFactor = Max(1.f - speed / (30.f * PhysicsConstants::MPH), 0.f);

			bool wheelie = !Wheel(0).OnGround_Step();
			bool endo = !Wheel(1).OnGround_Step();

			if (!wheelie && !endo)
			{
				// stand upright (low speed)
				{
					float r = SinRoll_Step() - m_steerSmooth * MaxSinRoll_Step() * Dynamics().steering.rollFraction
										- ClampedAbs(m_lowSpeedLean, 1.f) * Dynamics().steering.lowSpeedLean * leanSpeedFactor;

					float dr = ClampedAbs(RollRate_Step() + m_steerSmoothReduced, 2.f);
					r = ClampedAbs(r, 1.f);

					//dr *= Clamped(1.f - Abs(Body().AngularVelocity().Y() * 0.5f), 0.f, 1.f);
		
					float angleFactor = Clamped((1.f - Abs(SinRoll_Step())) * 1.5f, 0.f, 1.f);
					footForce = speedFactor * angleFactor * (r * Dynamics().feet.footForceScale 
										- dr * Dynamics().feet.footForceDamping);
				}

				// low-siding (knee down)
				{
					float dr = -RollRate_Step();
					float r = SinRoll_Step();
					float maxRoll = MaxSinRoll_Step() * rollFraction;
					float rollReduction = 1.f - Min(Abs(SinRoll_Step() - m_steerSmooth * maxRoll), 1.f) * 0.2f;
					float target = maxRoll * rollReduction;
					float k = r + dr * Dynamics().feet.kneeForceDamping;
					float s = Sign(k);
					k = s * Max(Abs(k) - target, 0.f) * Dynamics().feet.kneeForceScale;
					k *= Abs((r * r) / (target * target + 0.01f));
					kneeForce = k * (1.f - speedFactor);
				}
			}
			/*else if (wheelie)
			{
				float r = SinRoll_Step() - m_steerSmooth * MaxSinRoll_Step() * Dynamics().steering.rollFraction * 0.5f;

				float dr = ClampedAbs(RollRate_Step() + m_steerSmoothReduced, 2.f);
				r = ClampedAbs(r, 1.f);

				footForce = r * Dynamics().feet.footForceScale 
										- dr * Dynamics().feet.footForceDamping;

				footForce *= -Body().GetAxis().AxisZ().Y();
				footForce = ClampedAbs(footForce, 0.5f);
			}*/

			//footForce *= 1.f - RearBrake();

			// force = weight * clamped factor
			float footForceSize = Mass() * (1.f + DownforceFactor_Step()) * PhysicsConstants::GRAVITY * ClampedAbs(footForce + kneeForce, 0.75f);

			if (Abs(footForceSize) > 0.01f)
			{
				Normal3R groundNormal = Constants<Normal3R>::Y();
				Point3R footPosition;

				/*if (wheelie)
				{
					footPosition = Wheel(1).Position() - Body().GetAxis().AxisX() * Sign(footForceSize) * Dynamics().feet.footForceOffsetX;
				}
				else*/
				{
					Point3R p = Body().Position()
									+ Body().GetAxis().AxisY() * 0.5f
									+ Body().GetAxis().AxisZ() * Dynamics().feet.footForceOffsetZ 
									- Body().GetAxis().AxisX() * Sign(footForceSize) * Dynamics().feet.footForceOffsetX;
					
					Point3R groundPosition = Body().Position() - Body().GetAxis().AxisY() * Wheel(0).Radius();
					footPosition = p - groundNormal * ((p - groundPosition) | groundNormal);
				}
				
				m_footPosition = footPosition; // debug only

				Vector3R vForce = groundNormal * Abs(footForceSize);
				m_footForce.Accumulate(vForce * AccumulationFactor());

				// friction
				/*{
					Vector3R x = Normalised(groundNormal ^ Body().GetAxis().AxisZ());
					Vector3R pv = Body().PointVelocity(footPosition);
					pv -= x * Dynamics().feet.footForceSteer * m_steerSmooth * Throttle() * (1.f - brakes);
					float mpv = Magnitude(pv);

					if (mpv > 0.001f)
					{
						Vector3R direction = -MakeNormal(pv / mpv);
						Vector3R friction = direction
											* Min(mpv / 10.f, 1.f)
											* Abs(m_footForceSize)
											* Dynamics().feet.footFriction;
						m_footFriction.Accumulate(friction * AccumulationFactor());
					}
				}*/

				ApplyExternalForce(derivative, BodyIndex(), vForce, footPosition);

				{
					Vector3R v = vForce * AccumulationFactor();// * 0.5f;
					//Wheel(0).m_frame.groundNormalForce.Accumulate(v);
					Wheel(0).m_frame.groundNormalForce.Accumulate(v);
				}
			}
		}

		if (Reverse())
		{
			float s = -Speed_LocalZ();
			float f = Clamped(4.0f * PhysicsConstants::MPH - s, -1.f, 1.f);
			ApplyExternalForce(derivative, BodyIndex(), Body().GetAxis().AxisZ() * ReverseThrottle() * Mass() * 4.f * f, Body().CofG());
		}
	}
}

float Motorbike::SuspensionSpeed(int wheelIndex) const
{
	int bi = (wheelIndex == 0) ? ForkIndex() : BodyIndex();

	Normal3R n = GetRigidBody(bi).GetAxis().AxisY();
	Vector3R v1 = Wheel(wheelIndex).Body().Velocity();
	Vector3R v2 = GetRigidBody(bi).PointVelocity(Wheel(wheelIndex).Body().Position());

	float rv = (v1 - v2) | n;

	return rv;
}

float Motorbike::SuspensionDistance(int wheelIndex) const
{
	int bi = (wheelIndex == 0) ? ForkIndex() : BodyIndex();
	Normal3R n = GetRigidBody(bi).GetAxis().AxisY();
	return (Wheel(wheelIndex).Body().Position() - RestingWheelPosition_Global(wheelIndex)) | n;
}

void Motorbike::UpdateSuspensionDistance(int i) 
{
	int bi = (i == 0) ? ForkIndex() : BodyIndex();
	Normal3R n = GetRigidBody(bi).GetAxis().AxisY();
	Wheel(i).SetSuspensionDistance((Wheel(i).Body().Position() - RestingWheelPosition_Global(i)) | n);
}

void Motorbike::EvaluateSpringForces(Derivative& derivative)
{
	for (int i = 0; i < m_wheels.Size(); i++)
	{
		Wheel(i).PutWheelOnGround();

		int bi = (i == 0) ? ForkIndex() : BodyIndex();

		Normal3R n = GetRigidBody(bi).GetAxis().AxisY();

		float mass;
		float staticLoad;

		if  (!Wheel(i).OnGround_Step() || !RiderPresent())
		{
			mass = WheelMass();
			staticLoad = mass * PhysicsConstants::GRAVITY;
		}
		else 
		{
			float f = Abs(Wheel(i).CamberAngle_Step());
			mass = SprungMass() * 0.5f;
			staticLoad = SprungMass() * (1.f + DownforceFactor_Step()) * 0.5f * PhysicsConstants::GRAVITY;
			//* (1.f - f) + Dynamics().wheelMass * f;
		}
		
		float scale = (mass * PhysicsConstants::GRAVITY / Dynamics().suspension.maxSuspensionMovement);
		float springConstant = scale * Dynamics().suspension.SpringConstant(i);
		float damping = Maths::Sqrt(springConstant * mass) * Dynamics().suspension.SpringDamping(i);
		float offset = -(staticLoad) / springConstant;

		float d = ((Wheel(i).Body().Position() - RestingWheelPosition_Global(i)) | n) - offset;
		float rv = (Wheel(i).Body().Velocity() - GetRigidBody(bi).PointVelocity(Wheel(i).Body().Position())) | n;

		Wheel(i).SetSuspensionDistance(d + offset);

		d = ClampedAbs(d, 0.5f);
		rv = ClampedAbs(rv, 10.f);

		float springForce = d * springConstant + ClampedAbs(rv * damping, 10.f * (mass * PhysicsConstants::GRAVITY));

		if (Abs(springForce) > 5.f * (mass * PhysicsConstants::GRAVITY))
			int breakhere = 0;

		Vector3R f = n * springForce;

		ApplyForce(derivative, bi, WheelBodyIndex(i), f, Wheel(i).Body().Position());
	}

	// rider 
	{
		Vector3R d = Body().GetAxis().Global(RiderPosition_Local()) 
					//+ Body().GetAxis().AxisX() * Wheel(1).m_camberAngle 
					- Rider().Position();
		Vector3R vs = Body().PointVelocity(Body().GetAxis().Global(RiderPosition_Local()));
		//vs -= 2.f * (vs | Body().GetAxis().AxisX()) * Body().GetAxis().AxisX();
		Vector3R v = vs - Rider().Velocity();

		float springConstant = Dynamics().rider.riderMass * PhysicsConstants::GRAVITY / Dynamics().rider.riderMovement;
		float springDamping = 2.f * Maths::Sqrt(springConstant * Dynamics().rider.riderMass);

		Vector3R springForce = d * springConstant + v * springDamping;

		ApplyForce(derivative, RiderIndex(), BodyIndex(), springForce, Rider().Position());
	}

	m_step.bodyOnGround = false;

	float gripScale = (Wheel(0).TransverseGripScaleDry() + Wheel(1).TransverseGripScaleDry()) * 0.5f;
	float maxSinRoll = CalculateMaxSinRoll(200.f, gripScale, 0.f);

	if (Abs(SinRoll_Step()) > maxSinRoll || Body().GetAxis().AxisY().Y() < 0.f)
	{
		m_step.bodyOnGround = true;
		m_frame.bodyOnGround = true;
	}

	if (!BodyOnGround_Step())
		int breakhere = 0;

	// tyres
	for (int i = 0; i < m_wheels.Size(); i++)
	{
		if (Wheel(i).OnGround_Step())
		{
			FloatVR mass = Mass() * 0.5f;
			FloatVR springConstant = (mass * PhysicsConstants::GRAVITY * (1.f + DownforceFactor_Step()) / Dynamics().suspension.tyreSpringMovement);
			FloatVR damping = Maths::Sqrt(springConstant * mass);

			Vector3R pv = Wheel(i).Body().PointVelocity(Wheel(i).GroundContactPosition_Step());
			FloatVR v = pv | Wheel(i).GroundNormal_Step();
			FloatVR d = (Wheel(i).GroundPosition_Step() - Wheel(i).GroundContactPosition_Step()) | Wheel(i).GroundNormal_Step();

			RP_ASSERT(Abs(v) < 100.f);

			d = ClampedAbs(d, FloatVR(0.2f));
			v = ClampedAbs(v, FloatVR(5.f));

			FloatVR springForce = d * springConstant - v * damping;

			Vector3R force(0.f, 0.f, 0.f);

			if (springForce > 0.f)
			{
				force = Wheel(i).GroundNormal_Step() * springForce;

				// friction
				if (BodyOnGround_Step()) 
				{
					Vector3R tangentialVelocity = pv - v * Wheel(i).GroundNormal_Step();
					float speed = Magnitude(tangentialVelocity);
					force -= springForce * 0.5f * (tangentialVelocity / (speed + 0.001f)) * Min(speed * 0.1f, 1.f);
				}

				Point3R px;

				// project the force position onto the x axis of the wheel
				{
					Point3R pw = Wheel(i).Body().Position();
					Point3R pg = Wheel(i).GroundContactPosition_Step();
					Normal3R nw = Wheel(i).Body().GetAxis().AxisX();
					Normal3R ng = Wheel(i).GroundNormal_Step();

					Vector3R vp = nw ^ ng;
					Normal3R n = Normalised(ng ^ vp);
					float f = (n | nw);
					float l = (n | (pg - pw)) / f;
					px = pw + nw * l;
				}

				ApplyExternalForce(derivative, WheelBodyIndex(i), force, px);
				//ApplyExternalForce(derivative, WheelBodyIndex(i), f, Wheel(i).Body().CofG());
			}

			Wheel(i).m_frame.groundNormalForce.Accumulate(force * AccumulationFactor());

			//if (springForce <= 0.f)
			//	Wheel(i).SetOffGround();
		}
	}
}

Debug::Float g_airLiftForce("RacingPhysics.Bikes.BikeLiftOnJump", 1.f, Debug::Range(0.f, 1.f));
Debug::Float g_airLiftDamping("RacingPhysics.Bikes.BikeLiftDamping", 0.2f, Debug::Range(0.f, 1.f));
Debug::Float g_airLiftOffset("RacingPhysics.Bikes.BikeLiftOffset", 0.2f, Debug::Range(0.f, 1.f));
  
void Motorbike::EvaluateFrictionForces(Derivative& derivative)
{
	// axle friction
	for (int wi = 0; wi < m_wheels.Size(); wi++)
	{
		int bi = (wi == 0) ? ForkIndex() : BodyIndex();

		FloatVR rollingFriction = Dynamics().friction.rollingFriction;
		FloatVR rollingFriction2 = 0.f;

		S32 surfaceType = Wheel(wi).m_frame.surfaceType;

		/*if (surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_GRASS)
			rollingFriction2 += Surfaces::GetRollingFriction(Surfaces::SURFACE_TYPE_GRASS, Surfaces::SURFACE_TYPE_BIKE_WHEEL);
		else if (surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_GRAVEL)
			rollingFriction2 += Surfaces::GetRollingFriction(Surfaces::SURFACE_TYPE_GRAVEL, Surfaces::SURFACE_TYPE_BIKE_WHEEL);
		else if (surfaceType == CollisionMeshSurfaceFlags::SURFACE_TYPE_STICKY)
			rollingFriction2 += Surfaces::GetRollingFriction(Surfaces::SURFACE_TYPE_STICKY, Surfaces::SURFACE_TYPE_BIKE_WHEEL);

		Surfaces::SurfaceModifierType surfaceModifier = Wheel(wi).m_frame.surfaceModifier;

		if (surfaceModifier != Surfaces::SURFACE_MODIFIER_UNKNOWN)
			rollingFriction2 += Surfaces::GetRollingFriction(surfaceModifier, Surfaces::SURFACE_TYPE_BIKE_WHEEL);*/

		// slow the wheels down when no rider
		if (!RiderPresent())
			rollingFriction += 100.f;

		// get final angular momentum
		Vector3R angularMomentum = Wheel(wi).Body().AngularMomentum() + derivative.parts[WheelBodyIndex(wi)].torque * TimeStep();
		Vector3R angularVelocity = Wheel(wi).Body().InverseMIGlobal() * angularMomentum;
		FloatVR axleRotation = angularVelocity | Wheel(wi).Body().GetAxis().AxisX();
		rollingFriction += rollingFriction2 * Abs(axleRotation);
		rollingFriction = Min(rollingFriction, Mass() * PhysicsConstants::GRAVITY * (1.f + DownforceFactor_Step()) * Wheel(wi).Radius() * 0.5f);
		FloatVR t = ClampedAbs(axleRotation / TimeStep(), rollingFriction / Wheel(wi).Inertia()) * Wheel(wi).Inertia();
		Vector3R torque = Wheel(wi).Body().GetAxis().AxisX() * -t;

		ApplyTorque(derivative, WheelBodyIndex(wi), bi, torque);
	}

	// thrust
	if (Wheel(1).OnGround_Step() && !Air())
	{
		Normal3R groundZ = Normalised(Body().GetAxis().AxisX() ^ SmoothedGroundNormal());
		Vector3R thrust = -groundZ * Controls().throttle * Mass() * (g_debugThrust + ForcedAcceleration());
		ApplyExternalForce(derivative, BodyIndex(), thrust, CofG());
	}

	// air friction
	{
		Vector3R v = Body().Velocity();
		Vector3R frictionForce = -v * Magnitude(v) * Dynamics().friction.airFriction 
									* (1.f + Dynamics().friction.riderFrictionScale * Clamped(FullBrakes(), 0.f, 1.f));

		frictionForce -= v * Dynamics().friction.wheelieFriction * Clamped(-m_controls.lean, 0.f, 1.f) * Mass();

		float slipStream = Clamped(SlipStream(), 0.f, 1.f);
		frictionForce *= 1.f - slipStream;

		ApplyExternalForce(derivative, BodyIndex(), frictionForce, CofG());
		m_airFrictionForce.Accumulate(frictionForce * AccumulationFactor());
	}

	// stability
	if (RiderPresent())
	{
		if (!Wheel(0).OnGround_Step() && !Wheel(1).OnGround_Step() && Body().GetAxis().AxisY().Y() > 0.f) //air
		{
			float x = (Body().GetAxis().AxisZ() | Constants<Normal3R>::Y()) + g_airLiftOffset;
			float v = Body().AngularVelocity() | Body().GetAxis().AxisX();
			float f = Clamped(x * g_airLiftForce - v * g_airLiftDamping, -0.5f, 0.5f);
			ApplyExternalTorque(derivative, BodyIndex(), Body().GetAxis().AxisX() * Mass() * PhysicsConstants::GRAVITY * f);
		}
		
		if (Wheel(1).OnGround_Step() && (m_controls.lean < 0.f || !Wheel(0).OnGround_Step())
			&& !Dynamics().engine.antiFlip) // help with wheelie
		{
			float x = -Body().GetAxis().AxisZ().Y();
			float v = Body().AngularVelocity() | Body().GetAxis().AxisX();

			float liftRatio = Dynamics().engine.liftRatio;
			liftRatio *= Clamped(-m_controls.lean * Throttle() * (1.f - FullBrakes()) * (1.f - RearBrake()), 0.f, 1.f);
			liftRatio = Clamped(liftRatio, 0.f, 1.f);

			float f = Clamped((liftRatio - x) * 2.f - v * Dynamics().engine.wheelieDamping, -1.f, 1.f);

			if (Wheel(0).OnGround_Step()) f *= Throttle();

			ApplyExternalTorque(derivative, BodyIndex(), Body().GetAxis().AxisX() * Mass() * PhysicsConstants::GRAVITY * DownforceFactor_Step() * f * Dynamics().engine.wheelieLiftConstant);
		}
		else if (Wheel(0).OnGround_Step() && (m_controls.lean > 0.f || !Wheel(1).OnGround_Step())
			&& !Dynamics().brakes.antiFlip) // help with endo
		{
			float x = Body().GetAxis().AxisZ().Y();
			float v = -(Body().AngularVelocity() | Body().GetAxis().AxisX());

			float liftRatio = Dynamics().brakes.liftRatio;
			liftRatio *= Clamped(m_controls.lean * FullBrakes(), 0.f, 1.f);
			liftRatio = Clamped(liftRatio, 0.f, 1.f);

			float speedFactor = Clamped((float)Magnitude(Body().Velocity()), 0.f, 1.f);
			float steerFactor = Abs(m_steerSmooth);
			float f = Clamped((liftRatio - x) * 2.f - v * Dynamics().brakes.endoDamping, -1.f, 1.f) * speedFactor * (1.f - steerFactor);

			if (Wheel(1).OnGround_Step()) f *= FullBrakes();

			ApplyExternalTorque(derivative, BodyIndex(), -Body().GetAxis().AxisX() * Mass() * PhysicsConstants::GRAVITY * DownforceFactor_Step() * f * Dynamics().brakes.endoLiftConstant);
		}
		
		if (Wheel(0).OnGround_Step() && !Wheel(1).OnGround_Step()) // endo stability
		{
			Normal3R n1 = Body().GetAxis().AxisX();
			Normal3R n2 = Constants<Normal3R>::Y();
			Normal3R n3 = Normalised(n1 ^ n2);

			float rollConstant = 0.25f;
			float rollDamping = -0.1f;
			float yawConstant = 0.1f;
			float yawDamping = 0.1f;
			
			float yaw = (Fork().GetAxis().AxisZ() | Body().GetAxis().AxisX());
			float yawRate = (Fork().AngularVelocity() - Body().AngularVelocity()) | Fork().GetAxis().AxisY();

			float d = (Wheel(0).GroundNormal_Step() | Body().GetAxis().AxisZ());
			float v = Body().AngularVelocity() | Wheel(1).Body().GetAxis().AxisX();
			float s = Magnitude(Body().Velocity());
			float f = /*Max(1.f - Abs(v), 0.f) **/ Max(d, 0.f) * Min(s, 1.f);

			float rollTorque = m_step.sinRoll * rollConstant + m_step.dr * rollDamping;
			float yawTorque = yaw * yawConstant + yawRate * yawDamping;

			//static float test = 0.05f;

			//if (Abs(yaw) > test || Abs(m_step.sinRoll) > test)
			//	int breakhere = 0;

			float minSpeed = 0.f;

			if (s < minSpeed)
				f = 0.f;

			Vector3R t = n2 * rollTorque + n3 * yawTorque * f;
			ApplyExternalTorque(derivative, ForkIndex(), (Mass() * PhysicsConstants::GRAVITY) * t);
		}

		if (!Wheel(0).OnGround_Step() && Wheel(1).OnGround_Step()) // wheelie stability and steering
		{		
			float s_rollConstant = 0.4f;
			float s_rollDamping = -0.3f;
			float s_steerFeedbackRate = 1.f;
			//float s_yawConstant = 0.1f;
			float s_yawDamping = -0.4f;
			
			float yawRate = (Body().AngularVelocity()) | Body().GetAxis().AxisY();
			
			float d = (Wheel(1).GroundNormal_Step() | Body().GetAxis().AxisZ());
			float v = Body().AngularVelocity() | Wheel(1).Body().GetAxis().AxisX();

			float r = SinRoll_Step() - m_steerSmooth * MaxSinRoll_Step() * Dynamics().steering.rollFraction;
			float dr = RollRate_Step() + m_steerSmoothReduced * s_steerFeedbackRate;

			float azy = -Body().GetAxis().AxisZ().Y();
			float rollTorque = (r * s_rollConstant + dr * s_rollDamping) * (1.f - azy * azy);
			float yawTorque = yawRate * s_yawDamping * azy;
			
			Vector3R t = Constants<Normal3R>::Y() * rollTorque + Body().GetAxis().AxisY() * yawTorque;
			ApplyExternalTorque(derivative, BodyIndex(), Mass() * PhysicsConstants::GRAVITY * t);

			/*static float sb2 = 0.f;
			static float sb3 = 0.f;
			static float sf2 = 0.f;
			static float sf3 = 0.f;

			Vector3R tb = n2 * sb2 + n3 * sb3;
			Vector3R tf = n2 * sf2 + n3 * sf3;

			ApplyExternalTorque(derivative, BodyIndex(), Mass() * PhysicsConstants::GRAVITY * tb);
			ApplyExternalTorque(derivative, ForkIndex(), Mass() * PhysicsConstants::GRAVITY * tf);*/
		}
	}
}

extern Debug::Bool g_updatePhysics;

void Motorbike::EvaluateConstraintForces(Derivative& derivative, float dt, bool constrainPositions)
{
	RP_ASSERT(Validate());

	State oldState;
	GetState(oldState);

	if (!constrainPositions)
		UpdateVelocities(derivative, dt);

	ConstraintSolver& solver(m_constraintSolver);

	solver.Initialise(NumRigidBodies() + 1, 22);

	if (constrainPositions)
		solver.SetVelocityCorrection(dt);

	for (int i = 0; i < NumRigidBodies(); i++)
		solver.AddRigidBody(&GetRigidBody(i));

	solver.AddRotor(&GetRotor(0));

	int ci_wheelGroundNormal[2];
	ci_wheelGroundNormal[0] = -1;
	ci_wheelGroundNormal[1] = -1;

	int ci_wheelGroundFrictionX[2];
	ci_wheelGroundFrictionX[0] = -1;
	ci_wheelGroundFrictionX[1] = -1;

	int ci_wheelGroundFrictionZ[2];
	ci_wheelGroundFrictionZ[0] = -1;
	ci_wheelGroundFrictionZ[1] = -1;

	// wheel constraints
	for (int wi = 0; wi < m_wheels.Size(); wi++)
	{
		int bi = (wi == 0) ? ForkIndex() : BodyIndex();

		Point3R wheelPosition =  Wheel(wi).Body().Position();
		Point3R bodyWheelPosition = RestingWheelPosition_Global(wi);
		Normal3R wheelX = Wheel(wi).Body().GetAxis().AxisX();
		Normal3R wheelY = Normalised(GetRigidBody(bi).GetAxis().AxisZ() ^ Wheel(wi).Body().GetAxis().AxisX());
		Normal3R wheelZ = Normalised(Wheel(wi).Body().GetAxis().AxisX() ^ GetRigidBody(bi).GetAxis().AxisY());

		solver.SetDefaultRigidBodies(WheelBodyIndex(wi), bi);
		solver.ConstrainPositions(GetRigidBody(bi).GetAxis().AxisX(), wheelPosition, bodyWheelPosition);
		solver.ConstrainPositions(GetRigidBody(bi).GetAxis().AxisZ(), wheelPosition, bodyWheelPosition);
		solver.ConstrainAxis(GetRigidBody(bi).GetAxis().AxisZ(), wheelY, GetRigidBody(bi).GetAxis().AxisY());
		solver.ConstrainAxis(GetRigidBody(bi).GetAxis().AxisY(), wheelZ, GetRigidBody(bi).GetAxis().AxisZ());

		// suspension limits
		//if (!constrainPositions)
		{
			float s = Dynamics().suspension.maxSuspensionMovement;
			const Normal3R& n = GetRigidBody(bi).GetAxis().AxisY();

			if (Wheel(wi).SuspensionDistance_Step() > s)
			{
				solver.ConstrainPositions(n, wheelPosition, bodyWheelPosition + n * s);
				solver.SetConstraintMinMax(-FLT_MAX, 0.f);
			}
			else if (Wheel(wi).SuspensionDistance_Step() < -s)
			{
				solver.ConstrainPositions(n, wheelPosition, bodyWheelPosition - n * s);
				solver.SetConstraintMinMax(0.f, FLT_MAX);
			}
		}

		float brakeForce = (wi == 0) ? FullBrakes() : RearBrake();
		float a = Abs(Wheel(wi).AngularVelocity1D());
		
		// brakes
		if (brakeForce > 0.01f && a < 1.f)
		{
			float s_max = 10.f;
			float f = s_max * brakeForce * (1.f - a); // small constraint force is enough

			solver.ConstrainAxis(GetRigidBody(bi).GetAxis().AxisX(), wheelY, GetRigidBody(bi).GetAxis().AxisY());
			solver.SetConstraintMinMax(-f, f);
		}
		
		if (!constrainPositions)
		{
			if (Wheel(wi).OnGround_Step())
			{
				float normalForce = (Wheel(wi).GroundForce() | Wheel(wi).GroundNormal_Step()) * Wheel(wi).TransverseGripScale() * dt;

				if (normalForce > 0.f)
				{
					float fx = Wheel(wi).StaticFrictionX_Step() * normalForce;
					float fz = Wheel(wi).StaticFrictionZ_Step() * normalForce;

					if (fx != 0.f) // low speed only
					{
						// lateral force
						ci_wheelGroundFrictionX[wi] = solver.GetNumConstraints();

						solver.AddConstraintForce(Wheel(wi).GroundX_Step(), 
													Wheel(wi).GroundContactPosition_Step(),
													0.f,
													WheelBodyIndex(wi), -1);
						
						solver.SetConstraintMinMax(-fx, fx);
					}

					if (fz != 0.f) // low speed only
					{
						ci_wheelGroundFrictionZ[wi] = solver.GetNumConstraints();

						// longitudinal force
						solver.AddConstraintForce(Wheel(wi).GroundZ_Step(), 
													Wheel(wi).GroundContactPosition_Step(),
													0.f,
													WheelBodyIndex(wi), -1);

						solver.SetConstraintMinMax(-fz, fz);
					}
				}
			}
		}
		/*else
		{
			if (Wheel(wi).OnGround())
			{
				float normalForce = (Wheel(wi).GroundForce() | Wheel(wi).GroundNormal()) * Wheel(wi).TransverseGripScale() * dt;

				if (normalForce > 0.f)
				{
					float fz = RearBrake() * normalForce * (1.f - Min(Speed_GlobalXZ(), 1.f));

					if (fz != 0.f) // low speed only
					{
						ci_wheelGroundFrictionZ[wi] = solver.GetNumConstraints();
						Normal3R n = Normalised(m_wheels[wi].Body().GetAxis().AxisX() ^ m_wheels[wi].GroundForce());

						// longitudinal force
						solver.AddConstraintForce(n, Wheel(wi).GroundPosition(), 0.f, WheelBodyIndex(wi), -1);

						solver.SetConstraintMinMax(-fz, fz);
					}
				}
			}
		}*/
	}

	// attach fork to body
	{
		Point3R forkPosition = Fork().Position();
		Point3R bodyForkPosition = Body().GetAxis().Global(ForkPosition_Local());
		Normal3R bodyForkAxisY = Body().GetAxis().Global(ForkAxis_Local());

		Normal3R bodyForkAxisX = -Normalised(Fork().GetAxis().AxisZ() ^ bodyForkAxisY);
		Normal3R bodyForkAxisZ = Normalised(Fork().GetAxis().AxisX() ^ bodyForkAxisY);

		solver.SetDefaultRigidBodies(ForkIndex(), BodyIndex());
		solver.ConstrainPositions(Fork().GetAxis().AxisX(), forkPosition, bodyForkPosition);
		solver.ConstrainPositions(Fork().GetAxis().AxisY(), forkPosition, bodyForkPosition);
		solver.ConstrainPositions(Fork().GetAxis().AxisZ(), forkPosition, bodyForkPosition);
		solver.ConstrainAxis(Fork().GetAxis().AxisZ(), Fork().GetAxis().AxisX(), bodyForkAxisX);
		solver.ConstrainAxis(Fork().GetAxis().AxisX(), Fork().GetAxis().AxisZ(), bodyForkAxisZ);

		float a = asin(Fork().GetAxis().AxisZ() | Body().GetAxis().AxisX());
		float limit = Dynamics().steering.steeringLock;

		if (Abs(a) >= limit)
		{
			Normal3R rotatedBodyForkAxisZ = Normalised(Body().GetAxis().AxisX() ^ bodyForkAxisY);
			rotatedBodyForkAxisZ = Rotated(rotatedBodyForkAxisZ, Fork().GetAxis().AxisY(), limit * Sign(a));
			solver.ConstrainAxis(Fork().GetAxis().AxisY(), Fork().GetAxis().AxisZ(), rotatedBodyForkAxisZ);

			if (!constrainPositions)
			{
				if (limit == 0.f)
					solver.SetConstraintMinMax(-FLT_MAX, FLT_MAX);
				else if (a > 0.f) 
					solver.SetConstraintMinMax(-FLT_MAX, 0.f);
				else 
					solver.SetConstraintMinMax(0.f, FLT_MAX);
			}
		}
	}

	// engine
	if (!constrainPositions)
	{
		solver.SetDefaultRigidBodies(BodyIndex(), EngineIndex());
		solver.AddConstraintTorque(Body().GetAxis().AxisY());
		solver.AddConstraintTorque(Body().GetAxis().AxisZ());
	}

	// roll
	//{
	//	solver.ConstrainAxis(Body().GetAxis().AxisZ(), WheelBodyIndex(1), -1);
	//}

	solver.Solve();

	if (constrainPositions)
		int breakhere = 0;

	int ci = 0;

	// check constraints
	bool constraintsOk = true;

	FloatVR inverseTime = Maths::Reciprocal(FloatVR(dt));

	for (int ci = 0; ci < solver.GetNumConstraints(); ci++)
	{
		Vector3R f = solver.GetConstraint(ci) * inverseTime;

		RP_ASSERT(Maths::Finite(f));
		RP_ASSERT(Abs(solver.GetConstraintSize(ci)) < Mass() * 100.f);

		int i0 = solver.GetConstraintBodyIndex0(ci);
		int i1 = solver.GetConstraintBodyIndex1(ci);

		if (solver.GetConstraintType(ci) == ConstraintSolver::CONSTRAINT_TYPE_FORCE)
		{
			if (i1 > -1)
				ApplyForce(derivative, i0, i1, f, solver.GetConstraintPosition(ci));
			else
				ApplyExternalForce(derivative, i0, f, solver.GetConstraintPosition(ci));
		}
		else
		{
			if (i1 > -1)
				ApplyTorque(derivative, i0, i1, f);
			else
				ApplyExternalTorque(derivative, i0, f);
		}

		float fx = derivative.parts[1].torque | Wheel(0).Body().GetAxis().AxisY();
		int breakhere = 0;
	}

	if (!constrainPositions)
	{
		for (int wi = 0; wi < m_wheels.Size(); wi++)
		{
			if (ci_wheelGroundNormal[wi] > -1)
			{
				Wheel(wi).m_frame.groundNormalForce.Accumulate(solver.GetConstraint(ci_wheelGroundNormal[wi]) * AccumulationFactor() / dt);		
			}

			if (ci_wheelGroundFrictionX[wi] > -1)
			{
				Wheel(wi).m_frame.groundFriction.Accumulate(solver.GetConstraint(ci_wheelGroundFrictionX[wi]) * AccumulationFactor() / dt);
			}

			if (ci_wheelGroundFrictionZ[wi] > -1)
			{
				Wheel(wi).m_frame.groundFriction.Accumulate(solver.GetConstraint(ci_wheelGroundFrictionZ[wi]) * AccumulationFactor() / dt);
			}
		}
	}

	RestoreState(oldState);

	// debug
	if (0 && !constrainPositions)
	{
		State oldState;
		GetState(oldState);

		for (int i = 0; i < NumRigidBodies(); i++)
			GetRigidBody(i).UpdateVelocities(derivative.parts[i], dt);

		CheckConstraints();

		RestoreState(oldState);
	}

	solver.Clear();

	//return constraintsOk;
}

void Motorbike::ClearStepVariables()
{
	memset(&m_step, 0xffffffff, sizeof(m_step));

	Wheel(0).ClearStepVariables();
	Wheel(1).ClearStepVariables();
}

int g_testDerivative = 0;

void Motorbike::EvaluateDerivative(Derivative& output)
{
	//PixEvent pix(0xffff0000, "Motorbike::EvaluateDerivative");
	RP_ASSERT(Validate());
	ClearStepVariables();

	m_step.downforceFactor = CalculateDownforceFactor();

	for (int i = 0; i <  NumRigidBodies(); i++)
	{
		GetRigidBody(i).EvaluateDerivative(output.parts[i]);
		ApplyGravity(output, i);

		if (!Air())
			output.parts[i].force -= SmoothedGroundNormal() * (GetRigidBody(i).Mass() * PhysicsConstants::GRAVITY * DownforceFactor_Step());
	}

	GetRotor(0).EvaluateDerivative(output.parts[NumRigidBodies()]);

	{
		float tanRoll, sinPitch, sinYaw;
		Body().GetAxis().GetRollPitchYaw_Trig(tanRoll, sinPitch, sinYaw);
		m_step.sinRoll = tanRoll / sqrt(1.f + tanRoll * tanRoll);
		m_frame.sinRoll.Accumulate(m_step.sinRoll* AccumulationFactor());
	}

	float gripScale = (Wheel(0).TransverseGripScale() + Wheel(1).TransverseGripScale()) * 0.5f;
	float slideFactor = (1.f - Wheel(1).m_frame.slipScaleFactor.GetValue());
	m_step.maxSinRoll = CalculateMaxSinRoll(Magnitude(Body().Velocity()), gripScale, slideFactor);
	
	{
		float dr_1, dr_2;

		float dr_x = Vector3R(Body().AngularVelocity() ^ Body().GetAxis().AxisX()).Y();
		float dr_y = Vector3R(Body().AngularVelocity() ^ Body().GetAxis().AxisY()).Y();
		float y = GetAxis().AxisY().Y();
		float x = GetAxis().AxisX().Y();
		dr_2 = (-x * dr_y - y * dr_x) / (y * y);

		Axis axis2 = Body().GetAxis();
		axis2.Rotate(Body().AngularVelocity() * TimeStep());
		float tanRoll, sinPitch, sinYaw;
		axis2.GetRollPitchYaw_Trig(tanRoll, sinPitch, sinYaw);
		float sinRoll2 = tanRoll / sqrt(1.f + tanRoll * tanRoll);

		dr_1 = (SinRoll_Step() - sinRoll2) / TimeStep();
		dr_1 -= RollCorrection();

		m_step.dr = dr_1;
		m_frame.dr.Accumulate(m_step.dr * AccumulationFactor());
	}

	EvaluateSpringForces(output);
	if (g_testDerivative == 1) {ClearStepVariables(); return;};
	if (!BodyOnGround_Step()) EvaluateTyreForces(output);
	if (g_testDerivative == 2) {ClearStepVariables(); return;};
	EvaluateFrictionForces(output);
	EvaluateNetworkForces(output);
	if (g_testDerivative == 3) {ClearStepVariables(); return;};
	EvaluateControlForces(output);
	if (g_testDerivative == 4) {ClearStepVariables(); return;};

	/*if (Wheel(1).OnGround())
	{
		Vector3R newAngularMomentum = Wheel(1).Body().AngularMomentum() + output.parts[WheelBodyIndex(1)].torque * TimeStep();
		Vector3R newAngularVelocity = Wheel(1).Body().InverseMIGlobal() * newAngularMomentum;
		float w = newAngularVelocity | Wheel(1).Body().GetAxis().AxisX();
		float wRequired = Wheel(1).TractionControlAV();
		float maxEngineTorque = -(wRequired - w) * Wheel(1).Inertia() / TimeStep();
		int breakhere = 0;
	}*/

	EvaluateConstraintForces(output, TimeStep(), false);

	/*if (Wheel(1).OnGround())
	{
		Vector3R newAngularMomentum = Wheel(1).Body().AngularMomentum() + output.parts[WheelBodyIndex(1)].torque * TimeStep();
		Vector3R newAngularVelocity = Wheel(1).Body().InverseMIGlobal() * newAngularMomentum;
		float w = newAngularVelocity | Wheel(1).Body().GetAxis().AxisX();
		float wRequired = Wheel(1).TractionControlAV();
		float maxEngineTorque = -(wRequired - w) * Wheel(1).Inertia() / TimeStep();
		int breakhere = 0;
	}*/

	ClearStepVariables();
	RP_ASSERT(Validate());
}

float Motorbike::ConstrainInternal()
{
	//PixEvent pix(0xffff0000, "Motorbike::ConstrainInternal");

	Derivative derivative(GetNumParts());

	for (int i = 0; i < m_wheels.Size(); i++)
		UpdateSuspensionDistance(i);

	for (int i = 0; i < derivative.parts.Size(); i++)
		Clear(derivative.parts[i]);

	//Clear(derivative.rotor);

	// no need to use actual frame time here
	float dt = 1.f / 60.f;

	EvaluateConstraintForces(derivative, dt, true);

	float sinRoll1, sinRoll2;
	float tanRoll, sinPitch, sinYaw;

	Body().GetAxis().GetRollPitchYaw_Trig(tanRoll, sinPitch, sinYaw);
	sinRoll1 = tanRoll / sqrt(1.f + tanRoll * tanRoll);

	for (int i = 0; i < NumRigidBodies(); i++)
		GetRigidBody(i).UpdatePositions(derivative.parts[i], dt);

	Body().GetAxis().GetRollPitchYaw_Trig(tanRoll, sinPitch, sinYaw);
	sinRoll2 = tanRoll / sqrt(1.f + tanRoll * tanRoll);

	m_frame.rollCorrection = (sinRoll2 - sinRoll1) / dt;

	return 0.f;
}

void Motorbike::CheckConstraints()
{
	/*if (Wheel(0).OnGround_Step())
	{
		int wi = 0;

		Point3R wheelPosition = Wheel(wi).Body().Position();
			
		Vector3R rv = Wheel(wi).Body().PointVelocity(wheelPosition)
						- Fork().PointVelocity(wheelPosition);

		float rvz = rv | Fork().GetAxis().AxisZ();
		float rvx = rv | Fork().GetAxis().AxisX();

		if (Abs(rvx) > 1.f || Abs(rvz) > 1.f)
			int breakhere = 0;

		Vector3R v = Wheel(0).Body().PointVelocity(Wheel(wi).GroundContactPosition_Step());
		float f = v | Wheel(wi).GroundX_Step();

		int breakhere = 0;
	}*/

	Vector3R v = Wheel(1).Body().PointVelocity(Wheel(1).GroundContactPosition_Step());
	float f = v | Wheel(1).GroundZ_Step();

	int breakhere = 0;
}

////////////////////////////////////////////////////////////////////////
// collisions

/*
FloatVR Motorbike::GetLargestVelocity() const
{
	FloatVR v = Magnitude(Body().PointVelocity(CollisionObject().TransformedCentre())) 
			  + Magnitude(Body().AngularVelocity()) * CollisionObject().Radius();

	return v;
}
*/

void Motorbike::Push(const Point3R& p, const Vector3R& v)
{
	m_collisionBody.Push(p, v);

	CopyStateFromCollisionBody();
}

/*
void Motorbike::GetCollisionBoundingBox(Point3R& min, Point3R& max) const
{
	min = max = CollisionObject().TransformedCentre();
	min -= CollisionObject().AABB();
	max += CollisionObject().AABB();
}
*/

void Motorbike::ApplyImpulse(const Point3R& p, const Vector3R& impulse, FloatVR torqueFactor) 
{
	m_collisionBody.ApplyImpulse(p, impulse, torqueFactor);
	CopyStateFromCollisionBody();
}

void Motorbike::HandleCollision(const Point3R& p, const Vector3R& impulse)
{
	// stop bike steering into the wall
	float a = (p - Body().Position()) | Body().GetAxis().AxisX();
	if (a > 0.1f) m_leftCollisionTime = 0.05f;
	if (a < -0.1f) m_rightCollisionTime = 0.05f;

	m_feedback.HandleCollision(Magnitude(impulse));
}


/*float Motorbike::GetCollisionTime(float time, float resolution) const
{
	RP_ASSERT(time > 0.f);
	RP_ASSERT(resolution <= time);

	for (int i = 0; i < 2; i++)
	{
		float wheelSpeed = Abs(Wheel(i).Body().Velocity().Y());
		float t = Wheel(i).Radius() / wheelSpeed;
		time = Min(time, t);
	}

	time = Max(time, resolution);
	return Vehicle::GetCollisionTime(time, resolution);
}*/

//////////////////////////////////////////////////////////////////////////////////

void Motorbike::XtReload()
{
	bool changed;
	m_pSettings = MotorbikeDynamicsXt::LoadDynamicsXt(GetFilename(), changed);

	if (changed)
		m_dynamicsChangedCounter = 30;
}

bool Motorbike::PutOnGround()
{
	RP_ASSERT(GetAxis().DebugTest());

	// put bike on ground
	float h = -10000.f;
	float wheelHeight[2];

	for (int j = 0; j < 2; j++)
	{
		float average = 0.f;

		for (int i = 0; i < 2; i++)
		{
			wheelHeight[i] = InfinitePlaneSystem::Instance().GetPlane().GetHeight(Wheel(i).Position());

			average += wheelHeight[i];
		}

		h = average / 2.f;

		if (h > -1000.f)
			break;

		SetPosition(GetAxis().Position() + Vector3R(0.f, 20.f, 0.f));
	}
	
	if (h > -1000.f)
	{
		SetPosition(Point3R(GetAxis().Position().X(), h + Wheel(0).Radius() + Abs(GetAxis().AxisZ().Y()), GetAxis().Position().Z()));

		FloatVR angleX = FloatVR(atan((wheelHeight[0] - wheelHeight[1]) / Dynamics().geometry.wheelBase));
		SetAxis(GetAxis().Rotated(GetAxis().AxisX(), angleX));
	}
	else
		return false;

	return true;
}

}
