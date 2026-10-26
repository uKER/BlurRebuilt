#include "stdafx.h"
#include <GameLogic/Layers/RaceBookLayer.h>	// [Blur2PC]
#include "UiHelpers.h"
#include "Game/AmaxCam/CameraSystem.h"
#include "GameLogic\Layers\AmaxWorldLayer.h"
#include "GameLogic\RaceLogicCore\Core\RaceLogicCore.h"
#include "RaceBook\RaceBookManager.h"
#include <camera2/stock/DebugFeed.h>

namespace UI
{

C_GameApp* GetGameApp()
{
	return C_GameApp::GetInstance();
}

Game::C_RaceBookLayer* GetRaceBookLayer()
{
	if ( GetGameApp() )
		return GetGameApp()->GetRaceBookLayer();

	return NULL;
}

Game::C_RaceLogicLayer* GetRaceLogicLayer()
{
	if ( GetRaceBookLayer() )
		return GetRaceBookLayer()->GetRaceLogicLayer();

	return NULL;
}

Game::C_AmaxWorldLayer* GetAmaxWorldLayer()
{
	if ( GetRaceBookLayer() )
		return GetRaceBookLayer()->GetAmaxWorldLayer();

	return NULL;
}

Game::Actors::RaceLogicCore* GetRLC()
{
	if ( GetAmaxWorldLayer() )
		return GetAmaxWorldLayer()->GetRaceLogicCore();

	return NULL;
}

S8 GetPlayerVehicleIndex( const S32 sLocalPlayerIndex )
{
	if ( GetRLC() )
		return GetRLC()->GetViewportFocusVehicleIndex( sLocalPlayerIndex );

	return INVALID_VALUE;
}

S32 GetLocalPlayerIndex( const S32 sVehicleIndex )
{
	if ( GetRLC() )
		return GetRLC()->GetViewportIndex( sVehicleIndex );

	return -1;
}

Game::Actors::C_VehicleLogicActor* GetVehicleLogicActor( const S32 sVehicleIndex )
{
	if ( GetRaceLogicLayer() )
		return GetRaceLogicLayer()->GetLogicVehicleActor( sVehicleIndex );

	return NULL;
}

const Cam2::C_Camera& GetPlayerCamera( const S32 sLocalPlayerIndex )
{
	return AmaxCam::CameraSystem().DebugFeed( sLocalPlayerIndex ).Live();
}

Game::C_LevelState& GetLevelState()
{
	return Game::C_Game::GetInstance()->GetLevelState();
}

RaceBook::C_RaceBookProfile* GetPlayerProfile( const S32 sLocalPlayerIndex )
{
	S32 sVehicleIndex = GetPlayerVehicleIndex( sLocalPlayerIndex );

	if ( sVehicleIndex == INVALID_VALUE )
		return NULL;

	return RaceBook::C_RaceBookManager::Instance().GetProfileByID( (GAMEDATA_ID)GetLevelState().mCars[ sVehicleIndex ].mCharacterID );
}

Game::C_GameWorld* GetGameWorld()
{
	if ( GetAmaxWorldLayer() )
		return GetAmaxWorldLayer()->GetGameWorld();

	return NULL;
}

Game::C_GameActorList* GetActorList()
{
	if ( GetGameWorld() )
		return GetGameWorld()->GetActorList();

	return NULL;
}

bool ShowAiNames()
{
	Game::C_LevelState& levelState = GetLevelState();

	bool bShowNames 
		 = ( levelState.mRaceMode != Game::E_RaceMode_StreetRace  ) && ( levelState.mRaceMode != Game::E_RaceMode_FriendDemand_StreetRace )
		&& ( levelState.mRaceMode != Game::E_RaceMode_Destruction ) && ( levelState.mRaceMode != Game::E_RaceMode_FriendDemand_Destruction )
		&& ( levelState.mRaceMode != Game::E_RaceMode_Checkpoint  ) && ( levelState.mRaceMode != Game::E_RaceMode_FriendDemand_Checkpoint )
		&& ( levelState.mRaceMode != Game::E_RaceMode_FanDemand   ) && ( levelState.mRaceMode != Game::E_RaceMode_FriendDemand_FanDemand );

	return bShowNames;
}

const char*	GetVehicleSheet( const AmaxCarClass carClass )
{
	switch ( carClass )
	{
		case AmaxCarClass_A:	return "car_icons_a";
		case AmaxCarClass_B:	return "car_icons_b";
		case AmaxCarClass_C:	return "car_icons_c";
		case AmaxCarClass_D:	return "car_icons_d";
	}

	return "";
}

}; // namespace UI
