#include "stdafx.h"

#include "UIMultiplayerData.h"

#include <ui/Game/UILocaliser.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Game/Profiles/ProfileInterface.h>
#include <ui/Graphics/SimpleTimer.h>
#include <ui/Game/UIDataFinder.h>

#include "UiHelpers.h"
#include "UiMessages.h"

#include "GameLogic/Structures/MpResults.h"
#include "GameLogic/GameData/MultiplayerData.h"
#include "GameLogic/Layers/RaceBookLayer.h"
#include "GameLogic\Layers\RaceLogicLayer.h"

#include <algorithm>
#include <Ui\SoundEffects.h>

#include <Ui\UiXtConfig.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UIMultiplayerData, "UIMultiplayerData" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_FontStylePasses, "styles" )
	XT_FIELD( m_DataFontStylePasses, "data_styles" )
	XT_FIELD( m_TitleFontStylePasses, "title_styles" )
	XT_FIELD( m_fTeamSize, "team_size" )
	XT_FIELD( m_fIconSize, "icon_size" )
	XT_FIELD( m_xtColourFirst, "colour_first" )
	XT_FIELD( m_xtColourOther, "colour_other" )
	XT_FIELD( m_xtColourLast, "colour_last" )
	XT_FIELD( m_xtModeTitleColour, "mode_title_colour" )
	XT_FIELD( m_xtTimerTitleColour, "timer_title_colour" )
	XT_FIELD( m_fGapX, "gap_x" )
	XT_FIELD( m_fTitleSize, "title_size" )
	XT_FIELD( m_fTitleOffset, "title_offset" )
	XT_FIELD( m_bFullscreen, "fullscreen" )
	XT_FIELD( m_bOnlyShowFinishedCars, "only_show_finished_cars" )
	XT_FIELD( m_bShowFullscreenTitle, "show_fullscreen_title" )
	XT_FIELD( m_bShowFullscreenTarget, "show_fullscreen_target" )
XT_END_TYPE

#ifndef CFG_GOLD
Debug::Bool g_UiMultiplayerFakeLeaderboardHUD( "Multiplayer.FakeLeaderboardHUD", false );
#endif

namespace UI
{

// errrrr
static const char* s_AbilityCategory[ E_MultiplayerAbilityCategory_Max ] =
{
	"Slot1", 		// E_MultiplayerAbilityCategory_Slot1
	"Slot2", 		// E_MultiplayerAbilityCategory_Slot2
	"Slot3", 		// E_MultiplayerAbilityCategory_Slot3
};

// --------------
// C_UIMpSortInfo
// --------------
bool C_UIMpSortInfo::operator < ( const C_UIMpSortInfo& o ) const
{
	return ( m_sSort < o.m_sSort );
}

void C_UIMpSortInfo::Clear()
{
	m_sVehicleIndex = -1;
	m_sData = -1;
	m_bActive = false;
}

// -----------------
// UIMultiplayerData
// -----------------
void UIMultiplayerData::Init()
{
	C_GadgetBase::Init();

	// grab multiplayer data
	Game::C_LevelState& levelState = GetLevelState();

	m_eRaceMode = levelState.mRaceMode;
	m_eEliminationEvent = levelState.GetMultiplayerSettings().mEliminationEvent;
	m_eEliminationType = levelState.GetMultiplayerSettings().mEliminationType;

	m_sPositionMe = -1;

	m_fPosLength = 0.0f;
	m_fPointsLength = 0.0f;
	m_fRankLength = 0.0f;
	m_fNameLength = 0.0f;
	m_fTotalLength = 0.0f;

	m_bFullscreen = false;
	m_bFullscreenDoRender = false;
	m_bShowDataColumn = false;
	m_bOnlyShowFinishedCars = false;

	m_bShowFullscreenTitle = false;
	m_bShowFullscreenTarget = false;

	m_sMaxRank = C_MultiplayerData::Instance().m_pConfig->m_LevelingData.count;

	// messages
	if ( levelState.mNetworkRace &&
		(  m_eRaceMode == Game::E_RaceMode_Multiplayer_Racing 
		|| m_eRaceMode == Game::E_RaceMode_Multiplayer_Elimination 
		|| m_eRaceMode == Game::E_RaceMode_Multiplayer_Destruction  
		|| m_eRaceMode == Game::E_RaceMode_Multiplayer_Points ) )
	{
		AddSubscriber( E_UiMsg_FrameUpdate );
		AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
	}
	else
	{
		RemoveSubscriber( E_UiMsg_Render );
	}

	switch( m_eRaceMode )
    {
          case Game::E_RaceMode_Multiplayer_Racing		: m_RaceTypeName.Format( C_TextStringID( "MP_RACE_TYPE_RACING" ).String() );		break;
          case Game::E_RaceMode_Multiplayer_Elimination : m_RaceTypeName.Format( C_TextStringID( "MP_RACE_TYPE_ELIMINATION" ).String() );	break;
          case Game::E_RaceMode_Multiplayer_Points		: m_RaceTypeName.Format( C_TextStringID( "MP_RACE_TYPE_POINTS" ).String() );		break;
          case Game::E_RaceMode_Multiplayer_Destruction : m_RaceTypeName.Format( C_TextStringID( "MP_RACE_TYPE_DESTRUCTION" ).String() );	break;
    }

	m_sLocalPlayerPreviousScore = 0;
	m_pBackground = NULL;
}

void UIMultiplayerData::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// resolve font styles
	m_FontStylePasses.Setup( pState );
	m_DataFontStylePasses.Setup( pState );
	m_TitleFontStylePasses.Setup( pState );

	// resolve colour styles
	m_ColourFirst.SetByName( m_xtColourFirst );
	m_ColourOther.SetByName( m_xtColourOther );
	m_ColourLast.SetByName( m_xtColourLast );
	m_ModeTitleColour.SetByName( m_xtModeTitleColour );
	m_TimerTitleColour.SetByName( m_xtTimerTitleColour );

	gFont.FixedSpaceNext( E_FixedSpaceType_Numbers );
	m_fPosLength = gFont.StringLength( L"20.", m_Size.y, m_FontStylePasses.GetFontStyle( 0 ) );
	gFont.FixedSpaceNext( E_FixedSpaceType_Numbers );

	if ( m_eRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
		m_fPointsLength = gFont.StringLength( L"888", m_Size.y, m_DataFontStylePasses.GetFontStyle( 0 ) );
	else
		m_fPointsLength = gFont.StringLength( L"88", m_Size.y, m_DataFontStylePasses.GetFontStyle( 0 ) );

	gFont.FixedSpaceNext( E_FixedSpaceType_Numbers );
	m_fRankLength = gFont.StringLength( L"88", m_Size.y, m_FontStylePasses.GetFontStyle( 0 ) );
	m_fNameLength = gFont.StringLength( L"WWWWWWWWWWWWWWWW", m_Size.y, m_FontStylePasses.GetFontStyle( 0 ) );

	if ( ( m_eRaceMode == Game::E_RaceMode_Multiplayer_Points ) 
		|| ( m_eRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
		|| ( ( m_eRaceMode == Game::E_RaceMode_Multiplayer_Elimination ) 
			  && ( ( m_eEliminationType == E_EliminationType_LeastFans ) || ( m_eEliminationType == E_EliminationType_MostFans ) ) ) )
	{
		m_bShowDataColumn = true;
	}

	if ( GetRaceBookLayer() && GetRaceBookLayer()->GetGameModePtr() )
		m_bSystemLink = !GetRaceBookLayer()->GetGameModePtr()->IsOnline();

	m_fTotalLength
		= m_fGapX * 0.5f + m_fPosLength														// pos
		+ m_fGapX + m_fIconSize + m_fRankLength												// rank icon
		+ m_fGapX * 0.5f + m_fNameLength													// gamertag
		+ m_fGapX
	;

	if ( m_bShowDataColumn )
		m_fTotalLength += ( m_fGapX + m_fPointsLength );									// points

	if ( !m_bSystemLink )
		m_fTotalLength += ( m_fGapX + ( m_fIconSize * E_MULTIPLAYER_LOADOUT_SLOT_MAX ) );	// loadout

	if ( m_bFullscreen )
	{
		AddSubscriber( E_UiMsg_CtrlButton_Back );
		
		// cache the loadouts
		Game::C_LevelState& levelState = GetLevelState();
		
		for ( S32 i = 0; i < levelState.mCars.Size(); i++ )
		{
			for ( S32 j = 0; j < levelState.mCars[ i ].mLoadOuts.Size(); j++ )
			{
				E_MultiplayerVehicleAbilities eAbility = (E_MultiplayerVehicleAbilities)levelState.mCars[ i ].mLoadOuts[ j ];

				if ( eAbility == E_MultiplayerVehicleAbilities_None )
					continue;

				const MultiplayerAbilityData* pAbility = C_MultiplayerData::Instance().FindAbility( eAbility );

				if ( !pAbility )
					continue;

				m_VehicleLoadouts[ i ].PushBack( AbilityPair( eAbility, pAbility->mAbilityCategory ) );
			}
		}

		// resolve category colours
		for ( S32 i = 0; i < m_AbilityCategoryColours.Size(); i++ )
		{
			m_AbilityCategoryColours[ i ].SetByName( s_AbilityCategory[ i ] );
			static F32 s_fAbilityAlpha = 0.5f;
			m_AbilityCategoryColours[ i ].SetAlpha( (U8)( s_fAbilityAlpha * 255.0f ) );
		}

		m_RenderStateAbility = m_RenderState;
		m_RenderStateAbility.m_EffectIndex = E_Effect_IntensityAsAlpha;
	}
}

void UIMultiplayerData::PostInit()
{
	// grab background object
	ButtonArray childrenMe;
	FindChildren( &childrenMe, true );
	m_pBackground = ArrayFindByName( childrenMe, "Mp_DataFullScreen_Backing" );

	if ( m_pBackground )
	{
		m_pBackground->m_Size.y = m_Size.y;
		m_pBackground->m_Size.x = m_fTotalLength;
		m_pBackground->RemoveSubscriber( E_UiMsg_Render );
	}
}

void UIMultiplayerData::FrameUpdate()
{
	m_Activated = true;	// reset this every frame

	if ( m_sLocalPlayerIndex == -1 )
		return;

	// reset
	m_SortInfo.Clear();
	m_RenderIndices.Clear();

	m_sPositionMe = -1;

	Game::C_RaceLogicLayer* pRLL = GetRaceLogicLayer();

	if ( !pRLL )
		return;

	// grab sort info
	Game::MpVehiclesData& sortedData = Game::C_MpResults::Instance().m_SortedVehicleResults;
	
	for ( S32 i = 0; i < pRLL->GetNumLiveVehicles(); i++ )
	{
		if ( i >= sortedData.Size() )
			break;	// wtf

		if ( m_bOnlyShowFinishedCars && ( sortedData[ i ].m_State != Game::E_VEHICLE_STATE_FINISHED ) )
			continue;

  		m_SortInfo.PushBack( C_UIMpSortInfo() );
		m_RenderIndices.PushBack( 0 );

		C_UIMpSortInfo& info = m_SortInfo.Back();

		info.m_sSort = sortedData[ i ].m_Position;

		if ( m_eRaceMode == Game::E_RaceMode_Multiplayer_Racing )
			info.m_sData = info.m_sSort;

		else if ( m_eRaceMode == Game::E_RaceMode_Multiplayer_Points )
			info.m_sData = sortedData[ i ].m_Statistics[UI::E_MultiplayerStat_Fans];

		else if ( m_eRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
			info.m_sData = sortedData[ i ].m_Statistics[UI::E_MultiplayerStat_OnTrackPoints];

		else if ( m_eRaceMode == Game::E_RaceMode_Multiplayer_Elimination )
		{
			switch ( m_eEliminationType )
			{
				case E_EliminationType_LastPlace:
				case E_EliminationType_FirstPlace:
					info.m_sData = info.m_sSort;
					break;

				case E_EliminationType_LeastHealth:
				case E_EliminationType_MostHealth:
					info.m_sData = info.m_sSort;
					break;

				case E_EliminationType_LeastFans:
				case E_EliminationType_MostFans:
					info.m_sData = sortedData[ i ].m_Statistics[UI::E_MultiplayerStat_EliminationPoints];
					break;

				default:
					info.m_sData = info.m_sSort;
					break;
			}
		}
		else
		{
			break;	// borked
		}

		info.m_bActive = true;
		info.m_sVehicleIndex = sortedData[ i ].m_LocalIndex;
	}

#ifndef CFG_GOLD
	if ( g_UiMultiplayerFakeLeaderboardHUD && ( m_SortInfo.Size() != 0 ) )
	{
		for ( S32 i = m_SortInfo.Size(); i < MAX_VEHICLES; i++ )
		{
			m_SortInfo.PushBack( m_SortInfo[ 0 ] );
			m_RenderIndices.PushBack( 0 );
		}
	}
#endif

	// calculate where you are
	S32 sPlayerVehicleIndex = 0;
	m_sPositionMe = Game::C_MpResults::Instance().GetPlayerPosition( sPlayerVehicleIndex ) - 1;

	if ( ( m_sPositionMe == -1 ) || ( !m_bFullscreen && ( m_sPositionMe >= m_SortInfo.Size() ) ) || ( !m_bFullscreen && m_SortInfo.Size() < 2 ) )
	{
		// someone's been cheating
		m_Activated = false;
		return;
	}

	if ( !m_bFullscreen )
	{
		// determine who to render
		m_RenderIndices[ m_sPositionMe ]++;				// me

		if ( m_sPositionMe == 0 )
		{
			m_RenderIndices[ m_sPositionMe + 1 ]++;		// next
			m_RenderIndices[ m_SortInfo.Size() - 1 ]++;	// last

			if ( m_SortInfo.Size() > 3 )
				m_RenderIndices[ m_sPositionMe + 2 ]++;	// next next
			if ( m_SortInfo.Size() > 4 )
				m_RenderIndices[ m_sPositionMe + 3 ]++;	// next next next
		}
		else if ( m_sPositionMe == 1 )
		{
			m_RenderIndices[ 0 ]++;						// first
			m_RenderIndices[ m_SortInfo.Size() - 1 ]++;	// last

			if ( m_SortInfo.Size() > 3 )
				m_RenderIndices[ m_sPositionMe + 1 ]++;	// next
			if ( m_SortInfo.Size() > 4 )
				m_RenderIndices[ m_sPositionMe + 2 ]++;	// next next
		}
		else if ( m_sPositionMe == ( m_SortInfo.Size() - 2 ) )
		{
			m_RenderIndices[ 0 ]++;						// first
			m_RenderIndices[ m_SortInfo.Size() - 1 ]++;	// last

			if ( m_SortInfo.Size() > 3 )
				m_RenderIndices[ m_sPositionMe - 1 ]++;	// previous
			if ( m_SortInfo.Size() > 4 )
				m_RenderIndices[ m_sPositionMe - 2 ]++;	// previous previous
		}
		else if ( m_sPositionMe == ( m_SortInfo.Size() - 1 ) )
		{
			m_RenderIndices[ 0 ]++;						// first
			m_RenderIndices[ m_sPositionMe - 1 ]++;		// previous

			if ( m_SortInfo.Size() > 3 )
				m_RenderIndices[ m_sPositionMe - 2 ]++;	// previous previous
			if ( m_SortInfo.Size() > 4 )
				m_RenderIndices[ m_sPositionMe - 3 ]++;	// previous previous previous
		}
		else
		{
			m_RenderIndices[ 0 ]++;						// first
			m_RenderIndices[ m_SortInfo.Size() - 1 ]++;	// last
			m_RenderIndices[ m_sPositionMe - 1 ]++;		// previous
			m_RenderIndices[ m_sPositionMe + 1 ]++; 	// next
		}
	}
}

bool UIMultiplayerData::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
			if ( m_sLocalPlayerIndex == -1 )
				m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
			break;

		case E_UiMsg_CtrlButton_Back:
		{
			const C_UiMessagePadData* pPadMessage = (C_UiMessagePadData*)pMessage;
			if ( pPadMessage->deb )
			{
				m_bFullscreenDoRender = !m_bFullscreenDoRender;
				C_UiTimelineMessage msg( "mp_fullscreen_leaderboard_on", m_bFullscreenDoRender );
				gButtons.BroadcastMessageNow( 0xffffffff, &msg );
			}
			break;
		}

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIMultiplayerData::Render()
{
	if ( !m_Activated || ( m_sLocalPlayerIndex == -1 ) )
		return;

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	if ( m_AlphaInherit < 0.001f )
		return;

	if ( m_bFullscreen )
		RenderFullscreen();
	else
		RenderMini();
}

void UIMultiplayerData::RenderFullscreen()
{
	// full screen fade
	static F32 s_fFullScreenAlpha = 0.75f;
	C_UISpriteColours backgroundCol = 0xff000000;
	backgroundCol.SetAlpha( (U8)( s_fFullScreenAlpha * 255.0f ) );
	backgroundCol.Set( backgroundCol, m_AlphaInherit, m_IntensityInherit );

	gSimple2D.AddSprite( m_Layer, Maths::HVec3( 320.0f, 240.0f, m_Position.z ), 856.0f, 482.0f, backgroundCol, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 3 ) );

	Game::C_LevelState& levelState = GetLevelState();

	F32 fStartX = m_Position.x - m_fTotalLength / 2.0f;
	F32 y = m_Position.y;

	// sort out colours
	C_UISpriteColours timerTitleColour;
	timerTitleColour.Set( m_TimerTitleColour, m_AlphaInherit, m_IntensityInherit );
	C_UISpriteColours modeTitleColour;
	modeTitleColour.Set( m_ModeTitleColour, m_AlphaInherit, m_IntensityInherit );
	C_UISpriteColours colourFirst;
	colourFirst.Set( m_ColourFirst, m_AlphaInherit, m_IntensityInherit );
	C_UISpriteColours colourOther;
	colourOther.Set( m_ColourOther, m_AlphaInherit, m_IntensityInherit );

	for ( S32 i = 0; i < m_AbilityCategoryColours.Size(); i++ )
		m_AbilityCategoryColoursCurrent[ i ].Set( m_AbilityCategoryColours[ i ], m_AlphaInherit, m_IntensityInherit );


	// titles
	F32 fTitleY = m_Position.y + m_fTitleOffset;

	if ( m_bShowFullscreenTarget )
	{
		if ( ( m_eRaceMode == Game::E_RaceMode_Multiplayer_Destruction ) && ( levelState.GetMultiplayerSettings().mScoreLimit > 0 ) )
		{
			static C_TextStringID targetTitle( "MP_HUD_DESTRUCTION_TARGET" );
			static C_WideString128 tempString;
			static F32 s_fTargetMult = 0.85f;

			tempString.Format( targetTitle.String(), levelState.GetMultiplayerSettings().mScoreLimit );

			gFont.DrawText( m_Layer, Maths::HVec3( m_Position.x, fTitleY, m_Position.z ), E_Justify_TopCentre, 
				tempString.Chars(), timerTitleColour, m_fTitleSize * s_fTargetMult, m_TitleFontStylePasses, 0.0f, m_Panel );

			fTitleY -= ( m_fTitleSize * s_fTargetMult * Up() );
		}
	}

	if ( m_bShowFullscreenTitle )
	{
		gFont.DrawText( m_Layer, Maths::HVec3( m_Position.x, fTitleY, m_Position.z ), E_Justify_TopCentre, 
			m_RaceTypeName, modeTitleColour, m_fTitleSize, m_DataFontStylePasses, 0.0f, m_Panel );
	}

	C_TextureParam texRank = GetTextureParam( 0 );
	C_TextureParam texAbility = GetTextureParam( 1 );
	C_TextureParam texTeam = GetTextureParam( 2 );

	S32 sColumnCount = 0;
	C_WideString4 posString;

	for ( S32 i = 0; i < m_SortInfo.Size(); i++ )
	{
		if ( levelState.mTeams )
		{
			colourOther.Set( levelState.mCars[ m_SortInfo[ i ].m_sVehicleIndex ].mTeam ? C_UiXtConfig::Instance()->m_TeamBColour : C_UiXtConfig::Instance()->m_TeamAColour, m_AlphaInherit, m_IntensityInherit );
		}

		C_UISpriteColours& col = ( i == m_sPositionMe ) ? colourFirst : colourOther;
		
		F32 x = fStartX;
		x += m_fGapX;
		x += m_fPosLength;

		// background
		if ( ( ( i & 1 ) == 0 ) && m_pBackground )
		{
			m_pBackground->m_Position.x = m_Position.x;
			m_pBackground->m_Position.y = y;
			m_pBackground->DirtySet( E_Dirty_Matrix, true );
			m_pBackground->Render();
		}

		// pos
		posString.Format( L"%d.", i + 1 );
		gFont.FixedSpaceNext( E_FixedSpaceType_Numbers );
		gFont.DrawText( m_Layer, Maths::HVec3( x, y, m_Position.z ), E_Justify_MiddleRight, 
			posString.Chars(), col, m_Size.y, m_FontStylePasses, 0.0f, m_Panel );

		x += m_fGapX * 0.5f;

		// Add team icon
		if ( levelState.mTeams )
		{
			const U8 team = levelState.mCars[ m_SortInfo[ i ].m_sVehicleIndex ].mTeam;
			texTeam.m_Position.u = texTeam.m_Size.u * team;

			// Store effect then swap it back after render
			const U32 effect = m_RenderState.m_EffectIndex;
			m_RenderState.m_EffectIndex = E_Effect_IntensityAsAlpha;

			gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), m_fTeamSize, m_fTeamSize, col, m_Panel, E_SpriteType_Sprite, m_RenderState );
			gSimple2D.AddJustification( E_Justify_MiddleLeft );
			gSimple2D.AddTexUVs( 0, texTeam );

			m_RenderState.m_EffectIndex = effect;

			x += m_fTeamSize;
			x += m_fGapX * 0.5f;
		}

		S32 sRank = levelState.mCars[ m_SortInfo[ i ].m_sVehicleIndex ].mMultiplayerRank;
		S32 sLegend = levelState.mCars[ m_SortInfo[ i ].m_sVehicleIndex ].mMultiplayerLegendLevel;

		// rank icon
		if ( sLegend > 0 )
			texRank.m_Position.u = ( m_sMaxRank * texRank.m_Size.u ) + ( ( sLegend - 1 ) * texRank.m_Size.u );
		else
			texRank.m_Position.u = ( sRank * texRank.m_Size.u );

		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), m_fIconSize, m_fIconSize, m_SpriteColours, 
			m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddJustification( E_Justify_MiddleLeft );
		gSimple2D.AddTexUVs( 0, texRank );
		
		x += m_fIconSize;

		// rank num
		gFont.FixedSpaceNext( E_FixedSpaceType_Numbers );
		gFont.DrawText( m_Layer, Maths::HVec3( x, y, m_Position.z ), E_Justify_MiddleLeft, 
			C_UILocaliser::Instance().ConvertNumberToCommas( sRank + 1 ), 
			m_SpriteColours, m_Size.y, m_FontStylePasses, 0.0f, m_Panel );

		x += m_fRankLength;
		x += m_fGapX * 0.5f;

		// name
		gFont.DrawText( m_Layer, Maths::HVec3( x, y, m_Position.z ), E_Justify_MiddleLeft, 
			levelState.mCars[ m_SortInfo[ i ].m_sVehicleIndex ].mNameAboveCar.Chars(), 
			col, m_Size.y, m_FontStylePasses, 0.0f, m_Panel );

		x += m_fNameLength;
		x += m_fGapX;

		if ( !m_bSystemLink )
		{
			// loadout icons
			for ( S32 j = 0; j < m_VehicleLoadouts[ m_SortInfo[ i ].m_sVehicleIndex ].Size(); j++ )
			{
				AbilityPair& abilityPair = m_VehicleLoadouts[ m_SortInfo[ i ].m_sVehicleIndex ][ j ];
				texAbility.m_Position.u = ( abilityPair.first * texAbility.m_Size.u );

				gSimple2D.AddSprite( m_Layer, Maths::HVec3( x + ( m_fIconSize * j ), y, m_Position.z ), m_fIconSize, m_fIconSize, 
					m_AbilityCategoryColoursCurrent[ abilityPair.second ], m_Panel, E_SpriteType_Sprite, m_RenderStateAbility );
				gSimple2D.AddJustification( E_Justify_MiddleLeft );
				gSimple2D.AddTexUVs( 0, texAbility );
			}

			x += ( m_fIconSize * E_MULTIPLAYER_LOADOUT_SLOT_MAX );
			x += m_fGapX;
		}

		if ( m_bShowDataColumn )
		{
			x += m_fPointsLength;

			gFont.DrawText( m_Layer, Maths::HVec3( x, y, m_Position.z ), E_Justify_MiddleRight, 
				C_UILocaliser::Instance().ConvertNumberToCommas( m_SortInfo[ i ].m_sData ), 
				m_SpriteColours, m_Size.y, m_DataFontStylePasses, 0.0f, m_Panel );
			
			x += m_fGapX;
		}

		y += ( m_Size.y * Up() );
	}

	// this is only valid for one frame
	//m_bFullscreenDoRender = false;
}

void UIMultiplayerData::RenderMini()
{
	if ( m_eRaceMode == Game::E_RaceMode_Multiplayer_Destruction )
		return;

	// render the people we want to show
	Game::C_LevelState& levelState = GetLevelState();
	
	C_UISpriteColours col;
	C_UISpriteColours dataCol;

	dataCol = m_ColourLast;
	dataCol.Set( dataCol, m_AlphaInherit, m_IntensityInherit );

	F32 y = m_Position.y;

	for ( S32 i = 0; i < m_SortInfo.Size(); i++ )
	{
		if ( !m_RenderIndices[ i ] )
			continue;

		F32 x = m_Position.x;

		/*
		if ( i == 0 )
		{
			col = m_ColourFirst;
			col.Set( col, m_AlphaInherit, m_IntensityInherit );
		}
		else if ( i == ( m_SortInfo.Size() - 1 ) )
		{
			col = m_ColourLast;
			col.Set( col, m_AlphaInherit, m_IntensityInherit );
		}
		else if ( i == m_sPositionMe )
			col = m_SpriteColours;
		else
		{
			col = m_ColourOther;
			col.Set( col, m_AlphaInherit, m_IntensityInherit );
		}

		bool meFirst = ( ( i == 0 ) && ( i == m_sPositionMe ) );
		*/
		

		if ( i == m_sPositionMe )
		{
			col = m_ColourFirst;
			col.Set( col, m_AlphaInherit, m_IntensityInherit );
		}
		else
		{
			col = m_ColourOther;

			if ( levelState.mTeams )
			{
				const U8 team = levelState.mCars[ m_SortInfo[ i ].m_sVehicleIndex ].mTeam;
				col = team ? C_UiXtConfig::Instance()->m_TeamBColour : C_UiXtConfig::Instance()->m_TeamAColour;
			}
			col.Set( col, m_AlphaInherit, m_IntensityInherit );
		}

		// icon
		/*
		C_TextureParam& t = ( ( i == 0 ) 
			? GetTextureParam( 0 ) 
			: ( ( i == m_sPositionMe ) 
				? GetTextureParam( 1 ) 
				: GetTextureParam( 2 ) ) );
		*/

		F32 rot = 0.0f;
		S32 texIndex = 0;
		if ( i == 0 ) // first
		{
			texIndex = 0;
		}
		else
		if ( i == m_SortInfo.Size() - 1  ) // last
		{
			texIndex = 2;
		}
		else
		if ( i == m_sPositionMe ) // me and im not first
		{
			texIndex = 1;
			rot = -90.0f;
		}
		else // all the other muppets
		{
			texIndex = 3;
			rot = ( i > m_sPositionMe ) ? 180.0f : 0.0f;
		}

		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x + ( m_fIconSize / 2.0f ), y, m_Position.z ), m_fIconSize, m_fIconSize, col, 
			m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddTexUVs( 0, GetTextureParam( texIndex ) );

		if ( rot != 0.0f )
			gSimple2D.AddRotation( 0.0f, 0.0f, rot );

		/*
		if ( meFirst )	// arrow too
		{
			gSimple2D.AddSprite( m_Layer, Maths::HVec3( x - m_fGapX - ( m_fIconSize / 4.0f ), y, m_Position.z ), m_fIconSize, m_fIconSize, col, 
				m_Panel, E_SpriteType_Sprite, m_RenderState );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );
			gSimple2D.AddTexUVs( 0, GetTextureParam( 2 ) );
			gSimple2D.AddRotation( 0.0f, 0.0f, 90.0f );
		}
		*/

		x += m_fIconSize;
		x += m_fGapX;

		// Add team icon
		if ( levelState.mTeams )
		{
			C_TextureParam& tex = GetTextureParam( 4 );
			tex.m_Position.u = tex.m_Size.u * levelState.mCars[ m_SortInfo[ i ].m_sVehicleIndex ].mTeam;

			// Store effect then swap it back after render
			const U32 effect = m_RenderState.m_EffectIndex;
			m_RenderState.m_EffectIndex = E_Effect_IntensityAsAlpha;

			gSimple2D.AddSprite( m_Layer, Maths::HVec3( x + ( m_fTeamSize / 2.0f ), y, m_Position.z ), m_fTeamSize, m_fTeamSize, col, m_Panel, E_SpriteType_Sprite, m_RenderState );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );
			gSimple2D.AddTexUVs( 0, tex );

			m_RenderState.m_EffectIndex = effect;

			x += m_fTeamSize;
			x += m_fGapX;
		}

		// TODO - column title?

		x += m_fPointsLength;

		// data
		gFont.FixedSpaceNext( E_FixedSpaceType_Numbers );
		gFont.DrawText( m_Layer, Maths::HVec3( x, y, m_Position.z ), E_Justify_MiddleRight, 
			C_UILocaliser::Instance().ConvertNumberToCommas( m_SortInfo[ i ].m_sData ), dataCol, 
			m_Size.y, m_DataFontStylePasses, 0.0f, m_Panel );


		x += m_fGapX;

		if ( i == m_sPositionMe )
			col = m_SpriteColours;	// text is always in your colour

		// name
		gFont.DrawText( m_Layer, Maths::HVec3( x, y, m_Position.z ), E_Justify_MiddleLeft, 
			levelState.mCars[ m_SortInfo[ i ].m_sVehicleIndex ].mNameAboveCar.Chars(), 
			col, m_Size.y, m_FontStylePasses, 0.0f, m_Panel );

		y += ( m_Size.y * Up() );		
	}
}


void UIMultiplayerData::ToggleScoreboard()
{
	m_bFullscreenDoRender = !m_bFullscreenDoRender;
	C_UiTimelineMessage msg( "mp_fullscreen_leaderboard_on", m_bFullscreenDoRender );
	gButtons.BroadcastMessageNow( 0xffffffff, &msg );
}

}; // namespace UI
