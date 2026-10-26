#ifndef PLAY_MODE_BASE_H
#define PLAY_MODE_BASE_H

#include "UI/Game/Multiplayer/MultiplayerDefines.h"

namespace Game
{

/*---------------------------------------------------------------------------------------
	Class	: I_PlayModeBase

	Desc	: Interface class for play modes which supplies common functionality. All
			  methods should remain abstract so that we cant instanciate an object.
---------------------------------------------------------------------------------------*/
class I_PlayModeBase
{
public:
	~I_PlayModeBase(){};

	virtual UI::E_RaceType		GetRaceMode()	const = 0;

	virtual U32					GetCity()		const = 0;

	virtual U32					GetRoute()		const = 0;

	virtual S8					GetLapLimit()	const = 0;

	virtual S32					GetTimeLimit()	const = 0;
	
	virtual UI::E_VehicleClass	GetCarClass( const S16 sRaceIndex = -1 ) const = 0;
};

};// namespace Game

#endif // PLAY_MODE_BASE_H
