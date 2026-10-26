#ifndef MULTIPLAYER_ABILITIES_H
#define MULTIPLAYER_ABILITIES_H

#include "GameLogic/Abilities/MultiplayerAbility.h"
#include <GameLogic/GameData/MultiplayerData.h>

#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>
#include <Network\Player\Data\NetPlayerRaceData.h>

struct RLPickupSpecialFX;

namespace Game
{
	struct MultiplayerLoadOut
	{
		Btl::Vector< UI::E_MultiplayerVehicleAbilities, Btl::VectorParameters_Array< UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX > > slots;

		MultiplayerLoadOut()
		{
			Reset();
		}

		void Reset()
		{
			mLoadOutName = L"";
			for(int slotIndex = 0; slotIndex < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX; slotIndex++)
			{
				slots[slotIndex] = UI::E_MultiplayerVehicleAbilities_None;
			}
		}

		UI::E_MultiplayerVehicleAbilities GetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT eSlotType) const
		{
			return slots[eSlotType];
		}

		void SetAbilityInSlot(UI::E_MULTIPLAYER_LOADOUT_SLOT eSlotType, UI::E_MultiplayerVehicleAbilities ability)
		{
			slots[eSlotType] = ability;
		}

		void Serialise(Sys::BinarySerialiserAutoEndian& stream) const
		{
			Sys::BinarySerialiserAutoEndian out( stream );
			out << mLoadOutName;
			for(int slotIndex = 0; slotIndex < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX; slotIndex++)
			{
				out << (S8)slots[slotIndex];
			}
		}

		void Deserialise(const Sys::BinaryDeserialiserAutoEndian& stream)
		{
			Sys::BinaryDeserialiserAutoEndian in( stream );
			in >> mLoadOutName;
			S8 slotType = 0;
			for(int slotIndex = 0; slotIndex < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX; slotIndex++)
			{
				in >> slotType;
				
				slots[slotIndex] = (UI::E_MultiplayerVehicleAbilities)slotType;
			}
		}

		UI::C_WideString16						mLoadOutName;
	};

	class C_MultiplayerAbilities
	{
	public:
		typedef Btl::Vector< C_MultiplayerAbility,Btl::VectorParameters_Array< UI::E_MultiplayerVehicleAbilities_NumberOfAbilities > > MultiplayerAbilities;

		C_MultiplayerAbilities();
		void Init();
		const C_MultiplayerAbility* GetAbility(const UI::E_MultiplayerVehicleAbilities ability) const;
		void FireAbilityVFX(UI::E_MultiplayerVehicleAbilities ability, Maths::HMat43 matrix, int firingIdx, int targetIdx, Game::E_InGameSpecialFX_Trigger priority,
								float liveTime = -1.f, Maths::HVec3 scale = Maths::HVec3(1,1,1), Maths::HVec3 offset = Maths::HVec3(), bool extraOption = false);
		const MultiplayerAbilities* GetAbilities() const;

		void	FillOutLevelStateCar(const RacerNet::NetPlayerLoadOut& loadOut, Game::LevelStateCar& car);
		void	ActivateBossSlot(UI::E_MultiplayerVehicleAbilities ability, Game::LevelStateCar& car);
		UI::E_MultiplayerVehicleAbilities GetSorceryDefaultAbility(UI::E_MULTIPLAYER_LOADOUT_TYPE type, UI::E_MULTIPLAYER_LOADOUT_SLOT slot, bool ranked) const;
		void	ResetLoadOuts(Game::LevelStateCar& car);
		U32		IronFistActivatedOnPickupType(const E_PICKUP_TYPE pickupType) const;
		bool	TitaniumArmourActivatedOnPickupType(const E_PICKUP_TYPE pickupType) const;

	private:
		MultiplayerAbilities mAbilities;
	};
}

#endif //MULTIPLAYER_ABILITIES_H
