#include "stdafx.h"

#include <GameLogic/Pickups/Normal/PickupShock.h>
#include <GameLogic/Pickups/PickupClasses.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Layers\AmaxWorldLayer.h>
#include <GameLogic/InGameSpecialFX/InGameSpecialFXManager.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameDef\Polyflags.h>

#include "AppPhotoCapture.h"
#include <ui\Game\Gadgets\ButtonMessages.h>
#include <ai/AI_Amax.h>
#include <GameApp.h>

#include "GameAudio/GameAudioLayer.h"

#include "GameAudio/PickupSubsystem.h"
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <Gravity/NarrowPhase/Primitive.h>
#include <Game/GameDef/PhysicsConstants.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>

#include <Game/Game.h>

Debug::Int g_dbMaxDamageObjects("Perks.Shock.MaxObjectsToDamage", 3, Debug::Range(1, 100));

Debug::Float g_debugTempShockLifetimeFXMult("Perks.Shock.TempLifetimeFXMult", 5.0f);
Debug::Float g_debugTempShockWarningDistance("Perks.Shock.WarningDistance", 15.0f);

Debug::Float g_debugShockInitialGrowthRate("Perks.Shock.InitialGrowthRate", 1.5f);//1.1
Debug::Float g_debugShockLateGrowthRate("Perks.Shock.LateGrowthRate", 1.5f);//0.3
Debug::Float g_debugShockGrowthRateCutOff("Perks.Shock.GrowthRateCutOff", 0.7f);
Debug::Float g_debugShockSizeMultiplierBaseLow("Perks.Shock.MultiplierBaseLow", 0.85f);
Debug::Float g_debugShockSizeMultiplierBaseHigh("Perks.Shock.MultiplierBaseHigh", 1.15f);

Debug::Float g_debugShockRayTraceHeight("Perks.Shock.RayTraceHeight", 20000.f);
Debug::Float g_debugShockTraceGroundBuffer("Perks.Shock.RayTraceGroundBuffer", 0.3f);
Debug::Float g_debugShockTraceObstacleBuffer("Perks.Shock.RayTraceObstacleBuffer", 1.0f);
Debug::Float g_debugShockSizeDenominator("Perks.Shock.SizeDenominator", 60.f); // 50, 130
Debug::Float g_debugShockWidthDistanceModifier("Perks.Shock.WidthDistanceModifier", 10.0f);

Debug::Float g_debugShockMinWidth("Perks.Shock.MinWidth", 0.1f); // 0.5, 0.25
Debug::Float g_debugShockMaxWidth("Perks.Shock.MaxWidth", 0.8f); // 2.5

Debug::Bool g_debugShockDraw("Perks.DebugDraw.Shock", false);

namespace Game
{
	C_PickupShock::C_PickupShock(U32 id, int firingIdx, int targetIdx, E_PICKUP_SUB_TYPE subType, bool noFiringVehicleFX, float shockWidth, float lifetimeMod, int logicalIndex, bool noPhysical)
		: C_PickupBase(E_PickupClass_Shock, id, firingIdx, E_PICKUP_TYPE_SHOCK, subType, noFiringVehicleFX)
	{
		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHOCK);
		mpShockData							= (RLShockBalancingData*)pPickup->mNormal->mBalancingData;

		mCurrentInnerRadius = mCurrentOuterRadius = mCurrentLifetime = 0.f;
		mHasHitTarget = false;
		mNoPhysicalEffect = noPhysical;
		mTrackSegmentWidth = shockWidth;
		mTargetIdx = targetIdx;

		BIZ_ASSERT(mpShockData);

		Util::C_Random mRandGenerator;
		mRandGenerator.Seed((int)(mUniqueId * mTrackSegmentWidth));	//	Seed the rand with something that everyone knows about in a network game, but will give different(ish) results each time
		mSizeMultiplier = mRandGenerator.FRand(g_debugShockSizeMultiplierBaseLow, g_debugShockSizeMultiplierBaseHigh);

		mTotalLifetime = mpShockData->mLifetime + lifetimeMod;

		RLPickupLevel* pLevel = GetRLSubTypeLevel();
		if(pLevel)
		{
			RLShockSpecialFX* pShockSFX = (RLShockSpecialFX*)pLevel->mSpecialFX;

			if(!mNoFiringVehicleFX && pLevel->mSpecialFX)
			{
				C_InGameSpecialFXManager::Instance().CreateSpecialFX(pLevel->mSpecialFX->mChargingSpecialFX, mMatrix, firingIdx, -1, id, E_InGameSpecialFX_Trigger_Charging);
			}

			// Dodgy hack to scale lifetime to the animation
			if(pShockSFX->mShockDomeSpecialFX.count > 0)
			{
				float startAnimTime	= pShockSFX->mShockDomeSpecialFX[0].mDurationIntro.mEnd - pShockSFX->mShockDomeSpecialFX[0].mDurationIntro.mStart;
				float loopAnimTime	= pShockSFX->mShockDomeSpecialFX[0].mDurationLoop.mEnd - pShockSFX->mShockDomeSpecialFX[0].mDurationLoop.mStart;
				float mod			= fmodf(mTotalLifetime + startAnimTime, loopAnimTime);

				if(mod > 0.f)
				{
					if(mod < loopAnimTime / 2)
					{
						mTotalLifetime -= mod;
					}
					else
					{
						mTotalLifetime += (loopAnimTime - mod);
					}
				}
			}
		}	

		mAITrackableData.mIsTrackable = true;
		mAITrackableData.mIsShootable = false;	
		mAITrackableData.mAILogicIndex = logicalIndex;
	}

	// --------------------------------------------------------------------

	void C_PickupShock::PostDelayInitialise(Maths::HMat43 matrix, XtArray<RLPickupSpecialFX>* sfx)
	{
		C_PickupBase::PostDelayInitialise(matrix, sfx);

		if(!mActivated)
		{
			C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mFiringIdx);

			if(mSizeMultiplier <= 0.f)
				mSizeMultiplier = 1.0f;

			matrix.AX( Maths::HVec3( matrix.AX() * mSizeMultiplier * mTrackSegmentWidth ) );
			matrix.AY( Maths::HVec3( matrix.AY() * mSizeMultiplier * mTrackSegmentWidth ) );
			matrix.AZ( Maths::HVec3( matrix.AZ() * mSizeMultiplier * mTrackSegmentWidth ) );

			SetPostDelayMatrix(matrix, mTrackSegmentWidth);
		}
	}

	// --------------------------------------------------------------------

	void C_PickupShock::SetPostDelayMatrix( Maths::HMat43 targetMatrix, float fVal )
	{
		mTrackSegmentWidth = fVal;

		mTotalInnerRadius = mTrackSegmentWidth * mSizeMultiplier * mpShockData->mInnerRadius;
		mTotalOuterRadius = mTrackSegmentWidth * mSizeMultiplier * mpShockData->mOuterRadius;

		C_PickupBase::SetPostDelayMatrix(targetMatrix, fVal);

		RLPickupType* pPickup				= (RLPickupType*)::RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_SHOCK);
		RLShockSpecialFX* pShockFX			= (RLShockSpecialFX*)pPickup->mNormal->mSpecialFX;

		if( HasAbility(UI::E_MultiplayerVehicleAbilities_Iron_Fist, mFiringIdx) && AbilityActiveOnPickupType(UI::E_MultiplayerVehicleAbilities_Iron_Fist) )
		{
			C_InGameSpecialFXManager::Instance().CreateSpecialFX(pShockFX->mIronFistDomeSpecialFX, mMatrix, mFiringIdx, mTargetIdx, mUniqueId, 
				E_InGameSpecialFX_Trigger_Firing, mTotalLifetime + 4);
		}
		else
		{
			C_InGameSpecialFXManager::Instance().CreateSpecialFX(pShockFX->mShockDomeSpecialFX, mMatrix, mFiringIdx, mTargetIdx, mUniqueId, 
				E_InGameSpecialFX_Trigger_Firing, mTotalLifetime + 4);
		}

		//shock don't have a bona fide projectile, so we jump through a few hoops here to convince the audio it does..
		C_VehicleLogicMsg_FiredPerkMsg fired_msg(mPickupType, 0, mFiringIdx, mUniqueId, mMatrix, 1, false, E_PICKUP_SUB_TYPE_NORMAL, false, 1);
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, ::PickUpSubsystem::E_PickUpSubsystem_Fire_Projectile, 0, &fired_msg);

		//let the audio know the shock has moved to its landing position
		C_VehicleLogicMsg_PerkUpdateMatrixMsg updateMatrixMsg(mUniqueId, mMatrix, mFiringIdx,Maths::HVec3(0.f,0.f,0.f));
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_UpdateMatrix, 0, &updateMatrixMsg);


		//This is to indicate that the shock has landed. 
		//shock is a special case since i'm treating the landing as a projectile Object even though is doesn't physically have one
		C_VehicleLogicMsg_PerkEnded pMsg(mFiringIdx,mUniqueId,mPickupType,Game::E_BodyUserDataType_Actor);
		Audio::InterfaceController::Get().SendMessageToSubsystem(Game::C_GameAudioLayer::ePickupSubsystem, PickUpSubsystem::E_PickUpSubsystem_Ended, 0, &pMsg);

		mAITrackableData.mRadius = mTotalOuterRadius;


/*
		// now map into spline space for the ai......
		// get and store the track data for the mine...
		airacingtech::ITrackDataAI *pTrackDataSystem = AIAmax::AmaxAIR::Instance().GetTrackDataSystem();
		if (pTrackDataSystem)
		{
			const airacingtech::RouteInfo *pLogicRoute = pTrackDataSystem->GetLogicRoute();
			if (pLogicRoute && mAITrackableData.mAILogicIndex != -1)
			{
				// this is the starting point for the search....
				int startIndex = pLogicRoute->m_pRoute->GetIndexOffset(mAITrackableData.mAILogicIndex, -10);
				int endIndex = pLogicRoute->m_pRoute->GetIndexOffset(mAITrackableData.mAILogicIndex, 50);				
				Maths::HF32 paramDist = 0.0f;
				Maths::HVec3 positionBomb(mMatrix.AW());
				mAITrackableData.mAILogicIndex =  pLogicRoute->m_pRoute->FindNearest( startIndex, endIndex, positionBomb, paramDist, pLogicRoute->m_pRoute->GetStepSize());
				if (mAITrackableData.mAILogicIndex == -1)
				{
					mAITrackableData.mAILogicIndex = pLogicRoute->m_index;
				}

			}
		}
*/



	}

	// --------------------------------------------------------------------

	bool C_PickupShock::Update()
	{
		PERF_EVENT("PickupShock.Update");

		if(!mActivated)
			return false;

		if(mKillPickup)
		{
			//C_InGameSpecialFXManager::Instance().DestroyPerk(mUniqueId);
			Game::C_InGameSpecialFXManager::Instance().SetState(mUniqueId, E_InGameSpecialFX_Mode_Outro);
			C_PickupsManager::Instance().RemoveShock(mUniqueId);
			return true;
		}

		mCurrentLifetime	+= C_GameApp::GetInstance()->GetTimeStep();

		float growthLevel;
		if(mCurrentLifetime >= g_debugShockGrowthRateCutOff)
		{
			float lateLifetime = mCurrentLifetime - g_debugShockGrowthRateCutOff;
			growthLevel = (g_debugShockInitialGrowthRate * g_debugShockGrowthRateCutOff) + (g_debugShockLateGrowthRate * lateLifetime);
		}
		else
		{
			growthLevel = mCurrentLifetime * g_debugShockInitialGrowthRate;
		}

		if(growthLevel > 0.f)
			CheckDynamicObjects();

		//mCurrentInnerRadius = mTotalInnerRadius * growthLevel;
		mCurrentOuterRadius = mTotalOuterRadius * growthLevel;
		//mCurrentInnerRadius = Maths::Clamp(mCurrentInnerRadius, 0.f, mTotalInnerRadius);
		mCurrentOuterRadius = Maths::Clamp(mCurrentOuterRadius, 0.f, mTotalOuterRadius);

		if(g_debugShockDraw)
			DebugDraw();

		C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		const Maths::HF32 outerRadiusSquared(mCurrentOuterRadius * mCurrentOuterRadius);

		for( int i = 0; i < pLayer->GetNumVehicles(); i++ )
		{
			E_PICKUP_HIT_TYPE hitType = E_PICKUP_HIT_TYPE_NONE;

			Maths::HVec3 vehPos = pLayer->mVehiclePositions[i].mPosition;

			const Maths::HF32 dist2 = Maths::DistanceSquared(vehPos, mMatrix.AW());
			//if(dist <= mCurrentInnerRadius)
			//	hitType = E_PICKUP_HIT_TYPE_MAJOR; 
			if( dist2 <= outerRadiusSquared)
				hitType = E_PICKUP_HIT_TYPE_MINOR;

			float heightDiff = vehPos.Y() - mMatrix.AW().Y();

			if(hitType != E_PICKUP_HIT_TYPE_NONE && CheckCanHit(i, hitType) && heightDiff >= mpShockData->mHeightDiff && !mVehiclesShielded[i])
			{
				RegisterHit(i, hitType);
				//Have we just hit the target (When we havent targeted ourselves)?
				if(i == mTargetIdx)
				{
					mHasHitTarget = true;
				}

				// Store any data specific to this type of death
				mAutopsyReport.mTargettedVehicleId =  mTargetIdx;
				mAutopsyReport.mVehicleIdHit	   =  i;

				// Do the killing
				KillPickup(-1,E_PICKUP_CAUSE_OF_DEATH_HIT_VEHICLE);
			}
		}

		//if(mCurrentOuterRadius == mTotalOuterRadius)
		//	KillPickup(-1);

		if(mCurrentLifetime > mTotalLifetime)
			KillPickup(-1,E_PICKUP_CAUSE_OF_DEATH_OLD_AGE);

		return false;
	}

	// --------------------------------------------------------------------------------


	void C_PickupShock::CheckDynamicObjects()
	{
		PERF_EVENT("C_PickUpShock.CheckDynamicObjects");
		Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		Gravity::C_Scene		*pscene = pRLC->GetGameWorld()->GetPhysicsScene();
		Actor::C_ActorHandle	handle;
		const Actor::C_Actor	*pActor;

		Collision::C_Sphere sphere(mMatrix.AW(), mCurrentOuterRadius);	
		SharedPhysics::MemoryAllocator<Gravity::C_Body*> allocator;
		U8 stackData[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(stackData, 1024);
		Collision::C_CollisionFilter filter(Game::PhysicsConstants::kPerkCollisionFilter);
		filter.SetGroupMask(Game::PolyFlags::kbDynamicObject);
		S32 bodyCount = pscene->GetIntersectingBodies(sphere, &filter, allocator, stack);
		SharedPhysics::C_MemoryIterator<Gravity::C_Body*> iter(&allocator);
		U32 count = 0;

		for(S32 body = 0; body < bodyCount; body++)
		{
			Gravity::C_Body** pDoublePtr = iter.Get();
			Gravity::C_Body* pBody = *pDoublePtr;
			Game::C_BodyUserDataBase* pUserData = (Game::C_BodyUserDataBase*)pBody->GetUserData<void>();
			if (pUserData && pUserData->m_eType == Game::E_BodyUserDataType_Actor)
			{
				handle = ((Game::C_BodyUserDataActor*)pUserData)->m_hActorHandle;
				pActor = pRLC->GetGameWorld()->GetActorList()->GetActorPtr(handle);

				if (pActor->GetActorTypeId() == E_ActorTypeId_DynamicObject || pActor->GetActorTypeId() == E_ActorTypeId_PhysicalObject)
				{
					Btl::Set<Actor::C_ActorHandle>::InsertionReturnType insResult = mDynamicsHit.Insert(handle);
					if(!insResult.inserted)
					{
						RLPickupForceEffects* forces = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetDatabase()->mRLSystems[0]->mPickupForceEffects;
						Maths::HVec3 dir = Maths::Normalise(pActor->GetMatrix().AW() - mMatrix.AW());

						ActorMsg::C_DynamicObjectApplyImpulseMsg msg;
						msg.m_Force = dir * forces->m_Shock.m_MaxForce;
						msg.m_ElectricalEffect = forces->m_Shock.m_ElectricalEffect;
						msg.m_ElectricalEffectColour = forces->m_Shock.m_ElectricalEffectColour;
						msg.m_DamageScale = forces->m_Shock.m_DamageScale;

						pActor->GetActorList()->SendMessageToActor(pActor->GetHandle(), &msg);

						if (forces->m_Shock.m_ElectricalEffect)
							pActor->GetActorList()->DelayMessageToActorBySeconds(1.0f + ::Actors::C_ActorUtils::Instance().GetRandomNum(), pActor->GetHandle(), 
							ActorMsg::E_DynamicObjMsg_DynamicObjectDisableElectricalEffect);	// End electrical effect message

						if((++count) >= g_dbMaxDamageObjects)
							return;
					}
				}
			}
		}
	}

	// --------------------------------------------------------------------------------

	E_TARGETTING_DIRECTION C_PickupShock::GetHitDirection(Actors::C_VehicleLogicActor* pActor)
	{
		E_TARGETTING_DIRECTION hitDir = E_TARGETTING_DIRECTION_NONE;

		if(pActor)
		{
			Maths::HVec3 dir = Maths::Normalise(mMatrix.AW() - pActor->GetWorldMatrix().AW());
			float dotProdX = Maths::DotProduct(dir, pActor->GetWorldMatrix().AX());

			if( dotProdX < 0 )
			{
				hitDir = E_TARGETTING_DIRECTION_LEFT;
			}
			else
			{
				hitDir = E_TARGETTING_DIRECTION_RIGHT;
			}
		}

		return hitDir;
	}

	C_PickupShock::~C_PickupShock()
	{
	}

	// --------------------------------------------------------------------

	void C_PickupShock::DebugDraw()
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),mCurrentInnerRadius,Maths::RGBA(0,0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(mMatrix.AW(),Maths::HVec3(0,1,0),mCurrentOuterRadius,Maths::RGBA(0,0,255));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mMatrix.AW(),mCurrentOuterRadius,Maths::RGBA(0,0,255));
	}

	// --------------------------------------------------------------------

	C_PickupEffectsShock::C_PickupEffectsShock(E_PICKUP_TYPE type, int vehIdx, float time)
		: C_PickupLocalEffectsBase(type, vehIdx, time), mPhotoTaken(false)
	{
	}

	bool C_PickupEffectsShock::Update()
	{
		Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		if(levelState.mNumViewports == 1 && levelState.mCars[mVehicleIndex].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
		{
			if(!mPhotoTaken)
			{
				PhotoMode::AppPhotoCapture().TakePhoto(1);
				mPhotoTaken = true;
				return false;
			}

			C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			Actors::C_VehicleLogicActor* pActor = pLayer->GetLogicVehicleActor(mVehicleIndex);

			S32 local_index = pActor->GetRLC()->GetViewportIndex( mVehicleIndex );
			Util::C_String32 buffer;

			buffer.Printf( "Flash_fade_%d", local_index );
			UI::C_UiTimelineMessage msg( buffer.Chars(), true, 0.0f );
			UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg );

			buffer.Printf( "FlashGraphic_fade_%d", local_index );
			UI::C_UiTimelineMessage msg1( buffer.Chars(), true, 0.0f );
			UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg1 );
		}

		return true;
	}
}
