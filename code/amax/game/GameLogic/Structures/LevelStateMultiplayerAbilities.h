#ifndef LEVELSTATE_MULTIPLAYER_ABILITIES_H
#define LEVELSTATE_MULTIPLAYER_ABILITIES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <UI/Game/Multiplayer/MultiplayerDefines.h>

namespace SpecialFX
{
	class I_SpecialFXInstance;
}

// forward decls
namespace Game
{
	namespace Actors
	{
		class C_VehicleLogicActor;
	}

	class C_InGameSpecialFXBase;
}

namespace Game
{
	class C_MultiplayerVehicleAbility
	{
	public:
		C_MultiplayerVehicleAbility();

		void Clear();

		void SetActive( bool val);
		bool GetActive() const;

		void SetValue(float val); 
		float GetValue() const; 

		UI::E_MultiplayerVehicleAbilities GetType() const; 
		void SetType(UI::E_MultiplayerVehicleAbilities type);

	private:
		float mValue;
        bool mActive;
		UI::E_MultiplayerVehicleAbilities mType;
	};

	class C_LevelStateMultiplayerVehicleAbilities
	{
	public:
		C_LevelStateMultiplayerVehicleAbilities();
		void  Clear(); 
		C_MultiplayerVehicleAbility* GetAbility(const UI::E_MultiplayerVehicleAbilities type);

		void				SetAbilityFiring(const UI::E_MultiplayerVehicleAbilities type, bool isActivated, bool notLocal = false, U8 vehicleIndex = -1, U8 victimIndex = -1, bool alternateFX = false, F32 scaleValue = 1.0f, float timeStep = 0.f, Maths::HVec3 rotation = Maths::HVec3(0.f,0.f,0.f));
		bool				GetAbilityFiring(const UI::E_MultiplayerVehicleAbilities type) const;
		void				ResetInstantAbilityFiringBits();

		bool				ActivateAbilityFiringIcon(const UI::E_MultiplayerVehicleAbilities type, bool activate = true);
		bool				IsAbilityIconInstantOff(const UI::E_MultiplayerVehicleAbilities type);
		void				SwitchOnFiringIcon(U8 vehicleIndex, const UI::E_MultiplayerVehicleAbilities type);

		void				SetPickupSlotGiftedInto(const int pickupSlot);
		bool				HasPickupSlotBeenGiftedInto(const int pickupSlot);

		void				SendAbilityFiringMessage(const UI::E_MultiplayerVehicleAbilities type,  const U8 vehicleIndex);
		void				Update(float timeStep, int vehicleIndex);
		bool				IsVehicleScrambled();

		void				Reset();

		const U16			GetActivationTimes(UI::E_MULTIPLAYER_LOADOUT_SLOT slot) const;

		

	private:
		void				SetPassiveAbilityFiringBits();
		void				UpdateModStats(const UI::E_MultiplayerVehicleAbilities type,  Game::Actors::C_VehicleLogicActor* pLogicActor );

		Btl::Vector<C_MultiplayerVehicleAbility,Btl::VectorParameters_Array<UI::E_MultiplayerVehicleAbilities_NumberOfAbilities> > mAbilities;

		U64					mActivationBitset;
		U8					mPickupSlotBitset;

		//Ability-specific values (IF we end up with too many timeouts switch to a timeout for each slot)
		float				mBatteringRamTimeFired;

		SpecialFX::I_SpecialFXInstance*	m_LaserSightSpecialFX;
		float				mLasersightUpdateTime;

		SpecialFX::I_SpecialFXInstance*	m_LastGaspCountdownSpecialFX;
		float				mLastGaspUpdateTime;

		Game::C_InGameSpecialFXBase*	m_ScramblerSpecialFX;
		U8					mScrambleCount;

		//For data gathering store how often each ability activates
		U16					mActivationTimes[UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX];

		F32					mLaserSightVelocity;
		F32					mLaserSightCurrentRotation;
	};

}// end namespace Game

#endif //#ifndef LEVELSTATE_MULTIPLAYER_ABILITIES_H
