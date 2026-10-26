#ifndef GAMEMODE_DEFINES_H
#define GAMEMODE_DEFINES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

enum E_GameMode
{
	E_GameMode_Nothing,
	
	E_GameMode_Debug,
	E_GameMode_SinglePlayer,
	E_GameMode_MultiPlayer,	// Unused-Base 
	E_GameMode_Network,		// Unused-Base
	E_GameMode_Online,
	E_GameMode_SplitScreen,
	E_GameMode_SystemLink,

	E_GameMode_MAX,
};

enum E_PlayMode
{
	E_PlayMode_Nothing = 0,
	E_PlayMode_CustomRace,
	E_PlayMode_Playlist,
};

} // namespace Game

#endif // GAMEMODE_DEFINES_H
