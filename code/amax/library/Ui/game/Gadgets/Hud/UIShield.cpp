#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>

#include <GameLogic/Structures/LevelState.h>
#include <ui/Graphics/Font/Font.h>
#include <ui/Graphics/Font/FontStyle.h>

#include "UIShield.h"
#include "UiHelpers.h"

#include "UIMessages.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::UIShield, "UIShield" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_xtColourEmpty, "colour_empty" )
	XT_FIELD( m_fColourEmptyAlpha, "colour_empty_alpha" )
	XT_FIELD( m_sNumSegments, "num_segments" )
	XT_FIELD( m_fGapMult, "segment_gap_mult" )
XT_END_TYPE

namespace UI
{

// --------
// UIShield
// --------
void UIShield::Init()
{
	C_GadgetBase::Init();

	m_fColourEmptyAlpha = 1.0f;
	m_sNumSegments = 10;
	m_fGapMult = 0.0f;

	// messages
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
}

void UIShield::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_fSegmentSizeX = m_Size.x;
	m_Size.x = ( ( m_fSegmentSizeX * m_sNumSegments ) + ( m_fSegmentSizeX * m_fGapMult * ( m_sNumSegments - 1 ) ) );

	// resolve colour styles
	m_ColourEmpty.SetByName( m_xtColourEmpty );
	m_ColourEmpty.SetAlpha( (U8)( m_fColourEmptyAlpha * 255.0f ) );
}

bool UIShield::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
			if ( m_sLocalPlayerIndex == -1 )
			{
				m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
				S8 vehIdx = UI::GetPlayerVehicleIndex(m_sLocalPlayerIndex);

				Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
				m_sNumSegments = levelState.mCars[vehIdx].mUpgrades.NumShieldSlots();
				m_Size.x = ( ( m_fSegmentSizeX * m_sNumSegments ) + ( m_fSegmentSizeX * m_fGapMult * ( m_sNumSegments - 1 ) ) );			
			}
			break;

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIShield::Render()
{
	if ( !m_Activated )
		return;

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	const Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( GetPlayerVehicleIndex( m_sLocalPlayerIndex ) );

	if( !pLogicActor )
		return;

	C_UISpriteColours colour;
	C_UISpriteColours colourEmpty;

	colour.Set( m_SpriteColours, m_AlphaInherit, m_IntensityInherit );
	colourEmpty.Set( m_ColourEmpty, m_AlphaInherit, m_IntensityInherit );

	F32 x = m_Position.x;
	F32 y = m_Position.y;

	const Maths::HVec3 offset = gSimple2D.GetJustificationOffset( m_Justify, m_Panel );

	x -= ( offset.X() * m_Size.x );
	y -= ( offset.Y() * m_Size.y * Up() );

	// No longer relevant - SB
	//F32 fShield01 = pLogicActor->GetFanPower() / m_sNumSegments;

	//S32 sNumSegmentsActive = (S32)( fShield01 / ( 1.0f / m_sNumSegments ) );

	//for ( S32 i = 0; i < m_sNumSegments; i++ )
	//{
	//	F32 pos_x = ( x + m_fSegmentSizeX * i ) + ( m_fSegmentSizeX * m_fGapMult * i );
	//	gSimple2D.AddSprite( m_Layer, Maths::HVec3( pos_x, y, m_Position.z ), m_fSegmentSizeX, m_Size.y, 
	//		( i < sNumSegmentsActive ) ? colour : colourEmpty, m_Panel, E_SpriteType_Sprite, m_RenderState );
	//	gSimple2D.AddJustification( E_Justify_TopLeft );
	//	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	//}


}

}; // namespace UI
