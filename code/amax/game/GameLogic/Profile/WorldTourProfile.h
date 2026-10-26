
#ifndef GAME_WORLDTOUR_PROFILE_H
#define GAME_WORLDTOUR_PROFILE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "GameLogic/Profile/ProfileBase.h"

namespace Game
{

class C_WorldTourProfile : public C_ProfileBase
{
public:
	void			Init();

	const S32		NumPickupSlots() const;

	const bool		AllowAnyPerkType() const;

	const bool		AllowManualPerkChange() const;

	const bool		AllowShield() const;

	const S32		NumShieldSlots() const;
};

}; // namespace Game

#endif // GAME_WORLDTOUR_PROFILE_H