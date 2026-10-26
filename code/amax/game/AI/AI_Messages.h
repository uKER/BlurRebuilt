#ifndef AI_MESSAGES_H
#define AI_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game/Messaging/GameflowMessages.h>
#include <Game/Messaging/GameMessages.h>
#include <misc_shared/Messaging/GameMessaging.h>
#include <misc_shared/Messaging/MessagingDeclare.h>
#include <misc_shared/Messaging/MessageBase.h>
#include <GameLogic/RaceLogicCore/Data/RLDefinitions.h>
#include <AI_AmaxSetup.h>

namespace Game
{
	namespace Actors
	{
		class C_VehicleActor;
	}
}

namespace AIAmax
{
	class AILoadTrackDataMsg : public Messaging::C_MessageDerived<AILoadTrackDataMsg, Game::E_AIMessage_LoadAITrackResources>
	{
		//Functions
	public:
		AILoadTrackDataMsg(){};
		
		//Variables
	public:
		
		Util::C_String	m_filename;
	};


	class AILoadDriveraMsg : public Messaging::C_MessageDerived<AILoadDriveraMsg, Game::E_AIMessage_LoadAIDriverResources>
	{
		//Functions
	public:
		AILoadDriveraMsg(){};

		//Variables
	public:

		AI_Amax_Setup_Driver	m_driver;
	};


	class AISetRouteMsg : public Messaging::C_MessageDerived<AISetRouteMsg, Game::E_AIMessage_SetRoute>
	{
		//Functions
	public:
		AISetRouteMsg(){};

		//Variables
	public:

		//Btl::String256	m_routeName;
		Util::C_String	m_routeName;
		bool			m_Active;
	};

	class AIStopMsg : public Messaging::C_MessageDerived<AIStopMsg, Game::E_AIMessage_Stop>
	{
		//Functions
	public:
		AIStopMsg(){};

		//Variables
	public:

		Game::Actors::C_VehicleActor	*m_pVehicleActor;

	};
	



	// Temporary Message unitl we know exactly what data is passed...maybe next year as the old Hovis advert says :)
	class AICreateSplineMsg : public Messaging::C_MessageDerived<AICreateSplineMsg, Game::E_AIMessage_CreateIntroSpline>
	{
	public:
		AICreateSplineMsg()		{};
		~AICreateSplineMsg()		{};

		Btl::String32			m_SplineName;
		bool					m_ReSample;

		Btl::Vector<Maths::HVec3>	m_samplePoints;
		Btl::Vector<float>			m_sampleSpeeds;

	};


	class AISetIntroOutroSpeed : public Messaging::C_MessageDerived<AISetIntroOutroSpeed, Game::E_AIMessage_SetIntroOutroSpeed>
	{
	public:
		AISetIntroOutroSpeed()		{};
		~AISetIntroOutroSpeed()		{};

		Game::Actors::C_VehicleActor	*m_pVehicleActor;
		float							m_speed;
		
	};


	class AISetIntroOutroSpline : public Messaging::C_MessageDerived<AISetIntroOutroSpline, Game::E_AIMessage_SetIntroOutroSpline>
	{
	public:
		AISetIntroOutroSpline()		{};
		~AISetIntroOutroSpline()		{};

		Game::Actors::C_VehicleActor	*m_pVehicleActor;
		Btl::String32					m_SplineName;

	};
	
	class AIDisableAI : public Messaging::C_MessageDerived<AIDisableAI, Game::E_AIMessage_DisableAI>
	{
	public:
		AIDisableAI()					{};
		~AIDisableAI()					{};

		Game::Actors::C_VehicleActor	*m_pVehicleActor;
	};

	class AIDriveTowardsPoint : public Messaging::C_MessageDerived<AIDriveTowardsPoint, Game::E_AIMessage_DriveTowardsPoint>
	{
	public:
		AIDriveTowardsPoint()		{};
		~AIDriveTowardsPoint()		{};

		Game::Actors::C_VehicleActor	*m_pVehicleActor;
		Maths::HVec3					m_TargetPoint;
	};




	class AISideSwipeMessage : public Messaging::C_MessageDerived<AISideSwipeMessage, Game::E_AIMessage_AttemptedSideSwipe>
	{
	public:
		AISideSwipeMessage()		{};
		~AISideSwipeMessage()		{};

		int		m_aiDoing;
		int		m_aiAgainst;
		float	m_approxDistFromPlayer;
	};


	class AIAtEndOfIntroOutroSpline : public Messaging::C_MessageDerived<AIAtEndOfIntroOutroSpline, Game::E_AIMessage_AtEndIntroOutroSpline>
	{
	public:
		AIAtEndOfIntroOutroSpline()		{};
		~AIAtEndOfIntroOutroSpline()		{};

		Game::Actors::C_VehicleActor	*m_pVehicleActor;
	};


	class AIPerkLinkMessage : public Messaging::C_MessageDerived<AIPerkLinkMessage, Game::E_AIMessage_CreatePerkLink>
	{
	public:

		AIPerkLinkMessage(){};
		~AIPerkLinkMessage(){};


		RLPickupPosition	m_perk;

	};


	

} //namespace AIAmax

#endif // AI_MESSAGES_H
