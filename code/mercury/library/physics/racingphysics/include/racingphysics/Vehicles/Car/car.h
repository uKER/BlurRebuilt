#ifndef RP_CAR_H
#define RP_CAR_H
#ifdef _MSC_VER
#pragma once
#endif

#include "racingphysics\physics\rigidbody.h"
#include "racingphysics\physics\articulatedbody.h"
#include "racingphysics\physics\constraints.h"
//#include "racingphysics\collisions\collisionmesh.h"
//#include "racingphysics\collisions\collisionspheres.h"
#include "racingphysics\vehicles\vehicle.h"
//#include "racingphysics\vehicles\vehiclemanager.h"
#include "gravity/narrowphase/LineCollisionCache.h"
#include "carsettings.h"
#include "gravity/importer/importer.h"

#if !PLATFORM_SPU
#include "racingphysics\vehicles\debugsliprecorder.h"
#endif

namespace RacingPhysics
{

class Car;
class CarEngine;
class CarTransmission;

class CarWheel
{
public:
	CarWheel() {}

	// initialise
	void InitialiseConstants(int index, Car* pCar, Rotor* pRotor);
	void Initialise();
	void Shutdown();

	// physics
	const CarDynamicsXt& Dynamics() const;
	const CarDynamicsXt::CarTyreXt& TyreDynamics() const;
	const CarDynamicsXt::CarWheelSuspensionXt& SuspensionDynamics() const;
	void BeginIntegrate();
	void EndIntegrate();
	void EndFrame();
	void PutWheelOnGround();
	void CalculateSlip();
	void CalculateDynamicFrictionForce(Vector3R& force, Vector3R& torque) const;
	FloatVR GetTyreForceScale(FloatVR slip, FloatVR maxSlip) const;
	void SetSuspensionDistance(FloatVR s) {m_step.suspensionDistance = s;}
	void SetCarAxis(const Axis& axis, float steeringAngle);
	float CalculateSteeringAngle() const;
	void SetOnGround(bool b);

	// rotor
	Rotor& GetRotor() {return *m_pRotor;}
	const Rotor& GetRotor() const {return *m_pRotor;}

	Normal3R GetDriveAxis() const {return VEHICLE_REVERSE_Z_SIGN GetRotationMatrix().AxisX();}
	const RotationMatrix& GetRotationMatrix() const {return m_pRotor->GetRotationMatrix();}
	const Axis& GetOldAxis() const {return m_dd.oldAxis;}
	void CacheAxis() { m_dd.oldAxis.SetRotation(GetRotationMatrix()); m_dd.oldAxis.SetPosition(GetWheelPosition());}

	// constants
	__forceinline bool Driven() const {return m_dd.driven;}
	__forceinline const float& Radius() const {return TyreDynamics().radius;}
	float CollisionRadius() const;
	FloatVR TransverseGripScaleDry() const;
	FloatVR ForwardsGripScaleDry() const;
	FloatVR BackwardsGripScaleDry() const;
	FloatVR TransverseGripScale() const;
	FloatVR ForwardsGripScale() const;
	FloatVR BackwardsGripScale() const;
	FloatVR Inertia() const {return m_pRotor->MI();}
	FloatVR TransverseTyreSlip() const;
	FloatVR LongitudinalTyreSlip() const;
	FloatVR RotationScale() const;
	float CalculateStaticLoad() const;
	float GetTyreSaturation(float s) const;

	// frame
	__forceinline const Vector3R& GroundForce() const {return m_dd.frame.groundNormalForce.GetValue();}
	__forceinline const Vector3R& GroundFriction() const {return m_dd.frame.groundFriction.GetValue();}
	__forceinline const Vector3R& GroundTorque() const {return m_dd.frame.groundTorque.GetValue();}
	__forceinline const Vector3R& SmoothedGroundForce() const {return m_dd.frame.smoothGroundForce;}
	__forceinline const Point3R& GroundPosition() const {RP_ASSERT(OnGround()); return MakePoint(m_dd.frame.groundPosition.GetValue());}
	__forceinline bool OnGround() const {RP_ASSERT((char&) m_step.onGround == -1); return m_dd.frame.onGround;}
	__forceinline const float& WheelSpin() const {RP_ASSERT(!Maths::Finite(m_step.wheelSpin)); return m_dd.frame.wheelSpin.GetValue();}
	__forceinline const float& SlipAngle() const {RP_ASSERT(!Maths::Finite(m_step.slipAngle)); return m_dd.frame.slipAngle.GetValue();}
	__forceinline const Normal3R& GroundNormal() const {RP_ASSERT(OnGround()); return MakeNormal(m_dd.frame.groundNormal.GetValue());}
	__forceinline const Vector3R& GroundVelocity() const {RP_ASSERT(OnGround()); return m_dd.frame.groundVelocity.GetValue();}
	__forceinline U32 SurfaceType() const {RP_ASSERT(OnGround()); return m_dd.frame.surfaceFlags;}
	__forceinline U32 SurfaceModifier() const {return m_dd.frame.surfaceModifier;}
	__forceinline U32 UserData() const {RP_ASSERT(OnGround()); return m_dd.frame.userData;}
	__forceinline const Point3R& GetWheelPosition() const {RP_ASSERT(!Maths::Finite(m_step.wheelPosition)); return m_dd.frame.wheelPosition;}
	__forceinline const Vector3R GetVelocity() const;
	__forceinline const float& SuspensionDistance() const {return m_dd.frame.suspensionDistance.GetValue();}
	__forceinline const float& SurfaceModifierDepth() const {return m_dd.frame.surfaceModifierDepth;}
	__forceinline const float& SurfaceRoughnessScale() const {return m_dd.frame.surfaceRoughnessScale;}
	__forceinline const float& GetAngularVelocity1D() const {return m_dd.frame.angularVelocity1D;}
	
	bool Validate() const;
	void ResetTriangleCache();

	void GetDmaData(void** pDmaData);
	void SpuFetch(const void* pDmaData);
	void SpuStore(void* pDmaData) const;

private:
	CarWheel(const CarWheel& other);

	FRIEND_CLASS Car;
	FRIEND_CLASS CarEngine;
	FRIEND_CLASS CarTransmission;

	void ClearStepVariables();

	// step
	__forceinline bool OnGround_Step() const {return Checked(m_step.onGround);}
	__forceinline const Normal3R& SuspensionAxis_Step() const {return Checked(m_step.suspensionAxis);}
	__forceinline const Normal3R& GroundNormal_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.groundNormal);}
	__forceinline const Normal3R& GroundX_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.groundX);}
	__forceinline const Normal3R& GroundZ_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.groundZ);}
	__forceinline const Point3R& GroundPosition_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.groundPosition);}
	__forceinline const Point3R& RestPosition_Step() const {return Checked(m_step.restPosition);}
	__forceinline const float& AngularVelocity1D_Step() const {return Checked(m_step.angularVelocity1D);}
	__forceinline const float& SlipAngle_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.slipAngle);}
	__forceinline const float& WheelSpin_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.wheelSpin);}
	__forceinline const float& WheelSpinFactor_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.wheelSpinFactor);}
	__forceinline const float& TractionControlAV_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.tractionControlAV);}
	__forceinline const float& BrakingAV_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.brakingAV);}
	__forceinline const float& SuspensionDistance_Step() const {return Checked(m_step.suspensionDistance);}
	__forceinline const float& StaticFrictionX_Step() const {return Checked(m_step.staticFrictionX);}
	__forceinline const float& StaticFrictionZ_Step() const {return Checked(m_step.staticFrictionZ);}
	__forceinline const float& CamberAngle_Step() const {return Checked(m_step.camberAngle);}
	__forceinline const float& StaticLoad_Step() const {return Checked(m_step.staticLoad);}
	__forceinline const Vector3R& GroundVelocity_Step() const {RP_ASSERT(OnGround_Step()); return Checked(m_step.groundVelocity);}
	__forceinline const Point3R& GetPosition_Step() const {return Checked(m_step.wheelPosition);}
	__forceinline const float& SteeringAngle_Step() const {return Checked(m_step.steeringAngle);}
	
	// frame
	__forceinline const float& SurfaceGripScale() const {return Checked(m_dd.frame.surfaceGripScale);}

	FloatVR ForwardsGripScaleInGear(S32 gear) const;
	
	float GetAckermanSteeringCorrection(float heading) const;
	FloatVR SlipAlignment() const;

	void SetOffset(const Vector3R& offset) { m_dd.offset = offset; }
	Point3R CalculateRestPosition(const Axis& carBody) const;
	Normal3R CalculateSuspensionAxis(const Axis& carBody) const;
	FloatVR CalculateAngularVelocity1D() const;

	void SetMassVariables();
	const float& AccumulationFactor() const;
	void GetCollisionBoundingBox(Point3R& min, Point3R& max) const;

	void GetGroundIntersection(CollisionCache::Contact& contact, const Point3R& restPosition, const Normal3R& suspensionDirection, FloatVR heightOffset,
								const Collision::C_CollisionFilter* filter, Collision::C_SingleTriangleCache* pCache);
	
	// valid during derivative calculation
	class Step
	{
		FRIEND_CLASS CarWheel;

		Normal3R suspensionAxis;
		Normal3R groundNormal;
		Normal3R groundX;
		Normal3R groundZ;
		Point3R groundPosition;
		Point3R wheelPosition;
		Point3R restPosition;
		Vector3R groundVelocity;

		// slip
		float angularVelocity1D;
		float slipAngle;
		float wheelSpin;
		float wheelSpinFactor;
		float tractionControlAV;
		float brakingAV;
		float relaxedSlipAngle;
		float slipFactor;
		float slip;
		float maxGripScale;
		float staticFrictionX;
		float staticFrictionZ;
		float suspensionDistance;
		float camberAngle;
		float staticLoad;
		float steeringAngle;
		bool onGround;
	};

	// constants over the whole frame
	struct Frame
	{
		Accumulator<Vector3R> groundNormalForce;
		Accumulator<Vector3R> groundFriction;
		Accumulator<Vector3R> groundTorque;
		Accumulator<Vector3R> groundPosition;
		Accumulator<Vector3R> groundNormal;
		Accumulator<Vector3R> groundVelocity;
		Vector3R smoothGroundForce;
		Point3R restPosition;
		Point3R wheelPosition;
		Accumulator<float> suspensionDistance;
		Accumulator<float> wheelSpin;
		Accumulator<float> slipAngle;

		float surfaceGripScale;
		float surfaceRoughnessScale;
		float surfaceRollingFriction;
		float surfaceMIScale;

		float angularVelocity1D;

		U32 surfaceFlags;
		U32 surfaceModifier;
		U32 userData;
		float surfaceModifierDepth;

		bool onGround;
	};

	/////////////////////////////////////////
	// Members that are only used in PPU code 
	/////////////////////////////////////////

#if !PLATFORM_SPU
	DebugSlipRecorder m_debugSlipRecorder;
#endif

	/////////////////////////////////////////
	// Members that are only used in SPU code (or the equivalent job on other platforms)
	/////////////////////////////////////////

	Step m_step;
	mutable Collision::C_SingleTriangleCache m_CachedTriangle;
	mutable Collision::C_SingleTriangleCache m_PuddleCachedTriangle;

	//////////////////////////////////////////////////////////
	// Data that is directly copied between SPU and PPU code
	//////////////////////////////////////////////////////////

	struct ALIGN16 DmaData
	{
		Frame frame;
		Axis oldAxis;
		Vector3R offset;
		bool driven;
	};

	DmaData m_dd;

	//////////////////////////////////////////////////////////
	// Constant members that do not need to be transferred
	//////////////////////////////////////////////////////////

	int m_wheelIndex; // 0, 1 front, 2, 3 back
	float m_signX;
	float m_signZ;
	Car* m_pCar;
	Rotor* m_pRotor;
};

class CarEngine
{
public:
	CarEngine() {} 

	// initialise
	void InitialiseConstants(Car* pCar, Rotor* pRotor);
	void Initialise();
	void SetMassVariables();
	void SetCarAxis(const Axis& axis);

	void BeginIntegrate();
	void EndIntegrate();

	// physics
	void EvaluateControlForces(ArticulatedBody::Derivative& derivative, FloatVR maxTotalEngineTorque);

	// information
	const CarDynamicsXt& Dynamics() const;
	FloatVR RPM() const;
	FloatVR LimiterRPM() const;
	FloatVR AngularVelocity1D() const;
	const float& TractionControl() const {return m_dd.tractionControl.GetValue();}
	const float& AdjustedThrottle() const {return m_dd.adjustedThrottle.GetValue();}
	bool PowerCut() const {return m_dd.powerCut;}
	float Boost() const {return 0.f;}

	// rotor
	Rotor& GetRotor() {return *m_pRotor;}
	const Rotor& GetRotor() const {return *m_pRotor;}

	Normal3R GetDriveAxis() const {return VEHICLE_REVERSE_Z_SIGN GetRotationMatrix().AxisX();}
	const RotationMatrix& GetRotationMatrix() const {return m_pRotor->GetRotationMatrix();}

	void GetDmaData(void** pDmaData);
	void SpuFetch(const void* pDmaData);
	void SpuStore(void* pDmaData) const;
	
private:
	CarEngine(const CarEngine& other);
	FRIEND_CLASS Car;

	float GetThrottle(float rpm, float targetRPM) const;
	float GetTorque(float rpm) const;

	Car* m_pCar;
	Rotor* m_pRotor;

	struct ALIGN16 DmaData
	{
		Accumulator<float> tractionControl;
		Accumulator<float> adjustedThrottle;

		bool powerCut;
	};

	DmaData m_dd;
};

/////////////////////////////////////////////////////////////////////////////////////////////

class CarTransmission
{
public:
	CarTransmission() {}
		
	// initialise
	void InitialiseConstants(Car* pCar);
	void Initialise();

	void BeginFrame();
	void BeginIntegrate(float timeStep);

	// torque calculation
	void Setup();
	void SetWheelAngularVelocity(int i, float av);
	void SetEngineAngularVelocity(float av);
	void CalculateViscousForces(float timeStep);
	void CalculateConstraintForces(float timeStep);
	void Finish();
	float GetEngineTorque() const;
	float GetWheelTorque(int i) const;

	const CarDynamicsXt& Dynamics() const;
	const CarDynamicsXt::CarTransmissionXt& TransmissionDynamics() const;

	FloatVR ClutchRPMFraction();
	FloatVR GearRatio() const {return GearRatio(m_dd.currentGear);}
	FloatVR GearRatio(int gear) const;
	FloatVR FinalRatio() const;
	const float& ClutchEngaged() const {return m_dd.clutchEngaged;}
	int Gear() const {return m_dd.currentGear;}
	int NextGear() const {return m_dd.gearDest;}
	bool GearChange() const {return m_dd.gearChange;}
	float TargetRPM() const;

	void GetDmaData(void** pDmaData);
	void SpuFetch(const void* pDmaData);
	void SpuStore(void* pDmaData) const;

private:
	CarTransmission(const CarTransmission& other);

	struct DriveShaft
	{
		DriveShaft() : angularVelocity(0.f), inertia(0.f), torque(0.f) {}

		float angularVelocity;
		float inertia;
		float torque;
	};

	struct Differential
	{
		int input;
		int output1;
		int output2;

		CarDynamicsXt::CarDifferentialTypeXt type;
		float forwardsTorque;
		float backwardsTorque;
		float viscosity;
	};

	struct Clutch
	{
		int input;
		int output;
	};

	struct ALIGN16 DmaData
	{
		Btl::Vector<Differential, Btl::VectorParameters_Static<3> > differentials;
		Btl::Vector<DriveShaft, Btl::VectorParameters_Static<8> > driveShafts;
		Clutch clutch;

		int currentGear;
		int gearDest;
		float clutchEngaged;
		float gearChangeClutchDelay;
		float targetRPM;
		bool gearChange;
	};

	DmaData m_dd;

	Car* m_pCar;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class CarControls
{
public:
	float steer;
	float throttle;
	float brake;
	float handbrake;
	float pitch;
	float roll;
	bool reverse;
	bool gearUp;
	bool gearDown;
	bool isWheel;
	bool horn;

	CarControls() : isWheel(false) {Clear();}

	void Clear()
	{
		steer = 0.f;
		throttle = 0.f;
		brake = 0.f;
		handbrake = 0.f;
		pitch = 0.f;
		roll = 0.f;

		reverse = false;
		gearUp = false;
		gearDown = false;
		horn = false;
	}

	// stop it moving (but can rev engine)
	void Hold()
	{
		steer = 0.f;
		handbrake = 1.f;
		brake = 0.f;

		reverse = false;
		gearUp = false;
		gearDown = false;
	}

	bool Validate() const
	{
		RP_ASSERT(steer >= -1.f && steer <= 1.f);
		RP_ASSERT(throttle >= 0.f && throttle <= 1.f);
		RP_ASSERT(brake >= 0.f && brake <= 1.f);
		RP_ASSERT(handbrake >= 0.f && handbrake <= 1.f);
		return true;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////

struct CarDriverSettings 
{
	bool autoGears;
	bool tractionControl;
	bool ABS;
	bool stabilityControl;
};

////////////////////////////////////////////////////////////////////////////////////////////

struct CarPerformanceModifiers
{
	float peakTorqueScale;
	float forwardGripScale;
	float transverseGripScale;
	float backwardGripScale;
	float rpmScale;
	float clutchDelay;
	float finalRatioScale;
	float maxWheelSpin;
	float airFrictionScale;
	float downforceScale;
	float steeringAntiFlickScale;
	float surfaceRollingFrictionScale;

	struct TyrePerformanceModifiers
	{
		float radiusScale;
		float gripScale;
		float rollingFriction;
		Vector3R offset;
	};

	Btl::Vector<TyrePerformanceModifiers, Btl::VectorParameters_Array<4> > wheelModifiers;

	CarPerformanceModifiers() :	peakTorqueScale(1.f),
								forwardGripScale(1.f),
								transverseGripScale(1.f),
								backwardGripScale(1.f),
								rpmScale(1.f),
								clutchDelay(0.f),
								finalRatioScale(1.f),
								maxWheelSpin(100.f),
								airFrictionScale(1.f),
								downforceScale(1.f),
								steeringAntiFlickScale(1.f),
								surfaceRollingFrictionScale(1.f)
	{
		for (int i = 0; i < 4; i++)
		{
			wheelModifiers[i].radiusScale = 1.f;
			wheelModifiers[i].gripScale = 1.f;
			wheelModifiers[i].rollingFriction = 0.f;
			wheelModifiers[i].offset = Vector3R(0.0f, 0.0f, 0.0f);
		}
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

class Car : public Vehicle, public ArticulatedBody
{
public:
	Car();

#if !PLATFORM_SPU
	virtual ~Car();
	void DebugRenderTriangleCache(DbUtil::I_DebugRender* pDebugRender) const;
	void Initialise(const VehicleInitialiseInfo& initialiseInfo);
#endif

	void Shutdown();

	//////////////////////////////////////////////////////////////

	static const Car* GetCarPointer(const Vehicle* v) {return (v->GetVehicleType() == Vehicle::VEHICLE_TYPE_CAR) ? (Car*) v : 0;}
	static Car* GetCarPointer(Vehicle* v) {return (v->GetVehicleType() == Vehicle::VEHICLE_TYPE_CAR) ? (Car*) v : 0;}

	void SetCopyMassVariablesToGravity() {m_copyMassVariablesToGravity = true;} 
	void UnsetCopyMassVariablesToGravity() {m_copyMassVariablesToGravity = false;}
	void EnableWallRiding(bool bWallRiding);

	//////////////////////////////////////////////////////////////
	// vehicle interface

	virtual void SetAxis(const Axis& a, bool overwriteOldAxis = true);
	virtual bool PutOnGround() {return true;}
	virtual void Stop();
	virtual void SetPosition(const Point3R& p);
	virtual void ClearAllFeedback();
	virtual float ConstrainInternal();
	virtual void Integrate(float time);
	//virtual void Push(const Point3R& p, const Vector3R& v);
	//virtual void ApplyImpulse(const Point3R& p, const Vector3R& impulse, FloatVR torqueFactor);
	virtual void HandleCollision(const Point3R& p, const Vector3R& impulse);
	//virtual void GetCollisionBoundingBox(Point3R& min, Point3R& max) const;
	//virtual float GetCollisionTime(float time, float resolution) const;
	virtual S32 GetNumWheels() const {return 4;}
	virtual bool WheelOnGround(int i) const;
	virtual void GetWheelContact(int i, Point3R& position, Normal3R& normal) const;
	//virtual U32 GetCollisionFlags() const;
	virtual void SetHold();

#if !PLATFORM_SPU
	virtual void BeginFrame(float maxSpeed);
	virtual void EndFrame(const Maths::C_TimeS PCREF time);
	virtual void DisplayDebug() const;
#endif

	// information from dynamics
	virtual Vector3R GetAngularVelocity() const {return Body().AngularVelocity();}
	virtual Vector3R GetVelocity() const {return Body().Velocity();}
	virtual Point3R GetPosition() const {return Body().Position();}
	virtual const Axis& GetAxis() const {return Body().GetAxis();}
	virtual const Axis& GetOldAxis() const {return m_dd.oldAxis;}
	//virtual FloatVR GetLargestVelocity() const;
	virtual int GetGear() const {return Transmission().Gear();}
	virtual FloatVR GetAirSpeed_GlobalXZ() const {return VEHICLE_FORWARD_Z_SIGN (Body().Velocity() | Maths::Normalised(ComponentMultiply(Body().GetAxis().AxisZ(), Maths::Constants<Vector3R>::XZ())));} // speed along the Z axis projected onto the global XZ plane 
	virtual FloatVR GetAirSpeed_LocalZ() const {return VEHICLE_FORWARD_Z_SIGN (Body().Velocity() | Body().GetAxis().AxisZ());} // speed along the local Z axis
	virtual FloatVR GetGroundSpeed_GlobalXZ() const {return VEHICLE_FORWARD_Z_SIGN ((Body().Velocity() - GroundVelocity()) | Maths::Normalised(ComponentMultiply(Body().GetAxis().AxisZ(), Maths::Constants<Vector3R>::XZ())));} // speed along the Z axis projected onto the global XZ plane 
	virtual FloatVR GetGroundSpeed_LocalZ() const {return VEHICLE_FORWARD_Z_SIGN ((Body().Velocity() - GroundVelocity()) | Body().GetAxis().AxisZ());} // speed along the local Z axis
	virtual Normal3R GetHeading() const {return VEHICLE_FORWARD_Z_SIGN Body().GetAxis().AxisZ();}
	virtual Normal3R GetRight() const {return VEHICLE_REVERSE_Z_SIGN Body().GetAxis().AxisX();}
	virtual Point3R GetCofG() const {return Body().CofG();}

#if !PLATFORM_SPU
	// network
	virtual void GetNetworkControls(CompressedNetworkControls& controlsOut) const;
	virtual void GetNetworkState(CompressedNetworkState* state, const Vehicle* pTarget = 0) const;
	virtual void SetNetworkControls(const CompressedNetworkControls& carControlsIn);
	virtual void EnsureReducedBrakeValue();							// Fix for revs in cutscenes
	virtual bool ProcessNetworkState(const CompressedNetworkState& state, bool runCatchUp);
	virtual S32 GetNetworkPredictionFrameCount() const;
	bool CheckNetworkJumped();
#endif

	void NetworkJumpIfNeeded();
	bool NetworkJump(bool position, bool velocity);

/*
	//	replays
	virtual void ProcessReplayState(const CompressedNetworkState& state);
	virtual void SetReplayControls(const CompressedNetworkControls& carControlsIn);
*/
	static CompressedNetworkState BlendStatesLinear(const CompressedNetworkState& p_pState0, const CompressedNetworkState& p_pState1, float p_Blend, float dt);
	static Point3R GetPositionFromState(const Vehicle::CompressedNetworkState& vehicleState);


	/////////////////////////////////////////////////////////////////////////////////
	// articulated body

	virtual void EvaluateDerivative(Derivative& output);
	virtual void BeginIntegrate();
	virtual void EndIntegrate();
	virtual void GetIntegrationType(int& integrationType, int& nSteps);

	////////////////////////////////////////////////////////////////////////////////
	// car interface

	void SetOnGround();
	void SetAxis(const Axis& a, float steeringAngle, bool overwriteOldAxis = true);
	void CalculateAxes(const Axis& body, float steeringAngle, float* suspensionOffsets, // input
						Axis* wheelAxes) const; // output

	FloatVR GroundSpeed_LocalZ() const {return -((Body().Velocity() - GroundVelocity()) | Body().GetAxis().AxisZ());}
	FloatVR AirSpeed_LocalZ() const {return -(Body().Velocity() | Body().GetAxis().AxisZ());}

	bool Air() const;
	const Normal3R& SmoothedGroundNormal() const {RP_ASSERT(!Air()); return m_dd.frame.smoothedGroundNormal;}
	const Vector3R& GroundVelocity() const {return m_dd.frame.groundVelocity;}
	S32 NumWheelsOnGround() const;

	/////////////////////////////////////////////////////////////

	const CarDynamicsXt& Dynamics() const {return *m_pCarDynamics;}

	const CarPerformanceModifiers& PerformanceModifiers() const {return m_dd.performanceModifiers;}
	CarPerformanceModifiers& PerformanceModifiers() {return m_dd.performanceModifiers;}

	////////////////////////////////////////////////////////////
	// controls

	const CarControls& Controls() const {return m_dd.controls;}
	CarControls& Controls() {return m_dd.controls;}

	bool AutoReverse() const {return GetControlType() != RacingPhysics::Vehicle::CONTROL_TYPE_AI;}

	const CarDriverSettings& DriverSettings() const {return m_dd.driverSettings;}
	CarDriverSettings& DriverSettings() {return m_dd.driverSettings;}

	bool AutoGears() const {return DriverSettings().autoGears || GetControlType() == RacingPhysics::Vehicle::CONTROL_TYPE_AI;}

	const float& Throttle() const {return m_dd.throttle;}
	const float& Brake() const {return m_dd.brake;}
	const float& Steer() const {return m_dd.steer;}
	const float& Handbrake() const {return m_dd.handbrake;}
	bool Reverse() const {return m_dd.reverse;}

	/////////////////////////////////////////////
	// network

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
		int framesPredicted;
		bool networkStateValid;
		bool useFullPrediction;
		bool air;
	};

	static void CompressNetworkState(const NetworkState& networkState, CompressedNetworkState& vehicleState);
	static void UncompressNetworkState(const CompressedNetworkState& vehicleState, NetworkState& networkState);

	// parts
	FORCE_INLINE const CarEngine& Engine() const {return m_engine;}
	FORCE_INLINE const CarWheel& Wheel(int i) const {return m_wheels[i];}
	FORCE_INLINE const CarTransmission& Transmission() const {return m_transmission;}

#if !PLATFORM_SPU
	void SetGravityBody(Gravity::C_Body* pBody, const Maths::HVec3 PCREF offsetToBodyCentre, const Maths::HVec3 PCREF bounds, const Maths::HVec3 PCREF offsetToBoundsCentre);
	virtual Gravity::C_Body* GetGravityBody() {return m_pGravityCarBody;}
	virtual void CopyToGravity() const;
	virtual void CopyFromGravity();
#endif
	Collision::C_LineCollisionCache& GetLineCollisionCache() {return m_LineCache;}
	virtual void CreateGravityCache();

	void GetDmaData(void** pDmaData);
	void SpuFetch(const void* pDmaData);
	void SpuStore(void* pDmaData) const;

	void CopyMassVariablesToGravity() const;
	float CalculateDownforceFactor(float speed) const;
	float CalculateMaxAcceleration() const;

private:
	FRIEND_CLASS CarWheel;
	FRIEND_CLASS CarEngine;
	FRIEND_CLASS CarTransmission;

	void ClearStepVariables();
	void SetMassVariables();

#if! CFG_SPEED
	bool Validate() const;
#endif

#if !PLATFORM_SPU
	virtual void XtReload();
#endif

	void EvaluateTyreForces(Derivative& derivative);
	void EvaluateSpringForces(Derivative& derivative);
	void EvaluateTransmissionForces(Derivative& derivative);
	void EvaluateControlForces(Derivative& derivative);
	void EvaluateFrictionForces(Derivative& derivative);
	void EvaluateConstraintForces(Derivative& derivative, float dt, bool constrainPositions);
	
	float CalculateSpeedDependentDownforceFactor() const;
	
	float CalculateDownforceFactor() const;
	
	bool CalculateAir() const;
	float CalculateTurnSteerLimit() const;
	float CalculateSlipSteerOffset() const;
	float CalculateSlipSteerLimit_Centre() const;
	float CalculateTurningAngularVelocity() const;
	void CalculateSteerLimits(float& steerLimitL, float& steerLimitR, float& steerCentre, bool step) const;
	
	const float& DownforceFactor_Step() const {return Checked(m_step.downforceFactor);}
	const float& TurnSteerLimit_Step() const {return Checked(m_step.turnSteerLimit);}
	const float& MaxAcceleration_Step() const {return Checked(m_step.maxAcceleration);}
	const float& SteerLimitL_Step() const {return Checked(m_step.steerLimitL);}
	const float& SteerLimitR_Step() const {return Checked(m_step.steerLimitR);}
	bool Air_Step() const {return Checked(m_step.air);}

	__forceinline const RigidBody& Body() const {return GetRigidBody(0);}
	__forceinline RigidBody& Body() {return GetRigidBody(0);}
	int BodyIndex() const {return 0;}
	int WheelIndex(int i) const {return i + 1;}
	int EngineIndex() const {return 5;}

	CarEngine& Engine_NC() {return m_engine;}
	CarWheel& Wheel_NC(int i) {return m_wheels[i];}
	CarTransmission& Transmission_NC() {return m_transmission;}

	void CheckConstraints();

	// network
	void NetworkInitialise();
	void EvaluateNetworkForces(Derivative& output);
	void GetNetworkState(NetworkState& networkState) const;
	void UpdatePredictedNetworkState(NetworkState& networkState, bool constrain);
	void ConstrainNetworkPosition();
	void JumpToNetworkState(const NetworkState& networkState, bool position, bool velocity);
	void JumpToNetworkState(const Axis& axis, const Vector3R& velocity, const Vector3R& angularVelocity);
	
	// replays
//	void BeginReplayUpdate();

	/////////////////////////////////////////
	// Members that are only used in PPU code 
	/////////////////////////////////////////

#if !PLATFORM_SPU

	struct NetworkTiming
	{
		S32 framesPredictedLocally;
		S32 framesPredictedRemotely;
		S32 bestRoundTrip;
	};

	NetworkState m_networkState;
	NetworkTiming m_networkTiming;

	int m_dynamicsChangedCounter;
	Gravity::C_Body* m_pGravityCarBody;
	Maths::HVec3 m_OffsetToCarBodyCentre;
	
#endif

	/////////////////////////////////////////
	// Members that are only used in SPU code (or the equivalent job on other platforms)
	/////////////////////////////////////////

	ConstraintSolver m_constraintSolver;

	//bool m_bUseGravityBody;

	bool m_copyMassVariablesToGravity;
	bool m_bEnableWallRiding;
	//bool m_bUseGravityBody;

	// temp
	struct Step
	{
		float downforceFactor;
		float turnSteerLimit;
		float maxAcceleration;
		float steerLimitL;
		float steerLimitR;
		bool air;
	};

	Step m_step;

	Collision::C_LineCollisionCache m_LineCache;
	
	//////////////////////////////////////////////////////////
	// Data that is directly copied between SPU and PPU code
	//////////////////////////////////////////////////////////

	struct ALIGN16 DmaData
	{
		DmaData() : pNetworkCollisionShape(0), pPhantom(NULL), phantomCollisionGroup(Scheduler::JQ_GROUP_NONE) {}

		CarControls controls;
		CarDriverSettings driverSettings;
		CarPerformanceModifiers performanceModifiers;

		// controls
		float throttle;
		float brake;
		float steer;
		float handbrake;
		bool reverse;

		// temp
		struct Frame
		{
			Normal3R smoothedGroundNormal;
			Vector3R groundVelocity;
			
			float previousSteeringAngle;
			float steeringRate;
			Accumulator<Vector3R> airFrictionForce; // debug
			Accumulator<Vector3R> downforce; //debug
			Accumulator<Vector3R> networkForce; //debug
			Accumulator<Vector3R> steeringTorque; //debug
		};

		Frame frame;

		Collision::C_CRAxisBox gravityTransformedBox;
		Vector3R externalForce;
		Vector3R externalTorque;

		Axis oldAxis;

		NetworkState predictedNetworkState;
		NetworkState predictedNetworkStateJump;

		Collision::C_Shape* pNetworkCollisionShape;
		Gravity::C_CollisionPhantom* pPhantom;
		Scheduler::GroupHandle phantomCollisionGroup;

		S32 noNetworkJumpCounter;
		bool bNetworkJumpedThisFrame;

		int debug;
		bool m_UsePhysicsGravity;
		S32 m_OverrideSteps;
		S32 m_OverrideIntegrationMethod;

		// data for copying everything else
		Btl::Vector<void*, Btl::VectorParameters_Array<20> > dmaPointers;
	};

	DmaData m_dd;

	//////////////////////////////////////////////////////////////////
	// Members that need to be copied between PPU and SPU individually.
	//////////////////////////////////////////////////////////////////

	const XtRecord* m_pSettings;
	const CarDynamicsXt* m_pCarDynamics;
	Btl::Vector<CarWheel, Btl::VectorParameters_Array<4> > m_wheels;
	CarTransmission m_transmission;
	CarEngine m_engine;

	// + Vehicle
	// + ArticulatedBody
};

__forceinline const CarDynamicsXt& CarWheel::Dynamics() const {return m_pCar->Dynamics();}
__forceinline const CarDynamicsXt::CarTyreXt& CarWheel::TyreDynamics() const {return Dynamics().tyres.GetTyre(m_wheelIndex);}
__forceinline const CarDynamicsXt::CarWheelSuspensionXt& CarWheel::SuspensionDynamics() const {return Dynamics().suspension.GetWheel(m_wheelIndex);}
__forceinline const CarDynamicsXt& CarEngine::Dynamics() const {return m_pCar->Dynamics();}
__forceinline const CarDynamicsXt& CarTransmission::Dynamics() const {return m_pCar->Dynamics();}
__forceinline const CarDynamicsXt::CarTransmissionXt& CarTransmission::TransmissionDynamics() const {return m_pCar->Dynamics().transmission;}

__forceinline const float& CarWheel::AccumulationFactor() const {return m_pCar->AccumulationFactor();}
__forceinline const Vector3R CarWheel::GetVelocity() const {return m_pCar->Body().PointVelocity(GetWheelPosition());}
__forceinline FloatVR CarWheel::TransverseGripScaleDry() const {return TyreDynamics().grip.yScale * TyreDynamics().transverseGripScale * m_pCar->PerformanceModifiers().transverseGripScale * m_pCar->PerformanceModifiers().wheelModifiers[m_wheelIndex].gripScale;}
__forceinline FloatVR CarWheel::BackwardsGripScaleDry() const {return TyreDynamics().grip.yScale * TyreDynamics().backwardsGripScale * m_pCar->PerformanceModifiers().backwardGripScale * m_pCar->PerformanceModifiers().wheelModifiers[m_wheelIndex].gripScale;}
__forceinline FloatVR CarWheel::TransverseGripScale() const {return TransverseGripScaleDry() * SurfaceGripScale();}
__forceinline FloatVR CarWheel::ForwardsGripScale() const {return ForwardsGripScaleDry() * SurfaceGripScale();}
__forceinline FloatVR CarWheel::BackwardsGripScale() const {return BackwardsGripScaleDry() * SurfaceGripScale();}
__forceinline FloatVR CarEngine::LimiterRPM() const {return Dynamics().engine.limiterRPM * m_pCar->PerformanceModifiers().rpmScale;}

__forceinline float CarWheel::CollisionRadius() const {return Radius() * m_pCar->PerformanceModifiers().wheelModifiers[m_wheelIndex].radiusScale;}
__forceinline FloatVR CarWheel::TransverseTyreSlip() const {return TyreDynamics().transverseTyreSlipScale;}
__forceinline FloatVR CarWheel::LongitudinalTyreSlip() const {return TyreDynamics().longitudinalTyreSlipScale;}

float GetGroundOffset(const Point3R& position, float pScale);

}

#endif
