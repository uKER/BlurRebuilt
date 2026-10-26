#include "stdafx.h"
#include "GameLogic/GameModes/GameModeOnline.h"

namespace Game
{

C_GameModeOnline::C_GameModeOnline()
	: C_GameModeNetwork()
{
	SetInternalNameAndID("Online", E_GameMode_Online);	
}

}; // namespace Game