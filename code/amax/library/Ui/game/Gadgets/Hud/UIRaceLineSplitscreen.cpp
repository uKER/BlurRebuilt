#include "stdafx.h"

#include "UIRaceLineSplitscreen.h"
#include "UiHelpers.h"

#include "UIMessages.h"
#include "UiXtConfig.h"

#include "Game/GameActors/Components/Messages/VehicleLogicMessages.h"

#include <algorithm>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UIRaceLineSplitscreen, "UIRaceLineSplitscreen" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_fVehicleWidth, "vehicle_width" )
	XT_FIELD( m_fVehicleHeight, "vehicle_height" )
	XT_FIELD( m_fIconWidthStart, "icon_width_start" )
	XT_FIELD( m_fIconHeightStart, "icon_height_start" )
	XT_FIELD( m_fIconWidthEnd, "icon_width_end" )
	XT_FIELD( m_fIconHeightEnd, "icon_height_end" )
	XT_FIELD( m_xtColourYou0, "colour_you0" )
	XT_FIELD( m_xtColourYou1, "colour_you1" )
	XT_FIELD( m_xtColourYou2, "colour_you2" )
	XT_FIELD( m_xtColourYou3, "colour_you3" )
	XT_FIELD( m_xtColourOther, "colour_other" )
	XT_FIELD( m_xtColourStart, "colour_start" )
	XT_FIELD( m_xtColourEnd, "colour_end" )
	XT_FIELD( m_fPlayerOffsetY, "player_offset_y" )
XT_END_TYPE

namespace UI
{

// -------------------------------
// C_UIRaceLineSortInfoSplitscreen
// -------------------------------
bool C_UIRaceLineSortInfoSplitscreen::operator < ( const C_UIRaceLineSortInfoSplitscreen& o ) const
{
	if ( !m_bPlayer && o.m_bPlayer )
		return true;

	if ( ( ( m_bPlayer && o.m_bPlayer ) || ( !m_bPlayer && !o.m_bPlayer ) ) && ( m_fValue01 < o.m_fValue01 ) )
		return true;

	return false;
}

// ---------------------
// UIRaceLineSplitscreen
// ---------------------
void UIRaceLineSplitscreen::Init()
{
	C_GadgetBase::Init();

	m_fVehicleWidth = 10.0f;
	m_fVehicleHeight = 20.0f;
	
	m_fIconWidthStart = 20.0f;
	m_fIconHeightStart = 20.0f;
	m_fIconWidthEnd = 20.0f;
	m_fIconHeightEnd = 20.0f;

	m_fPlayerOffsetY = 0.0f;

	// just going off game mode since there isn't a sorcery side to this anyway (mtaylor)
	if(GetLevelState().mRaceMode == Game::E_RaceMode_Multiplayer_Destruction)
	{
		//Hopefully the variable name says it all
		m_HideBecauseRaceModeIsNonLapBased = true;
	}
	else
	{
		m_HideBecauseRaceModeIsNonLapBased = false;
	}
}

void UIRaceLineSplitscreen::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// resolve colour styles
	m_ColourYou0.SetByName( m_xtColourYou0, m_Alpha, m_Intensity  );
	m_ColourYou1.SetByName( m_xtColourYou1, m_Alpha, m_Intensity  );
	m_ColourYou2.SetByName( m_xtColourYou2, m_Alpha, m_Intensity  );
	m_ColourYou3.SetByName( m_xtColourYou3, m_Alpha, m_Intensity  );
	m_ColourOther.SetByName( m_xtColourOther, m_Alpha, m_Intensity );
	m_ColourStart.SetByName( m_xtColourStart, m_Alpha, m_Intensity );
	m_ColourEnd.SetByName( m_xtColourEnd, m_Alpha, m_Intensity );

	Game::Actors::RaceLogicCore* pCore = GetRLC();

	m_sNumVehicles = 0;

	if ( pCore )
		m_sNumVehicles = (S32)pCore->GetNumVehicles();

	m_SortInfo.Reserve( m_sNumVehicles );

	for ( S32 i = 0; i < m_sNumVehicles; i++ )
		m_SortInfo.PushBack( C_UIRaceLineSortInfoSplitscreen() );

	m_fMaxLogic01 = GetLevelState().mLaps * 1.0f;
}

void UIRaceLineSplitscreen::Render()
{
	if ( !m_Activated || m_HideBecauseRaceModeIsNonLapBased)
		return;

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	GatherSortInfo();

	C_RenderState renderStateCar = m_RenderState;
	renderStateCar.m_EffectIndex = E_Effect_MultiTexture;

	// render start icon
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x - ( m_Size.x / 2.0f ), m_Position.y, m_Position.z ),
		m_fIconWidthStart, m_fIconHeightStart, m_ColourStart, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );

	// render end icon
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x + ( m_Size.x / 2.0f ), m_Position.y, m_Position.z ),
		m_fIconWidthEnd, m_fIconHeightEnd, m_ColourEnd, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 2 ) );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );

	// render cars
	for ( S32 i = 0; i < m_SortInfo.Size(); i++ )
	{
		if ( !m_SortInfo[ i ].m_bActive )
			continue;

		F32 fPos01 = m_SortInfo[ i ].m_fValue01 / m_fMaxLogic01;
		F32 fOffsetY = 0.0f;
		
		C_UISpriteColours colour = m_ColourOther;

		if ( m_SortInfo[ i ].m_bPlayer )
		{
			S32 sPadIndex = GetLevelState().mCars[ m_SortInfo[ i ].m_sVehicleIndex ].mController;

				 if ( sPadIndex == 0 )		colour = m_ColourYou0;
			else if ( sPadIndex == 1 )		colour = m_ColourYou1;
			else if ( sPadIndex == 2 )		colour = m_ColourYou2;
			else							colour = m_ColourYou3;

			fOffsetY = ( sPadIndex - ( ( sPadIndex < 2 ) ? 2 : 1 ) ) * m_fPlayerOffsetY;
		}

		gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x - ( m_Size.x / 2.0f ) + ( m_Size.x * fPos01 ), m_Position.y + fOffsetY, m_Position.z ),
			m_fVehicleWidth, m_fVehicleHeight, colour, m_Panel, E_SpriteType_Sprite, renderStateCar );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddRotation( 0.0f, 0.0f, 90.0f );
	}
}

void UIRaceLineSplitscreen::GatherSortInfo()
{
	for ( S32 i = 0; i < m_sNumVehicles; i++ )
	{
		Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( i );

		if ( !pLogicActor )
		{
			m_SortInfo[ i ].m_bActive = false;
			continue;
		}

		m_SortInfo[ i ].m_sVehicleIndex = i;
		m_SortInfo[ i ].m_fValue01 = pLogicActor->GetPositionInfo().mLogicPositionCombined - 1.0f;	// this starts at 1

		Game::Actors::C_VehicleActor* pVehicleActor = pLogicActor->GetGraphicalActor( i );

		m_SortInfo[ i ].m_bActive = (pVehicleActor && GetLevelState().mCars[ i ].mState == Game::E_VEHICLE_STATE_RACING) ? true : false;
		m_SortInfo[ i ].m_bPlayer = GetLevelState().mCars[ i ].mHuman;
	}

	std::sort( m_SortInfo.Begin(), m_SortInfo.End() );
}

}; // namespace UI
