#include "stdafx.h"

#include "VLC_FanDemands.h"

#include <GameLogic\Structures\LevelState.h>
#include <Game\Achievements\AchievementSystem.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/RaceLogicLayer.h>

#include <GameLogic\ActorStudioActors\FanDemandPositionActor.h>
#include <GameLogic\GameData\GameData.h>
#include <GameLogic\AchievementsManager.h>
#include <GameLogic\StickerManager.h>

#include "Ui/SoundEffects.h"
#include <Ui/Game/Gadgets/ButtonMessages.h>
#include <Ui/UiInterface.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>

#include <GameLogic/SinglePlayer/SP_BossDemands.h>

#include "Automation\GameDataLogging.h"


//#pragma optimize("", off)


Debug::Logger &gVLC_FanDemandLogger( Debug::GetLogger("FanDemands") );

//Debug::Float	gDebugFanDemandGracePeriod("Perks.FanDemand.GracePeriod", 3.0f, Debug::Persist());
Debug::Bool gDebugDontAlphaFanRunGates("Perks.FanDemand.DontAlphaFanRunGates", false);

Debug::Bool	gStreetRaceUnlimitedFanRunTime("Amax.GameMode.StreetRace.UnlimitedFanRunTime", false);
Debug::Float gStreetRaceFanRunGateTriggerDelay("Amax.GameMode.StreetRace.FanRunGateTriggerDelay", 0.1f, Debug::Range(0.0f, 1.0f), Debug::Persist());



namespace Components
{

bool C_VehicleComponent_Fan_Demand::CreateImpl()
{
	const I_VehicleComponent_Fan_Demand_CreateInfo* creation_data = static_cast<const I_VehicleComponent_Fan_Demand_CreateInfo *>(GetCreator().GetData());
	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

	if(creation_data)
	{
		mVehicleIndex=creation_data->mVehicleIndex;
		mRaceLogicLayerPtr=creation_data->mRaceLogicLayerPtr;
	}

	//AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_PickupMessage_PickupPickedUp );
	AddSubscriber( E_VehicleLogicMsg_Scored );
	AddSubscriber( Game::E_VehicleMessage_TriggerActorEvent );
	AddSubscriber( E_VehicleLogicMsg_HitByPerk );
	AddSubscriber( E_VehicleLogicMsg_FiredPerk );

	mFanDemandActive=false;
	mRaceLogicLayerPtr->mFanDemandActive=false;
	
	mInTriggerZone.Clear();

	m_bNewFanCountdown = 0;
	mCurrentGate = 0;
	mDelayedGateTriggerTimeLeft = 0.0f;

	return true;
}

void C_VehicleComponent_Fan_Demand::ClearContainers()
{
	mPassedMove.Clear();
	mPassedMoveThisFrame.Clear();

	for(int a=0;a<mScoreTotals.Size();a++)
	{
		mScoreTotals[a]=0;
	}

	mTimeAtSpeed=0.f;
	mStartPosition=-1;

	mNumWheelsOnGround=0;
	mJumpDistance=0.f;

	mSlipAmount=0.f;
	mDriftingAngle=0.f;

	mDrifting=false;
	mHandbrakeInduced=false;

	mDriftDistance=0.f;
	mDriftTime=0.f;

	mJumpCompletedDistance=0.f;
	mDriftCompletedDistance=0.f;
	mDriftCompletedTime=0.f;

	mNumberOfJumps=0;

	for(int a=0;a<E_PICKUP_TYPE_MAX;a++)
	{
		mPickupAquiredTotals[a]=0;
		mPickupUseTotals[a]=0;
		mPickupHitTotals[a]=0;

		mPickupDoubleAquiredTotals[a]=0;
		mPickupDoubleUseTotals[a]=0;
		mPickupDoubleHitTotals[a]=0;
	}

	mObjectsHit=0;
	mPreviousTypePickedUp=E_PICKUP_TYPE_NONE;

	mCurrentTriggerZoneSequence=FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_NOT_STARTED;
	mCurrentTriggerZoneSequenceNext=0;
	mCurrentTriggerZoneSequenceNumber=0;
	mCurrentTriggerZoneSequenceClockwise=false;
	
	mTriggerZoneHealthOnEnter.Clear();
	mTriggerZoneHealthOnExit.Clear();

	mNewScoreMoves.Clear();
}

bool C_VehicleComponent_Fan_Demand::IsInTriggerRegion(int ref)
{
	Btl::Map<int, bool>::Iterator it = mInTriggerZone.Find(ref);

	if(it != mInTriggerZone.End())
	{
		if(it->GetValue() == true)
		{
			return true;
		}
	}

	return false;
}

bool C_VehicleComponent_Fan_Demand::HasExitedTriggerRegion(int ref)
{
	Btl::Map<int, bool>::Iterator it = mInTriggerZone.Find(ref);

	if(it != mInTriggerZone.End())
	{
		if(it->GetValue() == false)
		{
			return true;
		}
	}

	return false;
}

bool C_VehicleComponent_Fan_Demand::DoneWithinTriggerRegion(XtArray<const Actors::TriggerRegionCreationData*> do_within)
{
	if(do_within.count == 0)
		return true;

	for(int a=0;a<do_within.count;a++)
	{
		if(IsInTriggerRegion(do_within[a]->m_RefNumber))
		{
			return true;
		}
	}

	return false;
}

void C_VehicleComponent_Fan_Demand::CalculateDriftsandJumps()
{
	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());
	const Vehicle::C_ModelInput *pModelInput = vehActor->GetGraphicalActor(mVehicleIndex)->GetVehicleInput();
	AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];

	mNumWheelsOnGround=0;
	mSlipAmount=0.f;

	for(int iWheel = 0; iWheel < pModelInput->m_Wheels.Size(); iWheel++)
	{
		mSlipAmount += abs(pModelInput->m_Wheels[iWheel].m_fSlipAngle);

		if(pModelInput->m_Wheels[iWheel].m_bOnGround)
		{
			mNumWheelsOnGround++;
		}
	}

	if(mNumWheelsOnGround == 0)
	{
		mJumpDistance+=vehActor->mPositionInfo.mDistanceTravelledSinceLastUpdate;
	}
	else
	{
		if(mJumpDistance)
		{
			mJumpCompletedDistance=mJumpDistance;
			mJumpDistance=0.f;
		}
	}

	mDrifting=false;

	Maths::HVec3 vel_heading=pModelInput->m_vVelocity;
	if(vel_heading.Length())
	{
		vel_heading.Normalise();
	}
	else
	{
		vel_heading=vehActor->mPositionInfo.mDirection;
	}

	mDriftingAngle=Maths::DotProduct(vel_heading,vehActor->mPositionInfo.mDirection);
	float angle_in_degs=(float)Maths::ArcCos(mDriftingAngle) * (180/Maths::Pi);

	if(angle_in_degs >= pScoringConfig->drift_angle_min && angle_in_degs <= pScoringConfig->drift_angle_max)
	{
		if(mSlipAmount >= pScoringConfig->drift_slip_min && mSlipAmount <= pScoringConfig->drift_slip_max)
		{
			mHandbrakeInduced=false;
			if(mDrifting == false)
			{
				if(vehActor->GetGraphicalActor(mVehicleIndex)->m_handbrakeControllerInput)
				{
					mHandbrakeInduced=true;
				}
			}

			mDrifting=true;
			mDriftDistance+=vehActor->mPositionInfo.mDistanceTravelledSinceLastUpdate;
			mDriftTime+=mDeltaTime;
		}
	}

	if(!mDrifting && mDriftDistance)
	{
		mDriftCompletedDistance=mDriftDistance;
		mDriftCompletedTime=mDriftTime;

		mDriftDistance=0.f;
		mDriftTime=0.f;
	}
}

//pretty hardcoded, but we're getting close to finish and can't nice shiny new systems that do everything
void C_VehicleComponent_Fan_Demand::EnteredTriggerZone(int ref)
{
	if(mCurrentTriggerZoneSequence == FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_COMPLETE || mCurrentTriggerZoneSequence == FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_FAILED)
		return;

	if(mCurrentTriggerZoneSequence == FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_NOT_STARTED)
	{
		mCurrentTriggerZoneSequence = ref;
		mCurrentTriggerZoneSequenceNumber++;
	}
	else
	{
		//already in a sequence

		if(mCurrentTriggerZoneSequenceNumber == 1)
		{
			if(mCurrentTriggerZoneSequence == 2000)
			{
				if(ref == 2007)
				{
					mCurrentTriggerZoneSequenceNext=2006;
					mCurrentTriggerZoneSequenceClockwise=false;
					mCurrentTriggerZoneSequenceNumber++;
				}
				else if(ref == 2001)
				{
					mCurrentTriggerZoneSequenceNext=2002;
					mCurrentTriggerZoneSequenceClockwise=true;
					mCurrentTriggerZoneSequenceNumber++;
				}
				else
				{
					mCurrentTriggerZoneSequence = FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_FAILED;
				}
			}
			else if(mCurrentTriggerZoneSequence == 2007)
			{
				if(ref == 2006)
				{
					mCurrentTriggerZoneSequenceNext=2005;
					mCurrentTriggerZoneSequenceClockwise=false;
					mCurrentTriggerZoneSequenceNumber++;
				}
				else if(ref == 2000)
				{
					mCurrentTriggerZoneSequenceNext=2001;
					mCurrentTriggerZoneSequenceClockwise=true;
					mCurrentTriggerZoneSequenceNumber++;
				}
				else
				{
					mCurrentTriggerZoneSequence = FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_FAILED;
				}
			}
			else
			{
				if(ref == mCurrentTriggerZoneSequence-1)
				{
					mCurrentTriggerZoneSequenceNext=mCurrentTriggerZoneSequence-2;
					mCurrentTriggerZoneSequenceClockwise=false;
					mCurrentTriggerZoneSequenceNumber++;
				}
				else if(ref == mCurrentTriggerZoneSequence+1)
				{
					mCurrentTriggerZoneSequenceNext=mCurrentTriggerZoneSequence+2;
					mCurrentTriggerZoneSequenceClockwise=true;
					mCurrentTriggerZoneSequenceNumber++;
				}
				else
				{
					mCurrentTriggerZoneSequence = FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_FAILED;
				}
			}
		}
		else	//must be further into the Sequence than 1 or 2
		{
			if(ref == mCurrentTriggerZoneSequenceNext)
			{
				mCurrentTriggerZoneSequenceNumber++;
				if(mCurrentTriggerZoneSequenceNumber == 8)
				{
					mCurrentTriggerZoneSequence = FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_COMPLETE;
					return;
				}
				
				if(mCurrentTriggerZoneSequenceClockwise)
				{
					mCurrentTriggerZoneSequenceNext++;
					if(mCurrentTriggerZoneSequenceNext == 2008)
						mCurrentTriggerZoneSequenceNext=2000;
				}
				if(!mCurrentTriggerZoneSequenceClockwise)
				{
					mCurrentTriggerZoneSequenceNext--;
					if(mCurrentTriggerZoneSequenceNext == 1999)
						mCurrentTriggerZoneSequenceNext=2007;
				}
			}
			else
			{
				//retry
				mCurrentTriggerZoneSequence=FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_NOT_STARTED;
				mCurrentTriggerZoneSequenceNext=0;
				mCurrentTriggerZoneSequenceNumber=0;
				mCurrentTriggerZoneSequenceClockwise=false;
			}
		}
	}
}

void C_VehicleComponent_Fan_Demand::ExitedTriggerZone(int ref)
{

}


void C_VehicleComponent_Fan_Demand::Update()
{
	PERF_EVENT("C_VehicleComponent.FanDemand.Update");

	mDeltaTime = C_GameApp::GetInstance()->GetTimeStep();


	if (mDelayedGateTriggerTimeLeft > 0.0f)						// Check for a delayed fan run gate
	{
		mDelayedGateTriggerTimeLeft -= mDeltaTime;
		if (mDelayedGateTriggerTimeLeft <= 0.0f)
		{
			const Actors::FanDemandPositionCreationData *fd_data = ((Actors::FanDemandPositionActor *)GetActorList()->GetActorPtr(mCurrentFanDemandHandle))->GetCreationData();
			mFans += fd_data->mFanDemand.gates[mCurrentGate].num_fans_awarded;			// Add fans for this gate
			mCurrentGate++;
			mDelayedGateTriggerTimeLeft = 0.0f;
			if (mCurrentGate == fd_data->mFanDemand.gates.count)
				EndDemand(true);
			else
			{
				UpdateGateStates();
				UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_PassedFanRunCheckpointActionIn );
			}

			Game::C_StickerManager::Instance().HitFanGate(); // there's a sticker for an achievement involving this
		}
	}


	if(mFanDemandActive)
	{
		AmaxScoringConfig* pScoringConfig = C_GameData::Instance().mScoringConfig[0];
		Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

		const Actors::FanDemandPositionCreationData *fd_data=((Actors::FanDemandPositionActor *)GetActorList()->GetActorPtr(mCurrentFanDemandHandle))->GetCreationData();

		CalculateDriftsandJumps();

		//Check all the moves and see whether they're passed yet
		for(int a=0;a<fd_data->mFanDemand.moves.count;a++)
		{
			if (fd_data->mFanDemand.do_in_order)	// If this is not the first move and we have to do them in order
				if (a>0)
					if (!mPassedMove[a-1])			// then don't check if the previous one hasn't been completed
						continue;


			mPassedMoveThisFrame[a]=false;

			switch(fd_data->mFanDemand.moves[a]->type)
			{
				case Actors::FANDEMANDMOVE_TYPE_TRIGGER_BOX:
					{
						const Actors::FanDemandMoveTriggerBox *tb_data = (Actors::FanDemandMoveTriggerBox *)fd_data->mFanDemand.moves[a];

						if(tb_data->usage == Actors::FANDEMANDMOVE_TRIGGERBOX_TYPE_ENTER)
						{
							BIZ_ASSERT_MSG(tb_data->trigger_box, "NULL Triggerbox in Fan Demand");
							if(IsInTriggerRegion(tb_data->trigger_box->m_RefNumber) && (tb_data->time_out == 0.f || mTimer <= tb_data->time_out))
							{
								mPassedMove[a]=true;
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
						else if(tb_data->usage == Actors::FANDEMANDMOVE_TRIGGERBOX_TYPE_EXIT)
						{
							if(HasExitedTriggerRegion(tb_data->trigger_box->m_RefNumber) && (tb_data->time_out == 0.f || mTimer <= tb_data->time_out))
							{
								mPassedMove[a]=true;
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
						else if(tb_data->usage == Actors::FANDEMANDMOVE_TRIGGERBOX_TYPE_DONT_ENTER)
						{
							if(!IsInTriggerRegion(tb_data->trigger_box->m_RefNumber) && (tb_data->time_out == 0.f || mTimer <= tb_data->time_out))
							{
								mPassedMove[a]=true;
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
						else if(tb_data->usage == Actors::FANDEMANDMOVE_TRIGGERBOX_TYPE_IN_SEQUENCE)
						{
							if(mCurrentTriggerZoneSequence == FAN_DEMAND_TRIGGER_ZONE_SEQUENCE_COMPLETE)
							{
								mPassedMove[a]=true;
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
					}
					break;

				case Actors::FANDEMANDMOVE_TYPE_SCORE:
					{
						const Actors::FanDemandMoveScore *score_data = (Actors::FanDemandMoveScore *)fd_data->mFanDemand.moves[a];

						int num_moves=0;

						for(int s=0 ; s < mNewScoreMoves.Size() ; s++)
						{
							bool valid=false;

							//is the base move valid
							if(score_data->base_move == mNewScoreMoves[s].mBaseMove)
							{
								int move_count=1;

								//check the skill moves
								int skill_moves=0;
								for(int s1=0 ; s1 < score_data->skills.count ; s1++)
								{
									for( int s2=0 ; s2 < mNewScoreMoves[s].mSkillMoves.Size() ; s2++)
									{
										//welcome to exception corner - stay tuned for more exceptions in the future
										if(score_data->base_move == E_BLUR_SCORE_MOVE_BASE_DRIFTING || score_data->base_move == E_BLUR_SCORE_MOVE_BASE_OVERTAKE)
										{
											if(mNewScoreMoves[s].mSkillMoves[s2] >= score_data->skills[s1])
											{
												skill_moves++;
											}
										}
										else
										{
											if(score_data->skills[s1] == mNewScoreMoves[s].mSkillMoves[s2])
											{
												skill_moves++;
											}
										}
									}
								}

								//check the bonus moves
								int bonus_moves=0;
								for(int s1=0 ; s1 < score_data->bonuses.count ; s1++)
								{
									for( int s2=0 ; s2 < mNewScoreMoves[s].mBonusMoves.Size() ; s2++)
									{
										if(score_data->bonuses[s1] == mNewScoreMoves[s].mBonusMoves[s2])
										{
											bonus_moves++;
										}
									}
								}

								if(score_data->base_move == E_BLUR_SCORE_MOVE_BASE_BARGE)
								{
									for( int s2=0 ; s2 < mNewScoreMoves[s].mSkillMoves.Size() ; s2++)
									{
										if(mNewScoreMoves[s].mSkillMoves[s2] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_1)
											move_count=2;
										if(mNewScoreMoves[s].mSkillMoves[s2] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_2)
											move_count=3;
										if(mNewScoreMoves[s].mSkillMoves[s2] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_3)
											move_count=4;
										if(mNewScoreMoves[s].mSkillMoves[s2] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_4)
											move_count=5;
										if(mNewScoreMoves[s].mSkillMoves[s2] == E_BLUR_SCORE_MOVE_SKILL_LEVEL_5)
											move_count=6;
									}
								}

								if(skill_moves == score_data->skills.count && bonus_moves == score_data->bonuses.count)
								{
									num_moves+=move_count;
								}
							}

						}

						if(num_moves >= score_data->number_needed)
						{
							mPassedMove[a]=DoneWithinTriggerRegion(score_data->do_within);
							mPassedMoveThisFrame[a]=mPassedMove[a];
						}


						//if(mScoreTotals[score_data->score] >= score_data->number)
						//{
						//	mPassedMove[a]=DoneWithinTriggerRegion(score_data->do_within);
						//	mPassedMoveThisFrame[a]=mPassedMove[a];
						//}

					}
					break;

				case Actors::FANDEMANDMOVE_TYPE_OVERTAKE:
					{
						const Actors::FanDemandMoveOvertake *overtake_data = (Actors::FanDemandMoveOvertake *)fd_data->mFanDemand.moves[a];

						if((mStartPosition - vehActor->mPositionInfo.mRacePosition) >= overtake_data->number)
						{
							mPassedMove[a]=DoneWithinTriggerRegion(overtake_data->do_within);
							mPassedMoveThisFrame[a]=mPassedMove[a];
						}
					}
					break;

				case Actors::FANDEMANDMOVE_TYPE_JUMP:
					{
						const Actors::FanDemandMoveJump *jump_data = (Actors::FanDemandMoveJump *)fd_data->mFanDemand.moves[a];

						if(mJumpCompletedDistance)	//we have done a jump
						{
							if(mJumpCompletedDistance > jump_data->distance)
							{
								mNumberOfJumps++;

								if(mNumberOfJumps >= jump_data->number)
								{
									mPassedMove[a]=DoneWithinTriggerRegion(jump_data->do_within);
									mPassedMoveThisFrame[a]=mPassedMove[a];
								}
							}
						}

					}
					break;

				case Actors::FANDEMANDMOVE_TYPE_SPEED:
					{
						const Actors::FanDemandMoveSpeed *speed_data = (Actors::FanDemandMoveSpeed *)fd_data->mFanDemand.moves[a];

						float mph=vehActor->mPositionInfo.mSpeedInMph;

						if(speed_data->usage == Actors::FANDEMANDMOVE_SPEED_TYPE_STAY_AT)
						{
							if(mph > speed_data->mph)
							{
								mTimeAtSpeed+=mDeltaTime;
							}
							else
							{
								mTimeAtSpeed=0.f;
							}

							if(mTimeAtSpeed >= speed_data->time)
							{
								mPassedMove[a]=DoneWithinTriggerRegion(speed_data->do_within);
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
						else if(speed_data->usage == Actors::FANDEMANDMOVE_SPEED_TYPE_GET_TO)
						{
							if(mph >= speed_data->mph)
							{
								mPassedMove[a]=DoneWithinTriggerRegion(speed_data->do_within);
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
					}
					break;

				case Actors::FANDEMANDMOVE_TYPE_DRIFTING:
					{
						const Actors::FanDemandMoveDrifting *drifting_data = (Actors::FanDemandMoveDrifting *)fd_data->mFanDemand.moves[a];

						if(mDriftCompletedDistance && mDriftCompletedTime)	//you can't do a zero drift in some time or vice versus, hence the &&
							if((drifting_data->usage == Actors::FANDEMANDMOVE_DRIFTING_TYPE_HANDBRAKE && mHandbrakeInduced) || (drifting_data->usage == Actors::FANDEMANDMOVE_DRIFTING_TYPE_NORMAL && !mHandbrakeInduced))
						{
							if(mDriftCompletedDistance >= drifting_data->distance && mDriftCompletedTime > drifting_data->time)
							{
								mPassedMove[a]=DoneWithinTriggerRegion(drifting_data->do_within);
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
					}
					break;

				case Actors::FANDEMANDMOVE_TYPE_PICKUPS:
					{
						const Actors::FanDemandMovePickups *pickups_data = (Actors::FanDemandMovePickups *)fd_data->mFanDemand.moves[a];

						if(pickups_data->usage == Actors::FANDEMANDMOVE_PICKUPS_TYPE_HIT)
						{
							if((mPickupHitTotals[pickups_data->pickup_type] >= pickups_data->amount) && (!mPassedMove[a]))
							{
								mPassedMove[a]=DoneWithinTriggerRegion(pickups_data->do_within);
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
						else if(pickups_data->usage == Actors::FANDEMANDMOVE_PICKUPS_TYPE_PICKUP)
						{
							if((mPickupAquiredTotals[pickups_data->pickup_type] >= pickups_data->amount) && (!mPassedMove[a]))
							{
								mPassedMove[a]=DoneWithinTriggerRegion(pickups_data->do_within);
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
						else if(pickups_data->usage == Actors::FANDEMANDMOVE_PICKUPS_TYPE_USE)
						{
							if((mPickupUseTotals[pickups_data->pickup_type] >= pickups_data->amount) && (!mPassedMove[a]))
							{
								mPassedMove[a]=DoneWithinTriggerRegion(pickups_data->do_within);
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}

						else if(pickups_data->usage == Actors::FANDEMANDMOVE_PICKUPS_TYPE_HIT_DOUBLE)
						{
							if((mPickupDoubleHitTotals[pickups_data->pickup_type] >= pickups_data->amount) && (!mPassedMove[a]))
							{
								mPassedMove[a]=DoneWithinTriggerRegion(pickups_data->do_within);
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
						else if(pickups_data->usage == Actors::FANDEMANDMOVE_PICKUPS_TYPE_PICKUP_DOUBLE)
						{
							if((mPickupDoubleAquiredTotals[pickups_data->pickup_type] >= pickups_data->amount) && (!mPassedMove[a]))
							{
								mPassedMove[a]=DoneWithinTriggerRegion(pickups_data->do_within);
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
						else if(pickups_data->usage == Actors::FANDEMANDMOVE_PICKUPS_TYPE_USE_DOUBLE)
						{
							if((mPickupDoubleUseTotals[pickups_data->pickup_type] >= pickups_data->amount) && (!mPassedMove[a]))
							{
								mPassedMove[a]=DoneWithinTriggerRegion(pickups_data->do_within);
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}

						else if(pickups_data->usage == Actors::FANDEMANDMOVE_PICKUPS_TYPE_DONT_HIT)
						{
							mPassedMove[a]=true;
							if(mPickupHitTotals[pickups_data->pickup_type] > pickups_data->amount)
							{
								mPassedMove[a]=false;
							}
							mPassedMoveThisFrame[a]=mPassedMove[a];
						}
						else if(pickups_data->usage == Actors::FANDEMANDMOVE_PICKUPS_TYPE_DONT_PICKUP)
						{
							mPassedMove[a]=true;
							if(mPickupAquiredTotals[pickups_data->pickup_type] > pickups_data->amount)
							{
								mPassedMove[a]=false;
							}
							mPassedMoveThisFrame[a]=mPassedMove[a];
						}
						else if(pickups_data->usage == Actors::FANDEMANDMOVE_PICKUPS_TYPE_DONT_USE)
						{
							mPassedMove[a]=true;
							if(mPickupUseTotals[pickups_data->pickup_type] > pickups_data->amount)
							{
								mPassedMove[a]=false;
							}
							mPassedMoveThisFrame[a]=mPassedMove[a];
						}
					}
					break;

				case Actors::FANDEMANDMOVE_TYPE_CARNAGE:
					{
						const Actors::FanDemandMoveCarnage *carnage_data = (Actors::FanDemandMoveCarnage *)fd_data->mFanDemand.moves[a];

						if(mTimer == 0)	//do a little init(ing)
						{
							if(carnage_data->do_within.count == 0)
							{
								mRaceLogicLayerPtr->StartObjectHitCounting(vehActor->GetGraphicalActor(mVehicleIndex)->GetHandle());
							}
						}
						else
						{
							if(DoneWithinTriggerRegion(carnage_data->do_within))
							{
								if(!mRaceLogicLayerPtr->IsObjectHitCounting())
								{
									mRaceLogicLayerPtr->StartObjectHitCounting(vehActor->GetGraphicalActor(mVehicleIndex)->GetHandle());
								}
							}
							else
							{
								if(mRaceLogicLayerPtr->IsObjectHitCounting())
								{
									mObjectsHit+=mRaceLogicLayerPtr->GetObjectHitCounting();
									mRaceLogicLayerPtr->EndObjectHitCounting();
								}
							}

							if(mObjectsHit + mRaceLogicLayerPtr->GetObjectHitCounting() >= carnage_data->number)
							{
								mPassedMove[a]=true;
								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}

						LOGDEBUG(gVLC_FanDemandLogger, "Carnage: %d objects hit", mObjectsHit + mRaceLogicLayerPtr->GetObjectHitCounting());
					}
					break;

				case Actors::FANDEMANDMOVE_TYPE_DAMAGE:
					{
						const Actors::FanDemandMoveDamage *damage_data = (Actors::FanDemandMoveDamage *)fd_data->mFanDemand.moves[a];

						if(damage_data->do_within.count)
						{
							for(int b=0;b<damage_data->do_within.count;b++)
							{
								int ref=damage_data->do_within[b]->m_RefNumber;

								Btl::Map<int, int>::Iterator it_en = mTriggerZoneHealthOnEnter.Find(ref);

								if(it_en != mTriggerZoneHealthOnEnter.End())
								{
									Btl::Map<int, int>::Iterator it_ex = mTriggerZoneHealthOnExit.Find(ref);

									if(it_ex != mTriggerZoneHealthOnExit.End())
									{
										int health=it_en->GetValue() - it_ex->GetValue();

										if(damage_data->usage == Actors::FANDEMANDMOVE_DAMAGE_TYPE_STAY_BELOW)
										{
											if(health < damage_data->amount)
											{
												mPassedMove[a]=true;
											}
											else
											{
												mPassedMove[a]=false;
											}
										}
										else if(damage_data->usage == Actors::FANDEMANDMOVE_DAMAGE_TYPE_DO_OVER)
										{
											if(-health < damage_data->amount)
											{
												mPassedMove[a]=true;
											}
											else
											{
												mPassedMove[a]=false;
											}
										}

										mTriggerZoneHealthOnEnter.Erase(it_en->GetKey());
										mTriggerZoneHealthOnExit.Erase(it_ex->GetKey());
									}
								}
							}
							mPassedMoveThisFrame[a]=mPassedMove[a];
						}
						else
						{
							if(damage_data->usage == Actors::FANDEMANDMOVE_DAMAGE_TYPE_STAY_BELOW)
							{
								mPassedMove[a]=true;
								if( (mStartHealth - vehActor->GetCurrentHealth()) > damage_data->amount )
								{								
									mPassedMove[a]=false;
								}

								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
							else if(damage_data->usage == Actors::FANDEMANDMOVE_DAMAGE_TYPE_DO_OVER)
							{
								mPassedMove[a]=true;
								if( (mStartHealth - vehActor->GetCurrentHealth()) < damage_data->amount )
								{								
									mPassedMove[a]=false;
								}

								mPassedMoveThisFrame[a]=mPassedMove[a];
							}
						}
					}
					break;

				case Actors::FANDEMANDMOVE_TYPE_MISC:
					{
						const Actors::FanDemandMoveMisc *misc_data = (Actors::FanDemandMoveMisc *)fd_data->mFanDemand.moves[a];

						if(misc_data->usage == Actors::FANDEMANDMOVE_MISC_TYPE_360)
						{



						}
					}
					break;

				default:
					break;

			}
		}

		//check to see if we are complete yet
		bool has_passed=false;
		int num_passed=0;

		if(fd_data->mFanDemand.do_all_on_same_frame)
		{
			for(int a=0;a<mPassedMoveThisFrame.Size();a++)
			{
				if(mPassedMoveThisFrame[a])
					num_passed++;
			}
		}
		else
		{
			for(int a=0;a<mPassedMove.Size();a++)
			{
				if(mPassedMove[a])
					num_passed++;
			}
		}

		if(fd_data->mFanDemand.number_to_do == 0)					// Have we passed all the criteria
		{
			if(num_passed == mPassedMove.Size())
			{
				has_passed=true;
			}
		}
		else
		{
			if(num_passed >= fd_data->mFanDemand.number_to_do )
			{
				has_passed=true;
			}
		}

		mTimer += mDeltaTime;										// Decrement number of fans left based on time
//		mFans -= pScoringConfig->fan_demand_time * (mDeltaTime * (fd_data->mFanDemand.time_multiplier ? fd_data->mFanDemand.time_multiplier : 1.f));

		if (mGracePeriod > 0.0f)
		{
			mGracePeriod -= mDeltaTime;
		}
		else
		{
			if (!gStreetRaceUnlimitedFanRunTime)
			{
				if (fd_data->mFanDemand.time_to_complete)
					mFans -= (fd_data->mFanDemand.fans / fd_data->mFanDemand.time_to_complete) * mDeltaTime;
				else
					mFans -= (fd_data->mFanDemand.fans / 5.0f) * mDeltaTime;		// Default to complete in 5 secs if not specified
			}
		}


		//send it every new whole int
		if(m_bNewFanCountdown != S32(mFans))
		{
			//send mFans to audio
			UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_FanDemandCountdownActionIn, mFans );
			m_bNewFanCountdown = mFans;
		}


		if (mFans <= 0)						// If failed
		{
			EndDemand(false);
		}
		else if (has_passed)				// If passed
		{
			EndDemand(true);
		}
		else								// If on-going
		{
			UI::C_UiFanDemand uimsg((U32)mFans,true,1,UI::C_TextStringID(fd_data->mFanDemand.string.id).String(),mVehicleIndex,fd_data->mFanDemand.gold_demand);
			_UI_MESSAGE_FULL( uimsg );
		}
	}

	mJumpCompletedDistance=0.f;
	mDriftCompletedDistance=0.f;
	mDriftCompletedTime=0.f;
}


void C_VehicleComponent_Fan_Demand::EndDemand(bool has_passed)
{
	if (!mFanDemandActive)
		return;

	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

	const Actors::FanDemandPositionCreationData *fd_data = ((Actors::FanDemandPositionActor *)GetActorList()->GetActorPtr(mCurrentFanDemandHandle))->GetCreationData();
	for(S32 a=0;a<fd_data->mFanDemand.demand_markers.count;a++)			// Send deactivate message to any associated marker effects
		if (fd_data->mFanDemand.demand_markers[a])
			SendMessageToActor(fd_data->mFanDemand.demand_markers[a]->GetActorHandle(), ActorMsg::E_GameActorMsg_Deactivate);
	for(S32 a=0;a<fd_data->mFanDemand.gates.count;a++)					// plus any markers that are part of a fan run gate
	{
		if (fd_data->mFanDemand.gates[a].gate_marker_left)
			SendMessageToActor(fd_data->mFanDemand.gates[a].gate_marker_left->GetActorHandle(), ActorMsg::E_GameActorMsg_Deactivate);
		if (fd_data->mFanDemand.gates[a].gate_marker_right)
			SendMessageToActor(fd_data->mFanDemand.gates[a].gate_marker_right->GetActorHandle(), ActorMsg::E_GameActorMsg_Deactivate);
	}


	int pickups_remaining=0;

	Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
	if ((levelState.mRaceMode == Game::E_RaceMode_FanDemand) || (levelState.mRaceMode == Game::E_RaceMode_FriendDemand_FanDemand))		// If in FanDemand race mode and this Fan Demand gave pickups them take them all away when it finishes
	{
		if (fd_data->mFanDemand.gift_slot1 || fd_data->mFanDemand.gift_slot2 || fd_data->mFanDemand.gift_slot3)
		{
			Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

			pickups_remaining=pLayer->GetLogicVehicleActor(0)->mPickupInfo.mPickupStore.Size();

			Game::Actors::PickupRemovePowerMsg removePickupMsg(0, 1, true);		//Player vehicle, power, remove infinates
//			for(S32 a=0;a<3;a++)
//				pLayer->GetLogicVehicleActor(0)->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &removePickupMsg);
		}
	}

	UI::C_WideString64 typeStr;
	typeStr.Format(L"%s", UI::C_TextStringID(fd_data->mFanDemand.string.id).String());

	if(has_passed)
	{
		mFans += fd_data->mFanDemand.fan_bonus_on_completion;				// Golden (or any other if required) give a bonus number of fans when they're completed
#ifndef CFG_GOLD
		AmaxDataLogging::LogFanDemandOver(typeStr.AsString().Chars(), mFans, true);
#endif
		const C_VehicleLogicMsg_Fan_Demand_Passed msg(mVehicleIndex, mFans, (fd_data->mFanDemand.fans == mFans), pickups_remaining, fd_data->mFanDemand.gold_demand);
		GetActor()->SendMessageToComponent(Components::E_IF_VehicleComponent_NewScoring, &msg);

		vehActor->mPickupInfo.mNumFanPickupsCompleted.Insert(mCurrentFanDemandHandle);

		//Game::C_AchievementsManager::Instance().FanDemandCompleted(mVehicleIndex);

		UI::UiInterface()->PlaySound(UI::E_UiSoundEffect_FanDemandSuccessActionIn );			//send pass to audio

		vehActor->GetScoring().mFanDemandsComplete++;

		if (fd_data->mFanDemand.gold_demand)													// Remember that we've completed a gold demand
		{
			if (!fd_data->mFanDemand.chained_demand)											// but only single ones and the last in a chain
			{
				vehActor->GetScoring().mGoldenFanDemandComplete = true;
				vehActor->GetScoring().mGoldenFanDemandScore = mFans;
				SP_BossDemands::Instance().RegisterBossDemandAction(E_BOSSDEMAND_ACTION_COMPLETED_SINGLE_FAN_RUN);
				Game::C_StickerManager::Instance().CompletedFanGate();
			}
		}
		else
		{
			SP_BossDemands::Instance().RegisterBossDemandAction(E_BOSSDEMAND_ACTION_COMPLETE_FAN_DEMAND);
			Game::C_StickerManager::Instance().CompletedFanDemand();
		}

		if (fd_data->mFanDemand.chained_demand)													// Activate a chained fan demand pickup
			SendMessageToActor(fd_data->mFanDemand.chained_demand->GetActorHandle(), Game::E_PickupMessage_ActivatePickup);

	}
	else
	{
#ifndef CFG_GOLD
		AmaxDataLogging::LogFanDemandOver(typeStr.AsString().Chars(), mFans < 0.0f ? 0 : mFans, false);
#endif
		UI::UiInterface()->PlaySound(UI::E_UiSoundEffect_FanDemandFailActionIn );				//send fail to audio

		const C_VehicleLogicMsg_Fan_Demand_Passed msg(mVehicleIndex, 0, false, 0, fd_data->mFanDemand.gold_demand, true);
		GetActor()->SendMessageToComponent(Components::E_IF_VehicleComponent_NewScoring, &msg);

		// Reset any Fan Demands before this in the same chain
		const Actors::FanDemandPositionCreationData *fd_data = ((Actors::FanDemandPositionActor *)GetActorList()->GetActorPtr(mCurrentFanDemandHandle))->GetCreationData();
		for(S32 a=0;a<fd_data->mFanDemand.reset_demands.count;a++)			// Loop through all fail zones
			if (fd_data->mFanDemand.reset_demands[a])
				SendMessageToActor(fd_data->mFanDemand.reset_demands[a]->GetActorHandle(), Game::E_PickupMessage_ResetPickup);

		SendMessageToActor(mCurrentFanDemandHandle, Game::E_PickupMessage_ResetPickup);			// Make sure the current pickup is switched off if it was in a chain and set to start disabled
	}

	mFanDemandActive=false;
	mRaceLogicLayerPtr->mFanDemandActive=false;

	mRaceLogicLayerPtr->EndObjectHitCounting();

	::Actors::C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( Game::E_PickupMessage_UpdateFanDemandState );		// Update all the pickups

	UI::C_UiFanDemand uimsg((U32)mFans,true,has_passed?-1:0,UI::C_TextStringID(fd_data->mFanDemand.string.id).String(),mVehicleIndex,fd_data->mFanDemand.gold_demand);
	_UI_MESSAGE_FULL( uimsg );


}



// Message Handlers
bool C_VehicleComponent_Fan_Demand::OnMessage(const Messaging::C_MessageBase *pMessage) 
{
	PERF_EVENT("C_VehicleComponent.Fan_Demand.MessageHandler");

	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

	switch(pMessage->Type())
	{
		//case GameMessaging::E_GameMsg_Update:
		//	{
		//		mDeltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
		//		Update();

		//		return true;
		//	}
		//	break;

		case E_VehicleLogicMsg_Scored:
			{
				const C_VehicleLogicMsg_Scored* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_Scored >(pMessage);
				
				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					//mScoreTotals[pMsg->mScore]++;
					mNewScoreMoves.PushBack(*pMsg);
				}

				return true;
			}
			break;

		case Game::E_VehicleMessage_TriggerActorEvent:
			{
				const Game::Actors::C_VehicleCheckTriggerActorEvent* pMsg	= Messaging::MessageCast<Game::Actors::C_VehicleCheckTriggerActorEvent>(pMessage);
				const Actors::TriggerRegionActor* pTriggerActor = (const Actors::TriggerRegionActor*)pMsg->mTriggerObject;


				if (mFanDemandActive)				// Check for this being a fail trigger regionr a gate
				{
					const Actors::FanDemandPositionCreationData *fd_data = ((Actors::FanDemandPositionActor *)GetActorList()->GetActorPtr(mCurrentFanDemandHandle))->GetCreationData();

					for(S32 a=0;a<fd_data->mFanDemand.fail_trigger_zones.count;a++)			// Loop through all fail zones
						if (fd_data->mFanDemand.fail_trigger_zones[a] == pTriggerActor->GetCreationData())
						{
							EndDemand(false);											// If it's in the list then fail the demand
							return(true);
						}

//					for(S32 a=0;a<fd_data->mFanDemand.gates.count;a++)						// Loop through all fail zones incorporated in fan run gates
					for(S32 a=mCurrentGate;a<fd_data->mFanDemand.gates.count;a++)			// Loop through fail zones incorporated in fan run gates (ahead of us only)
						if ((fd_data->mFanDemand.gates[a].fail_trigger_left == pTriggerActor->GetCreationData()) || (fd_data->mFanDemand.gates[a].fail_trigger_right == pTriggerActor->GetCreationData()))
						{
							EndDemand(false);											// If it's in the list then fail the demand
							return(true);
						}

					if (mCurrentGate < fd_data->mFanDemand.gates.count)						// Have we just gone through the next gate?
						if (fd_data->mFanDemand.gates[mCurrentGate].trigger == pTriggerActor->GetCreationData())
						{
							mFans += fd_data->mFanDemand.gates[mCurrentGate].num_fans_awarded;			// Add fans for this gate
							mCurrentGate++;
							mDelayedGateTriggerTimeLeft = 0.0f;
							if (mCurrentGate == fd_data->mFanDemand.gates.count)
								EndDemand(true);
							else
							{
								UpdateGateStates();
								UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_PassedFanRunCheckpointActionIn );
							}

							Game::C_StickerManager::Instance().HitFanGate(); // there's a sticker for an achievement involving this
						}

					if (mCurrentGate < fd_data->mFanDemand.gates.count)						// Have we just gone through the next gate?
						if (fd_data->mFanDemand.gates[mCurrentGate].trigger == pTriggerActor->GetCreationData())
							if (mDelayedGateTriggerTimeLeft <= 0.0f)				// Move to next gate after a short pause in case we're about to go through the fail regions
								mDelayedGateTriggerTimeLeft = gStreetRaceFanRunGateTriggerDelay;


/*
					if (mCurrentGate < fd_data->mFanDemand.gates.count)						// Have we just gone through the next gate?
						if (fd_data->mFanDemand.gates[mCurrentGate].trigger == pTriggerActor->GetCreationData())
						{
							mFans += fd_data->mFanDemand.gates[mCurrentGate].num_fans_awarded;			// Add fans for this gate
							mCurrentGate++;
							mDelayedGateTriggerTimeLeft = 0.0f;
							if (mCurrentGate == fd_data->mFanDemand.gates.count)
								EndDemand(true);
							else
							{
								UpdateGateStates();
								UI::UiInterface()->PlaySound( UI::E_UiSoundEffect_PassedFanRunCheckpointActionIn );
							}

							Game::C_StickerManager::Instance().HitFanGate(); // there's a sticker for an achievement involving this
						}
*/

				}


				if(pTriggerActor->GetCreationData()->m_RefNumber >= 1000)
				{
					if(pMsg->mEvent->GetEventType() == Gravity::C_TriggerEvent::E_Enter_Event)
					{
						Btl::Map<int, bool>::InsertionReturnType retType = mInTriggerZone.Insert(pTriggerActor->GetCreationData()->m_RefNumber,true);
						if(!retType.inserted)
						{
							Btl::Map<int, bool>::Iterator it = retType.iterator;
							it->GetValue() = true;
						}

						{
							Btl::Map<int, int>::InsertionReturnType retType = mTriggerZoneHealthOnEnter.Insert(pTriggerActor->GetCreationData()->m_RefNumber,vehActor->GetCurrentHealth());
							if(!retType.inserted)
							{
								Btl::Map<int, int>::Iterator it = retType.iterator;
								it->GetValue() = vehActor->GetCurrentHealth();
							}
						}
					}
					else if (pMsg->mEvent->GetEventType() == Gravity::C_TriggerEvent::E_Exit_Event)
					{
						Btl::Map<int, bool>::InsertionReturnType retType = mInTriggerZone.Insert(pTriggerActor->GetCreationData()->m_RefNumber,false);
						if(!retType.inserted)
						{
							Btl::Map<int, bool>::Iterator it = retType.iterator;
							it->GetValue() = false;
						}

						{
							Btl::Map<int, int>::InsertionReturnType retType = mTriggerZoneHealthOnExit.Insert(pTriggerActor->GetCreationData()->m_RefNumber,vehActor->GetCurrentHealth());
							if(!retType.inserted)
							{
								Btl::Map<int, int>::Iterator it = retType.iterator;
								it->GetValue() = vehActor->GetCurrentHealth();
							}
						}
					}

					//special boxes for doing 360's around things
					if(pTriggerActor->GetCreationData()->m_RefNumber >= 2000)
					{
						if(pMsg->mEvent->GetEventType() == Gravity::C_TriggerEvent::E_Enter_Event)
						{
							EnteredTriggerZone(pTriggerActor->GetCreationData()->m_RefNumber);
						}
						else if (pMsg->mEvent->GetEventType() == Gravity::C_TriggerEvent::E_Exit_Event)
						{
							ExitedTriggerZone(pTriggerActor->GetCreationData()->m_RefNumber);
						}
					}
				}

				return true;
			}
			break;

		case Game::E_PickupMessage_PickupPickedUp:
			{
				const Game::Actors::PickupPickedUpMsg* pMsg = Messaging::MessageCast< Game::Actors::PickupPickedUpMsg >(pMessage);
				Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());

				if(pMsg->mePickupType == E_PICKUP_TYPE_FAN && pMsg->mVehicleIndex == mVehicleIndex)
				{
					if(GetActorList()->GetActorPtr(pMsg->mPickupActorHandle)->GetActorTypeId() != Game::E_ActorTypeId_FanDemandPosition)
						return true;

					if (mFanDemandActive)				// We can only have one active at any one time
						return(true);

					const Actors::FanDemandPositionCreationData *fd_data = ((Actors::FanDemandPositionActor *)GetActorList()->GetActorPtr(pMsg->mPickupActorHandle))->GetCreationData();

/*
					if ((vehActor->GetPickupInfo().mPickupStore.Size() == 3) && (fd_data->mFanDemand.gift_slot1 || fd_data->mFanDemand.gift_slot2 || fd_data->mFanDemand.gift_slot3))
					{
						// Pete R - WE NEED A UI MESSAGE HERE TO DISPLAY "ALL SLOTS ARE FULL"
						return(true);
					}
*/

					//if we're overwriting then its classed as a fail
//					if (mFanDemandActive)
//						vehActor->mPickupInfo.mNumFanPickupsFailed.Insert(mCurrentFanDemandHandle);

					mFanDemandActive = true;
					mRaceLogicLayerPtr->mFanDemandActive=true;
					mCurrentGate = 0;
					mDelayedGateTriggerTimeLeft = 0.0f;
					mCurrentFanDemandHandle = pMsg->mPickupActorHandle;


					for(S32 a=0;a<fd_data->mFanDemand.demand_markers.count;a++)			// Send activate message to any associated marker effects
						if (fd_data->mFanDemand.demand_markers[a])
							SendMessageToActor(fd_data->mFanDemand.demand_markers[a]->GetActorHandle(), ActorMsg::E_GameActorMsg_Activate);
					for(S32 a=0;a<fd_data->mFanDemand.gates.count;a++)					// plus any markers that are part of a fan run gate
					{
						if (fd_data->mFanDemand.gates[a].gate_marker_left)
							SendMessageToActor(fd_data->mFanDemand.gates[a].gate_marker_left->GetActorHandle(), ActorMsg::E_GameActorMsg_Activate);
						if (fd_data->mFanDemand.gates[a].gate_marker_right)
							SendMessageToActor(fd_data->mFanDemand.gates[a].gate_marker_right->GetActorHandle(), ActorMsg::E_GameActorMsg_Activate);
					}
					UpdateGateStates();			// Set alpha values for each gate


					ClearContainers();								// Initialise all monitors like max speed etc
					mTimer = 0.f;
					mStartPosition = vehActor->mPositionInfo.mRacePosition;
					mStartHealth = vehActor->GetCurrentHealth();
					mFans = fd_data->mFanDemand.fans;
					mGracePeriod = C_GameData::Instance().mScoringConfig[0]->fan_demand_grace_period;


					Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();		// Grant any pickups given by this Fan Demand
					if (fd_data->mFanDemand.gift_slot1)
					{
						Game::Actors::C_GrantPickupMsg grantPickupMsg(0, fd_data->mFanDemand.slot1, fd_data->mFanDemand.refill_slots);		//Player vehicle, pickup type, refill
						pLayer->GetLogicVehicleActor(0)->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);
					}
					if (fd_data->mFanDemand.gift_slot2)
					{
						Game::Actors::C_GrantPickupMsg grantPickupMsg(0, fd_data->mFanDemand.slot2, fd_data->mFanDemand.refill_slots);		//Player vehicle, pickup type, refill
						pLayer->GetLogicVehicleActor(0)->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);
					}
					if (fd_data->mFanDemand.gift_slot3)
					{
						Game::Actors::C_GrantPickupMsg grantPickupMsg(0, fd_data->mFanDemand.slot3, fd_data->mFanDemand.refill_slots);		//Player vehicle, pickup type, refill
						pLayer->GetLogicVehicleActor(0)->SendMessageToComponent(Components::E_IF_VehicleComponent_Pickups, &grantPickupMsg);
					}


					for(int a=0;a<fd_data->mFanDemand.moves.count;a++)
					{
						mPassedMove.PushBack(false);
						mPassedMoveThisFrame.PushBack(false);
					}

					UI::C_UiFanDemand uimsg(fd_data->mFanDemand.fans,false,1,UI::C_TextStringID(fd_data->mFanDemand.string.id).String(),mVehicleIndex,fd_data->mFanDemand.gold_demand);
					_UI_MESSAGE_FULL( uimsg );


					if (mFans == 0)							// Immediate win if # of fans == 0,  Usually used for chained demands
						EndDemand(true);
				}

				mPickupAquiredTotals[pMsg->mePickupType]++;									// Remember how many of this type of pickup we've collected
				if(pMsg->mePickupType != E_PICKUP_TYPE_FAN && pMsg->mePickupType != E_PICKUP_TYPE_REPAIR)
				{
					mPickupAquiredTotals[E_PICKUP_TYPE_ALL]++;
				}

				if(mPreviousTypePickedUp == pMsg->mePickupType)								// Collected two in a row?
				{
					mPickupDoubleAquiredTotals[mPreviousTypePickedUp]++;
					mPickupDoubleAquiredTotals[E_PICKUP_TYPE_ALL]++;

					mPreviousTypePickedUp=E_PICKUP_TYPE_NONE;
				}
				else
				{
					mPreviousTypePickedUp=pMsg->mePickupType;
				}

				return true;
			}
			break;

		case E_VehicleLogicMsg_HitByPerk:
			{
				const C_VehicleLogicMsg_HitByPerkMsg* pMsg	= Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);

				if( pMsg->mFiringVehicleIndex == mVehicleIndex )
				{
					if(pMsg->mLevel)
					{
						mPickupDoubleHitTotals[pMsg->mPickupType]++;
						mPickupDoubleHitTotals[E_PICKUP_TYPE_ALL]++;
					}
					else
					{
						mPickupHitTotals[pMsg->mPickupType]++;
						mPickupHitTotals[E_PICKUP_TYPE_ALL]++;
					}
				}
				return true;
			}
			break;

		case E_VehicleLogicMsg_FiredPerk:
			{
				const C_VehicleLogicMsg_FiredPerkMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_FiredPerkMsg>(pMessage);

				if(pMsg->mVehicleIndex == mVehicleIndex)
				{
					if(pMsg->mLevel)
					{
						mPickupDoubleUseTotals[pMsg->mPickupType]++;
						mPickupDoubleUseTotals[E_PICKUP_TYPE_ALL]++;
					}
					else
					{
						mPickupUseTotals[pMsg->mPickupType]++;
						mPickupUseTotals[E_PICKUP_TYPE_ALL]++;
					}
				}
				return true;
			}
			break;
	}

	return I_GameActorComponent::OnMessage(pMessage);
}


void C_VehicleComponent_Fan_Demand::UpdateGateStates()
{
	F32		alpha = 1.0f;

	const Actors::FanDemandPositionCreationData *fd_data = ((Actors::FanDemandPositionActor *)GetActorList()->GetActorPtr(mCurrentFanDemandHandle))->GetCreationData();

	for(S32 a=0;a<fd_data->mFanDemand.gates.count;a++)			// Update each gate visability setting
	{
		if (!gDebugDontAlphaFanRunGates)
		{			
			if (a == mCurrentGate)									// Calc each gates alpha value depending on how far through the run we are
				alpha = 1.0f;
			else if (a == (mCurrentGate+1))
				alpha = 0.5f;
			else if (a < mCurrentGate)
				alpha = 0.0f;
			else
				alpha = 0.1f;
		}

		ActorMsg::C_SetAlphaValueMsg	setalphamsg;
		setalphamsg.m_alpha = alpha;

		if (fd_data->mFanDemand.gates[a].gate_marker_left)
			SendMessageToActor(fd_data->mFanDemand.gates[a].gate_marker_left->GetActorHandle(), &setalphamsg);
		if (fd_data->mFanDemand.gates[a].gate_marker_right)
			SendMessageToActor(fd_data->mFanDemand.gates[a].gate_marker_right->GetActorHandle(), &setalphamsg);
	}


}


} // namespace Components
