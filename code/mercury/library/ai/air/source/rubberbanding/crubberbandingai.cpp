#include "stdafx.h"

#include <rubberbanding/crubberbandingai.h>
#include <data/idata.h>
#include <gameservertime/igameservertime.h>
#include <system/csystemai.h>
#include <racedirector/iracedirector.h>
#include <fuzzylib/fuzzylibone/fuzzyone.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

#define OLD_FIRST_PLAYABLE_SYSTEM 0

namespace airacingtech
{
	using namespace fuzzy_lib_two;
	using namespace fuzzy_lib_one;

	Debug::Logger& g_LogSegmentTracker = Debug::GetLogger( "air.segmenttracker" );


	CRubberbandingAI::CRubberbandingAI() : m_fzCatchup(0.0f), m_pISystem(0), m_pIData(0), m_pIGameServerTime(0), m_lastTime(0.0f),  m_pParameters(0)
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pISystem = 0;
		m_pIRaceDirector = 0;
		m_pGameplayBalancing = 0;
		m_pAIEventBalancing = 0;
		m_DirectTargetSpeed = -1.0f;
		m_pAIEventBalancingData = 0;
		m_pAIEventBalancingSegmentData= 0;
		m_driverIndexOverride = -1;
	}

	CRubberbandingAI::~CRubberbandingAI()
	{

	}

	void CRubberbandingAI::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_RUBBERBANDING_AI, CID_RUBBERBANDING_AI);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_RUBBERBANDING_AI, CRubberbandingAI::CreateMe, CRubberbandingAI::DestroyMe, CHash("RubberbandingAI"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RUBBERBANDING_AI, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RUBBERBANDING_AI, MT_QUICK_RESTART);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RUBBERBANDING_AI, MT_ADD_AI_DRIVER_TO_LIST);
		
	}

	IComponent *CRubberbandingAI::CreateMe()
	{
		return new CRubberbandingAI;
	}

	bool CRubberbandingAI::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CRubberbandingAI::Init(CObjectId oid, CParameterNode &paramNode)
	{
		RubberBandingInitialiseParams *pParams = static_cast<RubberBandingInitialiseParams *>( paramNode.GetData() );
		if (pParams)
		{
			m_pParameters = pParams;	
		}
		return true;
	}

	void CRubberbandingAI::Deinit(void)
	{
		if (m_pParameters)
			delete m_pParameters;
	}

	EMessageResult CRubberbandingAI::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch (msg.mType)
		{
		case MT_POST_INIT_AI_SYSTEM:
			{	
				msgRet =  MR_TRUE;
				CachedInterfaces();
			}
			break;

		case MT_ADD_AI_DRIVER_TO_LIST:
			{
				msgRet =  MR_TRUE;

				CObjectId *pParams = static_cast<CObjectId *>(msg.mpData);
				CObjectId id = *pParams;

				// do not add ourselves....
				CObjectId Myid = GetObjectId();
				//if (Myid != id)
				{
					IData *pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));
					m_AIlist.PushBack(pIData);
				}

			}
			break;


		default:
			break;
		}

		return msgRet;
	}


	EComponentTypeId CRubberbandingAI::GetComponentTypeId(void)
	{
		return CID_RUBBERBANDING_AI;
	}


	void CRubberbandingAI::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));
	
		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));
	
		CObjectId systemID("AmaxSystemAI");
		m_pISystem = static_cast<ISystemAI *>(GlobalsComponent::GetObjectManager().QueryInterface(systemID, IID_SYSTEM_AI));

		m_pIRaceDirector = static_cast<IRaceDirector *>(GlobalsComponent::GetObjectManager().QueryInterface(CObjectId("RaceDirectorSystem"), IID_RACEDIRECTOR));
		
	
		m_pAIEventBalancing = m_pIRaceDirector->GetEventBalancing();
	}


	void CRubberbandingAI::GetNearestBehindInfront(int &infront, int &behind, bool &infrontOfPlayer)
	{
		infrontOfPlayer = false;
		behind = -1;
		infront = -1;

		ISplineTracker	*pSplineTrackerThis = m_pIData->GetSplineTracker();
		float maxDist = -5000.0f;
		float minDist = 5000.0f;

		for(int i=0; i < (int)m_AIlist.Size(); i++)
		{
			IData *pIDataBehind = m_AIlist[i];
			if (pIDataBehind != m_pIData) // do not check against ourselves....
			{
				ISplineTracker	*pSplineTrackerBehind = pIDataBehind->GetSplineTracker();
				if (pSplineTrackerBehind && pSplineTrackerThis)
				{
					const RouteInfo *pRouteInfoBehind = pSplineTrackerBehind->GetLogicRoute();
					const RouteInfo *pRouteInfoThis = pSplineTrackerThis->GetLogicRoute();
					if (pRouteInfoBehind && pRouteInfoThis)
					{
						float pos = airacingtech::DistanceInfrontBehindF(pRouteInfoThis->m_pos, pRouteInfoBehind->m_pos);
						float dist = pos * pRouteInfoThis->m_pRoute->GetRouteLength();
						// -ve dist this is behind, +ve dist this is infront

						// don`t forget it is the distance this vehicle is in relation to others, therefore infront and behind are swapped
						if (dist < 0.0f)
						{
							if (dist > maxDist)
							{
								maxDist = dist;
								infront = i;
							}
						}
						else
						{
							if (dist < minDist)
							{
								minDist = dist;
								behind = i;
							}
						}

						if (i==0)
						{
							// check agains the player
							if (dist < 0.0f)
								infrontOfPlayer = false;
							else
								infrontOfPlayer = true;

						}

					}
				}
			}
		}
	}



void CRubberbandingAI::Update()
{

	m_pAIEventBalancingData = 0;
	m_pAIEventBalancingSegmentData = 0;

	if (m_pIGameServerTime && m_pIData && m_pISystem && m_pIRaceDirector && m_pAIEventBalancing && m_pAIEventBalancing->m_Data.Size() > 1)
	{
		if (m_pIGameServerTime->GetTime() > m_lastTime)
		{
			m_fzCatchup = 0.0f;
			m_fzSlowdown = 0.0f;
			m_DirectTargetSpeed = -1.0f;

			// are we on our last lap ?????
			int currentLap = m_pIData->GetVehicleLogic()->GetLogicState()->GetLap();
			int maxLap = m_pIData->GetVehicleLogic()->GetLogicState()->GetRaceLaps();
			float difficulty = m_pIData->GetParams()->GetDriverData()->m_difficulty;
			int currentRank = m_pIData->GetVehicleLogic()->GetLogicState()->GetRank();
			float racePercentage = m_pIData->GetVehicleLogic()->GetLogicState()->GetRacePercentage();
			int noSegments = 0;
			int driverIndex = m_pIData->GetState().m_index;
			if( m_driverIndexOverride != -1 )
			{
				driverIndex = m_driverIndexOverride;
			}
			AIEventBalancingData *pBalancingData = 0;
			

			m_pIData->GetState().m_fireNitro = false;

			if (driverIndex >=0 && driverIndex < m_pAIEventBalancing->m_Data.Size())
			{
				pBalancingData = m_pAIEventBalancing->m_Data[driverIndex];	
				assert(pBalancingData);
				m_pAIEventBalancingData = pBalancingData;
				noSegments = pBalancingData->m_SegmentData.Size();


				// Nitro usage condition, look along the spline for the min speed
				bool nitroTrackSpeedCheck = false;
				ISplineTracker	*pSplineTrackerThis = m_pIData->GetSplineTracker();
				const RouteInfo *pRouteInfoThisCurrent = pSplineTrackerThis->GetCurrentRoute();
				int carNO = m_pIData->GetState().m_index;
				if (pRouteInfoThisCurrent)
				{
					const int currentLane = AITrackDataLines::TRACK_DATA_LINE_RACING;
					float currentSpeed = m_pIData->GetVehicleState()->GetState()->GetSpeed();
					int lookahead = static_cast<int>(currentSpeed);
					lookahead = Maths::Min(lookahead, 80);
					float lookaheadSpeed = static_cast<float>(pRouteInfoThisCurrent->m_pRoute->GetSpeedMin(pRouteInfoThisCurrent->m_index, lookahead, currentLane, carNO));
					lookaheadSpeed -= 10.0f;

					if (lookaheadSpeed >= currentSpeed)
					{
						nitroTrackSpeedCheck = true;
					}
				}
				



				// Fire nitro if in posession.....
				//if (m_pIData->GetState().m_hasNitro)
				{
					IData *pIDataHumanPlayer = m_AIlist[0]; // Darren.. very bad this, but hey look at the current game 
					int rankHuman = 0;
					if (pIDataHumanPlayer)
					{
						rankHuman = pIDataHumanPlayer->GetVehicleLogic()->GetLogicState()->GetRank();
					}

					if (currentRank >= rankHuman)
					{
						// always use when behind........

						float angle = 0.0f;
						ISplineFollower	*pSplineFollowerThis = m_pIData->GetSplineFollower();
						if (pSplineFollowerThis)
						{
							angle = Maths::Abs(pSplineFollowerThis->GetTrackAngle());
							if (angle < 0.2f && nitroTrackSpeedCheck)
							{
								m_pIData->GetState().m_fireNitro = true;
							}
						}

					}
					else
					{
						// we are infront, limit it to a distance...
						if (pIDataHumanPlayer != m_pIData) // not equal to ourselves
						{
							ISplineTracker	*pSplineTrackerThis = m_pIData->GetSplineTracker();
							ISplineTracker	*pSplineTrackerTheirs = pIDataHumanPlayer->GetSplineTracker();
							if (pSplineTrackerThis && pSplineTrackerTheirs)
							{
								const RouteInfo *pRouteInfoThis = pSplineTrackerThis->GetLogicRoute();
								const RouteInfo *pRouteInfoTheirs = pSplineTrackerTheirs->GetLogicRoute();
								if (pRouteInfoThis && pRouteInfoTheirs && pRouteInfoThis->m_in && pRouteInfoTheirs->m_in)
								{
									float pos = airacingtech::DistanceInfrontBehindF(pRouteInfoThis->m_pos, pRouteInfoTheirs->m_pos);
									float dist = pos * pRouteInfoThis->m_pRoute->GetRouteLength();

									float rndDist = RandomInRange(-5.0f, 15.0f); 

									if (Maths::Abs(dist) < (12.0f + rndDist ))
									{
										float angle = 0.0f;
										ISplineFollower	*pSplineFollowerThis = m_pIData->GetSplineFollower();
										if (pSplineFollowerThis)
										{
											angle = Maths::Abs(pSplineFollowerThis->GetTrackAngle());
											if (angle < 0.3f && nitroTrackSpeedCheck)
											{
												m_pIData->GetState().m_fireNitro = true;	
											}
										}
									}
								}
							}
						}
					}
				}

				if (noSegments > 0 && pBalancingData )
				{
					float invSegmentSize = 1.0f / noSegments;
					int segmentIndex =  static_cast<int>(racePercentage * noSegments);
					segmentIndex = Maths::Clamp(segmentIndex, 0, noSegments-1);
					const AIEventSegmentData *pGameplaySegment = &pBalancingData->m_SegmentData[segmentIndex];
					assert(pGameplaySegment);

					m_pAIEventBalancingSegmentData = &pBalancingData->m_SegmentData[segmentIndex];



					if (!m_pIData->GetVehicleState()->GetState()->GetPlayer())
					{
						int sze = m_pISystem->GetSize();
						int rubberBandTo = Maths::Clamp(pGameplaySegment->m_To, 0 , sze-1);
						IData *pIDataTheirs = m_AIlist[rubberBandTo];	// potentially could be invalid number....
						assert(pIDataTheirs);

						bool playerNotAccel = false;

						if (pIDataTheirs && m_pIData && pIDataTheirs != m_pIData && pGameplaySegment->m_Enable)
						{	
							ISplineTracker	*pSplineTrackerThis = m_pIData->GetSplineTracker();
							ISplineTracker	*pSplineTrackerTheirs = pIDataTheirs->GetSplineTracker();

							if (pSplineTrackerThis && pSplineTrackerTheirs)
							{
								const RouteInfo *pRouteInfoThis = pSplineTrackerThis->GetLogicRoute();
								const RouteInfo *pRouteInfoTheirs = pSplineTrackerTheirs->GetLogicRoute();
								if (pRouteInfoThis && pRouteInfoTheirs && pRouteInfoThis->m_in && pRouteInfoTheirs->m_in)
								{
									float pos = airacingtech::DistanceInfrontBehindF(pRouteInfoThis->m_pos, pRouteInfoTheirs->m_pos);
									float dist = pos * pRouteInfoThis->m_pRoute->GetRouteLength();
									// -ve dist this is behind, +ve dist this is infront


									float distABS = Maths::Abs(dist);
									float speed = pIDataTheirs->GetVehicleState()->GetState()->GetSpeed();
									float speedThis = m_pIData->GetVehicleState()->GetState()->GetSpeed();

									float trackspeed = m_pIData->GetState().m_trackSpeedPreprocessed * pGameplaySegment->m_TrackSpeedPercentageMax;			
									float actualTrackPercenatge = ((speedThis / trackspeed)*100.0f);

									if (dist < pGameplaySegment->m_Distance)
									{
										// speed up. to achieve target......
										float targetDistance  = Maths::Abs(pGameplaySegment->m_Distance - dist);					

										float fzDist = FuzzyRightShoulder(targetDistance, 1.0f, 40.0f);

										float catchupRate = 1.0f + (fzDist * 4.0f);
										float targetSpeed = speed + catchupRate;
										targetSpeed = Maths::Min(trackspeed, targetSpeed);
										targetSpeed = Maths::Max(0.0f, targetSpeed);
										m_DirectTargetSpeed = targetSpeed;

										//if (speedThis >= targetSpeed)
										//{
										//	m_DirectTargetSpeed = -1.0f;
										//}

									}
									else
									{
										// slow down to achieve target......
										float targetDistance  = Maths::Abs(dist - pGameplaySegment->m_Distance);

										float fzDist = FuzzyRightShoulder(targetDistance, 1.0f, 40.0f);

										float slowDownRate = 1.0f + (fzDist * 4.0f); 


										// lets get the human player......
										IData *pIDataHumanPlayer = m_AIlist[0]; // Darren : Very bad
										if (pIDataHumanPlayer)
										{
											ISplineTracker	*pSplineTrackerHuman = pIDataHumanPlayer->GetSplineTracker();
											if (pSplineTrackerHuman)
											{
												const RouteInfo *pRouteInfoHuman = pSplineTrackerHuman->GetLogicRoute();
												if (pRouteInfoHuman)
												{
													float pos = airacingtech::DistanceInfrontBehindF(pRouteInfoThis->m_pos, pRouteInfoHuman->m_pos);
													float dist = pos * pRouteInfoThis->m_pRoute->GetRouteLength();
													// -ve dist this is behind, +ve dist this is infront

													if (Maths::Abs(dist) < 15.0f)
													{

														if (pIDataHumanPlayer->GetControlState()->GetState()->GetAccelControllerInput() < 0.1f || pIDataHumanPlayer->GetControlState()->GetState()->GetBrakeControllerInput() > 0.5f || pIDataHumanPlayer->GetControlState()->GetState()->GetHandbrakeControllerInput())
														{
															playerNotAccel = true;
														}
														else
														{
															playerNotAccel = false;
														}
													}
												}
											}
										}


										float targetSpeed = speed - slowDownRate;
										targetSpeed = Maths::Min(trackspeed, targetSpeed);
										targetSpeed = Maths::Max(0.0f, targetSpeed);
										m_DirectTargetSpeed = targetSpeed;

										//if (speedThis <= targetSpeed)
										//{
										//	m_DirectTargetSpeed = -1.0f;
										//}

									}

									bool someOneUsingNitro = false;
									if (pIDataTheirs->GetState().m_usingNitro || m_pIData->GetState().m_usingNitro)
									{
										someOneUsingNitro = true;
									}

									float angle = 0.0f;
									ISplineFollower	*pSplineFollowerThis = m_pIData->GetSplineFollower();
									if (pSplineFollowerThis)
									{
										angle = Maths::Abs(pSplineFollowerThis->GetTrackAngle());
									}

									if (speed < 10.0f || speedThis < 10.0f || actualTrackPercenatge < (pGameplaySegment->m_TrackSpeedPercentageMin*100.0f) || someOneUsingNitro || angle > 0.2f  || playerNotAccel)
									{
										m_DirectTargetSpeed = -1.0f;
									}
								}
							}

						}
					}
					
				}

			}
		
			m_lastTime = m_pIGameServerTime->GetTime();
		}
	}
}


	
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif
