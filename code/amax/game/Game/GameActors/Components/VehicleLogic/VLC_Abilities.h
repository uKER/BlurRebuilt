#ifndef VLC_ABILITIES_H
#define VLC_ABILITIES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <Game/GameActors/Components/GameComponents.h>

#include "misc_shared/Curve/HermiteCurve.h"  //Used for the silent running cloak effects

struct VehicleScoringInfo;
struct MultiplayerConfig;
namespace Game
{
	namespace Actors
	{
		class RaceLogicCore;
	};
};

namespace Components
{

	class BaseMessageHandler;

	class I_VehicleComponent_Abilities_CreateInfo : public I_GameComponentCreator
	{
	public:
		I_VehicleComponent_Abilities_CreateInfo() : m_pLevelState(0), m_pScoringInfo(0), m_pRLC(0), m_pConfig(0), m_pSilentRunningAbility(0), m_pXtAmaxVehicle(0) {}

		static Component::C_ComponentTypeId			ClassType() {return Components::E_VehicleComponent_Abilities;}
		virtual Component::C_ComponentTypeId		GetComponentTypeID() const {return Components::E_VehicleComponent_Abilities;}

		Actor::C_ActorHandle						mParentActorHandle;
		int											mVehicleIndex;
		Game::C_LevelState							*m_pLevelState;
		VehicleScoringInfo							*m_pScoringInfo;
		Game::Actors::RaceLogicCore					*m_pRLC;
		MultiplayerConfig							*m_pConfig;

		Game::Actors::C_VehicleLogicActor			*m_pVehicleLogicActor;
		Game::Actors::C_VehicleActor				*m_pVehicleGraphicalActor;

		const Game::C_MultiplayerAbility			*m_pSilentRunningAbility;
		AmaxVehicle									*m_pXtAmaxVehicle;
	};

	class C_VehicleComponent_Abilities : public I_GameActorComponent
	{
		static const float SILENT_RUNNING_A_REALLY_REALLY_REALLY_REALLY_LONG_TIME;

	public:
		C_VehicleComponent_Abilities();

		static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_VehicleComponent_Abilities;}
		virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_VehicleComponent_Abilities;} // from Component::I_GameComponent


		static Component::C_ComponentInterfaceId	ClassType() {return Components::E_VehicleComponent_Abilities;}
		virtual Component::C_ComponentInterfaceId	GetComponentTypeID() const {return Components::E_VehicleComponent_Abilities;} // from Component::I_GameComponent

		virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);

		virtual void								Update();

		const bool									GetSilentRunningSFXEnabled() const;
		void										SetSilentRunningSFXEnabled(const bool silentRunningSFXEnabled = true);

		void										EmergencyKillSilentRunningSFX();

		void										StartSilentRunningSFX();
		void										FinishSilentRunningSFX();

	//Functions
	protected:
		virtual bool								CreateImpl();
		void										SetHandler(BaseMessageHandler* pHandler) { mMessageHandler = pHandler; }

		void										Initialise();
		void										InitialiseSilentRunningSFX();

		static void									UpdateCloakingAlphaGraphs();
		static void									UpdateAlphaGraph(const SilentRunningModSettings& settings, Curve::C_HermiteCurve& alphaGraph);
		
		void										UpdateSilentRunning(const bool syncAcrossNetwork = true);
		virtual const bool							ShouldBeSilentlyRunning() const;
		virtual void								SetDesiredVehicleAlphas();

#ifndef CFG_GOLD
		void										UpdateDebugFunctionality();
#endif

	//Functions
	private:
		void										UpdateTimers(const float elapsedTime);
		void										UpdateFansNitroGift();

		void										UpdateNetworkForSilentRunning(const bool firing);

		//Handle the various events we care about
		void										HandleImpactMessage(const Game::C_GameplayImpactMsg *pGameplayImpactMsg);
		void										HandleFiredPowerUp();
		void										HandleHitByPowerUp();
		void										HandleRaceStarted();
		void										HandleEnvironmentHit();
		void										HandleVehicleHit();
		void										HandleHandOfGodReset();
		void										HandleDecoyDrop();


	//Variables
	protected:
		static Curve::C_HermiteCurve				sCloakingAlphaGraph;
		static Curve::C_HermiteCurve				sUnCloakingAlphaGraph;

		XtArray<RLPickupSpecialFX>					*m_pSilentRunningSFX; 

		Game::Actors::RaceLogicCore					*m_pRLC;
		MultiplayerConfig							*m_pConfig;

		Game::C_MultiplayerVehicleAbility			*m_pSilentRunningVehicleAbility;		//Don't know why there is a vehicle ability
		const Game::C_MultiplayerAbility			*m_pSilentRunningAbility;				//and a general ability...

		Game::C_LevelState							*m_pLevelState;
		Game::C_LevelStateMultiplayerVehicleAbilities	*m_pMultiplayerAbilities;
		AmaxVehicle									*m_pXtAmaxVehicle;

		Actor::C_ActorHandle						mActorHandle;
		int											mVehicleIndex;

		U32											mSilentRunningSFXId;
		bool										mSilentRunningSFXEnabled;

		Game::Actors::C_VehicleLogicActor			*m_pVehicleLogicActor;
		Game::Actors::C_VehicleActor				*m_pVehicleGraphicalActor;

	//Variables
	private:

		BaseMessageHandler*							mMessageHandler;
		Game::C_MultiplayerVehicleAbility			*m_pFansGiftNitroAbility;

		VehicleScoringInfo							*m_pScoringInfo;


		float										mWaitToCloakTimer;
		bool										mRaceStarted;

		int											mNumFanNitroAwarded;

	};

}

#endif //#ifndef VLC_ABILITIES_H
