#include "stdafx.h"
#include "System\Scheduler.h"
#include "racingphysics\vehicles\vehicle.h"
#include "racingphysics\vehicles\car\car.h"
#include "racingphysics\physics\physicstimer.h"

#include "gravity/sdk/CollisionPhantom.h"
#include "gravity/sharedphysics/ContactStream.h"
#include "Gravity/SharedPhysics/ContactStreamIterator.h"
#include <system/debug/I_DebugRender.h>
#include <gravity/sdk/ShapeDescriptors.h>

#ifdef PLATFORM_SPU
#include "core/maths/hvec/VectorConstGlobals.inl"
#include "../../../../../common/core/source/maths/hvec/Trigonometry.cpp.inl"
#include "../../../../../common/core/source/maths/hvec/Constants.cpp.inl"
#endif

extern Scheduler::jqModule NetworkUpdateModule;

namespace RacingPhysics
{
	DECLARE_DEBUG_FLOAT(g_debugCarNetworkJumpDistance, "RacingPhysics.Cars.Network.CarNetworkJumpDistance", 2.5f, Debug::Range(0.0f, 50.0f), Debug::Step(0.001f));
	DECLARE_DEBUG_FLOAT(g_debugCarNetworkJumpVelocity, "RacingPhysics.Cars.Network.CarNetworkJumpVelocity", 10.0f, Debug::Range(0.0f, 50.0f), Debug::Step(0.001f));

	EXTERN_DEBUG_FLOAT(g_timeStepScale);

	// See if the Car needs to jump
	void Car::NetworkJumpIfNeeded()
	{
		if (GetControlType() != RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK || !m_dd.predictedNetworkState.networkStateValid)
			return;

		PERF_EVENT("Car.BeginNetworkUpdate");

		float s_networkJumpDistance = g_debugCarNetworkJumpDistance;
		float s_networkJumpVelocity = g_debugCarNetworkJumpVelocity;
		static float s_networkJumpDot = 0.5f;

		UpdatePredictedNetworkState(m_dd.predictedNetworkState, false);
		UpdatePredictedNetworkState(m_dd.predictedNetworkStateJump, true);

		if (m_dd.noNetworkJumpCounter > 0)
		{
			m_dd.noNetworkJumpCounter--;
		}
		else if (m_dd.predictedNetworkStateJump.framesSinceRaceStart == PhysicsTimer::Instance().FramesSinceRaceStart() - 1)
		{
			Vector3R v = Body().Velocity();
			Point3R p = Body().GetAxis().Position();
			Vector3R dp = m_dd.predictedNetworkStateJump.axis.Position() - p;
			Vector3R dv = m_dd.predictedNetworkStateJump.velocity - v;
			float dz = m_dd.predictedNetworkStateJump.axis.AxisZ() | Body().GetAxis().AxisZ();
			float dy = m_dd.predictedNetworkStateJump.axis.AxisY() | Body().GetAxis().AxisY();

			float s = Magnitude(v);
			Vector3R vn = v / (s + 1.f);
			float dl = dp | vn;
			float dt = Magnitude(dp - dl * vn);

			bool jumpPosition = false;
			bool jumpVelocity = false;

			if (Maths::Abs(dt) > s_networkJumpDistance
				|| Maths::Abs(dl) > (s_networkJumpDistance + s / 20.f)
				|| dz < s_networkJumpDot || dy < s_networkJumpDot)
			{
				jumpPosition = true;
				m_dd.bNetworkJumpedThisFrame = true;
			}

			//if (Magnitude(dv) > s_networkJumpVelocity)
			//	jumpVelocity = true;

			if (jumpPosition)// || jumpVelocity)
				NetworkJump(jumpPosition, jumpPosition);
		}
	}

	bool Car::NetworkJump(bool position, bool velocity)
	{
		//if (!g_debugCarNetworkJumpToPredicted)
		//	m_dd.predictedNetworkStateJump = m_networkState;

		JumpToNetworkState(m_dd.predictedNetworkStateJump, position, velocity);

		m_dd.predictedNetworkState = m_dd.predictedNetworkStateJump;

		RP_ASSERT(Validate());

		return true;
	}

	void Car::JumpToNetworkState(const NetworkState& networkState, bool position, bool velocity)
	{
		RP_ASSERT(Validate());

		Vector3R eav = Body().GetAxis().Local(Engine().GetRotor().AngularVelocity() - Body().AngularVelocity());
		Vector3R wav0 = Body().GetAxis().Local(Wheel(0).GetRotor().AngularVelocity() - Body().AngularVelocity());
		Vector3R wav1 = Body().GetAxis().Local(Wheel(1).GetRotor().AngularVelocity() - Body().AngularVelocity());
		Vector3R wav2 = Body().GetAxis().Local(Wheel(2).GetRotor().AngularVelocity() - Body().AngularVelocity());
		Vector3R wav3 = Body().GetAxis().Local(Wheel(3).GetRotor().AngularVelocity() - Body().AngularVelocity());

		if (position)
		{
			Matrix33R wm0 = Body().GetAxis().Local(Wheel(0).GetRotor().GetRotationMatrix().Matrix3());
			Matrix33R wm1 = Body().GetAxis().Local(Wheel(1).GetRotor().GetRotationMatrix().Matrix3());
			Matrix33R wm2 = Body().GetAxis().Local(Wheel(2).GetRotor().GetRotationMatrix().Matrix3());
			Matrix33R wm3 = Body().GetAxis().Local(Wheel(3).GetRotor().GetRotationMatrix().Matrix3());

			Body().SetAxis(networkState.axis);
			m_engine.SetCarAxis(networkState.axis);
			m_dd.oldAxis = Body().GetAxis();

			Wheel_NC(0).GetRotor().SetRotation(RotationMatrix(Body().GetAxis().Global(wm0)));
			Wheel_NC(1).GetRotor().SetRotation(RotationMatrix(Body().GetAxis().Global(wm1)));
			Wheel_NC(2).GetRotor().SetRotation(RotationMatrix(Body().GetAxis().Global(wm2)));
			Wheel_NC(3).GetRotor().SetRotation(RotationMatrix(Body().GetAxis().Global(wm3)));
		}

		if(velocity)
		{
			ArticulatedBody::SetVelocity(networkState.velocity);
			Body().SetAngularVelocity(networkState.angularVelocity);
		}

		Engine_NC().GetRotor().SetAngularVelocity(Body().GetAxis().Global(eav) + Body().AngularVelocity());
		Wheel_NC(0).GetRotor().SetAngularVelocity(Body().GetAxis().Global(wav0) + Body().AngularVelocity());
		Wheel_NC(1).GetRotor().SetAngularVelocity(Body().GetAxis().Global(wav1) + Body().AngularVelocity());
		Wheel_NC(2).GetRotor().SetAngularVelocity(Body().GetAxis().Global(wav2) + Body().AngularVelocity());
		Wheel_NC(3).GetRotor().SetAngularVelocity(Body().GetAxis().Global(wav3) + Body().AngularVelocity());

		RP_ASSERT(Validate());
	}

	void Car::UpdatePredictedNetworkState(NetworkState& predictedNetworkState, bool constrain)
	{
		Spu::MemBaseMarker marker;
		PERF_EVENT("Car.UpdatePredictedNetworkState");

		if (!predictedNetworkState.networkStateValid)
			return;

		int framesToUpdate = PhysicsTimer::Instance().FramesSinceRaceStart() - predictedNetworkState.framesSinceRaceStart - 1;

		if (framesToUpdate <= 0)
			return;

		if (constrain)
			framesToUpdate = Min(framesToUpdate, 2);
		else
			framesToUpdate = Min(framesToUpdate, 30);

		float timeStep = PhysicsTimer::Instance().PhysicsUpdateTime() * g_timeStepScale;
		float total = framesToUpdate * timeStep;

		float speed = Magnitude(predictedNetworkState.velocity);
		float minSteps = (speed * total) / 0.5f;//VehicleManager::Instance().MaxCollisionTolerance();

		int nSteps = (int) minSteps + 1;//Max(int(minSteps + 0.5f), framesSinceLastUpdate);

		nSteps = Maths::Clamp(nSteps, 1, 1000);
		float t = total / nSteps;

		//Gravity::C_CollisionPhantom phantom(m_pNetworkCollisionShape);
		const U32 DataSize = 8192;
#ifndef PLATFORM_SPU
		ALIGN16 U8 data[DataSize + sizeof(SharedPhysics::C_ContactStreamResult)];
#else
		U8* data = (U8*)Spu::Alloc(DataSize + ((sizeof(SharedPhysics::C_ContactStreamResult) + 15) & 0xfffffff0));
#endif
		SharedPhysics::C_ContactStream stream(&data[0], DataSize);
		//SharedPhysics::C_ContactStream* stream = new SharedPhysics::C_ContactStream(8192);

		Gravity::C_CollisionPhantom* pPhantom = (Gravity::C_CollisionPhantom*)Spu::FetchAlloc(m_dd.pPhantom, sizeof(Gravity::C_CollisionPhantom));
		Gravity::C_Scene* pPpuScene = GetGravityScene();
		Gravity::C_Scene* pScene = (Gravity::C_Scene*)Spu::FetchAlloc(pPpuScene, sizeof(Gravity::C_Scene));

		for (int i = 0; i < nSteps; i++)
		{
			float throttle = Throttle();
			float brake = Brake();
			float handbrake = Handbrake();
			float steer = Steer();
			float speedZ = -(predictedNetworkState.velocity | predictedNetworkState.axis.AxisZ());
			float speedX = (predictedNetworkState.velocity | predictedNetworkState.axis.AxisX());

			float a = PhysicsConstants::GRAVITY * (1.f + CalculateDownforceFactor());

			float maxForwardsGrip = 0.5f;
			float maxBackwardsGrip = (Wheel(0).BackwardsGripScaleDry() + Wheel(1).BackwardsGripScaleDry()) * 0.4f;
			float maxTransverseGrip = (Wheel(0).TransverseGripScaleDry() + Wheel(1).TransverseGripScaleDry()) * 0.45f;
			float slideFactor = 0.6f;

			if (!predictedNetworkState.air)
			{
				float powerEfficiency = 0.7f;

				// change velocity according to control input
				float engineTorque = Dynamics().engine.engineTorque_Rpm.Evaluate(Engine().RPM()) * PerformanceModifiers().peakTorqueScale;
				float power = Engine().AngularVelocity1D() * engineTorque;
				float maxAcceleration = power * powerEfficiency / (Mass() * (Abs(speed) + 1.f));
				float forwardsAcceleration = maxAcceleration * throttle * throttle * (1.f - handbrake);

				if (forwardsAcceleration > a * maxForwardsGrip)
				{
					forwardsAcceleration = a * maxForwardsGrip;
					a *= Maths::Lerp(slideFactor, 1.f, Maths::Clamped(speed / 10.f, 0.f, 1.f));
				}

				float transverseAcceleration = maxTransverseGrip * Clamped(steer - speedX * 10.f / (Abs(speedZ) + 1.f), -1.f, 1.f)
					* a * Clamped(Abs(speed), 0.f, 1.f);

				Normal3R groundZ = VEHICLE_REVERSE_Z_SIGN Normalised(predictedNetworkState.axis.AxisX() ^ predictedNetworkState.groundNormal);
				Normal3R groundX = MakeNormal(predictedNetworkState.groundNormal ^ groundZ);

				Vector3R dv = (-groundZ * forwardsAcceleration
					- groundX * transverseAcceleration
					+ groundZ * maxBackwardsGrip * brake * a * Clamped(Abs(speed), 0.f, 1.f));

				float w = transverseAcceleration / (Abs(speed) + 1.f);
				float avy = predictedNetworkState.angularVelocity.Y();
				predictedNetworkState.angularVelocity.Y(Maths::Lerp(avy, w, 0.05f));

				// friction
				dv -= predictedNetworkState.velocity * 0.05f;

				float s = Magnitude(dv);
				if (s > a) dv = dv * (a / s);

				predictedNetworkState.acceleration = dv;

				RP_ASSERT(Magnitude(predictedNetworkState.acceleration) < 1000.f);
			}
			else
			{
				predictedNetworkState.acceleration = Constants<Normal3R>::NegativeY() * PhysicsConstants::GRAVITY * 1.5f;
			}

			predictedNetworkState.velocity += predictedNetworkState.acceleration * t;
			predictedNetworkState.axis.Translate(predictedNetworkState.velocity * t);
			predictedNetworkState.axis.Rotate(predictedNetworkState.angularVelocity * t);

			// constrain
			if (constrain)
			{
				// We know we won' t fall through the ground so take the current state from the car.
				//predictedNetworkState.air = Air();

				//Update the collision phantom and do collision detection with the world
				{
					Maths::HMat33 matrix(predictedNetworkState.axis.Matrix3());
					const Maths::HVec3 position(predictedNetworkState.axis.Position());
					const Maths::HVec3 translation(position + (pPhantom->GetOffsetToCentre()) * matrix);
					pPhantom->Transform(Maths::HMat43(matrix, translation));
					stream.Reset();

					//pScene->GetCollisionPhantomIntersectionMesh(m_dd.pPhantom, *stream, pScene->GetInternalTaskPolicy(), m_dd.phantomCollisionGroup);
					//pScene->CompletePhantomCollisionQuery(*stream, pScene->GetInternalTaskPolicy(), m_dd.phantomCollisionGroup);

					pScene->GetCollisionPhantomIntersectionMeshImmediate(pPhantom, stream);
					SharedPhysics::C_ContactStreamIterator iter(&stream, 0, 0.f);
					//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-m_dd.pNetworkCollisionShape->m_LocalBoundingBox.extents, 
					//	m_dd.pNetworkCollisionShape->m_LocalBoundingBox.extents, m_dd.pNetworkCollisionShape->m_PosRot, Maths::RGBA(0, 255, 0));

					while(iter.hasNextContact())
					{
						SharedPhysics::C_ContactStreamResult* pResult = iter.nextContact();
						while(iter.hasNextPatch())
						{
							SharedPhysics::C_ContactStreamPatch* pPatch = iter.nextPatch();
							float maxIntersection = 0.f;

							while(iter.hasNextPoint())
							{
								SharedPhysics::C_ContactStreamPoint* pPoint = iter.nextPoint();
								//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(Maths::Load<Maths::HVec3>(&pPoint->m_ContactPoint),
								//	0.15f, Maths::RGBA(255, 0, 0));
								maxIntersection = Maths::Max(maxIntersection, pPoint->m_PenetrationDepth);
							}

							Vector3R n = Maths::C_Vector(Maths::Load<Maths::HVec3>(&pPatch->m_Normal));
							Vector3R v = n * -maxIntersection;
							predictedNetworkState.axis.Translate(v);
							predictedNetworkState.velocity -= Maths::Max(predictedNetworkState.velocity | n, 0.f) * n * 0.1f;
						}
					}
				}
			}
		}


		//delete stream;
		stream.m_pStream = NULL;
		predictedNetworkState.framesSinceRaceStart += framesToUpdate;
		predictedNetworkState.framesPredicted += framesToUpdate;
	}

#ifdef SPU_NET_TASK

	void Car::SetAxis(const Axis& a, bool overwriteOldAxis) {assert(0);}
	void Car::SetPosition(const Point3R& p) {assert(0);}
	void Car::SetHold() {assert(0);}
	float Car::ConstrainInternal() {assert(0); return 0.f;}
	void Car::Integrate(float time) {assert(0);}
	bool Car::WheelOnGround(int i) const {assert(0); return true;}
	void Car::GetWheelContact(int i, Point3R& position, Normal3R& normal) const {assert(0);}
	void Car::ClearAllFeedback() {assert(0);}
	void Car::HandleCollision(const Point3R& p, const Vector3R& impulse) {assert(0);}
	void Car::CreateGravityCache() {assert(0);}
	void Car::EvaluateDerivative(Derivative& derivative) {assert(0);}
	void Car::BeginIntegrate() {assert(0);}
	void Car::EndIntegrate() {assert(0);}
	void Car::GetIntegrationType(int& integrationType, int& nSteps) {assert(0);}

#endif
}

namespace RacingPhysics_NetworkUpdate
{
	RacingPhysics::Vehicle* InitialiseVehicle(RacingPhysics::Vehicle* pVehicle, void* dmaData)
	{
#if !PLATFORM_SPU
		return pVehicle;
#else
		static RacingPhysics::Car car;
		car.SpuFetch(dmaData);
		return &car;
#endif
	}

	void FinaliseVehicle(const RacingPhysics::Vehicle* pLocal, void* dmaData)
	{ 
#ifdef PLATFORM_SPU
		pLocal->SpuStore(dmaData);
#endif
	}

	void NetworkUpdateTask(Scheduler::Task* pTask)
	{
		PERF_EVENT("RacingPhysics::NetworkUpdateTask");

		//DebugBreak();

		RacingPhysics::IntegrationCommand* pCommand = (RacingPhysics::IntegrationCommand*)&pTask->ParamData[0];

		RacingPhysics::Vehicle* pVehicle = InitialiseVehicle(pCommand->pVehicle, pCommand->vehicleDmaData);

		RacingPhysics::PhysicsTimer::Instance().SpuFetch(pCommand->physicsTimer);

		((RacingPhysics::Car*)pVehicle)->NetworkJumpIfNeeded();

		FinaliseVehicle(pVehicle, pCommand->vehicleDmaData);
	}

}

JQ_DECLARE_SPU_MODULE(NetworkUpdateModule, RacingPhysics_NetworkUpdate::NetworkUpdateTask, NetworkUpdateSpu, "RacingPhysics.NetworkUpdateTask");
//JQ_DECLARE_PPU_MODULE(NetworkUpdateModule, RacingPhysics_NetworkUpdate::NetworkUpdateTask);

#if PLATFORM_SPU

extern "C" void _exit() {snPause();}

#endif
