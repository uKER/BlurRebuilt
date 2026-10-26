#include "stdafx.h"
#include "VLC_Drafting.h"

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <Game/GameUtil/ResourceBag.h>

#include <Game/Messaging/GameflowMessages.h>
#include <Game\Messaging\VehicleActorMessages.h>
#include <GameLogic\ActorStudioActors\DynamicObjectMessages.h>

Debug::Float g_DebugDraftingBoost("Vehicle.Components.Drafting.Boost",4000.0f);
Debug::Float g_DebugDraftingAngle("Vehicle.Components.Drafting.Angle",0.96f);
Debug::Float g_DebugDraftingDistance("Vehicle.Components.Drafting.Distance",30.0f);
Debug::Float g_DebugDraftingActiveTimer("Vehicle.Components.Drafting.ActiveTimer",0.2f);
Debug::Float g_DebugDraftingKickInTimer("Vehicle.Components.Drafting.KickInTimer",1.0f);
Debug::Float g_DebugDraftingKickInSpeed("Vehicle.Components.Drafting.KickInSpeed",60.0f);
Debug::Float g_DebugDraftingOtherVehKickInSpeed("Vehicle.Components.Drafting.VehicleAheadKickInSpeed",60.0f);
Debug::Float g_DebugDraftingMaxSpeed("Vehicle.Components.Drafting.MaxSpeed",150.0f);

Debug::Logger &g_DraftingLogger( Debug::GetLogger("Amax.VLC.Drafting") );

namespace Components
{
	C_VehicleComponent_Drafting::~C_VehicleComponent_Drafting() 
	{ 
		if(mCurrentSFX)
		{
			mCurrentSFX->Stop();
			SAFE_DELETE_PTR(mCurrentSFX); 
		}
	}

	bool C_VehicleComponent_Drafting::CreateImpl()
	{
		const I_VehicleComponent_Drafting_CreateInfo* creation_data = static_cast<const I_VehicleComponent_Drafting_CreateInfo *>(GetCreator().GetData());
		Game::Actors::C_VehicleLogicActor* pActor = (Game::Actors::C_VehicleLogicActor*)GetActor();

		if(creation_data)
		{
			mVehicleIndex=creation_data->mVehicleIndex;
			mRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
		}

		//AddSubscriber( GameMessaging::E_GameMsg_Update );
		AddSubscriber( Game::E_GameflowMessage_RaceStarted );
		AddSubscriber( Game::E_GameflowMessage_RaceFinished );
		AddSubscriber( Game::E_GameflowMessage_BeginRollingStart );
		AddSubscriber( Game::E_GameflowMessage_EndRollingStart );
		AddSubscriber( ActorMsg::E_DynamicObjMsg_SetVehicleLogic);
		mActiveTimer = mBehindTimer = 0.0f;
		
		const RLDraftingSystem* pRLDraftingSystem	= ((Game::Actors::C_VehicleLogicActor*)GetActor())->GetRLC()->GetRLSystem()->mDraftingSystem;

		if(pRLDraftingSystem && pRLDraftingSystem->mDraftingSFX)
		{
			mCurrentSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pRLDraftingSystem->mDraftingSFX, 
				pActor->GetActorList()->GetDefaultContextProvider(), 
				C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());
			mSFXRunning = false;
		}

		mCurrentSFXMode = E_DRAFTING_SFX_MODE_INACTIVE;
		mEnabled = false;

		return true;
	}

	// ------------------------------------------------------------------------------------------------

	void C_VehicleComponent_Drafting::SwitchToSFXMode(E_DRAFTING_SFX_MODE mode)
	{
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
		const RLDraftingSystem* pRLDraftingSystem	= pActor->GetRLC()->GetRLSystem()->mDraftingSystem;

		if(mode == mCurrentSFXMode)
			return;

		mCurrentSFXMode = mode;
		mSFXTimer = 0.0f;
		
		if(mCurrentSFX)
		{
			mCurrentSFX->Stop();
			SAFE_DELETE_PTR(mCurrentSFX);
		}

		switch(mode)
		{
		case E_DRAFTING_SFX_MODE_FADEIN:
			{
				if(pRLDraftingSystem->mDraftingFadeInSFX)
				{
					mCurrentSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pRLDraftingSystem->mDraftingFadeInSFX, pActor->GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());
					mSFXTimer = pRLDraftingSystem->mDraftingFadeInTime;
				}

				break;
			}

		case E_DRAFTING_SFX_MODE_LOOPING:
			{
				if(pRLDraftingSystem->mDraftingSFX)
				{
					mCurrentSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pRLDraftingSystem->mDraftingSFX, pActor->GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());
					mSFXTimer = pRLDraftingSystem->mDraftingAnimTime;
				}

				break;
			}

		case E_DRAFTING_SFX_MODE_FADEOUT:
			{
				if(pRLDraftingSystem->mDraftingFadeOutSFX)
				{
					mCurrentSFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pRLDraftingSystem->mDraftingFadeOutSFX, pActor->GetActorList()->GetDefaultContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr());
					mSFXTimer = pRLDraftingSystem->mDraftingFadeOutTime;
				}

				break;
			}
		}

		if(mCurrentSFX)
			mCurrentSFX->Start();
	}

	void C_VehicleComponent_Drafting::HandleSFX()
	{
		float frameTime								= C_GameApp::GetInstance()->GetTimeStep();
		Game::Actors::C_VehicleLogicActor* pActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		if(mCurrentSFX)
		{
			mSFXTimer -= frameTime;
			mCurrentSFX->UpdateWorldMatrix(pActor->GetWorldMatrix());
			mCurrentSFX->Update(1 - mSFXTimer);
		}

		if(mSFXTimer > 0.0f || mCurrentSFXMode == E_DRAFTING_SFX_MODE_INACTIVE)
			return;

		const RLDraftingSystem* pRLDraftingSystem	= pActor->GetRLC()->GetRLSystem()->mDraftingSystem;

		switch(mCurrentSFXMode)
		{
		case E_DRAFTING_SFX_MODE_FADEIN:
			{
				if(mSFXRunning)
					SwitchToSFXMode(E_DRAFTING_SFX_MODE_LOOPING);
				else
					SwitchToSFXMode(E_DRAFTING_SFX_MODE_FADEOUT);

				break;
			}

		case E_DRAFTING_SFX_MODE_LOOPING:
			{
				if(!mSFXRunning)
					SwitchToSFXMode(E_DRAFTING_SFX_MODE_FADEOUT);
				else
					mSFXTimer = pRLDraftingSystem->mDraftingAnimTime;

				break;
			}

		case E_DRAFTING_SFX_MODE_FADEOUT:
			{
				if(mSFXRunning)
					SwitchToSFXMode(E_DRAFTING_SFX_MODE_FADEIN);
				else
					SwitchToSFXMode(E_DRAFTING_SFX_MODE_INACTIVE);

				break;
			}
		}
	}

	void C_VehicleComponent_Drafting::Update()
	{
		if (!mEnabled)
			return;

		PERF_EVENT("C_VehicleComponent.Drafting.Update");

		HandleSFX();

		Game::Actors::C_VehicleLogicActor* pActor	= (Game::Actors::C_VehicleLogicActor*)GetActor();
		float frameTime								= C_GameApp::GetInstance()->GetTimeStep();
		Game::Actors::RaceLogicCore* pRLC			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		S32 otherActorVehicleIndedx = -1;

		bool drafting = false;

		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		if(levelState.mCars[mVehicleIndex].mUpgrades.AdvancedSlipstream() && pActor->mPickupInfo.mPickupStore.Size() && 
			pActor->mPickupInfo.mPickupStore[pActor->mPickupInfo.mPickupStoreIndex].mType == E_PICKUP_TYPE_NITROUS)
		{
			for( int i = 0; i < mRaceLogicLayerPtr->GetNumVehicles(); i++ )
			{
				if( i == mVehicleIndex )
					continue;

				Game::Actors::C_VehicleLogicActor* pOtherActor = mRaceLogicLayerPtr->GetLogicVehicleActor(i);;

				float angle		= pActor->mVehicleProximities[i].mRelativeHeading;
				float checkDist	= pActor->mVehicleProximities[i].mDistanceSquared;

				if(angle > g_DebugDraftingAngle && checkDist < g_DebugDraftingDistance * g_DebugDraftingDistance && 
					pOtherActor->mPositionInfo.mSpeedInMph > g_DebugDraftingOtherVehKickInSpeed)
				{
					drafting = true;
					otherActorVehicleIndedx = i;
					LOGDEBUG(g_DraftingLogger, "Vehicle %i drafting behind vehicle %i.\n", mVehicleIndex, i);
					break;
				}
			}
		}

		// Must be behind a certain time before kicking in drafting.
		if(drafting)
			mBehindTimer += frameTime; 
		else
			mBehindTimer = 0;

		if(mBehindTimer > g_DebugDraftingKickInTimer)
			mActiveTimer = g_DebugDraftingActiveTimer;

		Game::Actors::VehiclePerformanceModifiers mods;
		mods.Initialise();
		mods.mDraftBoost.Set(true, 0.f);

		// The effect lasts a certain time after you are no longer behind.
		if(mActiveTimer > 0.0f)
		{
			mActiveTimer -= frameTime;

			if(!mVehicleIndex)
				LOGDEBUG(g_DraftingLogger, "Applying force to player vehicle.");

			// Must be going fast enough to benefit
			if(pActor->mPositionInfo.mSpeedInMph > g_DebugDraftingKickInSpeed && pActor->mPositionInfo.mSpeedInMph < g_DebugDraftingMaxSpeed)
			{

				float boostAmount = g_DebugDraftingBoost;

				/*if(otherActorVehicleIndedx != -1)
				{
					if(Game::C_Game::GetInstance()->GetLevelState().mCars[otherActorVehicleIndedx].mUpgrades.DraftDodger())
					{
						boostAmount *= Game::C_Game::GetInstance()->GetLevelState().mCars[otherActorVehicleIndedx].mUpgrades.DraftMultiplier();
					}
				}*/

				mods.mDraftBoost.Set(true, boostAmount);
				
				if( !pRLC->IsVehicleAIControlled(mVehicleIndex) && mCurrentSFXMode == E_DRAFTING_SFX_MODE_INACTIVE )
				{
					mSFXRunning = true;
					SwitchToSFXMode(E_DRAFTING_SFX_MODE_FADEIN);
				}
			}
		}

		if(!mods.mDraftBoost.mModifier && !pRLC->IsVehicleAIControlled(mVehicleIndex) && mCurrentSFXMode == E_DRAFTING_SFX_MODE_LOOPING)
		{
			mSFXRunning = false;
			SwitchToSFXMode(E_DRAFTING_SFX_MODE_FADEOUT);
		}

		if( !pRLC->IsVehicleAIControlled(mVehicleIndex) )
		{
			pActor->mDraftingAmount=mods.mDraftBoost.mModifier;

			//Game::Actors::C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pActor->GetHandle(), mVehicleIndex, mods);
			/*[ChadW] - BroadcastMessage_CleanUp*///pActor->GetActorList()->BroadcastMessage(&perfModMsg);
			//pActor->SendMessageToActor(pActor->GetGraphicalActor(mVehicleIndex)->GetHandle(), &perfModMsg);
		}
	}

	bool C_VehicleComponent_Drafting::OnMessage(const Messaging::C_MessageBase *pMessage) 
	{
		PERF_EVENT("C_VehicleComponent.Drafting.MessageHandler");

		switch(pMessage->Type())
		{
		//case GameMessaging::E_GameMsg_Update:
		//	{
		//		Update();
		//		return true;
		//	}
		//	break;

		case Game::E_GameflowMessage_RaceStarted:
			{
				mEnabled = true;
				return true;
			}
			break;

		case Game::E_GameflowMessage_RaceFinished:
			{
				mEnabled = false;
				return true;
			}
			break;
		case Game::E_GameflowMessage_BeginRollingStart:
			{
				mEnabled = false;
				return true;
			}
			break;
		case Game::E_GameflowMessage_EndRollingStart:
			{
				mEnabled = true;
				return true;
			}
			break;

		case ActorMsg::E_DynamicObjMsg_SetVehicleLogic:
			{
				const ActorMsg::C_SetVehicleLogicMsg* pMsg = Messaging::MessageCast<ActorMsg::C_SetVehicleLogicMsg>(pMessage);
				mEnabled = pMsg->m_Drafting;			
				return true;
			}
		}

		return I_GameActorComponent::OnMessage(pMessage);
	}
}
