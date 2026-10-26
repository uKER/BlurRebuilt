#ifndef GAME_MULTIPLAYER_LOADOUTS_H
#define GAME_MULTIPLAYER_LOADOUTS_H

#include "ui\game\Multiplayer\MultiplayerDefines.h"
#include "GameLogic\Structures\MultiplayerAbilities.h"

namespace Game
{	
	class C_MultiplayerProfile;

	class C_MultiplayerLoadouts
	{
	public:
		typedef Btl::Vector< MultiplayerLoadOut, Btl::VectorParameters_Array<UI::E_MULTIPLAYER_LOADOUT_TYPE_MAX> > LoadOutArray;

		void Reset();

		void Serialise( Sys::BinarySerialiserAutoEndian& stream ) const;
		void Deserialise( const Sys::BinaryDeserialiserAutoEndian& stream );


		const UI::E_MultiplayerVehicleAbilities	GetAbilityInCurrent(const UI::E_MULTIPLAYER_LOADOUT_SLOT eAbilitySlotType) const;
		void SetAbilityInCurrent(const UI::E_MULTIPLAYER_LOADOUT_SLOT eAbilitySlotType, const UI::E_MultiplayerVehicleAbilities ability);
		void ClearAbilityInCurrent(UI::E_MULTIPLAYER_LOADOUT_SLOT eAbilitySlotType);

		const UI::E_MultiplayerVehicleAbilities	GetAbilityInLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType, const UI::E_MULTIPLAYER_LOADOUT_SLOT eAbilitySlotType) const;
		void SetAbilityInLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType, const UI::E_MULTIPLAYER_LOADOUT_SLOT eAbilitySlotType, const UI::E_MultiplayerVehicleAbilities ability);
		void ClearAbilityInLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType, const UI::E_MULTIPLAYER_LOADOUT_SLOT eAbilitySlotType);


		const UI::E_MULTIPLAYER_LOADOUT_TYPE GetCurrentLoadOutType() const;
		void SetCurrentLoadOutType(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType);
		const MultiplayerLoadOut& GetLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType) const;
		void SetLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType, const MultiplayerLoadOut loadOut);
		void RenameLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType, const WCHAR* pName );
		void ResetLoadOut(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType);

		void BuildNetworkLoadOutData(RacerNet::NetPlayerLoadOut& out, bool ranked, Game::C_MultiplayerAbilities* abilities = 0);

		void Init();

		void PostRaceUpdate(const C_MultiplayerProfile& profile);

	private:
		void PopulateAllFromSorcery();
		void PopulateLoadOutFromSorcery(const UI::E_MULTIPLAYER_LOADOUT_TYPE eLoadOutType, const int sorceryLoadOutConfiguration);

		LoadOutArray mLoadOuts;						//Maps slot selection config choice to struct of 7 slots
		UI::E_MULTIPLAYER_LOADOUT_TYPE meCurrentLoadoutType;	//Slot selection choice configuration currently in use
	};
}

#endif // GAME_MULTIPLAYER_LOADOUTS_H
