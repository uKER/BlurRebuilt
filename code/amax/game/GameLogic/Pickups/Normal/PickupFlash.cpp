#include "stdafx.h"

#include <GameLogic/Pickups/Normal/PickupFlash.h>

#include <GameLogic\Structures\LevelState.h>
#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>

#include <ui\Game\Gadgets\ButtonController.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
#include "ui/UiHelpers.h"
#include <ui/UiMessages.h>
#include <RaceBook\RaceBookManager.h>

#include "GameAudio/GameAudioLayer.h"
#include <Game/GameActors/Components/Messages/AudioMessages.h>
#include "AppPhotoCapture.h"

Debug::Bool g_debugShowFlashDebug("Perks.ShowFlashDebug",false);
Debug::Float g_debugFlashWarningRadius("Perks.Flash.WarningRadius",-0.7f);
Debug::Float g_debugFlashWarningRange("Perks.Flash.WarningRange",15);

Debug::Bool g_debugShowFlashEffect("Perks.ShowFlashEffect",true);

namespace Game
{
	bool C_PickupFireFlash::Update(bool p_IsMultiplayer)
	{
		Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
		RLPickupType* pPickup				= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_FLASH);
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleActor* pVehActor	= pActor->GetGraphicalActor(mFiringVehicle);

		if (!pPickup || mLevel >= pPickup->mLevels.count)
			return true;

		RLFlashBalancingData* pFlashData = (RLFlashBalancingData *)pPickup->mLevels[(int)mLevel]->mBalancingData;

		if(mDelay)
		{
			mDelay -= frameTime;
			WarnPlayers();

			if(mDelay < 0.f)
			{
				mDelay=0.f;
				mMatrix = pVehActor->GetMatrix();

				float life=pFlashData->mLifespan;
				
				C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
				pLayer->mLiveMines.PushBack((C_PickupFireMineBase*)this);
				
				C_VehicleLogicMsg_DroppedPerkMsg msg(E_PICKUP_TYPE_FLASH, mFiringVehicle, mLevel, mMatrix, life, mUniqueId);
				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eDefaultSubsystem, 0, 0, &msg);
				Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eDefaultSubsystem, 0, 0, &msg);
				mIsMultiplayer = p_IsMultiplayer;
				//Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eDefaultSubsystem, 0, 0, &msg);

				CreateCollisionBody();

				SendFirePerkMessage();
			}

			return false;
		}

		UpdateCollisionBody();

		Maths::HVec3 dist = mMatrix.AW();

		mTimer -= frameTime;

		if(mTimer < 0.f)
		{
			Explode();

			C_VehicleLogicMsg_MissedPerkMsg missedMsg(E_PICKUP_TYPE_FLASH, mFiringVehicle, -1, mMatrix, mUniqueId);
			Game::C_InGameSpecialFXManager::Instance().MessageHandler(&missedMsg);

			return true;
		}	

		if(g_debugShowFlashDebug)
		{
			mRadius += frameTime * 4.f;

			if(mRadius > mTotalRadius)
				mRadius=0.f;

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mMatrix.AW(),1.f,Maths::RGBA(255,255,255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),mRadius,Maths::RGBA(128,128,128));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),mTotalRadius,Maths::RGBA(255,255,255));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(mMatrix.AW()+Maths::HVec3(0,0.5f,0),Util::C_String().Printf("%0.1f",mTimer));
		}

		return false;
	}

	void C_PickupFireFlash::Trigger(int idx, int mine_idx)
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		if( idx == mFiringVehicle )
			return;

		for( int i = 0; i < pLayer->GetNumVehicles(); i++ )
		{
			if(i == idx || i == mFiringVehicle)
				continue;

			Btl::Set<int>::Iterator it = mInitialTargets.Find(i);
			if(it != mInitialTargets.End())
			{
				C_VehicleLogicMsg_MissedPerkMsg missedMsg(E_PICKUP_TYPE_FLASH, mFiringVehicle, i, mMatrix, mUniqueId);

				if( Game::C_InGameDialogueManager::Instance().IsEnabled() )
					Game::C_InGameDialogueManager::Instance().MessageHandler(&missedMsg);

				UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_FINISHED,1.0f,i,Components::AMAX_TARGETTING_OUTCOME_EVADED);
				_UI_MESSAGE_FULL( uimsg );
			}
		}

		if(mIsMultiplayer)
		{
			if(!CanHitMp(idx))
				return;
		}
		else
		{
			if(!CanHit(idx))
				return;
		}

		mAlreadyHit[idx] = true;

		Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

		//flash the screen
		C_VehicleLogicMsg_HitByPerkMsg hit_by_msg(GetType(), mLevel, idx, mFiringVehicle, mUniqueId, mDroppedByIndex, -1);
		pRLC->GetActorList()->BroadcastMessage(&hit_by_msg);
		C_InGameSpecialFXManager::Instance().MessageHandler(&hit_by_msg);

		if( Game::C_InGameDialogueManager::Instance().IsEnabled() )
			Game::C_InGameDialogueManager::Instance().MessageHandler(&hit_by_msg);

		mTimer = 0;
	}

	void C_PickupFireFlash::Explode()
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringVehicle);

		Maths::HVec3 dist = mMatrix.AW();

		C_VehicleLogicMsg_DestroyPerkMsg msg(GetType(), mLevel, mMatrix, 2.0f, mUniqueId, mFiringVehicle);
		C_InGameSpecialFXManager::Instance().MessageHandler(&msg);

		float d = dist.Distance(mMatrix.AW());
		if(d < mTotalRadius * 2)
		{			
			mTimer = 0.1f;	//explode in x secs ... trying to create a ripple effect(ish)
		}

		//Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eDefaultSubsystem, 0, 1, &msg);
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, ActorMsg::E_AudioMsg_Pickups_Destroy, 0, &msg);
	}

	void C_PickupFireFlash::WarnPlayers()
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringVehicle);

		float distToCheckSq = g_debugFlashWarningRange * g_debugFlashWarningRange;

		for(int a = 0; a < pLayer->GetNumVehicles(); a++)
		{
			if(mFiringVehicle == a)
				continue;

			Actors::C_VehicleLogicActor* pOtherActor = pLayer->GetLogicVehicleActor(a);

			// This can't just use VehicleLogicActor::InRangeOfPickup like most, because we don't want to warn
			// everyone miles behind.

			float checkAngle = pActor->mVehicleProximities[a].mRelativeHeading;
			float checkDist  = pActor->mVehicleProximities[a].mDistanceSquared;

			pOtherActor->mTargetInDanger[mFiringVehicle] = ( checkAngle <= g_debugFlashWarningRadius && checkDist < distToCheckSq );

			if(!mInitialTargetsSet && pOtherActor->mTargetInDanger[mFiringVehicle])
			{
				mInitialTargets.Insert(a);
				mInitialTargetsSet = true;
			}
		}
	}

	// ------------------------------------------------------------------------------------------------------------------------------------------------

	bool C_PickupHitFlash::Update(int vehIdx, bool p_IsMultiplayer)
	{
		RLPickupType* pPickup						= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_FLASH);
		C_RaceLogicLayer* pLayer					= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor			= pLayer->GetLogicVehicleActor(vehIdx);
		Actors::C_VehicleLogicActor* pFiringActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		float frameTime								= C_GameApp::GetInstance()->GetTimeStep();

		if (!pPickup || mLevel >= pPickup->mLevels.count || !pLayer || !pActor )
		{
			//BIZ_ASSERT_FAIL("!hu || !pPickup || hu->mLevel >= pPickup->mLevels.count");
			return false;
		}

		RLFlashBalancingData* pFlashData = (RLFlashBalancingData *)pPickup->mLevels[(int)mLevel]->mBalancingData;

		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();

		if(mTimer == 0.f)
		{
			if(levelState.mCars[vehIdx].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
			{
				mPlayEffect=false;

				if (levelState.mNumViewports == 1)
				{
					PhotoMode::C_AppPhotoCapture* photoCapture = C_GameApp::GetInstance()->PhotoCapture();
					photoCapture->TakePhoto(1);
				}
			}
		}
		else if(!mPlayEffect)
		{
			if(levelState.mCars[vehIdx].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER && g_debugShowFlashEffect)
			{
				mPlayEffect=true;

				S32 local_index = UI::GetLocalPlayerIndex( vehIdx );
				Util::C_String32 buffer;

				buffer.Printf( "Flash_fade_%d", local_index );
				UI::C_UiTimelineMessage msg( buffer.Chars(), true, 0.0f );
				UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg );

				buffer.Printf( "FlashGraphic_fade_%d", local_index );
				UI::C_UiTimelineMessage msg1( buffer.Chars(), true, 0.0f );
				UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg1 );
			}
		}

		mTimer += frameTime;

		if(mTimer > 0.5f)
			return true;

		return false;
	}
}
