#ifndef AI_AMAX_SETUP_H
#define AI_AMAX_SETUP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <AI_AmaxData.h>
#include <actor_system/ActorDeclare.h>
#include <GameLogic/Messaging/GameLogicMessages.h>

namespace Game
{
	namespace Actors
	{
		class C_VehicleActor;
		class C_VehicleLogicActor;
		class C_VehicleFirePickupMsg;
	}
}

namespace AIAmax
{
	class AI_Amax_Setup_Driver
	{
	public:

		AI_Amax_Setup_Driver()
		{	
			Default();
		};

		~AI_Amax_Setup_Driver(){};
		
		void Default()
		{
			m_difficulty = 1.0f;
			m_rubberbanding = 0;
		}

		Util::C_String m_VehicleName;
		Util::C_String m_CharacterName;

		Game::Actors::C_VehicleActor *m_pVehicleActor;
		
		float			m_difficulty;	// this is passed from the event and not part of the AI behaviours
		
		U32				m_AIIndex;
		U32				m_SteeringAI;
		U32				m_rubberbanding;

		//AmaxRubberBandingXt		m_rubberbanding;
	};



	class AIPerkRouteInfo
	{
	public:

		AIPerkRouteInfo() : m_RouteNo(-1), m_Index(-1), m_Dist(0.0f),m_Offset(0.0f)  {};
		~AIPerkRouteInfo(){};
		int		m_RouteNo;
		int		m_Index;
		float	m_Dist;
		float	m_Offset;
	};

	class AIPerk
	{
	public:

		AIPerk(){};
		~AIPerk(){};
		
		// probably all we need in here.......
		Actor::C_ActorHandle			m_actorHandle;
		Btl::Vector<AIPerkRouteInfo>	m_routePerkInfo;
		bool							m_perkActive;
		bool							m_perkEnabled;
		int								m_targetBy;
	};


	class AIFireMessage
	{
	public:

		AIFireMessage(Game::Actors::C_VehicleLogicActor *pLogicActor, Game::Actors::C_VehicleFirePickupMsg *pMessage, bool drop) : m_message(pMessage->mVehicleIndex), m_drop(drop)
		{
			m_pLogicActor = pLogicActor;
			m_message.mVehicleIndex = pMessage->mVehicleIndex;
			m_message.mUniqueID = pMessage->mUniqueID;
			m_message.mPickupOverride = pMessage->mPickupOverride;
			m_message.mForwards = pMessage->mForwards;
			m_message.mBackwards = pMessage->mBackwards;
		};

		~AIFireMessage(){};

		bool								m_drop;
		Game::Actors::C_VehicleLogicActor *m_pLogicActor;
		Game::Actors::C_VehicleFirePickupMsg m_message;
	};
	

	class AIFireMessages
	{
	public:

		AIFireMessages(){};
		~AIFireMessages()
		{
			m_FireMessagesToFire.Clear();
		};

		void Reset()
		{
			m_FireMessagesToFire.Clear();
		}

		void Add(Game::Actors::C_VehicleLogicActor *pLogicActor, Game::Actors::C_VehicleFirePickupMsg *pMessage, bool drop = false)
		{
			AIFireMessage msg(pLogicActor, pMessage, drop);
			
			m_FireMessagesToFire.PushBack(msg);
		}

		Btl::Vector<AIFireMessage>	m_FireMessagesToFire;

	};


} //namespace AIAmax

#endif // AI_AMAX_SETUP_H
