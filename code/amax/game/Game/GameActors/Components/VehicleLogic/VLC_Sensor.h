//=============================================================================
// VLC_Sensor.h
// 30/04/09
//=============================================================================
#ifndef _VLC_SENSOR_H_
#define _VLC_SENSOR_H_


//=============================================================================
// Includes
//=============================================================================
#include <Game\GameActors\Components\GameComponents.h>
#include <Game\Messaging\GameMessages.h>
#include <SchedulerConfig.h>

//=============================================================================
// Forward Declarations
//=============================================================================
namespace Game
{
	namespace Actors
	{
		class C_VehicleLogicActor;
	}

	class C_ImpactMsg;
}

namespace Kanzen
{

	class BaseMessageHandler;
	//=============================================================================
	// Class Definitions
	//=============================================================================
	//---------------------------------------------------------------------
	// I_VLC_Sensor_CreateInfo
	//---------------------------------------------------------------------
	class I_VLC_Sensor_CreateInfo : public Components::I_GameComponentCreator
	{
	public:
		I_VLC_Sensor_CreateInfo() {}

		static	Component::C_ComponentTypeId		ClassType() {return Components::E_VehicleComponent_Sensor;}
		virtual	Component::C_ComponentTypeId		GetComponentTypeID() const {return Components::E_VehicleComponent_Sensor;}

				void								SetParentActorHandle(const Actor::C_ActorHandle& parentActorHandle);
				void								SetVehicleIndex(int vehicleIndex);

				const Actor::C_ActorHandle&			GetParentActorHandle() const;
				int									GetVehicleIndex() const;

	public:
				Actor::C_ActorHandle				m_ParentActorHandle;
				int									m_VehicleIndex;
	};

	//---------------------------------------------------------------------
	// C_VLC_Sensor
	//---------------------------------------------------------------------
	class C_VLC_Sensor : public I_GameActorComponent
	{
	public:

		C_VLC_Sensor();
		~C_VLC_Sensor();

		//-----------------------------------------------------------------
		// proximalEntity
		//-----------------------------------------------------------------
		class proximalEntity
		{
		public:
			friend class C_VLC_Sensor;

		
			class proximityData
			{
			public:
				Maths::HVec3 closestPointOnTargetNormal;
				Maths::HVec3 closestPointOnTargetPosition;
				Maths::HVec3 closestPointOnMyselfNormal;
				Maths::HVec3 closestPointOnMyselfPosition;
				Maths::HVec3 relativeVelocity;
				Maths::HVec3 targetVelocity;
				Maths::HVec3 myVelocity;

				proximityData() : closestPointOnTargetNormal(Maths::UNDEFINED), closestPointOnTargetPosition(Maths::UNDEFINED),
					closestPointOnMyselfNormal(Maths::UNDEFINED), closestPointOnMyselfPosition(Maths::UNDEFINED), relativeVelocity(Maths::UNDEFINED),
					targetVelocity(Maths::UNDEFINED), myVelocity(Maths::UNDEFINED)
				{
				}

			};
		
		public:
									proximalEntity();
			void					Reset();
			bool					IsInUse() const;
			bool					IsActor() const;
			bool					IsStaticGeometry() const;
			bool					IsInProximity() const;
			bool					HasCollidedWithinTime(F32 time) const;
			
			void					PreUpdate();
			void					UpdateInProximity(F32 distance,F32 elapsedTime,const proximityData& data);
			void					Update(F32 elapsedTime);
		
			F32						GetMinimumDistance() const;
			F32						GetMaximumDistance() const;
			F32						GetCurrentDistance() const;

			F32						GetTimeBeingTracked() const;
			F32						GetCurrentTimeInProximity() const;
			F32						GetTotalTimeInProximity() const;
			F32						GetMaxTimeInProximity() const;
			F32						GetTimeSinceLastInProximity() const;

			const proximityData&	GetCurrentProximityData() const;
			const proximityData&	GetClosestProximityData() const;

			const Gravity::C_Body*	GetTargetBody() const;
			Actor::C_ActorHandle	GetTargetActorHandle() const;

		private:

			bool m_needsUpdate;
			F32 m_distanceMinimum;
			F32 m_distanceMaximum;
			F32 m_distanceCurrent;
			F32 m_timeinProximityCurrent;
			F32 m_timeinProximityMax;
			F32 m_timeinProximityTotal;
			F32 m_timeBeingTracked;
			F32 m_timeSinceLastCollision;
			
			proximityData m_dataLastValid;
			proximityData m_dataAtClosestDistance;

			const Gravity::C_Body* m_targetBody;
			Actor::C_ActorHandle m_targetActor;
			
		};


	

	public:

		static	Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_VehicleComponent_Sensor;}
		virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_VehicleComponent_Sensor;} // from Component::I_GameComponent


		static	Component::C_ComponentInterfaceId	ClassType() {return Components::E_VehicleComponent_Sensor;}
		virtual Component::C_ComponentInterfaceId	GetComponentTypeID() const {return Components::E_VehicleComponent_Sensor;} // from Component::I_GameComponent

		void										UpdateFirstPass();
		void										UpdateSecondPass();
		void										Update();

		virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);

				int									GetMaxProximalEntities() const;
				const proximalEntity&				GetProximalEntityAtIndex(int index) const;
//				const proximalEntity&				GetProximalEntityInDirWS(const Maths::HVec3& dir, F32 angularTollerance = Maths::DegToRad(45.0f));
//				const proximalEntity&				GetProximalEntityInDirLS(const Maths::HVec3& dir, F32 angularTollerance = Maths::DegToRad(45.0f));
	
	protected:
		//-----------------------------------------------------------------
		// proximalRaytraceQueryData
		//-----------------------------------------------------------------
		class proximalRaytraceQueryData
		{
		public:
			Maths::HVec3 startPosLS;
			Maths::HVec3 startDirLS;
			F32	length;
		};

		//-----------------------------------------------------------------
		// proximalRaytraceInput
		//-----------------------------------------------------------------
		class proximalRaytraceInput
		{
		public:
			proximalRaytraceInput() { m_filter = NULL; m_physicsScene = NULL; }
			proximalRaytraceInput( const proximalRaytraceQueryData& data, const Collision::C_CollisionFilter* filter, const Maths::HMat43& toWorld, Gravity::C_Scene* physicsScene);
		public:
			Maths::HVec3 m_startPosWS;
			Maths::HVec3 m_endPosWS;
			const Collision::C_CollisionFilter* m_filter;
			Gravity::C_Scene* m_physicsScene;
		};


		//-----------------------------------------------------------------
		// proximalRaytraceOutput
		//-----------------------------------------------------------------
		class proximalRaytraceOutput
		{
		public:
			Collision::C_Ray m_ray;
		};
	
	
	protected:
				void								ImpactEvent( const Game::C_ImpactMsg* pMsg );
	
		virtual bool								CreateImpl();

		void										ProcessQuery(const proximalRaytraceInput& input,proximalRaytraceOutput& output);
		void										ProcessResult(const proximalRaytraceInput& input,const proximalRaytraceOutput& output,F32 elapsedTime);

	#ifndef CFG_GOLD	
		void										DebugDrawResult(const proximalRaytraceInput& input,const proximalRaytraceOutput& output);
		void										DebugDrawEntity(const proximalEntity& entity);
	#endif 		

				proximalEntity*						FindProximalEntity(Actor::C_ActorHandle);
				proximalEntity*						FindUnusedEntity();
				proximalEntity*						FindLeastRecentlyUsedEntity();

				Actor::C_ActorHandle				m_parentActorHandle;
				int									m_vehicleIndex;

		static const int							s_maxProximalEntities = 10;
				proximalRaytraceQueryData			m_leftData;
				proximalRaytraceQueryData			m_rightData;
				
				proximalEntity						m_entities[s_maxProximalEntities];

				proximalRaytraceInput				m_leftInput;
				proximalRaytraceInput				m_rightInput;
				proximalRaytraceOutput				m_leftOutput;
				proximalRaytraceOutput				m_rightOutput;

				Scheduler::GroupHandle				m_rayGroup;
	};

}

#endif // _VLC_SENSOR_H_
