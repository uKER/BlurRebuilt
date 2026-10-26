
#include "stdafx.h"

#include "GameLogic\Profile\WorldTourProfile.h"

namespace Game
{

void C_WorldTourProfile::Init()
{
}

const S32 C_WorldTourProfile::NumPickupSlots() const
{
	return 3; // TODO : Huck it up to the config 
}

const bool C_WorldTourProfile::AllowAnyPerkType() const
{
	return true; // // TODO : Huck it up to the config 
}

const bool C_WorldTourProfile::AllowManualPerkChange() const
{
	return true; // // TODO : Huck it up to the config 
}

const bool C_WorldTourProfile::AllowShield() const
{
	return true; // TODO : Huck it up to the config 
}

const S32 C_WorldTourProfile::NumShieldSlots() const
{
	return 3; // TODO : Huck it up to the config 
}

}; // namespace Game