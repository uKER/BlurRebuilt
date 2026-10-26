#ifndef INGAME_SPECIALFX_ENUMS_H
#define INGAME_SPECIALFX_ENUMS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{
	enum E_InGameSpecialFXBase
	{
		E_InGameSpecialFXBase_Vehicle,
		E_InGameSpecialFXBase_WorldPos,
		E_InGameSpecialFXBase_RearWheels,
		E_InGameSpecialFXBase_VehicleToMatrix,
	};

	enum E_InGameSpecialFX_Trigger
	{
		E_InGameSpecialFX_Trigger_Projectile = 0,
		E_InGameSpecialFX_Trigger_Wrecked,
		E_InGameSpecialFX_Trigger_Shield,
		E_InGameSpecialFX_Trigger_HitByPerk,
		E_InGameSpecialFX_Trigger_Targetted,
		E_InGameSpecialFX_Trigger_Firing,
		E_InGameSpecialFX_Trigger_Charging,
		E_InGameSpecialFX_Trigger_Missed,
		E_InGameSpecialFX_Trigger_Repairing,
		E_InGameSpecialFX_Trigger_PickedUp,
	};

	enum E_InGameSpecialFX_Mode
	{
		E_InGameSpecialFX_Mode_Intro,
		E_InGameSpecialFX_Mode_Looping,
		E_InGameSpecialFX_Mode_Outro,
	};
}

#endif // INGAME_SPECIALFX_ENUMS_H