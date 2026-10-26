//=============================================================================
// VLC_Sensor.cpp
// 30/04/09
//=============================================================================

//=============================================================================
// Includes
//=============================================================================
#include "stdafx.h"

#include "VLC_Sensor.h"
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <GameLogic\Layers\AmaxWorldLayer.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\Game.h>
#include <Game\GameWorld\GameWorld.h>
#include <Game\GameDef\Polyflags.h>
#include <GameApp.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <system\debug\I_DebugRender.h>
#include <shared_actors\SharedActorUtils.h>

Debug::Bool g_DebugVehicleShowSensorDebug("Vehicle.ShowSensorDebug", false);

namespace Kanzen
{
	
	//=========================================================================
	// Data
	//=========================================================================
	static const F32 s_maxCosine = Maths::Cos(Maths::DegToRad(55.0f));
	//=========================================================================
	// Class Definitions
	//=========================================================================
	//=========================================================================
	// I_VLC_Sensor_CreateInfo
	//=========================================================================
	//-------------------------------------------------------------------------
	// SetParentActorHandle
	//-------------------------------------------------------------------------
	void 
	I_VLC_Sensor_CreateInfo::SetParentActorHandle(
		const Actor::C_ActorHandle& parentActorHandle
	)
	{
		m_ParentActorHandle=parentActorHandle;
	}

	//-------------------------------------------------------------------------
	// SetVehicleIndex
	//-------------------------------------------------------------------------
	void 
	I_VLC_Sensor_CreateInfo::SetVehicleIndex(
		int vehicleIndex
	)
	{
		m_VehicleIndex=vehicleIndex;
	}

	//-------------------------------------------------------------------------
	// GetParentActorHandle
	//-------------------------------------------------------------------------
	const Actor::C_ActorHandle& 
	I_VLC_Sensor_CreateInfo::GetParentActorHandle(
	) const
	{
		return m_ParentActorHandle;
	}

	//-------------------------------------------------------------------------
	// GetVehicleIndex
	//-------------------------------------------------------------------------
	int 
	I_VLC_Sensor_CreateInfo::GetVehicleIndex(
	) const
	{
		return m_VehicleIndex;
	}

	//=========================================================================
	// C_VLC_Sensor::proximalEntity
	//=========================================================================
	//-------------------------------------------------------------------------
	// Constructor
	//-------------------------------------------------------------------------
	C_VLC_Sensor::proximalEntity::proximalEntity()
	{
		Reset();
	}
	//-------------------------------------------------------------------------
	// Reset
	//-------------------------------------------------------------------------
	void
	C_VLC_Sensor::proximalEntity::Reset()
	{
		m_timeBeingTracked = -1.0f;
		m_timeinProximityCurrent = -1.0f;
		m_timeinProximityMax = Maths::Constants<F32>::Min();
		m_timeinProximityTotal = 0.0f;
		m_timeSinceLastCollision = -1.0f;
		m_distanceCurrent = Maths::Constants<F32>::Max();
		m_distanceMinimum = Maths::Constants<F32>::Max();
		m_distanceMaximum = Maths::Constants<F32>::Min();
		m_targetActor = Actor::C_ActorHandle();
		m_targetBody = NULL;
		m_needsUpdate = true;
	}

	//-------------------------------------------------------------------------
	// Valid
	//-------------------------------------------------------------------------
	bool				
	C_VLC_Sensor::proximalEntity::IsInUse(
	) const
	{
		return m_timeBeingTracked >= 0.0f;
	}
	//-------------------------------------------------------------------------
	// IsActor
	//-------------------------------------------------------------------------
	bool				
	C_VLC_Sensor::proximalEntity::IsActor(
	) const
	{
		Actor::C_ActorList* actorList		= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList();
		return actorList->ActorExists(m_targetActor);	
	}
	//-------------------------------------------------------------------------
	// IsStaticGeometry
	//-------------------------------------------------------------------------	
	bool				
	C_VLC_Sensor::proximalEntity::IsStaticGeometry(
	) const
	{
		return !IsActor();
	}
	//-------------------------------------------------------------------------
	// IsInProximity
	//-------------------------------------------------------------------------	
	bool				
	C_VLC_Sensor::proximalEntity::IsInProximity(
		) const
	{
		return m_timeinProximityCurrent > 0.0f;
	}

	//-------------------------------------------------------------------------
	// HasCollidedWithinTime
	//-------------------------------------------------------------------------	
	bool				
		C_VLC_Sensor::proximalEntity::HasCollidedWithinTime(
		F32 time
		) const
	{
		if(m_timeSinceLastCollision>=0.0f)
		{
			return m_timeSinceLastCollision-time<0.0f;
		}
		return false;
	}

	//-------------------------------------------------------------------------
	// UpdateInProximity
	//-------------------------------------------------------------------------	
	void
	C_VLC_Sensor::proximalEntity::UpdateInProximity(
		F32 distance,
		F32 elapsedTime,
		const C_VLC_Sensor::proximalEntity::proximityData& data
	)
	{
		m_timeBeingTracked += elapsedTime;
		m_timeinProximityCurrent += elapsedTime;
		m_timeinProximityTotal += elapsedTime;
		if( m_timeSinceLastCollision >= 0.0f )
		{
			m_timeSinceLastCollision += elapsedTime;
		}
		if(m_timeinProximityCurrent>m_timeinProximityMax)
		{
			m_timeinProximityMax = m_timeinProximityCurrent;
		}

		m_dataLastValid = data;
		m_distanceCurrent = distance;
		m_distanceMaximum = Maths::Max(distance, m_distanceMaximum);
		if( distance < m_distanceMinimum )
		{
			m_distanceMinimum = distance;
			m_dataAtClosestDistance = data;
		}

		m_needsUpdate = false;
	}

	//-------------------------------------------------------------------------
	// Update
	//-------------------------------------------------------------------------
	void
	C_VLC_Sensor::proximalEntity::Update(
		F32 elapsedTime
	)
	{
		// Were we previously in proximity?
		if(IsInProximity())
		{
			m_timeinProximityCurrent = 0.0f;
		}
		else
		{
			m_timeinProximityCurrent -= elapsedTime;
		}

		m_timeBeingTracked += elapsedTime;
		if( m_timeSinceLastCollision >= 0.0f )
		{
			m_timeSinceLastCollision += elapsedTime;
		}
		m_distanceCurrent = -1.0f;
		m_needsUpdate = false;
	}

	//-------------------------------------------------------------------------
	// GetMinimumDistance
	//-------------------------------------------------------------------------	
	F32				
	C_VLC_Sensor::proximalEntity::GetMinimumDistance(
		) const
	{
		return m_distanceMinimum;
	}

	//-------------------------------------------------------------------------
	// GetMaximumDistance
	//-------------------------------------------------------------------------	
	F32				
	C_VLC_Sensor::proximalEntity::GetMaximumDistance(
		) const
	{
		return m_distanceMinimum;
	}

	//-------------------------------------------------------------------------
	// GetCurrentDistance
	//-------------------------------------------------------------------------	
	F32				
	C_VLC_Sensor::proximalEntity::GetCurrentDistance(
		) const
	{
		return m_distanceCurrent;
	}

	//-------------------------------------------------------------------------
	// GetTimeBeingTracked
	//-------------------------------------------------------------------------	
	F32				
	C_VLC_Sensor::proximalEntity::GetTimeBeingTracked(
		) const
	{
		return m_timeBeingTracked;
	}

	//-------------------------------------------------------------------------
	// GetCurrentTimeInProximity
	//-------------------------------------------------------------------------	
	F32				
	C_VLC_Sensor::proximalEntity::GetCurrentTimeInProximity(
		) const
	{
		return Maths::Clamped<F32>(m_timeinProximityCurrent,0.0f,Maths::Constants<F32>::Max());
	}

	//-------------------------------------------------------------------------
	// GetCurrentTimeInProximity
	//-------------------------------------------------------------------------	
	F32				
		C_VLC_Sensor::proximalEntity::GetMaxTimeInProximity(
		) const
	{
		return Maths::Clamped<F32>(m_timeinProximityMax,0.0f,Maths::Constants<F32>::Max());
	}
	//-------------------------------------------------------------------------
	// GetTimeSinceLastInProximity
	//-------------------------------------------------------------------------	
	F32				
	C_VLC_Sensor::proximalEntity::GetTimeSinceLastInProximity(
		) const
	{
		return Maths::Clamped<F32>(-m_timeinProximityCurrent,0.0f,Maths::Constants<F32>::Max());
	}

	//-------------------------------------------------------------------------
	// GetCurrentProximityData
	//-------------------------------------------------------------------------	
	const C_VLC_Sensor::proximalEntity::proximityData&	
	C_VLC_Sensor::proximalEntity::GetCurrentProximityData(
		) const
	{
		return m_dataLastValid;
	}

	//-------------------------------------------------------------------------
	// GetCurrentProximityData
	//-------------------------------------------------------------------------	
	const C_VLC_Sensor::proximalEntity::proximityData&	
	C_VLC_Sensor::proximalEntity::GetClosestProximityData(
		) const
	{
		return m_dataAtClosestDistance;
	}

	//-------------------------------------------------------------------------
	// GetTargetBody
	//-------------------------------------------------------------------------	
	const Gravity::C_Body*	
	C_VLC_Sensor::proximalEntity::GetTargetBody(
		) const
	{
		return m_targetBody;
	}

	//-------------------------------------------------------------------------
	// GetTargetActorHandle
	//-------------------------------------------------------------------------	
	Actor::C_ActorHandle	
	C_VLC_Sensor::proximalEntity::GetTargetActorHandle(
		) const
	{
		return m_targetActor;
	}
	
	//=========================================================================
	// proximalRaytraceInput
	//=========================================================================
	//-------------------------------------------------------------------------
	// Constructor
	//-------------------------------------------------------------------------
	C_VLC_Sensor::proximalRaytraceInput::proximalRaytraceInput( 
		const proximalRaytraceQueryData& data, 
		const Collision::C_CollisionFilter* filter, 
		const Maths::HMat43& toWorld,
		Gravity::C_Scene* physicsScene
		) : m_filter(filter), m_physicsScene(physicsScene)
	{
		Maths::HVec3 endPosLS;
		endPosLS = data.startPosLS + (data.startDirLS * data.length); 
		m_startPosWS = data.startPosLS * toWorld;
		m_endPosWS = endPosLS * toWorld;
	}

	//=========================================================================
	// C_VLC_Sensor
	//=========================================================================

	C_VLC_Sensor::C_VLC_Sensor()
	{
		m_rayGroup = Scheduler::JQ_GROUP_NONE;
	}

	C_VLC_Sensor::~C_VLC_Sensor()
	{
		Scheduler::DestroyGroup(m_rayGroup);
	}

	//-------------------------------------------------------------------------
	// CreateImpl
	//-------------------------------------------------------------------------
	bool 
	C_VLC_Sensor::CreateImpl(
	)
	{
		const I_VLC_Sensor_CreateInfo* creation_data = static_cast<const I_VLC_Sensor_CreateInfo*>(GetCreator().GetData());

		if(creation_data)
		{
			m_parentActorHandle=creation_data->GetParentActorHandle();
			m_vehicleIndex=creation_data->GetVehicleIndex();
		}

		AddSubscriber( Game::E_GameLogicMessage_Impact );

		// Setup our data
		m_leftData.length = 5.0f;
		m_leftData.startDirLS = Maths::HVec3(-1.0f,0.0f,0.0f);
		m_leftData.startPosLS = Maths::HVec3(-.2f,0.5f,0.0f);

		m_rightData.length = 5.0f;
		m_rightData.startDirLS = Maths::HVec3(1.0f,0.0f,0.0f);
		m_rightData.startPosLS = Maths::HVec3(.2f,0.5f,0.0f);

		m_rayGroup = Scheduler::CreateGroup();
		
		return true;
	}

	//-------------------------------------------------------------------------
	// MessageHandler
	//-------------------------------------------------------------------------
	bool 
	C_VLC_Sensor::OnMessage(
		const Messaging::C_MessageBase *pMessage
	) 
	{
		switch(pMessage->Type())
		{
			case Game::E_GameLogicMessage_Impact:
			{
				const Game::C_ImpactMsg* pMsg = Messaging::MessageCast<Game::C_ImpactMsg>(pMessage);
				ImpactEvent(pMsg);

				return true;
			}
		}

		return I_GameActorComponent::OnMessage(pMessage);
	}

	//-------------------------------------------------------------------------
	// UpdateFirstPass
	//-------------------------------------------------------------------------
	void 
	C_VLC_Sensor::UpdateFirstPass() 
	{
		PERF_EVENT("VLC_Sensor.UpdateFirstPass");

		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::C_VehicleActor* pGActor = pActor->GetGraphicalActor(m_vehicleIndex);

		float elapsedTime	= C_GameApp::GetInstance()->GetTimeStep();
		
		Collision::C_CollisionFilter rayCollisionFilter;

		rayCollisionFilter.SetBodyMask(Game::PolyFlags::kbVehicle | Game::PolyFlags::kbWorld);
		rayCollisionFilter.SetGroupId(pGActor->GetPhysicalGroupID());
		rayCollisionFilter.SetGroupFlags(0);
		
		const Maths::Mat43& toWorld = pGActor->GetMatrix();
		m_leftInput = proximalRaytraceInput(m_leftData,&rayCollisionFilter,toWorld,pGActor->GetPhysicsScene());
		m_rightInput = proximalRaytraceInput(m_rightData,&rayCollisionFilter,toWorld,pGActor->GetPhysicsScene());

		for(int i=0;i<s_maxProximalEntities;i++)
		{
			m_entities[i].m_needsUpdate = true;
		}
		
		ProcessQuery(m_leftInput,m_leftOutput);
		ProcessQuery(m_rightInput,m_rightOutput);	
	}

	//-------------------------------------------------------------------------
	// UpdateSecondPass
	//-------------------------------------------------------------------------
	void 
	C_VLC_Sensor::UpdateSecondPass() 
	{
		PERF_EVENT("VLC_Sensor.UpdateSecondPass");

		float elapsedTime	= C_GameApp::GetInstance()->GetTimeStep();

		Scheduler::Flush(m_rayGroup);

		ProcessResult(m_leftInput,m_leftOutput, elapsedTime);
		ProcessResult(m_rightInput,m_rightOutput, elapsedTime);

		for(int i=0;i<s_maxProximalEntities;i++)
		{
			if(m_entities[i].IsInUse()&&m_entities[i].m_needsUpdate)
			{
				static F32 s_maxTimeSinceLastInProximity = 2.5f;
				m_entities[i].Update(elapsedTime);
				if(m_entities[i].GetTimeSinceLastInProximity() > s_maxTimeSinceLastInProximity)
				{
					m_entities[i].m_timeBeingTracked = -1.0f;
				}
			}
		}

#ifndef CFG_GOLD
		if(g_DebugVehicleShowSensorDebug)
		{
			DebugDrawResult(m_leftInput,m_leftOutput);
			DebugDrawResult(m_rightInput,m_rightOutput);
		}
#endif			
	}

	//-------------------------------------------------------------------------
	// Update
	//-------------------------------------------------------------------------
	void 
	C_VLC_Sensor::Update() 
	{
	
	}

	//-------------------------------------------------------------------------
	// ImpactEvent
	//-------------------------------------------------------------------------
	void 
	C_VLC_Sensor::ImpactEvent( 
		const Game::C_ImpactMsg* pMsg 
	)
	{
		for(int i=0;i<s_maxProximalEntities;i++)
		{
			if(m_entities[i].IsInUse() && 
			  (m_entities[i].GetTargetActorHandle() == pMsg->mActorHandleA ||
			   m_entities[i].GetTargetActorHandle() == pMsg->mActorHandleB))
			{
				m_entities[i].m_timeSinceLastCollision = 0.0f;
			}
		}
	}

	//-------------------------------------------------------------------------
	// GetNumEntitiesInProximity
	//-------------------------------------------------------------------------
	int									
	C_VLC_Sensor::GetMaxProximalEntities(
	) const
	{
		return s_maxProximalEntities;
	}

	//-------------------------------------------------------------------------
	// GetProximalEntityAtIndex
	//-------------------------------------------------------------------------
	const C_VLC_Sensor::proximalEntity&				
	C_VLC_Sensor::GetProximalEntityAtIndex(
		int index
	) const
	{
		BIZ_ASSERT(index>=0&&index<s_maxProximalEntities);
		return m_entities[index];
	}


/*
	//-------------------------------------------------------------------------
	// GetProximalEntityInDirWS
	//-------------------------------------------------------------------------
	const C_VLC_Sensor::proximalEntity&				
	C_VLC_Sensor::GetProximalEntityInDirWS(
		const Maths::HVec3& dir, 
		F32 angularTollerance)
	{
		Maths::HVec3 dirLS;
		Maths::HMat43 invWorldMatrix;
		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::C_VehicleActor* pGActor = pActor->GetGraphicalActor(m_vehicleIndex);
		const Maths::HMat43& worldMatrix = pGActor->GetMatrix();
		
		invWorldMatrix = worldMatrix.InverseRotTrans();
		dirLS = dir * invWorldMatrix ;
		
		return GetProximalEntityInDirLS(dirLS,angularTollerance);
	}

	//-------------------------------------------------------------------------
	// GetProximalEntityInDirLS
	//-------------------------------------------------------------------------
	const C_VLC_Sensor::proximalEntity&				
	C_VLC_Sensor::GetProximalEntityInDirLS(
		const Maths::HVec3& dir, 
		F32 angularTollerance
	)
	{
		Maths::HVec3 LeftDirLS( -1.0f, 0.0f, 0.0f);
		Maths::HVec3 RightDirLS( 1.0f, 0.0f, 0.0f);

		F32 angularTolleranceCosine = Maths::Cos(angularTollerance);
		if(Maths::DotProduct(dir,LeftDirLS)>angularTolleranceCosine)
		{
			return m_leftSide;
		}
		else if(Maths::DotProduct(dir,RightDirLS)>angularTolleranceCosine)
		{
			return m_rightSide;
		}
		return s_dummyEntity;
	}
*/

	//-------------------------------------------------------------------------
	// ProcessResult
	//-------------------------------------------------------------------------
	void
	C_VLC_Sensor::ProcessResult(
		const proximalRaytraceInput& input,
		const proximalRaytraceOutput& output,
		F32 elapsedTime
	)
	{
		const Collision::C_RayTracingResult& result = output.m_ray.m_result;
		if( result.Hit() )
		{	
			Maths::HVec3 rayDir = input.m_endPosWS - input.m_startPosWS;
			rayDir = Maths::Normalise(rayDir);
			F32 dot =rayDir|result.Normal();
			dot = fabs(dot);
			if(dot>s_maxCosine)
			{
				// Are we already tracking an entity with the same actor handle
				
				const Gravity::C_Body* body = result.GetBody();

				Game::C_BodyUserDataBase* userData=body->GetUserData<Game::C_BodyUserDataBase>();

				{
					Actor::C_ActorHandle hdl;
					if(userData&&userData->m_eType==Game::E_BodyUserDataType_Actor)
					{
						hdl = static_cast<Game::C_BodyUserDataActor*>(userData)->m_hActorHandle;
					}

					bool newEntity = false;
					proximalEntity* entity = FindProximalEntity(hdl);
					if(!entity)
					{
						entity = FindUnusedEntity();
						newEntity = true;
					}
					if(!entity)
					{
						entity = FindLeastRecentlyUsedEntity();
						newEntity = true;
					}
					if(entity)
					{
						if(newEntity)
						{
							entity->Reset();
							entity->m_targetActor = hdl;
							entity->m_targetBody = body;
							entity->m_timeBeingTracked = 0.0f;
							entity->m_timeinProximityCurrent = 0.0f;
						}
						
						Maths::HVec3 dirWS = input.m_endPosWS - input.m_startPosWS;
						Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());
						Game::Actors::C_VehicleActor* pGActor = pActor->GetGraphicalActor(m_vehicleIndex);
						Maths::HVec3 myVelocity = pGActor->GetVehiclePhysics()->GetVelocity();

						proximalEntity::proximityData data;
						data.closestPointOnMyselfNormal = dirWS;
						data.closestPointOnTargetNormal = result.Normal();
						data.closestPointOnTargetPosition = result.HitPoint();
						data.closestPointOnMyselfPosition = input.m_startPosWS;
						data.targetVelocity = body?body->LinearVelocity():Maths::Constants<Maths::HVec3>::Zero();
						data.relativeVelocity = data.targetVelocity - myVelocity;
						data.myVelocity = myVelocity;
						entity->UpdateInProximity(result.Distance(),elapsedTime,data);
					}
				}
			}
		}
	}

	//-------------------------------------------------------------------------
	// proximalQuery
	//-------------------------------------------------------------------------
	void
	C_VLC_Sensor::ProcessQuery(
		const proximalRaytraceInput& input,
		proximalRaytraceOutput& output	
	)
	{		
		output.m_ray.InitialiseRay(input.m_startPosWS, input.m_endPosWS, input.m_filter);
		input.m_physicsScene->DoDeferredRayTrace(&output.m_ray, NULL, m_rayGroup, Game::C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy());		
	}

	//-------------------------------------------------------------------------
	// FindProximalEntity
	//-------------------------------------------------------------------------
	C_VLC_Sensor::proximalEntity*						
	C_VLC_Sensor::FindProximalEntity(Actor::C_ActorHandle handle)
	{
		for(int i=0;i<s_maxProximalEntities;i++)
		{
			if(m_entities[i].m_targetActor == handle && m_entities[i].IsInUse())
			{
				return &m_entities[i];
			}
		}
		return NULL;
	}
	
	//-------------------------------------------------------------------------
	// FindUnusedEntity
	//-------------------------------------------------------------------------
	C_VLC_Sensor::proximalEntity*					
	C_VLC_Sensor::FindUnusedEntity()
	{
		for(int i=0;i<s_maxProximalEntities;i++)
		{
			if(m_entities[i].IsInUse()==false)
			{
				return &m_entities[i];
			}
		}
		return NULL;
	}

	//-------------------------------------------------------------------------
	// FindLeastRecentlyUsedEntity
	//-------------------------------------------------------------------------
	C_VLC_Sensor::proximalEntity*					
	C_VLC_Sensor::FindLeastRecentlyUsedEntity()
	{
		int lru = -1;
		for(int i=0;i<s_maxProximalEntities;i++)
		{
			if(m_entities[i].IsInUse()==false)
			{
				return &m_entities[i];
			}
			if(!m_entities[i].IsInProximity())
			{
				if(lru==-1)
				{
					lru=i;
				}
				else if( m_entities[i].GetTimeSinceLastInProximity() > m_entities[lru].GetTimeSinceLastInProximity())
				{
					lru=i;
				}
			}
		}
		return lru==-1?NULL:&m_entities[lru];
	}
	//-------------------------------------------------------------------------
	// DebugDrawResult
	//-------------------------------------------------------------------------
#ifndef CFG_GOLD
	void
	C_VLC_Sensor::DebugDrawResult(
		const proximalRaytraceInput& input,
		const proximalRaytraceOutput& output
		)
	{
		const Maths::HVec3& endPosWS = output.m_ray.m_result.Hit()?output.m_ray.m_result.HitPoint():input.m_endPosWS;
	
		static Maths::RGBA Red = Maths::RGBA(0xFF, 0x00, 0x00, 0xff);
		static Maths::RGBA Green = Maths::RGBA(0x00, 0xFF, 0x00, 0xff);
		static Maths::RGBA Blue = Maths::RGBA(0x00, 0x00, 0xFF, 0xff);
		Maths::RGBA colour;
	
		Maths::HVec3 rayDir = input.m_endPosWS - input.m_startPosWS;
		rayDir = Maths::Normalise(rayDir);
		F32 dot =rayDir|output.m_ray.m_result.Normal();
		dot = fabs(dot);
				
		if(output.m_ray.m_result.Hit()&&dot>s_maxCosine)
		{
			colour = Green;
		}
		else
		{
			colour = Red;
		}
	
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(input.m_startPosWS,endPosWS,colour);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(endPosWS, 0.05f, colour);

	}
#endif // CFG_GOLD

	//-------------------------------------------------------------------------
	// DebugDrawEntity
	//-------------------------------------------------------------------------
#ifndef CFG_GOLD
	void
	C_VLC_Sensor::DebugDrawEntity(
		const proximalEntity& entity
		)
	{
		
		static Maths::RGBA Red = Maths::RGBA(0xFF, 0x00, 0x00, 0xff);
		static Maths::RGBA Green = Maths::RGBA(0x00, 0xFF, 0x00, 0xff);
		static Maths::RGBA Blue = Maths::RGBA(0x00, 0x00, 0xFF, 0xff);
		Maths::RGBA colour;
		Maths::HVec3 position;
		if(entity.IsInUse())
		{
			if(entity.IsInProximity())
			{
				colour = Green;
			}
			else
			{
				colour = Red;
			}
			
			
			position = entity.m_dataLastValid.closestPointOnTargetPosition;
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(position, 0.05f, colour);
		}
	}
#endif // CFG_GOLD


} // namespace Kanzen
