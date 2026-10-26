#include "stdafx.h"

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
#include <Game/Messaging/PickupActorMessages.h>

#include <RaceBook\RaceBookManager.h>

#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <GameLogic/Pickups/PickupMineBase.h>

#include <Ui\UiMessages.h>

extern Debug::Float g_debugShuntForce;
extern Debug::Float g_debugShuntForceRadius;

namespace Game
{
	bool C_PickupFireGrab::Update(bool p_IsMultiplayer)
	{
		Game::C_LevelState& levelState		= Game::C_Game::GetInstance()->GetLevelState();
		RLPickupType* pPickup				= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_GRAB);
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleActor* pVehActor	= pActor->GetGraphicalActor(mFiringVehicle);

		if (!pPickup || mLevel >= pPickup->mLevels.count)
			return true;

		RLGrabBalancingData* pGrabData = (RLGrabBalancingData *)pPickup->mLevels[(int)mLevel]->mBalancingData;

		if (!pGrabData->mForce.count || !pGrabData->mUpwardForce.count)
			return true;

		if(mDelay)
		{
			mDelay -= frameTime;
			WarnPlayers();

			if(mDelay < 0.f)
			{
				mDelay	=0.f;
				mMatrix = pLayer->mVehiclePositions[mFiringVehicle].mMatrix;
				SendFirePerkMessage();
			}

			UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_CHARGING,mDelay,mTargettedVehicle,Components::AMAX_TARGETTING_OUTCOME_NONE);
			_UI_MESSAGE_FULL( uimsg );

			return false;
		}

		Maths::HVec3 dist = mMatrix.AW();

		mRadius += ((frameTime * pGrabData->mSpeed) * pGrabData->mRadius);	//arbitary numbers until I get an effect to attach to

		if(mRadius > mTotalRadius)
			return true;

		float shortestDist = 0;
		int shortestIdx = -1;
		for(U32 a=0; a < levelState.mNumCars; a++)
		{
			//Maths::HVec3 dist=mRaceLogicLayerPtr->mVehiclePositions[mVehicleIndex].mPosition;
			if(mFiringVehicle == a)
				continue;

			float inRange = pActor->InRangeOfPickup(GetType(), mLevel, a);

			if(inRange > 1.0f)
				continue;

			if(pActor->mVehicleProximities[a].mDistanceSquared > mTotalRadius*mTotalRadius)
				continue;

			if(pActor->mVehicleProximities[a].mDistanceSquared < shortestDist || shortestIdx == -1)
			{
				shortestDist = pActor->mVehicleProximities[a].mDistanceSquared;
				shortestIdx = a;
			}
		}

		for(int i = 0; i < pLayer->mLiveMines.Size(); i++)
		{
			C_PickupFireMineBase* pickup = pLayer->mLiveMines[i];

			if(pickup->mMatrix.AW().DistanceSquared(pActor->mPositionInfo.mWorldPosition) > pGrabData->mRadius * pGrabData->mRadius)
				break;

			Maths::HVec3 normalPos = Maths::Normalise( pActor->mPositionInfo.mWorldPosition - pLayer->mVehiclePositions[mFiringVehicle].mMatrix.AW() );

			float dotProd = Maths::DotProduct(pLayer->mVehiclePositions[mFiringVehicle].mMatrix.AZ(), normalPos);

			if( dotProd < 1 - pGrabData->mAngleRange )
				break;

			pickup->mTimer = 0;

			// Steal this mine.
			Actors::C_GrantPickupMsg grantPickupMsg(mFiringVehicle, E_PICKUP_TYPE_FLASH);
			pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);

			if( pickup->GetType() == E_PICKUP_TYPE_BOMB )
			{
				Actors::C_GrantPickupMsg grantPickupMsg(mFiringVehicle, E_PICKUP_TYPE_FLASH);
				pActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);
			}
		}

		if(shortestIdx == -1 && mTargettedVehicle != -1)
		{
			//evaded
			UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_FINISHED,1.0f,mTargettedVehicle,Components::AMAX_TARGETTING_OUTCOME_EVADED);
			_UI_MESSAGE_FULL( uimsg );

			//C_VehicleLogicMsg_MissedPerkMsg missedMsg(mFiringVehicle, mTargettedVehicle, mUniqueId);

			//if( Game::C_InGameDialogueManager::Instance().IsEnabled() )
			//	Game::C_InGameDialogueManager::Instance().MessageHandler(&missedMsg);

			return true;
		}
		else if(shortestIdx != -1)
		{
			if(mTargettedVehicle == shortestIdx)
			{
				AMAX_PICKUPS_CANT_HIT_REASON reason=AMAX_PICKUPS_CANT_HIT_REASON_NONE;
				if(p_IsMultiplayer)
				{
					if(!CanHitMp(mTargettedVehicle))
					{
						return false;
					}
				}
				else
				{
					if(!CanHit(mTargettedVehicle,&reason))
					{
						if(reason == AMAX_PICKUPS_CANT_HIT_REASON_SHIELDED)
						{
							UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_FINISHED,1.0f,mTargettedVehicle,Components::AMAX_TARGETTING_OUTCOME_SHIELDED);
							_UI_MESSAGE_FULL( uimsg );
						}

						return true;
					}
				}
			}
			else
			{
				//evaded but hit someone else
				mTargettedVehicle=shortestIdx;
				UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_FINISHED,1.0f,mTargettedVehicle,Components::AMAX_TARGETTING_OUTCOME_HIT_SOMEONE_ELSE);
				_UI_MESSAGE_FULL( uimsg );
			}
		}
		else
		{
			//destoy updater
			return true;
		}

		Maths::HVec3 hv= mMatrix.AZ();
		hv=Maths::Normalise(hv);

		hv   *= mRadius;
		dist += hv;

		mAlreadyHit[mTargettedVehicle] = true;

		UI::C_UiGameMsg_TargettingEvent uimsg(mFiringVehicle,Components::AMAX_TARGETTING_PHASE_FINISHED,1.0f,mTargettedVehicle,Components::AMAX_TARGETTING_OUTCOME_HIT);
		_UI_MESSAGE_FULL( uimsg );

		Maths::HVec3 ball_pos = dist;
		hv = Maths::Normalise(hv);

		Maths::HVec3 impact_pos = ball_pos;
		impact_pos.Y(pLayer->mVehiclePositions[mTargettedVehicle].mPosition.Y());

		pActor->ClearDebugPoints();
		pActor->AddDebugPoint(impact_pos,Maths::RGBA(255,255,0));
		pActor->AddDebugPoint(pLayer->mVehiclePositions[mTargettedVehicle].mPosition,Maths::RGBA(255,255,255));

		Maths::HVec3 pos=impact_pos - pLayer->mVehiclePositions[mTargettedVehicle].mPosition ;
		pos=Maths::Normalise(pos);
		pos=pLayer->mVehiclePositions[mTargettedVehicle].mPosition + (pos * pGrabData->mForceHitRadius);

		pActor->AddDebugPoint(pos,Maths::RGBA(255,0,0));

		Actors::C_VehicleLogicActor* pTargetVeh = pLayer->GetLogicVehicleActor(mTargettedVehicle);
		int strength = pTargetVeh->mVehicleStrength;

		if(strength >= pGrabData->mForce.count)
			strength = pGrabData->mForce.count - 1; 

		float force = pGrabData->mForce[strength];

		if(strength >= pGrabData->mUpwardForce.count)
			strength = pGrabData->mUpwardForce.count - 1; 

		float upwardForce = pGrabData->mUpwardForce[strength];

		Game::Actors::VehiclePerformanceModifiers mods;
		mods.Initialise();
		mods.mForce.Set(true, (hv * force) + Maths::HVec3(0, upwardForce,0));
		mods.mPosition.Set(true, pos);

		Game::Actors::C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pActor->GetHandle(), mTargettedVehicle, mods);
		/*[ChadW] - BroadcastMessage_CleanUp*///pActor->GetActorList()->BroadcastMessage(&perfModMsg);
		pActor->SendMessageToActor(pActor->GetGraphicalActor(mTargettedVehicle)->GetHandle(), &perfModMsg);

		C_VehicleLogicMsg_HitByPerkMsg hit_by_msg(GetType(), mLevel, mTargettedVehicle, mFiringVehicle, mUniqueId);
		pActor->GetActorList()->BroadcastMessage(&hit_by_msg);
		C_InGameSpecialFXManager::Instance().MessageHandler(&hit_by_msg);

		if( Game::C_InGameDialogueManager::Instance().IsEnabled() )
			Game::C_InGameDialogueManager::Instance().MessageHandler(&hit_by_msg);

		return true;
	}

	void C_PickupFireGrab::WarnPlayers()
	{
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringVehicle);

		for(int a = 0; a < pLayer->GetNumVehicles(); a++)
		{
			if(mFiringVehicle == a)
				continue;

			Actors::C_VehicleLogicActor* pOtherActor = pLayer->GetLogicVehicleActor(a);

			float inRange = pActor->InRangeOfPickup(GetType(), mLevel, a);
			if(inRange <= 1.0f)
				pOtherActor->mTargetInDanger[mFiringVehicle] = true;
			else
				pOtherActor->mTargetInDanger[mFiringVehicle] = false;
		}
	}
	
	bool C_PickupHitGrab::Update(int vehIdx, bool p_IsMultiplayer)
	{
		C_RaceLogicLayer* pLayer					= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pFiringActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleLogicActor* pHitActor		= pLayer->GetLogicVehicleActor(vehIdx);

		if(!p_IsMultiplayer)
		{
			if( pHitActor->mPickupInfo.mPickupStore.Size() )
			{
				if(pFiringActor->QueryInterface(Components::E_IF_VehicleComponent_Pickups))
				{
					Actors::C_GrantPickupMsg grantPickupMsg(mFiringVehicle, pHitActor->mPickupInfo.mPickupStore[pHitActor->mPickupInfo.mPickupStoreIndex]);
					pFiringActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);
				}

				if(pHitActor->QueryInterface(Components::E_IF_VehicleComponent_Pickups))
				{
					Game::Actors::PickupRemovePowerMsg removePickupMsg(vehIdx, 1);
					pHitActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &removePickupMsg);

				}
			}
		}
		else
		{
			//	[GavinY] - Ok, this is another really fun pickup in MP...
			const Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();

			if( pHitActor->mPickupInfo.mPickupStore.Size() )
			{
				if(VehicleIsLocal(mFiringIndex))
				{
					//	If the firing vehicle is local to this kit then the pickup granting will be taken care of by the AddPickUp method of the MP_Local_Pickups component.
					if(pFiringActor->QueryInterface(Components::E_IF_VehicleComponent_Pickups))
					{
						Actors::C_GrantPickupMsg grantPickupMsg(mFiringVehicle, pHitActor->mPickupInfo.mPickupStore[pHitActor->mPickupInfo.mPickupStoreIndex]);
						pFiringActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);
					}
				}
				else
				{
					//	Firing vehicle is remote, so we need to instruct it to be given this pickup at the other end...
					//	and then it will add a PickupAdded net event and sync up all the other kits (including this one....) - sounds silly but keeps everyone right!
					C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Add_PickupAdded(levelState.mCars[mFiringVehicle].mNetJoinId, pHitActor->mPickupInfo.mPickupStore[pHitActor->mPickupInfo.mPickupStoreIndex], true);
				}

				//	Gotta record the pickup removal on this local vehicle.
				if(pHitActor->QueryInterface(Components::E_IF_VehicleComponent_Pickups))
				{
					Game::Actors::PickupRemovePowerMsg removePickupMsg(vehIdx, 1);
					pHitActor->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &removePickupMsg);

					C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Add_PickupRemoved(levelState.mCars[vehIdx].mNetJoinId);
				}
			}
		}

		return true;
	}
}
