#include "stdafx.h"
#include "racingphysics/vehicles/vehicle.h"
#include "racingphysics/physics/physics.h"
//#include "racingphysics/collisions/collisionmesh.h"
#include "racingphysics/collisions\infiniteplane.h"
#include "racingphysics/physics/surfaces.h"
#include "racingphysics/physics/physicstimer.h"

#if !PLATFORM_SPU
#include "system\debug/log/debuglog.h"
#include "system\debug/log/subscribers/DebugViewSubscriber.h"
#endif

#ifdef DEBUG_RACING_PHYSICS
#pragma optimize ("", off)
#endif

namespace RacingPhysics
{

#if !PLATFORM_SPU
Debug::Logger& g_RacingPhysicsLog = Debug::GetLogger("mercury.racingphysics");
#endif

DECLARE_DEBUG_BOOL(g_updatePhysics,"RacingPhysics.UpdatePhysics", true);
DECLARE_DEBUG_FLOAT(g_debugThrust,"RacingPhysics.Debug.DebugThrust", 0.f, Debug::Range(-100.f, 100.f));
DECLARE_DEBUG_INT(g_debugVehicleIndex,"RacingPhysics.Debug.DebugVehicleIndex", 0, Debug::Range(-1, 8), Debug::Persist());
DECLARE_DEBUG_BOOL(g_showTyreSlipGraph,"RacingPhysics.Debug.ShowTyreSlipGraph", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_debugShowSlipDiagram,"RacingPhysics.Debug.ShowSlipDiagram", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_debugShowForces,"RacingPhysics.Debug.ShowForces", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_debugShowSlipRecords,"RacingPhysics.Debug.SlipRecorder.ShowSlipRecords", false, Debug::Persist());
DECLARE_DEBUG_FLOAT(g_timeStepScale,"RacingPhysics.Integration.TimeStepScale", 1.f, Debug::Range(0.01f, 1.f), Debug::Step(0.01f));
DECLARE_DEBUG_BOOL(g_displayDebugVehicle,"RacingPhysics.Debug.DisplayDebugVehicle", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_showNetworkState,"RacingPhysics.Debug.DisplayNetworkState", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_showVehicleDebugText,"RacingPhysics.Debug.ShowVehicleDebugText", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_infiniteCollisionPlane,"RacingPhysics.Collision.InfiniteCollisionPlane", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_debugShowLocalCollisionCache,"RacingPhysics.Collision.ShowLocalCollisionCache", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_showDynamicsDebugText,"RacingPhysics.Debug.ShowDebugText", false, Debug::Persist());
DECLARE_DEBUG_BOOL(g_runNetworkTests,"RacingPhysics.NetworkTests", false);

EXTERN_DEBUG_INT(g_debugVehicleIndex);
EXTERN_DEBUG_BOOL(g_debugVehicleBoundingBox);
EXTERN_DEBUG_BOOL(g_infiniteCollisionPlane);
EXTERN_DEBUG_FLOAT(g_timeStepScale);

void (*Vehicle::s_buildCacheSolid) (CollisionCache* cache, const Point3R& min, const Point3R& max);
void (*Vehicle::s_buildCacheOverlay) (CollisionCache* cache, const Point3R& min, const Point3R& max);

///////////////////////////////////////////////////////////////////////////////////////////

NOINLINE void PhysicsError()
{
#ifndef CFG_SPEED
	BIZ_ASSERT_FAIL("Physics Error");
#if !PLATFORM_SPU
	g_updatePhysics = false;
#endif
#endif
}

#if !PLATFORM_SPU

// VehicleManager keeps track of all active vehicles.
class VehicleManager
{
public:
	static VehicleManager& Instance() {return s_instance;}
	
	int AddVehicle(Vehicle* p)
	{
		m_lock.Enter();

		for (int i = 0; i < m_vehicles.Size(); i++)
		{
			if (m_vehicles[i] == 0)
			{
				m_vehicles[i] = p;
				return i;
			}
		}

		m_vehicles.PushBack(p);
		int n = m_vehicles.Size() - 1;

		m_lock.Exit();
		return n;
	}

	void RemoveVehicle(Vehicle* p)
	{
		m_lock.Enter();

		for (int i = 0; i < m_vehicles.Size(); i++)
		{
			if (m_vehicles[i] == p)
				m_vehicles[i] = 0;
		}

		while (!m_vehicles.Empty() && m_vehicles.Back() == 0)
			m_vehicles.PopBack();

		m_lock.Exit();
	}

	void XtReload()
	{
		m_lock.Enter();

		for (int i = 0; i < m_vehicles.Size(); i++)
		{
			if (m_vehicles[i])
				m_vehicles[i]->XtReload();
		}

		m_lock.Exit();	
	}

	void NetworkTests()
	{
		if (!g_runNetworkTests || m_vehicles.Size() < 2)
			return;

		static int count = 0;
		static int max = 9;
		static int getState = 0;

		static Vehicle::CompressedNetworkControls controls;
		static Vehicle::CompressedNetworkState state;

		if (count == getState)
		{
			m_vehicles[1]->GetNetworkControls(controls);
			m_vehicles[1]->GetNetworkState(&state, m_vehicles[0]);
		}
		
		if (count >= max)
		{
			count = 0;
			
			m_vehicles[0]->SetControlType(Vehicle::CONTROL_TYPE_NETWORK);
			m_vehicles[0]->SetNetworkControls(controls);
			m_vehicles[0]->ProcessNetworkState(state, false);
		}
		else
			count++;
	}

private:
	VehicleManager() {}

	Btl::Vector<Vehicle*> m_vehicles;
	MT::Lock m_lock;

	static VehicleManager s_instance;
};

VehicleManager VehicleManager::s_instance;

void XtReloadVehicles()
{
	VehicleManager::Instance().XtReload();
}

void NetworkTests()
{
	VehicleManager::Instance().NetworkTests();
}

#endif

Vehicle::Vehicle() 
#if !PLATFORM_SPU
					: m_IntegrationCallback(this)
#endif
{
	m_dd.pGravityScene = 0;
	//m_dd.vehicleIndex = -1;
#if !PLATFORM_SPU
	m_dd.vehicleIndex = VehicleManager::Instance().AddVehicle(this);
#endif
}

Vehicle::~Vehicle() 
{
#if !PLATFORM_SPU
	VehicleManager::Instance().RemoveVehicle(this);
#endif
}

void Vehicle::Initialise(RigidBody* pCollisionBody, VehicleType type)
{
	RP_ASSERT(pCollisionBody);

	m_dd.vehicleType = type;
	m_dd.controlType = CONTROL_TYPE_UNKNOWN;

#ifdef AUDIO_ENABLED
	m_AudioHandle = 0;
#endif
	
#if !PLATFORM_SPU
	m_controllerPort = 0;
#endif

	m_dd.slipStream = 0.f;
	m_dd.forcedAcceleration = 0.f;

	Clear(&m_dd.acceleration);
	Clear(&m_dd.angularAcceleration);
}

void Vehicle::Shutdown()
{
	m_dd.networkForces.Reset();
}

void Vehicle::ToggleAIControl()
{
	if (m_dd.controlType == Vehicle::CONTROL_TYPE_PLAYER) 
		m_dd.controlType = RacingPhysics::Vehicle::CONTROL_TYPE_AI;
	else if (m_dd.controlType == RacingPhysics::Vehicle::CONTROL_TYPE_AI) 
		m_dd.controlType = Vehicle::CONTROL_TYPE_PLAYER;
}

#if !PLATFORM_SPU

void Vehicle::BeginFrame(float largestSpeed)
{
	m_dd.oldVelocity = GetVelocity();
	m_dd.oldAngularVelocity = GetAngularVelocity();
}

void Vehicle::EndFrame(const Maths::C_TimeS PCREF time)
{
	float t = PhysicsTimer::Instance().PhysicsUpdateTime() * g_timeStepScale;
	m_dd.acceleration = (GetVelocity() - m_dd.oldVelocity) / t;
	m_dd.angularAcceleration = (GetAngularVelocity() - m_dd.oldAngularVelocity) / t;

	float a = Magnitude(m_dd.acceleration) / PhysicsConstants::GRAVITY;
}

#endif

// Default is to only collide with walls
/*U32 Vehicle::GetCollisionFlags() const 
{
	return COLLISION_FLAG_NOT_GROUND;
}*/

void Vehicle::Stop()
{
	Clear(&m_dd.acceleration);
	Clear(&m_dd.angularAcceleration);
	Clear(&m_dd.oldVelocity);
	Clear(&m_dd.oldAngularVelocity);
}



void Vehicle::DisplayDebug() const
{

}

void Vehicle::GetDmaData(void** pDmaData)
{
	pDmaData[0] = &m_dd;
}

void Vehicle::SpuFetch(const void* pDmaData)
{
	Spu::Fetch(&m_dd, pDmaData, sizeof(DmaData));
}

void Vehicle::SpuStore(void* pDmaData) const
{
	Spu::Store(pDmaData, &m_dd, sizeof(DmaData));
}

}
