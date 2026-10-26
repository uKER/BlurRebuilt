#include "stdafx.h"

#include "UIDamage.h"
#include "UiHelpers.h"

#include "UIMessages.h"
#include "UiXtConfig.h"
#include <Ui/UiInterface.h>
#include "Ui/SoundEffects.h"

#include <ui/Game/Gadgets/UIStaticGraphic.h>
#include <ui/Game/Gadgets/UIStaticText.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <RaceBook/Data/RaceBookData.h>

#include <game/game.h>
#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"

extern Util::C_Random gUiRand;

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::C_UiDamageSlotParam, "UiDamageSlotParam" )
	XT_FIELD( m_fScale, "scale" )
	XT_FIELD( m_fAlpha, "alpha" )
XT_END_TYPE

XT_TYPE( UI::UIDamage, "UIDamage" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_xtColourHigh, "colour_high" )
	XT_FIELD( m_xtColourMedium, "colour_medium" ) 
	XT_FIELD( m_xtColourLow, "colour_low" )
	XT_FIELD( m_xtColourEmpty, "colour_empty" )
	XT_FIELD( m_fColourEmptyAlpha, "colour_empty_alpha" )
	XT_FIELD( m_fGapMult, "segment_gap_mult" )
	XT_FIELD( m_Slot, "slot" )
	XT_FIELD( m_sVehicleIndexOverride, "vehicle_index_override" )
	XT_FIELD( m_bBoss, "boss" )
	XT_FIELD( m_bAlwaysDoEffect, "always_do_effect" )	
XT_END_TYPE

namespace UI
{

// --------
// UIDamage
// --------
void UIDamage::Init()
{
	C_GadgetBase::Init();

	m_fColourEmptyAlpha = 1.0f;
	m_fGapMult = 0.0f;

	m_bDamageEffectStarted = false;
	m_bDamageWarning = false;
	m_eHealthLevel = E_HEALTH_LEVEL_HIGH;
	m_eHealthLevelPrev = m_eHealthLevel;
	m_sCurrentHealth = 0;

	m_sMaxSegments = 0;
	m_pHealthText = NULL;
	m_pMirrorL = NULL;
	m_pMirrorR = NULL;

	m_bIsDestruction = false;
	m_uAmaxVehicleIdEasy = 0;
	m_uAmaxVehicleIdMedium = 0;
	m_uAmaxVehicleIdHard = 0;

	// messages
	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
}

void UIDamage::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_fSegmentSizeX = m_Size.x;

	// resolve colour styles
	m_ColourHigh.SetByName( m_xtColourHigh );
	m_ColourMedium.SetByName( m_xtColourMedium );
	m_ColourLow.SetByName( m_xtColourLow );
	m_ColourEmpty.SetByName( m_xtColourEmpty );
	m_ColourEmpty.SetAlpha( (U8)( m_fColourEmptyAlpha * 255.0f ) );

	m_PositionOriginalX = m_Position.x;

	m_RenderStateScale = m_RenderState;
	m_RenderStateScale.m_BlendMode = Display::E_BlendMode_AlphaAdditive;

	// these are not used in Sorcery but set in SP Boss HUD
	m_sVehicleIndexOverride = -1;
	m_bBoss = false;
	m_bAlwaysDoEffect = false;

	m_bIsDestruction = ( GetLevelState().mRaceMode == Game::E_RaceMode_Destruction ) || ( GetLevelState().mRaceMode == Game::E_RaceMode_FriendDemand_Destruction );

	if ( m_bIsDestruction )
	{
		Game::C_RaceBookLayer* pRaceBookLayer = GetRaceBookLayer();

		if ( !pRaceBookLayer || !pRaceBookLayer->GetRaceModePtr() || ( ( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_Destruction ) && ( pRaceBookLayer->GetRaceMode() != Game::E_RaceMode_FriendDemand_Destruction ) ) )
			return;

		Game::C_RaceModeDestruction* pRace = (Game::C_RaceModeDestruction*)pRaceBookLayer->GetRaceModePtr();
		SP_DestructionEvent* pEvent = pRace->GetEvent();

		if ( !pEvent || !pEvent->event || !pEvent->event->raceMode )
			return;

		AmaxRaceModeDestruction* pAmaxRaceModeDestruction = (AmaxRaceModeDestruction*)pEvent->event->raceMode;

		RaceBookVehicle* pRaceBookVehicleEasy = pAmaxRaceModeDestruction->vehicleEasy.Object();
		RaceBookVehicle* pRaceBookVehicleMedium = pAmaxRaceModeDestruction->vehicleMedium.Object();
		RaceBookVehicle* pRaceBookVehicleHard = pAmaxRaceModeDestruction->vehicleHard.Object();
		RaceBookVehicle* pRaceBookVehicleVHard = pAmaxRaceModeDestruction->vehicleVHard.Object();

		if ( pRaceBookVehicleEasy && pRaceBookVehicleEasy->vehicle )
			m_uAmaxVehicleIdEasy = pRaceBookVehicleEasy->vehicle->ID;

		if ( pRaceBookVehicleMedium && pRaceBookVehicleMedium->vehicle )
			m_uAmaxVehicleIdMedium = pRaceBookVehicleMedium->vehicle->ID;

		if ( pRaceBookVehicleHard && pRaceBookVehicleHard->vehicle )
			m_uAmaxVehicleIdHard = pRaceBookVehicleHard->vehicle->ID;

		if ( pRaceBookVehicleVHard && pRaceBookVehicleVHard->vehicle )
			m_uAmaxVehicleIdVHard = pRaceBookVehicleVHard->vehicle->ID;
	}
}

void UIDamage::PostInit()
{
	C_GadgetBase* pGadget = gButtons.FindByName( "Mirror_Health_Text" );
	if ( pGadget && ( pGadget->GetType() == E_UIButtonType_StaticText ) )
		m_pHealthText = (UIStaticText*)pGadget;

	pGadget = gButtons.FindByName( "Mirror_ImageL" );
	if ( pGadget && ( pGadget->GetType() == E_UIButtonType_StaticGraphic ) )
		m_pMirrorL = (UIStaticGraphic*)pGadget;

	pGadget = gButtons.FindByName( "Mirror_ImageR" );
	if ( pGadget && ( pGadget->GetType() == E_UIButtonType_StaticGraphic ) )
		m_pMirrorR = (UIStaticGraphic*)pGadget;
}

void UIDamage::FrameUpdate()
{
}

bool UIDamage::MessageHandler( const Messaging::C_MessageBase* pMessage )
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

void UIDamage::Render()
{
	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();	

	if ( !m_Activated )
		return;

	// Logic actor
	S32 sVehicleIndex = ( m_sVehicleIndexOverride == -1 ) ? GetPlayerVehicleIndex( m_sLocalPlayerIndex ) : m_sVehicleIndexOverride;
	Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( sVehicleIndex );
	if( !pLogicActor )
		return;

	// Amax vehicle
	AmaxVehicle* pVehicleAM = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)levelState.mCars[ sVehicleIndex ].mCarID ];
	if( !pVehicleAM )
		return;

	// Colours
	C_UISpriteColours colour;
	C_UISpriteColours colourEmpty;

	S32 sOldHealth = m_sCurrentHealth;
	m_sCurrentHealth = pLogicActor->GetCurrentHealth();

	m_eHealthLevel = pLogicActor->GetCurrentHealthLevel();

	// We override the health display when vehicle in last gasp mode
	if(pLogicActor->GetIsInLastGaspMode())
	{
		m_sCurrentHealth = 0; 
		m_eHealthLevel = E_HEALTH_LEVEL_WRECKED;
	}

	S32 sHealthIncrement = pLogicActor->GetRLC()->GetRLSystem()->mDamageSystem->mParameters->mHealthIncrement;

	if ( sHealthIncrement == 0 )
		sHealthIncrement = 100;

	m_sMaxSegments = (S32)(pLogicActor->GetRLC()->GetRLSystem()->mDamageSystem->mParameters->mVehicleHealthGroups[ pVehicleAM->vehicleHealthGroup ].mBaseHealth / sHealthIncrement);

	if ( levelState.mCars[ sVehicleIndex ].mHealthOverride != 0 )
		m_sMaxSegments = ( levelState.mCars[ sVehicleIndex ].mHealthOverride / sHealthIncrement );
	else if(levelState.mCars[ sVehicleIndex ].mUpgrades.NumAdditionalHealthSlots())
	{
		m_sMaxSegments += levelState.mCars[ sVehicleIndex ].mUpgrades.NumAdditionalHealthSlots();
	}

	if ( m_bIsDestruction && ( m_sVehicleIndexOverride != -1 ) )
	{
		// no offset
		m_bBoss = true;
		m_Position.x = 0.0f;
		m_Justify = E_Justify_BottomCentre;

		S32 sBlindFireDamage = RLDatabase::Instance().GetPickupType( E_PICKUP_TYPE_BLINDFIRE )->mNormal->mBalancingData->GetDamageInflicted();
		S32 sNumBars = ( ( m_sCurrentHealth + 1 ) / sBlindFireDamage );
		m_sCurrentHealth = sNumBars * sHealthIncrement;

			 if ( sNumBars <= 1 )	m_eHealthLevel = E_HEALTH_LEVEL_LOW;
		else if ( sNumBars <= 2 )	m_eHealthLevel = E_HEALTH_LEVEL_MEDIUM;
		else						m_eHealthLevel = E_HEALTH_LEVEL_HIGH;

		U32 uCarId = levelState.mCars[ m_sVehicleIndexOverride ].mCarID;

			 if ( uCarId == m_uAmaxVehicleIdEasy )		m_sMaxSegments = 1;
		else if ( uCarId == m_uAmaxVehicleIdMedium )	m_sMaxSegments = 2;
		else if ( uCarId == m_uAmaxVehicleIdHard )		m_sMaxSegments = 3;
		else											m_sMaxSegments = 0;		// ?
	}

	// auto-transform
	C_UIAutoTransform transform( this );
	C_GadgetBase::Render();

	if ( ( m_eHealthLevel != m_eHealthLevelPrev ) && ( m_eHealthLevel == E_HEALTH_LEVEL_LOW ) && ( m_sVehicleIndexOverride == -1 ) )
		UiInterface()->PlaySound( E_UiSoundEffect_HealthLowActionIn );

	m_eHealthLevelPrev = m_eHealthLevel;

	switch ( m_eHealthLevel )
	{
		case E_HEALTH_LEVEL_HIGH:
			colour.Set( m_ColourHigh, m_AlphaInherit, m_IntensityInherit );
			break;
		case E_HEALTH_LEVEL_MEDIUM:
			colour.Set( m_ColourMedium, m_AlphaInherit, m_IntensityInherit );
			break;
		case E_HEALTH_LEVEL_LOW:
		case E_HEALTH_LEVEL_WRECKED:
			colour.Set( m_ColourLow, m_AlphaInherit, m_IntensityInherit ); 
			break;
		default:
			break;
	}
	
	colourEmpty.Set( m_ColourEmpty, m_AlphaInherit, m_IntensityInherit );

	F32 x = m_PositionOriginalX;
	F32 y = m_Position.y;

	m_Size.x = ( ( m_fSegmentSizeX * m_sMaxSegments ) + ( m_fSegmentSizeX * m_fGapMult * ( m_sMaxSegments - 1 ) ) );

	if ( ( m_sVehicleIndexOverride != -1 ) && !m_bBoss )
	{
		static F32 fMult = 0.4f;
		S32 sMinSegments = (S32)(pLogicActor->GetRLC()->GetRLSystem()->mDamageSystem->mParameters->mVehicleHealthGroups[ AmaxCarHealthGroup_Low ].mBaseHealth / sHealthIncrement);
		F32 fMinSizeX = ( ( m_fSegmentSizeX * sMinSegments ) + ( m_fSegmentSizeX * m_fGapMult * ( sMinSegments - 1 ) ) );
		F32 fOffset = ( m_Size.x - fMinSizeX ) * fMult;
		x += fOffset;
		m_Position.x = m_PositionOriginalX + fOffset;
	}

	const Maths::HVec3 offset = gSimple2D.GetJustificationOffset( m_Justify, m_Panel );

	x -= ( offset.X() * m_Size.x );
	y -= ( offset.Y() * m_Size.y * Up() );

	if ( m_sCurrentHealth != sOldHealth )
	{
		if ( m_sVehicleIndexOverride == -1 )
			DoMirrorSmash();

		if ( ( sOldHealth != 0 ) && ( ( m_sVehicleIndexOverride == -1 ) || m_bAlwaysDoEffect ) )
		{
			//C_UiTimelineMessage msg( "damaged", true, 0.0f );	// TODO - support splitscreen if we end up using this again
			//gButtons.BroadcastMessageNow( 0xffffffff, &msg );

			if ( m_sCurrentHealth < sOldHealth )
				DoDamagedText();

			S32 sNumSlotsOld = (S32)ceilf( sOldHealth / (F32)sHealthIncrement );
			S32 sNumSlotsNew = (S32)ceilf( m_sCurrentHealth / (F32)sHealthIncrement );
			S32 sNumSlotsChanged = (S32)ceilf( ( Maths::Max( m_sCurrentHealth, sOldHealth ) - Maths::Min( m_sCurrentHealth, sOldHealth ) ) / (F32)sHealthIncrement );
			S32 sStartSlot = Maths::Max( sNumSlotsOld, sNumSlotsNew ) - sNumSlotsChanged;

			Util::C_String32 string;
			for ( S32 i = 0; i < sNumSlotsChanged; i++ )
			{
				S32 sSlot = sStartSlot + i;
				string.Printf( "scale_slot_%d", sSlot );
				C_UiTimelineMessage msg( string.Chars(), true, 0.0f );
				MessageHandler( &msg );
			}

			if ( sNumSlotsChanged > 0 )
				UiInterface()->PlaySound( E_UiSoundEffect_HudHealthLossActionIn );
		}
	}

	S32 sNumSegments = 0;
	S32 sLastHealth = 0;
	
	// alpha for last segment
	C_UISpriteColours colourLast = colour;

	if ( sHealthIncrement )
	{
		sNumSegments = m_sCurrentHealth / sHealthIncrement;
		sLastHealth = m_sCurrentHealth % sHealthIncrement;
		colourLast.SetAlpha( (U8)( colourLast[ 0 ].A01() * ( (F32)sLastHealth / sHealthIncrement ) * 255.0f ) );
	}

	if ( sLastHealth > 0 )
		sNumSegments++;

	F32 pos_x = x;
	F32 inc_x = ( 1.0f + m_fGapMult ) * m_fSegmentSizeX;

	// default
	for ( S32 i = 0; i < m_sMaxSegments; i++ )
	{
		C_UIColour col = ( i >= sNumSegments ) ? colourEmpty[ 0 ] : ( ( i == ( sNumSegments - 1 ) ) && ( sLastHealth > 0 ) ) ? colourLast[ 0 ] : colour[ 0 ];

		gSimple2D.AddSprite( m_Layer, Maths::HVec3( pos_x, y, m_Position.z ), m_fSegmentSizeX, m_Size.y, 
			col, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddJustification( E_Justify_TopLeft );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );

		pos_x += inc_x;
	}

	pos_x = x;

	// glow effect
	for ( S32 i = 0; i < m_sMaxSegments; i++ )
	{
		if ( ( i < m_Slot.count ) && ( m_Slot[ i ].m_fScale > 0.001f ) && ( m_Slot[ i ].m_fAlpha > 0.0f ) )
		{
			C_UIColour col2 = colour[ 0 ];
			col2.A01( m_Slot[ i ].m_fAlpha );

			gSimple2D.AddSprite( m_Layer, Maths::HVec3( pos_x + m_fSegmentSizeX * 0.5f, y + m_Size.y * 0.5f * Up(), m_Position.z ), 
				m_fSegmentSizeX * m_Slot[ i ].m_fScale, m_Size.y * m_Slot[ i ].m_fScale, col2, m_Panel, E_SpriteType_Sprite, m_RenderStateScale );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );
			gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );
		}

		pos_x += inc_x;
	}
}

void UIDamage::DoDamagedText()
{
	if ( !m_pHealthText || !C_UiXtConfig::Instance() )
		return;

	XtArray< C_UiHealthTextParam >& healthParams = C_UiXtConfig::Instance()->m_HealthTextParams;

	if ( ( m_eHealthLevel < 0 ) || ( m_eHealthLevel >= healthParams.count ) )
		return;

	if ( healthParams[ m_eHealthLevel ].m_Params.count == 0 )
		return;

	// health 01
	S32 sHealthIncrement = RLDatabase::Instance().mRLSystems[ 0 ]->mDamageSystem->mParameters->mHealthIncrement;
	S32 sHealthMin = RLDatabase::Instance().mRLSystems[ 0 ]->mDamageSystem->mHealthZones[ m_eHealthLevel + 1 ]->mHealthMin;
	S32 sHealthMax = RLDatabase::Instance().mRLSystems[ 0 ]->mDamageSystem->mHealthZones[ m_eHealthLevel + 1 ]->mHealthMax;

	if ( m_eHealthLevel == E_HEALTH_LEVEL_HIGH )
		sHealthMax = ( m_sMaxSegments * sHealthIncrement );

	F32 fHealth01 = ( m_sCurrentHealth - sHealthMin ) / (F32)( sHealthMax - sHealthMin );

	C_UiHealthTextParamState* pHealthState = &healthParams[ m_eHealthLevel ].m_Params[ 0 ];

	// find the correct param
	for ( S32 i = 1; i < healthParams[ m_eHealthLevel ].m_Params.count; i++ )
	{
		if ( fHealth01 < healthParams[ m_eHealthLevel ].m_Params[ i ].m_fLimit )
			pHealthState = &healthParams[ m_eHealthLevel ].m_Params[ i ];
		else
			break;
	}
	
	if ( pHealthState->m_Options.count == 0 )
		return;

	// randomise text id
	S32 sIndex = gUiRand.Rand( pHealthState->m_Options.count );
	C_TextStringID id( pHealthState->m_Options[ sIndex ].id );

	m_pHealthText->ChangeText( id );

	switch ( m_eHealthLevel )
	{
		case E_HEALTH_LEVEL_HIGH:	m_pHealthText->SetColourStyle( m_xtColourHigh );	break;
		case E_HEALTH_LEVEL_MEDIUM:	m_pHealthText->SetColourStyle( m_xtColourMedium );	break;
		case E_HEALTH_LEVEL_LOW:	m_pHealthText->SetColourStyle( m_xtColourLow );		break;
	}

	m_pHealthText->ResolveInheritedColours();
	m_pHealthText->DirtySet( C_GadgetBaseData::E_Dirty_Colour, true );

	m_pHealthText->SendPrivateTimelineMessaage( "damaged_text", true, 0.0f );
}

void UIDamage::DoMirrorSmash()
{
	if ( !m_pMirrorL || !m_pMirrorR || !C_UiXtConfig::Instance() )
		return;

	if ( ( m_pMirrorL->m_TextureParams.count < 3 ) || ( m_pMirrorR->m_TextureParams.count < 3 ) )
		return;

	XtArray< C_UiMirrorSmashParam >& mirrorSmashParams = C_UiXtConfig::Instance()->m_MirrorSmashParams;

	if ( mirrorSmashParams.count < 4 )
		return;

	C_UiMirrorSmashParam* pMirrorSmashParam = &mirrorSmashParams[ 0 ];

		 if ( m_sCurrentHealth <= 100 )		pMirrorSmashParam = &mirrorSmashParams[ mirrorSmashParams.count - 1 ];
	else if ( m_sCurrentHealth <= 200 )		pMirrorSmashParam = &mirrorSmashParams[ mirrorSmashParams.count - 2 ];
	else if ( m_sCurrentHealth <= 300 )		pMirrorSmashParam = &mirrorSmashParams[ mirrorSmashParams.count - 3 ];

#if 0
	if ( mirrorSmashParams.count == 0 )
		return;

	// health 01
	S32 sHealthMax = m_sMaxSegments * RLDatabase::Instance().mRLSystems[ 0 ]->mDamageSystem->mParameters->mHealthIncrement;
	F32 fHealth01 = m_sCurrentHealth / (F32)sHealthMax;

	C_UiMirrorSmashParam* pMirrorSmashParam = &mirrorSmashParams[ 0 ];

	// find the correct param
	for ( S32 i = 1; i < mirrorSmashParams.count; i++ )
	{
		if ( fHealth01 < mirrorSmashParams[ i ].m_fLimit )
			pMirrorSmashParam = &mirrorSmashParams[ i ];
		else
			break;
	}
#endif

	m_pMirrorL->m_TextureParams[ 2 ].m_Position = pMirrorSmashParam->m_Left.m_Position;
	m_pMirrorL->m_TextureParams[ 2 ].m_Size = pMirrorSmashParam->m_Left.m_Size;

	m_pMirrorR->m_TextureParams[ 2 ].m_Position = pMirrorSmashParam->m_Right.m_Position;
	m_pMirrorR->m_TextureParams[ 2 ].m_Size = pMirrorSmashParam->m_Right.m_Size;
}

}; // namespace UI
