#include "stdafx.h"

#include "UITeamTugOfWar.h"
#include "UiHelpers.h"

#include "UIMessages.h"

#include "GameLogic/Structures/MpResults.h"
#include <Network/Misc/NetDefines.h>

#include <GameLogic/Layers/RaceLogicLayer.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UITeamTugOfWar, "UITeamTugOfWar" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

namespace UI
{

// --------------
// UITeamTugOfWar
// --------------
void UITeamTugOfWar::Init()
{
	C_GadgetBase::Init();

	m_uTeamMe = GetLevelState().mCars[ 0 ].mTeam;

	m_uTeamPointsA = 0;
	m_uTeamPointsB = 0;

	// messages
	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
}

void UITeamTugOfWar::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// resolve colours
	m_TeamColourA.SetByName( "team_a" );
	m_TeamColourB.SetByName( "team_b" );
}

void UITeamTugOfWar::FrameUpdate()
{
	C_GadgetBase::FrameUpdate();

	Game::C_RaceLogicLayer* pRLL = GetRaceLogicLayer();

	if ( !pRLL )
		return;

	m_uTeamPointsA = 0;
	m_uTeamPointsB = 0;

	U32 uTeamCountA = 0;
	U32 uTeamCountB = 0;

	Game::MpVehiclesData& sortedData = Game::C_MpResults::Instance().m_SortedVehicleResults;
	S32 sNumVehicles = pRLL->GetNumLiveVehicles();
	
	for ( S32 i = 0; i < sNumVehicles; i++ )
	{
		if ( i >= sortedData.Size() )
			break;	// wtf

		U32 uPlayerPoints = ( sNumVehicles - sortedData[ i ].m_Position ) + 1;	// current driver score

		if ( sortedData[ i ].m_Team == RacerNet::NET_TEAM_A )
		{
			uTeamCountA++;
			m_uTeamPointsA += uPlayerPoints;
		}
		else
		{
			uTeamCountB++;
			m_uTeamPointsB += uPlayerPoints;
		}
	}

	// take off the least amount of points a team can earn to make the bar more "active"
	U32 uLeastPointsA = ( uTeamCountA * ( uTeamCountA + 1 ) ) / 2;
	U32 uLeastPointsB = ( uTeamCountB * ( uTeamCountB + 1 ) ) / 2;

	m_uTeamPointsA -= uLeastPointsA;
	m_uTeamPointsB -= uLeastPointsB;
}

bool UITeamTugOfWar::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
			if ( m_sLocalPlayerIndex == -1 )
				m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
			break;

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UITeamTugOfWar::Render()
{
	if ( !m_Activated )
		return;

	if ( ( m_uTeamPointsA + m_uTeamPointsB ) == 0 )
		return;	// div 0

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	//
	static F32 s_fTeamIconSize = 2.0f;
	static F32 s_fGapX = 0.5f;
	static F32 s_fBarHeight = 1.5f;
	static F32 s_fBarWidth = 10.0f;
	static F32 s_fScale = 0.65f;

	F32 fTeamIconSize = s_fTeamIconSize * s_fScale;
	F32 fGapX = s_fGapX * s_fScale;
	F32 fBarHeight = s_fBarHeight * s_fScale;
	F32 fBarWidth = s_fBarWidth * s_fScale;
	//

	C_UISpriteColours& colM = ( m_uTeamMe == RacerNet::NET_TEAM_A ) ? m_TeamColourA : m_TeamColourB;
	C_UISpriteColours& colO = ( m_uTeamMe == RacerNet::NET_TEAM_A ) ? m_TeamColourB : m_TeamColourA;

	C_TextureParam& texM = GetTextureParam( ( m_uTeamMe == RacerNet::NET_TEAM_A ) ? 0 : 1 );
	C_TextureParam& texO = GetTextureParam( ( m_uTeamMe == RacerNet::NET_TEAM_A ) ? 1 : 0 );

	// 01 team value
	F32 fValue01 = ( ( m_uTeamMe == RacerNet::NET_TEAM_A ) ? m_uTeamPointsA : m_uTeamPointsB ) / (F32)( m_uTeamPointsA + m_uTeamPointsB );

	F32 x = m_Position.x;
	F32 y = m_Position.y;
	F32 z = m_Position.z;

	// team icon me
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, z ), fTeamIconSize, fTeamIconSize, colM, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );
	gSimple2D.AddTexUVs( 0, texM );

	x += fTeamIconSize;
	x += fGapX;

	// left end
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, z ), fBarHeight / 2.0f, fBarHeight, colM, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 2 ) );

	x += fBarHeight / 2.0f;

	// bar me
	F32 fSizeM = fBarWidth * fValue01;

	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, z ), fSizeM, fBarHeight, colM, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 3 ) );

	x += fSizeM;

	// bar other
	F32 fSizeO = fBarWidth * ( 1.0f - fValue01 );

	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, z ), fSizeO, fBarHeight, colO, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 3 ) );

	x += fSizeO;

	// right end
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, z ), fBarHeight / 2.0f, fBarHeight, colO, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 4 ) );

	x += fBarHeight / 2.0f;
	x += fGapX;

	// team icon other
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, z ), fTeamIconSize, fTeamIconSize, colO, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );
	gSimple2D.AddTexUVs( 0, texO );

	// backing
	x = m_Position.x;
	x += fTeamIconSize;
	x += fGapX;

	// backing left
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, z ), fBarHeight / 2.0f, fBarHeight, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 5 ) );

	x += fBarHeight / 2.0f;

	// backing left middle
	F32 fSizeBM = ( fBarWidth - fBarHeight ) / 2.0f;

	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, z ), fSizeBM, fBarHeight, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 6 ) );

	x += fSizeBM;

	// backing middle
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, z ), fBarHeight, fBarHeight, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 7 ) );

	x += fBarHeight;

	// backing right middle
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, z ), fSizeBM, fBarHeight, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 8 ) );

	x += fSizeBM;

	// backing right
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, z ), fBarHeight / 2.0f, fBarHeight, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_MiddleLeft );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 9 ) );
}

}; // namespace UI
