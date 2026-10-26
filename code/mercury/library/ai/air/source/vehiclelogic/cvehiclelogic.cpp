#include "stdafx.h"
#include <vehiclelogic/cvehiclelogic.h>
#include <system/Perf/PerfEvent.h>
#include <fuzzylib/fuzzylibone/fuzzyone.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

// NOTE : Point to point races hacked in a none clean way...this whole logic wants tidying up.....

#ifndef CFG_GOLD
Debug::Bool g_IncrementLapCount("RLC.IncrementLapCount", false);
#endif

#define _DARREN_ADD_IN_POINT_TO_POINT 1

static Debug::Logger &g_LoggerAI = Debug::GetLogger("amax.AI");

namespace airacingtech
{

	using namespace fuzzy_lib_one;

	CVehicleLogic::CVehicleLogic() : m_pTrackData(0), m_pIData(0), m_pSplineTracker(0), m_pVehState(0), m_pIVehicleState(0), m_pIGameServerTime(0)
	{
		m_state.m_InStartZone = false;
		m_state.m_InFinishZone = false;
		m_state.m_InInter1Zone = false;
		m_state.m_InInter2Zone = false;
		m_SendLapIncMessagePointToPointHack = false;
	}


	CVehicleLogic::~CVehicleLogic()
	{

	}

	void CVehicleLogic::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_VEHICLE_LOGIC, CID_VEHICLE_LOGIC);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_VEHICLE_LOGIC, CVehicleLogic::CreateMe, CVehicleLogic::DestroyMe, CHash("VehicleLogic"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_POST_INIT_AI_SYSTEM);

		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_ENTERING_START_ZONE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_ENTERING_FINISH_ZONE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_ENTERING_INTERMEDIATE_1);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_ENTERING_INTERMEDIATE_2);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_ENTERING_LOGIC_ZONE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_INIT_RACE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_QUICK_RESTART);
		
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_LEAVING_START_ZONE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_LEAVING_FINISH_ZONE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_LEAVING_INTERMEDIATE_1);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_LEAVING_INTERMEDIATE_2);

		// Fix for the cut scenes and race initialisation...........
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_LOGIC, MT_START_AI_RACE);
	}

	IComponent *CVehicleLogic::CreateMe()
	{
		return new CVehicleLogic;
	}

	bool CVehicleLogic::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}

	// Virtual IComponent methods
	bool CVehicleLogic::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}


	void CVehicleLogic::Deinit(void)
	{

	}

	EMessageResult CVehicleLogic::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;
		switch (msg.mType)
		{

		case MT_START_AI_RACE:
			{
				InitialiseRace();
			}
			break;

		case MT_QUICK_RESTART:
			{
				m_state.Reset();	
			}
			break;

		case MT_ENTERING_START_ZONE:
			{
				m_state.m_InStartZone = true;
				StartZone();
			}
			break;
	
		case MT_ENTERING_FINISH_ZONE:
			{
				// keeping this the same as the 
				m_state.m_InFinishZone = true;
				FinishZone();
			}
			break;
		
		case MT_ENTERING_INTERMEDIATE_1:
			{
				m_state.m_InInter1Zone = true;

				IntermediateZoneOne();
			}
			break;
		
		case MT_ENTERING_INTERMEDIATE_2:
			{
				m_state.m_InInter2Zone = true;
				IntermediateZoneTwo();
			}
			break;
		
		case MT_ENTERING_LOGIC_ZONE:
			{
				LogicZone();
			}
			break;


		case MT_POST_INIT_AI_SYSTEM:
			{
				msgRet =  MR_TRUE;
				CachedInterfaces();
			}
			break;

		case MT_INIT_RACE:
			{
				InitRace();
			}
			break;


		case MT_LEAVING_START_ZONE:
			{

				ValidateZone();
			}
			break;

		case  MT_LEAVING_FINISH_ZONE:
			{
				ValidateZone();
			}
			break;

		
		case MT_LEAVING_INTERMEDIATE_1:
			{
				ValidateInter1();

			}
			break;


		case MT_LEAVING_INTERMEDIATE_2:
			{
				ValidateInter2();
			}
			break;
				
		default:
			break;
		}

		return msgRet;
	}

	void CVehicleLogic::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

		CObjectId trackdataSystem("TrackDataSystem");
		m_pTrackData = static_cast<ITrackDataAI *>(GlobalsComponent::GetObjectManager().QueryInterface(trackdataSystem, IID_TRACK_DATA_AI));

		m_pIVehicleState = static_cast<IVehicleState *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_VEHICLE_STATE));
		if (m_pIVehicleState)
		{
			m_pVehState = m_pIVehicleState->GetState();
		}

		m_pSplineTracker = static_cast<ISplineTracker *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_SPLINE_TRACKER));


		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

	}


	EComponentTypeId CVehicleLogic::GetComponentTypeId(void)
	{
		return CID_VEHICLE_LOGIC;
	}


	bool CVehicleLogic::DebugValidateZone()
	{
		bool invalidZone = false;

		if (!m_state.m_InStartZone && !m_state.m_InFinishZone && !m_state.m_InInter1Zone && !m_state.m_InInter2Zone)
		{
			
			int zone = CalculateZone(m_state.m_logicPosition);

			if (zone != m_state.m_zone)
			{
				invalidZone = true;	
			}
		}

		return invalidZone;
	}


	void CVehicleLogic::ValidateInter1()
	{
		if (m_pTrackData && m_pSplineTracker && m_state.m_InInter1Zone == true)
		{
/*
			int zone = CalculateZone(m_state.m_logicPosition);
			if (zone == LogicState::FIRST_THIRD)
			{
				m_state.m_zone = LogicState::FIRST_THIRD;
			}
*/



			const RouteInfo *pRoute = m_pSplineTracker->GetLogicInter1();
			if (pRoute)
			{
				Maths::HVec3 pos = m_pVehState->GetPosition();
				int inter1EndIndex = 1;
				
				if (pRoute->m_pRoute->InfrontSegmentBottomPlane(pos, inter1EndIndex))
				{

					m_state.m_zone = LogicState::SECOND_THIRD;
				}
				else
				{
					m_state.m_zone = LogicState::FIRST_THIRD;
				}
			}
		}

		m_state.m_InInter1Zone = false;
	}

	void CVehicleLogic::ValidateInter2()
	{
		if (m_pTrackData && m_pSplineTracker && m_state.m_InInter2Zone == true)
		{
			/*int zone = CalculateZone(m_state.m_logicPosition);
			if (zone == LogicState::SECOND_THIRD)
			{
				m_state.m_zone = LogicState::SECOND_THIRD;
			}*/

			const RouteInfo *pRoute = m_pSplineTracker->GetLogicInter2();
			if (pRoute)
			{
				Maths::HVec3 pos = m_pVehState->GetPosition();
				int inter2EndIndex = 1;//

				if (pRoute->m_pRoute->InfrontSegmentBottomPlane(pos, inter2EndIndex))
				{
					m_state.m_zone = LogicState::FINAL_THIRD;
				}
				else
				{
					m_state.m_zone = LogicState::SECOND_THIRD;
				}
			}
		}

		m_state.m_InInter2Zone = false;
	}

	void CVehicleLogic::ValidateZone()
	{
		if (m_pTrackData && m_pSplineTracker && m_state.m_InStartZone == true)
		{
			AITrackData *pAITrackData = m_pTrackData->GetTrackData();

			const RouteInfo *pLogicRouteInfo = m_pSplineTracker->GetLogicRoute();

			if (pLogicRouteInfo && pAITrackData->m_TrackLogicData.m_PointToPoint == false)
			{

				m_state.m_logicPosition = 0.0f;

				const RouteInfo *pRoute = m_pSplineTracker->GetLogicStart();
				if (pRoute)
				{
					Maths::HVec3 pos = m_pVehState->GetPosition();
					int startEndIndex = Maths::Min(8, pRoute->m_pRoute->GetNumSegments()-1);

					if (pRoute->m_pRoute->InfrontSegmentTopPlane(pos, startEndIndex))
					{
						m_state.m_zone = LogicState::FIRST_THIRD;
					}
					else
					{
						
						int startEndIndex = 1;

						if (pRoute->m_pRoute->InfrontSegmentBottomPlane(pos, startEndIndex))
						{
							m_state.m_zone = LogicState::FIRST_THIRD;
						}
						else
						{
							m_state.m_zone = LogicState::FINAL_THIRD;
						}
					}

					if (m_state.m_zone == LogicState::FINAL_THIRD)
					{
						// we have initially gone through the start zone and then turned around and went backwards around the track, exiting the start zone.	

						m_state.m_zone = LogicState::FINAL_THIRD;
						m_state.m_lap--;
						m_state.m_lap = Maths::Max(m_state.m_lap, 0);
						m_state.m_logicPositionCombined = m_state.m_logicPosition + (float)m_state.m_lap;
					}

				}




/*				m_state.m_logicPosition = (pLogicRouteInfo->m_index - pAITrackData->m_TrackLogicData.m_startSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;

				if (m_state.m_logicPosition < 0.0f)
					m_state.m_logicPosition += 1.0f;

				//add in the paramatised version.
				m_state.m_logicPosition += (pLogicRouteInfo->m_ZDist / pLogicRouteInfo->m_pRoute->GetNumSegments());

				int zone = CalculateZone(m_state.m_logicPosition);

				if (zone == LogicState::FINAL_THIRD)
				{
					// we have initially gone through the start zone and then turned around and went backwards around the track, exiting the start zone.	
				
					m_state.m_zone = LogicState::FINAL_THIRD;
					m_state.m_lap--;
					m_state.m_lap = Maths::Max(m_state.m_lap, 0);
					m_state.m_logicPositionCombined = m_state.m_logicPosition + (float)m_state.m_lap;
				}
*/
			}
		}
		else if (m_pTrackData && m_pSplineTracker && m_state.m_InFinishZone == true)
		{
			AITrackData *pAITrackData = m_pTrackData->GetTrackData();

			const RouteInfo *pLogicRouteInfo = m_pSplineTracker->GetLogicRoute();

			if (pLogicRouteInfo && pAITrackData->m_TrackLogicData.m_PointToPoint == true)
			{

				m_state.m_logicPosition = 0.0f;

				const RouteInfo *pRoute = m_pSplineTracker->GetLogicFinish();
				if (pRoute)
				{
					Maths::HVec3 pos = m_pVehState->GetPosition();
					int startEndIndex = Maths::Min(8, pRoute->m_pRoute->GetNumSegments()-1);

					if (pRoute->m_pRoute->InfrontSegmentTopPlane(pos, startEndIndex))
					{
						m_state.m_zone = LogicState::FIRST_THIRD;
					}
					else
					{

						int startEndIndex = 1;

						if (pRoute->m_pRoute->InfrontSegmentBottomPlane(pos, startEndIndex))
						{
							m_state.m_zone = LogicState::FIRST_THIRD;
						}
						else
						{
							m_state.m_zone = LogicState::FINAL_THIRD;
						}
					}

					if (m_state.m_zone == LogicState::FINAL_THIRD)
					{
						// we have initially gone through the start zone and then turned around and went backwards around the track, exiting the start zone.	

						m_state.m_zone = LogicState::FINAL_THIRD;
						m_state.m_lap--;
						m_state.m_lap = Maths::Max(m_state.m_lap, 0);
						m_state.m_logicPositionCombined = m_state.m_logicPosition + (float)m_state.m_lap;
					}

				}



/*
				m_state.m_logicPosition = (pLogicRouteInfo->m_index - pAITrackData->m_TrackLogicData.m_finishSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;

				if (m_state.m_logicPosition < 0.0f)
					m_state.m_logicPosition += 1.0f;

				//add in the paramatised version.
				m_state.m_logicPosition += (pLogicRouteInfo->m_ZDist / pLogicRouteInfo->m_pRoute->GetNumSegments());

				int zone = CalculateZone(m_state.m_logicPosition);

				if (zone == LogicState::FINAL_THIRD)
				{
					// we have initially gone through the start zone and then turned around and went backwards around the track, exiting the start zone.	

					m_state.m_zone = LogicState::FINAL_THIRD;
					m_state.m_lap--;
					m_state.m_lap = Maths::Max(m_state.m_lap, 0);
					m_state.m_logicPositionCombined = m_state.m_logicPosition + (float)m_state.m_lap;
				}

*/

			}
		}

		m_state.m_InStartZone = false;
		m_state.m_InFinishZone = false;
	}

	void CVehicleLogic::InitialiseRace()
	{
		CObjectId id = GetObjectId();

		if (m_pTrackData && m_pSplineTracker)
		{
			AITrackData *pAITrackData = m_pTrackData->GetTrackData();
			
			const RouteInfo *pLogicRouteInfo = m_pSplineTracker->GetLogicRoute();

			if (pLogicRouteInfo)
			{
				if (pAITrackData->m_TrackLogicData.m_PointToPoint == false)
				{
					m_state.m_logicPosition = (pLogicRouteInfo->m_index - pAITrackData->m_TrackLogicData.m_startSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;

					if (m_state.m_logicPosition < 0.0f)
						m_state.m_logicPosition += 1.0f;

					m_state.m_logicPosition += (pLogicRouteInfo->m_ZDist / pLogicRouteInfo->m_pRoute->GetNumSegments());

					if (m_state.m_InStartZone)
					{
						m_state.m_logicPosition = 0.0f;
					}
					m_state.m_logicPositionFurthest = m_state.m_logicPosition;

					m_state.m_zone = CalculateZone(m_state.m_logicPositionFurthest);

					// now for the hack of working out what lap we are on
					switch(m_state.m_zone)
					{
					case LogicState::FIRST_THIRD:
						{
							m_state.m_lap = 1;
						}
						break;

					case LogicState::SECOND_THIRD:
						{
							m_state.m_lap = 1;
						}
						break;

					case LogicState::FINAL_THIRD:
						{
							m_state.m_lap = 0;
						}
						break;

					case LogicState::NO_ZONE:
						{
							m_state.m_lap = 0;
						}
						break;

					default:
						break;
					}
				}
				else
				{
					m_state.m_logicPosition = (pLogicRouteInfo->m_index - pAITrackData->m_TrackLogicData.m_finishSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
					m_state.m_logicPosition += (pLogicRouteInfo->m_ZDist / pLogicRouteInfo->m_pRoute->GetNumSegments());
					if (m_state.m_logicPosition < 0.0f)
						m_state.m_logicPosition += 1.0f;
					m_state.m_logicPositionCombined = m_state.m_logicPosition + (float)m_state.m_lap;
	
					if (m_state.m_InFinishZone)
					{
						m_state.m_logicPosition = 0.0f;
					}
					m_state.m_logicPositionFurthest = m_state.m_logicPosition;

					m_state.m_zone = CalculateZone(m_state.m_logicPositionFurthest);
					m_state.m_lap = 1;


					// start the timers...
					// start the new lap
					float timeNow = m_pIGameServerTime->GetTime();
					m_state.m_current.Reset();
					m_state.m_current.StoreStartTime(timeNow);


					CMessage msg;
					msg.mType = MT_TRACKLOGIC_START;
					msg.mpData = 0;
					GlobalsComponent::GetObjectManager().PostAMessage(id,msg);
				}
			}
		
		}

		// initialise the buffered state........
		m_stateBuffered = m_state;
	}

	void CVehicleLogic::InitRace()
	{
		if (m_pTrackData && m_pSplineTracker)
		{
			AITrackData *pAITrackData = m_pTrackData->GetTrackData();

			// we must run this once
			m_pSplineTracker->UpdateMultiRoutes();
#if _DARREN_ADD_IN_POINT_TO_POINT
			InitialiseRace();
#else
			const RouteInfo *pLogicRouteInfo = m_pSplineTracker->GetLogicRoute();
			
			if (pLogicRouteInfo)
			{
				m_state.m_logicPosition = (pLogicRouteInfo->m_index - pAITrackData->m_TrackLogicData.m_startSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;

				if (m_state.m_logicPosition < 0.0f)
					m_state.m_logicPosition += 1.0f;

				m_state.m_logicPosition += (pLogicRouteInfo->m_ZDist / pLogicRouteInfo->m_pRoute->GetNumSegments());

				if (m_state.m_InStartZone)
				{
					m_state.m_logicPosition = 0.0f;
				}
				m_state.m_logicPositionFurthest = m_state.m_logicPosition;

				m_state.m_zone = CalculateZone(m_state.m_logicPositionFurthest);
				
			}
#endif
		}

		m_stateBuffered = m_state;
	}

	int CVehicleLogic::CalculateZone(float logicPos)
	{
		int zone = LogicState::FINAL_THIRD;

		AITrackData *pAITrackData = m_pTrackData->GetTrackData();
		if (pAITrackData)
		{

			if (pAITrackData->m_TrackLogicData.m_PointToPoint == false)
			{
				// Work out the zone.......
				float inter1Pos = (pAITrackData->m_TrackLogicData.m_inter1SamplePoint - pAITrackData->m_TrackLogicData.m_startSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
				if (inter1Pos < 0.0f)
					inter1Pos += 1.0f;
				float inter2Pos = (pAITrackData->m_TrackLogicData.m_inter2SamplePoint - pAITrackData->m_TrackLogicData.m_startSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
				if (inter2Pos < 0.0f)
					inter2Pos += 1.0f;
				
				if (logicPos>= 0.0f && logicPos < inter1Pos)
				{
					// first sector
					zone = LogicState::FIRST_THIRD;
				}
				else if (logicPos >= inter1Pos && logicPos < inter2Pos)
				{
					// second sector
					zone = LogicState::SECOND_THIRD;
				}
				else if (logicPos >= inter2Pos)
				{
					// third sector
					zone = LogicState::FINAL_THIRD;
				}
			}
			else
			{
				float inter1Pos = ((float)pAITrackData->m_TrackLogicData.m_inter1SamplePoint - pAITrackData->m_TrackLogicData.m_finishSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
				if (inter1Pos < 0.0f)
					inter1Pos+= 1.0f;

				float inter2Pos = ((float)pAITrackData->m_TrackLogicData.m_inter2SamplePoint - pAITrackData->m_TrackLogicData.m_finishSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
				if (inter2Pos < 0.0f)
					inter2Pos+= 1.0f;

				float startPos = (pAITrackData->m_TrackLogicData.m_startSamplePoint - pAITrackData->m_TrackLogicData.m_startSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
				if (startPos < 0.0f)
					startPos += 1.0f;	

				
				if (logicPos >=startPos  && logicPos < inter1Pos)
				{
					// first sector
					zone = LogicState::FIRST_THIRD;
				}
				else if (logicPos >= inter1Pos && logicPos < inter2Pos)
				{
					// second sector
					zone = LogicState::SECOND_THIRD;
				}
				else if (logicPos >= inter2Pos)
				{
					// third sector
					zone = LogicState::FINAL_THIRD;
				}
				else 
				{
					zone = LogicState::FINAL_THIRD;
				}
			
			}

			
		}

		return zone;
	}

	float CVehicleLogic::CheckIfInIntermediates()
	{
		AITrackData *pAITrackData = m_pTrackData->GetTrackData();
		float pos = m_state.m_logicPosition;

		if (m_state.m_InInter1Zone == true)
		{
			float inter1Pos = (pAITrackData->m_TrackLogicData.m_inter1SamplePoint - pAITrackData->m_TrackLogicData.m_startSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
			if (inter1Pos < 0.0f)
				inter1Pos += 1.0f;	
	
			if (pAITrackData->m_TrackLogicData.m_PointToPoint)
			{
				inter1Pos = (pAITrackData->m_TrackLogicData.m_inter1SamplePoint - pAITrackData->m_TrackLogicData.m_finishSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
				
			}

			if (inter1Pos < 0.0f)
				inter1Pos += 1.0f;	

			if (pos > inter1Pos)
				pos = inter1Pos;
		}

		if (m_state.m_InInter2Zone == true)
		{
			float inter2Pos = (pAITrackData->m_TrackLogicData.m_inter2SamplePoint - pAITrackData->m_TrackLogicData.m_startSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
			
			if (pAITrackData->m_TrackLogicData.m_PointToPoint)
			{
				inter2Pos = (pAITrackData->m_TrackLogicData.m_inter2SamplePoint - pAITrackData->m_TrackLogicData.m_finishSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
			}

			if (inter2Pos < 0.0f)
				inter2Pos += 1.0f;

			if (pos > inter2Pos)
				pos = inter2Pos;
		}

		return pos;
	}


	void CVehicleLogic::Update()
	{
		PERF_EVENT("AI.VehicleLogic.Update");



		const CObjectId& id = GetObjectId();

		
		if (m_pTrackData && m_pSplineTracker && m_pIGameServerTime)
		{

			m_state.m_InStartZone =m_pSplineTracker->InStartZone();
			m_state.m_InInter1Zone =m_pSplineTracker->InInter1();
			m_state.m_InInter2Zone =m_pSplineTracker->InInter2();
		

			m_state.m_LapIncMessage = false;
			m_state.m_LapDecMessage = false;
			m_state.m_SplitOneMessage = false;
			m_state.m_SplitTwoMessage = false;

			AITrackData *pAITrackData = m_pTrackData->GetTrackData();

			const RouteInfo *pLogicRouteInfo = m_pSplineTracker->GetLogicRoute();
			if (pLogicRouteInfo)
			{
				if (pAITrackData->m_TrackLogicData.m_PointToPoint == false)
				{
					m_state.m_logicPosition = (pLogicRouteInfo->m_index - pAITrackData->m_TrackLogicData.m_startSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;

					if (m_state.m_logicPosition < 0.0f)
						m_state.m_logicPosition += 1.0f;

					//add in the paramatised version.
					m_state.m_logicPosition += (pLogicRouteInfo->m_ZDist / pLogicRouteInfo->m_pRoute->GetNumSegments());

					// creating a dead zone over the start finish
					if (m_state.m_InStartZone)
					{
						m_state.m_logicPosition = 0.0f;
					}

					// Bad data....
					m_state.m_logicPosition = CheckIfInIntermediates();

					m_state.m_DistanceFromStart = m_state.m_logicPosition * pAITrackData->m_TrackLogicData.m_noSamples * pAITrackData->m_TrackLogicData.m_stepSize;
					m_state.m_logicPositionCombined = m_state.m_logicPosition + (float)m_state.m_lap;

					CalculateRacePercenatge();


					//printf("Position Furthest :%f\n", m_state.m_logicPositionFurthest);
					//printf("Position Current :%f\n", m_state.m_logicPosition);
					//printf("DistanceFromstart :%f\n", m_state.m_DistanceFromStart);
				}
				else
				{
					m_state.m_InFinishZone =m_pSplineTracker->InFinish();
					m_state.m_logicPosition = (pLogicRouteInfo->m_index - pAITrackData->m_TrackLogicData.m_finishSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
					
					if (m_state.m_logicPosition < 0.0f)
						m_state.m_logicPosition += 1.0f;

					m_state.m_logicPosition += (pLogicRouteInfo->m_ZDist / pLogicRouteInfo->m_pRoute->GetNumSegments());
					
					if (m_state.m_InStartZone)
					{
						float startPos = (pAITrackData->m_TrackLogicData.m_startSamplePoint - pAITrackData->m_TrackLogicData.m_finishSamplePoint) * pAITrackData->m_TrackLogicData.m_invSize;
						
						if (startPos < 0.0f)
							startPos += 1.0f;	

						m_state.m_logicPosition = startPos;


						// Bug fix for Point to point races....
						if (m_SendLapIncMessagePointToPointHack == false)
						{
							m_SendLapIncMessagePointToPointHack = true;
							m_state.m_LapIncMessage = true;
						}
					}

					m_state.m_logicPosition = CheckIfInIntermediates();
					
					m_state.m_logicPositionCombined = m_state.m_logicPosition + (float)m_state.m_lap;

					CalculateRacePercenatge();
					
				}
				
			}

			m_state.m_current.CalculateRealLapTime(m_pIGameServerTime->GetTime());

			/*
			bool invalidZone = DebugValidateZone();
			if (invalidZone)
			{
				LOGINFO(g_LoggerAI, "Invalid Zone car : %d\n", m_pIData->GetState().m_index);
				
				int stop = 0;
			}
			*/

			CheckWrongWay();

		}


		if (m_state.m_lap > m_stateBuffered.m_lap)
		{
			m_state.m_LapIncMessage = true;
		}
#ifndef CFG_GOLD
		else if(g_IncrementLapCount)
		{
			m_state.m_lap++;
			g_IncrementLapCount = false;
			m_state.m_LapIncMessage = true;
		}
#endif
		else if  (m_state.m_lap < m_stateBuffered.m_lap)
		{
			m_state.m_LapDecMessage = true;
		}

		if (m_state.m_zone != m_stateBuffered.m_zone)
		{
			if (m_state.m_zone == LogicState::FINAL_THIRD && m_stateBuffered.m_zone == LogicState::SECOND_THIRD)
			{
				m_state.m_SplitTwoMessage = true;
			}
			else if (m_state.m_zone == LogicState::SECOND_THIRD && m_stateBuffered.m_zone == LogicState::FIRST_THIRD)
			{
				m_state.m_SplitOneMessage = true;
			}
		}


		m_stateBuffered = m_state;
	}


	void CVehicleLogic::ResetWrongWayTracking()
	{
		m_state.m_reversedDistance = 0.0f;
		m_state.m_prevpos = m_state.m_logicPosition;
		m_state.m_wrongway = false;
	}

	void CVehicleLogic::CheckWrongWay()
	{
		AITrackData *pAITrackData = m_pTrackData->GetTrackData();

		float pos = airacingtech::DistanceInfrontBehindF(m_state.m_logicPosition, m_state.m_prevpos);
		float dist = pos * pAITrackData->m_TrackLogicData.m_noSamples * pAITrackData->m_TrackLogicData.m_stepSize;
		dist = Maths::Max(-1.0f, dist);
		m_state.m_reversedDistance -= dist;

		if (m_state.m_reversedDistance > 0.0f && dist < 0.0f )
		{
			if (m_state.m_reversedDistance > 25.0f) // increase this value to 25 from 15
			{
				// wrong way
				m_state.m_wrongway = true;
			}
			
		}
		else
		{
			ResetWrongWayTracking();
		}

		m_state.m_prevpos = m_state.m_logicPosition;

	}	

	void CVehicleLogic::StartZone()
	{
		CObjectId id = GetObjectId();


		AITrackData *pAITrackData = m_pTrackData->GetTrackData();
		if (pAITrackData->m_TrackLogicData.m_PointToPoint == false)
		{

			if (m_state.m_zone == LogicState::FINAL_THIRD)
			{

				m_state.m_zone = LogicState::FIRST_THIRD;
				m_state.m_lap++;
				m_state.m_logicPositionFurthest = 0.0f;
				m_state.m_logicPosition = 0.0f;
				m_state.m_logicPositionCombined = m_state.m_logicPosition + (float)m_state.m_lap;

				CalculateRacePercenatge();
				

				float timeNow = m_pIGameServerTime->GetTime();

				m_state.m_current.StoreIntermediate3(timeNow);

				m_state.m_last = m_state.m_current;

				if (m_state.m_FastestLap.IsComplete())
				{
					if (m_state.m_last.m_Laptime < m_state.m_FastestLap.m_Laptime)
					{
						m_state.m_FastestLap = m_state.m_last;
					}
				}
				else
				{
					m_state.m_FastestLap = m_state.m_last;
				}
				

				// start the new lap
				m_state.m_current.Reset();
				m_state.m_current.StoreStartTime(timeNow);


				CMessage msg;
				msg.mType = MT_TRACKLOGIC_START;
				msg.mpData = 0;
				GlobalsComponent::GetObjectManager().PostAMessage(id,msg);


			}
			else
			{
				

			}
		}
		else
		{
			// we do nothing on point to point races.....

		}
	}

	void CVehicleLogic::CalculateRacePercenatge()
	{
		AITrackData *pAITrackData = m_pTrackData->GetTrackData();
		if (pAITrackData->m_TrackLogicData.m_PointToPoint == false)
		{
			float combinedPos = m_state.m_logicPositionCombined - 1.0f;
			combinedPos = Maths::Max(combinedPos, 0.0f);
			m_state.m_racePercentage = combinedPos / (float)(m_state.m_Maxlaps);
		}
		else
		{
			//float combinedPos = m_state.m_logicPositionCombined - 1.0f;
			//combinedPos = Maths::Max(combinedPos, 0.0f);
			const RouteInfo *pLogicRouteInfo = m_pSplineTracker->GetLogicRoute();
			if (pLogicRouteInfo)
			{
				float combinedPos = FuzzyRightShoulder(pLogicRouteInfo->m_index, pAITrackData->m_TrackLogicData.m_startSamplePoint, pAITrackData->m_TrackLogicData.m_finishSamplePoint);
				m_state.m_racePercentage = combinedPos; // only rough......
			}
			else
			{
				m_state.m_racePercentage = 0.0f;
			}
			
		}
		
	}

	void CVehicleLogic::FinishZone()
	{
		CObjectId id = GetObjectId();


		AITrackData *pAITrackData = m_pTrackData->GetTrackData();
		if (pAITrackData->m_TrackLogicData.m_PointToPoint)
		{

			if (m_state.m_zone == LogicState::FINAL_THIRD)
			{
				m_state.m_zone = LogicState::FIRST_THIRD;
				m_state.m_lap++;
				m_state.m_logicPositionFurthest = 0.0f;
				m_state.m_logicPosition = 0.0f;
				m_state.m_logicPositionCombined = m_state.m_logicPosition + (float)m_state.m_lap;

				CalculateRacePercenatge();


				float timeNow = m_pIGameServerTime->GetTime();

				m_state.m_current.StoreIntermediate3(timeNow);

				m_state.m_last = m_state.m_current;

				if (m_state.m_FastestLap.IsComplete())
				{
					if (m_state.m_last.m_Laptime < m_state.m_FastestLap.m_Laptime)
					{
						m_state.m_FastestLap = m_state.m_last;
					}
				}
				else
				{
					m_state.m_FastestLap = m_state.m_last;
				}


				// start the new lap
				m_state.m_current.Reset();
				m_state.m_current.StoreStartTime(timeNow);


				CMessage msg;
				msg.mType = MT_TRACKLOGIC_START;
				msg.mpData = 0;
				GlobalsComponent::GetObjectManager().PostAMessage(id,msg);
			}
			else
			{


			}
		}
		else
		{
			// we do nothing on point to point races.....

		}	
	}

	void CVehicleLogic::IntermediateZoneOne()
	{
		CObjectId id = GetObjectId();

		const RouteInfo *pRoute = m_pSplineTracker->GetLogicInter1();
		if (pRoute)
		{
			Maths::HVec3 pos = m_pVehState->GetPosition();
			int inter1EndIndex = 1;

			if (pRoute->m_pRoute->InfrontSegmentBottomPlane(pos, inter1EndIndex))
			{

				m_state.m_zone = LogicState::SECOND_THIRD;
				float timeNow = m_pIGameServerTime->GetTime();
				m_state.m_current.StoreIntermediate1(timeNow);

				CMessage msg;
				msg.mType = MT_TRACKLOGIC_INTERMEDIATE_1;
				msg.mpData = 0;
				GlobalsComponent::GetObjectManager().PostAMessage(id,msg);
			}
			else
			{
				m_state.m_zone = LogicState::FIRST_THIRD;
			}
		}		
		
		
/*
		if (m_state.m_zone == LogicState::FIRST_THIRD)
		{
			m_state.m_zone = LogicState::SECOND_THIRD;
			float timeNow = m_pIGameServerTime->GetTime();
			m_state.m_current.StoreIntermediate1(timeNow);

			CMessage msg;
			msg.mType = MT_TRACKLOGIC_INTERMEDIATE_1;
			msg.mpData = 0;
			GlobalsComponent::GetObjectManager().PostAMessage(id,msg);
		}
		else if (m_state.m_zone == LogicState::SECOND_THIRD)
		{
			// for going backwards around the track
			m_state.m_zone = LogicState::FIRST_THIRD;	
		}
*/
	}

	void CVehicleLogic::IntermediateZoneTwo()
	{
		CObjectId id = GetObjectId();

		/*
		if (m_state.m_zone == LogicState::SECOND_THIRD)
		{
			m_state.m_zone = LogicState::FINAL_THIRD;
			float timeNow = m_pIGameServerTime->GetTime();
			m_state.m_current.StoreIntermediate2(timeNow);

			CMessage msg;
			msg.mType = MT_TRACKLOGIC_INTERMEDIATE_2;
			msg.mpData = 0;
			GlobalsComponent::GetObjectManager().PostAMessage(id,msg);
		}
		else if (m_state.m_zone == LogicState::FINAL_THIRD)
		{
			// for going backwards around the track
			m_state.m_zone = LogicState::SECOND_THIRD;
		}
		*/

		const RouteInfo *pRoute = m_pSplineTracker->GetLogicInter2();
		if (pRoute)
		{
			Maths::HVec3 pos = m_pVehState->GetPosition();
			int inter2EndIndex = 1;

			if (pRoute->m_pRoute->InfrontSegmentBottomPlane(pos, inter2EndIndex))
			{
				m_state.m_zone = LogicState::FINAL_THIRD;
				float timeNow = m_pIGameServerTime->GetTime();
				m_state.m_current.StoreIntermediate2(timeNow);

				CMessage msg;
				msg.mType = MT_TRACKLOGIC_INTERMEDIATE_2;
				msg.mpData = 0;
				GlobalsComponent::GetObjectManager().PostAMessage(id,msg);
			}
			else
			{
				m_state.m_zone = LogicState::SECOND_THIRD;
			}
		}
		
	}

	void CVehicleLogic::LogicZone()
	{
		CObjectId id = GetObjectId();

	}


}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


