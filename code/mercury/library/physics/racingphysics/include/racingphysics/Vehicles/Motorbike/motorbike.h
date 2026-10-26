#ifndef RP_MOTORBIKE_H
#define RP_MOTORBIKE_H
#ifdef _MSC_VER
#pragma once
#endif

#include "racingphysics\physics\rigidbody.h"
#include "racingphysics\physics\articulatedbody.h"
#include "racingphysics\physics\constraints.h"
//#include "racingphysics\collisions\collisionmesh.h"
//#include "racingphysics\collisions\collisionspheres.h"
#include "racingphysics\vehicles\debugsliprecorder.h"
#include "racingphysics\vehicles\vehicle.h"
//#include "racingphysics\vehicles\vehiclemanager.h"
#include "motorbikesettings.h"

#ifdef DEBUG_RACING_PHYSICS
#pragma message("DEBUG_RACING_PHYSICS is enabled")
#endif

////////////////////////////////////////////////////////////

namespace RacingPhysics
{

class Motorbike;
class MotorbikeEngine;

class MotorbikeWheel
{
public:
	MotorbikeWheel() {}

	// initialise
	void Initialise(int index, Motorbike* pMotorbike, RigidBody* pRigidBody);
	void Shutdown();
	
	// physics
	const MotorbikeDynamicsXt& Dynamics() const;
	void BeginFrame();
	void EndFrame();
	void BeginIntegrate();
	void EndIntegrate();
	void PutWheelOnGround();
	void CalculateSlip();
	void CalculateDynamicFrictionForce(Vector3R& force, Vector3R& torque);
	float GetTyreForceScale(float slip1, float slip2, float maxSlip) const;
	float SlipScaleFactor() const;
	void SetSuspensionDistance(float s) {m_step.suspensionDistance = s;}
	void ClearStepVariables();

	// rigid body
	RigidBody& Body() {return *m_pRigidBody;}
	const RigidBody& Body() const {return *m_pRigidBody;}

	// constants
	__forceinline bool Driven() const {return m_driven;}
	__forceinline float Radius() const {return Dynamics().geometry.WheelRadius(m_index);}
	__forceinline float TransverseGripScale() const {return Dynamics().tyres.grip.yScale * SurfaceGripScale() * Dynamics().tyres.GripScale(m_index) * Dynamics().tyres.transverseGripScale;}
	__forceinline float ForwardsGripScale() const {return Dynamics().tyres.grip.yScale * SurfaceGripScale() * Dynamics().tyres.GripScale(m_index) * Dynamics().tyres.forwardsGripScale;}
	__forceinline float BackwardsGripScale() const {return Dynamics().tyres.grip.yScale * SurfaceGripScale() * Dynamics().tyres.GripScale(m_index) * Dynamics().tyres.backwardsGripScale;}
	__forceinline float TransverseGripScaleDry() const {return Dynamics().tyres.grip.yScale * Dynamics().tyres.GripScale(m_index) * Dynamics().tyres.transverseGripScale;}
	__forceinline float ForwardsGripScaleDry() const {return Dynamics().tyres.grip.yScale * Dynamics().tyres.GripScale(m_index) * Dynamics().tyres.forwardsGripScale;}
	__forceinline float BackwardsGripScaleDry() const {return Dynamics().tyres.grip.yScale * Dynamics().tyres.GripScale(m_index) * Dynamics().tyres.backwardsGripScale;}
	__forceinline float CamberForceScale() const {return Dynamics().tyres.CamberForceScale(m_index);}
	__forceinline float Inertia() const {return m_inertia;}
	float TransverseTyreSlip() const;
	float LongitudinalTyreSlip() const;

	// state
	__forceinline const Point3R& Position() const {return m_pRigidBody->Position();}
	__forceinline const Vector3R Velocity() const {return m_pRigidBody->Velocity();}
	float AngularVelocity1D() const;
	float RotationScale() const;

	// frame
	__forceinline const Vector3R& GroundForce() const {return m_frame.groundNormalForce.GetValue();}
	__forceinline const Vector3R& GroundFriction() const {return m_frame.groundFriction.GetValue();}
	__forceinline const Vector3R& GroundTorque() const {return m_frame.groundTorque.GetValue();}
	__forceinline const Vector3R& SmoothedGroundForce() const {return m_frame.smoothGroundForce;}
	__forceinline const Point3R& GroundPosition() const {RP_ASSERT(OnGround()); return MakePoint(m_frame.groundPosition.GetValue());}
	__forceinline bool OnGround() const {RP_ASSERT((char&) m_step.onGround == -1); return m_frame.onGround;}
	__forceinline float WheelSpin() const {RP_ASSERT(!Maths::Finite(m_step.wheelSpin)); return m_frame.wheelSpin.GetValue();}
	__forceinline float SlipAngle() const {RP_ASSERT(!Maths::Finite(m_step.slipAngle)); return m_frame.slipAngle.GetValue();}
	__forceinline float SlipAngleWithCamber() const {RP_ASSERT(!Maths::Finite(m_step.slipAngle)); return m_frame.slipAngleWithCamber.GetValue();}
	__forceinline const Normal3R& GroundNormal() const {RP_ASSERT(OnGround()); return MakeNormal(m_frame.groundNormal.GetValue());}
	__forceinline const S32 SurfaceType() const {RP_ASSERT(OnGround()); return m_frame.surfaceType;}
	__forceinline const S32 SurfaceModifier() const {return m_frame.surfaceModifier;}

private:
	FRIEND_CLASS Motorbike;
	FRIEND_CLASS MotorbikeEngine;

	// step
	__forceinline bool OnGround_Step() const {return Checked(m_step.onGround);}
	__forceinline const Normal3R& GroundNormal_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.groundNormal);}
	__forceinline const Normal3R& GroundX_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.groundX);}
	__forceinline const Normal3R& GroundZ_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.groundZ);}
	__forceinline const Normal3R& BodyGroundZ_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.bodyGroundZ);}
	__forceinline const Normal3R& BodyGroundX_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.bodyGroundX);}
	__forceinline const Point3R& GroundPosition_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.groundPosition);}
	__forceinline const Point3R& GroundContactPosition_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.groundContactPosition);}
	__forceinline float SlipAngle_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.slipAngle);}
	__forceinline float SlipFactor_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.slipFactor);}
	__forceinline float SlipAngleWithCamber_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.slipAngleWithCamber);}
	__forceinline float CamberAngle_Step() const {return Checked(m_step.camberAngle);}
	__forceinline float WheelSpin_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.wheelSpin);}
	__forceinline float TractionControlAV_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.tractionControlAV);}
	__forceinline float BrakingAV_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.brakingAV);}
	__forceinline float SuspensionDistance_Step() const {return Checked(m_step.suspensionDistance);}
	__forceinline float StaticFrictionX_Step() const {return Checked(m_step.staticFrictionX);}
	__forceinline float StaticFrictionZ_Step() const {return Checked(m_step.staticFrictionZ);}

	__forceinline float SurfaceGripScale() const {return Checked(m_frame.surfaceGripScale);}

	void SetMassVariables();
	float AccumulationFactor() const;

	const float& Checked(const float& f) const {RP_ASSERT(Maths::Finite(f)); return f;}
	const bool& Checked(const bool& b) const {RP_ASSERT((char&) b == 0 || (char&) b == 1); return b;}
	const Normal3R& Checked(const Normal3R& n) const {RP_ASSERT(Maths::Finite(n)); return n;}
	const Point3R& Checked(const Point3R& p) const {RP_ASSERT(Maths::Finite(p)); return p;}

	// used by network prediction
	void GetGroundIntersection(CollisionCache::Contact& contact, const Point3R& position, const Normal3R& normal) const;

	// constant
	bool m_driven;
	int m_index;
	Motorbike* m_pMotorbike;
	RigidBody* m_pRigidBody;
	float m_inertia;

	// valid during derivative calculation
	class Step
	{
		FRIEND_CLASS MotorbikeWheel;

		Normal3R groundNormal;
		Normal3R groundX;
		Normal3R groundZ;
		Normal3R bodyGroundZ;
		Normal3R bodyGroundX;
		Point3R groundPosition;
		Point3R groundContactPosition;

		// slip
		float slipAngle;
		float slipAngleWithCamber;
		float wheelSpin;
		float tractionControlAV;
		float brakingAV;
		float relaxedSlipAngle;
		float slipFactor;
		float slip;
		float camberAngle;
		float maxGripScale;
		float staticFrictionX;
		float staticFrictionZ;
		float suspensionDistance;
		bool onGround;
	};

	Step m_step;

	// constants over the whole frame
	struct Frame
	{
		Accumulator<Vector3R> groundNormalForce;
		Accumulator<Vector3R> groundFriction;
		Accumulator<Vector3R> groundTorque;
		Accumulator<Vector3R> groundPosition;
		Accumulator<Vector3R> groundNormal;
		Vector3R smoothGroundForce;
		Accumulator<float> wheelSpin;
		Accumulator<float> slipAngle;
		Accumulator<float> slipAngleWithCamber;
		Accumulator<float> slipScaleFactor; // smoothed
		float surfaceGripScale;

		S32 surfaceType;
		S32 surfaceModifier;
		bool onGround;
		bool slipScaleFactorValid;
	};

	Frame m_frame;

	DebugSlipRecorder m_debugSlipRecorder;
};

////////////////////////////////////////////////////////////

struct MotorbikeControls
{
	float steer;
	float lean;
	float throttle;
	float fullBrakes;
	float rearBrake;
	bool reverse;
	bool gearUp;
	bool gearDown;
	bool isWheel;
	bool horn;

	MotorbikeControls() : isWheel(false) {Clear();}

	void Clear()
	{
		steer = 0.f;
		throttle = 0.f;
		fullBrakes = 0.f;
		rearBrake = 0.f;
		lean = 0.f;
		reverse = false;
		gearUp = false;
		gearDown = false;
		horn = false;
	}

	// stop it moving (but can rev engine)
	void Hold()
	{
		steer = 0.f;
		rearBrake = 1.f;
		fullBrakes = 0.f;
		lean = 0.f;
		reverse = false;
		gearUp = false;
		gearDown = false;
	}

	bool Validate() const
	{
		RP_ASSERT(steer >= -1.f && steer <= 1.f);
		RP_ASSERT(lean >= -1.f && lean <= 1.f);
		RP_ASSERT(throttle >= 0.f && throttle <= 1.f);
		RP_ASSERT(fullBrakes >= 0.f && fullBrakes <= 1.f);
		RP_ASSERT(rearBrake >= 0.f && rearBrake <= 1.f);
		return true;
	}
};

////////////////////////////////////////////////////////////

class MotorbikeFeedback
{
public:
	void Initialise(const Motorbike* p) {m_pMotorbike = p; Clear();}

	void Clear()
	{
		m_rumbleR = 0.f;
		m_rumbleL = 0.f;
		m_gearRumble = 0.f;
		m_collisionRumble = 0.f;
	}

	void Update(bool rumbleEnabled);
	void HandleCollision(float impulse);
	void HandleGearChange(float impulse);
	void Stop() {m_rumbleR = 0; m_rumbleL = 0;}

	float RumbleL() const {return m_rumbleL;}
	float RumbleR() const {return m_rumbleR;}

	float Collision() const {return m_collisionRumble;}

private:
	const MotorbikeDynamicsXt& Dynamics() const;
	void AddRumble(float value, float balance);
	void AddRumble(float value, float cutOff, float scale, float balance);

	float m_rumbleR;
	float m_rumbleL;
	float m_collisionRumble;
	float m_gearRumble;
	const Motorbike* m_pMotorbike;
};

////////////////////////////////////////////////////////////

class MotorbikeEngine
{
public:
	MotorbikeEngine() {}

	// initialise
	void Initialise(Motorbike* pMotorbike, Rotor* pRigidBody);

	void BeginIntegrate();
	void EndIntegrate();

	// physics
	void EvaluateControlForces(ArticulatedBody::Derivative& derivative, float maxTotalEngineTorque, float loadTorque, float clutchPowerCut);

	// information
	const MotorbikeDynamicsXt& Dynamics() const;
	const Rotor& Body() const {return *m_pRotor;}
	Rotor& Body() {return *m_pRotor;}
	float RPM() const;
	float AngularVelocity1D() const;
	float GearRatio() const {return GearRatio(m_currentGear);}
	float GearRatio(int gear) const;
	float ClutchEngaged() const {return m_clutchEngaged;}
	int Gear() const {return m_currentGear;}
	int NextGear() const {return m_gearDest;}
	float TractionControl() const {return m_tractionControl.GetValue();}
	float AdjustedThrottle() const {return m_adjustedThrottle.GetValue();}
	
private:
	//void GearUp();
	//void GearDown();

	Motorbike* m_pMotorbike;
	Rotor* m_pRotor;

	int m_currentGear;
	int m_gearDest;
	float m_clutchEngaged;
	float m_gearChangeClutch;
	float m_gearChangeClutchTime;
	bool m_powerCut;
	bool m_gearChange; 
	Accumulator<float> m_tractionControl;
	Accumulator<float> m_adjustedThrottle;
};

////////////////////////////////////////////////////////////

class Motorbike : public Vehicle, private ArticulatedBody
{
public:
	void Initialise(const VehicleInitialiseInfo& initialiseInfo);
	void Shutdown();
	virtual void DisplayDebug() const;

	// initialise
	virtual void SetAxis(const Axis& a, bool overwriteOldAxis = true);
	virtual bool PutOnGround();
	virtual void Stop();
	virtual void SetPosition(const Point3R& p);
	void SetAxis(const Axis& a, float steeringAngle, float suspensionOffset0, float suspensionOffset1);

	const MotorbikeDynamicsXt& Dynamics() const {return *m_pSettings;}

	/////////////////////////////////////////////////////////////

	static const Motorbike* GetMotorbikePointer(const Vehicle* v) {return (v->GetVehicleType() == Vehicle::VEHICLE_TYPE_BIKE) ? (Motorbike*) v : 0;}
	static Motorbike* GetMotorbikePointer(Vehicle* v) {return (v->GetVehicleType() == Vehicle::VEHICLE_TYPE_BIKE) ? (Motorbike*) v : 0;}

	/////////////////////////////////////////////////////////////////

	// geometry
	Normal3R ForkAxis_Local() const;
	float ForkOffset() const;
	Point3R ForkPosition_Local() const;
	Point3R RiderPosition_Local() const;
	Point3R RestingWheelPosition_Local(int i, float offsetY = 0.f) const;
	void CalculateAxes(const Axis& body, float steeringAngle, float suspensionOffset0, float suspensionOffset1, // input
						Axis& forkAxis, Axis& frontWheelAxis, Axis& rearWheelAxis) const; // output
	Point3R RestingWheelPosition_Global(int i) const;

	float MassCorrectionFactor() const;
	Point3R CorrectedCofGLocal() const;
	float WheelMass() const {return Maths::Max(Dynamics().mass.kerbMass * Dynamics().mass.wheelMassFraction, 15.f);}
	float SprungMass() const {return Dynamics().mass.kerbMass - 2.f * WheelMass() + Dynamics().rider.riderMass;}
	const MotorbikeWheel& Wheel(int i) const {return m_wheels[i];}
	MotorbikeWheel& Wheel(int i) {return m_wheels[i];}
	const MotorbikeEngine& Engine() const {return m_engine;}
	MotorbikeEngine& Engine() {return m_engine;}
	float CalculateMaxSinRoll(float speed, float gripScale, float slideFactor);
	float CorrectedRoll(float roll) const;
	float CalculateDownforceFactor() const;

	// step (valid during derivative calculation)
	float SinRoll_Step() const {return Checked(m_step.sinRoll);}
	float RollRate_Step() const {return Checked(m_step.dr);}
	float MaxSinRoll_Step() const {return Checked(m_step.maxSinRoll);}
	float DownforceFactor_Step() const {return Checked(m_step.downforceFactor);}
	bool BodyOnGround_Step() const {RP_ASSERT(m_step.bodyOnGround == 0 || m_step.bodyOnGround == 1); return m_step.bodyOnGround != 0;}

	// For kudos calculations.
	// SinRoll = sin(roll angle)
	// relative to horizon (not ground)
	// MaxSinRoll = largest safe value of SinRoll in a stable turn.
	//	corrected for grip, ground normal, rider position, tyre radius and speed.
	float SinRoll() const {RP_ASSERT(!Maths::Finite(m_step.sinRoll)); return m_frame.sinRoll.GetValue();}
	float RollRate() const {RP_ASSERT(!Maths::Finite(m_step.dr)); return m_frame.dr.GetValue();}
	float MaxSinRoll() const {RP_ASSERT(!Maths::Finite(m_step.maxSinRoll)); return m_frame.maxSinRoll;}
	bool BodyOnGround() const {RP_ASSERT((char&) m_step.bodyOnGround == -1); return m_frame.bodyOnGround != 0;}
	float RollCorrection() const {return m_frame.rollCorrection;}
	bool Air() const {return m_frame.air;}
	const Normal3R& SmoothedGroundNormal() const {RP_ASSERT(!m_frame.air); return m_frame.smoothedGroundNormal;}
	float DownforceFactor() const {return m_frame.downforceFactor;}
	float SuspensionSpeed(int wheelIndex) const;
	float SuspensionDistance(int wheelIndex) const;
	
	// vehicle interface
	virtual Vector3R GetVelocity() const {return Body().Velocity();}
	virtual Vector3R GetAngularVelocity() const {return Body().AngularVelocity();}
	virtual Point3R GetPosition() const {return Body().Position();}
	virtual const Axis& GetAxis() const {return Body().GetAxis();}
	//virtual FloatVR GetLargestVelocity() const;
	virtual const Axis& GetOldAxis() const {return m_oldAxis;}
	virtual int GetGear() const {return Engine().Gear();}
	virtual FloatVR GetAirSpeed_GlobalXZ() const {return Speed_GlobalXZ();}
	virtual FloatVR GetAirSpeed_LocalZ() const {return Speed_LocalZ();}
	virtual FloatVR GetGroundSpeed_GlobalXZ() const {RP_ASSERT(!"not supported"); return Speed_GlobalXZ();}
	virtual FloatVR GetGroundSpeed_LocalZ() const {RP_ASSERT(!"not supported"); return Speed_LocalZ();}
	virtual Normal3R GetHeading() const {return -Body().GetAxis().AxisZ();}
	virtual Point3R GetCofG() const {return CofG();}

	// non-virtual versions
	__forceinline const Axis& OldAxis() const {return m_oldAxis;}
	__forceinline int Gear() const {return Engine().Gear();}
	__forceinline float Speed_GlobalXZ() const {return -(Body().Velocity() | Normalised(ComponentMultiply(Body().GetAxis().AxisZ(), Maths::Constants<Vector3R>::XZ())));}
	__forceinline float Speed_LocalZ() const {return -(Body().Velocity() | Body().GetAxis().AxisZ());}
	__forceinline Normal3R Heading() const {return -Body().GetAxis().AxisZ();}

	// controls
	const MotorbikeControls& Controls() const {return m_controls;}
	MotorbikeControls& Controls() {return m_controls;}
	bool Reverse() const {return m_reverse;}
	float Throttle() const {return m_throttle;}
	float ReverseThrottle() const {return m_reverseThrottle;}
	float FullBrakes() const {return m_fullBrakes;}
	float RearBrake() const {return m_rearBrake;}
	float FrontBrake() const {return m_frontBrake;}
	bool AutoReverse() const {return GetControlType() != RacingPhysics::Vehicle::CONTROL_TYPE_AI;}
	bool CheckControlsPressed() const {return m_checkControlsPressed;}
	float LeanX() const {return m_leanX;}
	float LeanZ() const {return m_leanZ;}
	float SteerSmooth() const {return m_steerSmooth;}
	bool RiderPresent() const;
	void SetRiderPresent(bool b);
	virtual void SetHold() {m_controls.Hold();}
	void SetAutoGears(bool b) {m_autoGears = b;}
	bool AutoGears() const {return m_autoGears || GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_AI;}

	// for VehicleManager
	virtual void ClearAllFeedback();
	virtual void BeginFrame(float maxSpeed);
	virtual void EndFrame(const Maths::C_TimeS PCREF time);
	virtual float ConstrainInternal();
	virtual void Integrate(float time);
	virtual void Push(const Point3R& p, const Vector3R& v);
	virtual void ApplyImpulse(const Point3R& p, const Vector3R& impulse, FloatVR torqueFactor);
	virtual void HandleCollision(const Point3R& p, const Vector3R& impulse);
	//virtual void GetCollisionBoundingBox(Point3R& min, Point3R& max) const;
	//virtual float GetCollisionTime(float time, float resolution) const;
	virtual S32 GetNumWheels() const {return 2;}
	virtual bool WheelOnGround(int i) const {return Wheel(i).OnGround();}
	virtual void GetWheelContact(int i, Point3R& position, Normal3R& normal) const {position = Wheel(i).GroundPosition(); normal = Wheel(i).GroundNormal();}
	//virtual U32 GetCollisionFlags() const {return (Air() || BodyOnGround()) ? 0 : COLLISION_FLAG_NOT_GROUND;}

	// articulated body
	virtual void EvaluateDerivative(Derivative& output);
	virtual void BeginIntegrate();
	virtual void EndIntegrate();
	virtual void GetIntegrationType(int& integrationType, int& nSteps);

	// network and replays
	virtual void GetNetworkState(CompressedNetworkState* state) const;
	virtual bool ProcessNetworkState(const CompressedNetworkState& state, bool runCatchUp);
	virtual void GetNetworkControls(CompressedNetworkControls& controlsOut) const;
	virtual void SetNetworkControls(const CompressedNetworkControls& carControlsIn);
	bool NetworkJump(bool position, bool velocity);
	static CompressedNetworkState BlendStatesLinear(const CompressedNetworkState& p_pState0, const CompressedNetworkState& p_pState1, float p_Blend, float dt);
	static Point3R GetPositionFromState(const Vehicle::CompressedNetworkState& vehicleState);

	//[PK] moving to public as I need this in the BikerManager
	//CA - Adding the direction parameter so that we can point the fella in the right direction
	void Reset(const Normal3R& direction, const Point3R& position);

	// Reset but keep it pointing the same way as before.
	void Reset();

	using ArticulatedBody::Mass;
	
	bool Validate() const;

private:
	virtual void XtReload();

	FRIEND_CLASS MotorbikeWheel;
	FRIEND_CLASS MotorbikeEngine;
	FRIEND_CLASS MotorbikeFeedback;

	/////////////////////////////////////////////
	// physics

	void EvaluateTyreForces(Derivative& derivative);
	void EvaluateSpringForces(Derivative& derivative);
	void EvaluateControlForces(Derivative& derivative);
	void EvaluateFrictionForces(Derivative& derivative);
	void EvaluateConstraintForces(Derivative& derivative, float dt, bool constrainPositions);
	void UpdateSuspensionDistance(int i);
	void SetMassVariables();
	void ClearStepVariables();
	void CopyStateToCollisionBody();
	void CopyStateFromCollisionBody();

	/////////////////////////////////////////////
	// information

	const RigidBody& Body() const {return GetRigidBody(0);}
	RigidBody& Body() {return GetRigidBody(0);}
	int BodyIndex() const {return 0;}
	
	const RigidBody& Fork() const {return GetRigidBody(3);}
	RigidBody& Fork() {return GetRigidBody(3);}
	int ForkIndex() const {return 3;}

	const RigidBody& Rider() const {return GetRigidBody(4);}
	RigidBody& Rider() {return GetRigidBody(4);}
	int RiderIndex() const {return 4;}

	int WheelBodyIndex(int i) const {return i + 1;}

	int EngineIndex() const {return 5;}
	
	////////////////////////////////////////////
	// debug

	void CheckConstraints();
	float Checked(const float& f) const {RP_ASSERT(Maths::Finite(f)); return f;}

	////////////////////////////////////////////
	// parts
	Btl::Vector<MotorbikeWheel, Btl::VectorParameters_Array<2> >  m_wheels;
	MotorbikeEngine m_engine;

	const MotorbikeDynamicsXt* m_pSettings;
	int m_dynamicsChangedCounter;
	MotorbikeControls m_controls;
	//COObjectSpheres m_collisionObject;
	MotorbikeFeedback m_feedback;
	RigidBody m_collisionBody;

	ConstraintSolver m_constraintSolver;

	Axis m_oldAxis;

	// controls
	float m_steer;
	float m_steerReduced;
	float m_steerSmooth;
	float m_steerSmoothReduced;
	float m_rollFactor;
	float m_fullBrakes;
	float m_throttle;
	float m_reverseThrottle;
	bool m_reverse;
	float m_leanX;
	float m_leanZ;
	float m_lowSpeedLean;
	float m_rearBrake;
	float m_frontBrake;
	bool m_riderPresent;
	bool m_checkControlsPressed;
	float m_previousSteering;
	float m_leftCollisionTime;
	float m_rightCollisionTime;

	bool m_autoGears;

	// valid during derivative calculation
	struct Step
	{
		float sinRoll;
		float maxSinRoll;
		float dr;
		bool bodyOnGround;
		float downforceFactor;
	};

	Step m_step;

	// constants over the whole frame
	struct Frame
	{
		Normal3R smoothedGroundNormal;
		Accumulator<float> sinRoll;
		Accumulator<float> dr;
		float rollCorrection;
		float maxSinRoll;
		float downforceFactor;
		bool bodyOnGround;
		bool air;
	};

	Frame m_frame;

	/////////////////////////////////////////////
	// network

public:
	struct NetworkState
	{
		Axis axis;
		Vector3R velocity; 
		Vector3R angularVelocity;
		Vector3R acceleration;
		Normal3R groundNormal;
		float steeringAngle;
		int gear;
		int framesSinceRaceStart;
		bool networkStateValid;
		bool useFullPrediction;
		bool air;
	};

	NetworkState m_networkState;
	NetworkState m_predictedNetworkState;

	static void CompressNetworkState(const NetworkState& networkState, CompressedNetworkState& vehicleState);
	static void UncompressNetworkState(const CompressedNetworkState& vehicleState, NetworkState& networkState);
private:

	void NetworkJumpIfNeeded();
	void EvaluateNetworkForces(Derivative& output);
	void GetNetworkState(NetworkState& networkState) const;
	void UpdatePredictedNetworkState(bool constrain);
	void ConstrainNetworkPosition();

	// debug
	Accumulator<Vector3R> m_footForce;
	Accumulator<Vector3R> m_footFriction;
	Accumulator<Vector3R> m_airFrictionForce;
	Accumulator<Vector3R> m_steeringTorque;
	Point3R m_footPosition;
};

///////////////////////////////////////////////////////

inline const MotorbikeDynamicsXt& MotorbikeWheel::Dynamics() const {return m_pMotorbike->Dynamics();}
inline const MotorbikeDynamicsXt& MotorbikeEngine::Dynamics() const {return m_pMotorbike->Dynamics();}
inline const MotorbikeDynamicsXt& MotorbikeFeedback::Dynamics() const {return m_pMotorbike->Dynamics();}

inline float MotorbikeWheel::AccumulationFactor() const {return m_pMotorbike->AccumulationFactor();}

}

#endif // MOTORBIKE_H
