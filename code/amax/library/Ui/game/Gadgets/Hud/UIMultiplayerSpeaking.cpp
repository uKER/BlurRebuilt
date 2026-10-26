#include "stdafx.h"

#include "UIMultiplayerSpeaking.h"

#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Graphics/SimpleTimer.h>

#include <Network/Core/NetServices.h>
#include <Network/Player/NetPlayerManager.h>

#include <algorithm>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UIMultiplayerSpeaking, "UIMultiplayerSpeaking" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_FontStylePasses, "styles" )
	XT_FIELD( m_fIconScale, "icon_scale" )
	XT_FIELD( m_xtColourIcon, "colour_icon" )
	XT_FIELD( m_fMultGapX, "mult_gap_x" )
	XT_FIELD( m_sMaxToShow, "max_to_show" )
XT_END_TYPE

namespace UI
{

// ---------------------
// UIMultiplayerSpeaking
// ---------------------
void UIMultiplayerSpeaking::Init()
{
	C_GadgetBase::Init();

	m_fIconScale = 1.0f;
	m_fMultGapX = 0.0f;
	m_sMaxToShow = -1;

	// messages
	AddSubscriber( E_UiMsg_FrameUpdate );
}

void UIMultiplayerSpeaking::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// resolve font styles
	m_FontStylePasses.Setup( pState );

	// resolve colour styles
	m_ColourIcon.SetByName( m_xtColourIcon );
}

void UIMultiplayerSpeaking::FrameUpdate()
{
	F32 fFrameTime = theFETimer.FrameTime();

	S32 sCount = 0;
	S32 sCountActive = 0;

	RacerNet::C_NetServices& ns = RacerNet::C_NetServices::Instance();

	if ( ns.IsInitialised() )
	{
		for ( U8 i = 0; i < ns.PlayerManager().TotalSize(); i++ )
		{
			RacerNet::C_NetPlayer* pPlayer = ns.PlayerManager().Index( i );

			if ( pPlayer && ( pPlayer->m_IsSpeaking > 0.0f ) )
			{
				m_SortInfo[ i ].m_bActive = true;
				m_SortInfo[ i ].m_fTime  += fFrameTime;
				m_SortInfo[ i ].m_pPlayer = pPlayer;

				sCountActive++;
			}
			else
				m_SortInfo[ i ].Clear();

			sCount++;
		}
	}
	
	for ( S32 i = sCount; i < m_SortInfo.Size(); i++ )
		m_SortInfo[ i ].Clear();

	if ( sCountActive > 0 )
		std::sort( m_SortInfo.Begin(), m_SortInfo.End() );
}

void UIMultiplayerSpeaking::Render()
{
	if ( !m_Activated )
		return;

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	// render most time first
	S32 sCount = 0;
	for ( S32 i = ( m_SortInfo.Size() - 1 ); i >= 0; i-- )
	{
		sCount++;

		if ( !m_SortInfo[ i ].m_bActive || ( ( m_sMaxToShow != -1 ) && ( sCount > m_sMaxToShow ) ) )
			break;

		F32 x = m_Position.x;
		F32 y = m_Position.y + ( ( sCount - 1 ) * m_Size.y ) * -Up();	// bottom to top

		// icon
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), m_Size.x * m_fIconScale, m_Size.y * m_fIconScale, 
			m_ColourIcon, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddJustification( E_Justify_MiddleLeft );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );

		x += m_Size.x * m_fIconScale;
		x += m_Size.x * m_fMultGapX;

		// name
		gFont.DrawText( m_Layer, Maths::HVec3( x, y, m_Position.z ), E_Justify_MiddleLeft, m_SortInfo[ i ].m_pPlayer->m_User.m_Name, 
			m_SpriteColours, m_Size.y, m_FontStylePasses, 0.0f, m_Panel );
	}
}

}; // namespace UI
