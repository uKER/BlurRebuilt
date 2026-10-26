#include "stdafx.h"
#include "racingphysics/vehicles/car/car.h"

#include "racingphysics/physics/quaternion.h"
#include "racingphysics/physics/compressfloat.h"
#include "racingphysics/physics/physicstimer.h"

#include "gravity/sdk/CollisionPhantom.h"
#include "gravity/sharedphysics/ContactStream.h"
#include "Gravity/SharedPhysics/ContactStreamIterator.h"
#include <system/debug/I_DebugRender.h>
#include <gravity/sdk/ShapeDescriptors.h>

//using namespace Maths;

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize("", off) // DEBUG_RACING_PHYSICS
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace RacingPhysics
{

EXTERN_DEBUG_FLOAT( g_timeStepScale );
//Debug::Float g_debugCarNetworkJumpTolerance("RacingPhysics.ReplaySystem.CarNetworkJumpTolerance", 0.1f, Debug::Range(0.0f, 0.5f), Debug::Step(0.001f));
Debug::Bool g_debugBlendUseShortestSlerp("RacingPhysics.Cars.Network.UseShortestSlerp", true);
Debug::Bool g_debugBlendPatchAroundDeath("RacingPhysics.Cars.Network.PatchAroundDeath", true);
Debug::Bool g_debugCarNetworkJumpToPredicted("RacingPhysics.Cars.Network.CarNetworkJumpToPredicted", true);
Debug::Bool g_runCatchup("RacingPhysics.Cars.Network.RunCatchUp", true);
Debug::Bool g_resetRoundTrip("RacingPhysics.Cars.Network.ResetRoundTrip", false);

void Car::NetworkInitialise()
{
	//m_networkState.networkStateValid = false;
	//m_networkState.axis.SetPosition(Constants<Point3R>::Origin());
	memset(&m_networkState, 0, sizeof(m_networkState));

	m_dd.predictedNetworkState = m_networkState;
	m_dd.predictedNetworkStateJump = m_networkState;

	if (m_dd.phantomCollisionGroup == Scheduler::JQ_GROUP_NONE)
		m_dd.phantomCollisionGroup = Scheduler::CreateGroup();

	m_networkTiming.framesPredictedLocally = 255;
	m_networkTiming.framesPredictedRemotely = 255;
	m_networkTiming.bestRoundTrip = 255;
}

struct CarCompressedNetworkState // 24 bytes
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

struct CarNetworkControls // 4 bytes
{
	U32 throttle : 8;
	U32 brake : 8;
	U32 steer : 8;

	U32 horn : 1;
	U32 gearUp : 1;
	U32 gearDown : 1;
	U32 handbrake : 1;
	U32 reverse : 1;
	U32 isWheel : 1;
	U32 ghosted : 1;
	U32 stopUpdating : 1;
};

BIZ_STATIC_ASSERT(sizeof(Vehicle::CompressedNetworkControls) == sizeof(CarNetworkControls));
BIZ_STATIC_ASSERT(sizeof(Vehicle::CompressedNetworkState) == sizeof(CarCompressedNetworkState) + 1);

void Car::GetNetworkState(NetworkState& networkState) const
{
	networkState.velocity = Body().Velocity();
	networkState.angularVelocity = Body().AngularVelocity();
	networkState.gear = Transmission().Gear();
	networkState.framesSinceRaceStart = PhysicsTimer::Instance().FramesSinceRaceStart();
	networkState.framesPredicted = 0;
	networkState.networkStateValid = true;
	networkState.useFullPrediction = true;
	networkState.air = Air();
	networkState.groundNormal = !Air() ? SmoothedGroundNormal() : Constants<Normal3R>::Y();
	networkState.axis = GetAxis();
	networkState.steeringAngle = Wheel(0).CalculateSteeringAngle();
	Clear(&networkState.acceleration);
	RP_ASSERT(Magnitude(networkState.velocity) < 10000.f);
}

void Car::GetNetworkState(Vehicle::CompressedNetworkState* pVehicleState, const Vehicle* pTarget) const
{
	NetworkState networkState;
	GetNetworkState(networkState);
	CompressNetworkState(networkState, *pVehicleState);

	// Store in the network packet the number of frames that were predicted for the 
	// car corresponding to the packet destination.
	if (pTarget)
	{
		S32 n = pTarget->GetNetworkPredictionFrameCount();
		n = Maths::Clamped(n, 0, 255);
		pVehicleState->data[24] = n;
	}
	else
	{
		pVehicleState->data[24] = 255;
	}
}

S32 Car::GetNetworkPredictionFrameCount() const
{
	return m_networkTiming.framesPredictedLocally;
}

void Car::CompressNetworkState(const NetworkState& networkState, CompressedNetworkState& vehicleState)
{
	CarCompressedNetworkState* state = (CarCompressedNetworkState*) &vehicleState;

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

	state->vx = CompressFloat<U32, 12>(Clamped((float)networkState.velocity.X(), -200.f, 200.f), -200.f, 200.f);
	state->vy = CompressFloat<U32, 13>(Clamped((float)networkState.velocity.Y(), -100.f, 100.f), -100.f, 100.f);
	state->vz = CompressFloat<U32, 12>(Clamped((float)networkState.velocity.Z(), -200.f, 200.f), -200.f, 200.f);

	// clamp these: only happens in collisions when prediction is not critical
	state->avy = CompressFloat<U32, 12>(Clamped((float)networkState.angularVelocity.Y(), -50.f, 50.f), -50.f, 50.f);
	state->avx = CompressFloat<U32, 8>(Clamped((float)networkState.angularVelocity.X(), -10.f, 10.f), -10.f, 10.f);
	state->avz = CompressFloat<U32, 8>(Clamped((float)networkState.angularVelocity.Z(), -10.f, 10.f), -10.f, 10.f);

	state->sa = CompressFloat<U32, 8>(networkState.steeringAngle, -2.f, 2.f);

	state->framesSinceRaceStart = (U16) networkState.framesSinceRaceStart;
	state->gear = (U32 (networkState.gear + 1));
}

void Car::UncompressNetworkState(const CompressedNetworkState& vehicleState, NetworkState& networkState)
{
	const CarCompressedNetworkState& state = (const CarCompressedNetworkState&) vehicleState;

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

	networkState.steeringAngle = UncompressFloat<U32, 8>(state.sa, -2.f, 2.f);

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

bool Car::ProcessNetworkState(const CompressedNetworkState& vehicleState, bool runCatchUp)
{
#if !PLATFORM_SPU

	NetworkState networkState;
	UncompressNetworkState(vehicleState, networkState);

	if (networkState.framesSinceRaceStart < m_networkState.framesSinceRaceStart)
		return true;

	m_networkState = networkState;

	RP_ASSERT(m_networkState.axis.DebugTest());

	// Update the local timer if the state is newer.
	if (runCatchUp && g_runCatchup)
	{
		// This is the number of frames that were predicted for this car on the remote machine.
		m_networkTiming.framesPredictedRemotely = vehicleState.data[24];

		int catchUp = ((m_networkState.framesSinceRaceStart - 1) - PhysicsTimer::Instance().FramesSinceRaceStart());

		if (catchUp > 1)
		{
			PhysicsTimer::Instance().NetworkCatchUp(catchUp);
		}
		else
		{
			if (m_networkTiming.framesPredictedRemotely < 255)
			{
				if (g_resetRoundTrip)
				{
					g_resetRoundTrip = false;
					m_networkTiming.bestRoundTrip = 255;
				}

				S32 roundTrip = m_networkTiming.framesPredictedRemotely + m_networkTiming.framesPredictedLocally;

				if (roundTrip < m_networkTiming.bestRoundTrip)
				{
					S32 catchUp = (m_networkTiming.framesPredictedRemotely - m_networkTiming.framesPredictedLocally) / 2;

					if (catchUp > 0)
					{
						PhysicsTimer::Instance().NetworkCatchUp(catchUp);
						LOGDEBUG(g_RacingPhysicsLog, "Catch up %i", catchUp);
					}

					m_networkTiming.bestRoundTrip = roundTrip;
				}
			}
		}

		// Number of predicted frames, to be sent back to the machine that sent this packet.
		m_networkTiming.framesPredictedLocally = PhysicsTimer::Instance().FramesSinceRaceStart() - m_networkState.framesSinceRaceStart + 1;

		S32 real = PhysicsTimer::Instance().GetFramesSinceRaceStartReal();

		LOGDEBUG(g_RacingPhysicsLog, "frame %i (l), %i (r), %i (real), predicted %i (l), %i (r), best round trip %i", 
			PhysicsTimer::Instance().FramesSinceRaceStart(), m_networkState.framesSinceRaceStart, real,
			m_networkTiming.framesPredictedLocally, m_networkTiming.framesPredictedRemotely, 
			m_networkTiming.bestRoundTrip);
	}

	// see if the predicted state is up-to-date enough to accept a new one.
	{
		int frames = PhysicsTimer::Instance().FramesSinceRaceStart() - m_dd.predictedNetworkState.framesSinceRaceStart;

		if (frames <= 30 || m_networkState.framesSinceRaceStart > m_dd.predictedNetworkState.framesSinceRaceStart || !m_dd.predictedNetworkState.networkStateValid)
		{
			m_dd.predictedNetworkState = m_networkState;
			m_dd.predictedNetworkState.framesPredicted = 0;
		}
	}

	// jump state
	{
		int frames = PhysicsTimer::Instance().FramesSinceRaceStart() - m_dd.predictedNetworkStateJump.framesSinceRaceStart;

		if (frames <= 1 || m_networkState.framesSinceRaceStart > m_dd.predictedNetworkStateJump.framesSinceRaceStart || !m_dd.predictedNetworkStateJump.networkStateValid)
		{
			m_dd.predictedNetworkStateJump = m_networkState;
			m_dd.predictedNetworkStateJump.framesPredicted = 0;
		}
	}

	return true;

#else

	RP_ASSERT(0);
	return false;

#endif
}

void Car::GetNetworkControls(CompressedNetworkControls& controlsOut) const
{
	CarNetworkControls& networkControls = (CarNetworkControls&) controlsOut;

	networkControls.throttle = CompressFloat<U32, 8>(Controls().throttle, 0.f, 1.f);
	networkControls.brake = CompressFloat<U32, 8>(Controls().brake, 0.f, 1.f);
	networkControls.steer = CompressFloat<U32, 8>(Controls().steer, -1.f, 1.f);

	networkControls.horn = Controls().horn;
	networkControls.reverse = Controls().reverse;
	networkControls.gearUp = Controls().gearUp;
	networkControls.gearDown = Controls().gearDown;
	networkControls.handbrake = Controls().handbrake > 0.5f;
	networkControls.isWheel = Controls().isWheel;

	networkControls.ghosted = 0;//ShouldTreatAsGhost();
	networkControls.stopUpdating = 0;//!ShouldUpdate();
}

void Car::SetNetworkControls(const CompressedNetworkControls& carControlsIn)
{
	const CarNetworkControls& networkControls = (const CarNetworkControls&) carControlsIn;

	//SetShouldUpdate(!networkControls.stopUpdating);
	//SetShouldRender(!networkControls.stopUpdating);
	//SetTreatAsGhost(networkControls.ghosted);

	Controls().throttle = UncompressFloat<U32, 8>(networkControls.throttle, 0.f, 1.f);
	Controls().horn = networkControls.horn;
	Controls().brake = UncompressFloat<U32, 8>(networkControls.brake, 0.f, 1.f);
	Controls().steer = UncompressFloat<U32, 8>(networkControls.steer, -1.f, 1.f);

	Controls().reverse = networkControls.reverse;
	Controls().gearUp = networkControls.gearUp;
	Controls().gearDown = networkControls.gearDown;
	Controls().handbrake = networkControls.handbrake ? 1.f : 0.f;
	Controls().isWheel = networkControls.isWheel;
}

void Car::EnsureReducedBrakeValue()			// Fix for revs in cutscenes
{
	if (Controls().brake > 0.05f)
		Controls().brake = 0.05f;
}

void Car::JumpToNetworkState(const Axis& axis, const Vector3R& velocity, const Vector3R& angularVelocity)
{
	RP_ASSERT(Validate());

	Vector3R eav = Body().GetAxis().Local(Engine().GetRotor().AngularVelocity() - Body().AngularVelocity());
	Vector3R wav0 = Body().GetAxis().Local(Wheel(0).GetRotor().AngularVelocity() - Body().AngularVelocity());
	Vector3R wav1 = Body().GetAxis().Local(Wheel(1).GetRotor().AngularVelocity() - Body().AngularVelocity());
	Vector3R wav2 = Body().GetAxis().Local(Wheel(2).GetRotor().AngularVelocity() - Body().AngularVelocity());
	Vector3R wav3 = Body().GetAxis().Local(Wheel(3).GetRotor().AngularVelocity() - Body().AngularVelocity());

	
	Matrix33R wm0 = Body().GetAxis().Local(Wheel(0).GetRotor().GetRotationMatrix().Matrix3());
	Matrix33R wm1 = Body().GetAxis().Local(Wheel(1).GetRotor().GetRotationMatrix().Matrix3());
	Matrix33R wm2 = Body().GetAxis().Local(Wheel(2).GetRotor().GetRotationMatrix().Matrix3());
	Matrix33R wm3 = Body().GetAxis().Local(Wheel(3).GetRotor().GetRotationMatrix().Matrix3());

	Body().SetAxis(axis);
	m_engine.SetCarAxis(axis);
	m_dd.oldAxis = Body().GetAxis();

	Wheel_NC(0).GetRotor().SetRotation(RotationMatrix(Body().GetAxis().Global(wm0)));
	Wheel_NC(1).GetRotor().SetRotation(RotationMatrix(Body().GetAxis().Global(wm1)));
	Wheel_NC(2).GetRotor().SetRotation(RotationMatrix(Body().GetAxis().Global(wm2)));
	Wheel_NC(3).GetRotor().SetRotation(RotationMatrix(Body().GetAxis().Global(wm3)));
	
	ArticulatedBody::SetVelocity(velocity);
	Body().SetAngularVelocity(angularVelocity);

	Engine_NC().GetRotor().SetAngularVelocity(Body().GetAxis().Global(eav) + Body().AngularVelocity());
	Wheel_NC(0).GetRotor().SetAngularVelocity(Body().GetAxis().Global(wav0) + Body().AngularVelocity());
	Wheel_NC(1).GetRotor().SetAngularVelocity(Body().GetAxis().Global(wav1) + Body().AngularVelocity());
	Wheel_NC(2).GetRotor().SetAngularVelocity(Body().GetAxis().Global(wav2) + Body().AngularVelocity());
	Wheel_NC(3).GetRotor().SetAngularVelocity(Body().GetAxis().Global(wav3) + Body().AngularVelocity());

	RP_ASSERT(Validate());
}

Vehicle::CompressedNetworkState Car::BlendStatesLinear(	const CompressedNetworkState& p_pState0, 
																const CompressedNetworkState& p_pState1, float p_Blend, float dt)
{
	Car::NetworkState state0, state1, blendedState;

	UncompressNetworkState(p_pState0, state0);
	UncompressNetworkState(p_pState1, state1);

	blendedState.axis.SetPosition(Lerp(state0.axis.Position(), state1.axis.Position(), p_Blend)
									+ (state0.velocity - state1.velocity) * (0.5f * p_Blend * (1.f - p_Blend) * dt));

	blendedState.velocity = Lerp(state0.velocity, state1.velocity, p_Blend);

	blendedState.angularVelocity = Lerp(state0.angularVelocity, state1.angularVelocity, p_Blend);
	blendedState.framesSinceRaceStart = PhysicsTimer::Instance().FramesSinceRaceStart();
	blendedState.gear = p_Blend > 0.5f ? state1.gear : state0.gear;
	blendedState.steeringAngle = Maths::Lerp(state0.steeringAngle, state1.steeringAngle, p_Blend);

	//{
		Quaternion gmq;
		Quaternion gmq0(state0.axis);
		Quaternion gmq1(state1.axis);
		
		Maths::C_Quaternion q0((float)gmq0.A().X(), (float)gmq0.A().Y(), (float)gmq0.A().Z(), (float)gmq0.W());
		Maths::C_Quaternion q1((float)gmq1.A().X(), (float)gmq1.A().Y(), (float)gmq1.A().Z(), (float)gmq1.W());
		
		Maths::C_Quaternion xmq;

		if (p_Blend == 0.0f)
		{
			gmq.SetComponents(q0.Imag().X(), q0.Imag().Y(), q0.Imag().Z(), Maths::HF32(q0.Real()));
		}
		else
		{
			if (g_debugBlendUseShortestSlerp)
				Maths::C_Quaternion xmq = q0.ShortestSlerp(q1, p_Blend);
			else
				Maths::C_Quaternion xmq = q0.Slerp(q1, p_Blend);

			gmq.SetComponents(xmq.Imag().X(), xmq.Imag().Y(), xmq.Imag().Z(), Maths::HF32(xmq.Real()));
			
			if (g_debugBlendPatchAroundDeath)
			{
				//[ChadW] HACKY HACK HACK - wierd quaternion issues... Patch around this for replays && amax prototype
				if (gmq.A().X() == 0.0f && gmq.A().Y() == 0.0f && gmq.A().Z() == 0.0f)
				{
					if (p_Blend < 0.5f)
						gmq.SetComponents(q0.Imag().X(), q0.Imag().Y(), q0.Imag().Z(), Maths::HF32(q0.Real()));
					else
						gmq.SetComponents(q1.Imag().X(), q1.Imag().Y(), q1.Imag().Z(), Maths::HF32(q1.Real()));
				}
			}
		}

		gmq.Normalise();
		blendedState.axis.SetRotation(gmq);
	//}

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
Point3R Car::GetPositionFromState(const CompressedNetworkState& vehicleState)
{
	Car::NetworkState state;
	UncompressNetworkState(vehicleState, state);
	return state.axis.Position();
}

bool Car::CheckNetworkJumped()
{
	if (m_dd.bNetworkJumpedThisFrame)
	{
		m_dd.bNetworkJumpedThisFrame = false;
		return true;
	}
	return false;
}

} // namespace RacingPhysics
