
#ifndef CollisionSubsystem_H
#define CollisionSubsystem_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AppInterfaceLayer.h"
#include <Game/GameActors/AudioActors.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>

#define MAX_COLLSION_ENTITIES_LOCAL_AI 2
#define MAX_COLLSION_ENTITIES_WORLD_AI 2
#define MAX_COLLSION_ENTITIES_LOCAL_PLAYER 2
#define MAX_COLLSION_ENTITIES_WORLD_PLAYER 2
#define MAX_SCRAPE_ENTITIES_AI 1
#define MAX_SCRAPE_ENTITIES_PLAYER 2

enum E_COLLISION_TYPE
{

	E_COLLISION_TYPE_NONE = -1,
	E_COLLISION_TYPE_LOCAL,
	E_COLLISION_TYPE_WORLD,
	E_COLLISION_TYPE_SCRAPE,
	E_COLLISION_TYPE_LOCAL_AI,
	E_COLLISION_TYPE_WORLD_AI,
	E_COLLISION_TYPE_SCRAPE_AI,

	E_COLLISION_TYPE_MAX
};

	  
class CollisionSubsystem : public Audio::Subsystem/*,  public Messaging::C_MessageHandler*/
{
public:

	enum Message
	{
		E_CollisionSubsystem_Impact,
		E_CollisionSubsystem_Scrape,
		E_CollisionSubsystem_AudioFinished,
	};

	enum EntityDataHandles
	{
		E_ParticleScrapeOffset,

		E_NumEntityDataHandles
	};

	//	AUDIOTODO: is everything in this struct needed?
	class CollisionObject
	{
	public:
		CollisionObject() : m_pOcclusionRay(NULL)
		{
		}

		~CollisionObject()
		{
			delete m_pOcclusionRay;
		}

		void Reset();

		AudioEntityHandle m_hEntity; //assume max for now...
		bool m_markForDelete;
		Audio::PagedArrayHandle m_hQueryEntityFinished;
		Maths::HMat43 m_Matrix;	//	AUDIOTODO could we just use a position vector instead?
		U64 m_UniqueID; // vehicle index combined with the perk id to make a super unique id 
		bool m_Charger;
		bool m_Follower;
		S16 m_vehicleIndex;
		S16 m_targetIndex;
		F32 m_fDistance;
		F32 m_fTimeTriggered;

		E_COLLISION_TYPE m_eType; 
	//	eCollisionListType m_eListType;
		S32 m_translatedType;
		bool m_waitingForRaytraceResult;
		Collision::C_Ray* m_pOcclusionRay;
	};

	class CollisionCollection
	{
	public:
		CollisionCollection() : m_maxCollisionsForCollection(1) {}
		void Reset()
		{
			for ( Btl::Map<U64, CollisionObject*>::Iterator it = m_Map.Begin(); it != m_Map.End(); ++it )
				it->GetValue()->Reset();

			m_nextCollisionIndex = 0;
		}

		CollisionObject* LookupById(U64 id) 
		{ 
			Btl::Map<U64, CollisionObject*>::Iterator it = m_Map.Find(id);
			if (it == m_Map.End())
				return NULL;
			else
				return it->GetValue();
		}
		U32 Size() { return m_Map.Size(); }

		Btl::Map<U64, CollisionObject*> m_Map;

		U32	m_nextCollisionIndex;
		U32 m_maxCollisionsForCollection;

		
	};

	//	AUDIOTODO: needed anymore?
	enum E_CollisionSubsystemMessages
	{
		//E_CollisionSubsystemMessages_CollisionFinished

	};

	CollisionSubsystem();
	~CollisionSubsystem() { }

	bool		ProcessSystemMessage(const U32 u32MessageId);
	bool		ProcessCustomMessage(const U32 u32MessageId, U32 u32Data = 0, void* pData = 0);

	bool		SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)
	{
		return true;
	}
	void		Update();
	void		UpdateCollision(CollisionObject* pObj);
	void		RenderDebug();
	void		PopulateEntityDefinitions();
	bool		MessageHandler(const Messaging::C_MessageBase *pMessage );
	CollisionObject* CreateCollision( U64 uid,U32 eCollisionType, U8 vehicleIndex,bool isFocusVehicle);
	void		Connect(AudioEntityHandle m_hSourceEntity,AudioEntityHandle m_hDestEntity);

	U64 GetUniqueId(U32 vehicleId, U32 CollisionId) 
	{ 
		U64 id = (static_cast<U64>(vehicleId) << 32 | CollisionId);
		return id; 
	}

	CollisionCollection& GetCollisions(U32 uType);
	void	    CreateEntityCollections();
	CollisionObject*	GetNextCollisionObject(E_COLLISION_TYPE collType);

	void		DoOcclusionTest(Maths::HMat43* raySourceMatrix, Maths::HMat43* rayDestMatrix, F32 distanceToDest, Gravity::C_Scene* scene, CollisionObject* obj);
	void		ProcessOcclusionResult(CollisionObject * pObj);

	static void									SetSchedulerGroup(Scheduler::GroupHandle handle);
	static Scheduler::GroupHandle				GetSchedulerGroup() { return m_hSchedulerGroup; }

private:
	AudioEntityHandle hEntity;

	Actor::C_ActorHandle	m_CollisionHandle;
	Audio::XtEntityProxy*	pXtEntity;
	
	CollisionCollection		m_CollisionsLocal;
	CollisionCollection		m_CollisionsWorld;
	CollisionCollection		m_CollisionsScrape;
	CollisionCollection		m_CollisionsLocal_AI;
	CollisionCollection		m_CollisionsWorld_AI;
	CollisionCollection		m_CollisionsScrape_AI;
	
	Audio::XtEntityProxy*	CollisionEntityDefinition[E_COLLISION_TYPE_MAX];
	S32						m_EntityInstanceCount;
	bool					m_removeAllSubscribers;
	bool					m_isSplitScreen;
	bool					m_bRaceInProgress;
	S32						m_MaxCollisionsThisRace;
	U32						m_uniqueCollisionID;

	//	entity handles
	Audio::EntityDataMemberHandle m_haEntityHandles[E_NumEntityDataHandles];

	U32						m_activeCollisions;
	UI::E_MultiplayerVehicleAbilities abilitySlots[UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX];

	Scheduler::PolicyHandle			m_Policy;
	Scheduler::GroupHandle			m_Group;
	static Scheduler::GroupHandle				m_hSchedulerGroup;
};

#endif // CollisionSubsystem_H
