#include "stdafx.h"

#include "PickupPositionActor.h"

#include <GameLogic\GameModes\GameModeDefines.h>

#include <Game/ActorDef/GameActor.h>

#include "../../Game/Messaging/GameMessages.h"
#include "../../Game/Messaging/VehicleActorMessages.h"
#include "../../Game/Messaging/PickupActorMessages.h"
#include <Game/Messaging/GameflowMessages.h>

#include "../RaceLogicCore/Database/RLDatabase.h"
#include "../RaceLogicCore/Core/RaceLogicCore.h"

#include "../Layers/RaceBookLayer.h"
#include "../Layers/AmaxWorldLayer.h"

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/GameComponents.h>
#include <Game/GameActors/Components/VehicleLogic/VLC_FanDemands.h>

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <game\gameactors\components\messages\VehicleLogicMessages.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game\GameActors\Components\VehicleLogic\VLC_Pickups.h>
#include <GameLogic\AchievementsManager.h>

#include <GameApp.h>
#include <SpecialFX/SpecialFX.h>

#include <Gravity/SDK/ShapeDescriptors.h>
#include <Gravity/Triggers/TriggerSystem.h>

#include <shared_actors/SharedActorUtils.h>
#include <GameLogic/Profile/GameProfileManager.h>

#include <Game/GameUtil/ResourceBag.h>
#include "Game/GameActors/Components/VehicleLogic/VLC_Pickups_AI.h"

#include <actor_studio_msg_game/ActorStudioMessaging.h>

#include <shared_actors/Utility/VisibilityDetection.h>
#include <ui/game/UILocaliser.h>

#include <ai/AI_Amax.h>
#include <GameLogic/MultiplayerUtilities.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameLogic/ActorStudioActors/FanDemandPositionActor.h>
#include <GameLogic/MultiplayerUtilities.h>

#include <Game/GameWorld/ViewType.h>

#include "Game/GameActors/Components/Creation/AudioComponentsCreate.h"
#include <Game/Game.h>


Debug::Logger &g_PickupPositionLogger( Debug::GetLogger("Pickups.PickupPositionActor") );

Debug::Bool		gDebugEnforceRandomPickups("Perks.PickupActor.EnforceRandomPickups_NonPersist", false);

Debug::Float	gMaxRespawnTime("Perks.PickupActor.MaxRespawnTime", -1, Debug::Persist());
Debug::Float	gMinRespawnTime("Perks.PickupActor.MinRespawnTime", -1, Debug::Persist());
Debug::Int		gDebugPickupActiveLevel("Perks.PickupActor.ActiveLevelNonPersit", -1);
Debug::Bool		gDebugPickupCheckActiveLevel("Perks.PickupActor.ActiveLevelCheck", false);
Debug::Int		gDebugPickupActiveLevelForDebug("Perks.PickupActor.ActiveLevelForDebugRace", 1);
Debug::Float	gDebugPickupInitialDisableTime("Perks.PickupActor.InitialDisableTime_NonPersist", 0);
Debug::Bool		gRespawnPickups("Perks.PickupActor.RespawnPickups", false);

Debug::Float	gDebugRotationSpeed("Perks.PickupActor.RotationSpeed", 0.1f);

Debug::Bool		gDebugFixRandomPerks("Perks.PickupActor.FixRandomPerks_NonPersist", true);
Debug::Float	gDebugShuffleTime("Perks.PickupActor.ShuffleTime", 5.f);
Debug::Float	gDebugShuffleRotateTime("Perks.PickupActor.ShuffleRotateTime", 1.02f);
Debug::Float	gDebugShuffleRotateSpeedModifier("Perks.PickupActor.ShuffleRotateSpeedModifier", 1.f);
Debug::Float	gDebugShuffleRotateSpeedMin("Perks.PickupActor.ShuffleRotateSpeedMin", 0.f);
Debug::Float	gDebugShuffleRotateSpeedMax("Perks.PickupActor.ShuffleRotateSpeedMax", 1.f);

Debug::Bool		gDebugDisableNitrous("Perks.PickupActor.DisableNitrous_NonPersist", false);
Debug::Bool		gDebugDisableShock("Perks.PickupActor.DisableShock_NonPersist", false);
Debug::Bool		gDebugDisableShunt("Perks.PickupActor.DisableShunt_NonPersist", false);
Debug::Bool		gDebugDisableBlindfire("Perks.PickupActor.DisableBlindfire_NonPersist", false);
Debug::Bool		gDebugDisableBarge("Perks.PickupActor.DisableBarge_NonPersist", false);
Debug::Bool		gDebugDisableBomb("Perks.PickupActor.DisableBomb_NonPersist", false);
Debug::Bool		gDebugDisableRepair("Perks.PickupActor.DisableRepair_NonPersist", false);
Debug::Bool		gDebugDisableShield("Perks.PickupActor.DisableShield_NonPersist", false);

Debug::Bool		gDebugForceEqualSpawnProbability("Perks.PickupActor.ForceEqualProbability_NonPersist", false);
Debug::Bool		g_RenderPickupPositions("RLC.DebugLines.ShowPickupPositions", false);
Debug::Bool		g_debugOnlyDestroyPickUpIfCanHave("RLC.Pickups.OnlyDestroyPickupIfCanHave", true);
Debug::Bool		g_AllowMultiplePickupsPerLap("Perks.PickupActor.AllowMultiplePerLap_NonPersist", false);
Debug::Bool		gDebugPickupActorShowDebugInfo("Perks.PickupActor.ShowDebugInfo", false);
Debug::Bool		g_debugPickupsUpdateFromSorcery("Amax.Perks.UpdateFromSorcery", false, Debug::Persist());

Debug::Float	g_debugPickupActorTimeToAllowNextPickup("Perks.PickupActor.TimeToAllowNextPickup", 1.0f);

namespace Actors
{
	PickupPositionCreate::PickupPositionCreate() : m_OneShot(false), m_DroppedFromVehicleIndex(-1), m_AllowRandomPickupOverride(true), m_ForceSelectedType(false)
	{
	}

	PickupPositionActor::PickupPositionActor() :
		m_BeginTime(0.0f),
		m_EndTime(1.0f),
		m_Duration(1.0f), 
		m_pTriggerObject(NULL),
		m_pGameWorld(NULL), 
		m_pVisDetect(NULL), 
		m_bVisible(false),
		m_PickedUpByLocalPlayer(false),
		m_TimesPickedUpByLocalPlayer(0),
		m_UniqueDropID(-1),
		m_InsideCameraTriggerRegion(0),
		m_CampingPreventionEnabled(false),
		m_NumSuccessiveCollectionsBySameVehicle(0),
		m_LastVehicleIdxCollectedBy(-1),
		m_pPickupSpecialFX(NULL),
		m_PickupSpecialFXTime(0.f)
	{

	}

	PickupPositionActor::~PickupPositionActor()
	{
		Destroy();
	}

	bool PickupPositionActor::InitialiseImpl()
	{
		//Enable pickups by default
		mPickupType = E_PICKUP_TYPE_NONE;

		mpLevelState = &Game::C_Game::GetInstance()->GetLevelState();
		BIZ_ASSERT(mpLevelState);

		//NonRandomPickup override
		bool RandomPickupOverride = mpLevelState->mOverrideNonRandomPickups;

		//Check that I am allowed to override the non random pickups
		if(!((PickupPositionCreate*)GetCreationData())->m_AllowRandomPickupOverride)
		{
			RandomPickupOverride = false;
		}
		
		if( (!RandomPickupOverride) && ((PickupPositionCreate*)GetCreationData())->m_pPickupType )
		{
			AmaxEvent* pAmaxEv = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentEvent();
			mPickupType = ((PickupPositionCreate*)GetCreationData())->m_pPickupType->meType;

			if(mpLevelState->mDisableFanDemands)
			{
				if(mPickupType == E_PICKUP_TYPE_FAN)
				{
					mPickupType = E_PICKUP_TYPE_NONE; //We dont want any fan demands - set to none
				}
			}

			if(pAmaxEv && !GetCreationData()->m_ForceSelectedType)
			{	
				if(pAmaxEv->pickupProbabilities)
				{
					for( int i = 0; i < pAmaxEv->pickupProbabilities->pickupProbabilities.count; i++ )
					{
						if( pAmaxEv->pickupProbabilities->pickupProbabilities[i].pickupType == mPickupType && !pAmaxEv->pickupProbabilities->pickupProbabilities[i].probability )
						{
							mPickupType = E_PICKUP_TYPE_NONE;
							break;
						}
					}
				}
			}				
		}

		mEnabled = mpLevelState->mPickupsEnabled;

		mSemiHidden = false;
		mDisableDemand = false;
		m_Selected = false;
		mNeedShuffle = false;
		m_Respawn = true;
		mConstantShuffle = GetCreationData()->m_bConstantShuffle;
		mShuffleFlags = GetCreationData()->m_uShuffleTypes.value;
		mRotationSpeed = 0.f;
		mIsNetwork = C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork();

		mWorldMatrix = Load(GetCreationData()->m_WorldMatrix);
		mInitialMatrix = mWorldMatrix;
		mpRaceLogicLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		m_pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

		const RLPickupType *pPickupType = GetCreationData()->m_pPickupType;

		mpRaceLogicLayer->IncrementTotalLivePickups(GetHandle());

		if( !mpLevelState->mPerksRespawn )
		{
			m_Respawn = false;
			m_OneShot = false;
		}
		else
		{
			m_OneShot = GetCreationData()->m_OneShot;
			m_OneShotTime = GetCreationData()->m_OneShotTime;
		}
		mRotateGraphic = GetCreationData()->m_bRotateGraphic;
		m_ActivationTimer = GetCreationData()->m_fActivationTimer;
		m_ActivationLap = GetCreationData()->m_iActivationLap;
		m_PickupOnLapZero = !(AIAmax::AmaxAIR::Instance().IsDataInitialised());

		m_UniqueDropID = GetCreationData()->m_UniqueDropID;

		mCollisionRadius = m_pRLC->GetDatabase()->mRLSystems[0]->mPickupSystem->mParameters->mPickupRadius;

		if (pPickupType)
		{
			m_BeginTime = pPickupType->m_PickupSpecialFXBeginCycle;
			m_EndTime	= pPickupType->m_PickupSpecialFXEndCycle;
		}
		else
		{
			m_BeginTime = 0.0f;
			m_EndTime	= 1.0f;
		}

		for( int i = 0; i < MAX_VEHICLES; i++ )
		{
			if(m_PickupOnLapZero)
			{
				mVehicleCollectedLap[i] =-1;
			}
			else
			{
				mVehicleCollectedLap[i] =0;
			}
		}

		if (m_BeginTime == m_EndTime)
			m_EndTime = m_BeginTime + 1.0f;
		m_Duration = m_EndTime - m_BeginTime;

		mLastPickedUpPos = -1;

		AddSubscriber( GameMessaging::E_GameMsg_Update );
		AddSubscriber( GameMessaging::E_GameMsg_PreUpdate ); // Only used for constant shuffle
		AddSubscriber( Game::E_GameLogicMessage_LapIncremented );
		
//		if (g_debugUsePhysicsTriggersForPickups)
			AddSubscriber( Game::E_PickupMessage_EnteredPickup );
			AddSubscriber( Game::E_PickupMessage_ExitedPickup );
//		else
//			AddSubscriber( Game::E_VehicleMessage_VehicleWorldPosition );		

		if (mPickupType == E_PICKUP_TYPE_FAN)
			AddSubscriber( Game::E_PickupMessage_UpdateFanDemandState );

		AddSubscriber( Game::E_PickupMessage_EnablePickupType );
		AddSubscriber( Game::E_PickupMessage_DisablePickupType );
		AddSubscriber( Game::E_PickupMessage_DisableAllPickupTypes );
		AddSubscriber( Game::E_PickupMessage_EnableAllPickupTypes );

		AddSubscriber( Game::E_PickupMessage_SetPickupPositionId );
		AddSubscriber( ActorMsg::E_DynamicObjMsg_SetVehicleLogic );		

		AddSubscriber( E_VehicleLogicMsg_Ghosted);
		AddSubscriber( E_VehicleLogicMsg_Deghosted);

		AddSubscriber( Game::E_GameflowMessage_RaceStarted );
		AddSubscriber( Game::E_GameflowMessage_RaceLoaded );
		AddSubscriber( Game::E_PickupMessage_ActivatePickups );
		AddSubscriber( Game::E_PickupMessage_DebugForcePickupRespawn );

		AddSubscriber( Game::E_GameflowMessage_ClearStoredActorPtrs );		

		AddSubscriber( Game::E_PickupMessage_KillAllPickups );

		mPickupActive = false;
		mReSpawnTimer = 9999; // High value, always disabled during cutscene.
		mTimeSinceRespawned = 0.0f;

		// Disable any respawn delay multiplayers if not picked up for certain time period. 
		MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
		BIZ_ASSERT_MSG(pConfig," PickupPositionActor:InitialiseImpl - error mulitplayer Config invalid");
		mCampingPreventionTimeout = pConfig->m_CampingPreventionTimeout;
		mCheckCampingPreventionTimeout = false;

		mId = -1;
		mAllowPickup = false;

		mAllowPickup = mpRaceLogicLayer->PowerupsActivated();

		if(m_OneShot)
		{
			//[PK] need to do something different here
		}
		else
		{
			mpRaceLogicLayer->AddPickupPositionActor(GetCreationData()->m_SorceryUid, GetHandle());
		}

		mShuffleTimer		= 0.0f;
		mSpawnDelayTime		= mpRaceLogicLayer->mPickupSpawnInfo.respawnTime;
		mRotateSpeed		= gDebugRotationSpeed;

		m_pVisDetect = Actors::C_VisibilityDetector::CreateVisDetect(GetHandle());

		////create audio components
		Component::C_ComponentCreatorFactoryHelper<Components::C_AudioEntity_SpawnedCreateInfo> pCreator;

		//hard coding it to use one entity for all pickup position actors...
		if(Audio::C_XtAudioDb::Get())
			pCreator->m_AudioEntityDef =  Audio::C_XtAudioDb::Get()->LookupXtEntityProxyHandle("PickupPosition");

		// This doesn't seem to do anything other than bail (successfull) and stop all the important logic underneath from getting run, 
		// let's just warn rather than bail.
		if(!pCreator->m_AudioEntityDef)
			LOGWARN(Game::g_Log, "Failed to create audio entity def for PickupPositionActor");

		if (InitialiseComponent(pCreator.Get()) == NULL)
		{
			LOGERROR(Game::g_Log, "Failed to create audio component for PickPosition actor");
			return false;
		}

		if (mPickupType == E_PICKUP_TYPE_FAN)
		{
			FanDemandPositionCreationData* fdcreate = (FanDemandPositionCreationData*)GetCreationData();
			mDisableDemand = fdcreate->mFanDemand.start_deactivated;
		}
		
		mNextPickupType = mPickupType;

		if(mPickupType > E_PICKUP_TYPE_NONE)
			mpRaceLogicLayer->mLivePickups[mPickupType]++;

		SetupVisualPickupIcon();

		// If motor mash event.. (ask sorcery for event flag??)
		if(mpLevelState->mRaceMode == Game::E_RaceMode_Multiplayer_Destruction)
		{
			// then enable camping prevention (increases respawn multiplier if same person repeatedly picks up)
			m_CampingPreventionEnabled = true;
		}

		for( int i = 0; i < mVehiclesCollidedThisFrame.Size(); i++ )
			mVehiclesCollidedThisFrame[i].mCollided = false;

		for( int i = 0; i < mVehicleTimeToAllowPickup.Size(); i++ )
			mVehicleTimeToAllowPickup[i] = 0.f;

		return true;
	}

	void PickupPositionActor::SetupVisualPickupIcon()
	{
		FanDemandPositionCreationData* fdcreate = (FanDemandPositionCreationData*)GetCreationData();
		const RLPickupType* pPickup				= m_pRLC->GetPickupType(mPickupType);

		if(pPickup)
		{
			
			if (m_pPickupSpecialFX)
			{
				m_pPickupSpecialFX->Stop();
				SAFE_DELETE_PTR(m_pPickupSpecialFX);
			}
		

			SpecialFX::I_SpecialFXInstance* pickupFX = NULL;

			if (mPickupType == E_PICKUP_TYPE_FAN)
			{
				if (fdcreate->mFanDemand.gold_demand)
				{
					if (mSemiHidden && pPickup->mPickupSpecialFXGoldenFaded.Object())				// Faded out version used for Fan Demands
						pickupFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pPickup->mPickupSpecialFXGoldenFaded, m_pRLC->GetGameWorld()->GetContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
					else if(pPickup->mPickupSpecialFXGolden.Object())
						pickupFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pPickup->mPickupSpecialFXGolden, m_pRLC->GetGameWorld()->GetContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
				}
				else if (fdcreate->mFanDemand.use_aggressive_icon)
				{
					if (mSemiHidden && pPickup->mPickupSpecialFXAggressiveFaded.Object())					// Faded out version used for Fan Demands
						pickupFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pPickup->mPickupSpecialFXAggressiveFaded, m_pRLC->GetGameWorld()->GetContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
					else if(pPickup->mPickupSpecialFXAggressive.Object())
						pickupFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pPickup->mPickupSpecialFXAggressive, m_pRLC->GetGameWorld()->GetContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
				}
				else
				{
					if (mSemiHidden && pPickup->mPickupSpecialFXFaded.Object())					// Faded out version used for Fan Demands
						pickupFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pPickup->mPickupSpecialFXFaded, m_pRLC->GetGameWorld()->GetContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
					else if(pPickup->mPickupSpecialFX.Object())
						pickupFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pPickup->mPickupSpecialFX, m_pRLC->GetGameWorld()->GetContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
				}
			}
			else
			{
				if (mSemiHidden && pPickup->mPickupSpecialFXFaded.Object())					// Faded out version used for Fan Demands
					pickupFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pPickup->mPickupSpecialFXFaded, m_pRLC->GetGameWorld()->GetContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
				else if(pPickup->mPickupSpecialFX.Object())
					pickupFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pPickup->mPickupSpecialFX, m_pRLC->GetGameWorld()->GetContextProvider(), C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), m_pVisDetect);
			}


			if(pickupFX)
			{
				m_pPickupSpecialFX = pickupFX;
			}
		}
	}

	//DEBUG DEBUG DEBUG
	void PickupPositionActor::InitialiseDebugLineVertices()
	{
		//Go through each pickup and create a tetra hedron to show where the pick up is and its effective radius
		Maths::RGBA colour, green, red, blue, cyan, white, yellow;
		
		yellow = Maths::RGBA(0xff, 0xff, 0x00, 0xff);
		cyan = Maths::RGBA(0x00, 0xff, 0xff, 0xff);
		green = Maths::RGBA(0x00, 0xff, 0x00, 0xff);
		blue = Maths::RGBA(0x00, 0x00, 0xff, 0xff);
		red = Maths::RGBA(0xff, 0x00, 0x00, 0xff);
		white = Maths::RGBA(0xff, 0xff, 0xff, 0xff);

		Maths::HVec3 pos, x, y, z, v1, v2, v3, v4;
		float radius = 0.f;

		switch(mPickupType)
		{
		case E_PICKUP_TYPE_NITROUS:
			mColour = green;
			break;
		case E_PICKUP_TYPE_SHOCK:
			mColour = blue;
			break;
		case E_PICKUP_TYPE_SHUNT:
			mColour = red;
			break;
		case E_PICKUP_TYPE_REPAIR:
			mColour = cyan;
			break;
		case E_PICKUP_TYPE_BOMB:
			mColour = white;
			break;
		case E_PICKUP_TYPE_BARGE:
			mColour = yellow;
			break;
		}
			
		radius = mCollisionRadius;

		x = mWorldMatrix.AX();
		y = mWorldMatrix.AY();
		z = mWorldMatrix.AZ();

		pos = mWorldMatrix.AW();
		pos += (y*radius*sin(Maths::Pi/6.f));

		mV1 = pos + (z*radius) - (y*radius*sin(Maths::Pi/6.f));
		mV2 = pos - (z*radius*sin(Maths::Pi/6.f)) + (x*radius*cos(Maths::Pi/6.f)) - (y*radius*sin(Maths::Pi/6.f));
		mV3 = pos - (z*radius*sin(Maths::Pi/6.f)) - (x*radius*cos(Maths::Pi/6.f)) - (y*radius*sin(Maths::Pi/6.f));
		mV4 = pos + (y*radius);
	}
	//DEBUG DEBUG DEBUG

	bool PickupPositionActor::CreateImpl()
	{
		InitialiseDebugLineVertices();

		//ShowPickupSpecialFX();
		
		CreateTrigger(mCollisionRadius);

		return true;
	}

	void PickupPositionActor::CreateTrigger(float p_Radius)
	{
		if (m_pTriggerObject)
			DestroyTrigger();

		if (!m_pRLC || !m_pRLC->GetGameWorld())
			return;

		m_pGameWorld = m_pRLC->GetGameWorld();

		Gravity::C_TriggerLayer* pTriggerLayer = m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Pickup);
		if (!pTriggerLayer)
			return;

		m_pTriggerObject	= new Gravity::C_Trigger;

		if (!m_pTriggerObject)
			return;

		m_pTriggerObject->SetUserData( SharedActorUtils::HandleToUserData( GetHandle() ) );

		Collision::C_CollisionFilter filter;

		if(mPickupType == E_PICKUP_TYPE_FAN)
		{
			Gravity::C_BoxDesc desc;
			desc.m_HalfExtents=Maths::HVec3(2.5f,2.5f,0.5f);
			m_pTriggerObject->m_pTriggerShape = desc.CreateShape(&filter);
		}
		else
		{
			Gravity::C_BoxDesc desc;
			if(mRotateGraphic)
				desc.m_HalfExtents=Maths::HVec3(1.25f,1.25f,1.25f);
			else
				desc.m_HalfExtents=Maths::HVec3(1.25f,1.25f,0.25f);

			m_pTriggerObject->m_pTriggerShape = desc.CreateShape(&filter);

			//Gravity::C_SphereDesc desc;
			//desc.m_Radius = p_Radius;
			//m_pTriggerObject->m_pTriggerShape = desc.CreateShape(&filter);
		}
		
		pTriggerLayer->AddTrigger(m_pTriggerObject);
		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);

		//pTriggerLayer->SetTriggerMatrix(m_pTriggerObject, mWorldMatrix, stack);
		m_pTriggerObject->SetMatrix(mWorldMatrix, stack);
	}

	void PickupPositionActor::DestroyTrigger()
	{
		if (!m_pTriggerObject || !m_pGameWorld)
			return;

		Gravity::C_TriggerLayer* pTriggerLayer = m_pGameWorld->GetTriggerLayer(Game::E_TriggerLayer_Pickup);
		if (!pTriggerLayer)
			return;

		Actor::C_ActorHandle defaultHandle;
		m_pTriggerObject->SetUserData( SharedActorUtils::HandleToUserData( defaultHandle ) );
		
		pTriggerLayer->RemoveTrigger(m_pTriggerObject);

		SAFE_DELETE_PTR(m_pTriggerObject);
	}

	void PickupPositionActor::PreUpdate(float frameTime)
	{

		PERF_EVENT("PickupPositionActor.PreUpdate");

		if(mConstantShuffle && mReSpawnTimer <= 0)
		{
			mShuffleTimer -= frameTime;
			if( mShuffleTimer <= -gDebugShuffleRotateTime )
			{
				mShuffleTimer = mpRaceLogicLayer->mPickupSpawnInfo.shuffleTime;
				mRotateGraphic = false;
				mWorldMatrix = mInitialMatrix;
			}
			else if( mShuffleTimer <= 0 )
			{
				if(mNeedShuffle)
				{
					if(mPickupType > E_PICKUP_TYPE_NONE)
						mpRaceLogicLayer->mLivePickups[mPickupType]--;

					GenerateNextPickupType();

					mNeedShuffle = false;

					HideAllPickupSpecialFX();
					SetupVisualPickupIcon();
					ShowAllPickupSpecialFX();							
				}

				mRotateSpeed = (gDebugShuffleRotateTime - Maths::Abs(mShuffleTimer)) * gDebugShuffleRotateSpeedModifier;
				mRotateSpeed = Maths::Clamp(mRotateSpeed, (F32)gDebugShuffleRotateSpeedMin, (F32)gDebugShuffleRotateSpeedMax);
			}
			else if(mShuffleTimer <= gDebugShuffleRotateTime)
			{
				if(!mRotateGraphic)
					mRotateGraphic = true;

				mNeedShuffle = true;
				mRotateSpeed = (gDebugShuffleRotateTime - mShuffleTimer) * gDebugShuffleRotateSpeedModifier;
				mRotateSpeed = Maths::Clamp(mRotateSpeed, (F32)gDebugShuffleRotateSpeedMin, (F32)gDebugShuffleRotateSpeedMax);
			}
		}

		float closestToVeh	= -1.f;
		int closestArrayIdx	= -1;
		float dist			= 0.f;

		for( int i = 0; i < mVehiclesCollidedThisFrame.Size(); i++ )
		{
			if( mVehiclesCollidedThisFrame[i].mCollided && CheckPickup(mVehiclesCollidedThisFrame[i]))
			{
				dist = Maths::Distance(mVehiclesCollidedThisFrame[i].mWorldPosition, mWorldMatrix.AW());

				int vehIdx										= mVehiclesCollidedThisFrame[i].mVehicleIndex;
				Game::Actors::C_VehicleLogicActor* pLogicActor	= mpRaceLogicLayer->GetLogicVehicleActor(vehIdx);
				Game::Actors::C_VehicleActor* pVehActor			= pLogicActor->GetGraphicalActor(vehIdx);

				Maths::HVec3 vel = pVehActor->GetCollisionBody()->GetBody().m_LinearVelocity;
				Maths::HVec3 lineEnd = mVehiclesCollidedThisFrame[i].mWorldPosition + vel;

				Maths::C_Vertex outPt;
				Maths::C_Normal outNorm;
				Maths::HF32 outDist;

				Maths::LineClosestPoint(mVehiclesCollidedThisFrame[i].mWorldPosition, lineEnd, mWorldMatrix.AW(), outPt, outNorm, outDist);
				dist += outDist;

				LOGDEBUG(g_PickupPositionLogger, "Vehicle %i contending for pickup %u with dist %f (line %f)", vehIdx, mId, dist, (float)outDist);

				if( closestToVeh == -1.f || dist < closestToVeh )
				{
					closestToVeh	= dist;
					closestArrayIdx	= i; 
				}
			}
		}

		if(closestArrayIdx != -1)
		{
			LOGDEBUG(g_PickupPositionLogger, "Vehicle %i won pickup %u", mVehiclesCollidedThisFrame[closestArrayIdx].mVehicleIndex, mId);
			GrantPickup(mVehiclesCollidedThisFrame[closestArrayIdx]);
		}

		for( int i = 0; i < mVehicleTimeToAllowPickup.Size(); i++ )
		{
			if(mVehicleTimeToAllowPickup[i] > 0.f)
				mVehicleTimeToAllowPickup[i] -= frameTime;
		}
	}

	void PickupPositionActor::Update(const float frameTime)
	{
		PERF_EVENT("PickupPositionActor.Update");

		if(mPickupType == E_PICKUP_TYPE_FAN)			// Handle fan demands
		{
			if (mReSpawnTimer > 0.0f)
			{
				mReSpawnTimer -= frameTime;
				if (mReSpawnTimer <= 0.0f)
				{
					if (!mPickupActive)
					{
						mPickupActive = true;
						SetFanDemandRenderState();
					}
				}
			}
			else
			{
				if (!m_bVisible)									// If not being rendered then don't subscribe to update
					RemoveSubscriber(GameMessaging::E_GameMsg_Update);
			}
		}
		else
		{
			if(mPickupActive)							// Handle perk pickups
			{
				mReSpawnTimer = 0.f;							// Only used for one shot pickups
				if(m_OneShot && m_Respawn)
				{
					m_OneShotTime -= frameTime;
					if(m_OneShotTime <= 0)
					{
						//Destroy();
						Kill();
					}
				}
			}
			else 
			{
				if((mReSpawnTimer - frameTime) > 0.f)
				{
					mReSpawnTimer -= frameTime;
					mPickupActive = false;						// Make sure it's hidden if still counting down respawn timer

					HideAllPickupSpecialFX();
				}
				else
				{
					if (!m_bVisible)
						RemoveSubscriber(GameMessaging::E_GameMsg_Update);

					if(mPickupType != E_PICKUP_TYPE_NONE)
					{
						mPickupActive = true;
						mReSpawnTimer = 0.f;	
						mTimeSinceRespawned = 0.0f;
						mCheckCampingPreventionTimeout = true;
					
						//mpRaceLogicLayer->mLivePickups[mPickupType]--;
						//HideSetShowCOMBO();

						ShowAllPickupSpecialFX();

						//cian, if we are within the wake pickup sphere, start the audio
						if(m_InsideCameraTriggerRegion)
						{
							if(QueryInterface(Components::E_AudioEntity_Spawned)) //if present
							{
								bool isGoldenDemand = false;

								FanDemandPositionCreationData* fdcreate = (FanDemandPositionCreationData*)GetCreationData();
								//check if a golded fan demand
								if (mPickupType == E_PICKUP_TYPE_FAN)
									isGoldenDemand =  fdcreate->mFanDemand.gold_demand;

								Game::Actors::WakeAudioPickupMsg wakeAudioPickupMsg(mPickupType, isGoldenDemand, GetActorTypeId());

								Actor::C_ActorList* pActorList				= m_pRLC->GetGameWorld()->GetActorList();

								//this is for split screen...send the numer of trigger regions it falls within for reference counting
								for(S8 i = 0; i < m_InsideCameraTriggerRegion;i++ )
								pActorList->SendMessageToActorComponent(GetHandle(),Components::E_AudioEntity_Spawned,&wakeAudioPickupMsg);
							}

						}
					}
				}
			}
		}

		if(mPickupActive && m_CampingPreventionEnabled && mCheckCampingPreventionTimeout)
		{
			mTimeSinceRespawned += frameTime;
			if(mTimeSinceRespawned > mCampingPreventionTimeout)
			{
				// Clear our memory of being picked up!
				m_NumSuccessiveCollectionsBySameVehicle = 0;
				m_LastVehicleIdxCollectedBy = -1;

				// Do we need to send this info around the network?

				// Dont do this again till next time
				mCheckCampingPreventionTimeout = false;
			}
		}

		AnimatePickup(frameTime);


#ifndef CFG_SPEED
		if (g_debugPickupsUpdateFromSorcery)
		{
			m_BeginTime = GetPickupType()->m_PickupSpecialFXBeginCycle;
			m_EndTime = GetPickupType()->m_PickupSpecialFXEndCycle;

			if (m_BeginTime == m_EndTime)
				m_EndTime = m_BeginTime + 1.0f;
			m_Duration = m_EndTime - m_BeginTime;

			float potentialNewRadius = mCollisionRadius;
			if(m_pRLC->GetRLSystem())
				if(m_pRLC->GetRLSystem()->mPickupSystem)
					if(m_pRLC->GetRLSystem()->mPickupSystem->mParameters)
					{
						potentialNewRadius = m_pRLC->GetDatabase()->mRLSystems[0]->mPickupSystem->mParameters->mPickupRadius;
						mSpawnDelayTime = m_pRLC->GetDatabase()->mRLSystems[0]->mPickupSystem->mParameters->mPickupDeactivateTimer;
					}

			//See if the radius has changed
			if(potentialNewRadius != mCollisionRadius)
			{
				mCollisionRadius = potentialNewRadius;

				if (m_pTriggerObject)
					CreateTrigger(mCollisionRadius);

				//DEBUG DEBUG DEBUG
				InitialiseDebugLineVertices();
				//DEBUG DEBUG DEBUG
			}
		}
#endif
	}


	void PickupPositionActor::AnimatePickup(F32 frameTime)
	{
		if (m_bVisible)												// Update animation if visible
		{
			if (mRotateGraphic)
			{
				mWorldMatrix = Maths::HMat43(Maths::RotY(mRotateSpeed)) * mWorldMatrix;
			}

			m_PickupSpecialFXTime += frameTime;

			if(mPickupActive)
			{
				if( m_pPickupSpecialFX )
				{
					F32 animTime = 0.0f;
					
					if (m_PickupSpecialFXTime > m_BeginTime)
					{
						animTime = m_BeginTime + fmodf(m_PickupSpecialFXTime - m_BeginTime, m_Duration);
					}
					else
					{
						animTime = m_PickupSpecialFXTime;
					}

					BIZ_ASSERT(animTime == animTime && "animTime is NaN");

					m_pPickupSpecialFX->UpdateWorldMatrix(mWorldMatrix);
					m_pPickupSpecialFX->Update(animTime);
				}
			}
		}
	}



	void PickupPositionActor::Render()
	{
#ifndef CFG_SPEED
		//DEBUG DEBUG DEBUG
		if(g_RenderPickupPositions)
		{
			Maths::RGBA colour = Maths::RGBA(0x00, 0x00, 0x00, 0xff);

			if(mPickupActive)
				colour = mColour;

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(mV4, mV1, colour);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(mV4, mV2, colour);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(mV4, mV3, colour);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(mV1, mV2, colour);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(mV2, mV3, colour);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(mV3, mV1, colour);
		}
		//DEBUG DEBUG DEBUG
#endif

		if(gDebugPickupActorShowDebugInfo)
		{
			Maths::HVec3 pos = mWorldMatrix.AW();

			Util::C_String32 str;
			str.Printf("Id %d",mId);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(pos, str);
		}
	}


	void PickupPositionActor::SetFanDemandRenderState()
	{
		mSemiHidden = false;

		if ((!mPickupActive) || (mDisableDemand))
		{
			mEnabled = false;											// Hide and stop it being picked up if it's not active or disabled

			SetupVisualPickupIcon();
			HideAllPickupSpecialFX();
			return;
		}

		if(mpRaceLogicLayer->GetNumVehicles() == 0)
			return;

		VehiclePickupInfo& info=mpRaceLogicLayer->GetLogicVehicleActor(0)->GetPickupInfo();		// Note: Always looking for player car ONLY
		Btl::Set<Actor::C_ActorHandle>::Iterator it = info.mNumFanPickupsCompleted.Find(GetCreationData()->GetActorHandle());
		if (it != info.mNumFanPickupsCompleted.End())
		{
			mEnabled = false;
			SetupVisualPickupIcon();
			HideAllPickupSpecialFX();										// Simply hide it if it's been completed
			return;
		}

		Game::Actors::C_VehicleLogicActor* pLogicActor						= mpRaceLogicLayer->GetLogicVehicleActor(0);			// Always the player car
		Components::C_VehicleComponent_Fan_Demand* vehFanDemandComponent	= (Components::C_VehicleComponent_Fan_Demand*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Fan_Demand);
		if (!vehFanDemandComponent)
		{
			mEnabled = false;
			SetupVisualPickupIcon();
			HideAllPickupSpecialFX();										// Simply hide it if player car has no fan component
			return;
		}

		if (!vehFanDemandComponent->GetCurrentFanDemandState())
		{
//			m_CanBePickedUp = true;
			mEnabled = true;
			SetupVisualPickupIcon();
			ShowAllPickupSpecialFX();										// If no demand active then show this one
			return;
		}

		if (vehFanDemandComponent->GetCurrentFanDemandHandle() == GetCreationData()->GetActorHandle())
		{
			mEnabled = false;
			SetupVisualPickupIcon();
			HideAllPickupSpecialFX();										// Hide if this demand is currently active
			return;
		}


		FanDemandPositionCreationData* fdcreate = (FanDemandPositionCreationData*)GetCreationData();
		if (fdcreate->mFanDemand.gold_demand)
		{
			mEnabled = true;
			SetupVisualPickupIcon();
			ShowAllPickupSpecialFX();										// Show and enable it if it's a golden demand
			return;
		}

		mEnabled = false;
		mSemiHidden = true;
		SetupVisualPickupIcon();
		ShowAllPickupSpecialFX();
	}



	void PickupPositionActor::HideSetShowCOMBO()
	{
		if(mEnabled)
		{
			HideAllPickupSpecialFX();

			bool setupVisual = true;
			if(!mConstantShuffle && !SetRandomPickupType())
				setupVisual = false;
			
			if(setupVisual)
				SetupVisualPickupIcon();

			ShowAllPickupSpecialFX();
		}
	}

	void PickupPositionActor::SetVehicleCollectedLap(int vehicleIndex, S8 lap, bool force)
	{
		//	If we don't have any spline data then this won't do anything - i.e. any vehicle will be able to pick it up (including the one that dropped it).
		if(!m_PickupOnLapZero || force)
			mVehicleCollectedLap[vehicleIndex] = lap;
	}


	void PickupPositionActor::SetPickupType(E_PICKUP_TYPE type)
	{
		PERF_EVENT("PickupPositionActor.SetPickupType");

		if(mPickupType > E_PICKUP_TYPE_NONE)
			mpRaceLogicLayer->mLivePickups[mPickupType]--;

		if(type > E_PICKUP_TYPE_NONE)
			mpRaceLogicLayer->mLivePickups[type]++;

		HideAllPickupSpecialFX();
		mPickupType = type;
		SetupVisualPickupIcon();

		if(mEnabled)
			ShowAllPickupSpecialFX();
	}

	void PickupPositionActor::SetNextPickupType(E_PICKUP_TYPE type)
	{
		mNextPickupType = type;
	}

	void PickupPositionActor::GenerateNextPickupType(int vehIdx)
	{
		E_PICKUP_TYPE oldType = mPickupType;

		bool canSend = (vehIdx == -1 && Game::HostIsLocal()) || (vehIdx != -1 && Game::VehicleIsLocal(vehIdx));
		if(!mIsNetwork || canSend)
		{
			SetRandomPickupType(true);

			E_PICKUP_TYPE next	= mPickupType;
			mPickupType			= mNextPickupType;
			mNextPickupType		= next;

			SendNextPowerupTypeNetworkMessage(mNextPickupType);
		}
		else
			mPickupType = mNextPickupType;

		if(oldType > E_PICKUP_TYPE_NONE)
			mpRaceLogicLayer->mLivePickups[oldType]--;

		if(mPickupType > E_PICKUP_TYPE_NONE)
			mpRaceLogicLayer->mLivePickups[mPickupType]++;
	}

	void PickupPositionActor::SetRandomPickupTypeMpSetup()
	{
		if(mpLevelState->mPerksRespawnRandomType)
		{
			if(mPickupType > E_PICKUP_TYPE_NONE)
				mpRaceLogicLayer->mLivePickups[mPickupType]--;

			SetRandomPickupType(false, true);
			SetRandomPickupType(true, true);

			if(mPickupType > E_PICKUP_TYPE_NONE)
				mpRaceLogicLayer->mLivePickups[mPickupType]++;
		}
		else
		{
			if(mPickupType > E_PICKUP_TYPE_NONE)
				mpRaceLogicLayer->mLivePickups[mPickupType]--;

			// Set my type based on my row
			const RLPickupPositionRow* row = GetCreationData()->m_RowType.Object();
			if(row)
			{
				mFixedType = true;
				mPickupType = mpRaceLogicLayer->GetPickupPositionManager().AddPickupActor(GetCreationData()->m_uRowId, row);
			}
			else
				mFixedType = false;

			// Check if my type is banned and reassign
			for( int j = 0; j < mpLevelState->mPickupProbabilities.Size(); j++ )
			{
				if( mpLevelState->mPickupProbabilities[j].pickupType == mPickupType && mpLevelState->mPickupProbabilities[j].probability == 0.f )
				{
					SetRandomPickupType(false, true);
				}
			}

			if(mPickupType > E_PICKUP_TYPE_NONE)
				mpRaceLogicLayer->mLivePickups[mPickupType]++;
		}

		HideAllPickupSpecialFX();
		SetupVisualPickupIcon();
		if(mPickupActive)
			ShowAllPickupSpecialFX();
	}

	bool PickupPositionActor::SetRandomPickupType(bool checkAgainstNext, bool forceChange)
	{
		PERF_EVENT("PickupPositionActor.SetRandomPickupType");

		if(mFixedType && !mpLevelState->mPerksRespawnRandomType && !forceChange)
			return false;

		bool NonRandomPickupOverride = false;

		//this bit of code will make all perks on the track random and overrides the non random pickups
		if(mpLevelState->mOverrideNonRandomPickups)
		{
			if(((PickupPositionCreate*)GetCreationData())->m_pPickupType)
			{
				NonRandomPickupOverride = true;
			}
		}

		bool canShuffle = mConstantShuffle || mpRaceLogicLayer->mPickupSpawnInfo.shuffleOnPickedUp || mpLevelState->mPerksRespawnRandomType; 

		if( !((PickupPositionCreate*)GetCreationData())->m_pPickupType || NonRandomPickupOverride || gDebugEnforceRandomPickups || canShuffle || forceChange)
		{
			int totalPickups	= mpRaceLogicLayer->GetTotalLivePickups();
			int numVehicles		= mpRaceLogicLayer->GetNumVehicles();

			// ------------------------------------------------------------------------------
			// Select a random pickup, but try and keep an even distribution across the track
			// ------------------------------------------------------------------------------

			Btl::Vector<E_PICKUP_TYPE> pickups;
			for(S32 i = 0; i < RLDatabase::Instance().mRLSystems[0]->mPickupSystem->mTypes.count; i++)
			{
				RLPickupType* pPickup = RLDatabase::Instance().mRLSystems[0]->mPickupSystem->mTypes[i];
				
				if( !pPickup->mDoNotRandomise || gDebugEnforceRandomPickups )
				{
					bool ignore = false;

					if( ( pPickup->meType == E_PICKUP_TYPE_NITROUS && gDebugDisableNitrous) || 
						( pPickup->meType == E_PICKUP_TYPE_SHOCK && gDebugDisableShock ) || 
						( pPickup->meType == E_PICKUP_TYPE_SHUNT && gDebugDisableShunt ) || 
						( pPickup->meType == E_PICKUP_TYPE_BLINDFIRE && gDebugDisableBlindfire ) ||
						( pPickup->meType == E_PICKUP_TYPE_BARGE && gDebugDisableBarge ) || 
						( pPickup->meType == E_PICKUP_TYPE_BOMB && gDebugDisableBomb ) ||
						( pPickup->meType == E_PICKUP_TYPE_SHIELD && gDebugDisableShield ) ||
						( pPickup->meType == E_PICKUP_TYPE_REPAIR && gDebugDisableRepair ) ||
						( pPickup->meType == E_PICKUP_TYPE_STOPWATCH ) ||
						( pPickup->meType == E_PICKUP_TYPE_SUPER_SHIELD ) || 
						( pPickup->meType == E_PICKUP_TYPE_FAN ) /*||
						( pPickup->meType == mPickupType && !checkAgainstNext ) || 
						( pPickup->meType == mNextPickupType && checkAgainstNext )*/
					  )
					{
						ignore = true;
					}

					if(mConstantShuffle)
					{
						if((mShuffleFlags & (1 << pPickup->meType)) == 0)
							ignore = true;
					}
					else
					{
						for( int i = 0; i < ((PickupPositionCreate*)GetCreationData())->m_pExcludedPickupTypes.count; i++ )
						{
							if( pPickup->meType == ((PickupPositionCreate*)GetCreationData())->m_pExcludedPickupTypes[i]->meType )
							{
								ignore = true;
								break;
							}
						}
					}

					if(!ignore)
					{
						// Adjust the probabilities based on distribution
						int numPickupTypeLive = mpRaceLogicLayer->mLivePickups[pPickup->meType];
						int prob = totalPickups - numPickupTypeLive;
						float fProb = 1.f;

						if(!gDebugForceEqualSpawnProbability)
						{
							// Adjust the probabilities based on the position on the track
							if( mLastPickedUpPos != -1 )
							{
								for( int j = 0; j < pPickup->mPositionModifiers.count; j++ )
								{
									if( mLastPickedUpPos >= pPickup->mPositionModifiers[j].minPos && mLastPickedUpPos <= pPickup->mPositionModifiers[j].maxPos )
									{
										fProb *= pPickup->mPositionModifiers[j].modifier;
										break;
									}
								}
							}	

							// Adjust the probabilities based on base probability
							for( int j = 0; j < mpLevelState->mPickupProbabilities.Size(); j++ )
							{
								if( mpLevelState->mPickupProbabilities[j].pickupType == pPickup->meType )
								{
									fProb *= mpLevelState->mPickupProbabilities[j].probability;
									break;
								}
							}
						}

						if(fProb > 0.f)
						{
							fProb = (prob * fProb) + 1.f;
							prob = (int)fProb;

							for( int j = 0; j < prob; j++ )
								pickups.PushBack(pPickup->meType);
						}
					}
				}
			}

			if( pickups.Size() != 0)
			{
				int randPickupIdx = mpRaceLogicLayer->GetRandomInt(pickups.Size());
													
				mPickupType = pickups[randPickupIdx];

				if(mPickupType == E_PICKUP_TYPE_NONE)
					BIZ_ASSERT_FAIL("PickupPositionActor has been set to NONE");
			}
			else
			{				
				LOGWARN(g_PickupPositionLogger, "Selected invalid pickup type - defaulting to Nitrous for safety.");
				mPickupType = E_PICKUP_TYPE_NITROUS;
			}
		}
		//else
		//{	
		//	mPickupType = ((PickupPositionCreate*)GetCreationData())->m_pPickupType->meType;
		//}

		RLPickupType* pPickup = (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(mPickupType);
		if (pPickup)
		{
			m_BeginTime = pPickup->m_PickupSpecialFXBeginCycle;
			m_EndTime	= pPickup->m_PickupSpecialFXEndCycle;
		}
		else
		{
			m_BeginTime = 0.0f;
			m_EndTime	= 1.0f;
		}

		m_Duration = m_EndTime - m_BeginTime;

		if(mPickupType == E_PICKUP_TYPE_NONE)
			BIZ_ASSERT_FAIL("PickupPositionActor has been set to NONE");

		return true;
	}

	void PickupPositionActor::SendPowerupTypeNetworkMessage(const E_PICKUP_TYPE type)
	{
		if (!mIsNetwork)
			return;

		m_PickedUpByLocalPlayer=false;

		const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		netGameVars.m_EventManager->Add_SetPowerupType(GetCreationData()->GetSorceryUid(),type,false);
	}

	void PickupPositionActor::SendNextPowerupTypeNetworkMessage(const E_PICKUP_TYPE type)
	{
		if (!mIsNetwork)
			return;

		const Game::C_NetGameVars& netGameVars = C_GameApp::GetInstance()->GetGame()->GetNetGameVars();
		netGameVars.m_EventManager->Add_SetPowerupType(GetCreationData()->GetSorceryUid(),type,true);
	}

	void PickupPositionActor::GrantPickup(VehiclePickupPosition& vehiclePosition)
	{
		Game::Actors::C_VehicleLogicActor *pLogicActor	= mpRaceLogicLayer->GetLogicVehicleActor(vehiclePosition.mVehicleIndex);

		// [Blur2PC] retail Blur PC (0x00b52e10) awards without checking the slots again: CheckPickup has
		// already refused a full carousel
		AwardPickup(vehiclePosition, vehiclePosition.mAwardSfxOnly);		
		SetRespawnTimer();

		// If we wish to prevent pickup camping...
		if( m_CampingPreventionEnabled )
		{
			float respawnDelayMultiplier = GetRespawnDelayMultiplier();
			mReSpawnTimer *= respawnDelayMultiplier; 

			if(Game::VehicleIsLocal(vehiclePosition.mVehicleIndex) && C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsNetwork())
			{
				// Always send this message
				U8 collectorJoinId = mpLevelState->mCars[vehiclePosition.mVehicleIndex].mNetJoinId;
				Game::C_Game::GetInstance()->GetNetGameVars().m_EventManager->Add_SetPowerupRespawnDelay(GetCreationData()->GetSorceryUid(), collectorJoinId ,respawnDelayMultiplier); 	
			}
		}

		AddSubscriber(GameMessaging::E_GameMsg_Update);		

		pLogicActor->GetPickupInfo().mTimeSinceTouchedLastPickup = 0.f;

		if( !m_PickupOnLapZero )
		{
			mVehicleCollectedLap[vehiclePosition.mVehicleIndex] = pLogicActor->GetLapInfo().mCurrentLap;
		}

		mPickupActive = false;
		mLastPickedUpPos = mpRaceLogicLayer->GetLogicVehicleActor(vehiclePosition.mVehicleIndex)->GetPositionInfo().mRacePosition;

		if(mReSpawnTimer > 0.001f)
		{
			HideAllPickupSpecialFX();
		}

		if(m_OneShot || !m_Respawn)
		{
			mEnabled=false;
			mReSpawnTimer=9999.f;
			mPickupActive=false;
			mAllowPickup=false;

			//Destroy();
			Kill();
		}

		// See if we need to randomly generate next pickup
		if(mpLevelState->mPerksRespawnRandomType)
		{
			GenerateNextPickupType(vehiclePosition.mVehicleIndex);
			HideAllPickupSpecialFX();
			SetupVisualPickupIcon();
			if(mPickupActive)
			{
				ShowAllPickupSpecialFX();
			}
		}

		mVehicleTimeToAllowPickup[vehiclePosition.mVehicleIndex] = g_debugPickupActorTimeToAllowNextPickup;
	}

	bool PickupPositionActor::CheckPickup(VehiclePickupPosition& vehiclePosition)
	{
		PERF_EVENT("PickupPositionActor.CheckPickup");

		// If in last gasp mode.. we cant pick anything up!
		Game::Actors::C_VehicleLogicActor *pLogicActor = mpRaceLogicLayer->GetLogicVehicleActor(vehiclePosition.mVehicleIndex);

		if (!pLogicActor || pLogicActor->IsWreckedOrEliminated() || pLogicActor->IsGhosted())		// Must be racing normally
			return false;

		if(!vehiclePosition.mCollided || mVehicleTimeToAllowPickup[vehiclePosition.mVehicleIndex] > 0.f || pLogicActor->GetIsInLastGaspMode())
			return false;

		if (mPickupType == E_PICKUP_TYPE_FAN)				// All Fan Demand logic done in here (less likely to cause problems with perks)
		{
			// Must be enabled to get here

			if (!mPickupActive || !mEnabled)				// Must be active
				return false;

			if (vehiclePosition.mVehicleIndex != 0)			// Must be the player vehicle
				return false;

			return true;
		}

		if(Game::VehicleIsLocal(pLogicActor->GetVehicleIndex()) && !m_OneShot)
		{
			// Make sure that the vehicle logic actor knows the ID of the last pickup it collected
			pLogicActor->SetLastPickupIdCollided(GetCreationData()->GetSorceryUid());
		}

		if(!mPickupActive || !mAllowPickup)
			return false;

		if ( g_AllowMultiplePickupsPerLap || pLogicActor->GetLapInfo().mCurrentLap > mVehicleCollectedLap[pLogicActor->GetVehicleIndex()] )
		{
			bool pickedUp = false;

			if( mPickupType == E_PICKUP_TYPE_STOPWATCH )
			{
				if (vehiclePosition.mVehicleIndex != 0)			// Must be the player vehicle
					return false;

				if (pLogicActor->IsWreckedOrEliminated() || pLogicActor->IsGhosted())		// Must be racing normally
					return false;

				pickedUp = true;
			}
			else
			{
				// [Blur2PC] retail Blur PC (0x00b51210): a full carousel leaves the pickup where it is, with
				// no sound or effect (CanHavePickUp checks the free slots)
				bool canHave = g_debugOnlyDestroyPickUpIfCanHave && !pLogicActor->CanHavePickUp(mPickupType);

				if(!canHave)
					pickedUp = true;
			}

			if(pickedUp)
			{
				return true;
			}
		}	

		return false;
	}

	bool PickupPositionActor::AwardPickup(const VehiclePickupPosition& vehiclePosition, bool fxOnly)
	{
		PERF_EVENT("PickupPositionActor.AwardPickup");

		Game::Actors::C_VehicleLogicActor* pLogicActor	= mpRaceLogicLayer->GetLogicVehicleActor(vehiclePosition.mVehicleIndex);
		if(!pLogicActor || pLogicActor->IsWreckedOrEliminated() || pLogicActor->IsGhosted())
			return false;

		if(mPickupType == E_PICKUP_TYPE_NONE)
			BIZ_ASSERT_FAIL("Pickups: Position Actor trying to send out a message of powerup NONE");

		Game::Actors::PickupPickedUpMsg pickupPickedUpMsg;
		pickupPickedUpMsg.mVehicleIndex = vehiclePosition.mVehicleIndex;
		pickupPickedUpMsg.mVehicleActorHandle = vehiclePosition.mActorHandle;
		pickupPickedUpMsg.mPickupActorHandle = GetCreationData()->GetActorHandle();
		pickupPickedUpMsg.mePickupType = mPickupType;
		pickupPickedUpMsg.mPickupPosition = mWorldMatrix.AW();
		pickupPickedUpMsg.mDroppedFromIndex = GetCreationData()->m_DroppedFromVehicleIndex;
		pickupPickedUpMsg.mUniqueId = mId;
		pickupPickedUpMsg.mIsGoldenFanDemand = false;
		pickupPickedUpMsg.mFxOnly = fxOnly;

		Actor::C_ActorList* pActorList = m_pRLC->GetGameWorld()->GetActorList();

		if(mPickupType == E_PICKUP_TYPE_FAN)
		{
			// Fail any currently active fan demand (since this could be a golden demand which trumps a normal demand)
			Components::C_VehicleComponent_Fan_Demand* vehFanDemandComponent	= (Components::C_VehicleComponent_Fan_Demand*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Fan_Demand);
			if (vehFanDemandComponent)
				vehFanDemandComponent->EndDemand(false);

			FanDemandPositionCreationData* fdcreate = (FanDemandPositionCreationData*)GetCreationData();
			//check if a golded fan demand
			if (mPickupType == E_PICKUP_TYPE_FAN)
				pickupPickedUpMsg.mIsGoldenFanDemand =  fdcreate->mFanDemand.gold_demand;
			else
				pickupPickedUpMsg.mIsGoldenFanDemand = false;

			// Send to vehicle
			pLogicActor->SendMessageToActor(pLogicActor->GetHandle(), &pickupPickedUpMsg);

			// Fan demand should have been effectively picked up by now so update all the fan demand pickups (including this one)
			if (mPickupType == E_PICKUP_TYPE_FAN)
				C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( Game::E_PickupMessage_UpdateFanDemandState );

			pickupPickedUpMsg.mFxOnly = true;
		}
		else if(mPickupType == E_PICKUP_TYPE_STOPWATCH)
		{
			//[PK] need to go to the actor first, so I know how much time is left (before the racemode increases it)
			pLogicActor->SendMessageToActor(pLogicActor->GetHandle(), Game::E_PickupMessage_CollectStopwatch);

			Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(Game::E_PickupMessage_CollectStopwatch);
			pickupPickedUpMsg.mFxOnly = true;
		}
		else
		{
			E_PICKUP_TYPE eType = E_PICKUP_TYPE_NONE;
			PickupPositionCreate* pCreate = (PickupPositionCreate*)GetCreationData();

			eType = mPickupType;

			//check if a golden fan demand for audio
			FanDemandPositionCreationData* fdcreate = (FanDemandPositionCreationData*)GetCreationData();	
			if (mPickupType == E_PICKUP_TYPE_FAN)
				pickupPickedUpMsg.mIsGoldenFanDemand =  fdcreate->mFanDemand.gold_demand;
			else
				pickupPickedUpMsg.mIsGoldenFanDemand = false;

			if(m_OneShot)
			{
				pickupPickedUpMsg.mUniqueId = m_UniqueDropID;

				if(Game::VehicleIsLocal(pickupPickedUpMsg.mVehicleIndex) && mIsNetwork)
				{
					C_GameApp::GetInstance()->GetGame()->GetNetGameVars().m_EventManager->Add_DroppedPickupPickedUp(m_UniqueDropID);
				}
			}
			else
			{
				pickupPickedUpMsg.mUniqueId = mId;
			}

			m_PickedUpByLocalPlayer=Game::VehicleIsLocal(pLogicActor->GetVehicleIndex());

			if(m_PickedUpByLocalPlayer)
			{
				m_TimesPickedUpByLocalPlayer++;

				if(m_CampingPreventionEnabled)
				{
					RecordCampingStats(pLogicActor->GetVehicleIndex());
				}
			}

			Components::C_VehicleComponent_Pickups_AI* aiComp = (Components::C_VehicleComponent_Pickups_AI*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Pickups_AI);

			if(aiComp)
				aiComp->EvaluatePickedUpPickup(eType);	
		}

		// Let's not broadcast this since it only goes to 2 places - BroadcastMessage(&pickupPickedUpMsg);
		pLogicActor->SendMessageToActor(pLogicActor->GetHandle(), &pickupPickedUpMsg);

		//send to audio component
		if(QueryInterface(Components::E_AudioEntity_Spawned))				//if present
			pActorList->SendMessageToActorComponent(GetHandle(),Components::E_AudioEntity_Spawned,&pickupPickedUpMsg);

		return true;
	}

	void PickupPositionActor::RecordCampingStats( U8 collectedByVehIdx )
	{
		// Is this the same vehicle that collected us last time?
		if(collectedByVehIdx == m_LastVehicleIdxCollectedBy)
		{
			m_NumSuccessiveCollectionsBySameVehicle++; 
		}
		else
		{
			m_NumSuccessiveCollectionsBySameVehicle = 0; 
		}

		// record idx of this vehicle
		m_LastVehicleIdxCollectedBy = collectedByVehIdx;
	}

	const RLPickupType* PickupPositionActor::GetPickupType() const
	{
		for(S32 i = 0; i < RLDatabase::Instance().mRLSystems[0]->mPickupSystem->mTypes.count; i++)
		{
			RLPickupType* pickup = RLDatabase::Instance().mRLSystems[0]->mPickupSystem->mTypes[i];

			if( mPickupType == pickup->meType )
				return pickup; 
		}

		return NULL;
	}

	bool PickupPositionActor::ViewportIsGhosted(int vp)
	{
		for( int i = 0; i < mpRaceLogicLayer->GetNumVehicles(); i++ )
		{
			if( m_pRLC->GetViewportIndex(i) == vp )
			{
				Game::Actors::C_VehicleLogicActor* pLogicActor = mpRaceLogicLayer->GetLogicVehicleActor(i);
				if(pLogicActor && pLogicActor->IsGhosted())
					return true;
			}
		}
		
		return false;
	}

	void PickupPositionActor::ShowPickupSpecialFX(int idx)
	{
		PERF_EVENT("PickupPositionActor.ShowPickupSpecialFX");

		if (m_pPickupSpecialFX)							// Switch off any current animations
		{
			const RLPickupType* pickup = GetPickupType();
			if(pickup && !ViewportIsGhosted(idx))
			{
				U32 params[2];
				params[0] = 1 << (Game::E_ViewType_Main1 + idx);
				params[1] = 1 << (Game::E_ViewType_Main1 + idx);
				if (idx == 0) //Add to mirror and reflection as well
				{
					params[0] |= Game::E_ViewTypeFlag_CubeMap | Game::E_ViewTypeFlag_Mirror | Game::E_ViewTypeFlag_Reflection;
					params[1] |= Game::E_ViewTypeFlag_CubeMap | Game::E_ViewTypeFlag_Mirror | Game::E_ViewTypeFlag_Reflection;
				}
				m_pPickupSpecialFX->SetParameter(SpecialFX::E_ParamID_SetMeshViewFlags, (void*)params);
				m_PickupSpecialFXTime = 0.f;
			}
		}
	}

	void PickupPositionActor::HidePickupSpecialFX(int idx)
	{
		PERF_EVENT("PickupPositionActor.HidePickupSpecialFX");

		if (m_pPickupSpecialFX)
		{
			U32 params[2];
			params[0] = 0;
			params[1] = 1 << (Game::E_ViewType_Main1 + idx);
			if (idx == 0) //Remove from mirror and reflection as well
			{
				params[1] |= Game::E_ViewTypeFlag_CubeMap | Game::E_ViewTypeFlag_Mirror | Game::E_ViewTypeFlag_Reflection;
			}
			m_pPickupSpecialFX->SetParameter(SpecialFX::E_ParamID_SetMeshViewFlags, (void*)params);
		}
	}

	void PickupPositionActor::ShowAllPickupSpecialFX()
	{
		if (m_pPickupSpecialFX)
		{
			m_pPickupSpecialFX->Start();
			m_pPickupSpecialFX->UpdateWorldMatrix(GetMatrix());
			m_pPickupSpecialFX->Update( 0 );
			U32 params[2];
			params[0] = Game::E_ViewTypeFlag_Main | Game::E_ViewTypeFlag_CubeMap | Game::E_ViewTypeFlag_Mirror | Game::E_ViewTypeFlag_Reflection;
			params[1] = Game::E_ViewTypeFlag_Main | Game::E_ViewTypeFlag_CubeMap | Game::E_ViewTypeFlag_Mirror | Game::E_ViewTypeFlag_Reflection;;
			m_pPickupSpecialFX->SetParameter(SpecialFX::E_ParamID_SetMeshViewFlags, (void*)params);
		}
	}

	void PickupPositionActor::HideAllPickupSpecialFX()
	{
		if (m_pPickupSpecialFX)
		{
			m_pPickupSpecialFX->Stop();
		}
	}

	void PickupPositionActor::Destroy()
	{
		if (m_pVisDetect)
		{
			Actors::C_VisibilityDetector::ReleaseVisDetect(m_pVisDetect);
			m_pVisDetect = NULL;
		}

		if (m_pPickupSpecialFX)
		{
			m_pPickupSpecialFX->Stop();
			SAFE_DELETE_PTR(m_pPickupSpecialFX);
		}

		//[CWright:35079] RaceLogicLayer could have been destroyed at this point so left with dangling "mpRaceLogicLayer" pointer
		if (C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer())
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->DecrementTotalLivePickups(GetHandle());

		DestroyTrigger();

		for( int i = 0; i < mVehiclesCollidedThisFrame.Size(); i++ )
		{
			mVehiclesCollidedThisFrame[i].mCollided = false;
		}

		m_pGameWorld = NULL;
	}

	void PickupPositionActor::ActivatePickups(float p_fTime)
	{
		mAllowPickup = true;

		if(mpLevelState->mPickupDisableTime == -1)
		{
			mEnabled = false;
			mPickupActive = false;
		}
		else if(m_ActivationTimer)
			mReSpawnTimer = m_ActivationTimer;
		else 
			mReSpawnTimer = p_fTime;
		//else if(mpLevelState->mPickupDisableTime)
		//	mReSpawnTimer = mpLevelState->mPickupDisableTime;
		//else
		//	mReSpawnTimer = gDebugPickupInitialDisableTime;

		AddSubscriber(GameMessaging::E_GameMsg_Update);

		if(mPickupType == E_PICKUP_TYPE_FAN)
		{
			if(Game::C_Game::GetInstance()->GetLevelState().mDisableFanDemands)
			{
				mPickupActive = false;
				mPickupType = E_PICKUP_TYPE_NONE;
			}
			else
			{
				if (mReSpawnTimer <= 0.0f)
					mPickupActive = true;
			}

			SetFanDemandRenderState();
		}

		//if(m_InsideCameraTriggerRegion)
		//{
		//	if(QueryInterface(Components::E_AudioEntity_Spawned)) //if present
		//	{
		//		bool isGoldenDemand = false;
		//		FanDemandPositionCreationData* fdcreate = (FanDemandPositionCreationData*)GetCreationData();
		//		//check if a golded fan demand
		//		if (mPickupType == E_PICKUP_TYPE_FAN)
		//			isGoldenDemand =  fdcreate->mFanDemand.gold_demand;

		//		Game::Actors::WakeAudioPickupMsg wakeAudioPickupMsg(mPickupType, isGoldenDemand);

		//		Actor::C_ActorList* pActorList				= m_pRLC->GetGameWorld()->GetActorList();
		//		pActorList->SendMessageToActorComponent(GetHandle(),Components::E_AudioEntity_Spawned,&wakeAudioPickupMsg);
		//	}

		//}
	}

	int PickupPositionActor::GetActiveLevel()
	{
		int level = 0;
		if (C_GameApp::GetInstance() && 
			C_GameApp::GetInstance()->GetRaceBookLayer() && 
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr() && 
			C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->GetID() == Game::E_GameMode_Debug)
		{
			level = gDebugPickupActiveLevelForDebug;
		}
		else
		{
			if(gDebugPickupActiveLevel != -1)
				level = gDebugPickupActiveLevel;
			else
				level = Game::C_Game::GetInstance()->GetLevelState().mPickupSpawnLevel;
		}

		return level;
	}

	void PickupPositionActor::SetRespawnTimer()
	{
		E_PICKUP_RESPAWN_TYPE respawnType = Game::C_Game::GetInstance()->GetLevelState().mPickupRespawnType;
		switch (respawnType)
		{
		case E_PICKUP_RESPAWN_TYPE_ALWAYS_ON:
			{
				mReSpawnTimer = 0.0f;
			}
			break;

		case E_PICKUP_RESPAWN_TYPE_NORMAL:
			{
				if (GetCreationData()->m_fRespawnTimerOverride >= 0.001f)
				{
					mReSpawnTimer = GetCreationData()->m_fRespawnTimerOverride;
				}
				else if( gMinRespawnTime != -1 && gMaxRespawnTime != -1 && gMaxRespawnTime >= gMinRespawnTime )
					mReSpawnTimer = mpRaceLogicLayer->GetRandomFloat(gMinRespawnTime, gMaxRespawnTime);
				else
					mReSpawnTimer = mSpawnDelayTime;
			}
			break;
		case E_PICKUP_RESPAWN_TYPE_LONG:
			{
				mReSpawnTimer = C_MultiplayerData::Instance().m_pConfig->m_PowerupRespawnToggleLong;
			}
			break;

		case E_PICKUP_RESPAWN_TYPE_VERY_LONG:
			{
				mReSpawnTimer = C_MultiplayerData::Instance().m_pConfig->m_PowerupRespawnToggleVeryLong;
			}
			break;
		case E_PICKUP_RESPAWN_TYPE_NONE:
			{
				m_Respawn = false;
			}
			break;
		}
	}

	bool PickupPositionActor::OnMessage(const Messaging::C_MessageBase *pMessage)
	{
		//PERF_EVENT("PickupPositionActor::MessageHandler");

		switch(pMessage->Type())
		{
		case Game::E_GameflowMessage_ClearStoredActorPtrs:
		{
			m_pRLC = NULL;
		}
		break;

		case ::Actors::E_VisibilityMsg_OnHidden:
			m_bVisible = false;
			break;

		case ::Actors::E_VisibilityMsg_OnVisible:
			AddSubscriber(GameMessaging::E_GameMsg_Update);
			m_bVisible = true;
			break;

		case GameMessaging::E_GameMsg_PreUpdate:
			{
				const GameMessaging::C_UpdateMsg* pMsg = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage);
				PreUpdate(pMsg->GetValue());
				break;
			}

		case GameMessaging::E_GameMsg_Update:
			{
				if (mPickupType == E_PICKUP_TYPE_FAN)				// Don't need any of the stuff after this for fan demands
				{
					const GameMessaging::C_UpdateMsg* pMsg = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage);
					if ((mEnabled) || (mReSpawnTimer > 0.0f))
					{
						Update(pMsg->GetValue());
						Render();									// Just for debug
					}
					else
					{
						if (mSemiHidden)
							AnimatePickup(pMsg->GetValue());
					}
					return(true);
				}

				if(mEnabled)
				{
					const GameMessaging::C_UpdateMsg* pMsg = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage);
					Update(pMsg->GetValue());
					Render();
				}

				if(gDebugPickupCheckActiveLevel)
				{
					int level = GetActiveLevel();
					if(mEnabled && GetCreationData()->m_iPickupActiveLevel > level)
					{
						mEnabled = false;
						HideAllPickupSpecialFX();
					}
					else if( !mEnabled && mPickupActive && GetCreationData()->m_iPickupActiveLevel <= level )
					{
						mEnabled = true;
						ShowAllPickupSpecialFX();
					}
				}

				return true;
			}

		case Game::E_GameflowMessage_RaceLoaded:
			{	
				HideSetShowCOMBO();

				return true;
			}

		case Game::E_GameLogicMessage_LapIncremented:
			{
				if(m_ActivationLap)
				{
					const Game::C_LapIncrementedMsg* pMsg = Messaging::MessageCast<Game::C_LapIncrementedMsg>(pMessage);
					Game::Actors::RaceLogicCore* pCore = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();

					if(pMsg->mLap == m_ActivationLap && !pCore->IsVehicleAIControlled(pMsg->mVehicleIndex))
					{
						mEnabled = true;
						mPickupActive = true;
						ShowAllPickupSpecialFX();
					}
				}
				return true;
			}

		case Game::E_PickupMessage_EnteredPickup:							// Sent when vehicle enters a trigger region
			{
				if(mEnabled && m_pRLC && m_pRLC->GetGameWorld())		// Only check if enabled
				{
					const Game::Actors::C_EnteredPickupMsg* pMsg		= Messaging::MessageCast<Game::Actors::C_EnteredPickupMsg>(pMessage);
					const Actor::C_ActorList* pActorList				= m_pRLC->GetGameWorld()->GetActorList();
					const Game::Actors::C_VehicleActor* pVehicleActor	= Actor::ActorCast_Safe<Game::Actors::C_VehicleActor>( pActorList->GetActorPtr(pMsg->mVehicleActorHandle) );
					int	vehIdx											= pVehicleActor->GetVehicleIndex();

					bool bufferOverrun = true;
					bool alreadyInserted = false;

					for( int i = 0; i < mVehiclesCollidedThisFrame.Size(); i++ )
					{
						if(mVehiclesCollidedThisFrame[i].mCollided && mVehiclesCollidedThisFrame[i].mVehicleIndex == vehIdx)
							alreadyInserted = true;
					}

					if(!alreadyInserted)
					{
						for( int i = 0; i < mVehiclesCollidedThisFrame.Size(); i++ )
						{
							if(!mVehiclesCollidedThisFrame[i].mCollided)
							{
								mVehiclesCollidedThisFrame[i].mVehicleIndex		= vehIdx;
								mVehiclesCollidedThisFrame[i].mWorldPosition	= pVehicleActor->GetMatrix().AW();
								mVehiclesCollidedThisFrame[i].mActorHandle		= pMsg->mVehicleActorHandle;
								mVehiclesCollidedThisFrame[i].mAwardSfxOnly		= false;
								mVehiclesCollidedThisFrame[i].mCollided			= true;

								//if(!CheckPickup(mVehiclesCollidedThisFrame[i]))
								//	mVehiclesCollidedThisFrame[i].mCollided	= false;

								bufferOverrun = false;
								break;
							}
						}
					}

					BIZ_ASSERT_MSG(!bufferOverrun, "PickupPositionActor ran out of buffer for contention logic - let SB know");
				}
				return true;
			}

		case Game::E_PickupMessage_ExitedPickup:
			{
				const Game::Actors::C_ExitedPickupMsg* pMsg			= Messaging::MessageCast<Game::Actors::C_ExitedPickupMsg>(pMessage);

				for( int i = 0; i < mVehiclesCollidedThisFrame.Size(); i++ )
				{
					if(mVehiclesCollidedThisFrame[i].mCollided && mVehiclesCollidedThisFrame[i].mActorHandle == pMsg->mVehicleActorHandle)
					{
						mVehiclesCollidedThisFrame[i].mCollided = false;
					}
				}
				break;
			}

		case Game::E_PickupMessage_UpdateFanDemandState:
			{
				SetFanDemandRenderState();
				return true;
			}

		case E_VehicleLogicMsg_Ghosted:
			{
				const C_VehicleLogicMsg_GhostedMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_GhostedMsg>(pMessage);

				if(pMsg->m_Viewport != -1)
					HidePickupSpecialFX(pMsg->m_Viewport);

				return true;
			}

		case E_VehicleLogicMsg_Deghosted:
			{
				const C_VehicleLogicMsg_DeghostedMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_DeghostedMsg>(pMessage);

				if(pMsg->m_Viewport != -1)
				{
					ShowPickupSpecialFX(pMsg->m_Viewport);
					m_bVisible = true;
				}

				return true;
			}

		case Game::E_PickupMessage_EnablePickupType:
			{
				const Game::Actors::EnablePickupTypeMsg* pMsg = Messaging::MessageCast<Game::Actors::EnablePickupTypeMsg>(pMessage);
				if(pMsg->mePickupType == mPickupType)
				{
					mEnabled = true;
					//ShowPickupSpecialFX();
				}
				return true;
			}
		case Game::E_PickupMessage_DisablePickupType:
			{
				const Game::Actors::DisablePickupTypeMsg* pMsg = Messaging::MessageCast<Game::Actors::DisablePickupTypeMsg>(pMessage);
				if(pMsg->mePickupType == mPickupType)
				{
					mEnabled = false;
					HideAllPickupSpecialFX();
				}
				return true;
			}

		case Game::E_PickupMessage_EnableAllPickupTypes:
			{
				mEnabled = true;
				//ShowPickupSpecialFX();
				return true;
			}
		case Game::E_PickupMessage_DisableAllPickupTypes:
			{
				mEnabled = false;
				HideAllPickupSpecialFX();
				return true;
			}

		case Game::E_PickupMessage_SetPickupPositionId:
			{
				const Game::Actors::SetPickupPositionIdMsg* pMsg = Messaging::MessageCast<Game::Actors::SetPickupPositionIdMsg>(pMessage);
				mId = pMsg->mPickupPositionId;
				return true;
			}

		case Game::E_PickupMessage_PickupPickedUp:
			{
				SetRespawnTimer();
				if(mReSpawnTimer > 0.001f)
				{
					mPickupActive = false;

					HideAllPickupSpecialFX();
				}

				return true;
			}

		case Game::E_PickupMessage_ActivatePickups:
			{
				const Game::Actors::PickupMessage_ActivatePickups* pMsg = Messaging::MessageCast<Game::Actors::PickupMessage_ActivatePickups>(pMessage);
				ActivatePickups(pMsg->m_fTime);
				
				return true;
			}
		case Game::E_GameflowMessage_RaceStarted:
			{
				// A good idea to tell everyone our pickup here in case someone wasn't ready to receive the message earlier.
				//SendPowerupTypeNetworkMessage(mPickupType);

				return true;
			}

		case Game::E_PickupMessage_ActivatePickup:
			{
				mReSpawnTimer = 0.f;
				mPickupActive = true;

				ShowAllPickupSpecialFX();

				if(m_InsideCameraTriggerRegion)
				{
					if(QueryInterface(Components::E_AudioEntity_Spawned)) //if present
					{
						bool isGoldenDemand = false;
						FanDemandPositionCreationData* fdcreate = (FanDemandPositionCreationData*)GetCreationData();
						//check if a golded fan demand
						if (mPickupType == E_PICKUP_TYPE_FAN)
							isGoldenDemand =  fdcreate->mFanDemand.gold_demand;

						Game::Actors::WakeAudioPickupMsg wakeAudioPickupMsg(mPickupType, isGoldenDemand, GetActorTypeId());

						Actor::C_ActorList* pActorList				= m_pRLC->GetGameWorld()->GetActorList();

						//this is for split screen...send the numer of trigger regions it falls within for reference counting
						for(S8 i = 0; i < m_InsideCameraTriggerRegion;i++ )
						pActorList->SendMessageToActorComponent(GetHandle(),Components::E_AudioEntity_Spawned,&wakeAudioPickupMsg);
					}

				}

				if (mPickupType == E_PICKUP_TYPE_FAN)				// Could be activating a chained demand
				{
					mDisableDemand = false;
					SetFanDemandRenderState();
				}

				return true;
			}

		case Game::E_PickupMessage_ResetPickup:
			{
				if (mPickupType == E_PICKUP_TYPE_FAN)				// Could be activating a chained demand
				{
					FanDemandPositionCreationData* fdcreate = (FanDemandPositionCreationData*)GetCreationData();
					mDisableDemand = fdcreate->mFanDemand.start_deactivated;

					VehiclePickupInfo& info			= mpRaceLogicLayer->GetLogicVehicleActor(0)->GetPickupInfo();
					Btl::Set<Actor::C_ActorHandle>::Iterator it = info.mNumFanPickupsCompleted.Find(GetCreationData()->GetActorHandle());
					if (it != info.mNumFanPickupsCompleted.End())
						info.mNumFanPickupsCompleted.Erase(it);

					// Assuming the achievement manager doesn't need notifying since the achievement will be awarded on the last demand in the chain
					SetFanDemandRenderState();

						//cian, if we are within the wake pickup sphere, start the audio
						if(m_InsideCameraTriggerRegion)
						{
							if(QueryInterface(Components::E_AudioEntity_Spawned)) //if present
							{
								bool isGoldenDemand = false;

								FanDemandPositionCreationData* fdcreate = (FanDemandPositionCreationData*)GetCreationData();
								//check if a golded fan demand
								if (mPickupType == E_PICKUP_TYPE_FAN)
									isGoldenDemand =  fdcreate->mFanDemand.gold_demand;

								Game::Actors::WakeAudioPickupMsg wakeAudioPickupMsg(mPickupType, isGoldenDemand, GetActorTypeId());

								Actor::C_ActorList* pActorList				= m_pRLC->GetGameWorld()->GetActorList();

								//this is for split screen...send the numer of trigger regions it falls within for reference counting
								for(S8 i = 0; i < m_InsideCameraTriggerRegion;i++ )
								pActorList->SendMessageToActorComponent(GetHandle(),Components::E_AudioEntity_Spawned,&wakeAudioPickupMsg);
							}

						}
				}

				return true;
			}

		case Game::E_PickupMessage_KillAllPickups:
			{
				//Destroy();
				Kill();
				return true;
			}

		case ActorMsg::E_DynamicObjMsg_SetVehicleLogic:
			{
				const ActorMsg::C_SetVehicleLogicMsg* pMsg = Messaging::MessageCast<ActorMsg::C_SetVehicleLogicMsg>(pMessage);
				//mEnabled = mPickupActive = mAllowPickup = pMsg->m_Pickups;
				mAllowPickup = pMsg->m_Pickups;

				if(mEnabled && mReSpawnTimer <= 0.f )
					ShowAllPickupSpecialFX();
				else
					HideAllPickupSpecialFX();

				return true;
			}

		case Game::E_PickupMessage_DebugForcePickupRespawn:
			{
				//Get rid of the current pickup type from the RaceLogicLayer

				if(mPickupType > E_PICKUP_TYPE_NONE)
					mpRaceLogicLayer->mLivePickups[mPickupType]--;

				mPickupType = E_PICKUP_TYPE_NONE;

				HideSetShowCOMBO();
				return true;
			}

		case ActorStudioMsgShared::E_ActorStudioMsg_Drag:
		{
			const ActorStudioMsgGame::C_DragMsg* msg = Messaging::MessageCast<ActorStudioMsgGame::C_DragMsg>(pMessage);
			mWorldMatrix = msg->GetValue();
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_EndDrag:
		{
			if (m_pTriggerObject)
			{
				U8 data[1024 * sizeof(void*)];
				SharedPhysics::C_SafeStack<true> physicsStack(data, 1024);
				m_pTriggerObject->SetMatrix(mWorldMatrix, physicsStack);
			}
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_Selected:
		{
			m_Selected = true;
		}
		break;

		case ActorStudioMsgShared::E_ActorStudioMsg_Deselected:
		{
			m_Selected = false;
		}
		break;

		case Game::E_VehicleMessage_TriggerActorEvent:
			{

			const Game::Actors::C_VehicleCheckTriggerActorEvent* pMsg	= Messaging::MessageCast<Game::Actors::C_VehicleCheckTriggerActorEvent>(pMessage);

				
			if(pMsg->mEvent->GetEventType() == Gravity::C_TriggerEvent::E_Enter_Event)
			{
					m_InsideCameraTriggerRegion++;
					if(QueryInterface(Components::E_AudioEntity_Spawned) && mPickupActive) //if present
					{
						bool isGoldenDemand = false;
						FanDemandPositionCreationData* fdcreate = (FanDemandPositionCreationData*)GetCreationData();
						//check if a golded fan demand
						if (mPickupType == E_PICKUP_TYPE_FAN)
							isGoldenDemand =  fdcreate->mFanDemand.gold_demand;

						Game::Actors::WakeAudioPickupMsg wakeAudioPickupMsg(mPickupType, isGoldenDemand, GetActorTypeId());

						Actor::C_ActorList* pActorList				= m_pRLC->GetGameWorld()->GetActorList();
						pActorList->SendMessageToActorComponent(GetHandle(),Components::E_AudioEntity_Spawned,&wakeAudioPickupMsg);
					}

			}

			if(pMsg->mEvent->GetEventType() == Gravity::C_TriggerEvent::E_Exit_Event)
			{
					m_InsideCameraTriggerRegion--;
					if(QueryInterface(Components::E_AudioEntity_Spawned)/* && mPickupActive*/) //if present
					{
						Actor::C_ActorList* pActorList				= m_pRLC->GetGameWorld()->GetActorList();
						pActorList->SendMessageToActorComponent(GetHandle(),Components::E_AudioEntity_Spawned,Game::E_PickupMessage_SleepAudio);
					}
			}

		}
		break;

		}				

		//return C_GameActor::OnMessage(pMessage);
		return true;
	}

	void PickupPositionActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
	{
		debugRenderPtr->Draw3dMatrix(mWorldMatrix, 0.5f);
		if(m_Selected)
			debugRenderPtr->Draw3dText(Maths::HVec3(mWorldMatrix.Translation()), "Selected");
	}

	float PickupPositionActor::GetRespawnDelayMultiplier()
	{
		// Init
		float multiplier = 1.0f;

		// Only mess with this multiplier if in appropriate race modes. 
		if(Game::C_Game::GetInstance()->GetLevelState().mRaceMode == Game::E_RaceMode_Multiplayer_Destruction)
		{
			// Sanity checks...
			MultiplayerConfig* pConfig = C_MultiplayerData::Instance().m_pConfig;
			BIZ_ASSERT_MSG(pConfig," PickupPositionActor::GetRespawnDelayMultiplier() - error mulitplayer Config invalid");
			BIZ_ASSERT_MSG(pConfig->m_MotormashRespawnDelayMultipliers.count > 0," PickupPositionActor::GetRespawnDelayMultiplier() - error m_MotormashRespawnDelayMultipliers is empty.. defaulting to 1.0f multiplier");

			if(pConfig->m_MotormashRespawnDelayMultipliers.count > 0)
			{
				// Convert number of times successively picked up by same person into Idx for multiplier table (cap to max if necessary)
				U32 maxLookupValue = pConfig->m_MotormashRespawnDelayMultipliers.count-1; 
				U32 lookupIndex = m_NumSuccessiveCollectionsBySameVehicle;

				// Clamp lookup index
				if(lookupIndex > maxLookupValue)
				{
					lookupIndex = maxLookupValue; 
				}

				// Get value at that position
				multiplier = pConfig->m_MotormashRespawnDelayMultipliers[lookupIndex];
			}
		}
		
		// Return
		return multiplier; 
	}

	void PickupPositionActor::ForceNetRespawnDelay(const F32 respawnDelayMultiplier, const U8 localCollectorIdx)
	{
		// Record the fact we have been picked up...
		RecordCampingStats(localCollectorIdx); 

		// Update our respawn timer...
		// NOTE update from scratch (will be innacurate by time msg has taken to get here... but better than just Multiplying the error!)

		SetRespawnTimer();

		mReSpawnTimer *= respawnDelayMultiplier; 
	}
}

XT_TYPE(Actors::PickupPositionCreate, "PickupPosition")
	XT_BASE(Actors::C_PosActorCreate)
	
	XT_FIELD(m_pPickupType,				"PickupType")
	XT_FIELD(m_pExcludedPickupTypes,	"ExcludedPickupTypes")
	XT_FIELD(m_iPickupActiveLevel,		"PickupActiveLevel")
	XT_FIELD(m_bRotateGraphic,			"RotateGraphic")
	XT_FIELD(m_fActivationTimer,		"ActivationTimer")
	XT_FIELD(m_iActivationLap,			"ActivationLap")
	XT_FIELD(m_fRespawnTimerOverride,	"RespawnTimerOverride")
	XT_FIELD(m_bConstantShuffle,		"ConstantShuffle")
	XT_FIELD(m_uShuffleTypes,			"ShuffleTypes")
	XT_FIELD(m_RowType,					"RowType")
	XT_FIELD(m_uRowId,					"RowId")
XT_END_TYPE
