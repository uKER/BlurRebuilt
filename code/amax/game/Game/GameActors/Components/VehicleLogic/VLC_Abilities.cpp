#include "stdafx.h"
#include <Game\GameActors\Components\VehicleLogic\VLC_Abilities.h>

#include <GameLogic\Structures\LevelState.h>
#include <GameLogic\Structures\LevelStateMultiplayerAbilities.h>
#include <GameLogic\Abilities\MultiplayerAbility.h>

#include <GameApp.h>
#include <Game/game.h>
#include <GameLogic\Structures\VehicleScoringInfo.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/GameData/MultiplayerData.h>
#include <Game/Messaging/PickupActorMessages.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

Debug::Bool g_SilentRunningForceAbilityActive("Multiplayer.Upgrades.SilentRunning.ForceAbilityActive", false);
Debug::Bool g_SilentRunningLoadNewGraph("Multiplayer.Upgrades.SilentRunning.LoadNewGraph", false);
Debug::Bool g_SilentRunningForceUnCloak("Multiplayer.Upgrades.SilentRunning.ForceUnCloak", false);
Debug::Float g_SilentRunningForceUnCloakWaitToCloakTimer("Multiplayer.Upgrades.SilentRunning.ForceUnCloakWaitToCloakTimer", 2.f, Debug::Range(0.5f, 5.f));

Debug::Bool g_SilentRunningForceRaceStarted("Multiplayer.Upgrades.SilentRunning.ForceRaceStarted", false);

Debug::Float g_SilentRunningEffectScaleX("Multiplayer.Upgrades.SilentRunning.ScaleTestX", 0.9f);
Debug::Float g_SilentRunningEffectScaleY("Multiplayer.Upgrades.SilentRunning.ScaleTestY", 1.0f);
Debug::Float g_SilentRunningEffectScaleZ("Multiplayer.Upgrades.SilentRunning.ScaleTestZ", 0.45f);

namespace Components
{
	Curve::C_HermiteCurve C_VehicleComponent_Abilities::sCloakingAlphaGraph;
	Curve::C_HermiteCurve C_VehicleComponent_Abilities::sUnCloakingAlphaGraph;

	const float C_VehicleComponent_Abilities::SILENT_RUNNING_A_REALLY_REALLY_REALLY_REALLY_LONG_TIME = 86400.f; //24hrs

	C_VehicleComponent_Abilities::C_VehicleComponent_Abilities() :	mVehicleIndex(-1), 
																	m_pLevelState(0),  
																	m_pSilentRunningVehicleAbility(0), 
																	m_pRLC(0), 
																	m_pConfig(0), 
																	m_pSilentRunningSFX(0),
																	mSilentRunningSFXId(0),
																	mSilentRunningSFXEnabled(true),
																	m_pVehicleLogicActor(0),
																	m_pVehicleGraphicalActor(0)
	{
	}

	bool C_VehicleComponent_Abilities::CreateImpl()
	{
		const I_VehicleComponent_Abilities_CreateInfo* pCreationData = static_cast<const I_VehicleComponent_Abilities_CreateInfo *>(GetCreator().GetData());
		BIZ_ASSERT_MSG(pCreationData, "C_VehicleComponent_Abilities::CreateImpl() > Couldn't get a pointer to the creation data of VLC_Abilities");

		mActorHandle = pCreationData->mParentActorHandle;
		mVehicleIndex = pCreationData->mVehicleIndex;
		m_pLevelState = pCreationData->m_pLevelState;
		m_pScoringInfo = pCreationData->m_pScoringInfo;
		m_pRLC = pCreationData->m_pRLC;
		m_pConfig = pCreationData->m_pConfig;
		m_pSilentRunningAbility = pCreationData->m_pSilentRunningAbility;
		m_pXtAmaxVehicle = pCreationData->m_pXtAmaxVehicle;

		m_pVehicleLogicActor = pCreationData->m_pVehicleLogicActor;
		BIZ_ASSERT_MSG(m_pVehicleLogicActor, "C_VehicleComponent_Abilities::CreateImpl() > Couldn't get a pointer to the vehicle logic actor");

		m_pVehicleGraphicalActor = pCreationData->m_pVehicleGraphicalActor;
		BIZ_ASSERT_MSG(m_pVehicleGraphicalActor, "C_VehicleComponent_Abilities::CreateImpl() > Couldn't get a pointer to the vehicle graphical actor");

		//Cache pointers to abilities
		m_pMultiplayerAbilities = m_pLevelState->mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
		m_pSilentRunningVehicleAbility = m_pMultiplayerAbilities->GetAbility( UI::E_MultiplayerVehicleAbilities_Silent_Running );
		m_pFansGiftNitroAbility = m_pMultiplayerAbilities->GetAbility( UI::E_MultiplayerVehicleAbilities_Boss_FansNitroGift );

		//Setup listeners
		AddSubscriber( E_VehicleLogicMsg_FiredPerk );
		AddSubscriber( E_VehicleLogicMsg_HitByPerk );
		AddSubscriber( Game::E_GameflowMessage_RaceStarted );
		AddSubscriber( Game::E_GameLogicMessage_GameplayImpact);
		AddSubscriber( E_VehicleLogicMsg_HOGReset );
		AddSubscriber( E_VehicleLogicMsg_DecoyPickupDropOnTrack );

		//
		Initialise();
		
		return true;
	}

	void C_VehicleComponent_Abilities::Initialise()
	{
		mWaitToCloakTimer = 0.f;
		mNumFanNitroAwarded = 0;
		mRaceStarted = false;

		//Setup the silent running SFX
		InitialiseSilentRunningSFX();

		//We don't require dynamic updating of the curve if in master build, only needs doing once
		UpdateCloakingAlphaGraphs();
	}

	void C_VehicleComponent_Abilities::InitialiseSilentRunningSFX()
	{
		BIZ_ASSERT_MSG(!m_pSilentRunningSFX, "C_VehicleComponent_Abilities::InitialiseSilentRunningSFX() > The cache of the silent running SFX definition isn't NULL");
		m_pSilentRunningSFX = (XtArray<RLPickupSpecialFX> *)&m_pSilentRunningAbility->GetSpecialFX();
		BIZ_ASSERT_MSG(m_pSilentRunningSFX, "C_VehicleComponent_Abilities::InitialiseSilentRunningSFX() > The cache of the silent running SFX definition is NULL");
	}

	const bool C_VehicleComponent_Abilities::GetSilentRunningSFXEnabled() const
	{
		return mSilentRunningSFXEnabled && mRaceStarted;
	}

	void C_VehicleComponent_Abilities::SetSilentRunningSFXEnabled(const bool silentRunningSFXEnabled)
	{
		mSilentRunningSFXEnabled = silentRunningSFXEnabled;
		if(silentRunningSFXEnabled)
		{
			//We've just reenabled silent running we'll treat this like a hand of god reset
			HandleHandOfGodReset();
		}
	}

	void C_VehicleComponent_Abilities::EmergencyKillSilentRunningSFX()
	{
		//Kill the existing one instantly
		Game::C_InGameSpecialFXManager::Instance().DestroyPerkForced(mSilentRunningSFXId);
		mSilentRunningSFXId = -1;

		//Make sure it don't come back none neither
		mRaceStarted = false;
		SetSilentRunningSFXEnabled(false);
	}

	void C_VehicleComponent_Abilities::Update()
	{
		PERF_EVENT("VehicleComponent.Abilities");

#ifndef CFG_GOLD
		UpdateDebugFunctionality();
#endif
		
		const float elapsedTime = C_GameApp::GetInstance()->GetTimeStep();
		UpdateTimers(elapsedTime);		

		if( m_pSilentRunningVehicleAbility->GetActive() || g_SilentRunningForceAbilityActive )
		{
			UpdateSilentRunning();
		}

		// Will be nice to move this to a message when PK has unified the scoring messages
		if( m_pFansGiftNitroAbility->GetActive() )
		{
			UpdateFansNitroGift();
		}

		m_pMultiplayerAbilities->Update(elapsedTime, mVehicleIndex);
	}

#ifndef CFG_GOLD
	void C_VehicleComponent_Abilities::UpdateDebugFunctionality()
	{
		if(g_SilentRunningLoadNewGraph)
		{
			UpdateCloakingAlphaGraphs();

			g_SilentRunningLoadNewGraph = false;
		}

		if(g_SilentRunningForceUnCloak)
		{
			mWaitToCloakTimer = g_SilentRunningForceUnCloakWaitToCloakTimer;

			g_SilentRunningForceUnCloak = false;
		}

		if(g_SilentRunningForceRaceStarted)
		{
			mRaceStarted = true;
			g_SilentRunningForceRaceStarted = false;
		}
	}
#endif

	void C_VehicleComponent_Abilities::UpdateTimers(const float elapsedTime)
	{
		
		if((mWaitToCloakTimer - elapsedTime) < 0.f)
		{
			mWaitToCloakTimer = 0.f;
		}
		else
		{
			mWaitToCloakTimer -= elapsedTime;
		}
	}

	const bool C_VehicleComponent_Abilities::ShouldBeSilentlyRunning() const
	{
		bool shouldBeCloaked = false;
		if(mWaitToCloakTimer <= 0.f && mRaceStarted)
		{
			shouldBeCloaked = mSilentRunningSFXEnabled;
		}

		return shouldBeCloaked;
	}

	void C_VehicleComponent_Abilities::UpdateSilentRunning(const bool syncAcrossNetwork)
	{
		bool cloaked = m_pVehicleLogicActor->IsCloaked();
		const bool shouldBeCloaked = ShouldBeSilentlyRunning();


		if(cloaked && !shouldBeCloaked)
		{
			//Silent running powering down
			if(sUnCloakingAlphaGraph.Valid())
			{
				m_pVehicleLogicActor->DisEngageCloak(&sUnCloakingAlphaGraph);
			}
			else
			{
				m_pVehicleLogicActor->DisEngageCloak();
			}

			//This is getting sent across the network so we don't want to send an update every frame
			if(syncAcrossNetwork)
			{
				m_pMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Silent_Running, false);
				UpdateNetworkForSilentRunning(false);
			}
		}
		else if(!cloaked && shouldBeCloaked)
		{
			//Silent running powering up
			if(sCloakingAlphaGraph.Valid())
			{
				m_pVehicleLogicActor->EngageCloak(&sCloakingAlphaGraph);
			}
			else
			{
				m_pVehicleLogicActor->EngageCloak();
			}

			if(syncAcrossNetwork)
			{
				//This is getting sent across the network so we don't want to send an update every frame
				m_pMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Silent_Running, true);
				UpdateNetworkForSilentRunning(true);
			}
		}
	}

	void C_VehicleComponent_Abilities::SetDesiredVehicleAlphas()
	{
		//For local vehicles we want the car to entirely disappear but everything else to be fully visible
		m_pVehicleGraphicalActor->SetAlpha(E_VAC_TYPE_ALL, 1.f, E_VAC_TARGET_TYPE_DESIRED);

		//Set the vehicle to disappear completely
		m_pVehicleGraphicalActor->SetAlpha(E_VAC_TYPE_VEHICLE, 0.f, E_VAC_TARGET_TYPE_DESIRED);
	}

	void C_VehicleComponent_Abilities::StartSilentRunningSFX()
	{
		if(!mSilentRunningSFXEnabled)
			return;

		SetDesiredVehicleAlphas();

		BIZ_ASSERT_MSG(m_pSilentRunningSFX, "C_VehicleComponent_Abilities::StartSilentRunningSFX() > m_pSilentRunningSFX is NULL");

		Game::Actors::C_VehicleActor* pVehicleGraphicalActor = m_pRLC->GetVehicleActor(mVehicleIndex);
		BIZ_ASSERT_MSG(pVehicleGraphicalActor, "C_VehicleComponent_Abilities::StartSilentRunningSFX() > Couldn't get a pointer to the graphical vehicle actor");
		
		Maths::HVec3 offset;
		Maths::HVec3 extents = pVehicleGraphicalActor->GetExtents();
		Maths::HVec3 scaleTest(	g_SilentRunningEffectScaleX * extents.X() * (m_pXtAmaxVehicle->m_ShieldSizeModifier.vx ? m_pXtAmaxVehicle->m_ShieldSizeModifier.vx : 1.f), 
								g_SilentRunningEffectScaleY * extents.Y() * (m_pXtAmaxVehicle->m_ShieldSizeModifier.vy ? m_pXtAmaxVehicle->m_ShieldSizeModifier.vy : 1.f),
								g_SilentRunningEffectScaleZ * extents.Z() * (m_pXtAmaxVehicle->m_ShieldSizeModifier.vz ? m_pXtAmaxVehicle->m_ShieldSizeModifier.vz : 1.f) );


		mSilentRunningSFXId = Game::C_PickupsManager::Instance().AllocateUniqueId(mVehicleIndex);
		Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX( *m_pSilentRunningSFX, 
																	pVehicleGraphicalActor->GetMatrix(), 
																	mVehicleIndex, 
																	-1, 
																	mSilentRunningSFXId, 
																	Game::E_InGameSpecialFX_Trigger_Firing,
																	SILENT_RUNNING_A_REALLY_REALLY_REALLY_REALLY_LONG_TIME,
																	scaleTest);

	}

	void C_VehicleComponent_Abilities::FinishSilentRunningSFX()
	{
		Game::C_InGameSpecialFXManager::Instance().SetState(mSilentRunningSFXId, Game::E_InGameSpecialFX_Mode_Outro);
		mSilentRunningSFXId = -1;
	}

	void C_VehicleComponent_Abilities::UpdateNetworkForSilentRunning(const bool firing)
	{
		C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Add_SilentRunningEvent(mVehicleIndex, firing);
	}

	void C_VehicleComponent_Abilities::UpdateFansNitroGift()
	{
		int currentScore	= m_pVehicleLogicActor->GetScoring().mFinalScoreForUI;
		int fansReq			= (int)m_pFansGiftNitroAbility->GetValue();
		int currAwardedFans	= fansReq * (mNumFanNitroAwarded + 1);

		if(currentScore >= currAwardedFans && m_pFansGiftNitroAbility && m_pFansGiftNitroAbility->GetActive())
		{
			m_pMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Boss_FansNitroGift, true, false, mVehicleIndex);
			m_pMultiplayerAbilities->SetPickupSlotGiftedInto(m_pVehicleLogicActor->GetPickupInfo().mPickupStore.Size() + 1);

			Game::Actors::C_GrantPickupMsg grantNitrousMsg(mVehicleIndex, E_PICKUP_TYPE_NITROUS);
			m_pVehicleLogicActor->SendMessageToComponent(E_IF_VehicleComponent_Pickups, &grantNitrousMsg);

			mNumFanNitroAwarded++;
		}
	}

	void C_VehicleComponent_Abilities::HandleImpactMessage(const Game::C_GameplayImpactMsg *pGameplayImpactMsg)
	{
		//Work out if this is an intra vehicular impact or not
		if(pGameplayImpactMsg->mOtherVehicleIdx == INVALID_VEHICLE_IDX)
		{
			HandleEnvironmentHit();
		}
		else
		{
			HandleVehicleHit();
		}
	}

	void C_VehicleComponent_Abilities::HandleFiredPowerUp()
	{
		mWaitToCloakTimer = m_pConfig->m_SilentRunningModSettings.m_PostPowerUpFireUnCloakTime;
	}

	void C_VehicleComponent_Abilities::HandleHitByPowerUp()
	{
		mWaitToCloakTimer = m_pConfig->m_SilentRunningModSettings.m_PostPowerUpHitUnCloakTime;
	}

	void C_VehicleComponent_Abilities::HandleRaceStarted()
	{
		mWaitToCloakTimer = m_pConfig->m_SilentRunningModSettings.m_PostRaceStartUnCloakTime;
	}

	void C_VehicleComponent_Abilities::HandleEnvironmentHit()
	{
		mWaitToCloakTimer = m_pConfig->m_SilentRunningModSettings.m_PostEnvironmentHitUnCloakTime;
	}

	void C_VehicleComponent_Abilities::HandleVehicleHit()
	{
		mWaitToCloakTimer = m_pConfig->m_SilentRunningModSettings.m_PostVehicularHitUnCloakTime;
	}

	void C_VehicleComponent_Abilities::HandleHandOfGodReset()
	{
		//We don't have a specific timer for this...yet so I'm going to use the same one as the start of the race
		mWaitToCloakTimer = m_pConfig->m_SilentRunningModSettings.m_PostRaceStartUnCloakTime;
	}

	void C_VehicleComponent_Abilities::HandleDecoyDrop()
	{
		//We don't have a specific timer for this...yet so I'm going to use the same as when we fire a power up
		mWaitToCloakTimer = m_pConfig->m_SilentRunningModSettings.m_PostPowerUpFireUnCloakTime;
	}


	//Update the cloaking alpha curve
	void C_VehicleComponent_Abilities::UpdateCloakingAlphaGraphs()
	{
		const SilentRunningModSettings& settings = C_MultiplayerData::Instance().m_pConfig->m_SilentRunningModSettings;

		sCloakingAlphaGraph.Clear();
		sUnCloakingAlphaGraph.Clear();

		UpdateAlphaGraph(settings, sCloakingAlphaGraph);
		UpdateAlphaGraph(settings, sUnCloakingAlphaGraph);
	}

	//Update the passed in alpha graph
	void C_VehicleComponent_Abilities::UpdateAlphaGraph(const SilentRunningModSettings& settings, Curve::C_HermiteCurve& alphaGraph)
	{
		SpecialFX::C_HermiteKeyFrame keyFrame;

		XtArray<SpecialFX::C_HermiteKeyFrame> graph = settings.m_AlphaGraphs.m_CloakingAlphaGraph.m_AlphaGraph;
		for(int keyFrameIndex = 0; keyFrameIndex < graph.count; keyFrameIndex++)
		{
			keyFrame = graph[keyFrameIndex];

			alphaGraph.AddKeyFrame( keyFrame.mTime, 
									keyFrame.mValue, 
									keyFrame.mInTangent, 
									keyFrame.mOutTangent);
		}
	}


	bool C_VehicleComponent_Abilities::OnMessage(const Messaging::C_MessageBase *pMessage) 
	{
		PERF_EVENT("C_VehicleComponent.Abilities.MessageHandler");

		switch(pMessage->Type())
		{
		case E_VehicleLogicMsg_FiredPerk:
			{
				const C_VehicleLogicMsg_FiredPerkMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_FiredPerkMsg>(pMessage);
				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					HandleFiredPowerUp();
				}
				return true;
			}

		case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);
				if( pMsg->mVehicleIndex == mVehicleIndex )
				{
					HandleHitByPowerUp();
				}
				return true;
			}

		case Game::E_GameflowMessage_RaceStarted:
			{
				if(!mRaceStarted)
				{
					mRaceStarted = true;
					HandleRaceStarted();
				}
				return true;
			}
		case Game::E_GameLogicMessage_GameplayImpact:
			{
				const Game::C_GameplayImpactMsg* pGameplayImpactMsg	= Messaging::MessageCast<Game::C_GameplayImpactMsg>(pMessage);
				if( pGameplayImpactMsg->mVehicleIdx == mVehicleIndex || pGameplayImpactMsg->mOtherVehicleIdx == mVehicleIndex)
				{
					HandleImpactMessage(pGameplayImpactMsg);
				}
				return true;
			}

		case E_VehicleLogicMsg_HOGReset:
			{
				const C_VehicleLogicMsg_HoGResetMsg* pHoGResetMsg = Messaging::MessageCast<C_VehicleLogicMsg_HoGResetMsg>(pMessage);
				if(pHoGResetMsg->mVehicleIndex == mVehicleIndex)
				{
					HandleHandOfGodReset();
				}
				return true;
			}

		case E_VehicleLogicMsg_DecoyPickupDropOnTrack:
			{
				const C_VehicleLogicDecoyPickupDropOnTrackMsg* pDecoyDropMsg = Messaging::MessageCast<C_VehicleLogicDecoyPickupDropOnTrackMsg>(pMessage);
				if(pDecoyDropMsg->mDecoyDropperVehicleIndex == mVehicleIndex)
				{
					HandleDecoyDrop();
				}
				return true;
			}
		}

		return I_GameActorComponent::OnMessage(pMessage);
	}

} // namespace Components
