#include "stdafx.h"

#include <game/Game.h>

#include "UIPerks.h"
#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Graphics/SimpleTimer.h>
#include "Ui/UiXtConfig.h"
#include "UiHelpers.h"

#include "Game/GameActors/Components/VehicleLogic/VLC_Pickups.h"

#include "UiMessages.h"
#include "GameDataFinder.h"

#include <RaceBook\RaceBookManager.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

Debug::Bool g_UiHudPerksOnlyOneForAI( "UI.HUD.PerksOnlyOneForAI", true, Debug::Persist() );

XT_TYPE( UI::C_UIPerkInfo, "UIPerkInfo" )
	XT_FIELD( m_sTextureIndex, "texture_index" )
	XT_FIELD( m_xtColourFill, "colour_fill" )
	XT_FIELD( m_xtColourGlow, "colour_glow" )
	XT_FIELD( m_xtColourText, "colour_text" )
	XT_FIELD( m_LocIdName, "loc_id_name" )
	XT_FIELD( m_PastLocIdName, "past_loc_id_name" )
XT_END_TYPE

XT_TYPE( UI::UIPerks, "UIPerks" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

namespace UI
{

// ------------
// C_UIPerkInfo
// ------------
void C_UIPerkInfo::Resolve()
{
	m_ColourFill.SetByName( m_xtColourFill );
	m_ColourGlow.SetByName( m_xtColourGlow );
	m_ColourText.SetByName( m_xtColourText );
}

// -------
// UIPerks
// -------
void UIPerks::Init()
{
	C_GadgetBase::Init();

	// messages
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
}

void UIPerks::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_RenderStatePickupNone = m_RenderState;
	m_RenderStatePickupNone.m_EffectIndex = E_Effect_MultiTexture;
}

bool UIPerks::MessageHandler( const Messaging::C_MessageBase* pMessage )
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

void UIPerks::Render()
{
	Game::C_RaceLogicLayer *pRLCLayer = GetRaceLogicLayer();
	if( !pRLCLayer )
		return;

	if( !pRLCLayer->PowerupsActivated() )
		return;

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	if ( !m_Activated || ( m_sLocalPlayerIndex == -1 ) )
		return;

	if ( m_Alpha == 0.0f )
		return;

	S32 sVehicleIndex = ( m_sVehicleIndexOverride == -1 ) ? GetPlayerVehicleIndex( m_sLocalPlayerIndex ) : m_sVehicleIndexOverride;

	// handle case of no vehicles loaded
	if ( sVehicleIndex == INVALID_VALUE )
		return;

	Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( sVehicleIndex );
	if ( !pLogicActor )
		return;

	if( !C_UiXtConfig::Instance() )
		return;

	// extra bolt mod
	bool bHasExtraBoltMod = false;
	Btl::Vector< S32 >& loadouts = levelState.mCars[ sVehicleIndex ].mLoadOuts;

	for ( S32 i = 0; i < loadouts.Size(); i++ )
	{
		if ( loadouts[ i ] == UI::E_MultiplayerVehicleAbilities_Boss_AdditionalBlindfireShots )
		{
			bHasExtraBoltMod = true;
			break;
		}
	}

	// perk info

	const VehiclePickupInfo& pickupInfo = pLogicActor->GetPickupInfo();
	C_UIPerkInfo* pInfo = NULL;
	bool bPickupNone = false;

	if( pickupInfo.mPickupStoreIndex != -1 && pickupInfo.mPickupStoreIndex < pickupInfo.mPickupStore.Size() )
		pInfo = C_UiXtConfig::Instance()->m_PerkDisplayParameters.GetPerkInfo( pickupInfo.mPickupStore[ pickupInfo.mPickupStoreIndex ].mType, pickupInfo.mPickupStore[ pickupInfo.mPickupStoreIndex ].mNumShots, bHasExtraBoltMod );
	else
	{
		pInfo = C_UiXtConfig::Instance()->m_PerkDisplayParameters.GetPerkInfo( E_PICKUP_TYPE_NONE );
		bPickupNone = true;
	}

	if ( !pInfo )
		return;

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	// Icon
	S32 sTexIndex = pInfo->m_sTextureIndex;
	C_TextureParam* pParam = NULL;

	if ( sTexIndex < 100 )
		pParam = &GetTextureParam( 0 );
	else
	{
		sTexIndex -= 100;
		pParam = &GetTextureParam( 2 );
	}

	C_UISpriteColours cols;
	cols.Set( pInfo->m_ColourFill, m_AlphaInherit, m_IntensityInherit );

	gSimple2D.AddSprite( m_Layer, m_Position, m_Size.x, m_Size.y, cols, m_Panel, E_SpriteType_Sprite, bPickupNone ? m_RenderStatePickupNone : m_RenderState );
	gSimple2D.AddJustification( m_Justify );

	if ( bPickupNone )
		gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );
	else
		gSimple2D.AddTexUVs( 0, pParam->m_TextureID, pParam->m_Position.u, pParam->m_Position.v + (pParam->m_Size.v * (F32)sTexIndex), pParam->m_Size.u, pParam->m_Size.v  );
}

}; // namespace UI
