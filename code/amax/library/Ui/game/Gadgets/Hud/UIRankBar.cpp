#include "stdafx.h"

#include <ui/Graphics/SimpleTimer.h>
#include <GameLogic/Profile/GameProfileManager.h>

#include "UIRankBar.h"
#include "UiHelpers.h"
#include "UIMessages.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::UIRankBar, "UIRankBar" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_xtColourBacking, "colour_backing" )
XT_END_TYPE

namespace UI
{

// --------
// UIRankBar
// --------
void UIRankBar::Init()
{
	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );

	/*
	m_pMpProfile = NULL;

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
	if( pGameProfile )
	{
		Game::C_RaceBookLayer* pLayer = UI::GetRaceBookLayer();
		if( pLayer && pLayer->GetGameModePtr()->IsMultiplayer() )
			m_pMpProfile = pGameProfile->GetMultiplayerProfile();
	}

	m_sCurrentRank = -1;
	*/
	
	C_GadgetBase::Init();
}

void UIRankBar::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_BackingColour.SetByName( m_xtColourBacking );
}

void UIRankBar::Render()
{
	if ( !m_Activated )
		return;

	/*
	if( !m_pMpProfile )
		return;

	//	[GavinY] - Needed to move this into the first frame of rendering so that the MP profile PreRaceUpdate had been called before we grab the InRaceLevel.
	if(m_sCurrentRank == -1)
	{
		if ( m_pMpProfile )
			m_sCurrentRank = m_pMpProfile->GetInRaceLevel();
	}

	C_UIAutoTransform transform( this );
	C_GadgetBase::Render();

	C_UISpriteColours backingColour;
	backingColour.Set( m_BackingColour, m_AlphaInherit, m_IntensityInherit );

	// Draw backing
	gSimple2D.AddSprite(m_Layer, m_Position, m_Size.x, m_Size.y, backingColour, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddJustification( m_Justify );
	gSimple2D.AddTexUVs(0, GetTextureParam( 0 ));

	F32 num = (m_pMpProfile->GetInRaceFans() - m_pMpProfile->GetInRaceLevelFansPrev());
	F32 denom = (m_pMpProfile->GetInRaceLevelFans() - m_pMpProfile->GetInRaceLevelFansPrev());
	if( denom <= 0.0f )
		return;

	// Draw bar
	F32 scale_x = num / denom;

	if ( ( m_sCurrentRank != -1 ) && ( m_pMpProfile->GetInRaceLevel() > m_sCurrentRank ) )
	{
		// we've just ranked up
		_UI_MESSAGE4( 
			E_UiGameMsg_MpRankedUp, 
			m_pMpProfile->GetInRaceLevel(), m_pMpProfile->GetLegendLevel(),
			m_pMpProfile->GetInRaceFans(), m_pMpProfile->GetInRaceLevelFans() 
		);

		m_sCurrentRank = m_pMpProfile->GetInRaceLevel();
	}

	gSimple2D.AddSprite(m_Layer, m_Position, m_Size.x * scale_x, m_Size.y, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddJustification( m_Justify );
	gSimple2D.AddTexUVs(0, GetTextureParam( 1 ));
	*/
}

}; // namespace UI
