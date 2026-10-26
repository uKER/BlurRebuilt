// PR - not used - TODO - REMOVE ME!
#include "stdafx.h"

#include "UIRivalHud.h"
#include <ui/Game/UILocaliser.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui/Game/Gadgets/UIStaticText.h>
//#include <ui/Game/Gadgets/UIStaticScene.h>
#include "UIDamage.h"
#include "UIPerks.h"

#include "GameLogic/Layers/RaceBookLayer.h"
#include "GameLogic/Layers/AmaxWorldLayer.h"
#include "GameLogic/InGameCarInfoManager.h"
#include "GameLogic/RaceLogicCore/Core/RaceLogicCore.h"

#include <GameLogic\RaceRules\RaceRulesManager.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

namespace Game
{
extern Debug::Float g_debugNameMaxFontSize;
};

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::UIRivalHud, "UIRivalHud" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

namespace UI
{

// ----------
// UIRivalHud
// ----------
void UIRivalHud::Init()
{
	C_GadgetBase::Init();

	for ( S32 i = 0; i < m_Objects.Size(); i++ )
		m_Objects[ i ] = NULL;
}

void UIRivalHud::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// get the gadgets
	m_Objects[ E_RivalHud_Parent ]		= gUIScreenControl.FindByName( "rival_parent" );
	m_Objects[ E_RivalHud_Pos ]			= gUIScreenControl.FindByName( "rival_pos" );
	m_Objects[ E_RivalHud_Text ]		= gUIScreenControl.FindByName( "rival_text" );
	m_Objects[ E_RivalHud_Name ]		= gUIScreenControl.FindByName( "rival_name" );
	m_Objects[ E_RivalHud_RevsFull ]	= gUIScreenControl.FindByName( "rival_revs_full" );
	m_Objects[ E_RivalHud_RevsPartial ]	= gUIScreenControl.FindByName( "rival_revs_partial" );
	m_Objects[ E_RivalHud_Damage ]		= gUIScreenControl.FindByName( "rival_damage" );
	m_Objects[ E_RivalHud_Perks ]		= gUIScreenControl.FindByName( "rival_perks" );

	for ( S32 i = 0; i < m_Objects.Size(); i++ )
		if ( !m_Objects[ i ] )
			return;

	// remove render subscriber
	for ( S32 i = 0; i < m_Objects.Size(); i++ )
		m_Objects[ i ]->RemoveSubscriber( E_UiMsg_Render );
}

void UIRivalHud::Render()
{
	if ( !m_Activated )
		return;

	for ( S32 i = 0; i < m_Objects.Size(); i++ )
		if ( !m_Objects[ i ] )
			return;

	if ( !C_GameApp::GetInstance() || 
		 !C_GameApp::GetInstance()->GetRaceBookLayer() || 
		 !C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() )
		return;

	Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

	if ( !pCore )
		return;

	// *** TODO - this player index will come from Lua
	S32 sLocalPlayerIndex = 0;
	S32 sPlayerVehicleIndex = pCore->GetViewportFocusVehicleIndex( sLocalPlayerIndex );

	const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(sLocalPlayerIndex);
	if( !pLogicActor )
		return;

	for ( S32 i = 0; i < MAX_VEHICLES; i++ )
	{
		bool bValidRival = Game::C_RaceRulesManager::Instance().TrackVehicle(i);

		if ( !bValidRival || ( i == sPlayerVehicleIndex ) )
			continue;

		// get projected UI car coords
		Maths::HVec3 pos = Game::C_InGameCarInfoManager::Instance().GetCarCoords( i );

		if ( pos.Z() == -1.0f )
			continue;	// off screen

		F32 fAlpha = Game::C_InGameCarInfoManager::Instance().GetAlpha( i );
		F32 fScale = Game::C_InGameCarInfoManager::Instance().GetFontSize( i ) / Game::g_debugNameMaxFontSize;

		// set parent position
		m_Objects[ E_RivalHud_Parent ]->m_Position = pos + ( Maths::HVec3( m_Position ) * fScale );
		m_Objects[ E_RivalHud_Parent ]->m_Alpha = fAlpha;
		m_Objects[ E_RivalHud_Parent ]->m_Scale.x = fScale * m_Scale.x;
		m_Objects[ E_RivalHud_Parent ]->m_Scale.y = fScale * m_Scale.y;
		m_Objects[ E_RivalHud_Parent ]->Render();

		// set position
		((UIStaticText*)m_Objects[ E_RivalHud_Pos ])->SetDirectText( C_UILocaliser::Instance().ConvertNumberToPosition( pLogicActor->GetPositionInfo().mRacePosition, true ) );
		m_Objects[ E_RivalHud_Text ]->Render();

		C_WideString128 rivalName;// = Game::C_RaceRulesManager::Instance().GetVehicleRivalName(i);

		// name
		((UIStaticText*)m_Objects[ E_RivalHud_Name ])->SetDirectText( rivalName.Chars() );
		m_Objects[ E_RivalHud_Name ]->Render();

		// revs partial
		U32 uRevs = pCore->GetVehicleRevs( i );
		U32 uMaxRevs = pCore->GetVehicleRedLine( i );
		F32 fRevs01 = (F32)uRevs / (U32)uMaxRevs;

		// revs full
		m_Objects[ E_RivalHud_RevsFull ]->Render();
//		((UIStaticScene*)m_Objects[ E_RivalHud_RevsFull ])->UseVertexCount01( true );
//		((UIStaticScene*)m_Objects[ E_RivalHud_RevsFull ])->SetMaxVertexCount01( 1.0f - fRevs01 );

//		((UIStaticScene*)m_Objects[ E_RivalHud_RevsPartial ])->UseVertexCount01( true );
//		((UIStaticScene*)m_Objects[ E_RivalHud_RevsPartial ])->SetMaxVertexCount01( fRevs01 );
		m_Objects[ E_RivalHud_RevsPartial ]->Render();

		// damage
		((UIDamage*)m_Objects[ E_RivalHud_Damage ])->SetVehicleIndexOverride( i );
		m_Objects[ E_RivalHud_Damage ]->Render();

		// perks
		F32 fNameLength = gFont.StringLength( rivalName.Chars(), m_Objects[ E_RivalHud_Name ]->m_Size.y, ((UIStaticText*)m_Objects[ E_RivalHud_Name ])->m_FontStylePasses.GetFontStyle( 0 ) );
		((UIPerks*)m_Objects[ E_RivalHud_Perks ])->m_Position.x = fNameLength;
		((UIPerks*)m_Objects[ E_RivalHud_Perks ])->SetVehicleIndexOverride( i );
		m_Objects[ E_RivalHud_Perks ]->Render();
	}

	C_GadgetBase::Render();
}

}; // namespace UI
