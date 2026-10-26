#ifndef RP_VEHICLE_H
#define RP_VEHICLE_H
#ifdef _MSC_VER
#pragma once
#endif

#if !PLATFORM_SPU
#include "btl/utility/string.h"
#endif

#include "racingphysics/physics/rigidbody.h"
#include "racingphysics/collisions/collisioncache.h"
#include "racingphysics/physics/surfaces.h"
#include "gravity/sdk/body.h"
#include "gravity/narrowphase/shape.h"
#include "gravity/sdk/Scene.h"

#ifndef VEHICLE_FORWARD_Z_SIGN
#define VEHICLE_FORWARD_Z_SIGN
#define VEHICLE_REVERSE_Z_SIGN -
#endif

namespace Gravity
{
	class C_ImportShape;
}

#ifdef _RACERNOTAMAX
#include "game\audio\audiogamesection.h"
#endif

namespace RacingPhysics
{

class COObject;
class Vehicle;

#if !PLATFORM_SPU

class IntegrationCallback : public Gravity::C_IntegrationCallback
{
public:

	IntegrationCallback(RacingPhysics::Vehicle* vehicle) : m_pVehicle(vehicle)
	{
	}

	virtual void PostIntegration(const Maths::C_TimeS PCREF time);
	virtual Scheduler::GroupHandle BeginIntegrationTask(Scheduler::GroupHandle group, Scheduler::PolicyHandle policy, float time);
	
	RacingPhysics::Vehicle* m_pVehicle;

	static Scheduler::GroupHandle s_networkUpdateGroup;
	static Scheduler::GroupHandle s_integrationGroup;
};

#endif

struct IntegrationCommand
{
	RacingPhysics::Vehicle* pVehicle;
	void* vehicleDmaData;
	void* surfaceDmaData;
	void* physicsTimer;
	float time;
};

////////////////////////////////////////

// General vehicle. 
// Car and bike classes derive from this.
class Vehicle
{
public:

	enum ControlType
	{
		CONTROL_TYPE_UNKNOWN = -1,
		CONTROL_TYPE_PLAYER,
		CONTROL_TYPE_AI,
		CONTROL_TYPE_NETWORK,
		CONTROL_TYPE_REPLAY,
		CONTROL_TYPE_SHOWROOM,
	};

	enum VehicleType
	{
		VEHICLE_TYPE_UNKNOWN = -1,
		VEHICLE_TYPE_CAR,
		VEHICLE_TYPE_BIKE
	};

	struct CompressedNetworkState
	{
		U8 data[25];
	};

	struct CompressedNetworkControls
	{
		U8 data[4];
	};

	// To do: take this out.
	struct NetworkForces
	{
		NetworkForces()
		{
			Reset();	
		}

		void Reset()
		{
			m_Spring				= 5.0f;
			m_Damping				= 1.0f;
			m_MaxForce				= 5.0f;
			m_TorqueSpring			= -5.0f;
			m_TorqueDamping			= 0.5f;
			m_TorqueMaxForce		= 4.0f;
		}

		float				m_Spring;
		float				m_Damping;
		float				m_MaxForce;

		float				m_TorqueSpring;
		float				m_TorqueDamping;
		float				m_TorqueMaxForce;
	};

	/////////////////////////////////////

	Vehicle();
	virtual ~Vehicle();

	// initialisation
	void Initialise(RigidBody* pCollisionBody, VehicleType type);
	void Shutdown();
	virtual void SetAxis(const Axis& a, bool overwriteOldAxis = true) = 0;
	
	virtual bool PutOnGround() = 0; // returns false on collision error
	bool ValidatePosition();

	virtual void SetPosition(const Point3R& p) = 0;
	virtual void Stop();

	// display
	virtual void DisplayDebug() const;

	// flags
	void SetControlType(ControlType c) {m_dd.controlType = c;}
	void SetTreatAsGhost(bool b);
	virtual void SetHold() = 0; // just set brakes and clutch once
	void ToggleAIControl();
	void ForceAIControl() {m_dd.controlType = Vehicle::CONTROL_TYPE_AI;};
	void ForcePlayerControl() {m_dd.controlType = Vehicle::CONTROL_TYPE_PLAYER;};
	
	VehicleType GetVehicleType() const {return m_dd.vehicleType;}
	ControlType GetControlType() const {return m_dd.controlType;}
	int VehicleIndex() const {return m_dd.vehicleIndex;}
	float SlipStream() const {return m_dd.slipStream;} // reduction in air friction due to slip streaming
	float ForcedAcceleration() const {return m_dd.forcedAcceleration;} // extra thrust applied to the vehicle
	
#if !PLATFORM_SPU
	U32 GetAudioHandle() const {return m_AudioHandle;}
	void SetAudioHandle(U32 hAudio) {m_AudioHandle = hAudio;}
	U32 m_AudioHandle;

	S32 ControllerPort() const {return m_controllerPort;}
	void SetControllerPort(S32 port) { RP_ASSERT((port >= -128) && (port <= 127)); m_controllerPort = (S8)port;}
#endif

	//void SetIndex(int index) {m_dd.vehicleIndex = index;}
	void SetSlipStream(float s) {m_dd.slipStream = s;}
	void SetForcedAcceleration(float a) {m_dd.forcedAcceleration = a;}

	// physics functions
#if !PLATFORM_SPU
	virtual void BeginFrame(float maxSpeed) = 0;
	virtual void EndFrame(const Maths::C_TimeS PCREF time) = 0;
#endif
	virtual float ConstrainInternal() = 0;
	virtual void Integrate(float time) = 0;
	virtual void HandleCollision(const Point3R& p, const Vector3R& impulse) {}
	virtual S32 GetNumWheels() const = 0;
	virtual bool WheelOnGround(int i) const = 0;
	virtual void GetWheelContact(int i, Point3R& position, Normal3R& normal) const = 0;
	virtual void ClearAllFeedback() = 0;
	void UpdateCollisionCache(const Point3R& min, const Point3R& max);
	void UpdateCollisionCache(float largestSpeed);

	// information from dynamics
	virtual Vector3R GetAngularVelocity() const = 0;
	virtual Vector3R GetVelocity() const = 0;
	virtual Point3R GetPosition() const = 0;
	virtual const Axis& GetAxis() const = 0;
	//virtual FloatVR GetLargestVelocity() const = 0;
	virtual const Axis& GetOldAxis() const = 0;
	virtual int GetGear() const = 0;
	virtual FloatVR GetAirSpeed_GlobalXZ() const = 0; // speed along the Z axis projected onto the global XZ plane 
	virtual FloatVR GetAirSpeed_LocalZ() const = 0; // speed along the local Z axis
	virtual FloatVR GetGroundSpeed_GlobalXZ() const = 0; // speed along the Z axis projected onto the global XZ plane 
	virtual FloatVR GetGroundSpeed_LocalZ() const = 0; // speed along the local Z axis
	virtual Normal3R GetHeading() const = 0;
	virtual Point3R GetCofG() const = 0;
	//virtual U32 GetCollisionFlags() const;

	const Vector3R& Acceleration() const {return m_dd.acceleration;}
	const Vector3R& AngularAcceleration() const {return m_dd.angularAcceleration;}

#if !PLATFORM_SPU
	// network and replays
	virtual void GetNetworkControls(CompressedNetworkControls& controlsOut) const = 0;
	virtual void GetNetworkState(CompressedNetworkState* state, const Vehicle* pTarget = 0) const = 0;
	virtual void SetNetworkControls(const CompressedNetworkControls& carControlsIn) = 0;
	virtual bool ProcessNetworkState(const CompressedNetworkState& state, bool runCatchUp) = 0; // returns false for failure
	//bool NetworkJump(bool position, bool velocity); // returns true if it had to jump
	virtual S32 GetNetworkPredictionFrameCount() const = 0;
#endif

	void SetNetworkForces(NetworkForces networkForces) { m_dd.networkForces = networkForces; }
	NetworkForces& GetNetworkForces() { return m_dd.networkForces; }

#if !PLATFORM_SPU
	void SetFilename(const char* p) {m_filename = p;}
	const char *GetFilename() const {return m_filename.Chars();}
#endif

	static void (*s_buildCacheSolid) (CollisionCache* cache, const Point3R& min, const Point3R& max);
	static void (*s_buildCacheOverlay) (CollisionCache* cache, const Point3R& min, const Point3R& max);

	void SetWheelGroundFilter(const Collision::C_CollisionFilter* pFilter) {m_dd.wheelGroundFilter = *pFilter;}
	void SetWheelOverlayFilter(const Collision::C_CollisionFilter* pFilter) {m_dd.wheelOverlayFilter = *pFilter;}

	const Collision::C_CollisionFilter& WheelGroundFilter() const {return m_dd.wheelGroundFilter;}
	const Collision::C_CollisionFilter& WheelOverlayFilter() const {return m_dd.wheelOverlayFilter;}

#if !PLATFORM_SPU
	//Gravity copy virtual functions!
	virtual void CopyFromGravity() {}
	virtual void CopyToGravity() const {}
	virtual void SetApplyGravity(bool bApply) {}
	//virtual void SetGravityBody(Gravity::C_Body* pBody, const Maths::HVec3 PCREF offset) {}
	void SetGravityScene(Gravity::C_Scene* scene) {m_dd.pGravityScene = scene;}
	virtual Gravity::C_Body* GetGravityBody() {return NULL;}
	IntegrationCallback* GetIntegrationCallback() {return &m_IntegrationCallback;}
#endif

	Gravity::C_Scene* GetGravityScene() {return m_dd.pGravityScene;}
	virtual void CreateGravityCache() {}

	virtual void GetDmaData(void** pDmaData);
	virtual void SpuFetch(const void* pDmaData);
	virtual void SpuStore(void* pDmaData) const;

	friend class VehicleManager;

private:

#if !PLATFORM_SPU
	virtual void XtReload() {}
#endif

	//////////////////////////////////////////////////////////
	// Data that is directly copied between SPU and PPU code
	//////////////////////////////////////////////////////////

	struct ALIGN16 DmaData
	{
		VehicleType vehicleType;
		ControlType controlType;
		int vehicleIndex;
		float slipStream;
		float forcedAcceleration;

		Vector3R acceleration;
		Vector3R angularAcceleration;
		Vector3R oldVelocity;
		Vector3R oldAngularVelocity;

		NetworkForces networkForces;

		Collision::C_CollisionFilter wheelGroundFilter;
		Collision::C_CollisionFilter wheelOverlayFilter;

		Gravity::C_Scene* pGravityScene;
	};

	DmaData m_dd;

	/////////////////////////////////////////
	// Members that are only used in PPU code 
	/////////////////////////////////////////

#if !PLATFORM_SPU
	Btl::String m_filename;
	IntegrationCallback m_IntegrationCallback;
	S8 m_controllerPort;
#endif

};

#if !PLATFORM_SPU

class VehicleInitialiseInfo
{
public:
	VehicleInitialiseInfo()
	{
		vehicleToVehicleCollisions = true;
		forceHandbrake = false;
		vehicleIndex = 0;
		controlType = Vehicle::CONTROL_TYPE_PLAYER;
		controllerPort = -1;
		autoGears	= true;
		autoReverse =	true;
		quickRestart = false;

		collisionCentre.X(0.f);
		collisionCentre.Z(0.f);
		collisionCentre.Y(0.3f);
		collisionSize.X(0.f);
		collisionSize.Z(1.f);
		collisionSize.Y(0.3f);
	}

	Btl::String folder;
	Btl::String filename;

	S32 vehicleIndex; // car index in the cars vector
	Point3R collisionCentre;
	Vector3R collisionSize;
	Vehicle::ControlType controlType; // what type of control will this car have
	S8 controllerPort; // if player car which controller port..
	bool forceHandbrake; // should we force this cars handbrake, used for delayed starts. (game logic must turn handbrake off later..)
	bool autoGears; // Automatic transmission
	bool autoReverse; // Reverse using the brake button..
	bool vehicleToVehicleCollisions;
	bool quickRestart;

	//AIInitialiseInfo aiInfo;
};

void XtReloadVehicles();
void NetworkTests();

#endif

}

#endif // VEHICLE_H
