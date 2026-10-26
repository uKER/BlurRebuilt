
#ifndef PickUpSubsystem_H
#define PickUpSubsystem_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AppInterfaceLayer.h"
#include <Game/GameActors/AudioActors.h>

#define MAX_PICKUP_INSTANCES 12
#define RAYTRACE_FRAME_SKIP 30

enum E_PICKUP_LIST_TYPE
{
	E_PICKUP_LIST_TYPE_NONE=-1,
	E_PICKUP_LIST_TYPE_CHARGEFIRE=0,
	E_PICKUP_LIST_TYPE_HITSHIELD,
	E_PICKUP_LIST_TYPE_PROJECTILE,

	E_PICKUP_LIST_TYPE_MAX,
};

enum E_PICKUP_ENTITY_MEMBER_HANDLE
{
	ObjectVelocity,
	ObjectPosition,
	ObjectDirection,
	ListenerPosition,
	ListenerDirection,
	ListenerUp,
	ListenerVelocity,
	PriorityVolume,
	OcclusionValue,

	E_PICKUP_ENTITY_MEMBER_HANDLE_MAX
};
	  
class PickUpSubsystem : public Audio::Subsystem/*,  public Messaging::C_MessageHandler*/
{
public:

		enum Message
		{
			E_PickUpSubsystem_Fire,
			E_PickUpSubsystem_ReverseFire,
			E_PickUpSubsystem_Ended,
			E_PickUpSubsystem_Destroy,
			E_PickUpSubsystem_Hit,
			E_PickUpSubsystem_Missed,
			E_PickUpSubsystem_Blocked,
			E_PickUpSubsystem_Charging,
			E_PickUpSubsystem_Update,
			E_PickUpSubsystem_Awarded,
			E_PickUpSubsystem_UpdateMatrix,
			E_PickUpSubsystem_Fire_Projectile,
			E_PickUpSubsystem_Projectile_Hit_World,

			E_PickUpSubsystem_AudioFinished,
			E_PickUpSubsystem_SetFXDurations,
		};

	class PickupObject
	{
	public:
		PickupObject() : m_pOcclusionRay(NULL)
		{
		}

		~PickupObject()
		{
			delete m_pOcclusionRay;
		}

		void Reset();

		AudioEntityHandle m_hEntity; //assume max for now...
		bool m_markForDelete;
		Audio::PagedArrayHandle m_hQueryEntityFinished;
		Maths::HMat43 m_Matrix;
		
		Maths::HVec3 m_prevObjPos;
		Maths::HVec3 m_prevObjDir;
		Maths::HVec3 m_prevObjVel;
		F32 m_prevPriorityVolume;
		S32 m_PrevOcclusionVal;

		U64 m_UniqueID; // vehicle index combined with the perk id to make a super unique id 
		bool m_Charger;
		bool m_Follower;
		bool m_Projectile;
		S16 m_vehicleIndex;
		S16 m_targetIndex;
		F32 m_fDistance;
		F32 m_fTimeTriggered;

		E_PICKUP_TYPE m_eType; 
		E_PICKUP_LIST_TYPE m_eListType;
		S32 m_translatedType;
		Audio::EntityDataMemberHandle hEntityMemberHandle[E_PICKUP_ENTITY_MEMBER_HANDLE_MAX];
		Collision::C_Ray* m_pOcclusionRay;
		bool m_waitingForRaytraceResult;
		F32	m_fTimeOfLastRT;
		bool m_firedByFocusVehicle;
		bool m_IsSuspendedPrePause;
	};

	class PickupCollection
	{
	public:
		void Reset()
		{
			for ( Btl::Map<U64, PickupObject*>::Iterator it = m_Map.Begin(); it != m_Map.End(); ++it )
				it->GetValue()->Reset();
		}

		PickupObject* LookupById(U64 id) 
		{ 
			Btl::Map<U64, PickupObject*>::Iterator it = m_Map.Find(id);
			if (it == m_Map.End())
				return NULL;
			else
				return it->GetValue();
		}
		U32 Size() { return m_Map.Size(); }

		Btl::Map<U64, PickupObject*>* GetMap(){ return &m_Map;}

		Btl::Map<U64, PickupObject*> m_Map;

		int m_pickupsWithinMixScaleDistance;
	};

	enum E_PickUpSubSystemMessages
	{
		//E_PickUpSubSystemMessages_PickUpFinished

	};

	PickUpSubsystem();
	~PickUpSubsystem() { }

	bool		ProcessSystemMessage(const U32 u32MessageId);
	bool		ProcessCustomMessage(const U32 u32MessageId, U32 u32Data = 0, void* pData = 0);

	bool		SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)
	{
		return true;
	}

	static void									SetSchedulerGroup(Scheduler::GroupHandle handle);
	static Scheduler::GroupHandle				GetSchedulerGroup() { return m_hSchedulerGroup; }

	void		DeferredUpdate();
	void		Update();
	void		UpdatePickup(PickupObject* pObj);
	void		RenderDebug();
	void		PopulateEntityDefinitions();
	bool		MessageHandler(const Messaging::C_MessageBase *pMessage );
	PickupObject* CreatePickup( U64 uid,S32 ePickupType, U8 vehicleIndex, E_PICKUP_LIST_TYPE eType);
	void		Connect(AudioEntityHandle m_hSourceEntity,AudioEntityHandle m_hDestEntity);

	void		DoPriorityVolumeScaling(PickupObject* pObj);
	bool		DeleteDoomedObjects(PickupObject* pObj, Btl::Map<U64, PickupObject*>::Iterator iter,U32 collectionType );
	void		DoOcclusionTest(Maths::HMat43* raySourceMatrix, Maths::HMat43* rayDestMatrix, F32 distanceToDest, Gravity::C_Scene* scene, PickupObject* obj);
	void		ProcessOcclusionResult(PickupObject * pObj);
	void		CreateProjectilePickUp(void* pData = 0);


	U64 GetUniqueId(U8 vehicleId, U32 pickupId) 
	{ 
		U64 id = (static_cast<U64>(vehicleId) << 32 | pickupId);
		return id; 
	}

	PickupCollection& GetPickupCollection(U32 uType)
	{ 
	
		switch(uType)
		{
			case E_PICKUP_LIST_TYPE_CHARGEFIRE:
				return m_pickupsChargeFire;
				break;

			case E_PICKUP_LIST_TYPE_PROJECTILE:
				return m_pickupsProjectile;
				break;

			case E_PICKUP_LIST_TYPE_HITSHIELD:
				return m_pickupsHitShield;
				break;

			default:
				return m_pickupsChargeFire;
				break;

		}

		
	}

private:
	AudioEntityHandle hEntity;

	Actor::C_ActorHandle	m_PickUpHandle;
	Audio::XtEntityProxy*	pXtEntity;
	PickupObject			m_pickupObject[MAX_PICKUP_INSTANCES*E_PICKUP_LIST_TYPE_MAX];//twice as big to store the hit/shield and award versions
	
	PickupCollection		m_pickupsChargeFire;
	PickupCollection		m_pickupsHitShield;
	PickupCollection		m_pickupsProjectile;
	
	Audio::XtEntityProxy*	pickupEntityDefinition[E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_MAX];
	S32						m_EntityInstanceCount;
	bool					m_removeAllSubscribers;
	bool					m_isSplitScreen;
	bool					m_bRaceInProgress;
	Btl::MultiMap<F32, U64>		m_DistanceMap;
	F32						m_AudibleDistance;
	UI::E_MultiplayerVehicleAbilities abilitySlots[UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX];

	static Scheduler::GroupHandle				m_hSchedulerGroup;
	bool					m_gamePaused;
	F32						m_pauseTime;
	Scheduler::PolicyHandle			m_Policy;
	Scheduler::GroupHandle			m_Group;
	Maths::HMat43 m_camMatrix;
	Game::Actors::RaceLogicCore* pRLC;
	U32 m_FrameCount;
	Game::C_LevelState* m_levelState;
};

void DeferredPickupUpdate(Scheduler::Task* pTask);

#endif // PickUpSubsystem_H