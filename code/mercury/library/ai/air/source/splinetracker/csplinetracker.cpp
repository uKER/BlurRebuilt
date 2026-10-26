#include "stdafx.h"
#include <splinetracker/csplinetracker.h>
#include <fuzzylib/fuzzylibone/fuzzyone.h>
#include <messages/RouteChange.h>
#include <messages/EnterDynJunction.h>
#include <system/Perf/PerfEvent.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif


namespace airacingtech
{
	Debug::Logger& g_LogSplineTracker = Debug::GetLogger( "air.splinetracker" );

	using namespace fuzzy_lib_one;


	CSplineTracker::CSplineTracker() : m_pTrackData(0), m_pIData(0), m_pRouteInfoLogic(0), m_pIGameServerTime(0)
	{
		m_CurrentLane = -1;
		m_pCurrentRoute = 0;
		m_PreviousRoute = 0;
		m_distribute_cpu_load = true;
		m_currentTime = airacingtech::RandomInRange (0.016f, 1.0f);
		m_lastTime = m_currentTime;
		m_lastTime = 0.0f;

		m_pStartRoute = 0;
		m_pInter1Route = 0;
		m_pInter2Route = 0;
		m_pFinishRoute = 0;
	}

	CSplineTracker::~CSplineTracker()
	{

	}

	void CSplineTracker::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_SPLINE_TRACKER, CID_SPLINE_TRACKER);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_SPLINE_TRACKER, CSplineTracker::CreateMe, CSplineTracker::DestroyMe, CHash("SplineTracker"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPLINE_TRACKER, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPLINE_TRACKER, MT_DISTRIBUTE_TRACKING_CPU_LOAD);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPLINE_TRACKER, MT_START_AI_RACE);
	
	}

	IComponent *CSplineTracker::CreateMe()
	{
		return new CSplineTracker;
	}

	bool CSplineTracker::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}

	// Virtual IComponent methods
	bool CSplineTracker::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}


	void CSplineTracker::Deinit(void)
	{
		m_splineIndexList.Clear();

		m_RouteInfo.Clear();
		m_RouteInfoBuffered.Clear();

		m_pCurrentRoute = 0;
		m_PreviousRoute = 0;;
	}

	EMessageResult CSplineTracker::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;
		switch (msg.mType)
		{
		
			case MT_POST_INIT_AI_SYSTEM:
			{
				msgRet =  MR_TRUE;
				CachedInterfaces();

				Setup();
			}
			break;

			case MT_START_AI_RACE:
			{
				// need to re-intialise at the start due to cars swerving to the other side of the track in the splinefollower, due to its offset being set to zero during the intro
				RecordDistanceToSplines();

				if (m_pCurrentRoute)
				{
					
					CObjectId id = GetObjectId();	
					// broadcast we have just changed route.........
					RouteChangeMSG routeChnageMessage(m_pCurrentRoute->m_offset);
					CMessage msg;
					msg.mType = MT_CHANGED_ROUTE;
					msg.mpData = &routeChnageMessage;
					GlobalsComponent::GetObjectManager().PostAMessage(id,msg);
				}
				
			}
			break;

			case MT_INIT_RACE:
				{
				}

				break;

			case MT_DISTRIBUTE_TRACKING_CPU_LOAD:
				{
					m_distribute_cpu_load = true;
				}
				break;

		
			default:
				break;
		}

		return msgRet;
	}

	void CSplineTracker::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));
		
		CObjectId trackdataSystem("TrackDataSystem");
		m_pTrackData = static_cast<ITrackDataAI *>(GlobalsComponent::GetObjectManager().QueryInterface(trackdataSystem, IID_TRACK_DATA_AI));
	
		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));
		
	}

	void CSplineTracker::Setup()
	{
		//CObjectId id = GetObjectId();

		if (m_pTrackData)
		{
			AITrackData *pAITrackData = m_pTrackData->GetTrackData();

			int noRoutes = pAITrackData->GetNumRoutes();

			m_RouteInfo.Reserve(noRoutes);
			m_RouteInfoBuffered.Reserve(noRoutes);

			for(int i=0; i<noRoutes; i++)
			{
				AITrackDataRoutes *pRoute = pAITrackData->GetRoute(i);

				m_RouteInfo.PushBack(RouteInfo(-1, pRoute, false,i));
				m_RouteInfoBuffered.PushBack(RouteInfo(-1, pRoute, false,i));

				RouteInfo *pRouteInfo = &m_RouteInfo.Back();
				RouteInfo *pRouteInfo2 = &m_RouteInfoBuffered.Back();
				
				assert(pRouteInfo && pRouteInfo2);

				if (pRouteInfo->m_pRoute->IsLogic())
				{
					// cache this route
					m_pRouteInfoLogic = pRouteInfo;
				}

				if (pRouteInfo->m_pRoute->IsStart())
					m_pStartRoute = pRouteInfo;

				if (pRouteInfo->m_pRoute->IsIntermediate1())
					m_pInter1Route = pRouteInfo;

				if (pRouteInfo->m_pRoute->IsIntermediate2())
					m_pInter2Route = pRouteInfo;

				if (pRouteInfo->m_pRoute->IsFinish())
					m_pFinishRoute = pRouteInfo;

			}

		}
	}

	


	EComponentTypeId CSplineTracker::GetComponentTypeId(void)
	{
		return CID_SPLINE_TRACKER;
	}


	


	void CSplineTracker::Update()
	{

		//CObjectId id = GetObjectId();

		m_currentTime = m_pIGameServerTime->GetTime();
	
		UpdateMultiRoutesOpt();

		m_lastTime = m_currentTime;
		
	}

	

	void CSplineTracker::UpdateMultiRoutesOpt()
	{

		PERF_EVENT("AI.SplineTracker.UpdateMultiRoutesOpt");
		
	





		const CObjectId& id = GetObjectId();	

		// update the logic route first.....
		if (m_pIData && m_pTrackData && m_pRouteInfoLogic)
		{
			int indexLogic = -1;
			if (m_pRouteInfoLogic->m_in == false)
			{
				// should only be in here occassionally, maybe at the start.
				FindNearestIndexFromRoute(m_pRouteInfoLogic->m_pRoute, indexLogic, m_pRouteInfoLogic->m_ZDist);
			}
			else
			{
				// cached check.......
				FindNearestIndexFromRouteCached(m_pRouteInfoLogic->m_pRoute, indexLogic, m_pRouteInfoLogic->m_RouteIndex, m_pRouteInfoLogic->m_ZDist);
			}

			if (indexLogic != -1)
			{
				m_pRouteInfoLogic->m_in = true;
				m_pRouteInfoLogic->m_index = indexLogic;

				// update the other routes using this info
				
				int index = -1;

				// Darren....
				const AITrackDataCache& dc = m_pRouteInfoLogic->m_pRoute->m_dataCache[m_pRouteInfoLogic->m_index];
				const int noEntries = dc.m_data.Size();	

				//printf("No Volumes :%d, No Entries :%d\n",m_RouteInfo.Size(), noEntries );

				for(int i=0; i<m_RouteInfo.Size(); i++)
				{
					index = -1;

					RouteInfo *pTestRoute = &m_RouteInfo[i];
					if (pTestRoute != m_pRouteInfoLogic ) //&& pTestRoute->m_pRoute->IsActive())
					{
						if (pTestRoute->m_in == true)
						{
							// cached check
							FindNearestIndexFromRouteCached(m_RouteInfo[i].m_pRoute,index, m_RouteInfo[i].m_RouteIndex, m_RouteInfo[i].m_ZDist);				
						}
						else
						{
							for(int entries = 0; entries <noEntries; entries++ )
							{
								const AITrackDataCachedData& cachedData = dc.m_data[entries];
								
								if (cachedData.m_pRoute == pTestRoute->m_pRoute)
								{
									// we have found our valid route.....
									int startIndex = cachedData.m_index;
									startIndex = Maths::Clamped(startIndex, 0, pTestRoute->m_pRoute->GetNumSegments() - 1);

									pTestRoute->m_index = startIndex;
									
									// cached check
									FindNearestIndexFromRouteCached(pTestRoute->m_pRoute, index, pTestRoute->m_RouteIndex,pTestRoute->m_ZDist);				
									
									if (index != -1)
									{
										break;
									}
								}
							}
					
							
						}

						if (index != -1)
						{
							pTestRoute->m_in = true;
							pTestRoute->m_index = index;

							// specific fix for start zones that are too long.......
							// and making sure we are past the start line, problem is sample length is too great on corners...
							/*if (pTestRoute->m_pRoute->IsStart() && pTestRoute->m_index > 2)
							{
								AITrackData *pAITrackData = m_pTrackData->GetTrackData();
								if (m_pRouteInfoLogic->m_index >= pAITrackData->m_TrackLogicData.m_startSamplePoint) 
								{
									pTestRoute->m_in = false;
									pTestRoute->m_index = -1;
								}
							}
							else if (pTestRoute->m_pRoute->IsFinish() && pTestRoute->m_index > 2)
							{
								AITrackData *pAITrackData = m_pTrackData->GetTrackData();
								if (m_pRouteInfoLogic->m_index >= pAITrackData->m_TrackLogicData.m_finishSamplePoint) 
								{
									pTestRoute->m_in = false;
									pTestRoute->m_index = -1;
								}
							}*/

							{
								AITrackData *pAITrackData = m_pTrackData->GetTrackData();
								if (((pTestRoute->m_pRoute->IsStart() && m_pRouteInfoLogic->m_index > pAITrackData->m_TrackLogicData.m_startSamplePoint)
									|| (pTestRoute->m_pRoute->IsFinish() && m_pRouteInfoLogic->m_index > pAITrackData->m_TrackLogicData.m_finishSamplePoint)
									|| (pTestRoute->m_pRoute->IsIntermediate1() && m_pRouteInfoLogic->m_index > pAITrackData->m_TrackLogicData.m_inter1SamplePoint)
									|| (pTestRoute->m_pRoute->IsIntermediate2() && m_pRouteInfoLogic->m_index > pAITrackData->m_TrackLogicData.m_inter2SamplePoint)
									|| ((pTestRoute->m_pRoute->IsStart() || pTestRoute->m_pRoute->IsFinish() || pTestRoute->m_pRoute->IsIntermediate1() || pTestRoute->m_pRoute->IsIntermediate2()) && pTestRoute->m_index > Maths::Min(8, pTestRoute->m_pRoute->GetNumSegments()))) // this line stops us entering the start twice
									&& pTestRoute->m_index > Maths::Min(8, pTestRoute->m_pRoute->GetNumSegments())) 
								{
									pTestRoute->m_in = false;
									pTestRoute->m_index = -1;

								}
							}


							// only interested in the current route....
							if (pTestRoute->m_pRoute->IsChangeableRoute() && m_pCurrentRoute == pTestRoute && pTestRoute->m_pRoute->IsTrigger(index))
							{	
								CMessage msgTrigger;
								msgTrigger.mType = MT_SEGMENT_TRIGGER;
								msgTrigger.mpData = 0;
								GlobalsComponent::GetObjectManager().PostAMessage(id,msgTrigger);
							}
							

						}
						else
						{
							pTestRoute->m_in = false;
							pTestRoute->m_justEntered = false;
						}
					}
					
				}



				bool leavingCurrentVolume = false;

				// work out which volumes are different........
				for(int i=0; i<m_RouteInfo.Size(); i++)
				{
					if (m_RouteInfo[i].m_in != m_RouteInfoBuffered[i].m_in)
					{
						if (m_RouteInfo[i].m_in == true)
						{
							m_RouteInfo[i].m_justEntered = true;

							// Post any messages for logic....when entering a volume...
							LogicMessages(&m_RouteInfo[i], true);

						}
						else
						{

							// leaving a volume..........
							// Post any messages for logic....when entering a volume...
							LogicMessages(&m_RouteInfo[i], false);
							

							m_RouteInfo[i].m_justEntered = false;

							//RouteInfo *pDRoute = m_RouteInfo[i];
							//if (pDRoute->m_pRoute->IsChangeableRoute())
							//{
							//	StoreLastValidPositionAndDirection(pDRoute);
							//}

							// we are leaving a new volume.....
							if (m_pCurrentRoute == &m_RouteInfo[i])
							{
								// we are leaving the current volume....
								leavingCurrentVolume = true;
								m_pCurrentRoute = 0; // forces us to pick a new route

							}

						}

					}
					else
					{
						
					}



					// set the buffered input...copy
					m_RouteInfoBuffered[i].m_dist = m_RouteInfo[i].m_dist;
					m_RouteInfoBuffered[i].m_in = m_RouteInfo[i].m_in;
					m_RouteInfoBuffered[i].m_index = m_RouteInfo[i].m_index;
					m_RouteInfoBuffered[i].m_width = m_RouteInfo[i].m_width;
				}


				// new stuff with junction connections
				for(int i=0; i<m_RouteInfo.Size(); i++)
				{
					if (m_RouteInfo[i].m_justEntered == true && m_RouteInfo[i].m_pRoute->IsJunction())
					{
						ChangeRouteUsingJunctions(&m_RouteInfo[i], false);
					}

					if (m_RouteInfo[i].m_justEntered == true)
					{
						m_RouteInfo[i].m_justEntered = false;
					}
				}

				if (m_pCurrentRoute == 0 ||  m_pCurrentRoute->m_in == false)
				{
					// if current junction is null,
					for(int i=0; i<m_RouteInfo.Size(); i++)
					{
						if (m_RouteInfo[i].m_in == true && m_RouteInfo[i].m_pRoute->IsChangeableRoute() && m_RouteInfo[i].m_pRoute->IsDropPriority())
						{
							m_pCurrentRoute = &m_RouteInfo[i];

							WorkOutOffsetToRacingSpline(m_pCurrentRoute);

							CObjectId id = GetObjectId();	
							// broadcast we have just changed route.........
							RouteChangeMSG routeChnageMessage(m_pCurrentRoute->m_offset);
							CMessage msg;
							msg.mType = MT_CHANGED_ROUTE;
							msg.mpData = &routeChnageMessage;
							GlobalsComponent::GetObjectManager().PostAMessage(id,msg);

							break;
						}
					}
					
					if (m_pCurrentRoute == 0 ||  m_pCurrentRoute->m_in == false)
					{
						// if current junction is null,
						for(int i=0; i<m_RouteInfo.Size(); i++)
						{
							if (m_RouteInfo[i].m_in == true && m_RouteInfo[i].m_pRoute->IsChangeableRoute())
							{
								m_pCurrentRoute = &m_RouteInfo[i];

								WorkOutOffsetToRacingSpline(m_pCurrentRoute);

								CObjectId id = GetObjectId();	
								// broadcast we have just changed route.........
								RouteChangeMSG routeChnageMessage(m_pCurrentRoute->m_offset);
								CMessage msg;
								msg.mType = MT_CHANGED_ROUTE;
								msg.mpData = &routeChnageMessage;
								GlobalsComponent::GetObjectManager().PostAMessage(id,msg);

								break;
							}
						}
					}


				}


				if (m_pCurrentRoute)
				{
					//StoreLastValidPositionAndDirection(m_pCurrentRoute);
					StoreValidInfoForReset(m_pCurrentRoute);
				}


				///////////////////////////////////////////////////////////////////////////////////
				// Distance to the racing line paramatised....
				RecordDistanceToSplines();




			}
			else
			{
				m_pRouteInfoLogic->m_in = false;
				// if we are not in the logic volume, do nothing as it is an error......
			}
		}
		


	}

	void CSplineTracker::UpdateMultiRoutes()
	{
		UpdateMultiRoutesOpt();
	}

	void CSplineTracker::LogicMessages(RouteInfo *pRouteInfo, int entering)
	{
		CObjectId id = GetObjectId();

		if (!pRouteInfo->m_pRoute->IsChangeableRoute())
		{
			if (pRouteInfo->m_pRoute->IsStart())
			{
				// just the once.....
				CMessage msg;
				if (entering)
				{
					msg.mType = MT_ENTERING_START_ZONE;
				}
				else
				{
					msg.mType = MT_LEAVING_START_ZONE;
				}

				msg.mpData = 0;
				GlobalsComponent::GetObjectManager().PostAMessage(id, msg);
			}

			if (pRouteInfo->m_pRoute->IsIntermediate(1))
			{
				// just the once.....
				CMessage msg;
				if (entering)
					msg.mType = MT_ENTERING_INTERMEDIATE_1;
				else
					msg.mType = MT_LEAVING_INTERMEDIATE_1;

				msg.mpData = 0;
				GlobalsComponent::GetObjectManager().PostAMessage(id, msg);
			}

			if (pRouteInfo->m_pRoute->IsIntermediate(2))
			{
				// just the once.....
				CMessage msg;
				if (entering)
					msg.mType = MT_ENTERING_INTERMEDIATE_2;
				else 
					msg.mType = MT_LEAVING_INTERMEDIATE_2;

				msg.mpData = 0;
				GlobalsComponent::GetObjectManager().PostAMessage(id, msg);
			}

			if (pRouteInfo->m_pRoute->IsFinish())
			{
				// just the once.....
				CMessage msg;
				if (entering)
					msg.mType = MT_ENTERING_FINISH_ZONE;
				else
					msg.mType = MT_LEAVING_FINISH_ZONE;

				msg.mpData = 0;
				GlobalsComponent::GetObjectManager().PostAMessage(id, msg);
			}

			if (pRouteInfo->m_pRoute->IsDynamicJunction())
			{
				EnterDynamicJunctionMSG junctionMessage(pRouteInfo->m_pRoute);

				CMessage msg;
				if (entering)
				{
					msg.mType = MT_ENTERING_DYNAMIC_JUNCTION_ZONE;
				}
				else
				{
					msg.mType = MT_LEAVING_DYNAMIC_JUNCTION_ZONE;
				}
				msg.mpData = &junctionMessage;
				GlobalsComponent::GetObjectManager().BroadcastMessage(msg);
			}
		}
	}

	void CSplineTracker::ChangeRouteUsingJunctions(RouteInfo *pRouteInfo,  bool leaving)
	{
		//CObjectId id = GetObjectId();

		static CObjectId zeroRoute("zero"); // all lower case...

		if (pRouteInfo->m_pRoute->IsJunction() && pRouteInfo->m_pRoute->IsActive())
		{
			int routeInputIndex = FindRouteByName(pRouteInfo->m_pRoute->m_InputRoute);

			RouteInfo *pInputRoute = 0;
			if (routeInputIndex != -1)
				pInputRoute = &m_RouteInfo[routeInputIndex];
			
			if (m_pCurrentRoute == pInputRoute || pRouteInfo->m_pRoute->m_InputRoute == zeroRoute) // we are checking for a correct input route, sometimes we don`t car what that input route is, therefore ZERO is specified(NOTE : could also be an error)
			{
				// find this route
				int routeIndex = FindRouteByName(pRouteInfo->m_pRoute->m_OutputRoute);
				if (routeIndex != -1)
				{
					RouteInfo *pOutputRoute = &m_RouteInfo[routeIndex];
					assert(pOutputRoute);
					WorkOutOffsetToRacingSpline(pOutputRoute);

					if (pOutputRoute->m_pRoute->IsChangeableRoute())
					{
						m_pCurrentRoute = pOutputRoute;

						CObjectId id = GetObjectId();	

						// broadcast we have just changed route.........
						RouteChangeMSG routeChnageMessage(m_pCurrentRoute->m_offset);
						CMessage msg;
						msg.mType = MT_CHANGED_ROUTE;
						msg.mpData = &routeChnageMessage;
						GlobalsComponent::GetObjectManager().PostAMessage(id,msg);
					}
					

				}
			}
		}
	}

	// TODO Does current route match the route we are in.....

	void CSplineTracker::ChangeRoute(RouteInfo *pRouteInfo, bool leaving)
	{
		//CObjectId id = GetObjectId();

		if (pRouteInfo->m_pRoute->IsChangeableRoute())
		{
			// assumes the DataState holds a valid Vehicle Interface and state.
			DataState &myDataState = m_pIData->GetState();
			VehicleState *pVehicleState = myDataState.m_pIVehicleState->GetState();
			assert(pVehicleState);

			m_PreviousRoute = m_pCurrentRoute;

			WorkOutOffsetToRacingSpline(pRouteInfo);

			bool broadcastMessage = false;

			/*if (leaving == true)
			{
				//m_CurrentLane = DecideLane(pRouteInfo);
				m_pCurrentRoute = pRouteInfo;
				broadcastMessage = true;
			}
			else
			*/
			{

				if (pVehicleState->GetPlayer() == false)
				{
					// entering.. ai can really only enter in the first 30 metres of a new route
					// need to check if it is looped.......
					if (pRouteInfo->m_index > 3 && pRouteInfo->m_index < 15) // really early....30.0f
					{
						pRouteInfo->m_justEntered = false;
						m_pCurrentRoute = pRouteInfo;
						broadcastMessage = true;
					}

					// if the current route is null, make this a valid route
					if (m_pCurrentRoute == 0)
					{
						pRouteInfo->m_justEntered = false;
						m_pCurrentRoute = pRouteInfo;
						broadcastMessage = true;
					}
				}
				else
				{
					pRouteInfo->m_justEntered = false;
					m_pCurrentRoute = pRouteInfo;
					broadcastMessage = true;
				}
			}

			if (broadcastMessage == true)
			{
				// broadcast we have just changed route.........
				CObjectId id = GetObjectId();	
				RouteChangeMSG routeChnageMessage(pRouteInfo->m_offset);
				CMessage msg;
				msg.mType = MT_CHANGED_ROUTE;
				msg.mpData = &routeChnageMessage;
				GlobalsComponent::GetObjectManager().PostAMessage(id,msg);
			}
		}
	}

	int CSplineTracker::DecideLane(RouteInfo *pRouteInfo)
	{
		//CObjectId id = GetObjectId();

		int lane = -1;
		
		// simply the nearest spline......
		/*lane = NearestLane(pRouteInfo);
		*/

		lane = pRouteInfo->NearestLane();


		// we should check that we are within a tolerance of the racing line
		/*const float tol = 2.1f;//1.0f;
		float offset = pRouteInfo->m_pRoute->GetOffset( pRouteInfo->m_index, AITrackDataLines::TRACK_DATA_LINE_RACING);
		float diff = offset - pRouteInfo->m_dist;
		float distance = pRouteInfo->m_width * diff;
		if (Maths::Abs(distance) < tol)
		{
			lane = AITrackDataLines::TRACK_DATA_LINE_RACING;
		}
		*/

		return lane;
	}

	int CSplineTracker::NearestLane(RouteInfo *pRouteInfo)
	{
		//CObjectId id = GetObjectId();

		int lane = -1;

		float minDiff = 10000000.0f;
		
		for(int j=0; j<AITrackDataLines::TRACK_DATA_NUM_LINES; j++)
		{
			float offset = pRouteInfo->m_pRoute->GetOffset( pRouteInfo->m_index, j);
			float diff = Maths::Abs(offset - pRouteInfo->m_dist);
			if (diff < minDiff)
			{
				minDiff = diff;
				lane = j;
			}
		}

		return lane;
	}


	void CSplineTracker::WorkOutOffsetToRacingSpline(RouteInfo *pRouteInfo)
	{


		//CObjectId id = GetObjectId();

		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		if (pRouteInfo->m_in == true)
		{
			DataState &myDataState = m_pIData->GetState();
			VehicleState *pVehicleState = myDataState.m_pIVehicleState->GetState();
			assert(pVehicleState);

			Maths::HVec3 position = pVehicleState->GetPosition();
			Maths::HVec3 startPos(Maths::UNDEFINED);
			Maths::HVec3 endPos(Maths::UNDEFINED);

		// work out the distance to the lines...
		
			int startIndex = -1;
			int endIndex = -1;
			// distance to the lines.......
			if (pRouteInfo->m_pRoute->IsLoopedTrack())
			{
				startIndex = pRouteInfo->m_index; 
				endIndex = pRouteInfo->m_pRoute->GetIndexOffset(pRouteInfo->m_index, 1);	
			}
			else
			{
				// assuming that the spline is longer than 4 samples in length
				startIndex = pRouteInfo->m_index; 
				endIndex = pRouteInfo->m_pRoute->GetIndexOffset(pRouteInfo->m_index, 1);
				if (endIndex <= startIndex)
				{
					endIndex = startIndex;
					startIndex--;
				}
			}

			pRouteInfo->m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, startIndex , startPos);
			pRouteInfo->m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, endIndex , endPos);	

			// side distance is ? this has to be positive...due to using the extreme right position in the calculation.
			const Maths::HF32 d = Maths::Abs(airacingtech::DistToLine(position, startPos, endPos));
			
			// paramatise the distance
			const Maths::HF32 trackWidth = pRouteInfo->m_pRoute->GetWidth(startIndex);
			Maths::HF32 paramD = d / trackWidth;
			paramD = Maths::Clamped(paramD, zero, one );

			pRouteInfo->m_offsetCentre = half - paramD;
		

			Maths::HF32 fzOffset = zero;

			Maths::HF32 offsetRacing = pRouteInfo->m_pRoute->GetOffset(startIndex, AITrackDataLines::TRACK_DATA_LINE_RACING);

			if (paramD > offsetRacing)
			{
				// on the left of the racing line
				Maths::HF32 offsetLeft = pRouteInfo->m_pRoute->GetOffset(startIndex, AITrackDataLines::TRACK_DATA_LINE_LEFT_5);
				fzOffset = -Maths::Abs(FuzzyRightShoulder(paramD, offsetRacing, offsetLeft ));
			}
			else
			{
				// on the right of the racing line

				// right of the line
				Maths::HF32 offsetRight = pRouteInfo->m_pRoute->GetOffset(startIndex, AITrackDataLines::TRACK_DATA_LINE_RIGHT_5);
				fzOffset = one - FuzzyRightShoulder(paramD, offsetRight, offsetRacing );
			}

			pRouteInfo->m_offset = fzOffset; // store the distance
			pRouteInfo->m_width = trackWidth; // store the track width

		}
		else
		{
			pRouteInfo->m_offset = 0.0f;
			pRouteInfo->m_offsetCentre = FLT_MAX;
		}
	}

	/*
	void CSplineTracker::DistanceToSplines(RouteInfo *pRouteInfo)
	{
		//CObjectId id = GetObjectId();

		DataState &myDataState = m_pIData->GetState();
		VehicleState *pVehicleState = myDataState.m_pIVehicleState->GetState();
		assert(pVehicleState);

		Maths::HVec3 position = pVehicleState->GetPosition();
		Maths::HVec3 startPos;
		Maths::HVec3 endPos;

		// work out the distance to the lines...
		if (pRouteInfo->m_in == true)
		{
			int startIndex = -1;
			int endIndex = -1;
			// distance to the lines.......
			if (pRouteInfo->m_pRoute->IsLoopedTrack())
			{
				startIndex = pRouteInfo->m_index; 
				endIndex = pRouteInfo->m_pRoute->GetIndexOffset(pRouteInfo->m_index, 1);	
			}
			else
			{
				// assuming that the spline is longer than 4 samples in length
				startIndex = pRouteInfo->m_index; 
				endIndex = pRouteInfo->m_pRoute->GetIndexOffset(pRouteInfo->m_index, 1);
				if (endIndex <= startIndex)
				{
					endIndex = startIndex;
					startIndex--;
				}
			}

			pRouteInfo->m_pRoute->GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, startIndex , startPos);
			pRouteInfo->m_pRoute->GetPosition(AITrackDataLines::TRACK_DATA_EXTREME_RIGHT, endIndex , endPos);	

			// side distance is ? this has to be positive...due to using the extreme right position in the calculation.
			float d = Maths::Abs(airacingtech::DistToLine(position, startPos, endPos));

			// paramatise the distance
			float trackWidth = pRouteInfo->m_pRoute->GetWidth(startIndex);
			float paramD = d / trackWidth;
			pRouteInfo->m_offsetCentre = paramD;
			pRouteInfo->m_dist = paramD; // store the distance
			pRouteInfo->m_width = trackWidth; // store the track width

			float offset = pRouteInfo->m_pRoute->GetOffset(startIndex, AITrackDataLines::TRACK_DATA_LINE_RACING);

			// left or right of the racing line....
			float diffParam = offset - paramD;
			float diffDistance = diffParam * trackWidth;
		}
		else
		{
			pRouteInfo->m_dist = -1.0f;
		}
	}
	*/
	
	
	


	void CSplineTracker::RecordDistanceToSplines()
	{

		//CObjectId id = GetObjectId();

		for(int i=0; i<m_RouteInfo.Size(); i++)
		{
			WorkOutOffsetToRacingSpline(&m_RouteInfo[i]);

			WorkOutPositionAroundTheSpline(&m_RouteInfo[i]);
		}
	}

	
	void CSplineTracker::WorkOutPositionAroundTheSpline(RouteInfo *pRouteInfo)
	{
		if (pRouteInfo && pRouteInfo->m_in == true)
		{
			pRouteInfo->m_pos = pRouteInfo->m_index * (1.0f / pRouteInfo->m_pRoute->GetNumSegments());

			if (pRouteInfo->m_pos < 0.0f)
				pRouteInfo->m_pos += 1.0f;

			pRouteInfo->m_pos += (pRouteInfo->m_ZDist / pRouteInfo->m_pRoute->GetNumSegments());


			if (pRouteInfo->m_pRoute->IsLogic())
			{
				//LOGINFO(g_LogSplineTracker, "Pos : [%f] ZDist : [%f]", pRouteInfo->m_pos, pRouteInfo->m_ZDist );
			}


		}
	}
	
	void CSplineTracker::FindNearestIndexFromRouteCached(AITrackDataRoutes *pRoute, int &index, int routeIndex, float &paramDist)
	{

		PERF_EVENT("AI.SplineTracker.FindNearestIndexFromRouteCached");

		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		//CObjectId id = GetObjectId();
		DataState &myDataState = m_pIData->GetState();
		VehicleState *pVehicleState = myDataState.m_pIVehicleState->GetState();
		assert(pVehicleState);

		Maths::HVec3 position = pVehicleState->GetPosition();
		paramDist = zero;

		index = -1;
		if (pRoute->IsInBounds(position))
		{		
			// need to get the position.....
			Maths::HVec3 pos = position;

			// just so we can pass the RouteInfo to the other functions....
			RouteInfo *pRouteInfo = &m_RouteInfo[routeIndex];

			int startIndex = pRoute->GetIndexOffset(pRouteInfo->m_index, -2);//-5); // probably get away with 5
			int endIndex = pRoute->GetIndexOffset(pRouteInfo->m_index, 8);//5);

			if (pRoute->IsLoopedTrack() & (startIndex > endIndex))
			{
				int st = startIndex;
				int ed = pRoute->GetNumSegments() - 1;
				index =  pRoute->FindNearest( st, ed, pos, paramDist, pRoute->GetStepSize());
				if (index == -1)
				{
					st = 1;
					ed = endIndex;
					index =  pRoute->FindNearest( st, ed, pos, paramDist, pRoute->GetStepSize());
				}
			}
			else
			{
				startIndex = Maths::Max(startIndex-1, 1);
				index =  pRoute->FindNearest( startIndex, endIndex, pos, paramDist, pRoute->GetStepSize());
			}	
		}	
	}

	void CSplineTracker::FindNearestIndexFromRoute(AITrackDataRoutes *pRoute, int &index, float &paramDist)
	{
		PERF_EVENT("AI.SplineTracker.FindNearestIndexFromRoute");

		//CObjectId id = GetObjectId();

		// TODO : Optimise, to make it a lot lot lot lot lot lot faster.....
		// use larger steps.. ie 50 metre samples ???? then narrow down
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		
		DataState &myDataState = m_pIData->GetState();
		VehicleState *pVehicleState = myDataState.m_pIVehicleState->GetState();
		assert(pVehicleState);

		Maths::HVec3 position = pVehicleState->GetPosition();

		paramDist = zero;

		if (pRoute->IsInBounds(position))
		{
			// need to get the position.....
			Maths::HVec3 pos = position;


			if (pRoute->IsStart() | pRoute->IsFinish() | pRoute->IsIntermediate1() | 
				pRoute->IsIntermediate2() | pRoute->IsJunction())
			{
				index = pRoute->FindNearest(1, pRoute->GetNumSegments()-1, pos,paramDist, pRoute->GetStepSize());
			}
			else
			{
				index = pRoute->FindNearestQuick(1, pRoute->GetNumSegments()-1, pos,paramDist, 3, pRoute->GetStepSize());
			}
			


		}
		else
		{
			index = -1;
		}
	}

	void CSplineTracker::StoreValidInfoForReset(RouteInfo *pRouteInfo)
	{
		assert(pRouteInfo);
		assert(pRouteInfo->m_index != -1);
		assert(m_pRouteInfoLogic);
		assert(m_pRouteInfoLogic->m_index != -1);

		bool inStart = false;
		bool inInter1 = false;
		bool inInter2 = false;
		bool inFinsh = false;


		if (m_pStartRoute && m_pStartRoute->m_in && m_pStartRoute->m_index < Maths::Min(8, m_pStartRoute->m_pRoute->GetNumSegments()) )//10)
			inStart = true;

		if (m_pInter1Route && m_pInter1Route->m_in && m_pInter1Route->m_index < Maths::Min(8, m_pInter1Route->m_pRoute->GetNumSegments()) )//10)
			inInter1 = true;

		if (m_pInter2Route && m_pInter2Route->m_in && m_pInter2Route->m_index < Maths::Min(8, m_pInter2Route->m_pRoute->GetNumSegments()) )//10)
			inInter2 = true;

		if (m_pFinishRoute && m_pFinishRoute->m_in && m_pFinishRoute->m_index < Maths::Min(8, m_pFinishRoute->m_pRoute->GetNumSegments()) )//10)
			inFinsh = true;

		bool inTroubleZones = false;
		if (inStart || inInter1 || inInter2 || inFinsh)
			inTroubleZones = true;



		const AITrackDataCache& dc = m_pRouteInfoLogic->m_pRoute->m_dataCache[m_pRouteInfoLogic->m_index];
		const int noEntries = dc.m_data.Size();	
		for(int entries = 0; entries <noEntries; entries++ )
		{
			const AITrackDataCachedData& cachedData = dc.m_data[entries];
			if ((m_pFinishRoute && cachedData.m_pRoute == m_pFinishRoute->m_pRoute && m_pFinishRoute->m_index < Maths::Min(8, m_pFinishRoute->m_pRoute->GetNumSegments()) ) || 
				(m_pInter2Route && cachedData.m_pRoute == m_pInter2Route->m_pRoute && m_pInter2Route->m_index < Maths::Min(8, m_pInter2Route->m_pRoute->GetNumSegments()) ) || 
				(m_pInter1Route && cachedData.m_pRoute == m_pInter1Route->m_pRoute && m_pInter1Route->m_index < Maths::Min(8, m_pInter1Route->m_pRoute->GetNumSegments()) ) || 
				(m_pStartRoute && cachedData.m_pRoute == m_pStartRoute->m_pRoute && m_pStartRoute->m_index < Maths::Min(8, m_pStartRoute->m_pRoute->GetNumSegments()) )
				)
			{
				inTroubleZones = true;
				break;
			}		
		}


		if (inTroubleZones == false)
		{

			IVehicleLogic *pVehicleLogicMy = m_pIData->GetVehicleLogic();
			if (pVehicleLogicMy)
				pVehicleLogicMy->StoreResetLogicState();

			m_LastValidInfo.m_pResetRoute = pRouteInfo;
			m_LastValidInfo.m_ResetRouteIndex = pRouteInfo->m_index;

			if (m_pRouteInfoLogic->m_index != -1)
				m_LastValidInfo.m_LogicIndex =  m_pRouteInfoLogic->m_index;


			pRouteInfo->m_pRoute->GetDirection(pRouteInfo->m_index , m_LastValidInfo.m_SplineDirection);
			pRouteInfo->m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_LINE_RACING, m_LastValidInfo.m_ResetRouteIndex , m_LastValidInfo.m_position);

			//float minX = 0.0f;
			//float maxX = m_LastValidInfo.m_position.Y();
			Maths::HF32 minX(Maths::UNDEFINED), maxX(Maths::UNDEFINED);
			pRouteInfo->m_pRoute->GetHeightBounds(minX, maxX, pRouteInfo->m_index);

			m_LastValidInfo.m_position.Y(maxX-1.0f); //should be way above the track. The actual reset function should check against the landscape

			m_validPosition = m_LastValidInfo.m_position;
			m_validSplineDirection = m_LastValidInfo.m_SplineDirection;

			// work out a speed to reset to..
			m_LastValidInfo.m_speed = Maths::Min(30.0f, m_pIData->GetState().m_trackSpeed * 0.5f);
		}
		

	
	}


	void CSplineTracker::StoreLastValidPositionAndDirection(RouteInfo *pRouteInfo)
	{
		


		// do not record reset position in the trouble zones
		if (pRouteInfo)
		{
			int index = pRouteInfo->m_index;
			if (index != -1)
			{
				int startIndex = pRouteInfo->m_pRoute->GetIndexOffset(pRouteInfo->m_index, -6);
				int end = pRouteInfo->m_pRoute->GetIndexOffset(pRouteInfo->m_index, 6);

				DataState &myDataState = m_pIData->GetState();
				VehicleState *pVehicleState = myDataState.m_pIVehicleState->GetState();
				assert(pVehicleState);
				m_validPosition = pVehicleState->GetPosition();

				

				// height data is a bit rubbish at the moment.......
				
				pRouteInfo->m_pRoute->GetPositionFromLine(AITrackDataLines::TRACK_DATA_LINE_RACING, startIndex , m_validSplinePosition);
				
				if (end != -1)
					pRouteInfo->m_pRoute->GetDirection(end , m_validSplineDirection);
				else
					pRouteInfo->m_pRoute->GetDirection(startIndex , m_validSplineDirection);

				//m_validPosition = m_validSplineDirection;
				//m_validPosition.Y(pVehicleState->GetPosition().Y());
				
			}
			
		}
		
	}

	


}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


