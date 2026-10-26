#include "stdafx.h"

#include "RLScoringController.h"
#include "../Data/RLDefinitions.h"
#include "../Core/RaceLogicCore.h"
#include "../Database/RLDatabase.h"

#include <Game/Game.h>

#include "GameLogic/Messaging/GameLogicMessages.h"

#include <Vehicle/VehicleModel.h>

#include <ui/util/WideString.h>

#include <GameLogic/Layers/AmaxWorldLayer.h>
#include "GameLogic/Layers/RaceBookLayer.h"
#include <Game/GameWorld/GameWorld.h>

#ifdef RLC_DEBUG
#pragma optimize ("", off) //RLC_DEBUG
#endif

static Debug::Logger &g_RLScoringLog = Debug::GetLogger("Amax.RLC.RLScoringLog");
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern Debug::Bool g_Stamp;

Debug::Bool g_OutputDebugScoreStuff("RLC.Scoring.OutputDebugScoreStuff", false);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////



namespace Game
{
namespace Actors
{
RLScoringController::RLScoringController()
{
	Initialise();
}

RLScoringController::~RLScoringController()
{
}

void RLScoringController::Initialise()
{
	meType = E_RLCOMPONENTTYPE_SCORING_CONTROLLER;
}


const bool RLScoringController::ValidateDatabase()
{
	//[ChadW] Optim
	//return true;

	bool valid = true;
	
	if(m_pRLC->GetDatabase()->mRLSystems.Size() < 1)
		valid = false;
	else if(!m_pRLC->GetDatabase()->mRLSystems[0]->mScoringSystem)
		valid = false;
	else if(!m_pRLC->GetDatabase()->mRLSystems[0]->mScoringSystem->mParameters)
		valid = false;

	if(m_pRLC->GetTrackData()->mWarnAboutMissingScoringData)
	{
		/*
		//CA - TOOK THIS OUT AS ITS WINDING ME UP!
		LOGERROR(g_RLScoringLog, "RLScoringController::ValidateDatabase() > *************************************************");
		LOGERROR(g_RLScoringLog, "RLScoringController::ValidateDatabase() > ** SCORING DISABLED AS SORCERY DATA IS MISSING **");
		LOGERROR(g_RLScoringLog, "RLScoringController::ValidateDatabase() > *************************************************");
		*/
		m_pRLC->GetTrackData()->mWarnAboutMissingScoringData = false;
	}

	
	return valid;
}

void RLScoringController::PreUpdate()
{
	PERF_EVENT("RLScoringController::PreUpdate");

	if(!ValidateDatabase())
		return;

	//Go through each vehicle and fill in the bits of data we need for scoring things
	for(int vehicleIndex = 0; vehicleIndex < m_pRLC->mVehicles.Size(); vehicleIndex++)
		PreUpdateVehicle(vehicleIndex);
}

//Work out all the things we score against, don't do actual processing here...we're just gathering data
void RLScoringController::PreUpdateVehicle(const U8 vehicleIndex)
{
	//[ChadW] - Temp optimisations - only update scoring controller if vehicle is local player controlled
	if (C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[vehicleIndex].mControlType != E_VEHICLE_CONTROL_TYPE_PLAYER)
		return;
			
	PERF_EVENT("RLScoringController::PreUpdateVehicle");

	//The model input will tell us what the vehicle is doing in a thread safe way (hopefully!)
	const Vehicle::C_ModelInput *pModelInput = m_pRLC->GetVehicleModelInput(vehicleIndex);

	RLVehicleData* pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	RLSystem* pSystem = m_pRLC->GetDatabase()->mRLSystems[0];
	RLScoringParameters* pScoringParams = pSystem->mScoringSystem->mParameters;

	//Work out if all four wheels are on the ground
	float totalSlipAmount = 0.f;
	int wheelsOnGround = 0;
	for(int iWheel = 0; iWheel < pModelInput->m_Wheels.Size(); iWheel++)
	{
		//Build up the total slip amount for the car...basically if the driven wheels are not gripping we might be in a drift
		totalSlipAmount += abs(pModelInput->m_Wheels[iWheel].m_fSlipAngle);

		//If any wheel's on the ground the vehicle's not in the air...
		if(pModelInput->m_Wheels[iWheel].m_bOnGround)
			wheelsOnGround++;
	}

	pVehicle->mWheelsOnGround = wheelsOnGround;
	if(pModelInput->m_Wheels.Size() == wheelsOnGround)
		pVehicle->mAllWheelsOnGround = true;
	else 
		pVehicle->mAllWheelsOnGround = false;
	
	//If the vehicle's still in the air, add the distance we've flown since the last frame
	//N/B if we've just landed we don't want to reset the air distance here as we might score some
	//cash/xp/kudos or whatever...we'll process that in UpdateVehicle()
	if(wheelsOnGround == 0)
	{
		pVehicle->mAirDistance += pVehicle->mDistanceTravelledSinceLastUpdate;
		pVehicle->mInAir = true;
	}
	else
		pVehicle->mInAir = false;

	//Look at whether we're drifting or not
	pVehicle->mTotalSlipAmount = totalSlipAmount;
	
	float driftAngle = Maths::DotProduct(pVehicle->mVelocityDirection, pVehicle->mDirection);
	pVehicle->mDriftAngleInDegrees = (float)Maths::ArcCos(driftAngle) * (180/Maths::Pi);

	//See if we're drifting
	pVehicle->mDrifting = false;
	if(pVehicle->mDriftAngleInDegrees >= pScoringParams->mDriftAngle.mMin && pVehicle->mDriftAngleInDegrees <= pScoringParams->mDriftAngle.mMax)
	{
		if(pVehicle->mTotalSlipAmount >=  pScoringParams->mSlipAmount.mMin && pVehicle->mTotalSlipAmount <= pScoringParams->mSlipAmount.mMax)
		{
			pVehicle->mDrifting = true;
			pVehicle->mDriftDistance += pVehicle->mDistanceTravelledSinceLastUpdate;
		}
	}

}

void RLScoringController::Update()
{
	PERF_EVENT("RLScoringController::Update");

	if(!ValidateDatabase())
		return;

	//This function is probably going to need to updated late in the update process
	for(U8 vehicleIndex = 0; vehicleIndex < m_pRLC->mVehicles.Size(); vehicleIndex++)
		UpdateVehicle(vehicleIndex);
}

void RLScoringController::UpdateVehicle(const U8 vehicleIndex)
{
	//[ChadW] - Temp optimisations - only update scoring controller if vehicle is local player controlled
	if (C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[vehicleIndex].mControlType != E_VEHICLE_CONTROL_TYPE_PLAYER)
		return;

	PERF_EVENT("RLScoringController::UpdateVehicle");

	RLScoringSystem *pScoringSystem = RLDatabase::Instance().mRLSystems[0]->mScoringSystem;

	//Go through each scoring type
	RLScoringTypeArray *pScoringTypeArray = pScoringSystem->mScoringTypeArray;
	for(int scoringTypeIndex = 0; scoringTypeIndex < pScoringTypeArray->mTypes.count; scoringTypeIndex++)
	{
		//Each scoring type is analogous to the Kudos type, it determines one single way 
		//of scoring (kudos). This time we use rules to validate when a particular score 
		//type is earned
		RLScoringType scoringType = pScoringSystem->mScoringTypeArray->mTypes[scoringTypeIndex];
		
		//Now go through each rule for this scoring type, some score types don't have rules and therefore don't need updating each frame
		bool score = false;
		for(int ruleIndex = 0; ruleIndex < scoringType.mRules.count; ruleIndex++)
		{
			//This score type has at least one rule
			if(ruleIndex == 0)
				score = true;

			//If we fail any rule we need to stop and move onto the next rule
			score = EvaluateRule(	vehicleIndex,
									scoringType.mRules[ruleIndex], 
									scoringType.mRules[ruleIndex]->mNot, 
									scoringType.mRules[ruleIndex]->mOutputDebugInfo,
									scoringType.mRules[ruleIndex]->mConditionalDisplay,
									scoringType.mRules[ruleIndex]->mDisplayWhenTrue);
			if(!score) //FAIL
				break;
		}

		if(score)
			AddScoringEvent(vehicleIndex, scoringType);
	}
}

void RLScoringController::PostUpdate()
{
	PERF_EVENT("RLScoringController::PostUpdate");

	//Go through each vehicle and fill in the bits of data we need for scoring things
	for(int vehicleIndex = 0; vehicleIndex < m_pRLC->mVehicles.Size(); vehicleIndex++)
		PostUpdateVehicle(vehicleIndex);
}

void RLScoringController::PostUpdateVehicle(const U8 vehicleIndex)
{
	//[ChadW] - Temp optimisations - only update scoring controller if vehicle is local player controlled
	if (C_GameApp::GetInstance()->GetGame()->GetLevelState().mCars[vehicleIndex].mControlType != E_VEHICLE_CONTROL_TYPE_PLAYER)
		return;
			
	PERF_EVENT("RLScoringController::PostUpdateVehicle");

	//We use this function to clean up 
	RLVehicleData* pVehicle = &m_pRLC->mVehicles[vehicleIndex];
	if(pVehicle->mWheelsOnGround > 0)
		//Back on the ground again
		pVehicle->mAirDistance = 0.f;

	if(!pVehicle->mDrifting)
		//Finished our drift
		pVehicle->mDriftDistance = 0.f;

}

const bool RLScoringController::EvaluateRule(	const U8 vehicleIndex,
												const RLScoringRuleType& rule, 
												const bool invertRule, 
												const bool outputDebugInfo, 
												const bool conditionalDisplay, 
												const bool displayWhenTrue) const
{
	bool rulePassed = false;

	UI::C_WideString32 ruleName;
	const RLVehicleData& vehicleData = m_pRLC->mVehicles[vehicleIndex];
	const RLVehicleData& lastVehicleData = m_pRLC->mLastVehicles[vehicleIndex];
	switch(rule.meRule)
	{
	case E_SCORE_RULE_WHEELS_OFF_GROUND:
		{
			//See if the required number of wheels are off the ground
			rulePassed = true;
			ruleName = L"WheelsOffGround";
			for(int valueIndex = 0; valueIndex < rule.mValues.count; valueIndex++)
			{
				if(!EvaluateValue(	rule.mValues[valueIndex], 
									(float)vehicleData.mWheelsOnGround, 
									rule.mValues[valueIndex].mOutputDebugInfo, 
									rule.mValues[valueIndex].mConditionalDisplay,
									rule.mValues[valueIndex].mDisplayWhenTrue) )
				{
					rulePassed = false;
					break;
				}
			}
			break;
		}
	case E_SCORE_RULE_WHEELS_ON_GROUND:
		{
			//See if the required number of wheels are on the ground
			rulePassed = true;
			ruleName = L"WheelsOnGround";
			for(int valueIndex = 0; valueIndex < rule.mValues.count; valueIndex++)
			{
				if(!EvaluateValue(	rule.mValues[valueIndex], 
									(float)vehicleData.mWheelsOnGround, 
									rule.mValues[valueIndex].mOutputDebugInfo, 
									rule.mValues[valueIndex].mConditionalDisplay,
									rule.mValues[valueIndex].mDisplayWhenTrue) )
				{
					rulePassed = false;
					break;
				}
			}			
			break;
		}

	case E_SCORE_RULE_AIR_DISTANCE_TRAVELLED:
		{
			//See if the air distance is valid
			rulePassed = true;
			ruleName = L"AirDistance";
			for(int valueIndex = 0; valueIndex < rule.mValues.count; valueIndex++)
			{
				if(!EvaluateValue(	rule.mValues[valueIndex], 
									vehicleData.mAirDistance, 
									rule.mValues[valueIndex].mOutputDebugInfo, 
									rule.mValues[valueIndex].mConditionalDisplay,
									rule.mValues[valueIndex].mDisplayWhenTrue))
				{
					rulePassed = false;
					break;
				}
			}			
			break;
		}
	case E_SCORE_RULE_DRIFT_DISTANCE_TRAVELLED:
		{
			//See if we've drifted far enough
			rulePassed = true;	
			ruleName = L"DriftDistance";
			for(int valueIndex = 0; valueIndex < rule.mValues.count; valueIndex++)
			{
				if(!EvaluateValue(	rule.mValues[valueIndex], 
									vehicleData.mDriftDistance, 
									rule.mValues[valueIndex].mOutputDebugInfo, 
									rule.mValues[valueIndex].mConditionalDisplay,
									rule.mValues[valueIndex].mDisplayWhenTrue))
				{
					rulePassed = false;
					break;
				}
			}
			break;
		}
	case E_SCORE_RULE_BEST_RACE_POSITION:
		{
			//Make sure we have initialised our vehicle for at least one frame (hopefully prevents a
			//bug where you start scoring even just at the start of the race)
			rulePassed = false;
			ruleName = L"BestRacePosition";
			if( vehicleData.mBestRacePosition != INVALID_RACE_POSITION 
				&& 
				lastVehicleData.mBestRacePosition != INVALID_RACE_POSITION)
			{
				if(vehicleData.mBestRacePosition > lastVehicleData.mBestRacePosition)
					rulePassed = true;
			}
			break;
		}
		
	case E_SCORE_RULE_SPEED_IN_MPH:
		{
			//See if we're going fast enough]
			rulePassed = true;
			ruleName = L"MPH";
			for(int valueIndex = 0; valueIndex < rule.mValues.count; valueIndex++)
			{
				if(!EvaluateValue(	rule.mValues[valueIndex], 
									vehicleData.mSpeedInMilesPerHour, 
									rule.mValues[valueIndex].mOutputDebugInfo, 
									rule.mValues[valueIndex].mConditionalDisplay,
									rule.mValues[valueIndex].mDisplayWhenTrue))
				{
					rulePassed = false;
					break;
				}
			}
			break;
		}
	case E_SCORE_RULE_DRIFTING:
		{
			//See if we're going fast enough
			ruleName = L"Drifting";
			rulePassed = vehicleData.mDrifting;
			break;
		}
	/*
	case E_SCORE_RULE_LIGHT_INTRAVEHICULAR_IMPACT:
		{
			ruleName = L"LightIntraVehicularImpact";
			if(vehicleData.mLastDamageEvent.meImpactLevel == E_IMPACT_LEVEL_LIGHT)
				rulePassed = true;
			else
				rulePassed = false;
			break;
		}
	case E_SCORE_RULE_MEDIUM_INTRAVEHICULAR_IMPACT:
		{
			ruleName = L"MediumIntraVehicularImpact";
			if(vehicleData.mLastDamageEvent.meImpactLevel == E_IMPACT_LEVEL_MEDIUM)
				rulePassed = true;
			else
				rulePassed = false;
			break;
		}
	case E_SCORE_RULE_HEAVY_INTRAVEHICULAR_IMPACT:
		{
			ruleName = L"HeavyIntraVehicularImpact";
			if(vehicleData.mLastDamageEvent.meImpactLevel == E_IMPACT_LEVEL_HEAVY)
				rulePassed = true;
			else
				rulePassed = false;
			break;
		}
	*/
	}

	//Allows us to negate a rule simply
	if(invertRule)
	{
		rulePassed = !rulePassed;
		ruleName += L"!NOT!";
	}

	if(outputDebugInfo)
	{
		UI::C_WideString256 output;
		output.Format(L"RLScoringController::EvaluateRule() > Rule %s = ", ruleName.Chars());
		if(rulePassed)
			output += UI::C_WideString6("TRUE");
		else
			output += UI::C_WideString6("FALSE");
		
		if(conditionalDisplay)
		{
			 if(displayWhenTrue == rulePassed)
				LOGDEBUG(g_RLScoringLog, output.AsString());
		}
		else
			LOGDEBUG(g_RLScoringLog, output.AsString());
	}
	
	return rulePassed;
}

const bool RLScoringController::EvaluateValue(	const RLScoringRuleValue& ruleValue, 
												const float valueToBeEvaluated, 
												const bool outputDebugInfo, 
												const bool conditionalDisplay, 
												const bool displayWhenTrue) const
{
	bool validValue = false;
	UI::C_WideString32  valueName;

	switch(ruleValue.meType)
	{
	case E_SCORE_RULE_VALUE_TYPE_GREATER_THAN:
		{
			valueName = L"GreaterThan";
			if(valueToBeEvaluated > ruleValue.mValue)
				validValue = true;
			break;
		}

	case E_SCORE_RULE_VALUE_TYPE_GREATER_THAN_OR_EQUAL_TO:
		{
			valueName = L"GreaterThanOrEqualTo";
			if(valueToBeEvaluated >= ruleValue.mValue)
				validValue = true;
			break;
		}

	case E_SCORE_RULE_VALUE_TYPE_LESS_THAN:
		{
			valueName = L"LessThan";
			if(valueToBeEvaluated < ruleValue.mValue)
				validValue = true;
			break;
		}

	case E_SCORE_RULE_VALUE_TYPE_LESS_THAN_OR_EQUAL_TO:
		{
			valueName = L"LessThanOrEqualTo";
			if(valueToBeEvaluated <= ruleValue.mValue)
				validValue = true;
			break;
		}

	case E_SCORE_RULE_VALUE_TYPE_EQUAL_TO:
		{
			valueName = L"EqualTo";
			if(valueToBeEvaluated == ruleValue.mValue)
				validValue = true;
			break;
		}

	case E_SCORE_RULE_VALUE_TYPE_NOT_EQUAL_TO:
		{
			valueName = L"NotEqualTo";
			if(valueToBeEvaluated != ruleValue.mValue)
				validValue = true;
			break;
		}
	}

	if(outputDebugInfo)
	{	
		UI::C_WideString256 output;
		output.Format(L"RLScoringController::EvaluateValue() > Value %3.02f %s %3.02f = ", valueToBeEvaluated, valueName.Chars(), ruleValue.mValue);
		if(validValue)
			output += L"TRUE";
		else
			output += L"FALSE";

		if(conditionalDisplay)
		{
			if(displayWhenTrue == validValue)
				LOGDEBUG(g_RLScoringLog, output.AsString());
		}
		else
			LOGDEBUG(g_RLScoringLog, output.AsString());
	}

	return validValue;
}

void RLScoringController::AddScoringEvent(const U8 vehicleIndex, const RLScoringType& scoringType, const float multiplier, const UI::C_WideString64 overrideScoreTypeName)
{
	if( !m_pRLC->VehicleValid(vehicleIndex) )
		return;
	else if(m_pRLC->mVehicles[vehicleIndex].mExcludeFromRaceLogic)
		return;

	if(overrideScoreTypeName.Length() == 0)
	{
		C_ScoringEventMsg scoringEventMsg(vehicleIndex, scoringType.mScoreValue, UI::C_TextStringID( scoringType.mLoc_string.id).String());
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&scoringEventMsg);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&scoringEventMsg);
	}
	else
	{
		C_ScoringEventMsg scoringEventMsg(vehicleIndex, scoringType.mScoreValue, overrideScoreTypeName);
		C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&scoringEventMsg);
		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem().BroadcastMessage(&scoringEventMsg);
	}
	
	//Apply any XP multiplier
	int scoreToAdd = scoringType.mScoreValue * m_pRLC->mVehicles[vehicleIndex].mPerformanceModifiers.mXPMultiplier * multiplier;


	if(g_OutputDebugScoreStuff)
	{
		UI::C_WideString128 scoreMessage;
		if(overrideScoreTypeName.Length() == 0)
		{
			scoreMessage.Format(L"VID:%d %s = %d", (int)vehicleIndex, UI::C_TextStringID( scoringType.mLoc_string.id).String(), scoreToAdd);
		}
		else 
		{
			//Use the overrided version
			scoreMessage.Format(L"VID:%d %s = %d", (int)vehicleIndex, overrideScoreTypeName.Chars(), scoreToAdd);
		}


		LOGDEBUG(g_RLScoringLog, scoreMessage.AsString());
	}



	//DEBUG DEBUG DEBUG
	//TODO Add this to a stash and to a score store
	//Add to our score
	m_pRLC->mVehicles[vehicleIndex].mScore += scoreToAdd;
	//DEBUG DEBUG DEBUG
}



};//Actors
};//Game

#ifdef RLC_DEBUG
#pragma optimize ("", on) //RLC_DEBUG
#endif
