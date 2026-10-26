#include "stdafx.h"

#include "RLRacePositionController.h"
#include "../Data/RLDefinitions.h"

#include "../Core/RaceLogicCore.h"
#include "../Controllers/RLControllers.h"
#include <algorithm>

#ifdef RLC_DEBUG
#pragma optimize ("", off) //RLC_DEBUG
#endif

static Debug::Logger &g_RLRacePositionLog = Debug::GetLogger("Amax.RLC.RLRacePositionLog");
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern Debug::Bool g_Stamp;

Debug::Float g_RacePositionCalculationFrequency("RLC.RacePositionController.RacePositionCalculationFrequency", 1.f, Debug::Range(0.f, 5.f), Debug::Step(0.1f));
Debug::Bool g_RacePositionDebugSpam("RLC.RacePositionController.PositionDebugSpam", false);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



namespace Game
{
namespace Actors
{
RLRacePositionController::RLRacePositionController()
{
	Initialise();
}

RLRacePositionController::~RLRacePositionController()
{
}

void RLRacePositionController::Initialise()
{
	meType = E_RLCOMPONENTTYPE_RACEPOSITION_CONTROLLER;
	mFirstPosition = -1;
}

void RLRacePositionController::Update()
{
	PERF_EVENT("RLRacePositionController::Update");

	float frameTime = m_pRLC->GetFrameTime();

	//Calculate our race positions
	CalculateRacePositions(frameTime);
}

void RLRacePositionController::CalculateRacePositions(const float frameTime)
{
	PERF_EVENT("RLRacePositionController::CalculateRacePositions");

	C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();
	RLVehicleData* pVehicle = NULL;

	// Sort the vehicles in order of position.
	Btl::Vector< RacePositionBase*, Btl::VectorParameters_Static<MAX_VEHICLES> > positions;
	for(unsigned int i = 0; i < m_pRLC->GetNumVehicles(); i++)
	{
		pVehicle = &m_pRLC->mVehicles[i];

		RacePositionBase* newPos = NULL;

		// What are we sorting by?
		switch(levelState.mLeaderboardType)
		{
		case E_LEADERBOARDTYPE_POSITION:
			newPos = new RacePosition_ByDistance(pVehicle->mVehicleIndex, pVehicle->mDistanceFromStartLine);
			break;

		case E_LEADERBOARDTYPE_LOWDAMAGE:
			//newPos = new RacePosition_ByDamage(pVehicle->mVehicleIndex, m_pRLC->GetVehicleDamagePercentage(pVehicle->mVehicleIndex));
			break;

		default:
			LOGWARN(g_RLRacePositionLog, "RLRacePositionController::CalculateRacePositions() > Unknown leaderboard type - cannot determine player ranking.");
			continue;
		}

		positions.PushBack(newPos);
	}
	std::sort(positions.Begin(), positions.End(), &RacePositionBase::SortBy);

	//We need to keep an eye on where the vehicle really is this frame, but we inform them of a change less frequently
	bool updatePositions = false;
	if( (m_pRLC->GetTimeData()->mRacePositionCalculationTimer - frameTime) <= 0.f )
	{
		updatePositions = true;
		m_pRLC->GetTimeData()->mRacePositionCalculationTimer = g_RacePositionCalculationFrequency;
	}
	else
		m_pRLC->GetTimeData()->mRacePositionCalculationTimer -= frameTime;

	// Update the vehicles with their new positions
	for(int i = 0; i < m_pRLC->mVehicles.Size(); i++)
	{
		pVehicle = &m_pRLC->mVehicles[positions[i]->mVehicleId];
		pVehicle->mLiveRacePosition = i + 1;

		if( updatePositions )
		{
			if( pVehicle->mLiveRacePosition == 1 )
			{
				mFirstPosition = positions[i]->mVehicleId;
			}
				

			pVehicle->mPreviousRacePosition = pVehicle->mRacePosition;
			pVehicle->mRacePosition = pVehicle->mLiveRacePosition;

			if( pVehicle->mRacePosition < pVehicle->mBestRacePosition || pVehicle->mBestRacePosition == -1 )
			{
				pVehicle->mBestRacePosition = pVehicle->mRacePosition;
				pVehicle->mOvertakes++;
			}
		}
	}

	// Inform the world of all the places taken
	if( updatePositions )
	{
		for(int i = 0; i < m_pRLC->mVehicles.Size(); i++)
		{
			pVehicle = &m_pRLC->mVehicles[i];
			S8 placesTaken = pVehicle->mPreviousRacePosition - pVehicle->mRacePosition;
			for( int i = 1; i <= placesTaken; i++ )
			{
				U8 overtakenVehicleIndex = m_pRLC->GetVehicleIndexAtRacePosition(pVehicle->mRacePosition + i);

				if(overtakenVehicleIndex != 255)
				{
					// Inform the world
					C_VehicleOvertakeMsg overtakeMsg( pVehicle->mActorHandle,
						pVehicle->mVehicleIndex, 
						pVehicle->mRacePosition, 
						m_pRLC->mVehicles[overtakenVehicleIndex].mActorHandle, 
						overtakenVehicleIndex);
					m_pRLC->BroadcastMessage(&overtakeMsg);
					C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage(&overtakeMsg);
					m_pRLC->GetActorList()->GetSubscriberSystem().BroadcastMessage(&overtakeMsg);
				}
			}
		}	
	}

	if( g_RacePositionDebugSpam )
	{
		for(int j = 0; j < m_pRLC->mVehicles.Size(); j++)
		{
			pVehicle = &m_pRLC->mVehicles[j];
			LOGINFO(g_RLRacePositionLog, "Vehicle %i is in place %u with %f of the race completed\n", j, pVehicle->mLiveRacePosition, pVehicle->mRacePercentage);
		}

		LOGINFO(g_RLRacePositionLog, "------------------------------------------------------\n");		
	}

	// Can't trust Destroy() to work on pointers, so I'll kill them myself.
	for( int i = 0; i < positions.Size(); i++ )
		delete(positions[i]);

	positions.Destroy(); // Just making sure any internals of Vector are killed - might not be needed.
}


};//Actors
};//Game

#ifdef RLC_DEBUG
#pragma optimize ("", on) //RLC_DEBUG
#endif
