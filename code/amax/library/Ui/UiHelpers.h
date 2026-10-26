#ifndef UIHELPERS_H
#define UIHELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameData/GameVehicleData.h>

class C_GameApp;
namespace Game
{
	class C_RaceBookLayer;
	class C_RaceLogicLayer;
	class C_AmaxWorldLayer;
	class C_LevelState;
	class C_GameWorld;
	class C_GameActorList;

	namespace Actors
	{
		class RaceLogicCore;
		class C_VehicleLogicActor;
	}
}

namespace Cam2
{
	class C_Camera;
}

namespace RaceBook
{
	class C_RaceBookProfile;
}

namespace UI
{

C_GameApp*								GetGameApp();
Game::C_RaceBookLayer*					GetRaceBookLayer();
Game::C_RaceLogicLayer* 				GetRaceLogicLayer();
Game::C_AmaxWorldLayer* 				GetAmaxWorldLayer();
Game::Actors::RaceLogicCore*			GetRLC();
S8										GetPlayerVehicleIndex( const S32 sLocalPlayerIndex );
S32										GetLocalPlayerIndex( const S32 sVehicleIndex );
Game::Actors::C_VehicleLogicActor*		GetVehicleLogicActor( const S32 sVehicleIndex );
const Cam2::C_Camera&					GetPlayerCamera( const S32 sLocalPlayerIndex );
Game::C_LevelState&						GetLevelState();
RaceBook::C_RaceBookProfile*			GetPlayerProfile( const S32 sLocalPlayerIndex );
Game::C_GameWorld*						GetGameWorld();
Game::C_GameActorList*					GetActorList();

bool									ShowAiNames();

const char*								GetVehicleSheet( const AmaxCarClass carClass );

}; // namespace UI

#endif /*UIHELPERS_H*/
