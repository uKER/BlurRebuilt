#include "StdAfx.h"
#include <Game/GameWorld/GameWorld.h>	// [Blur2PC]

#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <vehicle\VehicleModel.h>
#include <Game\GameWorld\ViewType.h>
#include <kanzen\camera\HollywoodVehicleCamera.h>

#include <ui/Graphics/SimpleTimer.h>

#include "Game/Gadgets/Hud/UIPerks2.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "Ui/SoundEffects.h"

#include "UIMessages.h"
#include "UiHelpers.h"
#include "UiXtConfig.h"

XT_TYPE( UI::C_UiPerkSlotEffectParam, "UiPerkSlotEffectParam" )
	XT_FIELD( m_fScale, "scale" )
	XT_FIELD( m_fAlpha, "alpha" )
	XT_FIELD( m_fModScale, "mod_scale" )
	XT_FIELD( m_fModAlpha, "mod_alpha" )
	XT_FIELD( m_fBlobScale, "blob_scale" )
	XT_FIELD( m_fBlobAlpha, "blob_alpha" )
XT_END_TYPE

XT_TYPE( UI::UIPerks2, "UIPerks2" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_AttachedToCar, "attached_to_car" )
	XT_FIELD( m_FoldoutScale, "foldout_scale" )
	XT_FIELD( m_ScaleAmount, "scale_amount" )
	XT_FIELD( m_ScaleSpeed, "scale_speed" )
	XT_FIELD( m_Padding, "padding" )
	XT_FIELD( m_ItemSize, "item_size" )
	XT_FIELD( m_DrawAspect, "aspect_ratio" )
	XT_FIELD( m_ScaleEpsilon, "epsilon" )
	XT_FIELD( m_SelectedEffectIndex, "selected_effect" )

	XT_FIELD( m_Bender, "bender" )
	XT_FIELD( m_AngleStep, "angle_step" )
	XT_FIELD( m_SlotStep, "slot_step" )
	XT_FIELD( m_SlotEffect, "slot_effect" )
XT_END_TYPE

// There is just one copy of this now in the UIConfig
XT_TYPE( UI::C_PerkDisplayParameters, "UiPerkDisplayParameters" )
	XT_FIELD( m_PerkInfoNitrous, "nitrous" )
	XT_FIELD( m_PerkInfoShock, "shock" )
	XT_FIELD( m_PerkInfoShunt, "shunt" )
	XT_FIELD( m_PerkInfoBarge, "barge" )
	XT_FIELD( m_PerkInfoBomb, "bomb" )
	XT_FIELD( m_PerkInfoBlindfire3, "blindfire3" )
	XT_FIELD( m_PerkInfoBlindfire2, "blindfire2" )
	XT_FIELD( m_PerkInfoBlindfire1, "blindfire1" )
	XT_FIELD( m_PerkInfoRepair,	"repair" )
	XT_FIELD( m_PerkInfoShield,	"shield" )
	XT_FIELD( m_PerkInfoSuperShield, "superShield" )
	XT_FIELD( m_PerkInfoEmpty, "empty" )
	XT_FIELD( m_PerkInfoExtraBlindfire4, "extra_blindfire4" )
	XT_FIELD( m_PerkInfoExtraBlindfire3, "extra_blindfire3" )
	XT_FIELD( m_PerkInfoExtraBlindfire2, "extra_blindfire2" )
	XT_FIELD( m_PerkInfoExtraBlindfire1, "extra_blindfire1" )
XT_END_TYPE

namespace UI
{

DECLARE_DEBUG_BOOL( g_PerkRenderEnableDepthTest, "ui.perk.EnableDepthTest", false );
DECLARE_DEBUG_BOOL( g_PerkRenderCullBackface, "ui.perk.CullBackface", true );

DECLARE_DEBUG_FLOAT( g_PlacePerksBehindCarLerp, "ui.perk.BehindCarLerp", 0.6f, Debug::Range(0.0f, 1.0f));
DECLARE_DEBUG_FLOAT( g_PlacePerksBehindCarAcc, "ui.perk.BehindCarAcc", 0.4f, Debug::Range(0.0f, 120.0f));
DECLARE_DEBUG_FLOAT( g_PlacePerkBehindCarDamping, "ui.perk.BehindCarDamping", 0.0f, Debug::Range(0.0f, 1.0f));

DECLARE_DEBUG_FLOAT( g_PerkLightRadius, "ui.perk.PerkLightRadius", 1.0f, Debug::Range( 0.0f, 16.0f ) );
DECLARE_DEBUG_FLOAT( g_PerkLightIntensity, "ui.perk.PerkLightIntensity", 1.0f, Debug::Range( 0.0f, 16.0f ) );
DECLARE_DEBUG_FLOAT( g_PerkLightHeight, "ui.perk.LightHeight", -0.2f,  Debug::Range( -8.0f, 8.0f ) );

C_UIPerkInfo* C_PerkDisplayParameters::GetPerkInfo( const E_PICKUP_TYPE ePickupType, const S32 sAmount, const bool bHasExtraBoltMod )
{
	switch ( ePickupType )
	{
		case E_PICKUP_TYPE_NITROUS:				return &m_PerkInfoNitrous;
		case E_PICKUP_TYPE_SHOCK:				return &m_PerkInfoShock;
		case E_PICKUP_TYPE_SHUNT:				return &m_PerkInfoShunt;
		case E_PICKUP_TYPE_BARGE:				return &m_PerkInfoBarge;
		case E_PICKUP_TYPE_BOMB:				return &m_PerkInfoBomb;

		case E_PICKUP_TYPE_BLINDFIRE:
		{
			if ( bHasExtraBoltMod )
			{
					 if	( sAmount == 4 )			return &m_PerkInfoExtraBlindfire4;
				else if ( sAmount == 3 )			return &m_PerkInfoExtraBlindfire3;
				else if ( sAmount == 2 )			return &m_PerkInfoExtraBlindfire2;
				else if ( sAmount == 1 )			return &m_PerkInfoExtraBlindfire1;
				else								return &m_PerkInfoExtraBlindfire4;
			}
			else
			{
					 if	( sAmount == 3 )			return &m_PerkInfoBlindfire3;
				else if ( sAmount == 2 )			return &m_PerkInfoBlindfire2;
				else if ( sAmount == 1 )			return &m_PerkInfoBlindfire1;
				else								return &m_PerkInfoBlindfire3;
			}
		}

		case E_PICKUP_TYPE_REPAIR:				return &m_PerkInfoRepair;
		case E_PICKUP_TYPE_SHIELD:				return &m_PerkInfoShield;
		case E_PICKUP_TYPE_SUPER_SHIELD:		return &m_PerkInfoSuperShield;

		case E_PICKUP_TYPE_NONE:				return &m_PerkInfoEmpty;

		default:								return NULL;
	}
}

// -------
// UIPerks2
// -------
void UIPerks2::Init()
{
	m_ScaleAmount = 0.0f;
	m_ScaleSpeed = 1.0f;
	m_Padding = 0.0f;
	m_CurrentItem = -1;
	m_sLocalPlayerIndex = -1;
	m_pVehicleActor = NULL;
	m_pLogicActor = NULL;
	m_pPickupInfo = NULL;
	m_fPickupActivatedIn = 0.0f;
	m_LightVisibilityFlags = 0;

	m_bPickupActivated = false;
	m_LastGaspActive = false;
	m_ScramblerActive = false;
	m_bHasExtraBoltMod = false;

	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
	AddSubscriber( E_UiGameMsg_PickedupAPerk );
	AddSubscriber( E_UiGameMsg_PerkGifted );

	C_GadgetBase::Init();
}

void UIPerks2::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// Calculate width of gadget
	m_Size.y = m_ItemSize * m_ScaleAmount * m_DrawAspect;

	m_Items.Resize( GetNumSlots() );
	for( S32 i = 0; i < m_Items.Size(); i++ )
		m_Items[ i ].Init();

	m_Origin = m_Position;

	// Add light to scene
	if( m_AttachedToCar )
	{
		SceneGraph::C_SceneGraph *p = UI::GetGameWorld()->GetSceneGraph();
		if( p )
		{
			m_PointLightHandle = SceneGraph::AddPointLight( p );

			// Clear
			SceneGraph::C_PointLight* pLight = m_PointLightHandle.Object();
			if( pLight )
			{
				pLight->SetVisibilityFlags( 0 );
				pLight->SetFlags( 0xffffffff );
			}
		}
	}

	// Alternate effect index for selected icons
	m_SelectedRS = m_RenderState;
	m_SelectedRS.m_EffectIndex = (U8)m_SelectedEffectIndex;

	// resolve colours
	m_ColourMod1.SetByName( "Slot1" );
	m_ColourMod2.SetByName( "Slot2" );
	m_ColourMod3.SetByName( "Slot3" );
	m_ColourModSp.SetByName( "Support_4" );
}

S32 UIPerks2::GetNumSlots()
{
	if( m_sLocalPlayerIndex == -1 )
		return 0;

	S32 sVehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );
	if( sVehicleIndex == -1 )
		return 0;

	return GetLevelState().mCars[ sVehicleIndex ].mUpgrades.NumPickupSlots();

	//Game::C_LevelState& levelState = GetLevelState();
	//RaceBook::C_RaceBookProfile *rb_profile = RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)levelState.mCars[ sVehicleIndex ].mCharacterID);
	//if( !rb_profile )
	//	return 0;

	//return rb_profile->mShopUpgrades[E_RACEBOOK_SHOP_UPGRADES_ADDITIONAL_PERK_SLOT];
}

bool UIPerks2::CanDoubleDip()
{
	if( !m_pPickupInfo )
		return false;

	// Not enough pickups
	if( m_pPickupInfo->mPickupStore.Size() < 2 )
		return false;

	const E_PICKUP_TYPE current = m_pPickupInfo->mPickupStore[ m_pPickupInfo->mPickupStoreIndex ].mType;
	const int index = m_pPickupInfo->mPickupStoreIndex;

	if( index == m_pPickupInfo->mPickupStore.Size() - 1 )
	{
		// We're at the end, check the previous
		if( current == m_pPickupInfo->mPickupStore[ index - 1 ].mType )
			return true;
	}
	else
	{
		// Check next
		if( current == m_pPickupInfo->mPickupStore[ index + 1 ].mType )
			return true;
	}

	return false;
}

F32 UIPerks2::GetWidth()
{
	S32 num_slots = ( GetNumSlots() < m_Items.Size() ) ? GetNumSlots(): m_Items.Size();
	if( num_slots == 0 )
		return 0.0f;

	F32 size = 0.0f;
	for( S32 i = 0; i < num_slots; i++ )
		size += ( m_ItemSize + m_Padding ) * m_Items[ i ].m_Scale;

	return size;
}

C_UIPerkInfo* UIPerks2::GetPerkInfo( const E_PICKUP_TYPE ePickupType, const S32 sAmount, const bool bHasExtraBoltMod )
{
	if( !C_UiXtConfig::Instance() )
		return NULL;

	return C_UiXtConfig::Instance()->m_PerkDisplayParameters.GetPerkInfo( ePickupType, sAmount, bHasExtraBoltMod );
}

void UIPerks2::UpdateActivatingIn()
{
	if( m_bPickupActivated )
		return;

	if( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_Debug )// || GetLevelState().mPickupSpawnLevel != 0 )
	{
		C_UiTimelineMessage msg( "Unfold_Pickup", true, 0.0, true );
		MessageHandler( &msg );

		m_bPickupActivated = true;
		return;
	}

	Game::C_RaceLogicLayer *pRLCLayer = GetRaceLogicLayer();
	if( !pRLCLayer )
		return;

	// No message (probably come back from pause just unfold
	if( pRLCLayer->PowerupsActivated() && m_fPickupActivatedIn == 0.0f )
	{
		C_UiTimelineMessage msg( "Unfold_Pickup", true, 0.0, true );
		MessageHandler( &msg );

		m_bPickupActivated = true;
		return;
	}

	// Activating
	const float time = pRLCLayer->GetPerksActivatedIn();
	if(time <= 3.0f )
	{
		S32 msg_time = (S32)ceilf( time );
		S32 my_time =  (S32)ceilf( m_fPickupActivatedIn );
		
		// Crossed boundary?
		if( msg_time != my_time )
		{
			// Play sound
			if( m_sLocalPlayerIndex == 0 && m_AttachedToCar )
				UiInterface()->PlaySound( (msg_time == 0) ? E_UiSoundEffect_PerksGoActionIn : E_UiSoundEffect_PerksCountdownActionIn );

			if(msg_time == 0) // Finished
			{
				C_UiTimelineMessage msg( "Unfold_Pickup", true, 0.0, true );
				MessageHandler( &msg );

				m_bPickupActivated = true;
			}
			else
			{
				C_UiTimelineMessage msg( "Pulse_Pickup", true, 0.0 );
				MessageHandler( &msg );
			}
		}
	}

	m_fPickupActivatedIn = time;
}

void UIPerks2::FrameUpdate( )
{
	C_GadgetBase::FrameUpdate();

	if ( !m_Activated )
		return;

	if( m_sLocalPlayerIndex == -1 )
		return;

	UpdateActivatingIn();

	// Resize
	Game::C_LevelState* pLevelState	= &Game::C_Game::GetInstance()->GetLevelState();
	Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = pLevelState->mCars[m_sLocalPlayerIndex].mUpgrades.GetMultiplayerAbilities();	
	m_LastGaspActive = pAbilities->GetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Last_Gasp);
	m_ScramblerActive = pAbilities->IsVehicleScrambled();

	S32 selected = -1;
	if( m_pPickupInfo )
		selected = m_pPickupInfo->mPickupStoreIndex;

	const F32 ft = theFETimer.FrameTime();
	for( S32 i = 0; i < m_Items.Size(); i++ )
		m_Items[ i ].FrameUpdate( ft, m_ScaleSpeed, (i == selected) ? (F32)m_ScaleAmount : 1.0f );

	m_Size.x = GetWidth();
}

bool UIPerks2::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
		{
			if ( m_sLocalPlayerIndex != -1 )
				break;

			m_sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
			S32 sVehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );

			// check for extra bolt mod
			Btl::Vector< S32 >& loadouts = GetLevelState().mCars[ sVehicleIndex ].mLoadOuts;

			for ( S32 i = 0; i < loadouts.Size(); i++ )
			{
				if ( loadouts[ i ] == UI::E_MultiplayerVehicleAbilities_Boss_AdditionalBlindfireShots )
				{
					m_bHasExtraBoltMod = true;
					break;
				}
			}

			m_pLogicActor = GetVehicleLogicActor( sVehicleIndex );
			if( !m_pLogicActor )
				break;

			m_pVehicleActor = m_pLogicActor->GetGraphicalActor( sVehicleIndex );
			if( !m_pVehicleActor )
				break;

			m_pPickupInfo = &m_pLogicActor->GetPickupInfo();

			S32 sNumSlots = GetNumSlots();

			m_Items.Resize( sNumSlots );
			for( S32 i = 0; i < m_Items.Size(); i++ )
				m_Items[ i ].Init();

			if( m_AttachedToCar )
			{
				m_Panel = E_GameSpritePanel_VP0 + m_sLocalPlayerIndex;
#if PLATFORM_PC
				if( m_sLocalPlayerIndex == 0 )
				{	// [Blur2PC] own panel, re-placed at the car's render pose each frame (C_UiManager::PreRender)
					m_Panel = E_GameSpritePanel_VP0_CAR;
					m_Blur2PCBuilt = false;
					s_pBlur2PCCarGadget = this;
				}
#endif
				m_LightVisibilityFlags = Game::E_ViewTypeFlag_Main1 + m_sLocalPlayerIndex;

				// Unit distance
				F32 unitDist = 1.0f;
				Kanzen::HollywoodVehicleParams pHCam = m_pVehicleActor->GetHollywoodCameraParams(0);
				unitDist = pHCam.IdealDistanceNormal / 3.0f;

				m_Scale.x *= unitDist;
				m_Scale.y *= unitDist;

				// Set to vehicle bounds
				if( m_pVehicleActor->GetVehicleModel() && 
					m_pVehicleActor->GetVehicleModel()->GetModelResource() && 
					m_pVehicleActor->GetVehicleModel()->GetModelResource()->GetModelData() )
				{
					const Maths::C_AxisBox& box = m_pVehicleActor->GetVehicleModel()->GetModelResource()->GetModelData()->GetBounds();

//					m_Position.y += box.Min().Y();
					m_Position.z += box.Min().Z();
				}
			}

			m_GiftedAbility.Resize( sNumSlots );
			for ( S32 i = 0; i < m_GiftedAbility.Size(); i++ )
				m_GiftedAbility[ i ] = E_MultiplayerVehicleAbilities_None;

			break;
		}

		case E_UiGameMsg_PickedupAPerk:
		{
			if ( m_sLocalPlayerIndex == -1 )
				break;

			const C_UiGameMsg_StartedAPerk* pMsg = Messaging::MessageCast<C_UiGameMsg_StartedAPerk>(pMessage);

			S32 sVehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );

			if( pMsg && pMsg->mVehicleIndex == sVehicleIndex )
				AddPerk( pMsg->mType );

			break;
		}

		case E_UiGameMsg_PerkGifted:
		{
			if ( m_sLocalPlayerIndex == -1 )
				break;

			const C_UiGameMsg_PerkGifted* pMsg = Messaging::MessageCast< C_UiGameMsg_PerkGifted >( pMessage );

			S32 sVehicleIndex = GetPlayerVehicleIndex( m_sLocalPlayerIndex );

			if ( sVehicleIndex != pMsg->m_sVehicleIndex )
				break;

			if ( pMsg->m_sSlot >= 0 && pMsg->m_sSlot < m_GiftedAbility.Size() )
				m_GiftedAbility[ pMsg->m_sSlot ] = pMsg->m_eAbility;

			break;
		}

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIPerks2::AddPerk( const E_PICKUP_TYPE type )
{
}

void UIPerks2::RemovePerk()
{
	// Get active perks
	if( m_Items.Size() <= 0 || m_CurrentItem == -1 )
		return;
	
	m_Items[ m_CurrentItem ].m_Advance += ((m_ItemSize + m_Padding) * m_Items[ m_CurrentItem ].m_Scale);
	// Move scales down
	for( S32 i = m_CurrentItem; i < m_Items.Size()-1; i++ )
		m_Items[ i ].m_Scale = m_Items[ i + 1 ].m_Scale;

	// Zero out the last
	m_Items[ m_Items.Size()-1 ].m_Scale = 0.0f;
	m_CurrentItem--;
}

void UIPerks2::Render()
{
	C_GadgetBase::Render();

	if (m_fPickupActivatedIn > 3.0f || (m_fPickupActivatedIn == 0.0f && !m_bPickupActivated) )
		return;

	if( m_LastGaspActive || m_ScramblerActive )
		return;

	// Turn off light by default
	SceneGraph::C_PointLight* pLight = NULL;
	if( m_AttachedToCar )
	{
		pLight = m_PointLightHandle.Object();
		if( pLight )
			pLight->SetVisibilityFlags( 0 );
	}

	if ( !m_Activated || m_pVehicleActor == NULL || m_pLogicActor == NULL )
		return;

	// Local players only please
	if( m_sLocalPlayerIndex == -1 )
		return;

	AmaxCam::E_VFCamera camType = m_pVehicleActor->GetCameraType( m_sLocalPlayerIndex );
	bool isBumper( camType == AmaxCam::E_VFCamera_Bumper );

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

	// Show alternate types
	if( levelState.mNumViewports == 1 )
	{
		if( isBumper && m_AttachedToCar )
			return;

		if( !isBumper && !m_AttachedToCar )
			return;
	}

	if( m_AttachedToCar && Parent() )
	{
		// New code start
#if PLATFORM_PC
		// [Blur2PC] render interpolation: this runs per rendered frame, so follow the pose the car is drawn at
		const Maths::HMat43 carMatrix = m_pVehicleActor->Blur2PCRenderMatrix();
#else
		const Maths::HMat43 carMatrix = m_pLogicActor->GetWorldMatrix();
#endif
		const Maths::HVec3 idealDir = carMatrix.AZ();
		const Maths::HVec3 currDir = Maths::HMat33(m_PrevQuat).AZ();
		const Maths::HVec3 upDir = Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();
		Maths::HVec3 idealSideDir = idealDir ^ upDir;
		idealSideDir.Normalise();
		

		F32 DifX = fabsf(currDir.X() - idealDir.X());
		F32 DifZ = fabsf(currDir.Z() - idealDir.Z());

		F32 angle =  Maths::RadToDeg(atan2f(DifX, DifZ));
		
		angle = cos(angle);
		

		if(angle < -0.5f && PrevAngle > 0.5f)		
		{
			angle = 1.f + (1.f + angle);
		}
		else if(angle > 0.5f && PrevAngle < -0.5f)
		{
			angle = -1.f - (1.f - angle);
		}

		F32 adjustedDamping = ( 1.0f - powf( g_PlacePerkBehindCarDamping, ( (1.0f/60.0f) * 60.0f )));

		F32 dot = Maths::Clamped<F32>(Maths::DotProduct(idealDir,currDir),-1.0f,1.0f);
	
		F32 angleMag = acos( dot );
		if(DotProduct(currDir,idealSideDir)<0.0f)
		{
			angleMag *= -1.0f;
		}
		//printf("angle: %f dot: %f\n", angleMag, dot);
		F32 idealAngleMag = 0.0f;
		F32 newAngleMag = angle + (( idealAngleMag - angle) * adjustedDamping);
		F32 newAngleMagNormalised = 1.0f - Maths::Clamped<F32>( newAngleMag / angle, 0.0f,1.0f );

		Maths::C_Quaternion currRotation( carMatrix.Rotation() );
		Maths::C_Quaternion newRotation = m_PrevQuat.Slerp( currRotation, newAngleMagNormalised);

		PrevAngle = angle;

		m_PrevQuat = newRotation;
		s_prevIdealDir = idealDir;
		// New code end




#if 0
		Maths::C_Quaternion q( m_pLogicActor->GetWorldMatrix().Rotation() );

		static Maths::C_Quaternion m_PrevQuat;
		static Maths::C_Quaternion m_QuatVel;

		if ((q | m_PrevQuat) < 0.0f)
		{
			m_PrevQuat = -m_PrevQuat;
		}
	
		printf(" prev quat %f \n", (q.CosTheta()));
 
		m_QuatVel = (m_QuatVel * (F32)g_PlacePerksBehindCarLerp) + (q - m_PrevQuat);
	
		m_PrevQuat = (m_PrevQuat + (m_QuatVel * (F32)g_PlacePerksBehindCarAcc));
#endif

		Parent()->DirtySet( E_Dirty_Matrix, false );
		Parent()->m_Rotation.x = 45.0f;

		Maths::HMat43 parentMat = Parent()->Matrix();

		parentMat.Rotation( newRotation/*m_PrevQuat.Normalised()*/ );
		parentMat.AW( carMatrix.AW() );

		Parent()->SetMatrix(parentMat);
#if PLATFORM_PC
		m_Blur2PCBuiltMatrix = carMatrix;	// [Blur2PC]
		m_Blur2PCBuilt = true;
#endif

		// Push render state change
		gSimple2D.AddSprite( m_Layer,		m_Panel, E_SpriteType_SetRendersStateStack, 1 );

		// Culling
		gSimple2D.AddSprite( m_Layer,		m_Panel, E_SpriteType_SetCullMode, (g_PerkRenderCullBackface==true) ? Display::E_CullMode_Ccw : Display::E_CullMode_None );

		// Depth buffer
		if ( levelState.mNumViewports == 1 && g_PerkRenderEnableDepthTest )
		{
			C_Simple2D::C_StackedRenderState::C_DepthState depth;
			depth.function		= Display::E_CmpFunc_LessEqual;
			depth.write			= Display::E_Bool_False;
			depth.enabled		= Display::E_Bool_True;

			gSimple2D.AddSprite( m_Layer,		m_Panel, E_SpriteType_SetDepthBuffer, *((S32*)&depth) );
		}
	}

	// auto-transform
	C_UIAutoTransform transform( this );

	// Get active perks
	if( !m_pPickupInfo )
		return;

	const S32 num_pickups = m_pPickupInfo->mPickupStore.Size();
	m_CurrentItem = m_pPickupInfo->mPickupStoreIndex;

	// Get slots
	S32 num_slots = GetNumSlots();
	if( num_slots == 0 )
		return;

	bool can_switch = false;
	S32 valid_count = 0;
	for( S32 i = 0; i < num_pickups; i++ )
	{
		if( m_pPickupInfo->mPickupStore[ i ].mType != E_PICKUP_TYPE_NONE )
			valid_count++;

		if( valid_count > 1 )
		{
			can_switch = true;
			break;
		}
	}

	Maths::HVec3 pos( m_Position );
	pos.X( pos.X() - (m_Size.x * 0.5f) * m_FoldoutScale );
	Maths::HVec3 advance( m_ItemSize + m_Padding, 0.0f, 0.0f );
	Maths::HVec3 one_x(1.0f,0.0f,0.0f);
	Maths::HVec3 one_y(0.0f,1.0f,0.0f);

	for( S32 j = 0; j < num_slots; j++ )
	{
		S32 i = (m_pPickupInfo->mPickupStoreIndex == -1) ? j : (m_pPickupInfo->mPickupStoreIndex + j - 1);
		
#if 0
		if( i < 0 )
			i+= num_slots;
		else
		if( i >= num_slots )
			i -= num_slots;
#else
		i = j;
#endif

		if ( i > ( m_SlotEffect.count - 1 ) )
			break;

		const F32 scale = m_Items[ i ].m_Scale;
		const F32 drawSizeX = m_ItemSize * scale;
		const F32 drawSizeY = drawSizeX * m_DrawAspect;

		pos = pos + (advance * scale * m_FoldoutScale * 0.5f);

		pos.X( pos.X() + m_Items[ i ].m_Advance );

		C_UIPerkInfo* pInfo = NULL;
		bool bPickupNone = false;

		if ( i < num_pickups )
			pInfo = GetPerkInfo( m_pPickupInfo->mPickupStore[ i ].mType, m_pPickupInfo->mPickupStore[ i ].mNumShots, m_bHasExtraBoltMod );
		else
		{
			pInfo = GetPerkInfo( E_PICKUP_TYPE_NONE );
			bPickupNone = true;
		}

		if( pInfo )
		{
			S32 sTexIndex = pInfo->m_sTextureIndex;

			C_TextureParam* pParam = NULL;
			C_TextureParam* pParamInv = NULL;

			if ( sTexIndex < 100 )
			{
				pParam = &GetTextureParam( 0 );
				pParamInv = &GetTextureParam( 1 );
			}
			else
			{
				sTexIndex -= 100;
				pParam = &GetTextureParam( 6 );
				pParamInv = &GetTextureParam( 7 );
			}

			F32 alpha = m_AlphaInherit * m_SlotEffect[ i ].m_fAlpha;

			C_UISpriteColours colourFill;
			colourFill.Set( pInfo->m_ColourFill, alpha, m_IntensityInherit );

			C_UISpriteColours colourText;
			colourText.Set( pInfo->m_ColourText, m_AlphaInherit, m_IntensityInherit );

			F32 slot_off = (F32)i - ((F32)(num_slots-1) / 2.0f);
			F32 deg = slot_off * m_AngleStep * m_Bender;
			F32 ang_off = fabs(slot_off) * m_SlotStep * m_Bender;

			F32 vVal = 0.0f;
			UI::C_UISpriteColours spritecol = m_SpriteColours; // Point this at correct mod colour

			switch(m_GiftedAbility[ i ])
			{
			case E_MultiplayerVehicleAbilities_Front_Runner:
				{
					vVal = 0;
					spritecol = m_ColourMod1;
				}
				break;

			case E_MultiplayerVehicleAbilities_Bribe:
				{
					vVal = 1;
					spritecol = m_ColourMod3;
				}
				break;

			case E_MultiplayerVehicleAbilities_Safety_Net:
				{
					vVal = 2;
					spritecol = m_ColourMod2;
				}
				break;

			case E_MultiplayerVehicleAbilities_Drifter:
				{
					vVal = 3;
					spritecol = m_ColourMod1;
				}
				break;

			case E_MultiplayerVehicleAbilities_Boss_FansNitroGift:
				{
					vVal = 4;
					spritecol = m_ColourModSp;
				}
				break;

			default:
				break;
			}

			C_UIColour col = spritecol[0]; 

			// Multiply by size
			C_TextureParam& modTex = GetTextureParam( 8 );		
			vVal *= modTex.m_Size.v;

			// glow effect
			if ( ( m_SlotEffect[ i ].m_fBlobScale > 0.001f ) && ( m_SlotEffect[ i ].m_fBlobAlpha > 0.001f ) )
			{
				F32 fBlobSize = drawSizeX * m_SlotEffect[ i ].m_fBlobScale;

				col.A01( m_SlotEffect[ i ].m_fBlobAlpha );

				gSimple2D.AddSprite( m_Layer, pos + Maths::HVec3( 0.0f, 0.0f, ang_off ), fBlobSize, fBlobSize, col, m_Panel, E_SpriteType_Sprite, m_RenderState );
				gSimple2D.AddJustification( m_Justify );
				gSimple2D.AddRotation( 0, deg, 0 );
				gSimple2D.AddTexUVs( 0, GetTextureParam( 9 ) );
			}

			// mod effect
			if ( ( m_GiftedAbility[ i ] != E_MultiplayerVehicleAbilities_None ) && ( m_SlotEffect[ i ].m_fModScale > 0.001f ) && ( m_SlotEffect[ i ].m_fModAlpha > 0.001f ) )
			{
				F32 fModSize = drawSizeX * m_SlotEffect[ i ].m_fModScale;

				col.A01( m_SlotEffect[ i ].m_fModAlpha );
				
				gSimple2D.AddSprite( m_Layer, pos + Maths::HVec3( 0.0f, 0.0f, ang_off ), fModSize * 1.333f, fModSize, col, m_Panel, E_SpriteType_Sprite, m_RenderState );
				gSimple2D.AddJustification( m_Justify );
				gSimple2D.AddRotation( 0, deg, 0 );
			
				gSimple2D.AddTexUVs( 0, modTex.m_TextureID, modTex.m_Position.u, vVal, modTex.m_Size.u, modTex.m_Size.v );
			}

			// Icon
			const bool isSelected( i == m_pPickupInfo->mPickupStoreIndex && m_pPickupInfo->mPickupStore[ i ].mType != E_PICKUP_TYPE_NONE );
			F32 fSlotScale = m_SlotEffect[ i ].m_fScale;

			C_TextureParam* pParamRender = ( isSelected ? pParam : pParamInv );

			gSimple2D.AddSprite( m_Layer, pos + Maths::HVec3( 0.0f, 0.0f, ang_off ), drawSizeX * fSlotScale, drawSizeY * fSlotScale, colourFill, m_Panel, E_SpriteType_Sprite, bPickupNone ? m_SelectedRS : ( isSelected ? m_RenderState : m_SelectedRS ) );
			gSimple2D.AddJustification( m_Justify );
			gSimple2D.AddRotation( 0, deg, 0 );

			if ( bPickupNone || m_pPickupInfo->mPickupStore[ i ].mType == E_PICKUP_TYPE_NONE )
				gSimple2D.AddTexUVs( 0, GetTextureParam( 5 ) );
			else
				gSimple2D.AddTexUVs( 0, pParamRender->m_TextureID, pParamRender->m_Position.u, pParamRender->m_Position.v + (pParamRender->m_Size.v * (F32)sTexIndex), pParamRender->m_Size.u, pParamRender->m_Size.v );

			// Current selected
			if( isSelected )
			{
				// Update the light
				if( m_AttachedToCar && pLight )
				{
					const Maths::HVec3 offset( 0.0f, (F32)g_PerkLightHeight, 0.0f );

					pLight->SetVisibilityFlags( m_LightVisibilityFlags );
					pLight->SetDimensions( Matrix().AW() + offset, (F32)g_PerkLightRadius );
					pLight->SetColour( Maths::C_RGBA_F32( pInfo->m_ColourText[0].R01() * (F32)g_PerkLightIntensity, pInfo->m_ColourText[0].G01() * (F32)g_PerkLightIntensity, pInfo->m_ColourText[0].B01() * (F32)g_PerkLightIntensity, 1.0f ) );
				}
			}
		}
		pos = pos + (advance * scale * m_FoldoutScale * 0.5f);
	}

	// Pop render state change
	if( m_AttachedToCar )
		gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetRendersStateStack, 0 );
}

#if PLATFORM_PC
UIPerks2* UIPerks2::s_pBlur2PCCarGadget = NULL;	// [Blur2PC]

bool UIPerks2::Blur2PCCarPanelDelta( Maths::HMat43& delta )
{
	UIPerks2* p = s_pBlur2PCCarGadget;
	if( !p || !p->m_Blur2PCBuilt || !p->m_pVehicleActor )
		return false;
	delta = p->m_Blur2PCBuiltMatrix.InverseRotTrans() * p->m_pVehicleActor->Blur2PCRenderMatrix();
	return true;
}
#endif

void UIPerks2::Exit( )
{
#if PLATFORM_PC
	if( s_pBlur2PCCarGadget == this )	// [Blur2PC]
		s_pBlur2PCCarGadget = NULL;
#endif
	// Remove light
	if( m_AttachedToCar )
	{
		SceneGraph::C_SceneGraph *p = UI::GetGameWorld()->GetSceneGraph();
		if( p )
		{
			SceneGraph::RemovePointLight(p, m_PointLightHandle);
			m_PointLightHandle = SceneGraph::C_PointLightHandle();
		}
	}
}


}; // namespace UI
