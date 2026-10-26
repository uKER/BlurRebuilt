#include "stdafx.h"

#include "GameAudioLayer.h"
//#include "StringHash.h"

#include "AppInterfaceLayer.h"
#include "CollisionSubsystem.h"
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

//#include <Game/Messaging/CollisionActorMessages.h>

#include "GameLogic/RaceLogicCore/Core/RaceLogicCore.h"

#include "Game\GameActors\Components\AudioComponents.h"

#include <Camera2.h>

#include "Game\GameActors\Components\AudioMessageHandlers\BaseMessageHandler.h"

#include <GameLogic\Profile\GameProfileManager.h>
#include <GameLogic/Profile/GameProfile.h>
#include "GameLogic/Profile/MultiplayerProfile.h"

#include <Ui\game\Multiplayer\MultiplayerDefines.h>

#include <SchedulerConfig.h>

#ifdef AUDIO_OPTIMIZE_OFF
#undef optimize
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF

#define USE_INPLACE_ENTITIES

Debug::Logger &g_AudioCollisionsLogger( Debug::GetLogger("Audio.Game.Perks") );
//
Debug::Bool gbCollisionMessages("Audio.Subsystem.CollisionMsgsEnabled", true);
//
Debug::Bool g_bLoadCollisions("Audio.Subsystem.LoadSubsystem.LoadCollisions", true);

Debug::Int g_bMaxNumberCollision("Audio.Subsystem.Collisions.MaxNoCollisions", 10);
//
Util::C_String gAudioCollisionString[E_COLLISION_TYPE_MAX];

Debug::Int g_deleteTimeLimit("Audio.Subsystem.Collisions.DeleteTimeLimit", 10);

//Audio::InterfaceController& Audio::InterfaceController::Get() = Audio::InterfaceController::Get();
Debug::Bool g_bDoWorldCollisions("Audio.Subsystem.Collisions.DoWorldCollisions", true );
Debug::Bool g_bDoCarCollisions("Audio.Subsystem.Collisions.DoCarCollisions", true );
Debug::Bool g_bDoScrapes("Audio.Subsystem.Collisions.DoScrapes", true );

Debug::Bool g_singleCollision("Audio.Subsystem.Collisions.DoSingleCollision", false);

Debug::Bool g_DoCollisionOcclusion("Audio.Subsystem.Collisions.DoCollisionOcclusion", true );
Debug::Float g_CollisionOcclusionRayHeight("Audio.Subsystem.Collisions.OcclusionRayHeight", 0.53);




Scheduler::GroupHandle CollisionSubsystem::m_hSchedulerGroup = 0;

U32 occlusionCollisionNoRaysPerFrame = 0;
U32 maxOcclusionCollisionNoRaysPerFrame = 0;

void CollisionSubsystem::SetSchedulerGroup(Scheduler::GroupHandle handle)
{
	m_hSchedulerGroup = handle;
}
//-----------------------------------------------------------------------------
// CollisionSubsystem::CollisionSubsystem
//-----------------------------------------------------------------------------
CollisionSubsystem::CollisionSubsystem()
{
	for (S32 i = 0; i < E_COLLISION_TYPE_MAX; ++i)
		CollisionEntityDefinition[i] = NULL;

	m_removeAllSubscribers = false;
	m_EntityInstanceCount = 0;
	m_bRaceInProgress = false;

	m_uniqueCollisionID = 0;

	m_haEntityHandles[E_ParticleScrapeOffset] = 0xffffffff;

	m_activeCollisions = 0;
	
	m_Group = Scheduler::JQ_GROUP_NONE;
	m_Policy = 0;
}

//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------
bool CollisionSubsystem::ProcessSystemMessage(const U32 u32MessageId)
{
	if(!gbCollisionMessages)
		return false;

	switch (u32MessageId)
	{

		case Audio::eInit:
			{
				if(!g_bLoadCollisions)
					break;

				// load all dats & wavebanks for Collisions
				PopulateEntityDefinitions();
			}
			break;

			case Audio::eBeginLevel:
			{

				if(!g_bLoadCollisions)
					break;

				m_CollisionsLocal.Reset();
				m_CollisionsWorld.Reset();
				m_CollisionsLocal_AI.Reset();
				m_CollisionsWorld_AI.Reset();
				m_CollisionsScrape.Reset();
				m_CollisionsScrape_AI.Reset();

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
				m_MaxCollisionsThisRace = 0;
				m_uniqueCollisionID = 0;
				m_activeCollisions = 0;

				//get slot abilities
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

				m_Policy = Game::C_SchedulerConfig::GetInstance()->GetDeferredRaytracePolicy();
				m_Group = Scheduler::CreateGroup(false);
				

#ifdef USE_INPLACE_ENTITIES
				CreateEntityCollections();
#endif
			}
			break;

		case Audio::eEndLevel:
			{
				if(!g_bLoadCollisions)
					break;
				// release entities and destroy any active perks
				for (U32 l=0; l<E_COLLISION_TYPE_MAX; l++)
				{
					for (Btl::Map<U64, CollisionObject*>::Iterator it=GetCollisions(l).m_Map.Begin(); it!= GetCollisions(l).m_Map.End(); )
					{

						CollisionObject* pObj = it->GetValue();

						if(!m_isSplitScreen && g_DoCollisionOcclusion  && l > E_COLLISION_TYPE_SCRAPE)
						{	//delete the occlusion ray
								if(pObj->m_pOcclusionRay)
								{
									delete pObj->m_pOcclusionRay;
									pObj->m_pOcclusionRay = NULL;				
								}
								else
									LOGERROR(g_AudioCollisionsLogger, DBSTR" Expecting a valid Raytrace object here, please let cian know");
						}

						Btl::Map<U64, CollisionObject*>::Iterator toErase = it++;	// increment iterator BEFORE we erase. note: it++ returns the old position not the new one
						pObj->Reset();
						GetCollisions(l).m_Map.Erase(toErase);
						delete pObj;
					}
				}

				m_EntityInstanceCount = 0;
				m_activeCollisions = 0;

				m_removeAllSubscribers = true;
				m_bRaceInProgress = false;			

				Scheduler::Flush(m_Group);

				m_Policy = 0;
				Scheduler::DestroyGroup(m_Group);

				LOGINFO(g_AudioCollisionsLogger, DBSTR"Max No. Collision RT's on a frame was %d", maxOcclusionCollisionNoRaysPerFrame);
			}
			break;
		case Audio::eMute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,0);
				}
			}
			break;

		case Audio::eUnmute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,1);
				}
			}
			break;

		case Audio::eSuspend:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::InterfaceController::Get().SuspendEntity(pInstance->GetAudioEntityHandle());
				}
			}

			break;

		case Audio::eResume:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					//Audio::InterfaceController::Get().ResumeEntity(pInstance->GetAudioEntityHandle());
				}
			}

			break;

		case Audio::eRaceFinished:			
			m_bRaceInProgress = false;			
			break;	
	}
	return true;
}
 
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ProcessCustomMessage
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CollisionSubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data , void* pData)
{

	if (!gbCollisionMessages || !g_bLoadCollisions)
		return false;

		switch(u32MessageId)
		{
			case E_CollisionSubsystem_Impact:
			case E_CollisionSubsystem_Scrape:
			{

				if(pData && m_bRaceInProgress)
				{
					Components::VehicleMessageHandler::CollisionParams* pCollisionParams = (Components::VehicleMessageHandler::CollisionParams*)pData;

					//u32Data value is current vehicle index
					
					//sanity check
					if(u32Data == -1)
						u32Data = 0;

					U8 vehicleIndex = u32Data;

					Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

					bool isFocusVehicle = false;

					if(pRLC && !m_isSplitScreen)
					{
						S32 numVehicles = pRLC->GetNumVehicles();
						
						if(vehicleIndex == pRLC->GetViewportFocusVehicleIndex(0))
							 isFocusVehicle = true;
					}

					//the collision type will change based on if it's a focusCar or not
					E_COLLISION_TYPE overriddenType = E_COLLISION_TYPE_NONE;
					// create the Collision if it doesn't exist, go through all collision types and spawn if neccesary
					for (U32 collType=E_COLLISION_TYPE_LOCAL; collType<E_COLLISION_TYPE_MAX; collType++)
					{

						CollisionObject* pObj = NULL;

#ifdef USE_INPLACE_ENTITIES

						//TODO, test here if we need a world collision (DO or car - car collision), could send a separate message
						if (g_bDoCarCollisions && collType == E_COLLISION_TYPE_LOCAL && u32MessageId == E_CollisionSubsystem_Impact)
						{
							overriddenType = (E_COLLISION_TYPE)collType;

							//if it's a focus car, spawn the normal entity, else, spawn the AI version
							if(!isFocusVehicle)
								overriddenType = E_COLLISION_TYPE_LOCAL_AI;

							pObj = GetNextCollisionObject(overriddenType);
						

						}
						else
						if (g_bDoWorldCollisions && collType == E_COLLISION_TYPE_WORLD && u32MessageId == E_CollisionSubsystem_Impact)
						{
							//only trigger if we have actually hit the world
							if(pCollisionParams->m_CollidesWith == Components::VehicleMessageHandler::AUDIO_COLLISION_COLLIDES_WITH_WORLD)
							{
								overriddenType = (E_COLLISION_TYPE)collType;

								//if it's a focus car, spawn the normal entity, else, spawn the AI version
								if(!isFocusVehicle)
									overriddenType = E_COLLISION_TYPE_WORLD_AI;

								pObj = GetNextCollisionObject(overriddenType);

							}
						}
						else
						if (g_bDoScrapes && collType == E_COLLISION_TYPE_SCRAPE && u32MessageId == E_CollisionSubsystem_Scrape)
						{

							overriddenType = (E_COLLISION_TYPE)collType;

							//if it's a focus car, spawn the normal entity, else, spawn the AI version
							if(!isFocusVehicle)
							{
								overriddenType = E_COLLISION_TYPE_SCRAPE_AI;
								pObj = GetNextCollisionObject(overriddenType);
							}
							else
							{
								//the player car has two scrape entities, we want to keep them as separate left/right scrapes
								// so if the scrape is on the lef tside of the car, use entity 0, otherwise use entity 1
								U32 scrapeID = 0;
								//if scrape is right side of car, use 2nd entity
								if(pCollisionParams->m_Position.X() > 0.f)
									scrapeID = 1;
	
								//special case so not using GetNextCollisionObject to return the object, lookup using the
								// left/right entity index
								CollisionCollection& coll =  GetCollisions(overriddenType);
								pObj = coll.LookupById(scrapeID);

							}
								
						}
						else
							continue;


#else
						//TODO, test here if we need a world collision (DO or car - car collision), could send a separate message
						if (g_bDoCarCollisions && collType == E_COLLISION_TYPE_LOCAL && u32MessageId == E_CollisionSubsystem_Impact)
						{
							E_COLLISION_TYPE type = (E_COLLISION_TYPE)collType;

							//if it's a focus car, spawn the normal entity, else, spawn the AI version
							if(!isFocusVehicle)
								type = E_COLLISION_TYPE_LOCAL_AI;


							pObj = CreateCollision(GetUniqueId(vehicleIndex, m_uniqueCollisionID), type, vehicleIndex, isFocusVehicle);
						

						}

						if (g_bDoWorldCollisions && collType == E_COLLISION_TYPE_WORLD && u32MessageId == E_CollisionSubsystem_Impact)
						{
							//only trigger if we have actually hit the world
							if(pCollisionParams->m_CollidesWith == Components::VehicleMessageHandler::AUDIO_COLLISION_COLLIDES_WITH_WORLD)
							{
								E_COLLISION_TYPE type = (E_COLLISION_TYPE)collType;

								//if it's a focus car, spawn the normal entity, else, spawn the AI version
								if(!isFocusVehicle)
									type = E_COLLISION_TYPE_WORLD_AI;

								pObj = CreateCollision(GetUniqueId(vehicleIndex, m_uniqueCollisionID), type, vehicleIndex,isFocusVehicle);

							}
						}

						if (g_bDoScrapes && collType == E_COLLISION_TYPE_SCRAPE && u32MessageId == E_CollisionSubsystem_Scrape)
						{

							E_COLLISION_TYPE type = (E_COLLISION_TYPE)collType;

							//if it's a focus car, spawn the normal entity, else, spawn the AI version
							if(!isFocusVehicle)
								type = E_COLLISION_TYPE_SCRAPE_AI;
		
							pObj = CreateCollision((U64)vehicleIndex, type, vehicleIndex,isFocusVehicle);

						}

#endif

						if (pObj)
						{

#ifdef USE_INPLACE_ENTITIES

							if(pObj->m_hEntity.IsValid() && Audio::InterfaceController::Get().IsEntitySuspended(pObj->m_hEntity))
							{
								Audio::InterfaceController::Get().ResumeEntity(pObj->m_hEntity);
							}


#endif

							pObj->m_vehicleIndex = vehicleIndex;
						
							Audio::EntityDataMemberHandle  hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "IsFocusCar");
							Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, hParamIn,  (F32)isFocusVehicle);

							//	set collision position (in car space)
							pObj->m_Matrix.AW(pCollisionParams->m_Position);

							//	set collision object to update position with car
							if (overriddenType == E_COLLISION_TYPE_LOCAL || overriddenType == E_COLLISION_TYPE_LOCAL_AI)
								pObj->m_Follower = true;
							else
								pObj->m_Follower = false;

							//make the collisionType the overridden type
							pObj->m_eType = (E_COLLISION_TYPE)overriddenType;

							Audio::EntityDataMemberHandle hVectorIn;
							
							//	get collision position in world space
							Maths::HVec3 objPos = pCollisionParams->m_Position * pRLC->GetVehicleActor(pObj->m_vehicleIndex)->GetMatrix();


							//	AUDIOTODO: shouldn't look these up a runtime
							//collision Vectors
							hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "ObjectPosition");
							Audio::InterfaceController::Get().SetEntityVectorIn(pObj->m_hEntity, hVectorIn,  (F32*)&objPos);

							F32 upValue = pRLC->GetVehicleActor(pObj->m_vehicleIndex)->GetMatrix().AY().Y();

							hParamIn  = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "CarUpValue");
							Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, hParamIn , (F32) upValue);

							//sort out listener position in case of splitscreen etc
							Maths::HMat43 camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();

							Maths::HVec3 velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();

							//sort out viewports, get correct listener for view
							if(m_isSplitScreen && (vehicleIndex < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))//max 4 views
							{
								camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Mat43();
								velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Live().Velocity();
							}

							hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "ListenerPosition");
							Audio::InterfaceController::Get().SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AW()));

							hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "ListenerDirection");
							Audio::InterfaceController::Get().SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AZ()));		

							hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "ListenerUp");
							Audio::InterfaceController::Get().SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AY()));

							hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "ListenerVelocity");
							Audio::InterfaceController::Get().SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&velocity));

							//collision Parameters //TODO need to work out whihc is the profer 'surface' material, effects or physics?
							hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "CollisionSurfaceType");
							Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, hParamIn,  (F32)pCollisionParams->m_MaterialType);

							hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "CollisionForce");
							Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, hParamIn,  (F32)pCollisionParams->m_fMaxImpulseStrength);

							hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "CollidesWith");
							Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, hParamIn,  (F32)pCollisionParams->m_CollidesWith);//hard code to world collision

							F32 groundSpeed = pRLC->GetVehicleActor(vehicleIndex)->GetVehiclePhysics()->GetVelocity().Length();

							hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "GroundSpeed");
							Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, hParamIn,  (F32)groundSpeed);

							hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "IsShielded");
							Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, hParamIn,  (F32)pCollisionParams->m_Shielded);

							if (u32MessageId == E_CollisionSubsystem_Scrape)
							{
								//scrape Action Ins
								Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "ParticleScrape");
								Audio::InterfaceController::Get().TriggerEntityActionIn(pObj->m_hEntity, hActionIn);	

							}
							else
							{
								//collision Action Ins
								Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "CollisionActionIn");
								Audio::InterfaceController::Get().TriggerEntityActionIn(pObj->m_hEntity, hActionIn);	
							}
							
							pObj->m_fTimeTriggered = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();

							//fire off occlusion trace. don't do if splitScreen or focus car collision
							if(!m_isSplitScreen && g_DoCollisionOcclusion && pObj->m_eType > E_COLLISION_TYPE_SCRAPE)
							{
								//create collision matrix using vehicke
								Maths::HMat43  collisionMatrix = pRLC->GetVehicleActor(pObj->m_vehicleIndex)->GetMatrix();
								collisionMatrix.AW(objPos);


								DoOcclusionTest(&collisionMatrix,&camMatrix,pObj->m_fDistance,pRLC->GetPhysicsScene(),pObj);	

							}

						}

					}

					if(!g_singleCollision)
						m_uniqueCollisionID++;

				}//if data
			}
			break;
		}
	return false;
}


#ifndef CFG_GOLD
Debug::Float gfCollisionOffsetDebugY("Audio.Subsystem.Collisions.OffsetY", 130.f);
Debug::Bool gbCollisionDebug("Audio.Subsystem.Collisions.RenderDebug", false);
#endif

void CollisionSubsystem::RenderDebug()
{
#ifndef CFG_GOLD
	if (gbCollisionDebug)
	{
		// render nice little green square.
		// without this the text doesnt render. dont know why! answers on a postcard...
		Maths::HVec2 pos( 0, 0);
		Maths::HVec2 size( 10.0f,  10.f);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(pos,size,Maths::RGBA(0x00, 0xff, 0x00, 0xff));

		F32 curY;

		for (U32 l=0; l<E_COLLISION_TYPE_MAX; l++)
		{
			curY = gfCollisionOffsetDebugY;

			for ( Btl::Map<U64, CollisionObject*>::Iterator it = GetCollisions(l).m_Map.Begin(); it != GetCollisions(l).m_Map.End(); ++it )
			{
				CollisionObject* obj = it->GetValue();
				Btl::String entName;

				Audio::EntityInstance* pInstance = Audio::InterfaceController::Get().GetEntityInstanceFromHandle(obj->m_hEntity);
				if (pInstance)
				{
					Audio::EntityDefinition* pDef =  Audio::InterfaceController::Get().GetDefinition(pInstance);
					if (pDef)
						entName = pDef->Name();
				}

				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(l==E_COLLISION_TYPE_LOCAL?100:350,curY), Util::C_String().Printf("hEnt %8x uid %llx %s %10s d %.2f", 
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

void CollisionSubsystem::Update()
{
#ifdef USE_INPLACE_ENTITIES
//	if(m_activeCollisions<=0)
	//	return;
#endif

	if (!gbCollisionMessages || !g_bLoadCollisions)
		return;

	RenderDebug();

	F32 oldestTime = 0;
	
	for (U32 l=0; l<E_COLLISION_TYPE_MAX; l++)
	{
		// see if any Collisions need deleting
		for (Btl::Map<U64, CollisionObject*>::Iterator it=GetCollisions(l).m_Map.Begin(); it!= GetCollisions(l).m_Map.End(); )
		{
			CollisionObject* pObj = it->GetValue();

			Btl::Map<U64, CollisionObject*>::Iterator toErase = it++;	// increment iterator BEFORE we erase. note: it++ returns the old position not the new one
	
			if(!Audio::InterfaceController::Get().IsEntitySuspended(pObj->m_hEntity))
			{

				U32 callbackCount = Audio::InterfaceController::Get().GetCallbackTriggeredCount(pObj->m_hQueryEntityFinished,true);

			//	if(l == E_COLLISION_TYPE_WORLD) 
				//	pObj->m_markForDelete = true;

				bool bDelete = false;

				if (/*pObj->m_markForDelete &&*/ callbackCount)
					bDelete = true;

				// sanity check. cull Collisions after 10 seconds of triggering a sound
				F32 timeSinceTriggered = (C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - pObj->m_fTimeTriggered);

	#ifndef USE_INPLACE_ENTITIES
				if (/*(pObj->m_eType != MAX_COLLISION_TYPE_BOMB) && */timeSinceTriggered >= g_deleteTimeLimit)
					bDelete = true;
	#endif


				if(bDelete)
				{
	#ifdef USE_INPLACE_ENTITIES

					if(pObj->m_hEntity.IsValid()/* && !Audio::InterfaceController::Get().IsEntitySuspended(pObj->m_hEntity)*/)
					{
						Audio::InterfaceController::Get().SuspendEntity(pObj->m_hEntity);
					}
					//m_activeCollisions--;

	#else
					//LOGDEBUG(g_AudioCollisionsLogger, DBSTR" Finished action out received '%s', uid 0x%x, destroying.",  gAudioCollisionString[pObj->m_translatedType].Chars(), pObj->m_UniqueID);
					pObj->Reset();
					
					delete pObj;

					m_EntityInstanceCount--;

					GetCollisions(l).m_Map.Erase(toErase);

	#endif
					
				}
			}
		}
	}

	for (U32 l=0; l<E_COLLISION_TYPE_MAX; l++)
	{
		// now update all Collisions
		for ( Btl::Map<U64, CollisionObject*>::Iterator it = GetCollisions(l).m_Map.Begin(); it != GetCollisions(l).m_Map.End(); ++it )
			UpdateCollision(it->GetValue());
	}

	if(m_removeAllSubscribers)
	{
		m_removeAllSubscribers = false;
	//	RemoveAllSubscribers();
	}
}


void CollisionSubsystem::UpdateCollision(CollisionObject* pObj)
{
	if(!pObj->m_hEntity.IsValid())
		return;

	if(pObj->m_vehicleIndex != -1)
	{
		S16 targetIndex =  pObj->m_targetIndex;
		U8 vehicleIndex = (U8) pObj->m_vehicleIndex;

		if(!C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer())
			return;

		Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

		bool isFocusVehicle = false;

		Audio::EntityDataMemberHandle hVectorIn;

		Maths::HVec3 objPos = pObj->m_Matrix.AW() * pRLC->GetVehicleActor(pObj->m_vehicleIndex)->GetMatrix();

		if(pRLC)
		{
			S32 numVehicles = pRLC->GetNumVehicles();

			if(vehicleIndex >= numVehicles)
				return;
		
			if(vehicleIndex == pRLC->GetViewportFocusVehicleIndex(0))
				 isFocusVehicle = true;

			if(pObj->m_Follower) 
			{
				hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "ObjectPosition");
				Audio::InterfaceController::Get().SetEntityVectorIn(pObj->m_hEntity, hVectorIn,  (F32*)&objPos);

			}

			if (pObj->m_eType == E_COLLISION_TYPE_SCRAPE || pObj->m_eType == E_COLLISION_TYPE_SCRAPE_AI)
			{
				//loops starts at different time to prevent phasing when there's a car to car scrape
				F32 fParticleScrapeOffset = (F32)vehicleIndex / (F32)numVehicles;
				Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, m_haEntityHandles[E_ParticleScrapeOffset], fParticleScrapeOffset);
			}
		}
		
		//sort out listener position in case of splitscreen etc
		Maths::HMat43 camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();

		Maths::HVec3 velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();


		//sort out viewports, get correct listener for view
		if(m_isSplitScreen && (vehicleIndex < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount()))//max 4 views
		{
			camMatrix = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Mat43();
			velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(vehicleIndex).DebugFeed().Live().Velocity();
		}

		Maths::HVec3 fDistance = ((camMatrix.AW())  - (pObj->m_Matrix.AW()));
		pObj->m_fDistance = Maths::Length(fDistance);


		//Maths::HVec3 camPos = view.DebugFeed().Mat43().AW();

		//Maths::HMat43 camMatrix = view.DebugFeed().Mat43();

		hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "ListenerPosition");
		Audio::InterfaceController::Get().SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AW()));

		hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "ListenerDirection");
		Audio::InterfaceController::Get().SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AZ()));		

		hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "ListenerUp");
		Audio::InterfaceController::Get().SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&camMatrix.AY()));

		hVectorIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "ListenerVelocity");
		Audio::InterfaceController::Get().SetEntityVectorIn(pObj->m_hEntity, hVectorIn, (F32*)(&velocity));

		//test occlusion raytrace and fwd on the result if nonFocus car (assume focus is never occluded)
		if(!m_isSplitScreen && g_DoCollisionOcclusion  && pObj->m_eType > E_COLLISION_TYPE_SCRAPE)
			ProcessOcclusionResult(pObj);

		//if (pObj->m_fDistance > 250.f && pObj->m_translatedType != MAX_COLLISION_TYPE_BOMB)
		//	LOGDEBUG(g_AudioCollisionsLogger, DBSTR" THIS PERK IS OUT OF RANGE. uid 0x%x, '%s'.", pObj->m_UniqueID, gAudioCollisionString[pObj->m_translatedType].Chars());

	}

 //**debug*** get max concurrent RT's per frame//
	if(!m_isSplitScreen && g_DoCollisionOcclusion  && pObj->m_eType > E_COLLISION_TYPE_SCRAPE)
	{
		if(occlusionCollisionNoRaysPerFrame > maxOcclusionCollisionNoRaysPerFrame)
			maxOcclusionCollisionNoRaysPerFrame = occlusionCollisionNoRaysPerFrame;

		LOGDEBUG(g_AudioCollisionsLogger, DBSTR"No. RT's this a frame was %d", occlusionCollisionNoRaysPerFrame);

		occlusionCollisionNoRaysPerFrame = 0;
	}
}

void CollisionSubsystem::CreateEntityCollections()
{
#ifdef USE_INPLACE_ENTITIES
	for (U32 eCollisionType=0; eCollisionType<E_COLLISION_TYPE_MAX; eCollisionType++)
	{
		CollisionCollection& coll =  GetCollisions(eCollisionType);

		//set the appropiate no. collisions per collection type
		switch(eCollisionType)
		{	
			case E_COLLISION_TYPE_LOCAL:
				coll.m_maxCollisionsForCollection = MAX_COLLSION_ENTITIES_LOCAL_PLAYER;
				break;

			case E_COLLISION_TYPE_WORLD:
				coll.m_maxCollisionsForCollection = MAX_COLLSION_ENTITIES_WORLD_PLAYER;
				break;


			case E_COLLISION_TYPE_SCRAPE:
				coll.m_maxCollisionsForCollection = MAX_SCRAPE_ENTITIES_PLAYER;
				break;

			case E_COLLISION_TYPE_LOCAL_AI:
				coll.m_maxCollisionsForCollection = MAX_COLLSION_ENTITIES_LOCAL_AI;
				break;


			case E_COLLISION_TYPE_WORLD_AI:
				coll.m_maxCollisionsForCollection = MAX_COLLSION_ENTITIES_WORLD_AI;
				break;

			case E_COLLISION_TYPE_SCRAPE_AI:
				coll.m_maxCollisionsForCollection = MAX_SCRAPE_ENTITIES_AI;
				break;
		}

		

		for(U32 collKey = 0; collKey < coll.m_maxCollisionsForCollection;collKey++)
		{
			CreateCollision(collKey,eCollisionType,0,false);			
		}
	}
	
#endif
}

CollisionSubsystem::CollisionObject* CollisionSubsystem::GetNextCollisionObject(E_COLLISION_TYPE eCollisionType)
{
	CollisionCollection& coll =  GetCollisions(eCollisionType);

	CollisionObject* pObj = coll.LookupById(coll.m_nextCollisionIndex);

	coll.m_nextCollisionIndex = ++coll.m_nextCollisionIndex % coll.m_maxCollisionsForCollection;

	//m_activeCollisions++;

	return pObj;
}

void CollisionSubsystem::PopulateEntityDefinitions()
{
		//init all the defs
		for(int i = 0;i < E_COLLISION_TYPE_MAX; i++)
		{
			CollisionEntityDefinition[i] = NULL;
		}


		gAudioCollisionString[E_COLLISION_TYPE_LOCAL]	 =		"CollisionCar";
		gAudioCollisionString[E_COLLISION_TYPE_WORLD]	 =		"CollisionWorld";
		gAudioCollisionString[E_COLLISION_TYPE_SCRAPE]	 =		"CollisionScrape";

		gAudioCollisionString[E_COLLISION_TYPE_LOCAL_AI]	 =		"CollisionCar_AI";
		gAudioCollisionString[E_COLLISION_TYPE_WORLD_AI]	 =		"CollisionWorld_AI";
		gAudioCollisionString[E_COLLISION_TYPE_SCRAPE_AI]	 =		"CollisionScrape_AI";


		for(int i = 0;i < E_COLLISION_TYPE_MAX; i++)
		{
			CollisionEntityDefinition[i] = Audio::C_XtAudioDb::Get()->LookupXtEntityProxy(gAudioCollisionString[i]);

			if (CollisionEntityDefinition[i])
			{
				Game::CacheEntityResources(*CollisionEntityDefinition[i], "CollisionSubsystem");
			}
		}

		//	AUDIOTODO: suggested optimisation - lookup entity data handles here
		//	get entity handles
		if(CollisionEntityDefinition[E_COLLISION_TYPE_SCRAPE])
			m_haEntityHandles[E_ParticleScrapeOffset] = Audio::InterfaceController::Get().LookupInstanceComponent(CollisionEntityDefinition[E_COLLISION_TYPE_SCRAPE]->GetEntityDefHnd(), "ParticleScrapeOffset");
}

CollisionSubsystem::CollisionObject* CollisionSubsystem::CreateCollision( U64 uid, U32 eCollisionType, U8 vehicleIndex, bool isFocusVehicle)
{
			bool bReleasedEntity = false;

			CollisionCollection coll =  GetCollisions(eCollisionType);

#ifndef USE_INPLACE_ENTITIES

			S32 size = coll.m_Map.Size();

			///if we are over the max allowed collisions, just drop it (unless focus car)
			if(!isFocusVehicle && size >= g_bMaxNumberCollision)
				return NULL;
#endif

			//AUDIOTODO add priority system if needed like below
			//if we have too many collision in this list, delete oldest collsion

			//get new collision object
			CollisionObject* pObj = GetCollisions(eCollisionType).LookupById(uid);

			// create a Collision.


			// if could not lookup Collision by id then create one
			if (!pObj)
			{
				// TODO:: new this?
				pObj = new CollisionObject;
				if (pObj)
				{
					//LOGDEBUG(g_AudioCollisionsLogger, DBSTR" type %d '%s'. create new slot, uid 0x%x", eCollisionType, gAudioCollisionString[eCollisionType].Chars(), uid);

					GetCollisions(eCollisionType).m_Map.Insert(uid, pObj);
				}
			}
		
			// TODO: deal with no entity slot created
			if (pObj)
			{
				// create entity... only if no entity in the slot
				if(CollisionEntityDefinition[eCollisionType] && !pObj->m_hEntity.IsValid())
				{
					
					Audio::InterfaceController::Get().CreateEntity(CollisionEntityDefinition[eCollisionType]->GetEntityTypeHnd(), pObj->m_hEntity);

					pObj->m_markForDelete = false;
					pObj->m_UniqueID = uid;

					//set everything to follow car position for now TODO Cian, change this for local/world collisions
					pObj->m_Follower = true;
					pObj->m_vehicleIndex = -1;

					pObj->m_translatedType = eCollisionType;

					Audio::EntityDataMemberHandle hActionOut = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "EntityFinished");

					Audio::DefaultEntityCallback*  callback = new Audio::DefaultEntityCallback(pObj->m_hEntity,E_CollisionSubsystem_AudioFinished);

					if(hActionOut != -1)
						pObj->m_hQueryEntityFinished = Audio::InterfaceController::Get().SetCallback(callback, hActionOut);

			
					Audio::EntityDefinitionHandle	m_hEntityDefinition = CollisionEntityDefinition[eCollisionType]->GetEntityDefHnd();
					Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(m_hEntityDefinition);

					if (pDef && pDef->LookupInstanceComponent(Audio::StringHash("OutputToRenderer")) != 0xffffffff)
						Audio::InterfaceController::Get().SetMaster(pObj->m_hEntity, "OutputToRenderer");

					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "StartActionIn");
					Audio::InterfaceController::Get().TriggerEntityActionIn(pObj->m_hEntity, hActionIn);	

					//connect to car

				//	Game::Actors::RaceLogicCore* pRLC	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

				//	const Components::I_AudioEntity* audioComponent = (Components::I_AudioEntity*)pRLC->GetVehicleActor(vehicleIndex)->QueryInterface(Components::E_AudioEntity_AdditionalSpawned);


					//if(audioComponent)
					//{
					//	//Connect(audioComponent->GetEntityHnd(),pObj->m_hEntity);
					//}

					//connect t0 system
					Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::eSystemSubsystem, SystemSubsystem::eConnectToSystem, 0, &pObj->m_hEntity);

					m_EntityInstanceCount++;

#ifdef USE_INPLACE_ENTITIES
					Audio::InterfaceController::Get().SuspendEntity(pObj->m_hEntity);
#endif

					Audio::EntityDataMemberHandle hEntityIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "AbilitySlot1");
					Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)abilitySlots[0]);

					hEntityIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "AbilitySlot2");
					Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)abilitySlots[1]);

					hEntityIn = Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "AbilitySlot3");
					Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, hEntityIn,  (F32)abilitySlots[2]);

					if(!m_isSplitScreen && g_DoCollisionOcclusion  && eCollisionType > E_COLLISION_TYPE_SCRAPE)
					{
						//create occlusion object					
						BIZ_ASSERT(pObj->m_pOcclusionRay == NULL);
						pObj->m_pOcclusionRay =  new Collision::C_Ray; 
						pObj->m_waitingForRaytraceResult = false;
					}

				}

			}
			else
			{
				LOGERROR(g_AudioCollisionsLogger, DBSTR" Could not lookup Collision type %d '%s'. DIDNT CREATE ENTITY '%s' as entity '%s', id %d, already in slot", eCollisionType, gAudioCollisionString[eCollisionType].Chars(), CollisionEntityDefinition[eCollisionType]->Name(), CollisionEntityDefinition[pObj->m_translatedType]->Name(), pObj->m_UniqueID);
			}

			return pObj;
}

void  CollisionSubsystem::Connect(AudioEntityHandle m_hSourceEntity,AudioEntityHandle m_hDestEntity)
{

		Audio::EntityDefinitionHandle hDef = Audio::InterfaceController::Get().GetDefinitionHandle(m_hSourceEntity);
		
		Audio::EntityDefinitionHandle hDestDef = Audio::InterfaceController::Get().GetDefinitionHandle(m_hDestEntity);

		Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(hDestDef);
	
		Audio::LocalFunctionHandle hFunc = Audio::InterfaceController::Get().LookupParameterisedFunctionById(hDef, "DoConnections", pDef->GetId());
		//// only try to connect if "DoConnections" func exists
		if (hFunc != 0xffffffff)
		{
			Audio::InterfaceController::Get().RunFunctionInParameterisedEntityInstance(m_hSourceEntity, m_hDestEntity  , hFunc);
		} 
}


bool CollisionSubsystem::MessageHandler(const Messaging::C_MessageBase *pMessage )
{
	return true;
}

void CollisionSubsystem::CollisionObject::Reset()
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

	m_eType = E_COLLISION_TYPE_NONE;
	//m_eListType = eListTypeNone;
	m_translatedType = -1;
}

CollisionSubsystem::CollisionCollection& CollisionSubsystem::GetCollisions(U32 uType)
{ 

	switch(uType)
	{
		case E_COLLISION_TYPE_LOCAL:
			return m_CollisionsLocal;
			break;

		case E_COLLISION_TYPE_WORLD:
			return m_CollisionsWorld;
			break;

		case E_COLLISION_TYPE_SCRAPE:
			return m_CollisionsScrape;
			break;

		case E_COLLISION_TYPE_LOCAL_AI:
			return m_CollisionsLocal_AI;
			break;

		case E_COLLISION_TYPE_WORLD_AI:
			return  m_CollisionsWorld_AI;
			break;

		case E_COLLISION_TYPE_SCRAPE_AI:
			return m_CollisionsScrape_AI;
			break;

		default:
			return m_CollisionsLocal;
	}


	 
}

void CollisionSubsystem::DoOcclusionTest(Maths::HMat43* raySourceMatrix, Maths::HMat43* rayDestMatrix,F32 distanceToDest, Gravity::C_Scene* scene, CollisionObject* obj)
{
	//Fire off a raytrace to test occlusion

	PERF_EVENT("Audio.PickupSubsystem.OcclusionInit");
	
		Collision::C_CollisionFilter rayOcclusionCollisionFilter( 0xffffffff, 
			Game::PolyFlags::kDynamicObjectSolid, 0, 0, 0, 0xffffffff, Game::PolyFlags::kbWorld);			// Probably need to check ONLY for static scene geometry

		Maths::HVec3 slightRamp(0,(float)g_CollisionOcclusionRayHeight,0);//do ray from a metre up
		Maths::HVec3 raySourcePosition = Maths::HVec3_Add(raySourceMatrix->AW(),slightRamp);		
		Maths::HVec3 rayDestPosition = rayDestMatrix->AW();

		//U32 occlusionResult = 0;
		F32 rayDistance = distanceToDest;
		
		obj->m_pOcclusionRay->InitialiseRay(raySourcePosition,rayDestPosition,&rayOcclusionCollisionFilter);
	
		//m_pOcclusionRay->InitialiseRay(raySourcePosition, (Maths::C_Normal)rayDirection, (float)g_OcclusionRayDistanceMax, &rayOcclusionCollisionFilter, 0.1f);

		scene->DoDeferredRayTrace(obj->m_pOcclusionRay,NULL,m_Group,m_Policy);

		obj->m_waitingForRaytraceResult = true;
}

void CollisionSubsystem::ProcessOcclusionResult(CollisionObject * pObj)
{
	PERF_EVENT("Audio.PickupSubsystem.OcclusionResult");

	if(g_DoCollisionOcclusion && pObj->m_waitingForRaytraceResult  && pObj->m_eType > E_COLLISION_TYPE_SCRAPE)
	{
		Collision::C_RayTracingResult& 	result = pObj->m_pOcclusionRay->m_result;
		//if occluded by mesh
		if (result.Hit())
		{
			
			Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "OcclusionValue"),1);
		}
		else
		{
			Audio::InterfaceController::Get().SetEntityValue(pObj->m_hEntity, Audio::InterfaceController::Get().LookupInstanceComponent(pObj->m_hEntity, "OcclusionValue"),0);
		}

		pObj->m_waitingForRaytraceResult = false;
		occlusionCollisionNoRaysPerFrame++;
	}
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF


