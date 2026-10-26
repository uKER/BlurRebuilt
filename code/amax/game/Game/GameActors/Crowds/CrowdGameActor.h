#ifndef CROWD_GAME_ACTOR_H
#define CROWD_GAME_ACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <GameLogic\Layers\RaceLogicLayer.h>

namespace AnimatedCrowd
{
	class C_CrowdSystem;
}

class C_GameApp;
struct VehiclePositionInfo;
struct VehicleLapsInfo;

namespace Game
{
	class C_RaceBookLayer;
	class C_RaceLogicLayer;
	class C_AmaxWorldLayer;
	class C_GameWorld;
	class C_GameActorList;
	
	namespace Actors
	{
		class RaceLogicCore;
		class C_VehicleLogicActor;
	}
}

namespace Game
{
	namespace Crowd
	{
		class C_VehicleInfo
		{
		public:
			C_VehicleInfo();

			void		Initialise(const U8 racePosition);
			void		UpdateMegaFonzies(const U8 racePosition, const F32 lapTime);

			const S32	GetMegaFonzies() const;

		private:
			U8			m_RacePosition;			// Self explanatory
			F32			m_PreviousLapTime;		// Self explanatory
			S32			m_PreviousLapOwnage;	// -ve means everyone else perked the crap out of you, +ve means you perked the crap out of everyone else :)

			S32			m_MegaFonzies;			// Coolness (accumulates over race)
		};

		enum VehicleEventType
		{
			E_EVENT_CAR_NEARBY		= 0,
			E_EVENT_OVERTAKE		= 1,
			E_EVENT_CRASH			= 2,
			E_EVENT_CAR_WRONGWAY	= 3,

			E_EVENT_COUNT,
			E_EVENT_UNKNOWN = E_EVENT_COUNT
		};

		class C_VehicleEvent
		{
		public:
			C_VehicleEvent();
			C_VehicleEvent(const VehicleEventType eventType);

			void		Set(const VehicleEventType eventType, const Maths::HVec3& position);
			void		Set(const VehicleEventType eventType, const Maths::HVec3& position, const Maths::HVec3& direction);

			void		Update(const F32 deltaTime);

			const VehicleEventType
						GetEventType() const;
			
			const bool	HasExpired() const;

			const Maths::HVec3&	GetPosition() const;
			const Maths::HVec3&	GetDirection() const;

		private:
			static const F32	GetDuration(const VehicleEventType eventType);
			static const U8		GetPriority(const VehicleEventType eventType);

		private:
			VehicleEventType	m_EventType;
			U8					m_Priority;
			Maths::HVec3		m_Position;
			Maths::HVec3		m_Direction;
			F32					m_Duration;
			F32					m_Lifetime;
		};
	} // namespace Crowd

	class C_CrowdGameManager : public Messaging::C_MessageHandler
	{
	public:
		C_CrowdGameManager();
		virtual ~C_CrowdGameManager();

		static C_CrowdGameManager* Instance()	{ /*BIZ_ASSERT(s_pInstance);*/ return s_pInstance; }

		bool				Create(AnimatedCrowd::C_CrowdSystem* pCrowdSystem, Messaging::C_MessageSubscriberSystem* pSubscriberSystem);

		virtual bool		MessageHandler(const Messaging::C_MessageBase* pMessage);

		void				OnImpact(const S32 vehicleIndex, const Game::C_ImpactMsg* pImpactMessage);
		void				OnOvertake(U32 overtakerIndex, U32 overtakeeIndex, U32 overtakerRacePosition);
		void				OnFanDemand(const bool demandPassed);

	private:
		void				OnUpdate(const F32 deltaTime);

	private:
		C_GameApp							*GetGameApp() const;
		Game::C_RaceBookLayer				*GetRaceBookLayer() const;
		Game::C_RaceLogicLayer				*GetRaceLogicLayer() const;
		Game::C_AmaxWorldLayer				*GetAmaxWorldLayer() const;
		Game::C_GameWorld					*GetGameWorld() const;
		Game::C_GameActorList				*GetGameActorList() const;
		const Actor::C_Actor				*GetActorPtr(const Actor::C_ActorHandle& actorHandle) const;
		Game::Actors::RaceLogicCore			*GetRaceLogicCore() const;
		Game::Actors::C_VehicleLogicActor	*GetVehicleLogicActor(const U8 vehicleIndex) const;
		const VehiclePositionInfo			*GetVehiclePositionInfo(const U8 vehicleIndex) const;
		const VehicleLapsInfo				*GetVehicleLapsInfo(const U8 vehicleIndex) const;
	
	private:
		typedef Btl::Vector<Crowd::C_VehicleInfo, Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES> >	VehicleInfoArray;
		typedef Btl::Vector<Crowd::C_VehicleEvent, Btl::VectorParameters_Array<Game::MAX_RACELOGIC_VEHICLES> >	VehicleEventArray;
		
		AnimatedCrowd::C_CrowdSystem*	m_pCrowdSystem;

		VehicleInfoArray				m_VehicleInfo;
		VehicleEventArray				m_VehicleEvents;

		U32								m_RaceLoaded	: 1;

		static C_CrowdGameManager*		s_pInstance;
	};
} // namespace Game

#endif // CROWD_GAME_ACTOR_H
