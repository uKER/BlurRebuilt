////////////////////////////////////////////////////////////////////
// Bike Network Physics
////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "racingphysics/vehicles/motorbike/motorbike.h"
//#include "racingphysics/collisions/collisionmesh.h"
#include "racingphysics/collisions/raycast.h"
#include "racingphysics/physics/compressfloat.h"
#include "racingphysics/physics/physicstimer.h"
#include "racingphysics/physics/quaternion.h"
#include "core/maths/hvec/quaternion.h"

//using namespace Maths;

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize("", off) // DEBUG_RACING_PHYSICS
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace RacingPhysics
{

struct BikeCompressedNetworkState // 24 bytes
{
	U64 px : 22;
	U64 py : 20;
	U64 pz : 22;
	
	U64 rx : 12;
	U64 ry : 12;
	U64 rz : 12;
	U64 vx : 12;
	U64 avx : 8;
	U64 avz : 8;

	U64 vy : 13;
	U64 vz : 12;
	U64 avy : 12;
	U64 sa : 8;
	U64 framesSinceRaceStart : 16;
	U64 gear : 3;
};

struct BikeNetworkControls // 4 bytes
{
	U32 throttle : 5;
	U32 horn : 1;
	U32 fullBrakes : 5;
	U32 steer : 8;
	U32 lean : 4;

	U32 gearUp : 1;
	U32 gearDown : 1;
	U32 rearBrake : 1;
	U32 reverse : 1;
	U32 isWheel : 1;
	U32 ghosted : 1;
	U32 stopUpdating : 1;
	U32 riderPresent : 1;
};

BIZ_STATIC_ASSERT(sizeof(Vehicle::CompressedNetworkControls) == sizeof(BikeNetworkControls));
BIZ_STATIC_ASSERT(sizeof(Vehicle::CompressedNetworkState) == sizeof(BikeCompressedNetworkState) + 1);

void Motorbike::GetNetworkState(NetworkState& networkState) const
{
	networkState.velocity = Body().Velocity();
	networkState.angularVelocity = Body().AngularVelocity();
	networkState.gear = Engine().Gear();
	networkState.framesSinceRaceStart = PhysicsTimer::Instance().FramesSinceRaceStart();
	networkState.networkStateValid = true;
	networkState.useFullPrediction = true;
	networkState.air = Air();
	networkState.groundNormal = !Air() ? SmoothedGroundNormal() : Constants<Normal3R>::Y();
	networkState.axis = GetAxis();
	networkState.steeringAngle = (Fork().GetAxis().AxisZ() | Body().GetAxis().AxisX());
	Clear(&networkState.acceleration);
}

void Motorbike::GetNetworkState(Vehicle::CompressedNetworkState* pVehicleState) const
{
	NetworkState networkState;
	GetNetworkState(networkState);
	CompressNetworkState(networkState, *pVehicleState);
}

void Motorbike::CompressNetworkState(const NetworkState& networkState, CompressedNetworkState& vehicleState)
{
	BikeCompressedNetworkState* state = (BikeCompressedNetworkState*) &vehicleState;

	const Vector3R sceneCentre	=	Vector3R(0.f, 0.f, 0.f);//VehicleManager::Instance().SceneCentre();
	const Vector3R sceneSize	=	Vector3R(50000.f, 10000.f, 50000.f);//VehicleManager::Instance().SceneSize();

	RP_ASSERT(networkState.axis.DebugTest());

	Quaternion q = Quaternion(networkState.axis);
	RP_ASSERT(q.W() >= 0.f);
	q.Normalise(); // just to avoid > 1.0 due to floating point error

	state->px = CompressFloat<U32, 22>(networkState.axis.Position().X() - sceneCentre.X(), -sceneSize.X(), sceneSize.X());
	state->py = CompressFloat<U32, 20>(networkState.axis.Position().Y() - sceneCentre.Y(), -sceneSize.Y(), sceneSize.Y());
	state->pz = CompressFloat<U32, 22>(networkState.axis.Position().Z() - sceneCentre.Z(), -sceneSize.Z(), sceneSize.Z());

	state->rx = CompressFloat<U32, 12>(q.A().X(), -1.f, 1.f);
	state->ry = CompressFloat<U32, 12>(q.A().Y(), -1.f, 1.f);
	state->rz = CompressFloat<U32, 12>(q.A().Z(), -1.f, 1.f);

	state->vx = CompressFloat<U32, 12>(networkState.velocity.X(), -200.f, 200.f);
	state->vy = CompressFloat<U32, 13>(networkState.velocity.Y(), -100.f, 100.f);
	state->vz = CompressFloat<U32, 12>(networkState.velocity.Z(), -200.f, 200.f);

	// clamp these: only happens in collisions when prediction is not critical
	state->avy = CompressFloat<U32, 12>(Clamped((float)networkState.angularVelocity.Y(), -50.f, 50.f), -50.f, 50.f);
	state->avx = CompressFloat<U32, 8>(Clamped((float)networkState.angularVelocity.X(), -10.f, 10.f), -10.f, 10.f);
	state->avz = CompressFloat<U32, 8>(Clamped((float)networkState.angularVelocity.Z(), -10.f, 10.f), -10.f, 10.f);

	state->sa = CompressFloat<U32, 8>(networkState.steeringAngle, -1.f, 1.f);

	state->framesSinceRaceStart = (U16) networkState.framesSinceRaceStart;
	state->gear = (U32 (networkState.gear + 1));
}

void Motorbike::UncompressNetworkState(const CompressedNetworkState& vehicleState, NetworkState& networkState)
{
	const BikeCompressedNetworkState& state = (const BikeCompressedNetworkState&) vehicleState;

	const Vector3R sceneCentre	=	Vector3R(0.f, 0.f, 0.f);//VehicleManager::Instance().SceneCentre();
	const Vector3R sceneSize	=	Vector3R(50000.f, 10000.f, 50000.f);//VehicleManager::Instance().SceneSize();

	Vector3R rotation;

	Point3R position;
	position.X(UncompressFloat<U32, 22>(state.px, -sceneSize.X(), sceneSize.X()) + sceneCentre.X());
	position.Y(UncompressFloat<U32, 20>(state.py, -sceneSize.Y(), sceneSize.Y()) + sceneCentre.Y());
	position.Z(UncompressFloat<U32, 22>(state.pz, -sceneSize.Z(), sceneSize.Z()) + sceneCentre.Z());
	networkState.axis.SetPosition(position);

	rotation.X(UncompressFloat<U32, 12>(state.rx, -1.f, 1.f));
	rotation.Y(UncompressFloat<U32, 12>(state.ry, -1.f, 1.f));
	rotation.Z(UncompressFloat<U32, 12>(state.rz, -1.f, 1.f));

	networkState.velocity.X(UncompressFloat<U32, 12>(state.vx, -200.f, 200.f));  
	networkState.velocity.Y(UncompressFloat<U32, 13>(state.vy, -100.f, 100.f));
	networkState.velocity.Z(UncompressFloat<U32, 12>(state.vz, -200.f, 200.f));

	networkState.angularVelocity.Y(UncompressFloat<U32, 12>(state.avy, -50.f, 50.f));
	networkState.angularVelocity.X(UncompressFloat<U32, 8>(state.avx, -10.f, 10.f));
	networkState.angularVelocity.Z(UncompressFloat<U32, 8>(state.avz, -10.f, 10.f));

	networkState.steeringAngle = UncompressFloat<U32, 8>(state.sa, -1.f, 1.f);

	networkState.framesSinceRaceStart = state.framesSinceRaceStart;

	int framesSinceRaceStart = (int) PhysicsTimer::Instance().FramesSinceRaceStart();
	while (framesSinceRaceStart - networkState.framesSinceRaceStart > 30000)	// wrapped around
		networkState.framesSinceRaceStart += (1<<16);

	networkState.gear = state.gear;

	networkState.axis.SetRotation(Quaternion(rotation));
	networkState.networkStateValid = true;
	networkState.useFullPrediction = true;
	Clear(&networkState.acceleration);

	// not stored in compressed state
	networkState.air = false;
	networkState.groundNormal = Constants<Normal3R>::Y();

	Clear(&networkState.acceleration);
}

bool Motorbike::ProcessNetworkState(const CompressedNetworkState& vehicleState, bool runCatchUp)
{
	//RP_ASSERT(!assertOnFail || m_networkState.axis.DebugTest());

	UncompressNetworkState(vehicleState, m_networkState);

	RP_ASSERT(m_networkState.axis.DebugTest());

	float catchUpTime = ((m_networkState.framesSinceRaceStart - 1) - PhysicsTimer::Instance().FramesSinceRaceStart()) * PhysicsTimer::Instance().PhysicsUpdateTime(); 

	//if (catchUpTime > PhysicsTimer::Instance().PhysicsUpdateTime())
	//	VehicleManager::Instance().NetworkCatchUp(catchUpTime);  

	m_predictedNetworkState = m_networkState;

	return true;
}

void Motorbike::GetNetworkControls(CompressedNetworkControls& controlsOut) const
{
	BikeNetworkControls& bikeNetworkControls = (BikeNetworkControls&) controlsOut;

	bikeNetworkControls.throttle = CompressFloat<U32, 5>(Controls().throttle, 0.f, 1.f);
	bikeNetworkControls.horn = Controls().horn;
	bikeNetworkControls.fullBrakes = CompressFloat<U32, 5>(Controls().fullBrakes, 0.f, 1.f);
	bikeNetworkControls.steer = CompressFloat<U32, 8>(Controls().steer, -1.f, 1.f);
	bikeNetworkControls.lean = CompressFloat<U32, 4>(Controls().lean, -1.f, 1.f);

	bikeNetworkControls.reverse = Controls().reverse;
	bikeNetworkControls.gearUp = Controls().gearUp;
	bikeNetworkControls.gearDown = Controls().gearDown;
	bikeNetworkControls.rearBrake = Controls().rearBrake > 0.5f;
	bikeNetworkControls.isWheel = Controls().isWheel;

	bikeNetworkControls.ghosted = 0;//ShouldTreatAsGhost();
	bikeNetworkControls.stopUpdating = 0;//!ShouldUpdate();
	bikeNetworkControls.riderPresent = RiderPresent();
}

void Motorbike::SetNetworkControls(const CompressedNetworkControls& carControlsIn)
{
	const BikeNetworkControls& bikeNetworkControls = (const BikeNetworkControls&) carControlsIn;

	//SetShouldUpdate(!bikeNetworkControls.stopUpdating);
	//SetShouldRender(!bikeNetworkControls.stopUpdating);
	//SetTreatAsGhost(bikeNetworkControls.ghosted);
	m_riderPresent = bikeNetworkControls.riderPresent;

	Controls().throttle = UncompressFloat<U32, 5>(bikeNetworkControls.throttle, 0.f, 1.f);
	Controls().horn = bikeNetworkControls.horn;
	Controls().fullBrakes = UncompressFloat<U32, 5>(bikeNetworkControls.fullBrakes, 0.f, 1.f);
	Controls().steer = UncompressFloat<U32, 8>(bikeNetworkControls.steer, -1.f, 1.f);
	Controls().lean = UncompressFloat<U32, 4>(bikeNetworkControls.lean, -1.f, 1.f);

	Controls().reverse = bikeNetworkControls.reverse;
	Controls().gearUp = bikeNetworkControls.gearUp;
	Controls().gearDown = bikeNetworkControls.gearDown;
	Controls().rearBrake = bikeNetworkControls.rearBrake ? 1.f : 0.f;
	Controls().isWheel = bikeNetworkControls.isWheel;
}

Debug::Float g_debugBikeNetworkJumpTolerance("RacingPhysics.ReplaySystem.BikeNetworkJumpTolerance", 0.1f, Debug::Range(0.0f, 0.5f), Debug::Step(0.001f));

bool Motorbike::NetworkJump(bool position, bool velocity)
{
	// unconstrained position is not safe to jump to: reset and update
	m_predictedNetworkState = m_networkState;
	UpdatePredictedNetworkState(true);

	bool jumped = true;
	Axis storedAxis = Body().GetAxis();
	m_collisionBody.SetAxis(m_predictedNetworkState.axis);

	/*if (!ShouldTreatAsGhost())
	{
		float tolerance = 0.0f;

		if (!ghostOnVehicleCollision)
			tolerance = g_debugBikeNetworkJumpTolerance;

		if (VehicleManager::Instance().CheckForCollision(VehicleIndex(), tolerance))
		{
			if (ghostOnVehicleCollision)
				SetTreatAsGhost(true);
			else
				jumped = false;
		}
	}*/

	/*if (checkWorldCollision)
	{
		UpdateCollisionCache(0.f);

		CollisionCache::Contact contact = CollisionObject().GetCollision(*CollisionCachePtr(), 0);
	
		if (contact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
		{
			if (contact.intersectionDistance > VehicleManager::Instance().MaxCollisionTolerance())
			{
				jumped = false;
			}
		}
	}*/

	if (jumped)
	{
		float eav = Engine().AngularVelocity1D();
		float wav0 = Wheel(0).AngularVelocity1D();
		float wav1 = Wheel(1).AngularVelocity1D();

		SetAxis(m_predictedNetworkState.axis, asin(m_predictedNetworkState.steeringAngle), 0.f, 0.f);

		ArticulatedBody::SetVelocity(m_predictedNetworkState.velocity);

		Body().SetAngularVelocity(m_predictedNetworkState.angularVelocity);
		Fork().SetAngularVelocity(m_predictedNetworkState.angularVelocity);
		Engine().Body().SetAngularVelocity(m_predictedNetworkState.angularVelocity - Body().GetAxis().AxisX() * eav);
		Wheel(0).Body().SetAngularVelocity(m_predictedNetworkState.angularVelocity - Fork().GetAxis().AxisX() * wav0);
		Wheel(1).Body().SetAngularVelocity(m_predictedNetworkState.angularVelocity - Body().GetAxis().AxisX() * wav1);

		Wheel(1).m_frame.slipScaleFactorValid = false; // invalidate so it will be recalculated
	}
	else
	{
		m_collisionBody.SetAxis(storedAxis);
	}

	return jumped;
}

Vehicle::CompressedNetworkState Motorbike::BlendStatesLinear(	const CompressedNetworkState& p_pState0, 
																const CompressedNetworkState& p_pState1, float p_Blend, float dt)
{
	Motorbike::NetworkState state0, state1, blendedState;

	UncompressNetworkState(p_pState0, state0);
	UncompressNetworkState(p_pState1, state1);

	blendedState.axis.SetPosition(Lerp(state0.axis.Position(), state1.axis.Position(), p_Blend)
									+ (state0.velocity - state1.velocity) * (0.5f * p_Blend * (1.f - p_Blend) * dt));

	blendedState.velocity = Lerp(state0.velocity, state1.velocity, p_Blend);

	blendedState.angularVelocity = Lerp(state0.angularVelocity, state1.angularVelocity, p_Blend);
	blendedState.framesSinceRaceStart = PhysicsTimer::Instance().FramesSinceRaceStart();
	blendedState.gear = p_Blend > 0.5f ? state1.gear : state0.gear;
	blendedState.steeringAngle = Maths::Lerp(state0.steeringAngle, state1.steeringAngle, p_Blend);

	{
		Quaternion gmq;
		Quaternion gmq0(state0.axis);
		Quaternion gmq1(state1.axis);
		
		Maths::C_Quaternion q0((float)gmq0.A().X(), (float)gmq0.A().Y(), (float)gmq0.A().Z(), (float)gmq0.W());
		Maths::C_Quaternion q1((float)gmq1.A().X(), (float)gmq1.A().Y(), (float)gmq1.A().Z(), (float)gmq1.W());
		Maths::C_Quaternion xmq = q0.Slerp(q1, p_Blend);

		//XMVECTOR q0 = XMVectorSet(gmq0.A().X(), gmq0.A().Y(), gmq0.A().Z(), gmq0.W());
		//XMVECTOR q1 = XMVectorSet(gmq1.A().X(), gmq1.A().Y(), gmq1.A().Z(), gmq1.W());
		//XMVECTOR xmq = XMQuaternionSlerp(q0, q1, p_Blend);

		gmq.SetComponents(xmq.Imag().X(), xmq.Imag().Y(), xmq.Imag().Z(), Maths::HF32(xmq.Real()));
		gmq.Normalise();
		blendedState.axis.SetRotation(gmq);
	}

	blendedState.networkStateValid = true;
	blendedState.useFullPrediction = true;
	Clear(&blendedState.acceleration);

	// not stored in compressed state
	blendedState.air = false;
	blendedState.groundNormal = Constants<Normal3R>::Y();

	RacingPhysics::Vehicle::CompressedNetworkState result;
	CompressNetworkState(blendedState, result);
	return result;
}

// debug
Point3R Motorbike::GetPositionFromState(const CompressedNetworkState& vehicleState)
{
	Motorbike::NetworkState state;
	UncompressNetworkState(vehicleState, state);
	return state.axis.Position();
}

// See if the bike needs to jump
void Motorbike::NetworkJumpIfNeeded()
{
	if (GetControlType() != RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK || !m_networkState.networkStateValid || !RiderPresent())
		return;

	float s_networkJumpDistance = 2.f;
	float s_networkJumpVelocity = 10.f;
	static float s_networkJumpDot = 0.5f;

	UpdatePredictedNetworkState(false);

	//return;

	Vector3R dp = m_predictedNetworkState.axis.Position() - Body().GetAxis().Position();
	Vector3R dv = m_predictedNetworkState.velocity - Body().Velocity();
	float dz = m_predictedNetworkState.axis.AxisZ() | Body().GetAxis().AxisZ();
	float dy = m_predictedNetworkState.axis.AxisY() | Body().GetAxis().AxisY();

	if (Magnitude(dp) > s_networkJumpDistance
		|| Magnitude(dv) > s_networkJumpVelocity
		|| dz < s_networkJumpDot || dy < s_networkJumpDot)
	{
		NetworkJump(true, true);
	}
}

void Motorbike::EvaluateNetworkForces(Derivative& derivative)
{
	if (GetControlType() != RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK || !m_networkState.networkStateValid || !RiderPresent())
		return;

	int framesSinceLastUpdate = PhysicsTimer::Instance().FramesSinceRaceStart() - m_networkState.framesSinceRaceStart;;
	float forceScale = 1.f;

	if (framesSinceLastUpdate > 6)
		forceScale = 6.f / (float) framesSinceLastUpdate;

	Vector3R predictedVelocity = m_predictedNetworkState.velocity + m_predictedNetworkState.acceleration * DerivativeTimeOffset();
	Point3R predictedPosition = m_predictedNetworkState.axis.Position() + predictedVelocity * DerivativeTimeOffset();

	Vector3R dp = predictedPosition - Body().GetAxis().Position();
	Vector3R dv = predictedVelocity - Body().Velocity();

	Axis correctedAxis = m_predictedNetworkState.axis.Rotated(Vector3R(m_predictedNetworkState.angularVelocity * DerivativeTimeOffset()));
	Matrix33R r = ~Body().GetAxis().Matrix3() * correctedAxis.Matrix3();
	Quaternion q((RotationMatrix&) r);
	q.Normalise();
	float phi = acos(q.W()) * 2.f;
	Vector3R dav = m_predictedNetworkState.angularVelocity - Body().AngularVelocity();

	if (Magnitude(dp) < 0.1f 
		&& Magnitude(m_predictedNetworkState.velocity) < 0.5f
		&& Magnitude(dv) < 0.5f
		&& Maths::Abs(phi) < 0.1f
		&& Magnitude(m_predictedNetworkState.angularVelocity) < 0.5f
		&& Magnitude(dav) < 0.5f)
		return;

	if (Magnitude(dp) > 0.01f)
	{
		float s_networkSpring = 5.f;
		float s_networkDamping = 1.f;

		Vector3R networkForce = dp * s_networkSpring + dv * s_networkDamping;

		float s_max = 1.f;
		float s = Magnitude(networkForce);
		if (s > s_max)
			networkForce *= s_max / s;

		// don't apply forces in y direction
		networkForce.Y(0.f);

		ApplyExternalForce(derivative, BodyIndex(), networkForce * Mass() * (1.f + DownforceFactor_Step()) * PhysicsConstants::GRAVITY * forceScale, Body().CofG());
	}

	if (phi > 0.01f)
	{
		Normal3R v = Normalised(q.A());
		Vector3R da = v * phi;

		float s_networkTorqueSpring = -5.f;
		float s_networkTorqueDamping = 1.f;

		Vector3R networkTorque = da * s_networkTorqueSpring + dav * s_networkTorqueDamping;

		static float s_max = 4.f;
		float s = Magnitude(networkTorque);
		if (s > s_max)
			networkTorque *= s_max / s;

		Vector3R t = networkTorque * Body().GetAxis().Global(Body().MI());

		ApplyExternalTorque(derivative, BodyIndex(), t * (1.f + DownforceFactor_Step()) * PhysicsConstants::GRAVITY * forceScale);
	}
}

void Motorbike::UpdatePredictedNetworkState(bool constrain)
{
	int framesSinceLastUpdate = PhysicsTimer::Instance().FramesSinceRaceStart() - m_predictedNetworkState.framesSinceRaceStart;

	float timeStep = PhysicsTimer::Instance().PhysicsUpdateTime();
	float total = framesSinceLastUpdate * timeStep;

	float speed = Magnitude(m_predictedNetworkState.velocity);
	float minSteps = (speed * total) / 0.2f;//VehicleManager::Instance().MaxCollisionTolerance();
	int nSteps = Max(int(minSteps + 0.5f), framesSinceLastUpdate);
	float t = total / nSteps;

	for (int i = 0; i < nSteps; i++)
	{
		// limit to prediction time
		if (m_predictedNetworkState.framesSinceRaceStart - m_networkState.framesSinceRaceStart > 30)
			break;

		float throttle = Throttle();
		float fullBrakes = FullBrakes();
		float rearBrake = RearBrake();
		float steer = SteerSmooth();
		float speed = -(m_predictedNetworkState.velocity | m_predictedNetworkState.axis.AxisZ());

		float a = PhysicsConstants::GRAVITY * (1.f + DownforceFactor());

		float maxForwardsGrip = 0.5f;
		float maxBackwardsGrip = 0.7f;
		float maxTransverseGrip = 0.7f;
		float slideFactor = 0.6f;

		if (!m_predictedNetworkState.air)
		{
			float powerEfficiency = 0.7f;

			// change velocity according to control input
			float engineTorque = Dynamics().engine.engineTorque_Rpm.Evaluate(Engine().RPM());
			float power = Engine().AngularVelocity1D() * engineTorque;
			float maxAcceleration = power * powerEfficiency / (Mass() * (Abs(speed) + 1.f));
			float forwardsAcceleration = maxAcceleration * throttle * throttle * (1.f - rearBrake);

			if (forwardsAcceleration > a * maxForwardsGrip)
			{
				forwardsAcceleration = a * maxForwardsGrip;
				a *= Maths::Lerp(slideFactor, 1.f, Maths::Clamped(speed / 10.f, 0.f, 1.f));
			}

			float transverseAcceleration = maxTransverseGrip * steer * a * Clamped(Abs(speed), 0.f, 1.f);

			Normal3R groundZ = Normalised(m_predictedNetworkState.axis.AxisX() ^ m_predictedNetworkState.groundNormal);
			Normal3R groundX = MakeNormal(m_predictedNetworkState.groundNormal ^ groundZ);

			Vector3R dv = (-groundZ * forwardsAcceleration
							- groundX * transverseAcceleration
							+ groundZ * maxBackwardsGrip * fullBrakes * a * Clamped(Abs(speed), 0.f, 1.f));

			float w = transverseAcceleration / (Abs(speed) + 1.f);
			float avy = m_predictedNetworkState.angularVelocity.Y();
			m_predictedNetworkState.angularVelocity.Y(Maths::Lerp(avy, w, 0.05f));

			// friction
			dv -= m_predictedNetworkState.velocity * 0.05f;

			float s = Magnitude(dv);
			if (s > a) dv = dv * (a / s);

			m_predictedNetworkState.acceleration = dv;

			RP_ASSERT(Magnitude(m_predictedNetworkState.acceleration) < 1000.f);
		}
		else
		{
			m_predictedNetworkState.acceleration = Constants<Normal3R>::NegativeY() * PhysicsConstants::GRAVITY;
		}

		m_predictedNetworkState.velocity += m_predictedNetworkState.acceleration * t;
		m_predictedNetworkState.axis.Translate(m_predictedNetworkState.velocity * t);
		m_predictedNetworkState.axis.Rotate(m_predictedNetworkState.angularVelocity * t);

		// constrain
		if (constrain)
		{
			RigidBody predicted = Body();
			predicted.SetAxis(m_predictedNetworkState.axis);
			predicted.SetVelocity(m_predictedNetworkState.velocity);
			predicted.SetAngularVelocity(m_predictedNetworkState.angularVelocity);
			//CollisionObject().ClearTransformState();
			//CollisionObject().TransformBoundingBox(m_predictedNetworkState.axis);

			//FloatVR v = Magnitude(predicted.PointVelocity(CollisionObject().TransformedCentre())) 
			//			+ Magnitude(predicted.AngularVelocity()) * CollisionObject().Radius();

			//FloatVR d = v * PhysicsTimer::Instance().PhysicsUpdateTime() + 0.5f;

			//Vector3R margin(d, d + 0.5f, d);
			
			//margin += CollisionObject().AABB();
			//Point3R min = CollisionObject().TransformedCentre() - margin;
			//Point3R max = CollisionObject().TransformedCentre() + margin;
			//UpdateCollisionCache(min, max);

			Axis forkAxis, frontWheelAxis, rearWheelAxis;
			CalculateAxes(m_predictedNetworkState.axis, m_predictedNetworkState.steeringAngle, 0.f, 0.f, forkAxis, frontWheelAxis, rearWheelAxis);

			m_predictedNetworkState.air = true;
			m_predictedNetworkState.groundNormal = Constants<Normal3R>::Y();
			FloatVR offset = 0.f;

			// move the predicted position above the ground
			for (int i = 0; i < 2; i++)
			{
				Point3R wheelPosition = (i == 0) ? frontWheelAxis.Position() : rearWheelAxis.Position();
				Normal3R wheelAxis = (i == 0) ? frontWheelAxis.AxisX() : rearWheelAxis.AxisX();

				CollisionCache::Contact contact;
				Wheel(i).GetGroundIntersection(contact, wheelPosition, wheelAxis);

				if (contact.collisionType != CollisionCache::COLLISION_TYPE_NONE)
				{
					offset = Max(offset, FloatVR(contact.intersectionDistance));
					m_predictedNetworkState.air = false;
					m_predictedNetworkState.groundNormal = contact.intersectionNormal;
					m_predictedNetworkState.velocity -= contact.intersectionNormal * (m_predictedNetworkState.velocity | contact.intersectionNormal);
				}
			}

			RP_ASSERT(offset < 0.5f);
			m_predictedNetworkState.axis.Translate(m_predictedNetworkState.groundNormal * (offset - 0.01f));
			m_predictedNetworkState.framesSinceRaceStart ++;

			//ConstrainNetworkPosition();

			int breakhere = 0;
		}
	}

	m_predictedNetworkState.framesSinceRaceStart = (int) PhysicsTimer::Instance().FramesSinceRaceStart();

	// restore
	//CollisionObject().SetAxis(&CollisionBody().GetAxis());
	//m_pCollisionCacheAuto = previousCache;
}

/*
void Motorbike::ConstrainNetworkPosition()
{
	RigidBody predicted = CollisionBody();
	predicted.SetAxis(m_predictedNetworkState.axis);
	predicted.SetVelocity(m_predictedNetworkState.velocity);
	predicted.SetAngularVelocity(m_predictedNetworkState.angularVelocity);

	CollisionObject().SetAxis(&predicted.GetAxis());

	while (true)
	{
		CollisionCache::Contact contact = CollisionObject().GetCollision(*CollisionCachePtr(), 0);
		
		if (contact.collisionType == CollisionCache::COLLISION_TYPE_NONE)
			break;
		
		float dmove = contact.intersectionDistance;

		if (dmove == 0.f || dmove > 0.5f)
		{
			RP_ASSERT(!"Collision error.");
			break;
		}

		dmove += COLLISION_EPSILON;	

		Point3R collisionPosition(contact.intersectionPoint);
		predicted.Push(collisionPosition, contact.intersectionNormal * dmove);
		float impulse = predicted.CalculateImpulse(collisionPosition, contact.intersectionNormal, 0.f, 0.f);

		if (impulse > 0.f)
			predicted.ApplyImpulse(collisionPosition, contact.intersectionNormal * impulse, 0.f);
	}

	m_predictedNetworkState.axis = predicted.GetAxis();
	m_predictedNetworkState.velocity = predicted.Velocity();
	m_predictedNetworkState.angularVelocity = predicted.AngularVelocity();

	CollisionObject().SetAxis(&CollisionBody().GetAxis());
}
*/

}



