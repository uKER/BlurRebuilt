#include "stdafx.h"
#include "UIRacePreview.h"

#include "GameApp.h"
#include "Game\Game.h"
#include "GameLogic\Structures\NetGameVars.h"

#include <Network/Core/NetServices.h>
#include <Network/Core/NetRace.h>
#include <Network/Player/NetPlayer.h>
#include <Network/Player/NetplayerConnection.h>
#include <Network/Player/NetPlayerManager.h>

#include <Ui/Graphics/SimpleTimer.h>
#include <Ui/Game/Gadgets/UIStaticShape.h>

#include <GameLogic\GameModes\GameModeMultiPlayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>

#include "ui/uihelpers.h"

#include "Game/AmaxCam/CameraSystem.h"
#include <camera2/stock/DebugFeed.h>


XT_TYPE( UI::UIRacePreview, "UIRacePreview" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_VehicleSize,	"vehicle_size" )
	XT_FIELD( m_ColourFirst,	"colour_first" )
	XT_FIELD( m_ColourSecond,	"colour_second" )
	XT_FIELD( m_ColourThird,	"colour_third" )
	XT_FIELD( m_ColourOther,	"colour_other" )
XT_END_TYPE

Debug::Bool g_UseDebugVariables( "UI.RacePreview.UseDebug", false );
Debug::Bool g_RacePreviewDrawDebug("UI.RacePreview.DrawDebug", false );
Debug::Float g_RacePreviewRefreshRate("UI.RacePreview.RefreshRate", 0.5f );

namespace UI
{

void UIRacePreview::Init()
{
	C_GadgetBase::Init();

	m_Refresh = false;
	m_Scalar = 0.0f;
	m_OffsetX = 0.0f;
	m_OffsetY = 0.0f;

	m_Timer = g_RacePreviewRefreshRate;
	m_pTrack = NULL;
}

void UIRacePreview::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	AddSubscriber( E_UiMsg_FrameUpdate );

	m_PosColours[E_PosColour_First].SetByName( m_ColourFirst );
	m_PosColours[E_PosColour_Second].SetByName( m_ColourSecond );
	m_PosColours[E_PosColour_Third].SetByName( m_ColourThird );
	m_PosColours[E_PosColour_Other].SetByName( m_ColourOther );

	// Find the current route id
	U32 routeID = 0;
	Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr();
	if ( pGameMode->GetPlayMode() == Game::E_PlayMode_Playlist )
	{
		const MultiplayerRace* pRace = pGameMode->GetPlaylist().GetRaceData();
		if ( pRace )
			routeID = pRace->route->ID;
	}
	else
	if ( pGameMode->GetPlayMode() == Game::E_PlayMode_CustomRace )
	{
		routeID = pGameMode->GetCustomRace().GetRoute();
	}

	AmaxRoute *pRoute  = NULL;
#ifndef CFG_GOLD
	if( g_UseDebugVariables )
		pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)UI::GetLevelState().mRoute];
	else
#endif
		pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)routeID];

	if( pRoute )
	{
		Maths::HVec3 min( pRoute->city->boundsMin.vx, 0.0f, pRoute->city->boundsMin.vy );
		Maths::HVec3 max( pRoute->city->boundsMax.vx, 0.0f, pRoute->city->boundsMax.vy );

		m_TrackBounds = Maths::C_AxisBox( min, max );
	}
}

void UIRacePreview::PostInit()
{
	C_GadgetBase* pGadget = FindChildByName( "track_shape" );
	C_GadgetBase* pStartLine = FindChildByName( "start_line" );

	if ( pGadget && pGadget->GetType() == E_UIButtonType_StaticShape )
	{
		// Need to resize the track based on its bounds
		m_pTrack = (UIStaticShape*)pGadget;

		const Maths::C_AxisBox bounds = m_pTrack->SceneBounds();

		const F32 sizeX = bounds.Max().X() - bounds.Min().X();
		const F32 sizeY = bounds.Max().Y() - bounds.Min().Y();

		// How much smaller is the track than the box
		const F32 size = ( sizeX > sizeY ) ? sizeX : sizeY;

		m_Scalar = 1.0f / size;
		m_OffsetX = ( ( m_Size.x * m_Scalar ) * bounds.Min().X() );
		m_OffsetY = ( ( m_Size.y * m_Scalar ) * bounds.Min().Y() );

		m_pTrack->m_Position.x -= m_OffsetX;
		m_pTrack->m_Position.y -= m_OffsetY * Up();

		if( Up() < 0 )
			m_pTrack->m_Position.y += m_pTrack->m_Size.y;

		m_pTrack->m_Size.x = m_Size.x * m_Scalar;
		m_pTrack->m_Size.y = m_Size.y * m_Scalar;

		// position start line
		if ( pStartLine && pStartLine->GetType() == E_UIButtonType_StaticShape )
		{
			pStartLine->m_Position.x -= m_OffsetX;
			pStartLine->m_Position.y -= m_OffsetY * Up();

			if( Up() < 0 )
				pStartLine->m_Position.y += pStartLine->m_Size.y;

			pStartLine->m_Size.x = m_Size.x * m_Scalar;
			pStartLine->m_Size.y = m_Size.y * m_Scalar;
		}
	}
}

void UIRacePreview::FrameUpdate()
{
	if( !m_Activated )
		return;

	m_Timer -= theFETimer.FrameTime();
	if ( m_Timer <= 0.0f )
	{
		m_Timer = g_RacePreviewRefreshRate;
		m_Refresh = true;
	}
}

void UIRacePreview::Render()
{
	if( !m_Activated )
		return;	

	// auto-transform
	C_UIAutoTransform transform( this );
	C_GadgetBase::Render();

	if ( g_RacePreviewDrawDebug )
	{
		gSimple2D.AddSprite( m_Layer, m_Position, m_Size, 0xff660000, m_Panel );
		gSimple2D.AddJustification( m_Justify );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );
	}

	const F32 width = m_TrackBounds.Max().X() - m_TrackBounds.Min().X();
	const F32 height = m_TrackBounds.Max().Z() - m_TrackBounds.Min().Z();

	const Game::LobbyWaitingRaceInfo& lobbyInfo = C_GameApp::GetInstance()->GetGame()->GetNetGameVars().GetLobbyWaitingRaceInfo();

#ifndef CFG_GOLD
	if( g_UseDebugVariables )
	{
		// Camera
		const Cam2::I_Feed& feed = AmaxCam::CameraSystem().DebugFeed( 0 );
		F32 x = feed.Mat43().AW().X();
		F32 z = feed.Mat43().AW().Z();

		F32 posX = x - m_TrackBounds.Min().X();
		F32 posZ = m_TrackBounds.Max().Z() - z;

		posX /= width;
		posZ /= width;

		posX *= m_Size.x * m_Scalar;
		posZ *= m_Size.y * m_Scalar;

		posX -= m_OffsetX;
		posZ -= m_OffsetY;

		gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x + posX, m_Position.y + (posZ * Up()), m_Position.z ), m_VehicleSize, m_VehicleSize, 0xff00ff00, m_Panel );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );

	}
#endif
	// Render player vehicles
	for ( S32 i = lobbyInfo.m_Vehicles.Size()-1; i >= 0; i-- )
	{
		// Remote vehicle
		const Game::LobbyRaceVehicleInfo& vehicleInfo = lobbyInfo.m_Vehicles[i];
		F32 x = vehicleInfo.m_Xposition;
		F32 z = vehicleInfo.m_Zposition;
		F32 posX = x - m_TrackBounds.Min().X();
		F32 posZ = m_TrackBounds.Max().Z() - z;

		posX /= width;
		posZ /= width;

		posX *= m_Size.x * m_Scalar;
		posZ *= m_Size.y * m_Scalar;

		posX -= m_OffsetX;
		posZ -= m_OffsetY;

		gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x + posX, m_Position.y + (posZ * Up()), m_Position.z ), m_VehicleSize, m_VehicleSize, m_PosColours[ ( i < E_PosColour_Max ) ? i : E_PosColour_Other ], m_Panel );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	}
}

void UIRacePreview::SendToLua( lua_State* pState )
{
	lua_newtable( pState );
	m_Refresh = false; // clear it

	const Game::LobbyWaitingRaceInfo& lobbyInfo = C_GameApp::GetInstance()->GetGame()->GetNetGameVars().GetLobbyWaitingRaceInfo();

	RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();

	// Build a job list of all the current players in the lobby.
	if ( !services.IsInitialised() )
		return;

	RacerNet::C_NetPlayerManager& mgr = services.PlayerManager();

	bool loading = false;

	lua_pushstring( pState, "players_loading" );
	lua_newtable( pState );

	S32 index = 1;
	for ( U8 i = 0; i < mgr.TotalSize(); i++ )
	{
		RacerNet::C_NetPlayer* pPlayer = mgr.Index(i);

		// Dont include anybody not in the race
		if ( pPlayer->m_RaceConnection.m_PlayerState < RacerNet::eNetPlayerStateLoadingRace )
			continue;

		// Is anybody loading or in the 3,2,1 go stage
		if ( pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateLoadingRace || pPlayer->m_RaceConnection.m_PlayerState == RacerNet::eNetPlayerStateWaitingForOthers )
			loading = true;

		lua_pushnumber( pState, pPlayer->m_RaceConnection.m_JoinId );
		lua_rawseti( pState, -2, index++ );
	}
	lua_rawset( pState, -3 );

	// Push the list of players racing. This is only valid after the load
	lua_pushstring( pState, "players_racing" );
	lua_newtable( pState );

	for ( S32 i = 0; i < lobbyInfo.m_Vehicles.Size(); i++ )
	{
		lua_pushnumber( pState, lobbyInfo.m_Vehicles[i].m_JoinId );
		lua_rawseti( pState, -2, i+1 );
	}
	lua_rawset( pState, -3 );

	// Add other data
	Lua::AddDataMember( pState, "loading", loading || ( lobbyInfo.m_Vehicles.Size() == 0 ) );
}

}; // namespace UI
