#include "stdafx.h"
#include <GameLogic/Structures/LevelState.h>	// [Blur2PC]

#include <ui/Graphics/SimpleTimer.h>

#include "UILoadoutController.h"
#include "UiHelpers.h"
#include "UIMessages.h"
#include <Ui/UiInterface.h>
#include "Ui/SoundEffects.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::UILoadoutController, "UILoadoutController" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_AbilityNames, "ability_colours" )
	XT_FIELD( m_fDeactiveWaitTime, "deactive_wait_time" )
XT_END_TYPE

namespace UI
{

// --------
// UILoadoutController
// --------
void UILoadoutController::Init()
{
	m_sVehicleIndex = 0;
	m_fDeactiveWaitTime = 0;

	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiGameMsg_SetLocalPlayerIndex );
}

void UILoadoutController::PostInit()
{
	// Find children
	ButtonArray childrenMe;
	FindChildren( &childrenMe, true );

	Util::C_String tmp;
	for( U32 i = 0; i < E_MultiplayerAbilityCategory_Max; i++ )
	{
		tmp.Printf("Mirror_LoadOut%d", i + 1 );
		m_pGadget[ i ] = ArrayFindByName( childrenMe, tmp );

		// Type check
		if( !m_pGadget[ i ] || m_pGadget[ i ]->GetType() != E_UIButtonType_StaticGraphic )
		{
			LOGERROR( g_Log, "Loadout gadget %d invalid", i );
			m_Activated = false;
			return;
		}

		m_pGadget[ i ]->Activate( false );
	}

	//for the laser sight mod
	m_LaserSightDot = gButtons.FindByName("Mirror_LaserSight");
}

void UILoadoutController::SetupGadgets( S32 sPlayerVehicleIndex )
{
	Btl::Vector<int>& loadOuts = GetLevelState().mCars[ sPlayerVehicleIndex ].mLoadOuts;

	S32 count = 0;
	for ( S32 i = 0; i < loadOuts.Size(); i++ )
	{
		if( count >= E_MultiplayerAbilityCategory_Max )
		{
			LOGWARN( g_Log, DBSTR " : More active loadouts than gadgets" );
			break;
		}

		C_GadgetBase* pObject = m_pGadget[ count ];

		if ( loadOuts[ i ] == E_MultiplayerVehicleAbilities_None )
			continue;

		const MultiplayerAbilityData* pAbility = C_MultiplayerData::Instance().FindAbility( (E_MultiplayerVehicleAbilities)loadOuts[ i ] );
		if ( !pAbility )
			continue;

		// Set texture
		if( pObject->m_TextureParams.count )
		{
			pObject->m_TextureParams[ 0 ].m_Position.u = pObject->m_TextureParams[ 0 ].m_Size.u * pAbility->mAbilityId;
		}

		// set colours
		if( m_AbilityNames.count > pAbility->mAbilityCategory )
		{
			pObject->SetColourStyle( m_AbilityNames[ pAbility->mAbilityCategory ] );
			pObject->ResolveInheritedColours();
			pObject->DirtySet( C_GadgetBaseData::E_Dirty_Colour, true );
		}

		pObject->Activate( true );
		count++;
	}

	m_sVehicleIndex = sPlayerVehicleIndex;

	for ( S32 i = 0; i < E_MultiplayerAbilityCategory_Max; i++ )
	{
		m_AbilitySlotActive[ i ] = false;
		m_AbilitySlotDeactiveTimer[ i ] = 0.0f;
	}
}

bool UILoadoutController::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiGameMsg_SetLocalPlayerIndex:
			{
				S32 sLocalPlayerIndex = (S32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
				SetupGadgets( GetPlayerVehicleIndex( sLocalPlayerIndex ) );
			}
			break;

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}


void UILoadoutController::FrameUpdate()
{
	if ( !m_Activated )
		return;

	Game::C_LevelState& levelState = GetLevelState();

	Btl::Vector< S32 >& loadOuts = levelState.mCars[ m_sVehicleIndex ].mLoadOuts;
	Game::C_LevelStateMultiplayerVehicleAbilities* pAbilities = levelState.mCars[ m_sVehicleIndex ].mUpgrades.GetMultiplayerAbilities();

	if ( !pAbilities )
		return;

	F32 fFrameTime = theFETimer.FrameTime();

	S32 count = 0;
	for ( S32 i = 0; i < loadOuts.Size(); i++ )
	{
		if ( loadOuts[ i ] == E_MultiplayerVehicleAbilities_None )
			continue;

		if ( count >= E_MultiplayerAbilityCategory_Max )
			continue;

		if ( !m_pGadget[ count ] )
			continue;

		bool bAbilityActive = pAbilities->GetAbilityFiring( (E_MultiplayerVehicleAbilities)loadOuts[ i ] );

		if ( bAbilityActive )
		{
			if ( !m_AbilitySlotActive[ count ] )
			{
				m_AbilitySlotActive[ count ] = true;

				// We 'Pulse' the ECM icon .. the rest we just switch on
				if((E_MultiplayerVehicleAbilities)loadOuts[ i ] == E_MultiplayerVehicleAbilities_ECM) // TODO: Set this as a flag on the ability in sorcery if we get any more that want to pulse!
				{
					m_pGadget[ count ]->SendPrivateTimelineMessaage( "ability_active_pulse", true, 0.0f );
				}
				else
				{
					m_pGadget[ count ]->SendPrivateTimelineMessaage( "ability_active", true, 0.0f );
				}

				//animate differing aspects of the UI depending on what our mod has done
				bool sfxPlayed = false;
				switch( loadOuts[ i ] )
				{
					//health modifying abilities
					case E_MultiplayerVehicleAbilities_Shielded_Boosters:
					case E_MultiplayerVehicleAbilities_Vampiric_Wreck:
					{
						UiInterface()->PlaySound( E_UiSoundEffect_GiftHealthActionIn );
						sfxPlayed = true;

						UI::C_UiTimelineMessage tlm( "ability_active_health", true, 0.0f );
						_UI_MESSAGE_FULL( tlm );
						break;
					}

					//abilities that gift you a pickup
					case E_MultiplayerVehicleAbilities_Front_Runner:
					case E_MultiplayerVehicleAbilities_Safety_Net:
					case E_MultiplayerVehicleAbilities_Bribe:
					case E_MultiplayerVehicleAbilities_Drifter:
					case E_MultiplayerVehicleAbilities_Boss_FansNitroGift:
					{
						C_UiGameMsg_PerkGifted giftMsg( m_sVehicleIndex, (E_MultiplayerVehicleAbilities)loadOuts[ i ] );
						
						UiInterface()->PlaySound( E_UiSoundEffect_GiftPowerUpActionIn );
						sfxPlayed = true;

						//we check which slot has been gifted into and effect that slot
						if( pAbilities->HasPickupSlotBeenGiftedInto( 1 ) )
						{
							giftMsg.m_sSlot = 0;
							UI::C_UiTimelineMessage tlm( "ability_active_pickup_slot1", true, 0.0f );
							_UI_MESSAGE_FULL( tlm );
						}
						if( pAbilities->HasPickupSlotBeenGiftedInto( 2 ) )
						{
							giftMsg.m_sSlot = 1;
							UI::C_UiTimelineMessage tlm( "ability_active_pickup_slot2", true, 0.0f );
							_UI_MESSAGE_FULL( tlm );
						}
						if( pAbilities->HasPickupSlotBeenGiftedInto( 3 ) )
						{
							giftMsg.m_sSlot = 2;
							UI::C_UiTimelineMessage tlm( "ability_active_pickup_slot3", true, 0.0f );
							_UI_MESSAGE_FULL( tlm );
						}
						
						_UI_MESSAGE_FULL( giftMsg );

						break;
					}

					case E_MultiplayerVehicleAbilities_Decoy_Drop:
					{
						//UiInterface()->PlaySound( E_UiSoundEffect_IronFistActivateActionIn );
						sfxPlayed = true;
						break;
					}

					case E_MultiplayerVehicleAbilities_Iron_Fist:
					{
						UiInterface()->PlaySound( E_UiSoundEffect_IronFistActivateActionIn );
						sfxPlayed = true;
						break;
					}

					case E_MultiplayerVehicleAbilities_Laser_Sight:
					{
						if(m_LaserSightDot)
							m_LaserSightDot->Activate(true);
						break;
					}
				}
				
				// If we have played a sound effect already (e.g. for gifting a perk), this need not be played
				if(!sfxPlayed)
				{
					switch ( count )
					{
						case 0:		UiInterface()->PlaySound( E_UiSoundEffect_ModUse01ActionIn );		break;
						case 1:		UiInterface()->PlaySound( E_UiSoundEffect_ModUse02ActionIn );		break;
						case 2:		UiInterface()->PlaySound( E_UiSoundEffect_ModUse03ActionIn );		break;
						case 3:		UiInterface()->PlaySound( E_UiSoundEffect_ModUse01ActionIn );		break;			// TODO - need new sound
						default:																		break;
					}
				}
			}

			m_AbilitySlotDeactiveTimer[ count ] = m_fDeactiveWaitTime;
		}
		else
		if ( !bAbilityActive && m_AbilitySlotActive[ count ] )
		{
			m_AbilitySlotActive[ count ] = false;
			if(loadOuts[ i ] == E_MultiplayerVehicleAbilities_Laser_Sight && m_LaserSightDot)
				m_LaserSightDot->Activate(false);
		}

		if ( !m_AbilitySlotActive[ count ] && ( m_AbilitySlotDeactiveTimer[ count ] != 0.0f ) )
		{
			m_AbilitySlotDeactiveTimer[ count ] -= fFrameTime;

			if ( m_AbilitySlotDeactiveTimer[ count ] < 0.0f || pAbilities->IsAbilityIconInstantOff((E_MultiplayerVehicleAbilities)loadOuts[ i ]) )
			{
				m_AbilitySlotDeactiveTimer[ count ] = 0.0f;
				m_pGadget[ count ]->SendPrivateTimelineMessaage( "ability_active", false );
				m_pGadget[ count ]->SendPrivateTimelineMessaage( "ability_active_pulse", false );
			}
		}

		count++;
	}

	pAbilities->ResetInstantAbilityFiringBits();

}

}; // namespace UI
