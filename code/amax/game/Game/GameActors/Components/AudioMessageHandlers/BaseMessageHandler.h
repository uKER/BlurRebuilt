#ifndef _BASE_MESSAGE_HANDLER_H_
#define _BASE_MESSAGE_HANDLER_H_

#include <GameLogic/Messaging/GameLogicMessages.h>
#include "EntitySuspendResume.h"
#include "WheelSubsystem.h"

#ifndef NOT_USING_AUDIO_COMPONENETS

#ifdef _MSC_VER
#pragma once
#endif


#define MAX_NUMBER_AUDIO_CARS 20
#define MAX_NUMBER_DYNAMIC_OBJECT_COLLISIONS 1

namespace Components
{
	struct BaseMessageHandlerInfo
	{
		const Messaging::C_MessageBase *pMessage;
		Audio::EntityDefinitionHandle hEntityDef;
		const Actor::C_Actor *pActor;
		Messaging::C_MessageHandler* pHandler;
	};

	struct CarDistanceScale
	{

		CarDistanceScale() : distanceFromListener(0xFFFF),shouldUpdate(true){}
		F32 distanceFromListener; 
		bool isFocusCar; 
		int CarID;
		bool shouldUpdate;
		bool isMuted;
		S32 listenerViewPort;
		F32 volScale;
		AudioEntityHandle hEntityHandle;
		AudioEntityHandle hSharedEntityHandle;
		Audio::EntityDataMemberHandle hPriorityMemberHandle[4];		
		Audio::EntityDataMemberHandle hPrioritySharedMemberHandle[4];
		
	
	};

	static CarDistanceScale m_CarDistanceScale[MAX_NUMBER_AUDIO_CARS];
	static CarDistanceScale m_WheelDistanceScale[MAX_NUMBER_AUDIO_CARS];

	static CarDistanceScale m_NewCarDistanceScale[MAX_NUMBER_AUDIO_CARS];
	static CarDistanceScale m_NewWheelDistanceScale[MAX_NUMBER_AUDIO_CARS];

	static CarDistanceScale m_LockedCarDistanceScale[MAX_NUMBER_AUDIO_CARS];

	static AudioEntityHandle m_NewWheelInstanceResumeArray[MAX_NUMBER_AUDIO_CARS];

	static F32 m_prevPriorityVols[MAX_NUMBER_AUDIO_CARS];
	static F32 m_prevSharedPriorityVols[MAX_NUMBER_AUDIO_CARS];

	enum E_SUSPEND_RESUME_STATE
	{
		E_SUSPEND_RESUME_STATE_UNKNOWN,
		E_SUSPEND_RESUME_STATE_FLAGGED_FOR_SUSPEND,
		E_SUSPEND_RESUME_STATE_FLAGGED_FOR_RESUME,
		E_SUSPEND_RESUME_STATE_SUSPENDED,
		E_SUSPEND_RESUME_STATE_RESUMED,
	};

	struct suspendResumeState
	{
		E_SUSPEND_RESUME_STATE suspendState;
		S32 frameDelay;

	};
		 
	static suspendResumeState m_isFlaggedForSuspend[MAX_NUMBER_AUDIO_CARS];

	class BaseMessageHandler
	{
	public:
		BaseMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom ) : m_pCom( pCom ) { }
		virtual ~BaseMessageHandler() { }

		virtual bool ProcessMessage(BaseMessageHandlerInfo& info) = 0;

		I_AudioEntity* m_pCom;
	protected:
		virtual char* GetString( U32 uId ) = 0;
		Audio::EntityDataMemberHandle GetHandle( U32 uId ) { return m_aHandle[ uId ]; }

		Btl::Vector< Audio::EntityDataMemberHandle > m_aHandle;
	};

// [Cian] Audio messages can be requisite or optional.
//		  requesite message must be placed before the final external in the macro,
//		  optionals are placed after
#define AUDIO_WORLD_OBJECT_MESSAGES \
	AUDIO_MESSAGE( FINAL_REQUISITE_EXTERNAL), \
	AUDIO_MESSAGE( MaxDistanceOutput ), \
	AUDIO_MESSAGE( ListenerPosition ), \
	AUDIO_MESSAGE( ListenerDirection ), \
	AUDIO_MESSAGE( ListenerUp ), \
	AUDIO_MESSAGE( ListenerVelocity ), \
	AUDIO_MESSAGE( ObjectPosition ), \
	AUDIO_MESSAGE( ObjectDirection ), \
	AUDIO_MESSAGE( ObjectUp ), \
	AUDIO_MESSAGE( ObjectVelocity ), \
	AUDIO_MESSAGE( ObjectSpeed )
	
	


	class WorldObjectMessageHandler : public BaseMessageHandler
	{
	public:
		WorldObjectMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom ); /*: BaseMessageHandler( hEntityDefinition, pCom ){}*/

	public:
		#undef AUDIO_MESSAGE
		#define AUDIO_MESSAGE( x ) x
		enum MessageIds
		{
			AUDIO_WORLD_OBJECT_MESSAGES,
			NumMessageIds
		};

		char* GetString( U32 uId )
		{
			#undef AUDIO_MESSAGE
			#define AUDIO_MESSAGE( x ) #x
			static char* strMessages[] =
			{
				AUDIO_WORLD_OBJECT_MESSAGES
			}; 
				
			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			BIZ_ASSERT( uId < NumMessageIds );

			return strMessages[ uId ];
		}

		bool ProcessMessage(BaseMessageHandlerInfo& info);
		void AllocateDeclarations(Audio::EntityDefinitionHandle hDef);
		

	private:
		F32 m_distanceThreshold;
		Audio::ReturnDataHandle g_hMaxDistancePODataHandle;
		Audio::EntityDataMemberHandle g_hMaxDistanceHandle;

		Actor::C_ActorHandle						m_EventNotifyHandle;
		bool										m_isSplitScreen;

		Maths::HVec3								m_PrevObjPos;
		Maths::HVec3								m_PrevObjDir;
		Maths::HVec3								m_PrevObjUp;
		//C_SuspendResumeManagerBase*	CreateSuspendResumeManager() const = 0;
	};

// [Cian] Audio messages can be requisite or optional.
//		  requesite message must be placed before the comment in the macro,
//		  optionals are placed after
#define AUDIO_VEHICLE_OBJECT_MESSAGES \
	AUDIO_MESSAGE( Throttle ), \
	AUDIO_MESSAGE( PadThrottle ), \
	AUDIO_MESSAGE( DynamicsRPM ), \
	AUDIO_MESSAGE( Clutch ), \
	AUDIO_MESSAGE( Gear ), \
	AUDIO_MESSAGE( GroundSpeed ), \
	AUDIO_MESSAGE( CarPosition ), \
	AUDIO_MESSAGE( CarDirection ), \
	AUDIO_MESSAGE( CarUp ), \
	AUDIO_MESSAGE( CarVelocity )



#define AUDIO_VEHICLE_OBJECT_MESSAGES_2 \
	AUDIO_MESSAGE( FocusVehicle ), \
	AUDIO_MESSAGE( PriorityVolume ), \
	AUDIO_MESSAGE( CameraPreset ), \
	AUDIO_MESSAGE( PopTrigger ), \
	AUDIO_MESSAGE( BangTrigger ), \
	AUDIO_MESSAGE( LeftPopTrigger ), \
	AUDIO_MESSAGE( LeftBangTrigger ), \
	AUDIO_MESSAGE( RightPopTrigger ), \
	AUDIO_MESSAGE( RightBangTrigger ), \
	AUDIO_MESSAGE( RightReflectionPosition ), \
	AUDIO_MESSAGE( RightReflectionDirection ), \
	AUDIO_MESSAGE( RightReflectionUp ), \
	AUDIO_MESSAGE( LeftReflectionPosition ), \
	AUDIO_MESSAGE( LeftReflectionDirection ), \
	AUDIO_MESSAGE( LeftReflectionUp ), \
	AUDIO_MESSAGE( RightNormalAngle ), \
	AUDIO_MESSAGE( LeftNormalAngle ), \
	AUDIO_MESSAGE( CollidesWith ), \
	AUDIO_MESSAGE( CollisionPosition ), \
	AUDIO_MESSAGE( CollisionActionIn ), \
	AUDIO_MESSAGE( CollisionForce ), \
	AUDIO_MESSAGE( RightReflectionSurface ), \
	AUDIO_MESSAGE( LeftReflectionSurface ), \
	AUDIO_MESSAGE( MinePickup ), \
	AUDIO_MESSAGE( BargePickup ), \
	AUDIO_MESSAGE( MineHit ), \
	AUDIO_MESSAGE( BargeHit ), \
	AUDIO_MESSAGE( MineTier ), \
	AUDIO_MESSAGE( BargeTier ), \
	AUDIO_MESSAGE( MineInitiated), \
	AUDIO_MESSAGE( BargeInitiated )	, \
	AUDIO_MESSAGE( DamagedPartCode ), \
	AUDIO_MESSAGE( DamagedPartSubCode ), \
	AUDIO_MESSAGE( DamagedPartActionIn ), \
	AUDIO_MESSAGE( NitrousTier ), \
	AUDIO_MESSAGE( MineBeep), \
	AUDIO_MESSAGE( MineExplode), \
	AUDIO_MESSAGE( DiscardPerk), \
	AUDIO_MESSAGE( ReactivateVehicle), \
	AUDIO_MESSAGE( RightReflectionLevel), \
	AUDIO_MESSAGE( LeftReflectionLevel), \
	AUDIO_MESSAGE( IsVisible), \
	AUDIO_MESSAGE( AudioRPMLimit), \
	AUDIO_MESSAGE( AudioForcedInduction), \
	AUDIO_MESSAGE( CollisionSurfaceType), \
	AUDIO_MESSAGE( BargeSelected), \
	AUDIO_MESSAGE( MineSelected), \
	AUDIO_MESSAGE( NitrousSelected), \
	AUDIO_MESSAGE( RepairSelected), \
	AUDIO_MESSAGE( ShieldSelected), \
	AUDIO_MESSAGE( ShockSelected), \
	AUDIO_MESSAGE( ShuntSelected),\
	AUDIO_MESSAGE( ObjectFlyByTrigger),\
	AUDIO_MESSAGE( ObjectFlyBySpeed),\
	AUDIO_MESSAGE( ObjectFlyByRelativeSpeed),\
	AUDIO_MESSAGE( ObjectFlyByMass),\
	AUDIO_MESSAGE( ObjectFlyByPosition),\
	AUDIO_MESSAGE( ObjectFlyByDirecton),\
	AUDIO_MESSAGE( CarListenerPosition),\
	AUDIO_MESSAGE( CarListenerDirection),\
	AUDIO_MESSAGE( CarListenerUp),\
	AUDIO_MESSAGE( CarListenerVelocity),\
	AUDIO_MESSAGE( Horn ),\
	AUDIO_MESSAGE( ShockChargeStart ),\
	AUDIO_MESSAGE( ShockChargeStop ),\
	AUDIO_MESSAGE( ShuntChargeStart ),\
	AUDIO_MESSAGE( ShuntChargeStop ),\
	AUDIO_MESSAGE( BargeChargeStart ),\
	AUDIO_MESSAGE( BargeChargeStop ),\
	AUDIO_MESSAGE( OverallDamageLevel ),\
	AUDIO_MESSAGE( TunnelPreset ),\
	AUDIO_MESSAGE( InTunnel ),\
	AUDIO_MESSAGE( NitrousPickup ), \
	AUDIO_MESSAGE( ShockPickup ), \
	AUDIO_MESSAGE( ShuntPickup ), \
	AUDIO_MESSAGE( ShieldPickup ), \
	AUDIO_MESSAGE( RepairPickup ), \
	AUDIO_MESSAGE( Turbo ), \
	AUDIO_MESSAGE( RepairInitiated) , \
	AUDIO_MESSAGE( RepairFinished ), \
	AUDIO_MESSAGE( ParticleScrape ), \
	AUDIO_MESSAGE( HitLimiter ), \
	AUDIO_MESSAGE( ScrapeFriction ), \
	AUDIO_MESSAGE( ExhaustType), \
	AUDIO_MESSAGE( EngineType)	

#define AUDIO_VEHICLE_OBJECT_MESSAGES_PARTS \
	AUDIO_MESSAGE( BonnetClosed ), \
	AUDIO_MESSAGE( BootClosed ), \
	AUDIO_MESSAGE( DoorFrontRightClosed ), \
	AUDIO_MESSAGE( DoorFrontLeftClosed ), \
	AUDIO_MESSAGE( DoorBackRightClosed ), \
	AUDIO_MESSAGE( DoorBackLeftClosed ), \
	AUDIO_MESSAGE( BonnetOpened ), \
	AUDIO_MESSAGE( BootOpened ), \
	AUDIO_MESSAGE( DoorFrontRightOpened ), \
	AUDIO_MESSAGE( DoorFrontLeftOpened ), \
	AUDIO_MESSAGE( DoorBackRightOpened ), \
	AUDIO_MESSAGE( DoorBackLeftOpened ), \
	AUDIO_MESSAGE( BonnetCollision), \
	AUDIO_MESSAGE( BootCollision), \
	AUDIO_MESSAGE( DoorLeftCollision), \
	AUDIO_MESSAGE( DoorRightCollision), \
	AUDIO_MESSAGE( DoorBackLeftCollision), \
	AUDIO_MESSAGE( DoorBackRightCollision), \
	AUDIO_MESSAGE( WingRightCollision), \
	AUDIO_MESSAGE( WingLeftCollision), \
	AUDIO_MESSAGE( MirrorRightCollision), \
	AUDIO_MESSAGE( MirrorLeftCollision), \
	AUDIO_MESSAGE( BumperFrontCollision), \
	AUDIO_MESSAGE( BumperBackCollision), \
	AUDIO_MESSAGE( ShieldFinished), \
	AUDIO_MESSAGE( ShieldInitiated ), \
	AUDIO_MESSAGE( OverallDamageValue ), \
	AUDIO_MESSAGE( PartCollisionForce), \
	AUDIO_MESSAGE( ListenerPosition), \
	AUDIO_MESSAGE( ListenerDirection), \
	AUDIO_MESSAGE( ListenerUp), \
	AUDIO_MESSAGE( ListenerVelocity), \
	AUDIO_MESSAGE( ObjectPosition), \
	AUDIO_MESSAGE( ObjectUp), \
	AUDIO_MESSAGE( ObjectDirection), \
	AUDIO_MESSAGE( ObjectVelocity), \
	AUDIO_MESSAGE( OcclusionValue ), \
	AUDIO_MESSAGE( Wrecked ) , \
	AUDIO_MESSAGE( DurationIntro ) , \
	AUDIO_MESSAGE( DurationLoop ) , \
	AUDIO_MESSAGE( DurationOutro ) , \
	AUDIO_MESSAGE( SilentRunningInitiated ) , \
	AUDIO_MESSAGE( SilentRunningFinished ) , \
	AUDIO_MESSAGE( PriorityVolumeShared )

	

	class VehicleMessageHandler : public WorldObjectMessageHandler
	{
	public:

		enum AUDIO_TUNNEL_TYPE
		{
			AUDIO_TUNNEL_TYPE_REVERB = 0,
			AUDIO_TUNNEL_TYPE_REVERB_WHOOSH_LIGHT,
			AUDIO_TUNNEL_TYPE_REVERB_WHOOSH_HEAVY,
			AUDIO_TUNNEL_TYPE_WHOOSH_LIGHT,
			AUDIO_TUNNEL_TYPE_WHOOSH_HEAVY,
		};

		enum AUDIO_COLLISION_COLLIDES_WITH
		{
		
			AUDIO_COLLISION_COLLIDES_WITH_VEHICLE,
			AUDIO_COLLISION_COLLIDES_WITH_OTHER,
			AUDIO_COLLISION_COLLIDES_WITH_WORLD

		};

		struct CollisionParams
		{
				CollisionParams() : m_bCollisionThisFrame(false),m_bScrapeThisFrame(false),
									m_fMaxImpulseStrength(0.f), m_fDecayingPeakImpulse(0.f),m_Shielded(false),m_ImpulseReductionPercentage(1),m_CollidesWith(AUDIO_COLLISION_COLLIDES_WITH_VEHICLE){}
		
				float m_fMaxImpulseStrength;
				float m_fMaxFrictionStrength;				
				float m_fDecayingPeakImpulse;
				bool m_bCollisionThisFrame;
				bool m_bScrapeThisFrame;
				bool m_bCollidedWithStaticBody;
				bool m_Shielded;
				F32 m_ImpulseReductionPercentage;
				Maths::HVec3	m_Position;
				Maths::HVec3	m_Direction;
				S32 m_MaterialType; 
				U8 m_vehicleIndex;
				float m_fMaxFriction;
				AUDIO_COLLISION_COLLIDES_WITH m_CollidesWith;
				U32 m_CollisionHash;
				float m_fPreviousMaxImpulseStrength;
				//const Game::C_ImpactMsg* p_pImpactMsg;

				
		};

	public:
		VehicleMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, C_AudioEntity_AdditionalSpawned* pCom ); /*: WorldObjectMessageHandler( hEntityDefinition, pCom ) { }*/
		virtual ~VehicleMessageHandler();

	public:
		#undef AUDIO_MESSAGE
		#define AUDIO_MESSAGE( x ) x
		enum MessageIds
		{
			Previous = WorldObjectMessageHandler::NumMessageIds - 1,
			AUDIO_VEHICLE_OBJECT_MESSAGES,
			AUDIO_VEHICLE_OBJECT_MESSAGES_2,
			NumMessageIds
		};

		char* GetString( U32 uId )
		{
			#undef AUDIO_MESSAGE
			#define AUDIO_MESSAGE( x ) #x
			static char* strMessages[] =
			{
				AUDIO_VEHICLE_OBJECT_MESSAGES,
				AUDIO_VEHICLE_OBJECT_MESSAGES_2
			}; 
				
			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumMessageIds - WorldObjectMessageHandler::NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			BIZ_ASSERT( uId < NumMessageIds );

			return strMessages[ uId - WorldObjectMessageHandler::NumMessageIds ];
		}

		//do shared entity enums
		#undef AUDIO_MESSAGE
		#define AUDIO_MESSAGE( x ) x
		enum SharedEntityMessageIds
		{
			PreviousShared = NumMessageIds - 1,
			AUDIO_VEHICLE_OBJECT_MESSAGES_PARTS,
			NumSharedEntityMessageIds
		};

		char* GetSharedEntityString( U32 uId )
		{
			#undef AUDIO_MESSAGE
			#define AUDIO_MESSAGE( x ) #x
			static char* strMessages[] =
			{
				AUDIO_VEHICLE_OBJECT_MESSAGES_PARTS
			}; 
				
			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumSharedEntityMessageIds - NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			BIZ_ASSERT( uId < NumSharedEntityMessageIds && uId >= NumMessageIds  );

			return strMessages[ uId - NumMessageIds ];
		}


		bool ProcessMessage(BaseMessageHandlerInfo& info);

		U32 GetOcclusionValue(Maths::HMat43 raySourceMatrix, Maths::HMat43 rayDestMatrix, F32 distanceToDest, Gravity::C_Scene* scene);

	protected:
		void							VehicleCollision(int mVehicleIdx, const Game::C_ImpactMsg* p_pMsg, const Maths::HMat43 matrix,AUDIO_COLLISION_COLLIDES_WITH collisionOther = AUDIO_COLLISION_COLLIDES_WITH_VEHICLE);
		void							ProcessWheelMessages();

		C_AudioEntity_AdditionalSpawned* m_pCom;

		Collision::C_Ray*				m_pLeftRay;
		Collision::C_Ray*				m_pRightRay;
		Collision::C_Ray*				m_pOcclusionRay;

		CollisionParams					m_collideParams[MAX_NUMBER_AUDIO_CARS];
		CollisionParams					m_scrapeParams[MAX_NUMBER_AUDIO_CARS];
		//CollisionParams					m_shieldCollideParams[MAX_NUMBER_AUDIO_CARS];
		bool m_bHorn;
		bool m_bEngineEnabled;
		bool m_bCollisionsEnabled;
		Audio::PagedArrayHandle			m_hQueryPopTrigger;
		Audio::PagedArrayHandle			m_hQueryBangTrigger;
		Audio::PagedArrayHandle			m_hQueryLeftPopTrigger;
		Audio::PagedArrayHandle			m_hQueryRightPopTrigger;
		Audio::PagedArrayHandle			m_hQueryLeftBangTrigger;
		Audio::PagedArrayHandle			m_hQueryRightBangTrigger;

		Scheduler::PolicyHandle			m_Policy;
		Scheduler::GroupHandle			m_Group;

		F32								m_rightRayDistance;
		F32								m_leftRayDistance;

		F32								m_PrevGear;
		F32								m_PrevClutchValue;
		F32								m_prevCameraPreset;
		S32								m_PrevHealthLevel;
		S32								m_PrevHealth;

		F32								m_rpm;

		S32								m_reverbCounter;

		//bool							m_isFlaggedForSuspend;
		bool							m_bEnableCollisions;
		bool							m_isSplitScreen;
		bool							m_isPendingSuspend;

		static bool						m_isSlowMo;
		//Gravity::C_TriggerActor*		m_pDynamicObjectTriggerActor;
		//Game::C_GameWorld*				m_pGameWorld;
	//	Actor::C_ActorHandle			m_actorHandle;

		F32								m_fPreviousGndSpeed;
		F32								m_AudioRPMLimit;
		bool							m_isShielded;
		bool							m_isSilentRunning;
		U8								m_vehicleIndex;
		F32								m_distanceToOcclusionDestination;
		bool							m_isWrecked;
		bool							m_bayDisabled;
		S32								m_prevFocusCar;
		F32								m_prevThrottle;
		F32								m_prevPadThrottle;
		U32								m_prevRightReflectionSurface;
		U32								m_prevLeftReflectionSurface;
		S32								m_prevLeftReflectionVal;
		S32								m_prevRightReflectionVal;
		S32								m_prevOcclusionValue;
	};

// [Cian] Audio messages can be requisite or optional.
//		  requesite message must be placed before the comment in the macro,
//		  optionals are placed after
#define AUDIO_WHEEL_OBJECT_MESSAGES \
	AUDIO_MESSAGE( FLWheelDownForce ), \
	AUDIO_MESSAGE( FLWheelHitModifier ), \
	AUDIO_MESSAGE( FLWheelOnGround ), \
	AUDIO_MESSAGE( FLWheelSpeed ), \
	AUDIO_MESSAGE( FLWheelSpringForce ), \
	AUDIO_MESSAGE( FLWheelSurfaceType ), \
	AUDIO_MESSAGE( FLWheelXSlide ), \
	AUDIO_MESSAGE( FLWheelZSlide ), \
	AUDIO_MESSAGE( FRWheelDownForce ), \
	AUDIO_MESSAGE( FRWheelHitModifier ), \
	AUDIO_MESSAGE( FRWheelOnGround ), \
	AUDIO_MESSAGE( FRWheelSpeed ), \
	AUDIO_MESSAGE( FRWheelSpringForce ), \
	AUDIO_MESSAGE( FRWheelSurfaceType ), \
	AUDIO_MESSAGE( FRWheelXSlide ), \
	AUDIO_MESSAGE( FRWheelZSlide ), \
	AUDIO_MESSAGE( RLWheelDownForce ), \
	AUDIO_MESSAGE( RLWheelHitModifier ), \
	AUDIO_MESSAGE( RLWheelOnGround ), \
	AUDIO_MESSAGE( RLWheelSpeed ), \
	AUDIO_MESSAGE( RLWheelSpringForce ), \
	AUDIO_MESSAGE( RLWheelSurfaceType ), \
	AUDIO_MESSAGE( RLWheelXSlide ), \
	AUDIO_MESSAGE( RLWheelZSlide ), \
	AUDIO_MESSAGE( RRWheelDownForce ), \
	AUDIO_MESSAGE( RRWheelHitModifier ), \
	AUDIO_MESSAGE( RRWheelOnGround ), \
	AUDIO_MESSAGE( RRWheelSpeed ), \
	AUDIO_MESSAGE( RRWheelSpringForce ), \
	AUDIO_MESSAGE( RRWheelSurfaceType ), \
	AUDIO_MESSAGE( RRWheelXSlide ), \
	AUDIO_MESSAGE( RRWheelZSlide ), \
	AUDIO_MESSAGE( FRWheelPriorityVolume ), \
	AUDIO_MESSAGE( FLWheelPriorityVolume ), \
	AUDIO_MESSAGE( RRWheelPriorityVolume ), \
	AUDIO_MESSAGE( RLWheelPriorityVolume ), \
	AUDIO_MESSAGE( FINAL_REQUISITE_EXTERNAL), \
	AUDIO_MESSAGE( FLWheelAttached ), \
	AUDIO_MESSAGE( FRWheelAttached ), \
	AUDIO_MESSAGE( RLWheelAttached ), \
	AUDIO_MESSAGE( RRWheelAttached ), \
	AUDIO_MESSAGE( FLWheelHitModifierDepth ), \
	AUDIO_MESSAGE( FRWheelHitModifierDepth ), \
	AUDIO_MESSAGE( RLWheelHitModifierDepth ), \
	AUDIO_MESSAGE( RRWheelHitModifierDepth ), \
	AUDIO_MESSAGE( FLWheelPosition ), \
	AUDIO_MESSAGE( FLWheelDirection ), \
	AUDIO_MESSAGE( FLWheelStartOffset ), \
	AUDIO_MESSAGE( FRWheelPosition ), \
	AUDIO_MESSAGE( FRWheelDirection ), \
	AUDIO_MESSAGE( FRWheelStartOffset ), \
	AUDIO_MESSAGE( RLWheelPosition ), \
	AUDIO_MESSAGE( RLWheelDirection ), \
	AUDIO_MESSAGE( RLWheelStartOffset ), \
	AUDIO_MESSAGE( RRWheelPosition ), \
	AUDIO_MESSAGE( RRWheelDirection ), \
	AUDIO_MESSAGE( RRWheelStartOffset ), \
	AUDIO_MESSAGE( WheelListenerPosition),\
	AUDIO_MESSAGE( WheelListenerDirection),\
	AUDIO_MESSAGE( WheelListenerUp),\
	AUDIO_MESSAGE( WheelListenerVelocity), \
	AUDIO_MESSAGE( FLWheelDry_AudioResumed ), \
	AUDIO_MESSAGE( FRWheelDry_AudioResumed ), \
	AUDIO_MESSAGE( RLWheelDry_AudioResumed ), \
	AUDIO_MESSAGE( RRWheelDry_AudioResumed ), \
	AUDIO_MESSAGE( FLWheelOffRoad_AudioResumed ), \
	AUDIO_MESSAGE( FRWheelOffRoad_AudioResumed ), \
	AUDIO_MESSAGE( RLWheelOffRoad_AudioResumed ), \
	AUDIO_MESSAGE( RRWheelOffRoad_AudioResumed ), \
	AUDIO_MESSAGE( FLWheelDry_AudioStopped ), \
	AUDIO_MESSAGE( FRWheelDry_AudioStopped ), \
	AUDIO_MESSAGE( RLWheelDry_AudioStopped ), \
	AUDIO_MESSAGE( RRWheelDry_AudioStopped ), \
	AUDIO_MESSAGE( FLWheelOffRoad_AudioStopped ), \
	AUDIO_MESSAGE( FRWheelOffRoad_AudioStopped ), \
	AUDIO_MESSAGE( RLWheelOffRoad_AudioStopped ), \
	AUDIO_MESSAGE( RRWheelOffRoad_AudioStopped ), \
	AUDIO_MESSAGE( AIWheelOffRoad_AudioStopped ),\
	AUDIO_MESSAGE( AIWheelOffRoad_AudioResumed ),\
	AUDIO_MESSAGE( AIWheelDry_AudioStopped ),\
	AUDIO_MESSAGE( AIWheelDry_AudioResumed ), \
	AUDIO_MESSAGE( FLWheelNitrousInitiated ), \
	AUDIO_MESSAGE( FRWheelNitrousInitiated ), \
	AUDIO_MESSAGE( RLWheelNitrousInitiated ), \
	AUDIO_MESSAGE( RRWheelNitrousInitiated ), \
	AUDIO_MESSAGE( FLPadBrake ), \
	AUDIO_MESSAGE( FRPadBrake ), \
	AUDIO_MESSAGE( RLPadBrake ), \
	AUDIO_MESSAGE( RRPadBrake ), \
	AUDIO_MESSAGE( FLHandBrake ), \
	AUDIO_MESSAGE( FRHandBrake ), \
	AUDIO_MESSAGE( RLHandBrake ), \
	AUDIO_MESSAGE( RRHandBrake ), \
	AUDIO_MESSAGE( FLWheelStaticLoad ), \
	AUDIO_MESSAGE( FRWheelStaticLoad ), \
	AUDIO_MESSAGE( RLWheelStaticLoad ), \
	AUDIO_MESSAGE( RRWheelStaticLoad )







	//************* Wheel Class ***********************
	class WheelMessageHandler : public WorldObjectMessageHandler, public I_AudioEntityDataSource
	{
	public:
		
	//enum E_DataMember
	//{
	//			E_DataMember_Prev = -1,
	//	#define AUDIO_MESSAGE(x) E_DataMember_##x
	//			AUDIO_WHEEL_OBJECT_MESSAGES2
	//	#undef AUDIO_MESSAGE
	//			E_DataMember_Count
	//};

		enum E_Wheel_Surface
		{
			E_Wheel_Surface_Unknown = -1,
			E_Wheel_Surface_OnRoad,
			E_Wheel_Surface_OffRoad,

		};


	public:
		WheelMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom ); /*: WorldObjectMessageHandler( hEntityDefinition, pCom ) { }*/

	public:
		#undef AUDIO_MESSAGE
		#define AUDIO_MESSAGE( x ) x
		enum MessageIds
		{
			Previous = WorldObjectMessageHandler::NumMessageIds - 1,
			AUDIO_WHEEL_OBJECT_MESSAGES,
			NumMessageIds
		};

		char* GetString( U32 uId )
		{
			#undef AUDIO_MESSAGE
			#define AUDIO_MESSAGE( x ) #x
			static char* strMessages[] =
			{
				AUDIO_WHEEL_OBJECT_MESSAGES
			}; 
				
			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumMessageIds - WorldObjectMessageHandler::NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			BIZ_ASSERT( uId < NumMessageIds );

			return strMessages[ uId - WorldObjectMessageHandler::NumMessageIds ];
		}


		bool ProcessMessage(BaseMessageHandlerInfo& info);

		//C_SuspendResumeManagerBase*			GetManager() {return m_pManager;}

		
	protected:

		bool	m_PrevFLAttached;
		bool	m_PrevFRAttached;
		bool	m_PrevRLAttached;
		bool	m_PrevRRAttached;

	private:
		C_SuspendResumeManagerBase*	CreateSuspendResumeManager() const;
		C_SuspendResumeManagerBase*			m_pManager;

		U32									GetNumDataMembers() const;
		const char*							GetDataMemberName(const U32 id) const;
		Audio::EntityDataMemberHandle		GetDataMemberHandle(const U32 id) const;

		bool								TriggerAction(const Audio::EntityDataMemberHandle hEntityMember, const bool stopOnFail = false) const;
		Audio::EntityDefinitionHandle       m_hEntityDefinition;


		void DoActionOuts(AudioEntityHandle hCurrentEntity);
		void ResumeControllerInstances(AudioEntityHandle hCurrentEntity, bool resumeAll = false);
		void SuspendControllerInstances(AudioEntityHandle hCurrentEntity, bool suspendAll = false);

		E_Wheel_Surface RoadSurface(U32 surface);
		void DoWheelSurfacesAndInstanceSuspending(AudioEntityHandle hCurrentEntity, class RacingPhysics::Car *pCar);
		void DoAIWheelSurfacesAndInstanceSuspending(AudioEntityHandle hCurrentEntity, class RacingPhysics::Car *pCar);

		Audio::PagedArrayHandle			m_hQueryFLWheelDry_Resumed;
		Audio::PagedArrayHandle			m_hQueryFRWheelDry_Resumed;
		Audio::PagedArrayHandle			m_hQueryRLWheelDry_Resumed;
		Audio::PagedArrayHandle			m_hQueryRRWheelDry_Resumed;
		Audio::PagedArrayHandle			m_hQueryFLWheelOffRoad_Resumed;
		Audio::PagedArrayHandle			m_hQueryFRWheelOffRoad_Resumed;
		Audio::PagedArrayHandle			m_hQueryRLWheelOffRoad_Resumed;
		Audio::PagedArrayHandle			m_hQueryRRWheelOffRoad_Resumed;

		Audio::PagedArrayHandle			m_hQueryFLWheelDry_Stopped;
		Audio::PagedArrayHandle			m_hQueryFRWheelDry_Stopped;
		Audio::PagedArrayHandle			m_hQueryRLWheelDry_Stopped;
		Audio::PagedArrayHandle			m_hQueryRRWheelDry_Stopped;
		Audio::PagedArrayHandle			m_hQueryFLWheelOffRoad_Stopped;
		Audio::PagedArrayHandle			m_hQueryFRWheelOffRoad_Stopped;
		Audio::PagedArrayHandle			m_hQueryRLWheelOffRoad_Stopped;
		Audio::PagedArrayHandle			m_hQueryRRWheelOffRoad_Stopped;

		Audio::PagedArrayHandle			m_hQueryAIWheelDry_Stopped;
		Audio::PagedArrayHandle			m_hQueryAIWheelDry_Resumed;

		Audio::PagedArrayHandle			m_hQueryAIWheelOffRoad_Stopped;
		Audio::PagedArrayHandle			m_hQueryAIWheelOffRoad_Resumed;


		U32								m_FLSurfaceType;
		U32								m_FRSurfaceType;
		U32								m_RLSurfaceType;
		U32								m_RRSurfaceType;

		U32								m_FLSurfaceType_Prev;
		U32								m_FRSurfaceType_Prev;
		U32								m_RLSurfaceType_Prev;
		U32								m_RRSurfaceType_Prev;

		bool							m_isSuspended;
		bool							m_isSplitScreen;
		bool							m_isPendingSuspend;
		U32								m_occlusionUpdateRate;
		U32								m_currentFrame;
		E_Wheel_Surface					m_prevFLSurface;
		E_Wheel_Surface					m_prevFRSurface;
		E_Wheel_Surface					m_prevRLSurface;
		E_Wheel_Surface					m_prevRRSurface;
		F32								FLIsOnGround;
		F32								FRIsOnGround;
		F32								RLIsOnGround;
		F32								RRIsOnGround;
		F32								m_prevFLIsOnGround;
		F32								m_prevFRIsOnGround;
		F32								m_prevRLIsOnGround;
		F32								m_prevRRIsOnGround;
		F32								m_prevFLWheelHitModifier;
		F32								m_prevFRWheelHitModifier;
		F32								m_prevRLWheelHitModifier;
		F32								m_prevRRWheelHitModifier;
		F32								m_prevBrake;
		F32								m_prevHandBrake;
		bool							m_FLPendingSuspend;
		bool							m_FRPendingSuspend;
		bool							m_RLPendingSuspend;
		bool							m_RRPendingSuspend;
		Maths::HVec3					m_prevFLWheelDirection;
		Maths::HVec3					m_prevFRWheelDirection;
		Maths::HVec3					m_prevRLWheelDirection;
		Maths::HVec3					m_prevRRWheelDirection;

		F32								m_prevFocusCarHeight;

		WheelSubsystem::WheelSubsystemMessages m_FLSuspendMessage;
		WheelSubsystem::WheelSubsystemMessages m_FRSuspendMessage;
		WheelSubsystem::WheelSubsystemMessages m_RLSuspendMessage;
		WheelSubsystem::WheelSubsystemMessages m_RRSuspendMessage;

		F32								m_FLSuspendTime;
		F32								m_FRSuspendTime;
		F32								m_RLSuspendTime;
		F32								m_RRSuspendTime;

		E_Wheel_Surface					m_prevAISurface;
		F32								m_AISuspendTime;
		WheelSubsystem::WheelSubsystemMessages m_AISuspendMessage;
		bool							m_AIPendingSuspend;
		
	};


	//*********************************************************************************************
	//UI message handler
	class UIMessageHandler : public WorldObjectMessageHandler
	{
	public:
		UIMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom ); /*: WorldObjectMessageHandler( hEntityDefinition, pCom ) { }*/

	public:
		enum MessageIds
		{
			Unused = WorldObjectMessageHandler::NumMessageIds,

			NumMessageIds
		};

		char* GetString( U32 uId )
		{
			static char* strMessages[] =
			{
				"Unused",
			};

			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumMessageIds - WorldObjectMessageHandler::NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			BIZ_ASSERT( uId < NumMessageIds );

			return strMessages[ uId - WorldObjectMessageHandler::NumMessageIds ];
		}

		Audio::EntityDataMemberHandle GetHandle( U32 uId )
		{
			return m_aHandle[ WorldObjectMessageHandler::NumMessageIds + uId ];
		}

		bool ProcessMessage(BaseMessageHandlerInfo& info);
	};

		//*********************************************************************************************
	//Music message handler
	class MusicMessageHandler : public WorldObjectMessageHandler
	{
	public:
		MusicMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom ); /*: WorldObjectMessageHandler( hEntityDefinition, pCom ) { }*/

	public:
		enum MessageIds
		{
			eStartActionIn = WorldObjectMessageHandler::NumMessageIds,
		
			NumMessageIds
		};

		char* GetString( U32 Musicd )
		{
			static char* strMessages[] =
			{
				"StartActionIn",
			};

			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumMessageIds - WorldObjectMessageHandler::NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			BIZ_ASSERT( Musicd < NumMessageIds );

			return strMessages[ Musicd - WorldObjectMessageHandler::NumMessageIds ];
		}

		Audio::EntityDataMemberHandle GetHandle( U32 Musicd )
		{
			return m_aHandle[ WorldObjectMessageHandler::NumMessageIds + Musicd ];
		}

		bool ProcessMessage(BaseMessageHandlerInfo& info);
	};

		//*********************************************************************************************
	//VO message handler
	class VOMessageHandler : public WorldObjectMessageHandler
	{
	public:
		VOMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom ); /*: WorldObjectMessageHandler( hEntityDefinition, pCom ) { }*/

	public:
		enum MessageIds
		{
			eStartActionIn = WorldObjectMessageHandler::NumMessageIds,
		
			NumMessageIds
		};

		char* GetString( U32 VOd )
		{
			static char* strMessages[] =
			{
				"StartActionIn",
			};

			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumMessageIds - WorldObjectMessageHandler::NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			BIZ_ASSERT( VOd < NumMessageIds );

			return strMessages[ VOd - WorldObjectMessageHandler::NumMessageIds ];
		}

		Audio::EntityDataMemberHandle GetHandle( U32 VOd )
		{
			return m_aHandle[ WorldObjectMessageHandler::NumMessageIds + VOd ];
		}

		bool ProcessMessage(BaseMessageHandlerInfo& info);

	};

// [Cian] Audio messages can be requisite or optional.
//		  requesite message must be placed before the comment in the macro,
//		  optionals are placed after
#define AUDIO_DYNAMIC_OBJECT_MESSAGES \
	AUDIO_MESSAGE( FINAL_REQUISITE_EXTERNAL), \
	AUDIO_MESSAGE( IsVisible), \
	AUDIO_MESSAGE( EntityAsleep ), \
	AUDIO_MESSAGE( PhysicalObjectHit ), \
	AUDIO_MESSAGE( PhysicalObjectImpactForce ), \
	AUDIO_MESSAGE( StartActionIn ), \
	AUDIO_MESSAGE( CollisionForce ), \
	AUDIO_MESSAGE( EntityFinished ), \
	AUDIO_MESSAGE( CollidesWith ), \
	AUDIO_MESSAGE( PhysicalObjectBreak ), \
	AUDIO_MESSAGE( HitByPerk ), \
	AUDIO_MESSAGE( HitByPerkForce ), \
	AUDIO_MESSAGE( OcclusionValue), \
	AUDIO_MESSAGE( DynamicObjectHit)
	


	class DynamicObjMessageHandler : public WorldObjectMessageHandler
	{
	public:

		enum AUDIO_COLLISION_COLLIDES_WITH
		{
		
			AUDIO_COLLISION_COLLIDES_WITH_VEHICLE,
			AUDIO_COLLISION_COLLIDES_WITH_OTHER,
			AUDIO_COLLISION_COLLIDES_WITH_WORLD

		};

		struct DynamicObjectCollisionParams
		{
				DynamicObjectCollisionParams() : m_bCollisionThisFrame(false),
									m_fMaxImpulseStrength(0.f), m_fDecayingPeakImpulse(0.f),m_Shielded(false),m_ImpulseReductionPercentage(1){}
		
				float m_fMaxImpulseStrength;
				float m_fDecayingPeakImpulse;
				bool m_bCollisionThisFrame;
				bool m_bCollidedWithStaticBody;
				bool m_Shielded;
				F32 m_ImpulseReductionPercentage;
				Maths::HVec3	m_Position;
				Maths::HVec3	m_Direction;
				S32 m_MaterialType; 
				U8 m_vehicleIndex;
				Actor::C_ActorHandle dynamicObjectActorHandle;
				AUDIO_COLLISION_COLLIDES_WITH m_CollidesWith;
				bool	m_isVisible;
				U32 m_occlusionValue ;
				
		};
		DynamicObjMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom ); /*: WorldObjectMessageHandler( hEntityDefinition, pCom ) { }*/
		virtual ~DynamicObjMessageHandler();

	public:
		enum CallbackState
		{
			eCallbackNotSet=0,		// not playing
			eWaitingForCallback,	// playing
			eCallbackReceived,		// was playing but now finished
		};
		#undef AUDIO_MESSAGE
		#define AUDIO_MESSAGE( x ) x
		enum MessageIds
		{
			Previous = WorldObjectMessageHandler::NumMessageIds - 1,
			AUDIO_DYNAMIC_OBJECT_MESSAGES,
			NumMessageIds
		};

		char* GetString( U32 uId )
		{
			#undef AUDIO_MESSAGE
			#define AUDIO_MESSAGE( x ) #x
			static char* strMessages[] =
			{
				AUDIO_DYNAMIC_OBJECT_MESSAGES
			}; 
				
			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumMessageIds - WorldObjectMessageHandler::NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			BIZ_ASSERT( uId < NumMessageIds );

			return strMessages[ uId - WorldObjectMessageHandler::NumMessageIds ];
		}

		Audio::EntityDataMemberHandle GetHandle( U32 DynamicObjd )
		{
			return m_aHandle[ WorldObjectMessageHandler::NumMessageIds + DynamicObjd ];
		}

		bool ProcessMessage(BaseMessageHandlerInfo& info);
	//	void CreateTrigger(float p_Radius,const Actor::C_ActorHandle& actorHandle,Maths::HMat43	mWorldMatrix);
	//	void DestroyTrigger(const Actor::C_ActorHandle& actorHandle);
		CallbackState GetCallbackState() const { return m_eCallbackState; }
		void SetCallbackState(const AudioEntityHandle& hEntity, CallbackState state);

	protected:
		void							DynamicCollision(const Game::C_ImpactMsg* p_pMsg, Actor::C_ActorHandle actorHandle,AUDIO_COLLISION_COLLIDES_WITH collisionOther = AUDIO_COLLISION_COLLIDES_WITH_VEHICLE, U32 occlusionValue = 0);

		DynamicObjectCollisionParams	m_DynamicObjectCollideParams[MAX_NUMBER_DYNAMIC_OBJECT_COLLISIONS];
		bool							m_MarkForDelete;
		U32								m_CollisionHash;
		Audio::PagedArrayHandle			m_hQueryEntityFinished;
		Audio::PagedArrayHandle			m_hQueryEntityAsleep;
		CallbackState					m_eCallbackState;
		bool							m_bEnableCollisions;
		bool							m_isSplitScreen;
		U32							m_OcclusionValue;
		F32								m_TimeTriggered;
	//	Game::C_GameWorld*				m_pGameWorld;
	//	Gravity::C_Trigger*				m_pTriggerObject;
	//	Actor::C_ActorHandle			m_actorHandle;
	
	};

		//*********************************************************************************************
	//PickupPosition message handler

#define AUDIO_PICKUP_POSITION_MESSAGES \
	AUDIO_MESSAGE( FINAL_REQUISITE_EXTERNAL), \
	AUDIO_MESSAGE( StartActionIn), \
	AUDIO_MESSAGE( PerkPickupActionIn  ), \
	AUDIO_MESSAGE( FanDemandPickupActionIn  ), \
	AUDIO_MESSAGE( StartFanDemandActionIn ), \
	AUDIO_MESSAGE( StartStopwatchActionIn ), \
	AUDIO_MESSAGE( StopwatchPickupActionIn  ), \
	AUDIO_MESSAGE( GoldenFanDemandPickupActionIn  ), \
	AUDIO_MESSAGE( StartGoldenFanDemandActionIn ), \
	AUDIO_MESSAGE( StartDecoyActionIn )

	


	class PickupPositionMessageHandler : public WorldObjectMessageHandler
	{
	public:
		PickupPositionMessageHandler( Audio::EntityDefinitionHandle hEntityDefinition, I_AudioEntity* pCom ); /*: WorldObjectMessageHandler( hEntityDefinition, pCom ) { }*/

	public:
		#undef AUDIO_MESSAGE
		#define AUDIO_MESSAGE( x ) x
		enum MessageIds
		{
			Previous = WorldObjectMessageHandler::NumMessageIds - 1,
			AUDIO_PICKUP_POSITION_MESSAGES,
			NumMessageIds
		};

		char* GetString( U32 uId )
		{
			#undef AUDIO_MESSAGE
			#define AUDIO_MESSAGE( x ) #x
			static char* strMessages[] =
			{
				AUDIO_PICKUP_POSITION_MESSAGES
			}; 
				
			StaticAssert( sizeof( strMessages ) / sizeof( char* ) == NumMessageIds - WorldObjectMessageHandler::NumMessageIds, NUM_DECL_STRINGS_DONT_MATCH_NUM_ENUMS );

			BIZ_ASSERT( uId < NumMessageIds );

			return strMessages[ uId - WorldObjectMessageHandler::NumMessageIds ];
		}

		Audio::EntityDataMemberHandle GetHandle( U32 PickupPositionId )
		{
			return m_aHandle[ WorldObjectMessageHandler::NumMessageIds + PickupPositionId ];
		}

		bool ProcessMessage(BaseMessageHandlerInfo& info);

	private:
		S32	m_wakeSleepCount;
		bool m_isAwake;
	};

}

#endif // NOT_USING_AUDIO_COMPONENETS

#endif	//	_BASE_MESSAGE_HANDLER_H_
