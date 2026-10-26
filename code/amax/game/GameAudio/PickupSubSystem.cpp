#include "stdafx.h"

#include "GameAudioLayer.h"
//#include "StringHash.h"

#include "AppInterfaceLayer.h"
#include "PickUpSubsystem.h"
#include <GameApp.h>

#include "GameAudio\SystemSubsystem.H"

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/GameData/GameData.h>

#include <GameApp.h>
#include <Game/Game.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>

#include "Game/GameActors/Components/Messages/VehicleLogicMessages.h"

#include "Game/GameActors/Components/Messages/AudioMessages.h"

#include <Game/Messaging/PickupActorMessages.h>

#include "GameLogic/RaceLogicCore/Core/RaceLogicCore.h"

#include "Game\GameActors\Components\AudioComponents.h"

#include <Camera2.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic/Profile/GameProfile.h>
#include "GameLogic/Profile/MultiplayerProfile.h"

#include <SchedulerConfig.h>


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF

//enable this to count simulatantious occlusion rays per frame
//#define PICKUP_DEBUG
#define PICKUP_MAX_OCCLUSION_RAYS_PER_FRAME 2

Debug::Logger &g_AudioPerksLogger( Debug::GetLogger("Audio.Game.Perks") );

Debug::Bool gbPickupMessages("Audio.Subsystem.PickupMsgsEnabled", true, Debug::Persist());

Debug::Bool g_bLoadPickups("Audio.Subsystem.LoadSubsystem.LoadPickups", true, Debug::Persist());

Debug::Float g_bPickupLife("Audio.Subsystem.Pickups.PickupLifeTime", 12.f);

Debug::Float g_bProjectileLife("Audio.Subsystem.Pickups.ProjectileLifeTime", 200.f);

Debug::Bool g_distanceScale("Audio.Subsystem.Pickups.DistanceScale", true, Debug::Persist());

Debug::Int g_maxAudiblePerks("Audio.Subsystem.Pickups.MaxAudiblePerks", 4);

Debug::Bool g_disablePickupFire("Audio.Subsystem.Pickups.DisableFire", false);
Debug::Bool g_disablePickupProjectile("Audio.Subsystem.Pickups.DisableProjectile", false);
Debug::Bool g_disablePickupHit("Audio.Subsystem.Pickups.DisableHit", false);

Debug::Bool g_DoPickupOcclusion("Audio.Subsystem.Pickups.DoOcclusion", true);

Debug::Bool g_RenderPickupOcclusionRays("Audio.Subsystem.Pickups.RenderOcclusionRays", false);
Debug::Float g_PickupOcclusionRayHeight("Audio.Subsystem.Pickups.OcclusionRayHeight", 0.53);
Debug::Float g_PickupMaxUpdateDistance("Audio.Subsystem.Pickups.MaxUpdateDistance", 500.00);
Debug::Float g_PickupMaxRayDistance("Audio.Subsystem.Pickups.MaxRayDistance", 250.00);

Debug::Float g_PickupOcclusionRayTimeLimit("Audio.Subsystem.Pickups.RayTimeLimit", 30.0);

//Debug::Float g_ChargeFireMixPriorityDistance("Audio.Subsystem.Pickups.FireMixPriorityDistance", 50.f);
//Debug::Float g_HitMixPriorityDistance("Audio.Subsystem.Pickups.HitMixPriorityDistance", 50.f);
//Debug::Float g_ProjectileMixPriorityDistance("Audio.Subsystem.Pickups.ProjectileMixPriorityDistance", 50.f);
//
//Debug::Int g_ChargeFireMixObjectsMax("Audio.Subsystem.Pickups.FireMixObjectsMax", 6);
//Debug::Int g_HitMixPriorityObjectsMax("Audio.Subsystem.Pickups.HitMixPriorityObjectsMax", 6);
//Debug::Int g_ProjectileMixPriorityObjectsMax("Audio.Subsystem.Pickups.ProjectileMixPriorityObjectsMax", 6);
//
//
//Debug::Bool g_mixScale("Audio.Subsystem.Pickups.MixScale", true);


Scheduler::GroupHandle PickUpSubsystem::m_hSchedulerGroup = 0;


U32 occlusionNoRaysPerFrame = 0;
U32 maxOcclusionNoRaysPerFrame = 0;

void PickUpSubsystem::SetSchedulerGroup(Scheduler::GroupHandle handle)
{
	m_hSchedulerGroup = handle;
}
Util::C_String gAudioPickupString[E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_MAX];

Audio::InterfaceController& gAic = Audio::InterfaceController::Get();

//-----------------------------------------------------------------------------
// PickUpSubsystem::PickUpSubsystem
//-----------------------------------------------------------------------------
PickUpSubsystem::PickUpSubsystem()
{

	for (S32 i = 0; i < E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_MAX; ++i)
		pickupEntityDefinition[i] = NULL;

	m_removeAllSubscribers = false;
	m_EntityInstanceCount = 0;
	m_bRaceInProgress = false;
	m_AudibleDistance = 0;
	m_gamePaused = false;
	m_pauseTime = 0;
	m_Group = Scheduler::JQ_GROUP_NONE;
	m_Policy = 0;
}

//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------
bool PickUpSubsystem::ProcessSystemMessage(const U32 u32MessageId)
{
	if(!gbPickupMessages)
		return false;

	switch (u32MessageId)
	{

		case Audio::eInit:
			{
				if(!g_bLoadPickups)
					break;

				// load all dats & wavebanks for pickups
				PopulateEntityDefinitions();
			}
			break;

			case Audio::eBeginLevel:
			{
				m_pickupsChargeFire.Reset();
				m_pickupsHitShield.Reset();
				m_pickupsProjectile.Reset();
				m_isSplitScreen = false;

				if(Game::C_Game::GetInstance()->GetLevelState().mNumViewports > 1)
					m_isSplitScreen = true;


				//sort out viewports, get correct listener for view

				//if(isSplitScreen && (vehicleIndex < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))//max 4 views
				//{
				//	camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Mat43();
				//}

				m_EntityInstanceCount = 0;
				m_bRaceInProgress = true;
				m_AudibleDistance = 0;

					//Need a profile
				Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
				Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();

				Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
				bool isRanked = !pLayer || !pLayer->GetGameModePtr() || pLayer->GetGameModePtr()->IsRanked();
	
				UI::E_MULTIPLAYER_LOADOUT_TYPE loadOutType = pMultiplayerProfile->GetLoadouts(isRanked).GetCurrentLoadOutType();
				const Game::MultiplayerLoadOut& loadOut = pMultiplayerProfile->GetLoadouts(isRanked).GetLoadOut(loadOutType);

				for( S32 j = 0; j < UI::E_MULTIPLAYER_LOADOUT_SLOT_MAX; j++)
				{
				
					bool empty = true;
					abilitySlots[j] = loadOut.GetAbilityInSlot( (UI::E_MULTIPLAYER_LOADOUT_SLOT)j);
				}	

				m_levelState = &Game::C_Game::GetInstance()->GetLevelState();

				m_Policy = Game::C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy();
				m_Group = Scheduler::CreateGroup(false);
				pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
				m_FrameCount = 1;
			}
			break;

		case Audio::eEndLevel:
			{
				// release entities and destroy any active perks
				for (U32 l=0; l<E_PICKUP_LIST_TYPE_MAX; l++)
				{
					for (Btl::Map<U64, PickupObject*>::Iterator it=GetPickupCollection(l).m_Map.Begin(); it!= GetPickupCollection(l).m_Map.End(); )
					{
						PickupObject* pObj = it->GetValue();
						Btl::Map<U64, PickupObject*>::Iterator toErase = it++;	// increment iterator BEFORE we erase. note: it++ returns the old position not the new one
						pObj->Reset();
						GetPickupCollection(l).m_Map.Erase(toErase);
						delete pObj;
					}

					m_DistanceMap.Clear();

				}

				m_EntityInstanceCount = 0;

					// move to app quit message
					/*for(int i = 0;i < E_PICKUP_TYPE_MAX*NumPickupLists; i++)
					{
						if (pickupEntityDefinition[i])
						{
							pickupEntityDefinition[i]->ReleaseEntityType();
							pickupEntityDefinition[i] = NULL;
						}
					}*/

					m_removeAllSubscribers = true;
					m_bRaceInProgress = false;	
					Scheduler::Flush(m_Group);

					Scheduler::DestroyGroup(m_Group);
					m_Policy = 0;
					pRLC = NULL;

#ifdef PICKUP_DEBUG
					LOGINFO(g_AudioPerksLogger, DBSTR"Max No. RT's on a frame was %d", maxOcclusionNoRaysPerFrame);
#endif

			}
			break;
		case Audio::eMute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = gAic.LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					gAic.SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,0);
				}

				///fix for MP 'pause' game state, we'll want to delete objects as normal since the game is not paused
				if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsMultiplayer() && !m_isSplitScreen)
					m_gamePaused = false;
				else
					m_gamePaused = true;
			}
			break;

		case Audio::eUnmute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = gAic.LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					gAic.SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,1);
				}

				m_gamePaused = false;
			}
			break;

		case Audio::eSuspend:
			for (U32 l=0; l<E_PICKUP_LIST_TYPE_MAX; l++)
			{
				for (Btl::Map<U64, PickupObject*>::Iterator it=GetPickupCollection(l).m_Map.Begin(); it!= GetPickupCollection(l).m_Map.End(); ++it )
				{
					PickupObject* pObj = it->GetValue();
					gAic.SuspendEntity(pObj->m_hEntity);
				}
			}

			break;

		case Audio::eResume:
			for (U32 l=0; l<E_PICKUP_LIST_TYPE_MAX; l++)
			{
				for (Btl::Map<U64, PickupObject*>::Iterator it=GetPickupCollection(l).m_Map.Begin(); it!= GetPickupCollection(l).m_Map.End(); ++it )
				{
					PickupObject* pObj = it->GetValue();
					//if it was previously playing before 'pause' resume it.
					if(!pObj->m_IsSuspendedPrePause)
						gAic.ResumeEntity(pObj->m_hEntity);
				}
			}

			break;

		case Audio::eRaceFinished:			
			m_bRaceInProgress = false;	
			pRLC = NULL;
			break;	
	}
	return true;
}
 
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ProcessCustomMessage
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool PickUpSubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data , void* pData)
{

	if (!gbPickupMessages)
		return false;

		switch(u32MessageId)
		{
			case E_PickUpSubsystem_Projectile_Hit_World:
			{
					C_VehicleLogicMsg_PerkEnded* msg =  (C_VehicleLogicMsg_PerkEnded*)pData;

					U64 uniqueId = GetUniqueId(msg->mVehicleIndex, msg->mUniqueID);

					// lookup in both lists
					//for (U32 puColl=0; puColl<E_PICKUP_LIST_TYPE_MAX; puColl++)
					{
						PickupObject* pObj = GetPickupCollection(E_PICKUP_LIST_TYPE_PROJECTILE).LookupById(uniqueId);
					
						if (pObj)
						{
							if( msg->mPickupType == E_PICKUP_TYPE_SHUNT ||  msg->mPickupType == E_PICKUP_TYPE_BLINDFIRE )
							{
								if(msg->mUserBodyType == Game::E_BodyUserDataType_Unknown)
								{
									Audio::EntityDataMemberHandle  hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupHitWorld");
									gAic.TriggerEntityActionIn(pObj->m_hEntity, hActionIn,false);

									//if the entity is suspended when it hits the world, mark it for deletion and kill it
									if(gAic.IsEntitySuspended(pObj->m_hEntity))
										pObj->m_markForDelete = true;
								}
								else
								if(msg->mUserBodyType == Game::E_BodyUserDataType_Projectile)
								{
									Audio::EntityDataMemberHandle  hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupHitProjectile");
									gAic.TriggerEntityActionIn(pObj->m_hEntity, hActionIn,false);

									//if the entity is suspended when it hits the world, mark it for deletion and kill it
									if(gAic.IsEntitySuspended(pObj->m_hEntity))
										pObj->m_markForDelete = true;

								}
							}

						}

					}


			}
			break;

			case E_PickUpSubsystem_Ended:
			{
				if(pData)
				{
					//onst C_VehicleLogicMsg_DestroyPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_DestroyPerkMsg >(pMessage);

					C_VehicleLogicMsg_PerkEnded* msg =  (C_VehicleLogicMsg_PerkEnded*)pData;

					U64 uniqueId = GetUniqueId(msg->mVehicleIndex, msg->mUniqueID);

					// lookup in both lists
					for (U32 puColl=0; puColl<E_PICKUP_LIST_TYPE_MAX; puColl++)
					{
						PickupObject* pObj = GetPickupCollection(puColl).LookupById(uniqueId);
					
						if (pObj)
						{
							Audio::EntityDataMemberHandle  hActionIn;


							//select the fire type
							switch(msg->mPickupSubType)
							{
							case E_PICKUP_SUB_TYPE_NORMAL:
							case E_PICKUP_SUB_TYPE_DECOY:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupEnded");	
								}
								break;

							case E_PICKUP_SUB_TYPE_REVERSED:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupBackwardsEnded");
								}
								break;

							case E_PICKUP_SUB_TYPE_DOUBLE_TAP:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupDoubleEnded");
								}
								break;

							case E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupBackwardsDoubleEnded");
								}
								break;

							case E_PICKUP_SUB_TYPE_MOD:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupModEnded");
								}
								break;

							case E_PICKUP_SUB_TYPE_SUB_MOD_REVERSED:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupSubModBackwardsEnded");
								}
								break;

							case E_PICKUP_SUB_TYPE_MINI:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupMiniEnded");
								}
								break;
							
							default:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupEnded");	
									LOGWARN(g_AudioPerksLogger, DBSTR"Unexpected Pickup Ended sub Type...please let cian know");
								}
								break;


							}

							//send the trigger
							if(hActionIn != -1)
								gAic.TriggerEntityActionIn(pObj->m_hEntity, hActionIn,false);

							//if the entity is suspended when it ends, mark it for deletion and kill it
							if(gAic.IsEntitySuspended(pObj->m_hEntity))
								pObj->m_markForDelete = true;
						}
					}
				}
			}
			break;

			case E_PickUpSubsystem_Fire:
			{
				if(pData && m_bRaceInProgress)
				{
					if(g_disablePickupFire)
						break;

					C_VehicleLogicMsg_FiredPerkMsg* msg =  (C_VehicleLogicMsg_FiredPerkMsg*)pData;

					//when no vheicle has directly fired the perk for bomb/shock, don't play a 'fire' sound 
						//test if it's been fired by a vehicle, if not, we wish to delete the 'fire' object straight away
					if(msg->mNoVehicleFiringIndex && (msg->mPickupType == E_PICKUP_TYPE_BOMB || msg->mPickupType == E_PICKUP_TYPE_SHOCK))
						break;					

					U8 vehicleIndex = msg->mVehicleIndex;

					if(!pRLC)
						break;
					Game::Actors::C_VehicleActor* pActor = pRLC->GetVehicleActor(vehicleIndex);
					if(!pActor)
						break;

					enum E_PICKUP_TYPE ePickupType = msg->mPickupType;

					enum E_PICKUP_SUB_TYPE ePickupSubType = msg->mPickupSubType;

					F32 pickupTier  = (F32)msg->mLevel;
					
					U64 uniqueId = GetUniqueId(msg->mVehicleIndex, msg->mUniqueId);

					//LOGDEBUG(g_AudioPerksLogger, DBSTR" FIRE uid 0x%llx, veh %d, pid 0x%x, %d '%s'.", uniqueId, msg->mVehicleIndex, msg->mUniqueId, ePickupType, gAudioPickupString[ePickupType].Chars());

					//create the pickUp if it doesn't exist
					PickupObject* pObj = CreatePickup(uniqueId,ePickupType,vehicleIndex, E_PICKUP_LIST_TYPE_CHARGEFIRE);

					if (!pObj)
					{
						LOGWARN(g_AudioPerksLogger, DBSTR" Could not create fire perk");
						break;
					}

					bool isFocusVehicle = false;

					if(pRLC)
					{
						S32 numViewports = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();						
						//check all viewports to see if this car has focus
						for(int i = 0; i < (int)numViewports; i++)
						{
							if(vehicleIndex == pRLC->GetViewportFocusVehicleIndex(i))
								 isFocusVehicle = true;
						}

					}

					pObj->m_vehicleIndex = msg->mVehicleIndex;

					Audio::EntityDataMemberHandle hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupLevel");
					gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)pickupTier);

					hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "IsFocusCar");
					gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)isFocusVehicle);

					//special case to handle perks that charge before fireing
					if(pRLC)
						pObj->m_Matrix = pActor->GetMatrix();

					//fire off occlusion trace
					if(!m_isSplitScreen && g_DoPickupOcclusion && pRLC)
						DoOcclusionTest(&pObj->m_Matrix,&m_camMatrix,pObj->m_fDistance,pRLC->GetPhysicsScene(),pObj);						


					pObj->m_eType = msg->mPickupType;


					Audio::EntityDataMemberHandle hVectorIn;
					
					//take pos from the car
					Maths::HVec3 objPos = pObj->m_Matrix.AW();
					//Maths::HVec3 objDir = pObj->m_Matrix.AZ();
					//Maths::HVec3 objUp = pObj->m_Matrix.AY();

					hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ObjectPosition");
					gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn,  (F32*)&objPos);
					pObj->m_prevObjPos = objPos;

					//sort out listener position in case of splitscreen etc
					Maths::HMat43 camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();

					Maths::HVec3 velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();


					//sort out viewports, get correct listener for view
					if(m_isSplitScreen && (vehicleIndex < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))//max 4 views
					{
						camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Mat43();
						velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Live().Velocity();
					}

					hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerPosition");
					gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AW()));

					hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerDirection");
					gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AZ()));		

					hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerUp");
					gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AY()));

					hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerVelocity");
					gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&velocity));

					//send vehicle abilities					

					hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "AbilitySlot1");

					if(hEntityIn != -1)
						gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)abilitySlots[0]);

					hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "AbilitySlot2");

					if(hEntityIn != -1)
						gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)abilitySlots[1]);

					hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "AbilitySlot3");

					if(hEntityIn != -1)
						gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)abilitySlots[2]);

	
					
					if(msg->mPickupType == E_PICKUP_TYPE_SHUNT && m_levelState->mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities()->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_ShuntDamage)->GetActive())
					{
						if(msg->mPickupSubType == E_PICKUP_SUB_TYPE_REVERSED)
							msg->mPickupSubType = E_PICKUP_SUB_TYPE_MOD_REVERSED;
						else
							msg->mPickupSubType = E_PICKUP_SUB_TYPE_MOD;
					}

					Audio::EntityDataMemberHandle hActionIn;

					//select the fire type
					switch(msg->mPickupSubType)
					{
					case E_PICKUP_SUB_TYPE_NORMAL:
					case E_PICKUP_SUB_TYPE_DECOY:
						{
							hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupFire");
						}
						break;

					case E_PICKUP_SUB_TYPE_REVERSED:
						{
							hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupBackwardsFire");
						}
						break;

					case E_PICKUP_SUB_TYPE_DOUBLE_TAP:
						{
							hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupDoubleFire");
						}
						break;

					case E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP:
						{
							hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupBackwardsDoubleFire");
						}
						break;

					case E_PICKUP_SUB_TYPE_MOD:
						{
							hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupModFire");
						}
						break;

					case E_PICKUP_SUB_TYPE_SUB_MOD:
						{
							hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupSubModFire");
						}
						break;

					case E_PICKUP_SUB_TYPE_MOD_REVERSED:
						{
							hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupModBackwardsFire");
						}
						break;

					case E_PICKUP_SUB_TYPE_SUB_MOD_REVERSED:
						{
							hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupSubModBackwardsFire");
						}
						break;

					default:
						hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupFire");
						LOGWARN(g_AudioPerksLogger, DBSTR"Unexpected Pickup Fire sub Type...please let cian know");
						break;
					}

					gAic.TriggerEntityActionIn(pObj->m_hEntity, hActionIn,false);


					pObj->m_fTimeTriggered = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
				}//if data
			}
			break;

			case E_PickUpSubsystem_Fire_Projectile:
			{	
				CreateProjectilePickUp(pData);
			}
			break;


			case E_PickUpSubsystem_Hit:
			{

				if(g_disablePickupHit)
						break;
					
				if(pData && m_bRaceInProgress)
				{
					C_VehicleLogicMsg_HitByPerkMsg* msg =  (C_VehicleLogicMsg_HitByPerkMsg*)pData;

					S32 eIncomingPickupType = msg->mPickupType;
					S32 eIncomingPickupSubType = msg->mPickupSubType;

					//hack for hitting a decoy pickup. make it a shock.
					if(eIncomingPickupSubType == E_PICKUP_SUB_TYPE_DECOY)
						eIncomingPickupType = E_PICKUP_TYPE_SHOCK;



				
					U8 vehicleIndex = msg->mVehicleIndex;

					if(!pRLC)
						break;
				Game::Actors::	C_VehicleActor* pActor = pRLC->GetVehicleActor(vehicleIndex);
					if(!pActor)
						break;

					S32 ePickupType = (S32)eIncomingPickupType+E_PICKUP_TYPE_MAX * E_PICKUP_LIST_TYPE_HITSHIELD;
					U8 targetIndex = msg->mTargetedVehicleIndex;
					U64 uniqueId = GetUniqueId(msg->mVehicleIndex, msg->mUniqueID);
					F32 pickupTier  = (F32)msg->mLevel;
			
					//Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
					bool isFocusVehicle = false;

					if(pRLC)
					{
						S32 numViewports = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();						
						//check all viewports to see if this car has focus
						for(int i = 0; i < (int)numViewports; i++)
						{
							if(vehicleIndex == pRLC->GetViewportFocusVehicleIndex(i))
								 isFocusVehicle = true;
						}

					}

					bool bSplash = false;

					if(targetIndex == vehicleIndex || targetIndex == 0xff )
					{
						// has hit a car directly
						bSplash = false;
					}
					else if(isFocusVehicle)
					{
						// splash damage hitting the player car
						bSplash = true;
					}
					else
					{
						// otherwise its splash damage hitting an AI car, and we dont play a sound in that situation so do nothing
						break;
					}

					// create the pickUp if it doesn't exist
					PickupObject* pObj = CreatePickup(uniqueId,ePickupType,vehicleIndex, E_PICKUP_LIST_TYPE_HITSHIELD);

					if (!pObj)
					{
						LOGWARN(g_AudioPerksLogger, DBSTR" Could not create/get hit pickup");
						break;
					}

					pObj->m_vehicleIndex = vehicleIndex;
					pObj->m_targetIndex = targetIndex;

					if(msg->mFiringVehicleIndex == (U8)isFocusVehicle)
					{
						//did hit come from focus car?
							Audio::EntityDataMemberHandle hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "FocusCarFired");
							gAic.TriggerEntityActionIn(pObj->m_hEntity, hActionIn,false);	

					}

					Audio::EntityDataMemberHandle hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "IsFocusCar");
					gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)isFocusVehicle);

					hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupLevelHit");
					gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)pickupTier);

					//send vehicle abilities					

					hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "AbilitySlot1");

					if(hEntityIn != -1)
					{
						//if boss mod equipped
						if(m_levelState->mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities()->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_RepairBonus)->GetActive())
							gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)UI::E_MultiplayerVehicleAbilities_Titanium_Armour);
						else
							gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)abilitySlots[0]);

					}

					hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "AbilitySlot2");

					if(hEntityIn != -1)
						gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)abilitySlots[1]);

					hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "AbilitySlot3");

					if(hEntityIn != -1)
						gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)abilitySlots[2]);

					if(pRLC)
						pObj->m_Matrix = pActor->GetMatrix();

					pObj->m_eType = (E_PICKUP_TYPE)eIncomingPickupType;
					pObj->m_translatedType = ePickupType;

					//fire off occlusion trace

					if(!m_isSplitScreen && g_DoPickupOcclusion && pRLC)
						DoOcclusionTest(&pObj->m_Matrix,&m_camMatrix,pObj->m_fDistance,pRLC->GetPhysicsScene(),pObj);

					Audio::EntityDataMemberHandle hVectorIn;
										
					Maths::HVec3 objPos = pObj->m_Matrix.AW();

					hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, bSplash ? "ObjectSplashPosition" : "ObjectPosition" );
					gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn,  (F32*)&objPos);
					pObj->m_prevObjPos = objPos;

					//sort out listener position in case of splitscreen etc
					Maths::HMat43 camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();

					Maths::HVec3 velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();


					//sort out viewports, get correct listener for view
					if(m_isSplitScreen && (targetIndex < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))//max 4 views
					{
						//view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(targetIndex);
						camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(targetIndex).DebugFeed().Mat43();

						velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(targetIndex).DebugFeed().Live().Velocity();
					}

					hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerPosition");
					gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AW()));

					hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerDirection");
					gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AZ()));		

					hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerUp");
					gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AY()));

					hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerVelocity");
					gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&velocity));


					//TODO shielded from vehicleActor in branch
					bool isShielded = false;

					const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);

					if(pLogicActor && pLogicActor->mShieldTime > 0.f)
						isShielded = true;

					Audio::EntityDataMemberHandle hActionIn;

					if(bSplash)
					{
						if(isShielded)
						{
							hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity,"PickupSplashHitShield" );
						}
						else
						{
							hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupSplashHitCar");
						}

						gAic.TriggerEntityActionIn(pObj->m_hEntity, hActionIn,false);
					}
					else
					{
						//TODO shielded from vehicleActor in branch

						//check if car is shielded
						if(isShielded && (eIncomingPickupType != E_PICKUP_TYPE_NITROUS && eIncomingPickupType != E_PICKUP_TYPE_REPAIR))
						{
							hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupHitShield");
						}
						else
						{

							//update for shunt damage in singlePlayer
							if(eIncomingPickupType == E_PICKUP_TYPE_SHUNT && m_levelState->mCars[msg->mFiringVehicleIndex].mUpgrades.GetMultiplayerAbilities()->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_ShuntDamage)->GetActive())
							{
								eIncomingPickupSubType = E_PICKUP_SUB_TYPE_SUB_MOD;
							}
							else
							if(eIncomingPickupType == E_PICKUP_TYPE_NITROUS && m_levelState->mCars[msg->mFiringVehicleIndex].mUpgrades.GetMultiplayerAbilities()->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_NitroRift)->GetActive())
							{
								if(eIncomingPickupSubType == E_PICKUP_SUB_TYPE_NORMAL)
									eIncomingPickupSubType = E_PICKUP_SUB_TYPE_MOD;
							}

							switch(eIncomingPickupSubType)
							{
							case E_PICKUP_SUB_TYPE_NORMAL:
							case E_PICKUP_SUB_TYPE_DECOY:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupHitCar");
								}
								break;

							case E_PICKUP_SUB_TYPE_REVERSED:
								{
									if(msg->mPickupHitType == E_PICKUP_HIT_TYPE_MAJOR)
										hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupBackwardsHitCar");
									else
										hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupMinorBackwardsHitCar");

								}
								break;

							case E_PICKUP_SUB_TYPE_DOUBLE_TAP:
								{
									if(msg->mPickupHitType == E_PICKUP_HIT_TYPE_MAJOR)
										hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupDoubleHitCar");
									else
										hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupMinorDoubleHitCar");
								}
								break;

							case E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupDoubleBackwardsHitCar");
								}
								break;

						case E_PICKUP_SUB_TYPE_SUB_MOD:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupSubModHitCar");
								}
								break;

						case E_PICKUP_SUB_TYPE_MOD:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupModHitCar");
								}
								break;

						case E_PICKUP_SUB_TYPE_MOD_REVERSED:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupModBackwardsHitCar");
								}
								break;

						case E_PICKUP_SUB_TYPE_SUB_MOD_REVERSED:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupSubModBackwardsHitCar");
								}
								break;

						case E_PICKUP_SUB_TYPE_MINI:
								{
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupMiniHitCar");
								}
								break;

							default:
									hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupHitCar");
									LOGWARN(g_AudioPerksLogger, DBSTR"Unexpected Pickup Hit sub Type...please let cian know");
								break;
							}
						}

						gAic.TriggerEntityActionIn(pObj->m_hEntity, hActionIn,false);
					}


					pObj->m_fTimeTriggered = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();

					//if (hActionIn != -1)
					//	LOGDEBUG(g_AudioPerksLogger, DBSTR" triggering %s hit actionin uid 0x%x, %d '%s'.", bSplash ? "splash" : "direct", uniqueId, ePickupType, gAudioPickupString[ePickupType].Chars());
				}
			}
			break;

				case E_PickUpSubsystem_UpdateMatrix:
				{
					//update perk positions
					

					C_VehicleLogicMsg_PerkUpdateMatrixMsg* msg =  (C_VehicleLogicMsg_PerkUpdateMatrixMsg*)pData;

					U8 vehicleIndex = msg->mVehicleIndex;

					U64 uniqueId = GetUniqueId(vehicleIndex, msg->mUniqueID);

					// lookup in both lists
					PickupObject* pObj = m_pickupsProjectile.LookupById(uniqueId);

					if(!pObj)
						break;

					Maths::HVec3 vel = msg->mVelocity;
					if (!pObj->m_prevObjVel.All_EQ(vel))
					{
						Audio::EntityDataMemberHandle hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ObjectVelocity");
						gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn,  (F32*)&vel);
						pObj->m_prevObjVel = vel;
					}

					//update the projectile matrix, this will be sent in the update..
					pObj->m_Matrix = msg->mMatrix;
				}
				break;

				case E_PickUpSubsystem_SetFXDurations:
				{
					C_VehicleLogicMsg_PowerUpFXDurations* msg =  (C_VehicleLogicMsg_PowerUpFXDurations*)pData;

					U8 vehicleIndex = msg->mVehicleIndex;

					U64 uniqueId = GetUniqueId(vehicleIndex, msg->mUniqueID);

					for (U32 puColl=0; puColl<E_PICKUP_LIST_TYPE_MAX; puColl++)
					{
						// lookup in both lists
						PickupObject* pObj = GetPickupCollection(puColl).LookupById(uniqueId);

						if(!pObj)
							break;

						//send duration envelope times
						Audio::EntityDataMemberHandle hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "DurationIntro");
						gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)msg->mDurationIntro);

						hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "DurationLoop");
						gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)msg->mDurationLoop);

						hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "DurationOutro");
						gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)msg->mDurationOutro);
					}

				}
				break;
		}
	return false;
}


#ifndef CFG_GOLD
Debug::Float gfPickupOffsetDebugY("Audio.Subsystem.Pickups.OffsetY", 130.f);
Debug::Bool gbPickupDebug("Audio.Subsystem.Pickups.RenderDebug", false);
#endif

void PickUpSubsystem::CreateProjectilePickUp(void* pData)
{
	if(g_disablePickupProjectile)
			return;

	if(pData && m_bRaceInProgress)
	{

		C_VehicleLogicMsg_FiredPerkMsg* msg =  (C_VehicleLogicMsg_FiredPerkMsg*)pData;

		U8 vehicleIndex = msg->mVehicleIndex;

		if(!pRLC)
			return;
		Game::Actors::C_VehicleActor* pActor = pRLC->GetVehicleActor(vehicleIndex);
		if(!pActor)
			return;

		S32 ePickupType = (S32)msg->mPickupType+E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_PROJECTILE;//go to projectile array

		U64 uniqueId = GetUniqueId(msg->mVehicleIndex, msg->mUniqueId);

		//create the pickUp if it doesn't exist
		PickupObject* pObj = CreatePickup(uniqueId,ePickupType,vehicleIndex, E_PICKUP_LIST_TYPE_PROJECTILE);

		if (!pObj)
		{
			LOGWARN(g_AudioPerksLogger, DBSTR" Could not create projectile perk");
			return;
		}

		if(msg->mPickupType == E_PICKUP_TYPE_BOMB || msg->mPickupType == E_PICKUP_TYPE_BLINDFIRE || msg->mPickupType == E_PICKUP_TYPE_SHUNT || msg->mPickupType == E_PICKUP_TYPE_SHOCK)
			pObj->m_Follower = false;

		if(msg->mPickupType == E_PICKUP_TYPE_BOMB || E_PICKUP_TYPE_BLINDFIRE || msg->mPickupType == E_PICKUP_TYPE_SHUNT || msg->mPickupType == E_PICKUP_TYPE_SHOCK)
			pObj->m_Projectile = true;

		bool isFocusVehicle = false;

		if(pRLC)
		{
			S32 numViewports = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();						
			//check all viewports to see if this car has focus
			for(int i = 0; i < (int)numViewports; i++)
			{
				if(vehicleIndex == pRLC->GetViewportFocusVehicleIndex(i))
					 isFocusVehicle = true;
			}
		}

		pObj->m_firedByFocusVehicle = isFocusVehicle;

		Audio::EntityDataMemberHandle hEntityIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "IsFocusCar");
		gAic.SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)isFocusVehicle);

		pObj->m_vehicleIndex = msg->mVehicleIndex;

		//special case to handle perks that charge before fireing
		//pObj->m_Charger = true;
		if(pRLC)
			pObj->m_Matrix = pActor->GetMatrix();

		pObj->m_eType = msg->mPickupType;

		Audio::EntityDataMemberHandle hVectorIn;
		
		Maths::HVec3 objPos = pObj->m_Matrix.AW();
		Maths::HVec3 objDir = pObj->m_Matrix.AZ();

		hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ObjectPosition");
		gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn,  (F32*)&objPos);
		pObj->m_prevObjPos = objPos;

		hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ObjectDirection");
		gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn,  (F32*)&objDir);
		pObj->m_prevObjDir = objDir;

		//sort out listener position in case of splitscreen etc
		Maths::HMat43 camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();

		Maths::HVec3 velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();

		//sort out viewports, get correct listener for view
		if(m_isSplitScreen && (vehicleIndex < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))//max 4 views
		{
			camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Mat43();
			velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Live().Velocity();
		}

		hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerPosition");
		gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AW()));

		hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerDirection");
		gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AZ()));		

		hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerUp");
		gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AY()));

		hVectorIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerVelocity");
		gAic.SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&velocity));

		
		if(m_levelState->mCars[vehicleIndex].mUpgrades.GetMultiplayerAbilities()->GetAbility(UI::E_MultiplayerVehicleAbilities_Boss_ShuntDamage)->GetActive() && msg->mPickupType == E_PICKUP_TYPE_SHUNT)
		{
		//update f0r shunt damage in singlePlayer			
			msg->mPickupSubType = E_PICKUP_SUB_TYPE_SUB_MOD;		
		}


		Audio::EntityDataMemberHandle hActionIn;

		//select the fire type
		switch(msg->mPickupSubType)
		{
		
		case E_PICKUP_SUB_TYPE_NORMAL:
		case E_PICKUP_SUB_TYPE_DECOY:
			{
				hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupProjectileActive");
			}
			break;

		case E_PICKUP_SUB_TYPE_REVERSED:
			{
				hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupBackwardsProjectileActive");
			}
			break;

		case E_PICKUP_SUB_TYPE_DOUBLE_TAP:
			{
				 hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupDoubleProjectileActive");
			}
			break;

		case E_PICKUP_SUB_TYPE_REVERSED_DOUBLE_TAP:
			{
				hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupBackwardsDoubleProjectileActive");
			}
			break;

		case E_PICKUP_SUB_TYPE_MOD:
			{
				hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupModProjectileActive");
			}
			break;

		case E_PICKUP_SUB_TYPE_SUB_MOD:
			{
				hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupSubModProjectileActive");
			}
			break;

		case E_PICKUP_SUB_TYPE_MINI:
			{
				hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupMiniProjectileActive");
			}
			break;

		case E_PICKUP_SUB_TYPE_MOD_REVERSED:
			{
				hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupModBackwardsProjectileActive");
			}
			break;

		case E_PICKUP_SUB_TYPE_SUB_MOD_REVERSED:
			{
				hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupSubModBackwardsProjectileActive");
			}
			break;



		default:
			hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PickupProjectileActive");
			LOGWARN(g_AudioPerksLogger, DBSTR"Unexpected projectile sub Type...please let cian know");
			break;


		}

		gAic.TriggerEntityActionIn(pObj->m_hEntity, hActionIn,false);	

		pObj->m_fTimeTriggered = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();		

		pObj->m_fTimeOfLastRT = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();	

	}//if data

}

void PickUpSubsystem::RenderDebug()
{
#ifndef CFG_GOLD
	if (gbPickupDebug)
	{
		// render nice little green square.
		// without this the text doesnt render. dont know why! answers on a postcard...
		Maths::HVec2 pos( 0, 0);
		Maths::HVec2 size( 10.0f,  10.f);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(pos,size,Maths::RGBA(0x00, 0xff, 0x00, 0xff));

		F32 curY;

		for (U32 l=0; l<E_PICKUP_LIST_TYPE_MAX; l++)
		{
			curY = gfPickupOffsetDebugY;

			for ( Btl::Map<U64, PickupObject*>::Iterator it = GetPickupCollection(l).m_Map.Begin(); it != GetPickupCollection(l).m_Map.End(); ++it )
			{
				PickupObject* obj = it->GetValue();
				Btl::String entName;

				Audio::EntityInstance* pInstance = gAic.GetEntityInstanceFromHandle(obj->m_hEntity);
				if (pInstance)
				{
					Audio::EntityDefinition* pDef =  gAic.GetDefinition(pInstance);
					if (pDef)
						entName = pDef->Name();
				}

				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(l==E_PICKUP_LIST_TYPE_CHARGEFIRE?100:350,curY), Util::C_String().Printf("hEnt %8x uid %llx %s %10s d %.2f", 
					obj->m_hEntity.ArrayHandle().GetValue(),
					obj->m_UniqueID,
					obj->m_vehicleIndex == -1 ? "  " : obj->m_vehicleIndex == 0 ? "PL" : "AI",
					entName.Chars(),
					obj->m_fDistance),
					Maths::RGBA(0xff,0xff,0xff,0xff), 14.f);
				curY += 15.f;
			}
		}
	}
#endif
}

void PickUpSubsystem::DeferredUpdate()
{

	m_camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();

	for (U32 l=0; l<E_PICKUP_LIST_TYPE_MAX; l++)
	{
		//GetPickupCollection(l).m_pickupsWithinMixScaleDistance = 0;
		for (Btl::Map<U64, PickupObject*>::Iterator it=GetPickupCollection(l).m_Map.Begin(); it!= GetPickupCollection(l).m_Map.End(); )
		{
			PickupObject* pObj = it->GetValue();
	
			//update pickup Positions etc..
			UpdatePickup(pObj);

			int objectIndex = g_maxAudiblePerks;

			//check the distances of the projectiles and the number on track,if above the threshold then insert into a distanceMap, sorted by distance
			if(g_distanceScale)
			{
				if(l == E_PICKUP_LIST_TYPE_PROJECTILE &&  GetPickupCollection(E_PICKUP_LIST_TYPE_PROJECTILE).Size() >= (U32)g_maxAudiblePerks)
				{
						m_DistanceMap.Insert(it->GetValue()->m_fDistance,it->GetValue()->m_UniqueID);
				}
			}

			//if(g_mixScale)
			//{
			//	//GetPickupCollection(l).Size();
			//	if(it->GetValue()->m_fDistance < g_ChargeFireMixPriorityDistance)
			//		GetPickupCollection(l).m_pickupsWithinMixScaleDistance++;

			//}
			Btl::Map<U64, PickupObject*>::Iterator toErase = it++;	// increment iterator BEFORE we erase. note: it++ returns the old position not the new one

			// see if any pickups need deleting
			if (!DeleteDoomedObjects(pObj,toErase,l))
			{
				// DeleteDoomedObjects returns false is we didn't delete this object
				if(!m_isSplitScreen && g_DoPickupOcclusion)
				{
					bool performRaytrace = false;
					//sanity check, only do for projectiles
					if(pObj->m_eListType == E_PICKUP_LIST_TYPE_PROJECTILE)
					{
					
						F32 occlusionDistanceRatio  = pObj->m_fDistance / g_PickupMaxRayDistance;
						occlusionDistanceRatio *= occlusionDistanceRatio; 

						F32 elapsedTime = (C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - pObj->m_fTimeOfLastRT);

						if(elapsedTime > occlusionDistanceRatio && occlusionNoRaysPerFrame < PICKUP_MAX_OCCLUSION_RAYS_PER_FRAME)
							performRaytrace = true;

						//only perform if it's the projectile at this position's turn
						if(performRaytrace && pRLC)
						{
							DoOcclusionTest(&pObj->m_Matrix,&m_camMatrix,pObj->m_fDistance,pRLC->GetPhysicsScene(),pObj);
							//reset the time
							pObj->m_fTimeOfLastRT = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();	
						}

					}
				}
			}
		}

		//check if there we are within reach the threshold at with to start effecting the mix
		//if(g_mixScale)
		//{
		//	if(GetPickupCollection(l).m_pickupsWithinMixScaleDistance > g_ChargeFireMixObjectsMax)
		//		LOGINFO(g_AudioPerksLogger, DBSTR"Busy Perk Mix");	

		//}
	}

	//at this point the distanceMap will be full so go through it and suspend all objects placed above the audible threshold number
	if(g_distanceScale)
	{
		PERF_EVENT("Audio.PickupUpdate_distanceScale_loop");

		int size;

		size = m_DistanceMap.Size();
				
		U32 count = 0;
		for (Btl::Map<F32, U64>::Iterator it=m_DistanceMap.Begin(); it!= m_DistanceMap.End(); ++it )
		{

			//looking up value since we've stored the distance as the key, the unique ID is stored as the value
			U64 val = it->GetValue();
			Btl::Map<U64, PickupObject*> *m_PickupMap = GetPickupCollection(E_PICKUP_LIST_TYPE_PROJECTILE).GetMap();
			Btl::Map<U64, PickupObject*>::Iterator MapIt = m_PickupMap->Find(val);

			if(MapIt == m_PickupMap->End())
				continue;

			PickupObject* pObj = MapIt->GetValue();
			
			if(pObj)
			{
				PERF_EVENT("Audio.PickupUpdate_distanceScale_suspends");

				if(count == g_maxAudiblePerks-1)//set scale distance for volume scale
						m_AudibleDistance = pObj->m_fDistance;

				bool focusShock = false;

				if(pObj->m_eType == E_PICKUP_TYPE_SHOCK && pObj->m_firedByFocusVehicle)
					focusShock = true;

				//if object distance is greater than max, just suspsend it
				//we don't want to cull player fired shocks../
				if((count >= (U32)g_maxAudiblePerks || pObj->m_fDistance > g_PickupMaxUpdateDistance)/* && !focusShock */)
				{
					const AudioEntityHandle& entity = pObj->m_hEntity;

					//silence
					//Audio::EntityDataMemberHandle hParamIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "PriorityVolume");
					if (pObj->m_prevPriorityVolume != 0)
					{
						gAic.SetEntityValue(pObj->m_hEntity, pObj->hEntityMemberHandle[PriorityVolume],0);
						pObj->m_prevPriorityVolume = 0;
					}

					if(!gAic.IsEntitySuspended(entity))
					{
						gAic.SuspendEntity(entity);
						
						pObj->m_IsSuspendedPrePause = true;
					}

				}
				else
				{
					const AudioEntityHandle& entity = pObj->m_hEntity;

					if(gAic.IsEntitySuspended(entity))
					{
						gAic.ResumeEntity(pObj->m_hEntity);

						pObj->m_IsSuspendedPrePause = false;
					}				
				}
			}

			count++;			
		}
		
	}		


	 //**debug*** get max concurrent RT's per frame//
	if(!m_isSplitScreen && g_DoPickupOcclusion)
	{
		if(occlusionNoRaysPerFrame > maxOcclusionNoRaysPerFrame)
			maxOcclusionNoRaysPerFrame = occlusionNoRaysPerFrame;

#ifdef PICKUP_DEBUG
		LOGDEBUG(g_AudioPerksLogger, DBSTR"No. RT's this a frame was %d", occlusionNoRaysPerFrame);
#endif
		occlusionNoRaysPerFrame = 0;
	}

}

void DeferredPickupUpdate(Scheduler::Task* pTask)
{
	PERF_EVENT("PickUpSubsystemUpdateTask");
	PickUpSubsystem* pSubSystem = *(PickUpSubsystem**)pTask->ParamData;
	pSubSystem->DeferredUpdate();
}

Debug::Bool g_bDeferredPickupSubSystemUpdate("Audio.Optimisations.PickUpSubsystemDeferredUpdate", false);

JQ_DECLARE_PPU_MODULE(DeferredPickupUpdateTask, DeferredPickupUpdate);

void PickUpSubsystem::Update()
{
	RenderDebug();
	
	if (g_bDeferredPickupSubSystemUpdate && 
		(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() &&
		 C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()))
	{
		PickUpSubsystem* pSubSystem = this;
		BIZ_ASSERT(m_hSchedulerGroup != 0); // make sure you set the scheduler group before running the update
		Scheduler::AddTask(::DeferredPickupUpdateTask, 0, 0, m_hSchedulerGroup, 0, &pSubSystem, sizeof(pSubSystem));
	}
	else
	{
		DeferredUpdate();
	}
	
	m_DistanceMap.Clear();
	


	if(m_removeAllSubscribers)
	{
		m_removeAllSubscribers = false;
	//	RemoveAllSubscribers();
	}

	m_FrameCount = (m_FrameCount % (RAYTRACE_FRAME_SKIP)) + 1;
}


void PickUpSubsystem::UpdatePickup(PickupObject* pObj)
{
	PERF_EVENT("Audio.PickupUpdate_UpdatePickup()");

	if(!pObj->m_hEntity.IsValid())
		return;

	if(pObj->m_vehicleIndex != -1)
	{
		S16 targetIndex =  pObj->m_targetIndex;
		U8 vehicleIndex = (U8) pObj->m_vehicleIndex;

		if(!pRLC)
			return;
		Game::Actors::C_VehicleActor* pActor = pRLC->GetVehicleActor(vehicleIndex);
		if(!pActor)
			return;

		if(!C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer())
			return;

		//Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

		bool isFocusVehicle = false;

		if(pRLC)
		{
			S32 numVehicles = pRLC->GetNumVehicles();

			if(vehicleIndex >= numVehicles)
				return;
		
			S32 numViewports = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();						
			//check all viewports to see if this car has focus
			for(int i = 0; i < (int)numViewports; i++)
			{
				if(vehicleIndex == pRLC->GetViewportFocusVehicleIndex(i))
					 isFocusVehicle = true;
			}

		}
	
		// bombs dont want their position to be the car's position
		if(pObj->m_Follower && pRLC) 
		{
			pObj->m_Matrix = pActor->GetMatrix();
			class RacingPhysics::Car *pCar = pActor->GetVehiclePhysics();

			if(!pCar)
				return;

			Maths::HVec3 objVel = pCar->GetVelocity();

			if (!pObj->m_prevObjVel.All_EQ(objVel))
			{
				gAic.SetEntityVectorIn(pObj->m_hEntity, pObj->hEntityMemberHandle[ObjectVelocity],  (F32*)&objVel);
				pObj->m_prevObjVel = objVel;
			}
		}
										
		Maths::HVec3 objPos = pObj->m_Matrix.AW();
		Maths::HVec3 objDir = pObj->m_Matrix.AZ();
			
		if(pObj->m_Follower || pObj->m_Projectile) 
		{
			if (!pObj->m_prevObjPos.All_EQ(objPos))
			{
				gAic.SetEntityVectorIn(pObj->m_hEntity, pObj->hEntityMemberHandle[ObjectPosition],  (F32*)&objPos);
				pObj->m_prevObjPos = objPos;
			}

			if (!pObj->m_prevObjDir.All_EQ(objDir))
			{
				gAic.SetEntityVectorIn(pObj->m_hEntity, pObj->hEntityMemberHandle[ObjectDirection],  (F32*)&objDir);		
				pObj->m_prevObjDir = objDir;
			}
		}

		////sort out listener position in case of splitscreen etc
		//Maths::HMat43 camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();

		Maths::HVec3 velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();


		Maths::HMat43 camMatrix = m_camMatrix;
		//sort out viewports, get correct listener for view
		if(!pObj->m_Projectile && m_isSplitScreen && (vehicleIndex < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))//max 4 views
		{
			camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Mat43();
			velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Live().Velocity();
		}
		else
		if(pObj->m_Projectile && m_isSplitScreen && (vehicleIndex < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))//max 4 views
		{
			//if it's an AI car, we wish to play the sound from the closest viewport
			int currentView = 0;
			
			F32 nearestDistance = 1000000; //big value
			
			for(int i = 0 ; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); i++)
			{

				Maths::HVec3 entityDistance = ((Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i).DebugFeed().Mat43().AW())  - (objPos));
				F32 length = Maths::Length(entityDistance);
				
				if(length < nearestDistance)
				{
					nearestDistance = length;
					currentView = i;
				}

			}

			camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Mat43();
			velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Live().Velocity();
		}

		Maths::HVec3 fDistance = ((camMatrix.AW())  - (pObj->m_Matrix.AW()));
		pObj->m_fDistance = Maths::Length(fDistance);

		DoPriorityVolumeScaling(pObj);

		gAic.SetEntityVectorIn(pObj->m_hEntity, pObj->hEntityMemberHandle[ListenerPosition], (F32*)(&camMatrix.AW()));
		gAic.SetEntityVectorIn(pObj->m_hEntity, pObj->hEntityMemberHandle[ListenerDirection], (F32*)(&camMatrix.AZ()));		
		gAic.SetEntityVectorIn(pObj->m_hEntity, pObj->hEntityMemberHandle[ListenerUp], (F32*)(&camMatrix.AY()));
		gAic.SetEntityVectorIn(pObj->m_hEntity, pObj->hEntityMemberHandle[ListenerVelocity], (F32*)(&velocity));

		//test occlusion raytrace and fwd on the result
		if(!m_isSplitScreen  && g_DoPickupOcclusion )
			ProcessOcclusionResult(pObj);
	}
}

void PickUpSubsystem::PopulateEntityDefinitions()
{
		//init all the defs
		for(int i = 0;i < E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_MAX; i++)
			pickupEntityDefinition[i] = NULL;



		gAudioPickupString[E_PICKUP_TYPE_NITROUS]	 =		"Nitrous";
		gAudioPickupString[E_PICKUP_TYPE_SHOCK]	 =		"Shock";
		gAudioPickupString[E_PICKUP_TYPE_SHUNT]	 =		"Shunt";
		gAudioPickupString[E_PICKUP_TYPE_BARGE]	 =		"Barge";
		gAudioPickupString[E_PICKUP_TYPE_BOMB]		 =		"Bomb";	

		gAudioPickupString[E_PICKUP_TYPE_BLINDFIRE]		 =		"Blindfire";	
		gAudioPickupString[E_PICKUP_TYPE_REPAIR]		 =		"Repair";	
		gAudioPickupString[E_PICKUP_TYPE_SHIELD]		 =		"Shield";	

		gAudioPickupString[E_PICKUP_TYPE_NITROUS+E_PICKUP_TYPE_MAX]	 =		"NitrousHit";
		gAudioPickupString[E_PICKUP_TYPE_SHOCK+E_PICKUP_TYPE_MAX]		 =		"ShockHit";
		gAudioPickupString[E_PICKUP_TYPE_SHUNT+E_PICKUP_TYPE_MAX]		 =		"ShuntHit";
		gAudioPickupString[E_PICKUP_TYPE_BARGE+E_PICKUP_TYPE_MAX]		 =		"BargeHit";
		gAudioPickupString[E_PICKUP_TYPE_BOMB+E_PICKUP_TYPE_MAX]		 =		"BombHit";	

		gAudioPickupString[E_PICKUP_TYPE_BLINDFIRE+E_PICKUP_TYPE_MAX]	 =		"BlindfireHit";	
		gAudioPickupString[E_PICKUP_TYPE_REPAIR+E_PICKUP_TYPE_MAX]		 =		"RepairHit";	
		gAudioPickupString[E_PICKUP_TYPE_SHIELD+E_PICKUP_TYPE_MAX]		 =		"ShieldHit";

		gAudioPickupString[E_PICKUP_TYPE_NITROUS+E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_PROJECTILE]	 =		"NitrousProjectile";
		gAudioPickupString[E_PICKUP_TYPE_SHOCK+E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_PROJECTILE]	 =			"ShockProjectile";
		gAudioPickupString[E_PICKUP_TYPE_SHUNT+E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_PROJECTILE]	 =			"ShuntProjectile";
		gAudioPickupString[E_PICKUP_TYPE_BARGE+E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_PROJECTILE]	 =			"BargeProjectile";
		gAudioPickupString[E_PICKUP_TYPE_BOMB+E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_PROJECTILE]		 =		"BombProjectile";	

		gAudioPickupString[E_PICKUP_TYPE_BLINDFIRE+E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_PROJECTILE]		 =	"BlindfireProjectile";	
		gAudioPickupString[E_PICKUP_TYPE_REPAIR+E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_PROJECTILE]		 =	"RepairProjectile";	
		gAudioPickupString[E_PICKUP_TYPE_SHIELD+E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_PROJECTILE]		 =	"ShieldProjectile";	

		for(int i = 0;i < E_PICKUP_TYPE_MAX*E_PICKUP_LIST_TYPE_MAX; i++)
		{
			pickupEntityDefinition[i] = Audio::C_XtAudioDb::Get()->LookupXtEntityProxy(gAudioPickupString[i]);

			if (pickupEntityDefinition[i])
				Game::CacheEntityResources(*pickupEntityDefinition[i], "PickupSubsystem");
		}


}

PickUpSubsystem::PickupObject* PickUpSubsystem::CreatePickup( U64 uid,S32 ePickupType, U8 vehicleIndex, E_PICKUP_LIST_TYPE eType)
{
	bool bReleasedEntity = false;

			PickupObject* pObj = GetPickupCollection(eType).LookupById(uid);

			// create a pickup.
			// if there is an entity in the slot and if it was created earlier when playing the charge sound then dont release it and dont create an entity
			// if there is an entity in the slot then release it and create a new one
			// if no entity in the slot then create one

			// if could not lookup pickup by id then create one
			if (!pObj)
			{
				// TODO:: new this?
				if(pickupEntityDefinition[ePickupType]) //if there's a valid definition
					pObj = new PickupObject;
				if (pObj)
				{
					//LOGDEBUG(g_AudioPerksLogger, DBSTR" type %d '%s'. create new slot, uid 0x%x", ePickupType, gAudioPickupString[ePickupType].Chars(), uid);

					GetPickupCollection(eType).m_Map.Insert(uid, pObj);
				}
			}
			else
				LOGINFO(g_AudioPerksLogger, DBSTR" type %d '%s'. pickup already exists, uid 0x%x", ePickupType, gAudioPickupString[ePickupType].Chars(), pObj->m_UniqueID);

			// TODO: deal with no entity slot created
			if (pObj)
			{
				// create entity... only if no entity in the slot
				if(ePickupType < E_PICKUP_TYPE_MAX * E_PICKUP_LIST_TYPE_MAX && pickupEntityDefinition[ePickupType] && !pObj->m_hEntity.IsValid())
				{
					//LOGDEBUG(g_AudioPerksLogger, DBSTR" type %d '%s'. creating entity '%s' with uid 0x%x", ePickupType, gAudioPickupString[ePickupType].Chars(), pickupEntityDefinition[ePickupType]->Name(), uid);

					gAic.CreateEntity(pickupEntityDefinition[ePickupType]->GetEntityTypeHnd(), pObj->m_hEntity);
					//m_EntityInstanceCount++;

		
					pObj->m_markForDelete = false;
					pObj->m_UniqueID = uid;

					//set everything to follow car position
					pObj->m_Follower = true;
					pObj->m_Projectile =false;
					pObj->m_vehicleIndex = -1;
					pObj->m_IsSuspendedPrePause = false;

					pObj->m_eListType = eType;
					pObj->m_translatedType = ePickupType;

					//pObj->m_ArrayIndex = entityIndex;

					Audio::EntityDataMemberHandle hActionOut = gAic.LookupInstanceComponent(pObj->m_hEntity, "EntityFinished");

					Audio::DefaultEntityCallback*  callback = new Audio::DefaultEntityCallback(pObj->m_hEntity,E_PickUpSubsystem_AudioFinished);

					if(hActionOut != -1)
						pObj->m_hQueryEntityFinished = gAic.SetCallback(callback, hActionOut);


					//Audio::DefaultEntityCallback* pCallbackData = new Audio::DefaultEntityCallback(pObj->m_hEntity, E_PickUpSubSystemMessages_PickUpFinished);
					//pObj->m_hQueryEntityFinished = gAic.SetCallback(pCallbackData, hActionOut);

						

					Audio::EntityDefinitionHandle	m_hEntityDefinition = pickupEntityDefinition[ePickupType]->GetEntityDefHnd();
					Audio::EntityDefinition* pDef = gAic.GetEntityManager().GetDefinitionFromHandle(m_hEntityDefinition);

					if (pDef && pDef->LookupInstanceComponent(Audio::StringHash("OutputToRenderer")) != 0xffffffff)
						gAic.SetMaster(pObj->m_hEntity, "OutputToRenderer");

					//connect to car

					//if(audioComponent)
					//{
					//	//Connect(audioComponent->GetEntityHnd(),pObj->m_hEntity);
					//}

					//connect t0 system
					gAic.SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &pObj->m_hEntity);

					m_EntityInstanceCount++;

					for (U32 l=E_PICKUP_LIST_TYPE_CHARGEFIRE; l<E_PICKUP_LIST_TYPE_MAX; l++)
					{
						for (Btl::Map<U64, PickupObject*>::Iterator it=GetPickupCollection(l).m_Map.Begin(); it!= GetPickupCollection(l).m_Map.End(); ++it)
						{
							PickupObject* pObj = it->GetValue();

							if(pObj->m_hEntity.IsValid() && pObj->m_vehicleIndex == vehicleIndex && pObj->m_UniqueID != uid)
							{						
								Audio::EntityDataMemberHandle hActionIn = gAic.LookupInstanceComponent(pObj->m_hEntity, "NewPickupActive");
								gAic.TriggerEntityActionIn(pObj->m_hEntity, hActionIn,false);	
							}
						}
					}

					pObj->m_prevObjPos = Maths::HVec3(Maths::UNDEFINED);
					pObj->m_prevObjDir = Maths::HVec3(Maths::UNDEFINED);
					pObj->m_prevObjVel = Maths::HVec3(Maths::UNDEFINED);
					pObj->m_prevPriorityVolume = FLT_MAX;
					pObj->m_PrevOcclusionVal = -1;

					pObj->hEntityMemberHandle[ObjectVelocity] = gAic.LookupInstanceComponent(pObj->m_hEntity, "ObjectVelocity");
					pObj->hEntityMemberHandle[ObjectPosition] = gAic.LookupInstanceComponent(pObj->m_hEntity, "ObjectPosition");
					pObj->hEntityMemberHandle[ObjectDirection] = gAic.LookupInstanceComponent(pObj->m_hEntity, "ObjectDirection");
					pObj->hEntityMemberHandle[ListenerPosition] = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerPosition");
					pObj->hEntityMemberHandle[ListenerDirection] = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerDirection");
					pObj->hEntityMemberHandle[ListenerUp] = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerUp");
					pObj->hEntityMemberHandle[ListenerVelocity] = gAic.LookupInstanceComponent(pObj->m_hEntity, "ListenerVelocity");
					pObj->hEntityMemberHandle[PriorityVolume] = gAic.LookupInstanceComponent(pObj->m_hEntity, "PriorityVolume");
					pObj->hEntityMemberHandle[OcclusionValue] = gAic.LookupInstanceComponent(pObj->m_hEntity, "OcclusionValue");
					
					if(!m_isSplitScreen && g_DoPickupOcclusion)
					{
						//create occlusion object
						pObj->m_pOcclusionRay =  new Collision::C_Ray; 
						pObj->m_waitingForRaytraceResult = false;
					}		
				}
				else
				{
					pObj->m_waitingForRaytraceResult = false;
					/*if (!bReleasedEntity)
						LOGWARN(g_AudioPerksLogger, DBSTR" type %d '%s'. DIDNT CREATE ENTITY '%s' as entity '%s', id %d, already in slot", ePickupType, gAudioPickupString[ePickupType].Chars(), pickupEntityDefinition[ePickupType]->Name(), pickupEntityDefinition[pObj->m_translatedType]->Name(), pObj->m_UniqueID);
					else
						LOGINFO(g_AudioPerksLogger, DBSTR" type %d '%s'. Dont need to create entity for '%s' as entity '%s', id %d, already in slot", ePickupType, gAudioPickupString[ePickupType].Chars(), pickupEntityDefinition[ePickupType]->Name(), pickupEntityDefinition[pObj->m_translatedType]->Name(), pObj->m_UniqueID);*/
				}
			}
			else
			{
				//LOGERROR(g_AudioPerksLogger, DBSTR" Could not lookup pickup type %d '%s'. DIDNT CREATE ENTITY '%s' as entity '%s', id %d, already in slot", ePickupType, gAudioPickupString[ePickupType].Chars(), pickupEntityDefinition[ePickupType]->Name(), pickupEntityDefinition[pObj->m_translatedType]->Name(), pObj->m_UniqueID);
				LOGERROR(g_AudioPerksLogger, DBSTR" Could not lookup pickup type %d '%s'. DIDNT CREATE ENTITY. could already in slot or else it may not have a definition. see cian", ePickupType, gAudioPickupString[ePickupType].Chars());
			}

			return pObj;
}

void  PickUpSubsystem::Connect(AudioEntityHandle m_hSourceEntity,AudioEntityHandle m_hDestEntity)
{

		Audio::EntityDefinitionHandle hDef = gAic.GetDefinitionHandle(m_hSourceEntity);
		
		Audio::EntityDefinitionHandle hDestDef = gAic.GetDefinitionHandle(m_hDestEntity);

		Audio::EntityDefinition* pDef = gAic.GetEntityManager().GetDefinitionFromHandle(hDestDef);
	
		Audio::LocalFunctionHandle hFunc = gAic.LookupParameterisedFunctionById(hDef, "DoConnections", pDef->GetId());
		//// only try to connect if "DoConnections" func exists
		if (hFunc != 0xffffffff)
		{
			gAic.RunFunctionInParameterisedEntityInstance(m_hSourceEntity, m_hDestEntity  , hFunc);
		} 
}


bool PickUpSubsystem::MessageHandler(const Messaging::C_MessageBase *pMessage )
{
	return true;
}

void PickUpSubsystem::PickupObject::Reset()
{
	if(m_hEntity.IsValid())
	{
		Audio::InterfaceController::Get().ReleaseEntity(m_hEntity);
		m_hEntity = NULL;
	}

	m_Charger = false;

	m_vehicleIndex = -1; //for if vehicle not set, don't process it
	m_targetIndex = -1; //for if vehicle not set, don't process it
	m_hEntity = NULL;
	m_markForDelete = false;
	m_Charger = false;
	m_UniqueID = 0xffffffff;
	m_hQueryEntityFinished = NULL;
	m_fDistance = 0.f;

	m_eType = E_PICKUP_TYPE_NONE;
	m_eListType = E_PICKUP_LIST_TYPE_NONE;
	m_translatedType = -1;
}


void PickUpSubsystem::DoPriorityVolumeScaling(PickupObject* pObj)
{
	PERF_EVENT("Audio.PickupUpdate_PriorityScaling");
	//for (U32 l=E_PICKUP_LIST_TYPE_CHARGEFIRE; l<E_PICKUP_LIST_TYPE_MAX; l++)
	{
		//scale the volume according to distance
		if(pObj->m_Projectile /*&& pObj->m_eType != E_PICKUP_TYPE_SHOCK*/ && GetPickupCollection(E_PICKUP_LIST_TYPE_PROJECTILE).Size() >= (U32)g_maxAudiblePerks)
		{
			F32 volPercent = Maths::Clamp(pObj->m_fDistance / m_AudibleDistance,0.0f,1.0f);				
			volPercent	*= volPercent * volPercent;//powf(volPercent,3.0); //weight the vol scale, cubic	
			F32 volScale = 1.0f - volPercent; //get vol scale	
			if (pObj->m_prevPriorityVolume != volScale)
			{
				gAic.SetEntityValue(pObj->m_hEntity, pObj->hEntityMemberHandle[PriorityVolume],volScale);
				pObj->m_prevPriorityVolume = volScale;
			}
		}
		else
		{
			//play at full volume if not scaling
			if (pObj->m_prevPriorityVolume != 1.f)
			{
				gAic.SetEntityValue(pObj->m_hEntity, pObj->hEntityMemberHandle[PriorityVolume],1.f);
				pObj->m_prevPriorityVolume = 1.f;
			}
		}
	}

}

bool PickUpSubsystem::DeleteDoomedObjects(PickupObject* pObj, Btl::Map<U64, PickupObject*>::Iterator toErase,U32 collectionType )
{
	PERF_EVENT("Audio.PickupUpdate_DeleteDoomedObjects");
	U32 callbackCount = gAic.GetCallbackTriggeredCount(pObj->m_hQueryEntityFinished);

	bool bDelete = false;

	if (pObj->m_markForDelete || callbackCount)
		bDelete = true;

	//time until we auto delete an object
	F32 lifeTime = g_bPickupLife;

	if(pObj->m_eListType == E_PICKUP_LIST_TYPE_PROJECTILE)
		lifeTime = g_bProjectileLife;

	//offset the time for game being paused, only do for initial moment we pause
	if(m_pauseTime == 0 && m_gamePaused)
		m_pauseTime = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();

	//if the time was set this should be the moment we unpause the game
	if(m_pauseTime > 0 && !m_gamePaused)
	{
		//update the object creation time with the period since we have been paused
		pObj->m_fTimeTriggered += C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - m_pauseTime;
		m_pauseTime = 0; //reset the time
	}

	//only do if game paused
	if(!m_gamePaused)
	{
		// sanity check. cull pickups after 10 seconds of triggering a sound
		if ((C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - pObj->m_fTimeTriggered) >= lifeTime)
			bDelete = true;

		if(bDelete)
		{
			if(!m_isSplitScreen && g_DoPickupOcclusion)

			{	//delete the occlusion ray
					if(pObj->m_pOcclusionRay)
					{
						delete pObj->m_pOcclusionRay;
						pObj->m_pOcclusionRay = NULL;				
					}
					else
						LOGERROR(g_AudioPerksLogger, DBSTR" Expecting a valid Raytrace object here, please let cian know");
			}

			pObj->Reset();			
			delete pObj;
			m_EntityInstanceCount--;

			GetPickupCollection(collectionType).m_Map.Erase(toErase);		

			return true;
		}
	}

	return false;
}

void PickUpSubsystem::DoOcclusionTest(Maths::HMat43* raySourceMatrix, Maths::HMat43* rayDestMatrix,F32 distanceToDest, Gravity::C_Scene* scene, PickupObject* obj)
{
	//Fire off a raytrace to test occlusion

	PERF_EVENT("Audio.PickupSubsystem.OcclusionInit");

		if(!obj->m_pOcclusionRay)
			return;
	
		Collision::C_CollisionFilter rayOcclusionCollisionFilter( 0xffffffff, 
																		Game::PolyFlags::kDynamicObjectSolid, 0, 0, 0, 0xffffffff, Game::PolyFlags::kbWorld);		// Probably need to check ONLY for static scene geometry			// Probably need to check ONLY for static scene geometry

		Maths::HVec3 slightRamp(0,(float)g_PickupOcclusionRayHeight,0);//do ray from a metre up
		Maths::HVec3 raySourcePosition = Maths::HVec3_Add(raySourceMatrix->AW(),slightRamp);		
		Maths::HVec3 rayDestPosition = rayDestMatrix->AW();

		//U32 occlusionResult = 0;
		F32 rayDistance = distanceToDest;
		
		obj->m_pOcclusionRay->InitialiseRay(raySourcePosition,rayDestPosition,&rayOcclusionCollisionFilter);
	
		//m_pOcclusionRay->InitialiseRay(raySourcePosition, (Maths::C_Normal)rayDirection, (float)g_OcclusionRayDistanceMax, &rayOcclusionCollisionFilter, 0.1f);

		scene->DoDeferredRayTrace(obj->m_pOcclusionRay,NULL,m_Group,m_Policy);

		obj->m_waitingForRaytraceResult = true;

		occlusionNoRaysPerFrame++;

		#ifndef CFG_GOLD
		//DEBUG DEBUG DEBUG
		if(g_RenderPickupOcclusionRays)
		{

			Maths::HVec3 rayDirectionTemp = (rayDestMatrix->AW() - raySourcePosition);
			Maths::HVec3 rayDirection = Normalised(rayDirectionTemp);

			S32 red = 0xff;
			S32 green = 0xff;
			S32 blue = 0xff;
			F32 radius = 1;

			Maths::RGBA colourYellow = Maths::RGBA(red, green, 0x00, 0xff);
			Maths::RGBA colourRed = Maths::RGBA(red, 0x00, 0x00, 0x00);
			Maths::RGBA colourBlue = Maths::RGBA(0x00, 0x00, blue, 0xff);
			Maths::RGBA colourGreen = Maths::RGBA(0x00, green, 0x00, 0xff);

			//image source green
			Maths::RGBA occColour = colourGreen;

			//if(occlusionResult)
			//	occColour = colourRed;

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(raySourcePosition,radius,occColour);
		
			//image direction
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(raySourcePosition, rayDirection, rayDistance, 1, occColour);
				
		}
		#endif
}

void PickUpSubsystem::ProcessOcclusionResult(PickupObject * pObj)
{
	PERF_EVENT("Audio.PickupSubsystem.OcclusionResult");

	if(pObj->m_waitingForRaytraceResult)
	{
		Collision::C_RayTracingResult& 	result = pObj->m_pOcclusionRay->m_result;
		//if occluded by mesh
		if (result.Hit())
		{
			if (pObj->m_PrevOcclusionVal != 1)
			{
				gAic.SetEntityValue(pObj->m_hEntity, pObj->hEntityMemberHandle[OcclusionValue],1);
				pObj->m_PrevOcclusionVal = 1;
			}
		}
		else
		{
			if (pObj->m_PrevOcclusionVal != 0)
			{
				gAic.SetEntityValue(pObj->m_hEntity, pObj->hEntityMemberHandle[OcclusionValue],0);
				pObj->m_PrevOcclusionVal = 0;
			}
		}

		pObj->m_waitingForRaytraceResult = false;		
	}
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF