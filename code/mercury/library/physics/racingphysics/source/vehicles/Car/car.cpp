#include "stdafx.h"
#include "racingphysics/vehicles/car/car.h"
#include "racingphysics/physics/quaternion.h"
#include "racingphysics/physics/physicstimer.h"
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/sdk/shapedescriptors.inl"
#include "gravity/sdk/bodyhelper.h"
#include "gravity/sdk/bodyhelper.inl"
#include "gravity/sharedphysics/rigidbody.h"
#include "gravity/sharedphysics/rigidbody.inl"
#include "gravity/narrowphase/optimisedaabbtree.inl"
#include "gravity/sdk/CollisionPhantom.h"

/////////////////////////////////////////////////////////////////////////

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off)
#endif

namespace RacingPhysics
{

DECLARE_DEBUG_BOOL(g_overrideDriverSettings,"RacingPhysics.Cars.DriverSettings.OverrideDriverSettings", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_autoGears,"RacingPhysics.Cars.DriverSettings.AutoGears", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_tractionControl,"RacingPhysics.Cars.DriverSettings.TractionControl", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_ABS,"RacingPhysics.Cars.DriverSettings.ABS", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_stabilityControl,"RacingPhysics.Cars.DriverSettings.StabilityControl", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_debugLoadCollisionMeshFromFile,"RacingPhysics.Collision.LoadCollisionFromFile", false, Debug::Persist());
DECLARE_DEBUG_FLOAT(g_lowSpeedIntegration1Car,"RacingPhysics.Cars.Integration.EulerSpeed", 2.f, Debug::Range(0.f, 100.f));
DECLARE_DEBUG_FLOAT(g_lowSpeedIntegration2Car,"RacingPhysics.Cars.Integration.MidpointSpeed", 2.f, Debug::Range(0.f, 100.f));
DECLARE_DEBUG_BOOL(g_dbDoGravityIntegration,"RacingPhysics.Cars.Integration.GravityOverride", true);
DECLARE_DEBUG_BOOL(g_applyGravity,"RacingPhysics.Cars.Integration.ApplyGravity", true);
DECLARE_DEBUG_BOOL(g_copyFromGravity,"RacingPhysics.Cars.Integration.CopyFromGravity", true);
DECLARE_DEBUG_BOOL(g_copyVelocityFromGravity,"RacingPhysics.Cars.Integration.CopyVelocityFromGravity", true);
DECLARE_DEBUG_BOOL(g_copyPositionFromGravity,"RacingPhysics.Cars.Integration.CopyPositionFromGravity", true);
DECLARE_DEBUG_BOOL(g_ApplyNetworkForces,"RacingPhysics.Cars.Network.ApplyNetworkForces", true);
DECLARE_DEBUG_FLOAT(g_networkForceScale, "RacingPhysics.Cars.Network.NetworkForceScale", 1.f, Debug::Range(0.f, 10.f));
DECLARE_DEBUG_FLOAT(g_networkTorqueScale, "RacingPhysics.Cars.Network.NetworkTorqueScale", 1.f, Debug::Range(0.f, 10.f));

EXTERN_DEBUG_BOOL(g_updatePhysics);
EXTERN_DEBUG_FLOAT(g_debugThrust);

//////////////////////////////////////////////////////////////////////////////////

const static int TRIANGLE_CACHE_SIZE = 128; 
static float downforceScale = 100.f * PhysicsConstants::MPH;

#if defined(PLATFORM_SPU) && !defined(SPU_NET_TASK)
Collision::C_CachedTriangle g_triangleCache[TRIANGLE_CACHE_SIZE];
#endif

Car::Car()
#if !PLATFORM_SPU
: m_LineCache(TRIANGLE_CACHE_SIZE), m_pGravityCarBody(0)
#elif SPU_NET_TASK
: m_LineCache(0, 0)
#else
: m_LineCache(g_triangleCache, TRIANGLE_CACHE_SIZE)
#endif
{
	ArticulatedBody::Initialise(1, 5);
	Vehicle::Initialise(&Body(), Vehicle::VEHICLE_TYPE_CAR);

	for (int i = 0; i < 4; i++)
		m_wheels[i].InitialiseConstants(i, this, &GetRotor(i));

	m_engine.InitialiseConstants(this, &GetRotor(4));
	m_transmission.InitialiseConstants(this);

	m_bEnableWallRiding = false;
	m_dd.bNetworkJumpedThisFrame = false;
	m_dd.noNetworkJumpCounter = 0;

	ClearStepVariables();

	m_copyMassVariablesToGravity = false;
	m_dd.m_UsePhysicsGravity = false;
	m_dd.m_OverrideSteps = -1;
	m_dd.m_OverrideIntegrationMethod = -1;
}

void Car::ClearStepVariables()
{
#ifndef CFG_SPEED
	memset(&m_step, 0xffffffff, sizeof(m_step));

	for (int i = 0; i < m_wheels.Size(); i++)
		m_wheels[i].ClearStepVariables();
#endif
}

float Car::CalculateDownforceFactor() const
{
	return Dynamics().friction.constantDownforce + Dynamics().friction.downforce * PerformanceModifiers().downforceScale * Abs(AirSpeed_LocalZ()) / downforceScale;
}

void Car::Stop() 
{
	ArticulatedBody::Stop();
}

#if! CFG_SPEED

bool Car::Validate() const
{
	RP_ASSERT(Body().GetAxis().DebugTest());
	RP_ASSERT(_finite(Body().Velocity().X()));
	RP_ASSERT(_finite(Body().Velocity().Y()));
	RP_ASSERT(_finite(Body().Velocity().Z()));
	RP_ASSERT(_finite(Body().AngularVelocity().X()));
	RP_ASSERT(_finite(Body().AngularVelocity().Y()));
	RP_ASSERT(_finite(Body().AngularVelocity().Z()));
	RP_ASSERT(_finite(Body().Position().X()));
	RP_ASSERT(_finite(Body().Position().Y()));
	RP_ASSERT(_finite(Body().Position().Z()));
	RP_ASSERT(Magnitude(Body().Velocity()) < 1000.f);
	RP_ASSERT(Magnitude(Body().AngularVelocity()) < 250.f);
	RP_ASSERT(Body().GetAxis().Position().Y() > -9000.f);

	for (int i = 0; i < 4; i++)
		RP_ASSERT(Wheel(i).Validate());

	RP_ASSERT(_finite(m_dd.controls.steer));
	RP_ASSERT(_finite(m_dd.controls.throttle));
	RP_ASSERT(_finite(m_dd.controls.brake));
	RP_ASSERT(_finite(m_dd.controls.handbrake));

	RP_ASSERT(m_dd.controls.steer >= -1.f);
	RP_ASSERT(m_dd.controls.throttle >= 0.f);
	RP_ASSERT(m_dd.controls.brake >= 0.f);
	RP_ASSERT(m_dd.controls.handbrake >= 0.f);

	RP_ASSERT(m_dd.controls.steer <= 1.f);
	RP_ASSERT(m_dd.controls.throttle <= 1.f);
	RP_ASSERT(m_dd.controls.brake <= 1.f);
	RP_ASSERT(m_dd.controls.handbrake <= 1.f);

	return true;
}

#endif

#if !PLATFORM_SPU

Car::~Car()
{
	if(m_pGravityCarBody)
	{
		m_pGravityCarBody->SetIntegrationCallback(NULL);
	}

	delete m_dd.pPhantom;
	delete m_dd.pNetworkCollisionShape;

	if (m_dd.phantomCollisionGroup != Scheduler::JQ_GROUP_NONE)
		Scheduler::DestroyGroup( m_dd.phantomCollisionGroup );
}

void Car::DebugRenderTriangleCache(DbUtil::I_DebugRender* pDebugRender) const
{
	m_LineCache.DebugRender(pDebugRender);
}

void Car::Initialise(const VehicleInitialiseInfo& initialiseInfo) 
{
	SetFilename(initialiseInfo.filename.Chars());

	LOGDEBUG(g_RacingPhysicsLog, "Initialising car (%s)", initialiseInfo.filename.Chars());

	XtReload();

	m_dynamicsChangedCounter = 0;

	SetControlType(initialiseInfo.controlType);
	SetControllerPort(initialiseInfo.controllerPort);

	Axis axis;
	axis.SetIdentityMatrix();
	axis.SetPosition(Constants<Point3R>::Origin());

	ClearStepVariables();

	for (int i = 0; i < m_wheels.Size(); i++)
		Wheel_NC(i).Initialise();

	Point3R wheelPositions[4];
	for (int i = 0; i < 4; i++)
	{
		m_wheels[i].SetOffset(PerformanceModifiers().wheelModifiers[i].offset);
		wheelPositions[i] = Wheel(i).CalculateRestPosition(axis);
	}

	/*Btl::String physicsFilename = initialiseInfo.folder + initialiseInfo.filename + "_game.dat";
	bool success = m_collisionObject.InitialiseFromFile(&GetAxis(), initialiseInfo.collisionCentre, physicsFilename.Chars());

	if (!success)
	{
		Point3R pos(0.0f, 0.6f, initialiseInfo.collisionCentre.Z());
		Vector3R size(initialiseInfo.collisionSize.X(), initialiseInfo.collisionSize.Y(), initialiseInfo.collisionSize.Z());
		m_collisionObject.InitialiseCar(&Body().GetAxis(), 
											pos,
											size,
											wheelPositions,
											Dynamics().tyres.frontTyres.radius - Dynamics().suspension.front.movementUp);
	}*/

	//m_bodySurfaceType = (initialiseInfo.controlType == CONTROL_TYPE_AI) ? 
	//						Surfaces::SURFACE_TYPE_CAR_BODY_AI : Surfaces::SURFACE_TYPE_CAR_BODY;

	Body().Initialise(0);

	m_engine.Initialise();
	m_transmission.Initialise();

	SetMassVariables();

	SetAxis(axis);

	m_dd.throttle = 0.f;
	m_dd.brake = 0.f;
	m_dd.steer = 0.f;
	m_dd.handbrake = 0.f;

	m_dd.driverSettings.autoGears = true;
	m_dd.driverSettings.tractionControl = true;
	m_dd.driverSettings.ABS = true;
	m_dd.driverSettings.stabilityControl = false;

	{
		m_dd.frame.smoothedGroundNormal = Constants<Normal3R>::Y();
		Clear(&m_dd.frame.groundVelocity);
		m_dd.frame.previousSteeringAngle = 0.f;
		m_dd.frame.steeringRate = 0.f;
		m_dd.frame.airFrictionForce.Initialise();
		m_dd.frame.downforce.Initialise();
		m_dd.frame.networkForce.Initialise();
		m_dd.frame.steeringTorque.Initialise();
	}

	NetworkInitialise();
}

#endif

#ifndef SPU_NET_TASK

void Car::Shutdown() 
{
	for (int i = 0; i < m_wheels.Size(); i++)
		m_wheels[i].Shutdown();
}

void Car::GetIntegrationType(int& integrationType, int& nSteps)
{
	float v = Magnitude(Body().Velocity() - GroundVelocity());

	// change integration type at low speed
	if (integrationType == 2)
	{
		if (v < g_lowSpeedIntegration1Car)
		{
			nSteps *= 4;
			integrationType = 0; // euler
		}
		else if (v < g_lowSpeedIntegration2Car)
		{
			nSteps *= 2;
			integrationType = 1; // midpoint
		}
	}
	else if (integrationType == 1)
	{
		if (v < g_lowSpeedIntegration1Car)
		{
			nSteps *= 2;
			integrationType = 0; // euler
		}
	}
}

void Car::SetOnGround()
{
	for (int i = 0; i < m_wheels.Size(); i++)
		m_wheels[i].SetOnGround(true);
}

void Car::SetAxis(const Axis& a, float steeringAngle, bool overwriteOldAxis) 
{
	RP_ASSERT(Validate());

	Body().SetAxis(a);

	for (int i = 0; i < m_wheels.Size(); i++)
		m_wheels[i].SetCarAxis(a, steeringAngle);

	m_engine.SetCarAxis(a);

	if (overwriteOldAxis)
		m_dd.oldAxis = Body().GetAxis();

	SetOnGround();

	RP_ASSERT(Validate());
}

#if !PLATFORM_SPU

void Car::SetGravityBody(Gravity::C_Body* pBody, const Maths::HVec3 PCREF offsetToBodyCentre, const Maths::HVec3 PCREF bounds, const Maths::HVec3 PCREF offsetToBoundsCentre)
{
	m_pGravityCarBody = pBody;
	m_OffsetToCarBodyCentre = offsetToBodyCentre;
	pBody->SetIntegrationCallback(GetIntegrationCallback());

	{
		delete m_dd.pNetworkCollisionShape;
		delete m_dd.pPhantom;

		Collision::C_Shape* pOrigShape = m_pGravityCarBody->GetShape();

		Gravity::C_BoxDesc boxShapeDesc;
		boxShapeDesc.m_HalfExtents = bounds * 0.75f;

		m_dd.pNetworkCollisionShape = boxShapeDesc.CreateShape(&pOrigShape->m_Filter);
		m_dd.pNetworkCollisionShape->m_ElementOffset = offsetToBoundsCentre;
		m_dd.pPhantom = new Gravity::C_CollisionPhantom(m_dd.pNetworkCollisionShape);
	}
}
#endif

void Car::SetAxis(const Axis& a, bool overwriteOldAxis) 
{
	SetAxis(a, 0.f, overwriteOldAxis);
	//Maths::HMat33 matrix = a.Matrix3();
	//m_pGravityCarBody->Orientation(matrix);
}

void Car::CalculateAxes(const Axis& body, float steeringAngle, float* suspensionOffsets, Axis* wheelAxes) const
{
	for (int i = 0; i < m_wheels.Size(); i++)
	{
		wheelAxes[i] = body;
		wheelAxes[i].SetPosition(m_wheels[i].CalculateRestPosition(body));
		Normal3R n = m_wheels[i].CalculateSuspensionAxis(body);
		wheelAxes[i].Translate(n * suspensionOffsets[i]);
		wheelAxes[i].Rotate(n * steeringAngle);
	}
}

void Car::SetPosition(const Point3R& p) 
{
	Axis axis = Body().GetAxis();
	axis.SetPosition(p);
	SetAxis(axis);
	//m_pGravityCarBody->Position(p + axis.AxisY() * 0.6f);
}

void Car::ClearAllFeedback() {}

#if !PLATFORM_SPU

void Car::CopyToGravity() const
{
	Maths::HMat33 matrix(GetAxis().Matrix3());
	Maths::HVec3 angVel(-Body().AngularVelocity());
	//angVel.Z(-angVel.Z());
	SharedPhysics::C_RigidBody& rigid = m_pGravityCarBody->GetBody();
	const Maths::HVec3 elementOffset(m_pGravityCarBody->ElementOffset());
	const Maths::HVec3 offset((elementOffset + m_OffsetToCarBodyCentre + rigid.m_CenterOfMassOffset));
	const Maths::HVec3 offsetVelocity(Body().AngularVelocity().GetHVec() ^ ((offset - Body().CofG_Local()) * matrix));
	const Maths::HVec3 position(GetAxis().Position());
	const Maths::HVec3 newPosition(position + (offset - rigid.m_CenterOfMassOffset) * matrix);
	const Maths::HVec3 linearVelocity = Body().Velocity().GetHVec() + offsetVelocity;
	rigid.StoreVelocitiesAndPosition(linearVelocity, angVel, matrix, RacingPhysics::Point3R(newPosition));

	if(g_applyGravity)
	{
		m_pGravityCarBody->ClearBodyFlags(Gravity::C_Body::E_Type_Apply_Gravity);
	}
	else
	{
		m_pGravityCarBody->SetBodyFlags(Gravity::C_Body::E_Type_Apply_Gravity);
	}

	if (m_copyMassVariablesToGravity)
		CopyMassVariablesToGravity();
}

void Car::CopyFromGravity()
{
	m_dd.gravityTransformedBox = m_pGravityCarBody->GetObject()->m_TransformedBox;

	if (!g_copyFromGravity)
		return;

	const Maths::HVec3 elementOffset(m_pGravityCarBody->ElementOffset());
	const SharedPhysics::C_RigidBody& body(this->m_pGravityCarBody->GetBody());
	const Maths::HVec3 offset((elementOffset + m_OffsetToCarBodyCentre + body.m_CenterOfMassOffset));
	const RacingPhysics::Matrix33R rot(body.m_OrientationMatrix);

	const Maths::HF32 massRatio(Mass()*m_pGravityCarBody->GetBody().ReciprocalMass());
	m_dd.externalForce = Maths::C_Vector(m_pGravityCarBody->GetBody().m_Force * massRatio);
	m_dd.externalTorque = -m_pGravityCarBody->GetBody().m_Torque;

	Vector3R velocity = Body().Velocity();
	Vector3R angularVelocity = Body().AngularVelocity();
	Axis axis = GetAxis();
	
	if (g_copyVelocityFromGravity)
	{
		const Maths::HVec3 offsetVelocity(angularVelocity.GetHVec() ^ ((offset  - Body().CofG_Local()) * rot));

		Vector3R a = (body.m_LinearVelocity - offsetVelocity - velocity) / PhysicsTimer::Instance().PhysicsUpdateTime();

		if (Magnitude2(a) < (PhysicsConstants::GRAVITY * PhysicsConstants::GRAVITY))
			m_dd.externalForce += a * Body().Mass();
		else
			velocity = body.m_LinearVelocity - offsetVelocity;
			
		Vector3R aa = (-body.m_AngularVelocity - angularVelocity) / PhysicsTimer::Instance().PhysicsUpdateTime();

		if (Magnitude2(aa) < 1.f)
			m_dd.externalTorque += aa * Body().MI();
		else
			angularVelocity = -body.m_AngularVelocity;
	}

	if (g_copyPositionFromGravity)
	{
		const Maths::HVec3 newPosition = body.m_Position - ((offset - body.m_CenterOfMassOffset) * rot);

		/*{
			float x = Body().GetAxis().AxisX() | rot.AX();
			float y = Body().GetAxis().AxisY() | rot.AY();
			float z = Body().GetAxis().AxisZ() | rot.AZ();
			int breakhere = 0;
		}*/

		float d = Magnitude2(Vector3R(newPosition - Body().GetAxis().Position()));

		if (d > 0.00001f) // don't overwrite the (double precision) position unless necessary.
			axis.SetPosition(RacingPhysics::Point3R(newPosition));

		axis.SetRotation(rot);
	}

	bool setNetworkState = (GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK)
							&& (Magnitude(velocity - Body().Velocity()) > 20.f)
							&& (Magnitude(axis.Position() - Body().Position()) > 1.f);

	JumpToNetworkState(axis, velocity, angularVelocity);

	if (setNetworkState)
	{
		RacingPhysics::Vehicle::CompressedNetworkState compressedState;
		GetNetworkState(&compressedState, 0);
		ProcessNetworkState(compressedState, false);
		m_dd.noNetworkJumpCounter = 10;
	}
}

// Called once per frame - there may be several updates in between.
// BeginFrame and EndFrame run on the main thread.
void Car::BeginFrame(float largestSpeed) 
{
	if (!g_updatePhysics) return;

	RP_ASSERT(Validate());

	if (m_pGravityCarBody)
		CopyFromGravity();

	//Body().SetVelocity(Vector3R(0.f, 0.f, 0.f));

	RP_ASSERT(Validate());

	SetMassVariables();

	if (g_overrideDriverSettings)
	{
		m_dd.driverSettings.autoGears = g_autoGears;
		m_dd.driverSettings.tractionControl = g_tractionControl;
		m_dd.driverSettings.ABS = g_ABS;
		m_dd.driverSettings.stabilityControl = g_stabilityControl;
	}

	Vehicle::BeginFrame(largestSpeed);
	//NetworkJumpIfNeeded();
//	BeginReplayUpdate();

	// for gear changes
	m_transmission.BeginFrame();

	// in case of network jump

	m_dd.oldAxis = Body().GetAxis();

	for (int i = 0; i < m_wheels.Size(); i++)
	{
		m_wheels[i].CacheAxis();
		m_wheels[i].SetOffset(PerformanceModifiers().wheelModifiers[i].offset);
	}

	m_dynamicsChangedCounter = Max(m_dynamicsChangedCounter - 1, 0);
}

void Car::EndFrame(const Maths::C_TimeS PCREF time) 
{
	if (!g_updatePhysics) return;

	//m_pGravityCarBody->GetBody().m_LinearVelocity = Maths::HVec3(0.f, 0.f, 0.f);
	//m_pGravityCarBody->GetBody().m_LinearMomentum = Maths::HVec3(0.f, 0.f, 0.f);
	//m_pGravityCarBody->SetAngularDampingCoefficient(1.f);

	NetworkJumpIfNeeded();

	if(g_dbDoGravityIntegration && Air() && this->GetControlType() != Vehicle::CONTROL_TYPE_NETWORK)
	{
		if (m_pGravityCarBody)
		{
			if (!m_dd.m_UsePhysicsGravity)
			{
				m_dd.m_UsePhysicsGravity = true;
				m_pGravityCarBody->GetBody().AddForce(Maths::Constants<Maths::HVec3>::UnitAxisNegativeY() * m_pGravityCarBody->Mass() * PhysicsConstants::GRAVITY);
			}
			m_pGravityCarBody->SetBodyFlags(Gravity::C_Body::E_Type_Apply_Gravity);
			SharedPhysics::C_RigidBody& rigid = m_pGravityCarBody->GetBody();
			rigid.AngularDrag(Maths::HF32(Dynamics().friction.rotationalAirFriction));
			rigid.LinearDrag(Maths::HF32(Dynamics().friction.airFriction));
			//Put in linear/angular drag
			/*{
				const Maths::HVec3 v = rigid.m_AngularVelocity;
				const Maths::HVec3 frictionTorque = -v * Dynamics().friction.rotationalAirFriction;
				rigid.AddTorque(frictionTorque);
			}

			{
				const Maths::HVec3 v = rigid.m_LinearVelocity;
				const Maths::HVec3 frictionForce = -v * Magnitude(v) * Dynamics().friction.airFriction;
				rigid.AddForce(frictionForce);
			}*/
			
			m_pGravityCarBody->Integrate(m_pGravityCarBody->GetScene()->GetIntegrator(), time);
			CopyFromGravity();
		}
	}
	else
	{
		m_dd.m_UsePhysicsGravity = false;
		if (m_pGravityCarBody )
			CopyToGravity();
	}

	Vehicle::EndFrame(time);

	for (int i = 0; i < 4; i++)
		m_wheels[i].EndFrame();

	RP_ASSERT(Validate());
}

#endif

float Car::ConstrainInternal() 
{
	PERF_EVENT("RacingPhysics.ConstraintInternal");
	/*if(Air())
		0.f;*/
	static bool constrainInternal = true;

	if (!constrainInternal) return 0.f;

	Derivative derivative(GetNumParts());

	for (int i = 0; i < derivative.parts.Size(); i++)
		Clear(derivative.parts[i]);

	float dt = 1.f / 60.f;

	EvaluateConstraintForces(derivative, dt, true);

	{
		for (int i = 0; i < NumRigidBodies(); i++)
			GetRigidBody(i).UpdatePositions(derivative.parts[i], dt);

		int partIndex = NumRigidBodies();

		for (int i = 0; i < NumRotors(); i++, partIndex++)
			GetRotor(i).UpdatePositions(derivative.parts[partIndex], dt);
	}

	// Reset engine rotation.
	Engine_NC().SetCarAxis(Body().GetAxis());

	return 0.f;
}

void Car::CreateGravityCache()
{
	PERF_EVENT("Gravity.CreateGravityCache");

	void* data[512];
	SharedPhysics::C_SafeStack<true> stack(data, 512);
	Gravity::C_Scene* pScene = (Gravity::C_Scene*) Spu::FetchAlloc(GetGravityScene(), sizeof(Gravity::C_Scene));
	Collision::C_CollisionFilter filter = WheelGroundFilter();
	filter.MergeFlagsAndMask(WheelOverlayFilter());
	float time = 1.f / 60.f;

	Collision::C_CRAxisBox box = m_dd.gravityTransformedBox;
	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	float wheelDescentRange = 2.f;
	const Maths::C_Normal normal((m_bEnableWallRiding) ? -GetAxis().AxisY() : Maths::Constants<Maths::HVec3>::UnitAxisNegativeY(), Maths::C_Normal::UNIT_LENGTH_VECTOR);
	const Maths::HVec3 extrusion((GetVelocity() * time + normal.m_V * wheelDescentRange) * half);
	box.centre += extrusion;
	box.extents += Maths::Abs(extrusion);//this->GetVelocity() * time;
	//box.extents.Y(box.extents.Y() + wheelDescentRange);

	m_LineCache.Reset();	
	const Maths::HF32 maxCosAngle(-Maths::Constants<Maths::HF32>::Quarter());
	//const Maths::HF32 maxCosAngle(Maths::Constants<Maths::HF32>::Max());
	pScene->FillCache(box, m_LineCache, stack, &filter, normal, maxCosAngle);

	for(int a = 0; a < GetNumWheels(); ++a)
	{
		CarWheel& wheel = m_wheels[a];
		wheel.ResetTriangleCache();
	}
}

void Car::Integrate(float time) 
{
	PERF_EVENT("RacingPhysics.Integrate");
	ArticulatedBody::Integrate(time);
}

/*void Car::Push(const Point3R& p, const Vector3R& v) 
{
	Body().Push(p, v);
}

void Car::ApplyImpulse(const Point3R& p, const Vector3R& impulse, FloatVR torqueFactor) 
{
	Body().ApplyImpulse(p, impulse, torqueFactor);
}*/

void Car::HandleCollision(const Point3R& p, const Vector3R& impulse) {}

/*
void Car::GetCollisionBoundingBox(Point3R& min, Point3R& max) const 
{
	min = max = CollisionObject().TransformedCentre();
	min -= CollisionObject().AABB();
	max += CollisionObject().AABB();

	for (int i = 0; i < m_wheels.Size(); i++)
		m_wheels[i].GetCollisionBoundingBox(min, max);
}
*/

/*float Car::GetCollisionTime(float time, float resolution) const 
{
	return Vehicle::GetCollisionTime(time, resolution);
}*/
	
bool Car::WheelOnGround(int i) const {return m_wheels[i].OnGround();}

void Car::GetWheelContact(int i, Point3R& position, Normal3R& normal) const 
{
	position = m_wheels[i].GroundPosition();
	normal = m_wheels[i].GroundNormal();
}

/*U32 Car::GetCollisionFlags() const 
{
	return 0;//return (!AllWheelsOnGround()) ? 0 : COLLISION_FLAG_NOT_GROUND;
}*/

void Car::SetHold() 
{
	m_dd.controls.handbrake = true;
}

#if !PLATFORM_SPU

void Car::XtReload() 
{
	PERF_EVENT("Car::XtReload");

	bool changed;

	const XtRecord* p = CarDynamicsXt::LoadDynamicsXt(GetFilename(), changed);

	if (p)
	{
		m_pSettings = p;
		m_pCarDynamics = (CarDynamicsXt*)m_pSettings->object;
	}
	else
	{
		m_pSettings = CarDynamicsXt::LoadDynamicsXt("DefaultCar", changed);
		
		RP_ASSERT(m_pSettings);
		if (m_pSettings)
			m_pCarDynamics = (CarDynamicsXt*)m_pSettings->object;
	}

	if (changed)
		m_dynamicsChangedCounter = 30;
}

#endif

// articulated body
void Car::EvaluateDerivative(Derivative& derivative) 
{
	PERF_EVENT("RacingPhysics.Car.EvaluateDerivative");
	RP_ASSERT(Validate());

	ClearStepVariables();

	Body().EvaluateDerivative(derivative.parts[0]);

	derivative.parts[0].force += m_dd.externalForce;
	derivative.parts[0].torque += m_dd.externalTorque;

	if (g_applyGravity || !m_dd.m_UsePhysicsGravity)
		ApplyGravity(derivative, 0);

	if (m_wheels[0].m_dd.frame.onGround || m_wheels[1].m_dd.frame.onGround)
	{
		FloatVR constantDownforce = Mass() * PhysicsConstants::GRAVITY * Dynamics().friction.constantDownforce;
		FloatVR downforceFactor = Mass() * PhysicsConstants::GRAVITY * CalculateSpeedDependentDownforceFactor();

		Normal3R cd = Normalised(GetHeading() ^ (Constants<Normal3R>::Y() ^ GetHeading())); 
		Vector3R downforce = -cd * constantDownforce - Body().GetAxis().AxisY() * downforceFactor;
		m_dd.frame.downforce.Accumulate(downforce * AccumulationFactor());

		derivative.parts[0].force += downforce;
		derivative.parts[0].torque += GetRight() * downforceFactor * Dynamics().friction.downforceOffset;

		m_step.downforceFactor = CalculateDownforceFactor();
	}
	else
	{
		m_step.downforceFactor = 0.f;
	}

	m_step.maxAcceleration = CalculateMaxAcceleration();
	m_step.turnSteerLimit = CalculateTurnSteerLimit();
	float steerCentre;
	CalculateSteerLimits(m_step.steerLimitL, m_step.steerLimitR, steerCentre, true);

	for (int i = 0; i < NumRotors(); i++)
		GetRotor(i).EvaluateDerivative(derivative.parts[NumRigidBodies() + i]);

	{
		PERF_EVENT("RacingPhysics.Car.Forces");

		EvaluateSpringForces(derivative);
		EvaluateTyreForces(derivative);
		EvaluateFrictionForces(derivative);
		EvaluateNetworkForces(derivative);
		EvaluateTransmissionForces(derivative);
		EvaluateControlForces(derivative);
		EvaluateConstraintForces(derivative, TimeStep(), false);
	}

	ClearStepVariables();
	RP_ASSERT(Validate());
}

void Car::BeginIntegrate() 
{
	//PERF_EVENT("RacingPhysics.BeginIntegrate");
 

	float reverseSpeed = (m_dd.reverse) ? -0.1f : -1.f;

	if (!Air() && GetGroundSpeed_LocalZ() < Dynamics().gears.maxSpeedForReverse_Mph && DriverSettings().autoGears
			&& (Controls().reverse || 
				(AutoReverse() && ((Controls().brake > 0.1f && Controls().throttle < 0.1f) 
								|| GetGroundSpeed_LocalZ() < reverseSpeed))))
		m_dd.reverse = true;
	else 
		m_dd.reverse = false;

	{
		float throttle;

		if (m_dd.reverse)
		{
			if (Controls().reverse)
				throttle = 1.f;
			else 
				throttle = Controls().brake;
		}
		else
			throttle = Controls().throttle;

		float f = exp(-TimeStep() / (Dynamics().engine.throttleSmoothTime + 0.001f));

		m_dd.throttle = m_dd.throttle * f + throttle * (1.f - f);
	}
		
	{
		float brake;

		if (AutoReverse() && m_dd.reverse)
			brake = Controls().throttle;
		else
			brake = Controls().brake;

		float f = exp(-TimeStep() / (Dynamics().brakes.brakeSmoothTime + 0.001f));

		m_dd.brake = m_dd.brake * f + brake * (1.f - f);
	}

	{
		float steer = Controls().steer * Abs(Controls().steer);
		float f = exp(-TimeStep() / (Dynamics().steering.steerSmoothTime + 0.001f));

		m_dd.steer = m_dd.steer * f + steer * (1.f - f);
	}

	{
		float f = exp(-TimeStep() / (Dynamics().brakes.handbrakeSmoothTime + 0.001f));

		m_dd.handbrake = m_dd.handbrake * f + Controls().handbrake * (1.f - f);
	}

	m_dd.frame.airFrictionForce.Begin();
	m_dd.frame.downforce.Begin();
	m_dd.frame.networkForce.Begin();
	m_dd.frame.steeringTorque.Begin();

	for (int i = 0; i < m_wheels.Size(); i++)
		m_wheels[i].BeginIntegrate();

	m_engine.BeginIntegrate();
	m_transmission.BeginIntegrate(TimeStep());
}

void Car::EndIntegrate() 
{
	RP_ASSERT(Validate());

	//PERF_EVENT("RacingPhysics.EndIntegrate");
	m_dd.frame.airFrictionForce.End();
	m_dd.frame.downforce.End();
	m_dd.frame.networkForce.End();
	m_dd.frame.steeringTorque.End();

	for (int i = 0; i < m_wheels.Size(); i++)
		m_wheels[i].EndIntegrate();

	m_engine.EndIntegrate();

	if (!Air())
	{
		Vector3R averageNormal = Constants<Vector3R>::Null();
		Vector3R averageVelocity = Constants<Vector3R>::Null();
		int n = 0;

		for (int i = 0; i < m_wheels.Size(); i++)
		{
			if (Wheel(i).OnGround()) 
			{
				averageNormal += Wheel(i).GroundNormal();
				averageVelocity += Wheel(i).GroundVelocity();
				n++;
			}
		}
		
		float f = 0.9f;
		m_dd.frame.smoothedGroundNormal = Normalised(Normalised(averageNormal) * (1.f - f) + m_dd.frame.smoothedGroundNormal * f);
		m_dd.frame.groundVelocity = averageVelocity / (float) n;
	}

	// For steer damping.
	// (alternative would be adding another rotor and more constraint forces)
	float steeringAngle = Wheel(0).CalculateSteeringAngle();
	m_dd.frame.steeringRate = (steeringAngle - m_dd.frame.previousSteeringAngle) / TimeStep();
	m_dd.frame.previousSteeringAngle = steeringAngle;
}

void Car::SetMassVariables()
{
	Vector3R mi(Dynamics().mass.MIScaleX, Dynamics().mass.MIScaleY, Dynamics().mass.MIScaleZ);

	FloatVR surfaceMIScale = 0.f;
	for (int i = 0; i < m_wheels.Size(); i++)
		surfaceMIScale += m_wheels[i].m_dd.frame.surfaceMIScale;

	mi *= Dynamics().mass.mass;
	mi *= surfaceMIScale * 0.25f;

	Body().SetMassVariables(Dynamics().mass.mass, mi, Point3R(0.f, Dynamics().mass.cofGOffsetY, Dynamics().mass.cofGOffsetZ));
	
	for (int i = 0; i < m_wheels.Size(); i++)
		m_wheels[i].SetMassVariables();

	ArticulatedBody::CalculateMass();
}

#if !PLATFORM_SPU

void Car::CopyMassVariablesToGravity() const
{
	SharedPhysics::C_RigidBody& body = m_pGravityCarBody->GetBody();
	body.Mass(Dynamics().mass.mass);
	body.ReciprocalMass(Maths::Reciprocal(Dynamics().mass.mass));
	if (Dynamics().mass.physicsParameters)
		body.m_CenterOfMassOffset = (Maths::HVec3(0.f, Dynamics().mass.physicsParameters->physicsCofMassOffsetY, Dynamics().mass.physicsParameters->physicsCofMassOffsetZ) - m_OffsetToCarBodyCentre) - m_pGravityCarBody->ElementOffset();
	else
		body.m_CenterOfMassOffset = (Maths::HVec3(0.f, Dynamics().mass.physicsCofMassOffsetY, Dynamics().mass.physicsCofMassOffsetZ) - m_OffsetToCarBodyCentre) - m_pGravityCarBody->ElementOffset();

	if (Dynamics().mass.physicsParameters ||
		(Dynamics().mass.interiaTensorScaleX != 0.0f &&
		 Dynamics().mass.interiaTensorScaleY != 0.0f &&
		 Dynamics().mass.interiaTensorScaleZ != 0.0f))
	{
		Maths::HMat33 iBody;
		float ix = Dynamics().mass.interiaTensorScaleX;
		float iy = Dynamics().mass.interiaTensorScaleY;
		float iz = Dynamics().mass.interiaTensorScaleZ;
		if (Dynamics().mass.physicsParameters)
		{
			ix = Dynamics().mass.physicsParameters->interiaTensorScaleX != 0.0f ? Dynamics().mass.physicsParameters->interiaTensorScaleX : 1.0f;
			iy = Dynamics().mass.physicsParameters->interiaTensorScaleY != 0.0f ? Dynamics().mass.physicsParameters->interiaTensorScaleY : 1.0f;
			iz = Dynamics().mass.physicsParameters->interiaTensorScaleZ != 0.0f ? Dynamics().mass.physicsParameters->interiaTensorScaleZ : 1.0f;
		}
		float scale = Dynamics().mass.mass * 0.25f;

		iBody.AX(Maths::Constants<Maths::HVec3>::UnitAxisPositiveX() * ix * scale);
		iBody.AY(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY() * iy * scale);
		iBody.AZ(Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ() * iz * scale);

		m_pGravityCarBody->IBodyMatrix(iBody);
	}
}

void Car::EnableWallRiding(bool bWallRiding)
{
	m_bEnableWallRiding = bWallRiding;
}
#endif

/*
FloatVR Car::GetLargestVelocity() const
{
	FloatVR v = Magnitude(Body().PointVelocity(CollisionObject().TransformedCentre())) 
			  + Magnitude(Body().AngularVelocity()) * CollisionObject().Radius();

	return v;
}
*/

float Car::CalculateSpeedDependentDownforceFactor() const
{
	return Abs(AirSpeed_LocalZ()) * Dynamics().friction.downforce * PerformanceModifiers().downforceScale / downforceScale;
}

float Car::CalculateDownforceFactor(float speed) const
{
	return Dynamics().friction.constantDownforce + Dynamics().friction.downforce * PerformanceModifiers().downforceScale * Abs(speed) / downforceScale;
}

bool Car::Air() const
{
	for (int i = 0; i < m_wheels.Size(); i++)
		if (m_wheels[i].OnGround())
			return false;

	return true;
}

bool Car::CalculateAir() const
{
	for (int i = 0; i < m_wheels.Size(); i++)
		if (m_wheels[i].OnGround_Step())
			return false;

	return true;
}

S32 Car::NumWheelsOnGround() const
{
	S32 count = 0;

	for (int i = 0; i < m_wheels.Size(); i++)
		if (m_wheels[i].m_dd.frame.onGround)
			count++;

	return count;
}

float Car::CalculateMaxAcceleration() const
{
	FloatVR h = Dynamics().tyres.frontTyres.radius + Dynamics().mass.cofGOffsetY;
	FloatVR lf = Dynamics().geometry.wheelBase * 0.5f + Dynamics().mass.cofGOffsetZ;
	FloatVR lr = Dynamics().geometry.wheelBase * 0.5f - Dynamics().mass.cofGOffsetZ;
	FloatVR l = Dynamics().geometry.wheelBase;
	FloatVR gf = (Wheel(0).ForwardsGripScale() + Wheel(1).ForwardsGripScale()) * 0.5f * ((Dynamics().transmission.driveType != CarDynamicsXt::DRIVE_TYPE_REARWHEEL) ? 1.f : 0.f);
	FloatVR gr = (Wheel(2).ForwardsGripScale() + Wheel(3).ForwardsGripScale()) * 0.5f * ((Dynamics().transmission.driveType != CarDynamicsXt::DRIVE_TYPE_FRONTWHEEL) ? 1.f : 0.f);
	FloatVR grMax = lr / h; gr = Min(gr, grMax);
	FloatVR downforceFactor = (1.f + DownforceFactor_Step());
	FloatVR downforceTorque = Dynamics().friction.downforceOffset * CalculateSpeedDependentDownforceFactor();
	FloatVR dt = downforceTorque / downforceFactor;

	FloatVR frontRatio = 2.f * (lr + h * gr - dt) / (l + h * (gf - gr));
	FloatVR rearRatio = 2.f * (lf - h * gf + dt) / (l + h * (gf - gr));

	FloatVR fy = PhysicsConstants::GRAVITY * downforceFactor * 0.25f;
	FloatVR a(0.f);

	a += frontRatio * 2.f * fy * ((Dynamics().transmission.driveType != CarDynamicsXt::DRIVE_TYPE_REARWHEEL) ? 1.f : 0.f);
	a += rearRatio * 2.f * fy * ((Dynamics().transmission.driveType != CarDynamicsXt::DRIVE_TYPE_FRONTWHEEL) ? 1.f : 0.f);

	return a;
}

//////////////////////////////////////////////////////

void Car::EvaluateTyreForces(Derivative& derivative)
{
	//PERF_EVENT("RacingPhysics.Car.EvaluateTyreForces");

	for (int wi = 0; wi < m_wheels.Size(); wi++)
	{
		if (Wheel(wi).OnGround_Step())
		{
			Vector3R force, torque;
			Wheel_NC(wi).CalculateDynamicFrictionForce(force, torque);

			Wheel_NC(wi).m_dd.frame.groundFriction.Accumulate(force * AccumulationFactor());
			Wheel_NC(wi).m_dd.frame.groundTorque.Accumulate(torque * AccumulationFactor());

			// Force and torque calculated at the contact patch.
			// Translate to force and torque at the wheel centre.
			torque += (Wheel(wi).GroundPosition_Step() - Wheel(wi).GetPosition_Step()) ^ force;

			// Apply force to the body, minus Y component.
			force -= Wheel(wi).SuspensionAxis_Step() * (Wheel(wi).SuspensionAxis_Step() | force);
			ApplyExternalForce(derivative, BodyIndex(), force, Wheel(wi).GetPosition_Step());

			// Apply torque to the wheel.
			ApplyExternalTorque(derivative, WheelIndex(wi), torque);
		}
	}
}

void Car::EvaluateSpringForces(Derivative& derivative)
{
	//PERF_EVENT("RacingPhysics.Car.EvaluateSpringForces");
	FloatVR frontAntiRollDistance = 0.f;
	FloatVR rearAntiRollDistance = 0.f;
	float springForces[4];

	for (int i = 0; i < m_wheels.Size(); i++)
	{
		m_wheels[i].PutWheelOnGround();

		if (Wheel(i).OnGround_Step())
		{
			FloatVR mass = FloatVR(Dynamics().mass.mass) * 0.25f;
			FloatVR staticLoad = mass * PhysicsConstants::GRAVITY * (1.f + DownforceFactor_Step());
			Normal3R ns = Wheel(i).SuspensionAxis_Step();
			Normal3R ng = Wheel(i).GroundNormal_Step();
			FloatVR r =  Max(ns | ng, 0.5f);

			// find spring velocity from car movement and ground normal (massless wheel)
			FloatVR rv = ((Body().PointVelocity(Wheel(i).GetPosition_Step()) - Wheel(i).GroundVelocity_Step()) | ng) / r;

			FloatVR scale = staticLoad / Wheel(i).SuspensionDynamics().movementUp;
			FloatVR springConstant = scale * Wheel(i).SuspensionDynamics().springStiffness;
			FloatVR damping = Maths::Sqrt(springConstant * mass) 
								* ((rv > 0.f) ? Wheel(i).SuspensionDynamics().springDampingRelease 
											: Wheel(i).SuspensionDynamics().springDampingCompress);

			FloatVR springProgression = Clamped(1.f - Wheel(i).SuspensionDynamics().springProgression, 0.f, 0.99f);
			FloatVR movement = 2.f * Wheel(i).SuspensionDynamics().movementUp;

			FloatVR a = springConstant * (1.f - springProgression);
			FloatVR b = springConstant * springProgression * movement;
			FloatVR c = -staticLoad * movement;

			FloatVR offset = -(-b + Sqrt(b * b - 4.f * a * c)) / (2.f * a);
			//FloatVR offset = -(staticLoad) / springConstant;
			FloatVR d = ((Wheel(i).GetPosition_Step() - Wheel(i).RestPosition_Step()) | ns) - offset;
			
			springConstant *= Lerp(springProgression, 1.f, Clamped(d / movement, FloatVR(0.f), FloatVR(1.f)));

			FloatVR dmax = movement - offset;
			d = Clamped(d, FloatVR(0.f), dmax);
			rv = ClampedAbs(rv, movement * 60.f);

			FloatVR springForce = d * springConstant - rv * damping;

			springForce = Max(springForce, FloatVR(0.f));

			// net force is along ground normal
			Vector3R force = ng * springForce / r;

			if (i < 2)
				frontAntiRollDistance += d * ((i % 2) ? -1.f : 1.f);
			else
				rearAntiRollDistance += d * ((i % 2) ? -1.f : 1.f);

			//RP_ASSERT(springForce / r < Mass() * PhysicsConstants::GRAVITY * 50.f);
			Maths::Store(springForce / r, &springForces[i]);

			Wheel_NC(i).m_dd.frame.groundNormalForce.Accumulate(force * AccumulationFactor());
			ApplyExternalForce(derivative, BodyIndex(), force, Wheel(i).GroundPosition_Step());

			//Vector3R torque = (Wheel(i).GroundPosition_Step() - Wheel(i).GetPosition_Step()) ^ force;
			//ApplyExternalForce(derivative, BodyIndex(), force, Wheel(i).GetPosition_Step());
			//ApplyExternalTorque(derivative, WheelIndex(i), torque);
		}
	}

	// anti-roll
	if (Wheel(0).OnGround_Step() && Wheel(1).OnGround_Step())
	{
		FloatVR scale = Dynamics().mass.mass * (1.f + DownforceFactor_Step())
							/ (Dynamics().suspension.front.movementUp + Dynamics().suspension.front.movementDown);

		FloatVR force = (frontAntiRollDistance * Dynamics().suspension.front.rollStiffness * scale);
		FloatVR min = -springForces[0];
		FloatVR max = springForces[1];
		force = Clamped(force, min, max);

		Wheel_NC(0).m_dd.frame.groundNormalForce.Accumulate(force * Wheel(0).GroundNormal_Step() * AccumulationFactor());
		Wheel_NC(1).m_dd.frame.groundNormalForce.Accumulate(-force * Wheel(1).GroundNormal_Step() * AccumulationFactor());
		
		ApplyExternalForce(derivative, BodyIndex(), force * Wheel(0).GroundNormal_Step(), Wheel(0).GetPosition_Step());
		ApplyExternalForce(derivative, BodyIndex(), -force * Wheel(1).GroundNormal_Step(), Wheel(1).GetPosition_Step());
	}

	if (Wheel(2).OnGround_Step() && Wheel(3).OnGround_Step())
	{
		FloatVR scale = Dynamics().mass.mass * (1.f + DownforceFactor_Step())
							/ (Dynamics().suspension.rear.movementUp + Dynamics().suspension.rear.movementDown);

		FloatVR force = (frontAntiRollDistance * Dynamics().suspension.rear.rollStiffness * scale);
		FloatVR min = -springForces[2];
		FloatVR max = springForces[3];
		force = Clamped(force, min, max);

		Wheel_NC(2).m_dd.frame.groundNormalForce.Accumulate(force * Wheel(2).GroundNormal_Step() * AccumulationFactor());
		Wheel_NC(3).m_dd.frame.groundNormalForce.Accumulate(-force * Wheel(3).GroundNormal_Step() * AccumulationFactor());
		
		ApplyExternalForce(derivative, BodyIndex(), force * Wheel(2).GroundNormal_Step(), Wheel(2).GetPosition_Step());
		ApplyExternalForce(derivative, BodyIndex(), -force * Wheel(3).GroundNormal_Step(), Wheel(3).GetPosition_Step());
	}

	m_step.air = CalculateAir();
}

void Car::EvaluateTransmissionForces(Derivative& derivative)
{
	//PERF_EVENT("RacingPhysics.Car.EvaluateTransmissionForces");
	m_transmission.Setup();

	{
		m_transmission.SetEngineAngularVelocity(Engine().AngularVelocity1D());

		for (int i = 0; i < m_wheels.Size(); i++)
			m_transmission.SetWheelAngularVelocity(i, m_wheels[i].AngularVelocity1D_Step());

		m_transmission.CalculateViscousForces(TimeStep());

		// apply forces
		{
			Vector3R torque = -Engine().GetDriveAxis() * m_transmission.GetEngineTorque();
			ApplyTorque(derivative, EngineIndex(), BodyIndex(), torque);
		}

		for (int i = 0; i < m_wheels.Size(); i++)
		{
			Vector3R torque = -m_wheels[i].GetDriveAxis() * m_transmission.GetWheelTorque(i);
			ApplyTorque(derivative, WheelIndex(i), BodyIndex(), torque);
		}
	}

	// update the velocities so that the constraints 
	// take into account the forces from this frame
	{
		State oldState;
		GetState(oldState);
		UpdateVelocities(derivative, TimeStep());

		m_transmission.SetEngineAngularVelocity(Engine().AngularVelocity1D());

		for (int i = 0; i < m_wheels.Size(); i++)
			m_transmission.SetWheelAngularVelocity(i, m_wheels[i].AngularVelocity1D_Step());

		m_transmission.CalculateConstraintForces(TimeStep());

		// apply forces
		{
			Vector3R torque = -Engine().GetDriveAxis() * m_transmission.GetEngineTorque();
			ApplyTorque(derivative, EngineIndex(), BodyIndex(), torque);
		}

		for (int i = 0; i < m_wheels.Size(); i++)
		{
			Vector3R torque = -m_wheels[i].GetDriveAxis() * m_transmission.GetWheelTorque(i);
			ApplyTorque(derivative, WheelIndex(i), BodyIndex(), torque);
		}

		RestoreState(oldState);
	}

	m_transmission.Finish();
}

float Car::CalculateTurnSteerLimit() const
{
	Vector3R v = Body().Velocity() - GroundVelocity();

	FloatVR acceleration = (Wheel(0).TransverseGripScale() + Wheel(1).TransverseGripScale()) * 0.5f * PhysicsConstants::GRAVITY * (1.f + CalculateDownforceFactor());
	FloatVR v2 = Magnitude2(v);
	FloatVR turnSteerLimit = Maths::ArcTan(Dynamics().geometry.wheelBase * acceleration / (v2 * 2.f));
	return turnSteerLimit;
}

float Car::CalculateTurningAngularVelocity() const
{
	Vector3R v = Body().Velocity() - GroundVelocity();
	FloatVR a = (Wheel(0).TransverseGripScale() + Wheel(1).TransverseGripScale()) * 0.5f * PhysicsConstants::GRAVITY * (1.f + CalculateDownforceFactor());
	FloatVR v2 = Magnitude2(v);
	FloatVR w2 =  (a * a) / (v2 + 100.f);
	return Sqrt(w2);
}

float Car::CalculateSlipSteerOffset() const
{
	Point3R p = Body().GetAxis().Global(Point3R(0.f, 0.f, VEHICLE_FORWARD_Z_SIGN Dynamics().geometry.wheelBase * 0.5f));
	Vector3R v = Body().PointVelocity(p) - GroundVelocity();
	FloatVR x = v | GetRight();
	FloatVR z = Abs(v | GetHeading());
	return -Maths::ArcTan(x / z);
}

void Car::EvaluateControlForces(Derivative& derivative)
{
	//PERF_EVENT("RacingPhysics.Car.EvaluateControlForces");
	float maxEngineTorque = 10000.f;

	Engine_NC().EvaluateControlForces(derivative, maxEngineTorque / Abs(Transmission().GearRatio()));

	// steering
	{
		bool air = Air_Step();

		FloatVR steeringAngle = Wheel(0).SteeringAngle_Step();
		FloatVR speed = Magnitude(Body().Velocity());
		FloatVR slip = (Body().Velocity() | GetRight()) / ((speed + 1.f) * Dynamics().tyres.rearTyres.transverseTyreSlipScale);
		FloatVR stableTurnAV = CalculateTurningAngularVelocity();
		FloatVR steeringRate = ClampedAbs(FloatVR(m_dd.frame.steeringRate), FloatVR(10.f));

		FloatVR lowSpeedSpringFactor, damping;
		FloatVR groundSpeedLocalZ = GetGroundSpeed_LocalZ();

		if (!air)
		{
			lowSpeedSpringFactor = Max(10.f - Abs(groundSpeedLocalZ), 0.f) * 0.1f;
			damping = (GetGroundSpeed_LocalZ() > -1.f)	? Dynamics().steering.steerDamping 
														: Dynamics().steering.reverseSteerDamping;
		}
		else
		{
			lowSpeedSpringFactor = 1.f;
			damping = Dynamics().steering.steerDamping;
		}

		FloatVR resistanceTorque;
		FloatVR steerSpringAngle;
		FloatVR steer;
		FloatVR avSteer;
		FloatVR slideSteer;

		// steer amount
		{
			FloatVR s = Steer();
			FloatVR b = Brake();
			FloatVR scale = Sqrt(s * s + b * b);
			steer = s / Max(scale, FloatVR(1.f));
		}

		// resistance torque
		{
			// Calculate the torque from the ground forces.
			FloatVR pitch0 = (Wheel(0).SuspensionDistance() - Wheel(2).SuspensionDistance()) / (Dynamics().geometry.wheelBase);
			FloatVR pitch1 = (Wheel(1).SuspensionDistance() - Wheel(3).SuspensionDistance()) / (Dynamics().geometry.wheelBase);
			FloatVR roll = Abs(Wheel(0).SuspensionDistance() - Wheel(1).SuspensionDistance()) * Abs(steeringAngle) / (Dynamics().geometry.trackWidthFront);

			FloatVR roll0 = Wheel(0).OnGround_Step() ? Abs(Normalised(Wheel(0).GroundFriction()) ^ Wheel(0).GroundNormal_Step() | Wheel(0).SuspensionAxis_Step()) * Abs(steeringAngle) : FloatVR(0.f);
			FloatVR roll1 = Wheel(1).OnGround_Step() ? Abs(Normalised(Wheel(1).GroundFriction()) ^ Wheel(1).GroundNormal_Step() | Wheel(1).SuspensionAxis_Step()) * Abs(steeringAngle) : FloatVR(0.f);
			
			FloatVR sinCasterAngle = Dynamics().suspension.front.casterAngle - (pitch0 + pitch1) * 0.5f;

			FloatVR trail0 = Dynamics().tyres.frontTyres.radius * (Dynamics().suspension.front.casterAngle + roll0);
			FloatVR trail1 = Dynamics().tyres.frontTyres.radius * (Dynamics().suspension.front.casterAngle + roll1);

			FloatVR maxGrip0 = Wheel(0).TransverseGripScale() * Magnitude(Wheel(0).GroundForce());
			FloatVR maxGrip1 = Wheel(1).TransverseGripScale() * Magnitude(Wheel(1).GroundForce());

			FloatVR resistanceTorque0 = maxGrip0 * (trail0 + Wheel(0).SlipAlignment());		
			FloatVR resistanceTorque1 = maxGrip1 * (trail1 + Wheel(1).SlipAlignment());
			resistanceTorque = (resistanceTorque0 + resistanceTorque1);
			
			// Gyroscopic torque due to the rotation of the car (which forces the wheel to turn with it).
			FloatVR gyroscopicTorque = Dynamics().mass.frontWheelMI * sinCasterAngle *
										stableTurnAV * steer *
										((Wheel(0).GetRotor().AngularVelocity() | Wheel(0).GetRotationMatrix().AxisX())
										 + (Wheel(1).GetRotor().AngularVelocity() | Wheel(1).GetRotationMatrix().AxisX()));

			resistanceTorque += Abs(gyroscopicTorque);

			// Add a factor proportional to the MI to allow for non-steady state turning and for when the wheel is not on the ground.
			resistanceTorque += Dynamics().mass.frontWheelMI * 20.f;
		}

		// steer spring
		{
			float steerLimitL, steerLimitR, steerCentre;
			CalculateSteerLimits(steerLimitL, steerLimitR, steerCentre, true);
			FloatVR desiredAngle;
			if (steer > 0.f) desiredAngle = Lerp(0.f, steerLimitR, steer);
			else desiredAngle = Lerp(0.f, steerLimitL, -steer);
			FloatVR scale = Dynamics().tyres.frontTyres.transverseTyreSlipScale;
			steerSpringAngle = ClampedAbs((steeringAngle - desiredAngle) / scale, FloatVR(1.f));
		}

		{
			// anti-flick
			FloatVR desiredAV = stableTurnAV * steer * (1.f + Dynamics().steering.antiFlickThreshold);
			FloatVR actualAV = (Body().AngularVelocity() | Body().GetAxis().AxisY());
			FloatVR avDifference = Min(0.f, Abs(desiredAV) - actualAV * Sign(desiredAV)) * Sign(desiredAV);
			avSteer = avDifference * Dynamics().steering.antiFlick * PerformanceModifiers().steeringAntiFlickScale;
			avSteer *= Clamped((Abs(groundSpeedLocalZ) - FloatVR(10.f)) / 4.f, FloatVR(0.f), FloatVR(1.f));
		}

		FloatVR lowSpeedSpring = Dynamics().steering.steerDirect * Dynamics().steering.steerSpringLowSpeed;

		FloatVR torque = resistanceTorque
								* (ClampedAbs(steer * Dynamics().steering.steerDirect, FloatVR(1.f))
									- Lerp(	steerSpringAngle * Dynamics().steering.steerSpringDrift , steeringAngle * lowSpeedSpring, lowSpeedSpringFactor)
									- steeringRate * damping
									+ avSteer
									);

		ApplyTorque(derivative, BodyIndex(), WheelIndex(0), Wheel(0).SuspensionAxis_Step() * -torque * 0.5f);
		ApplyTorque(derivative, BodyIndex(), WheelIndex(1), Wheel(1).SuspensionAxis_Step() * -torque * 0.5f);

		m_dd.frame.steeringTorque.Accumulate(Wheel(0).SuspensionAxis_Step() * torque * AccumulationFactor());
	}

	// brakes
	{
		FloatVR h = Dynamics().tyres.frontTyres.radius + Dynamics().mass.cofGOffsetY;
		FloatVR lf = Dynamics().geometry.wheelBase * 0.5f + Dynamics().mass.cofGOffsetZ;
		FloatVR lr = Dynamics().geometry.wheelBase * 0.5f - Dynamics().mass.cofGOffsetZ;
		FloatVR l = Dynamics().geometry.wheelBase;
		FloatVR gf = (Wheel(0).BackwardsGripScale() + Wheel(1).BackwardsGripScale()) * 0.5f * Brake();
		FloatVR gr = (Wheel(2).BackwardsGripScale() + Wheel(3).BackwardsGripScale()) * 0.5f * AddSmooth(Brake(), Handbrake());
		FloatVR downforceFactor = (1.f + DownforceFactor_Step());
		FloatVR downforceTorque = Dynamics().friction.downforceOffset * CalculateSpeedDependentDownforceFactor();
		FloatVR dt = downforceTorque / downforceFactor;

		FloatVR frontRatio = 2.f * (lr + h * gr - dt) / (l + h * (gr - gf));
		FloatVR rearRatio = 2.f * (lf - h * gf + dt) / (l + h * (gr - gf));

		FloatVR fy = Mass() * PhysicsConstants::GRAVITY * downforceFactor * 0.25f;

		FloatVR s = Steer();
		FloatVR b = Brake();
		FloatVR scale = Sqrt(s * s + b * b);
		FloatVR brake = b;// / Max(scale, FloatVR(1.f));

		Btl::Vector<FloatVR, Btl::VectorParameters_Array<4> > brakeForce;
		brakeForce[0] = brakeForce[1] = Min(Dynamics().brakes.frontBrakeForce, FloatVR(1.f)) * brake;
		brakeForce[2] = brakeForce[3] = Min(Dynamics().brakes.rearBrakeForce, FloatVR(1.f)) * brake;
		
		{
			Btl::Vector<FloatVR, Btl::VectorParameters_Array<4> > wav;
			FloatVR wa(0.f);

			for (int i = 0; i < m_wheels.Size(); i++)
			{
				wav[i] = Abs(Wheel(i).AngularVelocity1D_Step());
				wa += wav[i] * Magnitude(m_wheels[i].GroundForce()) * 0.25f / fy;
			}

			if (wa > 4.f)
			{
				FloatVR zero = 0.f;
				FloatVR one = 1.f;
				FloatVR wInv = 1.f / wa;

				FloatVR dy = Body().AngularVelocity() | Body().GetAxis().AxisY();
				FloatVR a = (Wheel(0).TransverseGripScale() + Wheel(1).TransverseGripScale()) * 0.5f * PhysicsConstants::GRAVITY * (1.f + DownforceFactor_Step());
				FloatVR at = -Acceleration() | GetRight();
				FloatVR v = Abs(GetGroundSpeed_LocalZ());
				FloatVR dyMax = a * Max(Steer() * Sign(dy), zero) / (v + 2.f);

				static float stabilityScale = 4.f;

				FloatVR stabilityFactor = Clamped((Abs(dy) - dyMax) * stabilityScale, zero, one);
				stabilityFactor *= Min(Maths::Abs(at / a), FloatVR(1.f));
				int stabilityIndex = (Steer() > 0.f) ? 0 : 1;

				FloatVR understeerFactor = Clamped((dyMax - Abs(dy)) * stabilityScale, zero, one);
				int understeerIndex = 1 - stabilityIndex;

				if (DriverSettings().stabilityControl)
				{
					brakeForce[stabilityIndex] = AddSmooth(brakeForce[stabilityIndex], stabilityFactor);
					brakeForce[stabilityIndex + 2] = AddSmooth(brakeForce[stabilityIndex + 2], stabilityFactor);
				}

				// ABS
				if (DriverSettings().ABS)
				{
					for (int i = 0; i < m_wheels.Size(); i++)
					{
						if (Wheel(i).OnGround_Step())
						{
							//FloatVR offset = 1.f - 2.f * Wheel(i).LongitudinalTyreSlip();
							//FloatVR scale = 1.f / (1.f - offset);
							//brakeForce[i] *= Clamped((wav[i] * wInv - offset) * scale, zero, one);

							FloatVR spin = Wheel(i).WheelSpin_Step() / Wheel(i).LongitudinalTyreSlip();
							brakeForce[i] *= Clamped(2.f - spin, zero, one);
						}
					}

					brakeForce[stabilityIndex] *= 1.f - understeerFactor;
					brakeForce[1 - stabilityIndex] *= 1.f - stabilityFactor;
				}

				// traction control
				if (DriverSettings().tractionControl)
				{
					for (int i = 0; i < m_wheels.Size(); i++)
					{
						if (Wheel(i).Driven() && Wheel(i).OnGround_Step())
						{
							FloatVR maxSlip = Dynamics().engine.tractionControlMaxSlip;

							FloatVR slip = Wheel(i).SlipAngle_Step() / Wheel(i).TransverseTyreSlip();
							slip = Max(Abs(slip) - 1.f, 0.f);
							FloatVR maxSpin = Dynamics().engine.tractionControlMaxSpin * 
												Maths::Sqrt(1.f - Min((slip * slip) / (maxSlip * maxSlip), FloatVR(1.f)));

							FloatVR maxSpeed = Dynamics().engine.wheelSpinMaxSpeed_Mph * PhysicsConstants::MPH;
							static float s_lowSpeedWheelSpin = 15.f;
							FloatVR speedScale = s_lowSpeedWheelSpin * (maxSpeed - Magnitude(Body().Velocity())) / (maxSpeed + 1.f);
							maxSpin *= Max(speedScale, 1.f);
							maxSpin = Min(maxSpin, PerformanceModifiers().maxWheelSpin);
							
							FloatVR spin = -Wheel(i).WheelSpin_Step() / Wheel(i).LongitudinalTyreSlip();

							if (spin > maxSpin)
							{
								FloatVR angularVelocity = Wheel(i).AngularVelocity1D_Step();

								FloatVR newAngularVelocity = angularVelocity 
															+ (maxSpin - spin) 
															* Wheel(i).LongitudinalTyreSlip()
															/ (Wheel(i).Radius() * Wheel(i).WheelSpinFactor_Step());

								FloatVR newAngularMomentum = newAngularVelocity * Wheel(i).GetRotor().MI() / Wheel(i).RotationScale();
								Normal3R x = Wheel(i).GetDriveAxis();
								FloatVR angularMomentum = -(Wheel(i).GetRotor().AngularMomentum() | x);
								FloatVR torque = (newAngularMomentum - angularMomentum) / TimeStep() + (derivative.parts[WheelIndex(i)].torque | x);

								if (torque < 0.f)
									ApplyTorque(derivative, BodyIndex(), WheelIndex(i), x * torque);
							}
						}
					}
				}
			}
		}

		brakeForce[0] *= frontRatio;
		brakeForce[1] *= frontRatio;
		brakeForce[2] *= rearRatio;
		brakeForce[3] *= rearRatio;

		// Allow burnouts.
		if (Dynamics().transmission.driveType == CarDynamicsXt::DRIVE_TYPE_FRONTWHEEL)
		{
			brakeForce[0] *= 1.f - Throttle();
			brakeForce[1] *= 1.f - Throttle();
		}
		else
		{
			brakeForce[2] *= 1.f - Throttle();
			brakeForce[3] *= 1.f - Throttle();
		}

		brakeForce[2] = Lerp(brakeForce[2], rearRatio * Dynamics().brakes.handbrakeForce, Handbrake());
		brakeForce[3] = Lerp(brakeForce[3], rearRatio * Dynamics().brakes.handbrakeForce, Handbrake());

		for (int i = 0; i < m_wheels.Size(); i++)
		{
			if (brakeForce[i] > 0.f)
			{
				FloatVR brakingTorque;

				if (m_wheels[i].OnGround_Step()) 
				{
					brakingTorque = brakeForce[i] * fy
										* Wheel(i).BackwardsGripScale()
										* Wheel(i).Radius();
				}
				else
				{
					brakingTorque = brakeForce[i] * Wheel(i).GetRotor().MI() * 10.f;
				}

				Normal3R x = Wheel(i).GetDriveAxis();
				Vector3R newAngularMomentum = Wheel(i).GetRotor().AngularMomentum() + derivative.parts[WheelIndex(i)].torque * TimeStep();
				Vector3R newAngularVelocity = newAngularMomentum / Wheel(i).GetRotor().MI();
				FloatVR w = -(newAngularVelocity | x);
				FloatVR w1 = Wheel(i).AngularVelocity1D_Step();

				FloatVR scale = Wheel(i).GetRotor().MI() * 0.5f / (TimeStep() * (brakingTorque + 1.f));
				FloatVR torque = ClampedAbs(-w * scale, FloatVR(1.f)) * brakingTorque;
				ApplyTorque(derivative, BodyIndex(), WheelIndex(i), Wheel(i).GetDriveAxis() * torque);
			}
		}
	}

	// axle friction
	for (int wi = 0; wi < m_wheels.Size(); wi++)
	{
		FloatVR rollingFriction = Dynamics().friction.rollingFriction;
		FloatVR rollingFriction2 = Wheel(wi).m_dd.frame.surfaceRollingFriction;

		U32 surfaceFlags = Wheel(wi).m_dd.frame.surfaceFlags;
		rollingFriction2 += PerformanceModifiers().wheelModifiers[wi].rollingFriction;

		// get final angular momentum
		Vector3R angularMomentum = Wheel(wi).GetRotor().AngularMomentum() + derivative.parts[WheelIndex(wi)].torque * TimeStep();
		Vector3R angularVelocity = angularMomentum / Wheel(wi).GetRotor().MI();
		FloatVR axleRotation = angularVelocity | Wheel(wi).GetDriveAxis();
		rollingFriction += rollingFriction2 * Abs(axleRotation);
		FloatVR t = ClampedAbs(axleRotation / TimeStep(), rollingFriction / Wheel(wi).Inertia()) * Wheel(wi).Inertia();
		Vector3R torque = Wheel(wi).GetDriveAxis() * -t;

		ApplyTorque(derivative, WheelIndex(wi), 0, torque);
	}
}

//Debug::Float g_airLiftTorqueCar("RacingPhysics.Cars.Jumps.LiftTorque", 0.f, Debug::Range(0.f, 1.f));
//Debug::Float g_airLiftDampingCar("RacingPhysics.Cars.Jumps.LiftDamping", 0.1f, Debug::Range(0.f, 1.f));
//Debug::Float g_airLiftOffsetCar("RacingPhysics.Cars.Jumps.LiftOffset", 0.2f, Debug::Range(0.f, 1.f));
//Debug::Float g_airDownforceCar("RacingPhysics.Cars.Jumps.Downforce", 0.f, Debug::Range(-1.f, 1.f));
DECLARE_DEBUG_INT(g_debugRotationalAirFrictionNumWheels, "RacingPhysics.Cars.NumWheelsOffGroundForRollingAirFriction", 2, Debug::Range(0, 4), Debug::Step(1));

void Car::EvaluateFrictionForces(Derivative& derivative)
{
	//PERF_EVENT("RacingPhysics.Car.EvaluateFrictionForces");

	// thrust
	{
		Vector3R thrust = GetHeading() * Controls().throttle * Mass() * (g_debugThrust + ForcedAcceleration());
		ApplyExternalForce(derivative, BodyIndex(), thrust, CofG());
	}

	// air friction
	{
		Vector3R v = Body().Velocity();
		Vector3R frictionForce = -v * Magnitude(v) * Dynamics().friction.airFriction;
		
		float slipStream = Clamped(SlipStream(), 0.f, 1.f);
		frictionForce *= 1.f - slipStream;
		frictionForce *= PerformanceModifiers().airFrictionScale;

		ApplyExternalForce(derivative, BodyIndex(), frictionForce, CofG());
		m_dd.frame.airFrictionForce.Accumulate(frictionForce * AccumulationFactor());
	}

	// rotational air friction
	if (NumWheelsOnGround() <= (int)g_debugRotationalAirFrictionNumWheels)
	{
		Vector3R v = Body().AngularVelocity();
		Vector3R frictionTorque = -v * Dynamics().friction.rotationalAirFriction;

		ApplyExternalTorque(derivative, BodyIndex(), frictionTorque);
	}

	// self-righting
	/*if (Air_Step() && Body().GetAxis().AxisY().Y() > 0.f)
	{
		Normal3R up = Rotated(Constants<Normal3R>::Y(), GetRight(), (float) g_airLiftOffsetCar);
		Vector3R liftTorque = (Body().GetAxis().AxisY() ^ up) * (float) g_airLiftTorqueCar;
		Vector3R dampingTorque = -Body().AngularVelocity() * (float) g_airLiftDampingCar;
		Vector3R downforce = -Constants<Normal3R>::Y() * (float) g_airDownforceCar;

		ApplyExternalTorque(derivative, BodyIndex(), (liftTorque + dampingTorque) * Mass() * PhysicsConstants::GRAVITY);
		ApplyExternalForce(derivative, BodyIndex(), downforce * Mass() * PhysicsConstants::GRAVITY, Body().CofG());
	}*/
}

void Car::CalculateSteerLimits(float& steerLimitL, float& steerLimitR, float& steerCentre, bool step) const
{
	float turnSteerLimit;

	if (step)
		turnSteerLimit = TurnSteerLimit_Step();
	else
		turnSteerLimit = CalculateTurnSteerLimit();

	float speed = GetGroundSpeed_LocalZ();

	steerLimitL = -Dynamics().steering.steerLimit;
	steerLimitR = Dynamics().steering.steerLimit;

	if (speed > 0.f)
	{
		float maxTyreSlip = Dynamics().tyres.frontTyres.transverseTyreSlipScale * Dynamics().steering.steerDirect;
		float slipScale = maxTyreSlip * (speed + Dynamics().tyres.frontTyres.slipSpeed) / speed;
		float slipSteerLimit = CalculateSlipSteerOffset();

		float tL = Maths::Clamped(1.f + 2.f * (turnSteerLimit + slipSteerLimit), Dynamics().steering.slideRecovery * 2.f - 1.f, 1.f);
		steerLimitL = slipSteerLimit - slipScale * tL;
		steerLimitL = Min(steerLimitL, -turnSteerLimit);
		steerLimitL = Max(steerLimitL, -Dynamics().steering.steerLimit);

		float tR = Maths::Clamped(1.f + 2.f * (turnSteerLimit - slipSteerLimit), Dynamics().steering.slideRecovery * 2.f - 1.f, 1.f);
		steerLimitR = slipSteerLimit + slipScale * tR;
		steerLimitR = Max(steerLimitR, turnSteerLimit);
		steerLimitR = Min(steerLimitR, Dynamics().steering.steerLimit);

		steerCentre = slipSteerLimit;
	}
	else
	{
		steerCentre = 0.f;
	}
}

void Car::EvaluateNetworkForces(Derivative& derivative)
{
	//PERF_EVENT("RacingPhysics.Car.EvaluateNetworkForces");

	if (GetControlType() != RacingPhysics::Vehicle::CONTROL_TYPE_NETWORK || !m_dd.predictedNetworkState.networkStateValid)
		return;

	if (!g_ApplyNetworkForces)
		return;

	//int framesSinceLastUpdate = PhysicsTimer::Instance().FramesSinceRaceStart() - m_networkState.framesSinceRaceStart;
	
	//if (framesSinceLastUpdate > 6)
	//	forceScale = 6.f / (float) framesSinceLastUpdate;

	Vector3R predictedVelocity = m_dd.predictedNetworkState.velocity + m_dd.predictedNetworkState.acceleration * DerivativeTimeOffset();
	Point3R predictedPosition = m_dd.predictedNetworkState.axis.Position() + predictedVelocity * DerivativeTimeOffset();

	Vector3R dp = predictedPosition - Body().GetAxis().Position();
	Vector3R dv = predictedVelocity - Body().Velocity();

	Axis correctedAxis = m_dd.predictedNetworkState.axis.Rotated(Vector3R(m_dd.predictedNetworkState.angularVelocity * DerivativeTimeOffset()));
	Matrix33R r = ~Body().GetAxis().Matrix3() * correctedAxis.Matrix3();
	Quaternion q((RotationMatrix&) r);
	q.Normalise();
	float phi = acos(q.W()) * 2.f;
	Vector3R dav = m_dd.predictedNetworkState.angularVelocity - Body().AngularVelocity();

	if (Magnitude(dp) < 0.1f 
		&& Magnitude(m_dd.predictedNetworkState.velocity) < 0.5f
		&& Magnitude(dv) < 0.5f
		&& Maths::Abs(phi) < 0.1f
		&& Magnitude(m_dd.predictedNetworkState.angularVelocity) < 0.5f
		&& Magnitude(dav) < 0.5f)
		return;

	float timeFactor = Clamped(1.f - m_dd.predictedNetworkState.framesPredicted / 60.f, 0.f, 1.f);

	if (Magnitude(dp) > 0.01f)
	{
		Vector3R networkForce = dp * GetNetworkForces().m_Spring + dv * GetNetworkForces().m_Damping;

		float s = Magnitude(networkForce);
		if (s > GetNetworkForces().m_MaxForce)
			networkForce *= GetNetworkForces().m_MaxForce / s;

		if (Air_Step())
			networkForce.Y(Min(networkForce.Y(), 0.f));
		else
			networkForce -= (networkForce | Body().GetAxis().AxisY()) * Body().GetAxis().AxisY();

		Vector3R f = networkForce * Mass() * (1.f + DownforceFactor_Step()) * PhysicsConstants::GRAVITY * g_networkForceScale * timeFactor;
		m_dd.frame.networkForce.Accumulate(f * AccumulationFactor());

		ApplyExternalForce(derivative, BodyIndex(), f, Body().CofG());
	}

	if (phi > 0.01f)
	{
		Normal3R v = Normalised(q.A());
		Vector3R da = v * phi;

		Vector3R networkTorque = da * GetNetworkForces().m_TorqueSpring + dav * GetNetworkForces().m_TorqueDamping;

		float s = Magnitude(networkTorque);
		if (s > GetNetworkForces().m_TorqueMaxForce)
			networkTorque *= GetNetworkForces().m_TorqueMaxForce / s;

		Vector3R t = networkTorque * Body().GetAxis().Global(Body().MI());

		ApplyExternalTorque(derivative, BodyIndex(), t * (1.f + DownforceFactor_Step()) * PhysicsConstants::GRAVITY * g_networkTorqueScale * timeFactor);
	}
}

void Car::EvaluateConstraintForces(Derivative& derivative, float dt, bool constrainPositions)
{
	//PERF_EVENT("RacingPhysics.Car.EvaluateConstraintForces");
	if (g_updatePhysics == false)
		return;

	RP_ASSERT(Validate());

	State oldState;
	GetState(oldState);

	float steerLimitL, steerLimitR, steerCentre;
	CalculateSteerLimits(steerLimitL, steerLimitR, steerCentre, !constrainPositions);

	if (!constrainPositions)
		UpdateVelocities(derivative, dt);
	else
		ArticulatedBody::Stop();

	ConstraintSolver& solver(m_constraintSolver);

	solver.Initialise(NumRigidBodies() + NumRotors(), 20);

	if (constrainPositions)
		solver.SetVelocityCorrection(dt);

	for (int i = 0; i < NumRigidBodies(); i++)
		solver.AddRigidBody(&GetRigidBody(i));

	for (int i = 0; i < NumRotors(); i++)
		solver.AddRotor(&GetRotor(i));

	int suspensionConstraints = 0; // 3 max
	int transverseRearConstraints = 0; // 1 max
	int transverseFrontConstraints = 0; // 1 max
	int longitudinalRightConstraints = 0; // 1 max
	int longitudinalLeftConstraints = 0; // 1 max
	int totalStaticGroundConstraints = 0; // 3 max

	int ci_wheelGroundFrictionX[4];
	int ci_wheelGroundFrictionZ[4];

	for (int i = 0; i < 4; i++)
		ci_wheelGroundFrictionX[i] = ci_wheelGroundFrictionZ[i] = -1;

	// wheel constraints
	int order1[] = {0, 1, 2, 3};
	int order2[] = {1, 0, 3, 2};

	for (int i = 0; i < m_wheels.Size(); i++)
	{
		int wi = (m_dd.steer > 0.f) ? order1[i] : order2[i];

		Normal3R suspensionAxis = Wheel(wi).CalculateSuspensionAxis(Body().GetAxis());
		Normal3R bodyX = Body().GetAxis().AxisX();
		Normal3R bodyZ = Normalised(bodyX ^ suspensionAxis);
		Normal3R wheelX = Wheel(wi).GetRotationMatrix().AxisX();
		Normal3R wheelZ = Normalised(wheelX ^ suspensionAxis);
		Normal3R wheelY = Normalised(wheelZ ^ wheelX);

		solver.SetDefaultRigidBodies(WheelIndex(wi), 0);

		solver.ConstrainAxis(wheelZ, wheelY, suspensionAxis);

		if (wi > 1) // rear wheels
		{
			Normal3R bodyZ = Body().GetAxis().AxisZ();
			solver.ConstrainAxis(suspensionAxis, wheelZ, bodyZ);
		}

		// suspension limits
		//if (!constrainPositions && Wheel(wi).OnGround_Step())
		if (!constrainPositions && suspensionConstraints < 3)
		{
			float su = Wheel(wi).SuspensionDynamics().movementUp;
			float s = 0.f;
			
			if (Wheel(wi).OnGround_Step())
			{
				s = Wheel(wi).SuspensionDistance_Step();
			
				if (s > su)
				{
					suspensionConstraints++;

					Point3R p = Wheel(wi).RestPosition_Step();
					Normal3R groundNormal = Wheel(wi).GroundNormal_Step();

					solver.ConstrainPositions(groundNormal, p + groundNormal * s, p + groundNormal * su, 0, -1);
					//solver.AddConstraintForce(groundNormal, p, 0.f, 0, -1);
					solver.SetConstraintMinMax(0.f, FLT_MAX);
				}
			}
		}

		// static friction
		if (!constrainPositions)
		{
			if (Wheel(wi).OnGround_Step())
			{
				float normalForce = (Wheel(wi).GroundForce() | Wheel(wi).GroundNormal_Step()) * Wheel(wi).TransverseGripScale() * dt;

				if (normalForce > 0.f)
				{
					float fx = Wheel(wi).StaticFrictionX_Step() * normalForce;
					float fz = Wheel(wi).StaticFrictionZ_Step() * normalForce;

					if (fx > 1.f) 
					{
						int count;

						if (wi > 1)
							count = ++transverseRearConstraints;
						else
							count = ++transverseFrontConstraints;

						if (count == 1)
						{
							if (totalStaticGroundConstraints < 3)
							{
								totalStaticGroundConstraints++;
								ci_wheelGroundFrictionX[wi] = solver.GetNumConstraints();

								// transverse force
								solver.AddConstraintForce(Wheel(wi).GroundX_Step(), 
															Wheel(wi).GroundPosition_Step(),
															(Wheel(wi).GroundVelocity_Step() | Wheel(wi).GroundX_Step()),
															0, -1);
								
								solver.SetConstraintMinMax(-fx, fx);
							}
						}
					}

					if (fz > 1.f)
					{
						int count;

						if (wi % 2 == 0)
							count = ++longitudinalLeftConstraints;
						else
							count = ++longitudinalRightConstraints;

						if (count == 1)
						{
							if (totalStaticGroundConstraints < 3)
							{
								totalStaticGroundConstraints++;
								ci_wheelGroundFrictionZ[wi] = solver.GetNumConstraints();

								// longitudinal force
								solver.AddConstraintForce(Wheel(wi).GroundZ_Step(), 
															Wheel(wi).GroundPosition_Step(),
															(Wheel(wi).GroundVelocity_Step() | Wheel(wi).GroundZ_Step()),
															0, -1);

								solver.SetConstraintMinMax(-fz, fz);
							}
						}
					}
				}
			}
		}
	}

	// steering
	{
		// constrain steering angle of wheel 0
		Normal3R suspensionAxis = Wheel(0).CalculateSuspensionAxis(Body().GetAxis());
		Normal3R bodyX = Body().GetAxis().AxisX();
		Normal3R bodyZ = Normalised(bodyX ^ suspensionAxis);
		Normal3R wheelX = Wheel(0).GetRotationMatrix().AxisX();
		Normal3R wheelZ = Normalised(wheelX ^ suspensionAxis);

		float steeringAngle = Maths::ArcSin(wheelZ | (suspensionAxis ^ bodyZ));

		if (steeringAngle <= steerLimitL)
		{
			Normal3R rotatedBodyZ = Rotated(bodyZ, suspensionAxis, steerLimitL);

			solver.SetDefaultRigidBodies(WheelIndex(0), 0);
			solver.ConstrainAxis(suspensionAxis, wheelZ, rotatedBodyZ);

			if (!constrainPositions)
				solver.SetConstraintMinMax(0.f, FLT_MAX);
		}
		else if (steeringAngle >= steerLimitR)
		{
			Normal3R rotatedBodyZ = Rotated(bodyZ, suspensionAxis, steerLimitR);

			solver.SetDefaultRigidBodies(WheelIndex(0), 0);
			solver.ConstrainAxis(suspensionAxis, wheelZ, rotatedBodyZ);

			if (!constrainPositions)
				solver.SetConstraintMinMax(-FLT_MAX, 0.f);
		}
		
		// attach wheel 1 to wheel 0
		Normal3R wheelZ0 = Normalised(Wheel(0).GetRotationMatrix().AxisX() ^ suspensionAxis);
		Normal3R wheelZ1 = Normalised(Wheel(1).GetRotationMatrix().AxisX() ^ suspensionAxis);

		solver.SetDefaultRigidBodies(WheelIndex(0), WheelIndex(1));
		solver.ConstrainAxis(suspensionAxis, wheelZ0, wheelZ1);
	}

	// engine
	{
		Normal3R bodyZ = Body().GetAxis().AxisZ();
		Normal3R bodyY = Body().GetAxis().AxisY();
		Normal3R bodyX = Body().GetAxis().AxisX();
		Normal3R engineX = Engine().GetRotationMatrix().AxisX();
		Normal3R engineZ = Normalised(engineX ^ bodyY);
		Normal3R engineY = Normalised(engineZ ^ engineX);

		solver.SetDefaultRigidBodies(EngineIndex(), 0);

		if (Dynamics().engine.transverse)
		{
			solver.ConstrainAxis(bodyZ, engineY, bodyY);
			solver.ConstrainAxis(bodyY, engineZ, bodyZ);
		}
		else
		{
			solver.ConstrainAxis(bodyX, engineY, bodyY);
			solver.ConstrainAxis(bodyY, engineZ, bodyX);
		}
	}

	bool solved = solver.Solve();
	
	if (solved)
	{
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
		}

		if (!constrainPositions)
		{
			for (int wi = 0; wi < 4; wi++)
			{
				if (ci_wheelGroundFrictionX[wi] > -1)
					m_wheels[wi].m_dd.frame.groundFriction.Accumulate(solver.GetConstraint(ci_wheelGroundFrictionX[wi]) * AccumulationFactor() / dt);

				if (ci_wheelGroundFrictionZ[wi] > -1)
					m_wheels[wi].m_dd.frame.groundFriction.Accumulate(solver.GetConstraint(ci_wheelGroundFrictionZ[wi]) * AccumulationFactor() / dt);
			}
		}
	}
	else
		LOGWARN(g_RacingPhysicsLog, "Failed to solve car constraints.");

	RestoreState(oldState);

#ifndef CFG_SPEED
	// debug
	if (!constrainPositions)
	{
		State oldState;
		GetState(oldState);

		for (int i = 0; i < NumRigidBodies(); i++)
			GetRigidBody(i).UpdateVelocities(derivative.parts[i], dt);

		CheckConstraints();

		RestoreState(oldState);
	}
#endif

	solver.Clear();

	RP_ASSERT(Validate());
}

void Car::CheckConstraints() 
{
	if (Wheel(0).OnGround_Step())
	{
		Point3R p = Wheel(0).GroundPosition_Step();
		Normal3R n = Wheel(0).GroundX_Step();

		FloatVR d = Body().PointVelocity(p) | n;

		int breakhere = 0;
	}
}

#endif // SPU_NET_TASK

#if !PLATFORM_SPU

int g_carSpuDebug = 0;

void Car::GetDmaData(void** pDmaData)
{
	m_dd.debug = g_carSpuDebug;

	// return a pointer to the dma data
	*pDmaData = &m_dd;

	// store the rest of the dma data inside the class
	void** p = &m_dd.dmaPointers[0];

	Vehicle::GetDmaData(p++);
	ArticulatedBody::GetDmaData(p++);

	for (int i = 0; i < 4; i++)
	{
		m_wheels[i].GetDmaData(p++);
	}

	m_transmission.GetDmaData(p++);
	m_engine.GetDmaData(p++);

	CarDynamicsXt::GetDmaData(m_pSettings, p++);

	*(p++) = &m_LineCache;
}

#else

void Car::GetDmaData(void** pDmaData) {}

#endif

void Car::SpuFetch(const void* pDmaData)
{
	// get the car data block
	Spu::Fetch(&m_dd, pDmaData, sizeof(DmaData));

	// get the rest of the data
	void** p = &m_dd.dmaPointers[0];

	Vehicle::SpuFetch(*p++);
	ArticulatedBody::SpuFetch(*p++);

	for (int i = 0; i < 4; i++)
	{
		m_wheels[i].SpuFetch(*p++);
	}

	m_transmission.SpuFetch(*p++);
	m_engine.SpuFetch(*p++);

	m_pCarDynamics = CarDynamicsXt::SpuFetch(*p++);

	if (m_dd.debug)
		DebugBreak();
}

void Car::SpuStore(void* pDmaData) const
{
	//DebugBreak();

	Spu::Store(pDmaData, &m_dd, sizeof(DmaData));

	void** p = (void**) &m_dd.dmaPointers[0];

	Vehicle::SpuStore(*p++);
	ArticulatedBody::SpuStore(*p++);

	for (int i = 0; i < 4; i++)
	{
		m_wheels[i].SpuStore(*p++);
	}

	m_transmission.SpuStore(*p++);
	m_engine.SpuStore(*p++);

	p++;

	if (m_LineCache.m_CacheSize > 0)
	{
		void* pLineCache = *(p++);

		Collision::C_LineCollisionCache lineCache(0, 0);
		Spu::Fetch(&lineCache, pLineCache, sizeof(lineCache));

		assert(lineCache.m_CacheSize == m_LineCache.m_CacheSize);
		lineCache.m_Low = m_LineCache.m_Low;
 		lineCache.m_High = m_LineCache.m_High;

		Spu::Store(pLineCache, &lineCache, sizeof(lineCache));
		Spu::Store(lineCache.m_pTriangles, m_LineCache.m_pTriangles, m_LineCache.m_CacheSize * sizeof(Collision::C_CachedTriangle));
	}
}

}
