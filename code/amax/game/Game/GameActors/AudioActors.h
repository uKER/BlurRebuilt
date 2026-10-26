#ifndef AUDIO_ACTORS_H
#define AUDIO_ACTORS_H

#ifndef NOT_USING_AUDIO_COMPONENETS

#ifdef _MSC_VER
#pragma once
#endif

//#include "actor_system/ActorSys.h"
#include <Game/ActorDef/ActorTypes.h>
#include <Game/GameDef/UserData.h>

#include "Game/ActorDef/GameActor.h"

#include <shared_actors/XtActorCreation.h>

#include <shared_actors/Components/Creation/MovementComponentsCreate.h>
#include "Game/GameActors/Components/Creation/AudioComponentsCreate.h"

#include <Framework/Layer/Layer.h>
#include <misc_shared/Messaging/MessageHandler.h>
#include <actor_system/ActorSys.h>

#include <Gravity/SDK/CollisionCallback.h>
#include <Gravity/SharedPhysics/ContactStreamIterator.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <GameLogic/Messaging/GameLogicMessages.h>

//#include <gravity/sdk/BodyHelper.h>

#define MAX_LISTENER_ENTITIES 2
#define MAX_LISTENER_VIEWPORTS 4



namespace Game
{
	namespace Actors
	{

		// Audio Actor
		// -----------

			struct C_AudioActorCreate : public ::Actors::C_PosActorCreate
		{
			const Actor::C_ActorTypeId								GetType() const {return Game::E_ActorTypeId_AudioEntity;}

			Components::C_MovementCreateInfo*						m_pSorceryMovementComponentCreate;
			Components::C_AudioEntity_ActorStudioCreateInfo			m_SorceryAudioComponent;

			Component::I_ComponentCreator::C_DataRef				m_pMovementCreator;
			const Components::C_MovementCreateInfo*					GetMovementCreator() const {return static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData());}
			Components::C_MovementCreateInfo*						GetMovementCreator() {return const_cast<Components::C_MovementCreateInfo*>(static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData()));}

			Component::I_ComponentCreator::C_DataRef				m_pAudioCreator;
			const Components::C_AudioEntity_ActorStudioCreateInfo*	GetAudioCreator() const {return static_cast<const Components::C_AudioEntity_ActorStudioCreateInfo*>(m_pAudioCreator.GetData());}
			Components::C_AudioEntity_ActorStudioCreateInfo*		GetAudioCreator() {return const_cast<Components::C_AudioEntity_ActorStudioCreateInfo*>(static_cast<const Components::C_AudioEntity_ActorStudioCreateInfo*>(m_pAudioCreator.GetData()));}

			// from Actor::C_ActorCreationData
			virtual void											DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);

		};

		// Actor
		class C_AudioActor  : public Game::Actors::C_GameActor
		{
		public:
			C_AudioActor(){}

			static const Actor::C_ActorTypeId	TypeId()	{ return Game::E_ActorTypeId_AudioEntity; }
			static const Btl::String32		TypeName()	{ return "AudioEntity"; }

			virtual bool	InitialiseImpl();
			virtual bool	CreateImpl();
			virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage );

			virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
			virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

			virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
			virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

			const Maths::HMat43 & GetMatrix() const {return m_WorldMatrix;}

		private:
			Maths::HMat43	m_WorldMatrix;

			void			Serialise(Sys::C_ContextualStream& stream) const;
			void			Deserialise(Sys::C_ContextualStream& stream);

		};

		////// VO Actor
		////class C_VOAudioActor  : public Game::Actors::C_GameActor
		////{
		////public:
		////	C_VOAudioActor(){}

		////	static const Actor::C_ActorTypeId	TypeId()	{ return Game::E_ActorTypeId_AudioEntity; }
		////	static const Btl::String32		TypeName()	{ return "AudioEntity"; }

		////	virtual bool	InitialiseImpl();
		////	virtual bool	CreateImpl();
		////	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage );

		////	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
		////	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

		////	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
		////	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

		////	const Maths::HMat43 & GetMatrix() const {return m_WorldMatrix;}

		////private:
		////	Maths::HMat43	m_WorldMatrix;

		////	void			Serialise(Sys::C_ContextualStream& stream) const;
		////	void			Deserialise(Sys::C_ContextualStream& stream);

		////};

		// Music Actor
		// -----------

		// This actor controls the music playback in the game

		// Creation Info
		struct C_MusicActorCreate : public ::Actors::C_GameActorCreate
		{
			const Actor::C_ActorTypeId					GetType() const {return Game::E_ActorTypeId_MusicActor;}
		};

		// Actor
		class C_MusicActor  : public Game::Actors::C_GameActor
		{
		public:
			C_MusicActor(){}

			static const Actor::C_ActorTypeId	TypeId()	{ return Game::E_ActorTypeId_MusicActor; }
			static const Btl::String32		TypeName()	{ return "MusicActor"; }

			virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage );

			AudioEntityHandle& GetMusicEntityHnd() { return m_hMusicEntity; }

		protected:
			virtual bool	CreateImpl();	// from Actor::C_Actor

			AudioEntityHandle m_hMusicEntity;

		private:
		};

		// UIAudio Actor
		// -----------

		// This actor controls the UIAudio playback in the game

		// Creation Info
		struct C_UIAudioActorCreate : public ::Actors::C_GameActorCreate
		{
			const Actor::C_ActorTypeId					GetType() const {return Game::E_ActorTypeId_UIAudioActor;}
		};

		// Actor
		class C_UIAudioActor  : public Game::Actors::C_GameActor
		{
		public:
			C_UIAudioActor(){}

			static const Actor::C_ActorTypeId	TypeId()	{ return Game::E_ActorTypeId_UIAudioActor; }
			static const Btl::String32		TypeName()	{ return "UIAudioActor"; }

			virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage );

			AudioEntityHandle& GetUIAudioEntityHnd() { return m_hUIAudioEntity; }
			void		 Create(Messaging::C_MessageSubscriberSystem* pSubscriberSystem);

		protected:
			virtual bool	CreateImpl();	// from Actor::C_Actor
			virtual void Update();

			AudioEntityHandle m_hUIAudioEntity;

			Audio::EntityDataMemberHandle m_Bin1;
			Audio::EntityDataMemberHandle m_Bin2;
			Audio::EntityDataMemberHandle m_Bin3;
			Audio::EntityDataMemberHandle m_Bin4;
			Audio::EntityDataMemberHandle m_Bin5;
			Audio::EntityDataMemberHandle m_Bin6;
			Audio::EntityDataMemberHandle m_Bin7;
			Audio::EntityDataMemberHandle m_Bin8;


		private:
		};

			// ListenerAudio Actor
		// -----------

		// This actor controls the ListenerAudio playback in the game

		// Creation Info
		/*struct C_ListenerAudioActorCreate : public ::Actors::C_GameActorCreate
		{
			static const Actor::C_ActorTypeId					GetType() const {return Game::E_ActorTypeId_ListenerAudioActor;}
		};*/

		class C_ListenerAudioActorCreate : public ::Actors::C_GameActorCreate
		{
		public:

			static Actor::C_ActorTypeId					ClassType() { return Game::E_ActorTypeId_ListenerAudioActor; }
			const Actor::C_ActorTypeId					GetType() const { return Game::E_ActorTypeId_ListenerAudioActor; }

			//C_GameWorld									*m_pGameWorld;
		};

		// Actor
		class C_ListenerAudioActor  : public Game::Actors::C_GameActor
		{
		public:
			C_ListenerAudioActor() : gameworld(NULL) {}
			~C_ListenerAudioActor();

			static const Actor::C_ActorTypeId	TypeId()	{ return Game::E_ActorTypeId_ListenerAudioActor; }
			static const Btl::String32		TypeName()	{ return "ListenerAudioActor"; }

			virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage );

			//AudioEntityHandle& GetListenerAudioEntityHnd() { return m_hListenerAudioEntity; }
			void		 Create(Messaging::C_MessageSubscriberSystem* pSubscriberSystem);

			void CreateTrigger(/*float p_Radius,const Actor::C_ActorHandle& actorHandle,Maths::HMat43	mWorldMatrix*/);
			void DestroyTrigger(/*const Actor::C_ActorHandle& actorHandle*/);
			void Destroy();
			Maths::HMat43 GetMatrix()						{return m_worldMatrix;};
			Maths::HVec3 GetLinearVelocity()						{return m_linearVelocity;};
			void		 PopulateEntities();

			

		protected:
			virtual bool	CreateImpl();	// from Actor::C_Actor
			virtual void Update();
			//virtual void Kill();

			AudioEntityHandle m_hListenerAudioEntity[MAX_LISTENER_ENTITIES];
			U32	m_currentEntityIndex;
			U32 m_currentTunnelEntityIndex;
		


		private:
			Game::C_GameWorld					*gameworld;	
		//	Gravity::C_Trigger*				m_pTriggerObject;
		//	Actor::C_ActorHandle			m_actorHandle;
		//	Gravity::C_TriggerLayer*		pTriggerLayer;
			Gravity::C_TriggerActor*		m_pObjectTriggerActor;
			Gravity::C_TriggerActor*		m_pPickupTriggerActor[MAX_LISTENER_VIEWPORTS];
			Gravity::C_TriggerActor*		m_pOverheadObjectTriggerActor;
			Maths::HMat43					m_worldMatrix;
			Maths::HVec3					m_linearVelocity;
			Game::C_BodyUserDataActor		m_UserData;
			bool							m_DraftingPrev;
			U32								m_numViewports;
			Audio::XtEntityProxy*			m_pXtEntity;
		};

	}
}


#endif //#ifndef NOT_USING_AUDIO_COMPONENETS
#endif
