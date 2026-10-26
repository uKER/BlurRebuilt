#include "stdafx.h"
#include "VLC_PlayerRubberBanding.h"

#include <Vehicle/Input.h>

#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/GameflowMessages.h>

#include <GameLogic\RaceLogicCore\Core\RaceLogicCore.h>
#include <GameLogic\Layers\RaceLogicLayer.h>

#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>

#include <ai/AI_Amax.h>

static Debug::Logger &g_AmaxMpPlayerRubberBandingLog = Debug::GetLogger("amax.multiplayer.playerrubberbanding");
Debug::Bool gDisableSplitScreenPlayerRubberBanding("Vehicle.Components.PlayerRubberBanding.DisablePlayerRubberBanding", false, Debug::Persist() );
Debug::Bool gDrawRubberBandGroupsOnRaceLine("Vehicle.Components.PlayerRubberBanding.DrawRubberBandGroupsOnRaceLine", false, Debug::Persist());
Debug::Bool gShowRubberBandingModifier( "Vehicle.Components.PlayerRubberBanding.ShowRubberBandingModifier", false, Debug::Persist());
Debug::Bool gShowRubberBandingImpulse( "Vehicle.Components.PlayerRubberBanding.ShowRubberBandingImpulse", false, Debug::Persist());
Debug::Bool gDrawGroupSizes( "Vehicle.Components.PlayerRubberBanding.ShowGroupSizes", false, Debug::Persist());

// This value is used to determine how quickly to smooth between snap changes (e.g. banding modifier is 0 in a frame and 1 in the next frame)
Debug::Float gSmoothingValueForSnapCases( "Vehicle.Components.PlayerRubberBanding.SmoothingValueForSnapCases", 0.01f, Debug::Persist());

extern Debug::Logger &g_Log;

namespace Components
{
	const float C_VehicleComponent_PlayerRubberBanding::s_minimumAcceleratorValue = 0.5f;
	int C_VehicleComponent_PlayerRubberBanding::s_componentCount = 0;
	Curve::C_HermiteCurve C_VehicleComponent_PlayerRubberBanding::s_impulseMagnitudeGraph;

	C_VehicleComponent_PlayerRubberBanding::C_VehicleComponent_PlayerRubberBanding()
	{
		s_componentCount++;
	}

	C_VehicleComponent_PlayerRubberBanding::~C_VehicleComponent_PlayerRubberBanding()
	{
		s_componentCount--;
	}

	bool C_VehicleComponent_PlayerRubberBanding::CreateImpl()
	{
		const I_VehicleComponent_PlayerRubberBanding_CreateInfo* creation_data = static_cast<const I_VehicleComponent_PlayerRubberBanding_CreateInfo *>(GetCreator().GetData());

		BIZ_ASSERT_MSG(creation_data, "VLC_PlayerRubberBanding::CreateImpl() > No creation data");

		mActorHandle=creation_data->mParentActorHandle;
		mVehicleIndex=creation_data->mVehicleIndex;
		mRLC=creation_data->mRLC;
		mLevelState=creation_data->mLevelState;
		mRaceLogicLayer=creation_data->mRaceLogicLayer;

		AddSubscriber( Game::E_GameflowMessage_RaceStarted );
		AddSubscriber( Game::E_GameflowMessage_RaceFinished );
		AddSubscriber( Game::E_GameflowMessage_ClearStoredActorPtrs );		

		Initialise();

		return true;
	}

	void C_VehicleComponent_PlayerRubberBanding::Initialise()
	{
		mEnabled = false;
		mRubberBandingModifier = 0.f;
		mDesiredRubberBandingModifier = 0.f;
		mSmoothingToNewModifier = false;
		mTrackLength = 0.f;
		mMaxRaceLogicPosition = 0.f;
		mMinRaceLogicPosition = 0.f;
		mOurRaceLogicPosition = 0.f;

#ifdef CFG_GOLD
		//we don't require dynamic updating of the curve if in master build, only needs doing once
		UpdateCurve();
#endif
	}

	void C_VehicleComponent_PlayerRubberBanding::Update()
	{
		if(!mEnabled || gDisableSplitScreenPlayerRubberBanding)
			return;

		PERF_EVENT("VehicleComponent.PlayerRubberBanding");

		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());

		mOurRaceLogicPosition = pActor->GetPositionInfo().mLogicPositionCombined; //overall race logic position

		mDesiredRubberBandingModifier = ComputePlayerRubberBandingModifier();

		// We don't want massive "snaps" in the rubber banding
		if(Maths::Abs(mDesiredRubberBandingModifier - mRubberBandingModifier) < 0.05)
		{
			mRubberBandingModifier = mDesiredRubberBandingModifier;
		}
		else
		{
			mRubberBandingModifier += gSmoothingValueForSnapCases * C_GameApp::GetInstance()->GetTimeStep() * 30.f
											* (mDesiredRubberBandingModifier - mRubberBandingModifier);
		}

		//do not apply a rubber banding impulse if the player is making no attempt to catch up
		int controllerIndex = mLevelState->mCars[mVehicleIndex].mController;
		if(mRLC->IsVehiclePlayerControlled(mVehicleIndex) && Input::GetGameInput(controllerIndex)->GetButtonValue(Game::E_Button_Throttle) >= s_minimumAcceleratorValue)
		{
			ApplyRubberBandingByImpulse(pActor);
		}

#ifndef CFG_GOLD
		DebugDraw();
#endif
	}

	void C_VehicleComponent_PlayerRubberBanding::DebugDraw()
	{
		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());

		//display mRubberBandingModifier
		if(gShowRubberBandingModifier)
		{
			Util::C_String banding;
			banding.Printf("Banding:%5.2f", mRubberBandingModifier);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText( 
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()
				->mVehiclePositions[pActor->GetVehicleIndex()].mPosition + Maths::HVec3(0.f,2.f,0.f), 0.5f, banding );
		}
		// display GetImpulseMagnitude()
		if(gShowRubberBandingImpulse)
		{
			Util::C_String bandingImpulse;
			bandingImpulse.Printf("Impulse:%5.2f", GetImpulseMagnitude());
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText( 
				C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()
				->mVehiclePositions[pActor->GetVehicleIndex()].mPosition + Maths::HVec3(0.f,1.75f,0.f), 0.5f, bandingImpulse );
		}

		//display group boundary lines along a debug-drawn race length
		if( gDrawRubberBandGroupsOnRaceLine )
		{
			const Framework::C_View& rFrameworkView = Framework::C_ApplicationBase::GetInstance()->GetFullscreenView();
			Maths::C_Rect viewExtents =  rFrameworkView.m_View.m_ViewSettings2D.m_ViewExtents;
			F32 yMin = viewExtents.Min().Y() + 50; //*cough*
			F32 xMax = viewExtents.Max().X();
			F32 pixelsAlongRaceLine_MinBoundary = GetMinRaceLogicPositionBetweenZeroAndOne()*(xMax-20);
			F32 pixelsAlongRaceLine_MaxBoundary = GetMaxRaceLogicPositionBetweenZeroAndOne()*(xMax-20);
			F32 pixelsAlongRaceLine_ThisCar = ((mOurRaceLogicPosition-1.f)/mLevelState->mLaps)*(xMax-20);

			//draw our race line
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine( Maths::HVec2( 10, yMin ),
																		Maths::HVec2( xMax - 10, yMin ),
																		Maths::RGBA(0,255,0) );

			//draw our max group boundary
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine( Maths::HVec2( pixelsAlongRaceLine_MaxBoundary + 10, yMin + 20 ),
																		Maths::HVec2( pixelsAlongRaceLine_MaxBoundary + 10, yMin ),
																		Maths::RGBA(255,0,0) );

			//draw our min group boundary
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine( Maths::HVec2( pixelsAlongRaceLine_MinBoundary + 10, yMin + 20 ),
																		Maths::HVec2( pixelsAlongRaceLine_MinBoundary + 10, yMin ),
																		Maths::RGBA(255,0,0) );

			//draw lines between the two
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dLine( Maths::HVec2( pixelsAlongRaceLine_MinBoundary + 10, yMin + 20 ),
																		Maths::HVec2( pixelsAlongRaceLine_MaxBoundary + 10, yMin + 20 ),
																		Maths::RGBA(255,0,0) );

			//draw this car
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dTriangle( Maths::HVec2( pixelsAlongRaceLine_ThisCar + 10, yMin ),
																			Maths::HVec2( pixelsAlongRaceLine_ThisCar + 10 + 10.f, yMin + 25 ),
																			Maths::HVec2( pixelsAlongRaceLine_ThisCar + 10 - 10.f, yMin + 25 ),
																			Maths::RGBA( 255, 255, 0 ) );

			if( gDrawGroupSizes )
			{
				Util::C_String groupSize;
				groupSize.Printf("%5.0fm", (mMaxRaceLogicPosition - mMinRaceLogicPosition)*mTrackLength);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText( 
					Maths::HVec2( (pixelsAlongRaceLine_MinBoundary) + 10, yMin + 20 ), groupSize, Maths::RGBA(255,255,255), 20.f );
			}
		}
		else
		{
			if( gDrawGroupSizes )
				gDrawGroupSizes = false;
		}
	}

	// -------------------------------------------------------------------------------------------------

	bool C_VehicleComponent_PlayerRubberBanding::OnMessage(const Messaging::C_MessageBase *pMessage) 
	{
		PERF_EVENT("C_VehicleComponent.PlayerRubberBanding.MessageHandler");

		switch(pMessage->Type())
		{
		case Game::E_GameflowMessage_ClearStoredActorPtrs:
			{
				mRLC = NULL;
			}
			break;

		case Game::E_GameflowMessage_RaceStarted:
			{
				if(AIAmax::AmaxAIR::Instance().AllOK()) //If we cant get any AI stuff then dont enable rubber-banding (AS)
				{
					mTrackLength = CalculateTrackLength();
					mEnabled = true;
				}
			}
			break;
		

		case Game::E_GameflowMessage_RaceFinished:
			{
				mEnabled = false;
			}
			break;
		}

		return true;
	}

	//------------------------------------------------------------------------------

	/*
	*	Speeds and slows cars by applying an impulse in the direction of motion, multiplied by the distance to the band target
	*	(clamped to a max distance) and by our magic modifier value that we use for tweaking.
	*/
	void C_VehicleComponent_PlayerRubberBanding::ApplyRubberBandingByImpulse(Game::Actors::C_VehicleLogicActor* pActor)
	{
		Game::Actors::C_VehicleActor* pGraphicalActor = pActor->GetGraphicalActor(mVehicleIndex);
		BIZ_ASSERT_MSG(pGraphicalActor, "C_VehicleComponent_PlayerRubberBanding::ApplyRubberBandingByImpulse > Invalid graphical actor");
		RacingPhysics::Car* pVehPhysics = pGraphicalActor->GetVehiclePhysics();
		BIZ_ASSERT_MSG(pVehPhysics, "C_VehicleComponent_PlayerRubberBanding::ApplyRubberBandingByImpulse > Invalid vehicle physics" );
		Gravity::C_Body* pGravityBody = pVehPhysics->GetGravityBody();
		BIZ_ASSERT_MSG(pGravityBody, "C_VehicleComponent_PlayerRubberBanding::ApplyRubberBandingByImpulse > Invalid gravity body");
#ifndef CFG_GOLD
		//Our modifier should be between -1 and 1, something has gone wrong (but not game-breakingly so) if it is outside this range
		if(mRubberBandingModifier > 1.f || mRubberBandingModifier < -1.f)
		{
			LOGWARN(g_Log, "C_VehicleComponent_PlayerRubberBanding::ApplyRubberBandingByImpulse > Rubber Banding Modifier is outside range -1 to 1 (was %.2f), see Mark T", 
							mRubberBandingModifier );
		}
#endif
		Maths::HVec3 direction( pGraphicalActor->GetDirection() );

		Maths::HVec3 impulse = Maths::HVec3( direction * GetImpulseMagnitude() );

		pGravityBody->GetBody().ApplyImpulseWorldSpace( impulse, pGraphicalActor->GetPosition() );
	}

	/*
	*	Retreives the magnitude of the impulse from the mapping defined in sorcery,
	*	modified by any "difficulty" value (e.g. light/medium/heavy banding)
	*/
	float C_VehicleComponent_PlayerRubberBanding::GetImpulseMagnitude() const
	{
		const PlayerRubberBandingSettings& cache = C_MultiplayerData::Instance().m_pConfig->m_PlayerRubberBandingSettings;
		float impulse = s_impulseMagnitudeGraph.Evaluate( mRubberBandingModifier );

		// note that the graph represents E_MultiplayerPlayerBanding_Heavy so we don't need to multiply for that setting
		switch(mLevelState->GetMultiplayerSettings().mHandicap)
		{
			case UI::E_HandicapLow:
			{
				impulse *= cache.m_LightAssistMultiplier;
				break;
			}
			case UI::E_HandicapMedium:
			{
				impulse *= cache.m_MediumAssistMultiplier;
				break;
			}
		}

		return impulse;
	}

	/*
	*	Gets the track distance in metres between this vehicle and the rubber band point specified. 
	*/
	const float C_VehicleComponent_PlayerRubberBanding::GetDistanceToBandPoint() const
	{
		float logicDistanceToTarget = ( mMaxRaceLogicPosition + mMinRaceLogicPosition )/2.f - mOurRaceLogicPosition;

		return logicDistanceToTarget * mTrackLength;
	}

	float C_VehicleComponent_PlayerRubberBanding::CalculateTrackLength()
	{
		airacingtech::ITrackDataAI *pTrackData = AIAmax::AmaxAIR::Instance().GetTrackDataSystem();
		airacingtech::AITrackDataRoutes* pLogicRoute = pTrackData->GetTrackData()->m_TrackLogicData.m_pLogicRoute;

		return pLogicRoute->GetRouteLength();
	}
	
	/*
	*	Computes a value from -1.0 for the firstmost car to 1.0 for the car at the back, ranging linearly by distance.
	*/
	const float C_VehicleComponent_PlayerRubberBanding::ComputePlayerRubberBandingModifier() const
	{
		float raceLogicWindow = mMaxRaceLogicPosition - mMinRaceLogicPosition;
		float rubberBandingModifier = mOurRaceLogicPosition - mMinRaceLogicPosition;

		rubberBandingModifier /= (raceLogicWindow/2.f);	//gives us a value between 0 and 2
		rubberBandingModifier -= 1.f;					//gives us a value between -1 and 1
		rubberBandingModifier = -rubberBandingModifier;	//gives us a value between 1 (for last) and -1 (for first)

		return rubberBandingModifier;
		//if you'd like to do this by race position instead!
		/*
		//convert position to rubber banding value
		float p = (float)mRacePositionRelativeToHumans;
		float h = (float)mTotalHumanPlayers;
		mRubberBandingModifier = ( (p/h) - ((h+1.f)/(2.f*h)) ) * ( h / ((h/2.f)-0.5f) );
		*/
	}

	float C_VehicleComponent_PlayerRubberBanding::ComputeDistanceBetweenVehicles(int vehicleIndexA, int vehicleIndexB)
	{
		Game::C_RaceLogicLayer* raceLogicLayerPtr = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		float logicPosA = raceLogicLayerPtr->GetLogicVehicleActor(vehicleIndexA)->GetPositionInfo().mLogicPositionCombined;
		float logicPosB = raceLogicLayerPtr->GetLogicVehicleActor(vehicleIndexB)->GetPositionInfo().mLogicPositionCombined;

		return Maths::Abs(logicPosA - logicPosB) * CalculateTrackLength(); 
	}

	void C_VehicleComponent_PlayerRubberBanding::SendGroupDataToComponents()
	{
		// Enable dynamic update of the graph (done in Initialise() if Gold )
#ifndef CFG_GOLD
		UpdateCurve();
#endif

		if( s_componentCount > 0 && AIAmax::AmaxAIR::Instance().AllOK() && !gDisableSplitScreenPlayerRubberBanding )
		{
			Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
			Game::C_RaceLogicLayer* raceLogicLayerPtr = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
			float trackLength = CalculateTrackLength();
			const PlayerRubberBandingSettings& cache = C_MultiplayerData::Instance().m_pConfig->m_PlayerRubberBandingSettings;
			float minDistForRubberBand = cache.m_MinDistanceForFullBanding;
			float maxBandDistance = cache.m_MaxDistanceBetweenGroupMembers;
			Btl::Vector<int> vehIndexes;
			int numberOfVehicles = 0;
			for( U32 i = 0; i < levelState.mNumCars; i++ )
			{
				// only count this vehicle into our calculations if it's a player or a network vehicle that isn't an AI
				if( levelState.mCars[i].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER 
					|| ((levelState.mCars[i].mControlType == E_VEHICLE_CONTROL_TYPE_NETWORK) && levelState.mCars[i].mOwnerNetJoinId == 0) )
				{
					numberOfVehicles++;
					vehIndexes.PushBack(i);
				}
			}

			//iterate through vehicles in reverse order and determine which vehicles they are grouped with

			//populate an array with race position vs vehicle index data and sort it by position
			RacePositionData sortedRacePositions[MAX_VEHICLES];
			for(int i=0; i<numberOfVehicles; i++)
			{
				sortedRacePositions[i].racePosition = raceLogicLayerPtr->GetLogicVehicleActor(vehIndexes[i])->GetPositionInfo().mRacePosition;
				sortedRacePositions[i].vehicleIndex = vehIndexes[i];
			}

			qsort(&sortedRacePositions[0], numberOfVehicles, sizeof(RacePositionData), SortRacePositionsFunc);

			Btl::Vector<int,Btl::VectorParameters_Static<MAX_VEHICLES> > groupVehicleIndexes;
			float backRaceLogicPosition;
			float frontRaceLogicPosition;
			for(int i=0; i<numberOfVehicles; i++)
			{
				groupVehicleIndexes.PushBack(sortedRacePositions[i].vehicleIndex);
				
				backRaceLogicPosition = raceLogicLayerPtr->GetLogicVehicleActor(sortedRacePositions[i].vehicleIndex)->GetPositionInfo().mLogicPositionCombined;
				
				while(i+1<numberOfVehicles 
					&& ComputeDistanceBetweenVehicles(sortedRacePositions[i].vehicleIndex, sortedRacePositions[i+1].vehicleIndex ) < maxBandDistance )
				{
					groupVehicleIndexes.PushBack(sortedRacePositions[i+1].vehicleIndex);
					i++;
				}

				//finished finding all vehicles in this "group", we know the last one added defines the front race logic position
				frontRaceLogicPosition = raceLogicLayerPtr->GetLogicVehicleActor(groupVehicleIndexes[groupVehicleIndexes.Size() -1])->GetPositionInfo().mLogicPositionCombined;

				//tell all vehicles in the group about the back/front race logic positions of their group "window"
				for(int j=0; j<groupVehicleIndexes.Size(); j++)
				{
					C_VehicleComponent_PlayerRubberBanding* component = (C_VehicleComponent_PlayerRubberBanding*)raceLogicLayerPtr->GetLogicVehicleActor(groupVehicleIndexes[j])->QueryInterface<C_VehicleComponent_PlayerRubberBanding>();
					if(component)
					{
						BIZ_ASSERT_MSG(component, "C_VehicleComponent_PlayerRubberBanding::Update() retrieved a null component from vehicle logic actor %i", groupVehicleIndexes[j] );
						float groupFirstDistanceToGroupLast = ComputeDistanceBetweenVehicles(groupVehicleIndexes[0], groupVehicleIndexes[groupVehicleIndexes.Size() -1]);
						bool useMinSize = minDistForRubberBand > groupFirstDistanceToGroupLast;
						component->SetMaxRaceLogicPosition(useMinSize ? frontRaceLogicPosition + ( (minDistForRubberBand-groupFirstDistanceToGroupLast)/2 )/trackLength : frontRaceLogicPosition);
						component->SetMinRaceLogicPosition(useMinSize ? backRaceLogicPosition - ( (minDistForRubberBand-groupFirstDistanceToGroupLast)/2 )/trackLength : backRaceLogicPosition);
					}
					//LOGINFO(g_AmaxMpPlayerRubberBandingLog, "Banding vehicle %i to window (%f to %f), multiplier: %f", groupVehicleIndexes[j], 
					//	frontRaceLogicPosition + (frontBufferDistance/mTrackLength),backRaceLogicPosition - (backBufferDistance/mTrackLength), component->mRubberBandingModifier );
				}
				
				//all components set correctly, clear the vector ready for the next group (if there is one)
				groupVehicleIndexes.Clear();
			}
		}
	}

	/*
	*	Update the Sorcery mapping curve that maps {-1 <= n <= 1} to an impulse
	*/
	void C_VehicleComponent_PlayerRubberBanding::UpdateCurve()
	{
		const PlayerRubberBandingSettings& cache = C_MultiplayerData::Instance().m_pConfig->m_PlayerRubberBandingSettings;

		SpecialFX::C_HermiteKeyFrame keyFrame;

		s_impulseMagnitudeGraph.Clear();

		for(int j = 0; j < cache.m_ImpulseMagnitudeMapping.count; j++)
		{
			keyFrame = cache.m_ImpulseMagnitudeMapping[j];

			s_impulseMagnitudeGraph.AddKeyFrame(	keyFrame.mTime, 
													keyFrame.mValue, 
													keyFrame.mInTangent, 
													keyFrame.mOutTangent);
		}
	}

	/*
	*	Sort function for the qsort
	*/
	int C_VehicleComponent_PlayerRubberBanding::SortRacePositionsFunc(const void *a, const void *b)
	{
		RacePositionData* racePosDataA = (RacePositionData*)a;
		RacePositionData* racePosDataB = (RacePositionData*)b;

		if(racePosDataA->racePosition < racePosDataB->racePosition)
		{
			return 1;
		}
		else
		{
			return -1;
		}

		return 0;
	}

} // namespace Components
