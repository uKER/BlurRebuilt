/////////////////////////////////////////////////////////////
// VehicleActor
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_ACTOR_H__
#define __VEHICLE_ACTOR_H__

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>

#include <game_resource/ModelResource.h>

namespace Vehicle { class C_VehicleModel; }
namespace Actors { class C_VisibleDetect; }
namespace Game { class C_GameWorld; }
namespace RacingPhysics { class Car; }
namespace Cam2 { class C_ChaseCamera2; }
namespace GameResource { class C_Model; }
namespace WorldRender { class C_ModelInstance; }

namespace Game
{

namespace Actors
{

/* 
===============================================================
					class C_VehicleActor

NOTES: 
===============================================================
*/
class C_VehicleActorCreate : public ::Actors::C_PosActorCreate
{
public:

	static Actor::C_ActorTypeId ClassType() { return E_ActorTypeId_Vehicle; }
	const Actor::C_ActorTypeId GetType() const { return E_ActorTypeId_Vehicle; }

	Util::C_String				m_ModelFilename;

	Game::C_GameWorld*			m_pGameWorld;

	//Vehicle::C_VehicleModel*	m_pVehicleModel;
	//RacingPhysics::Vehicle*		m_pVehiclePhysics;
};

/* 
===============================================================
					class C_VehicleActor

NOTES: 
===============================================================
*/
class C_VehicleActor : public C_GameActor//, public Resource::C_Listener
{
public:

	C_VehicleActor();

	~C_VehicleActor();

	const Maths::HMat43 &			GetMatrix() const {return m_Matrix;}

	static const Actor::C_ActorTypeId		TypeId()	{ return E_ActorTypeId_Vehicle; }
	static const Btl::String32	TypeName()	{ return "Vehicle"; }

	//! Update
	void Update();

#ifndef CFG_GOLD
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const; // from Actor::C_Actor
#endif

	virtual bool OnMessage(const Messaging::C_MessageBase *pMessage);

	//! Get pointer to vehicle model
	Vehicle::C_VehicleModel* GetVehicleModel() { return m_pVehicleModel; }
	void SetVehicleModel(Vehicle::C_VehicleModel* p_pVehicleModel) { m_pVehicleModel = p_pVehicleModel; }

	//! Get pointer to vehicle physics in the racing physics
	RacingPhysics::Car* GetVehiclePhysics() { return m_pVehiclePhysics; }
	void SetVehiclePhysics(RacingPhysics::Car* p_pVehiclePhysics) { m_pVehiclePhysics = p_pVehiclePhysics; }

	//! Syncs racing physics data with model data - done in a thread-safe happy place
	void SynchData();

	void ResourceNotify(Resource::C_ResourceBase* p_pResource, const Resource::E_Notify p_Result, void* p_pData);

	void Create();

	//! Create vehicle physics body
	void CreateCollisionMesh(const GameResource::C_Model* p_pPhysicsModel);

	void Destroy();

protected:

	virtual bool InitialiseImpl();
	virtual bool CreateImpl();

private:
	
	Maths::HMat43								m_Matrix;
	Maths::HMat43								m_BirthPlace;

	Vehicle::C_VehicleModel*					m_pVehicleModel;					//!< Render/game model
	
	RacingPhysics::Car*						m_pVehiclePhysics;					//!< Physics model
		
	Cam2::C_ChaseCamera2*						m_pChaseCamera;

	Gravity::C_Body*							m_pCollisionBody;

	Ut::Vector<WorldRender::C_ModelInstance*>	m_pModelInstances;

	::Actors::C_VisibleDetect*					m_pVisDetect;

	GameResource::C_ModelHandle					m_ModelHandle;
	GameResource::C_ModelResource*				m_pModelResource;
};

} //namespace Actors

} //namespace Game

#endif
