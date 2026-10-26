
#include "stdafx.h"

#include <ui/Game/UILua.h>
#include <ui/game/UIXt.h>

#include <GameLogic/GameModes/GameModeDefines.h>
#include <GameLogic/GameModes/GameModeBase.h>

#include <Network/Misc/NetRaceSettings.h>
#include <Network/Misc/NetRaceSettingsQos.h>
#include <Game/AmaxCam/CameraSystem.h>
#include <Game/AmaxCam/VehicleFeed.h>

namespace Game
{

void C_GameModeBase::Initialise()
{
	SetInternalNameAndID("Nothing",E_GameMode_Nothing);
}

void C_GameModeBase::Update()
{

}

void C_GameModeBase::Start()
{

}

void C_GameModeBase::SetupRace(void *data)
{

}

void C_GameModeBase::StartRace()
{

}

void C_GameModeBase::InterpretRaceResults()
{
	
}

int C_GameModeBase::SendInfoToLua(lua_State *pState)
{
	lua_pushstring( pState, "ID" );
	lua_pushnumber( pState, mID );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, "Name" );
	lua_pushstring( pState, mName );
	lua_rawset( pState, -3 );

	return 1;
}

bool C_GameModeBase::GetNetRaceSettings(RacerNet::NetRaceSettings& out)
{
	return false;
}

bool C_GameModeBase::GetNetRaceSettingsQos(RacerNet::NetRaceSettingsQos& out)
{
	return false;
}

void C_GameModeBase::SetNetRaceSettings(RacerNet::NetRaceSettings& in)
{
}

void C_GameModeBase::EnableChangeViewOnAllViewports()
{
	AmaxCam::C_CameraSystem& camSys=AmaxCam::CameraSystem();
	for(unsigned int i =0; i < camSys.GetViewportCount(); i++)
	{
		camSys.VehicleFeed(i)->SetChangeViewAllowed(true);
	}
}

} // namespace Game
