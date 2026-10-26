#include "stdafx.h"
#include <avoidance/cavoidance.h>
#include <data/idata.h>
#include <messages/RouteChange.h>



#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#pragma message("Cavoidance OPTIMISE_OFF is enabled")
#endif
#endif

namespace airacingtech
{

	CAvoidance::CAvoidance() 
	{
		m_returnToLine = false;
		m_returnToLineSeconds = 10.0f;
		m_TargetOffset = 0.0f;
		m_CurrentTargetOffset = 0.0f;
		m_pIGameServerTime = 0;
		m_pIData = 0;
		m_startRouteChangeTime = 0.0f;
		m_RandomOffsetTime = airacingtech::RandomInRange(20.0f, 40.0f);
		m_returnToOffset = 0.0f;
	}

	CAvoidance::~CAvoidance()
	{
	}

	void CAvoidance::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_AVOIDANCE, CID_AVOIDANCE);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_AVOIDANCE, CAvoidance::CreateMe, CAvoidance::DestroyMe, CHash("Avoidance"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_AVOIDANCE, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_AVOIDANCE, MT_ADD_AI_DRIVER_TO_LIST);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_AVOIDANCE, MT_QUICK_RESTART);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_AVOIDANCE, MT_CHANGED_ROUTE);
	}

	IComponent *CAvoidance::CreateMe()
	{
		return new CAvoidance;
	}

	bool CAvoidance::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CAvoidance::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CAvoidance::Deinit(void)
	{
		m_AIlist.Clear();
	}

	EMessageResult CAvoidance::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;
		switch (msg.mType)
		{

		case MT_CHANGED_ROUTE:
			{
				msgRet =  MR_TRUE;

				RouteChangeMSG *pData = static_cast<RouteChangeMSG *>(msg.mpData);
				if (pData)
				{
					ChangedRoute(pData);
				}
			}
			break;

		case MT_QUICK_RESTART:
			{
				
			}
			break;


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
				if (Myid != id)
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


	EComponentTypeId CAvoidance::GetComponentTypeId(void)
	{
		return CID_AVOIDANCE;
	}

	

	void CAvoidance::CachedInterfaces()
	{
		// cache all the interfaces needed for the ai...
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));
	
		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

	
	}

	
	void CAvoidance::ChangedRoute(RouteChangeMSG *pData)
	{
		m_TargetOffset = pData->m_offset;
		m_CurrentTargetOffset = pData->m_offset;
		//m_returnToOffset = m_TargetOffset;

		if (m_pIGameServerTime)
			m_startRouteChangeTime = m_pIGameServerTime->GetTime();

	}
	

	void CAvoidance::PreUpdate()
	{

#ifdef AI_DEBUG
		CObjectId Myid = GetObjectId();
#endif

		CObjectId id = GetObjectId();

		if (m_pIGameServerTime && m_pIData)
		{
			if (m_pIGameServerTime->GetTime() > m_startRouteChangeTime + 3.0f)
			{
				m_startRouteChangeTime = 0.0f;

				// randomise the frequency of the the change, then randomise the offset value
					
				float off = 0.0f;

				ISplineTracker	*pSplineTrackerMy = m_pIData->GetSplineTracker();
				if (pSplineTrackerMy)
				{
					const RouteInfo *pRouteInfoMy = pSplineTrackerMy->GetCurrentRoute();
					if (pRouteInfoMy)
					{
						off = pRouteInfoMy->m_offset;
					}
				}

				if (m_pIGameServerTime->GetTime() > m_RandomOffsetTime)
				{
					float rndOffT = airacingtech::RandomInRange (20.0f, 40.0f);
					m_RandomOffsetTime = rndOffT + m_pIGameServerTime->GetTime();	
					
					float offSet1 = airacingtech::RandomInRange (0.05f, 0.5f);
					float offSet = airacingtech::RandomInRange (0.0f, offSet1);

					float minRnd = Maths::Max(-1.0f, off - offSet);
					float maxRnd = Maths::Min(1.0f, off + offSet);

					offSet = airacingtech::RandomInRange (minRnd, maxRnd);

					m_returnToOffset = offSet;
									
				}

			}

			
		}
		

		if (m_pIData)
		{

			// NOTES :	We could potential run this at a lower frequency if necessary......
			//			We can also move this out into a different system, to process pairs of cars..

			// clear the data out............
			m_Infront.Reset();
			m_InfrontLeft.Reset();
			m_InfrontRight.Reset();
			m_AlongsideLeft.Reset();
			m_AlongsideRight.Reset();
			m_Behind.Reset();
			m_InfrontDirectly.Reset();

			// get required interfaces.........
			IVehicleState	*pIVehicleStateMy = m_pIData->GetVehicleState();
			assert(pIVehicleStateMy);
			
			VehicleState	*pVehicleStateMy = pIVehicleStateMy->GetState();
			assert(pVehicleStateMy);
			
			IParamsAI		*pIParamsMy = m_pIData->GetParams();
			assert(pIParamsMy);

			ISplineTracker	*pSplineTrackerMy = m_pIData->GetSplineTracker();
			assert(pSplineTrackerMy);

			const RouteInfo *pRouteInfoMy = pSplineTrackerMy->GetCurrentRoute();

			IVehicleLogic *pVehicleLogicMy = m_pIData->GetVehicleLogic();
			LogicState *pLogicStateMy = 0;
			if (pVehicleLogicMy)
			{
				pLogicStateMy = pVehicleLogicMy->GetLogicState();
			}

			bool useSplineFollower = false;
			ISplineFollower *pISplineFollowerMy = m_pIData->GetSplineFollower();
			if (pISplineFollowerMy)
			{	
				useSplineFollower = false;//true; 
			}

			Maths::HVec3 MyzAxis = pVehicleStateMy->GetZaxis();
			Maths::HVec3 MyxAxis = pVehicleStateMy->GetXaxis();
			Maths::HVec3 MyPosition = pVehicleStateMy->GetPosition();
			bool isPlayer = pVehicleStateMy->GetPlayer();

			

			if (pRouteInfoMy)
			{
				float ld,rd;
				ld = m_leftDistance = Maths::Abs(pRouteInfoMy->m_pRoute->GetDistanceUsingOffsetParamAndLine(pRouteInfoMy->m_offset, AITrackDataLines::TRACK_DATA_LINE_LEFT_5, pRouteInfoMy->m_index));
				rd = m_rightDistance = Maths::Abs(pRouteInfoMy->m_pRoute->GetDistanceUsingOffsetParamAndLine(pRouteInfoMy->m_offset, AITrackDataLines::TRACK_DATA_LINE_RIGHT_5, pRouteInfoMy->m_index));
				float splineXDist = 0.0f;
				if (useSplineFollower)
				{
					splineXDist = pRouteInfoMy->m_pRoute->GetDistanceUsingOffsetParamAndLine(pISplineFollowerMy->GetOffset(), AITrackDataLines::TRACK_DATA_LINE_RACING, pRouteInfoMy->m_index);
				}
				else
				{
					splineXDist = pRouteInfoMy->m_pRoute->GetDistanceUsingOffsetParamAndLine(pRouteInfoMy->m_offset, AITrackDataLines::TRACK_DATA_LINE_RACING, pRouteInfoMy->m_index);
				}
				m_actualSplineXDist = splineXDist;

				int myLane = pSplineTrackerMy->GetCurrentLane();
			

				for(int i=0; i<(int)m_AIlist.Size();i++)
				{
					AvoidCar	avoidV;	

					// setup the interfaces to get the data
					avoidV.m_pIData = m_AIlist[i];
					avoidV.m_pIVehicleState = avoidV.m_pIData->GetVehicleState();
					avoidV.m_pVehicleState = avoidV.m_pIVehicleState->GetState();
					avoidV.m_pSplineTracker = avoidV.m_pIData->GetSplineTracker();
					avoidV.m_pISplineFollower = avoidV.m_pIData->GetSplineFollower();

					int routeIndex = pRouteInfoMy->m_RouteIndex;
					bool inRoute = avoidV.m_pSplineTracker->IsInRoute(routeIndex);
					if (inRoute == true)
					{
						// on the same route, it is now a concern.....
						avoidV.m_RouteIndex = routeIndex;	
						avoidV.m_pRouteInfo = avoidV.m_pSplineTracker->GetRouteInfo(routeIndex);
						assert(avoidV.m_pRouteInfo);

						
						float pos = airacingtech::DistanceInfrontBehindF(avoidV.m_pRouteInfo->m_pos,pRouteInfoMy->m_pos);
						avoidV.m_splineZDist = pos * pRouteInfoMy->m_pRoute->GetRouteLength();
							
						
						// lets cull before doing more
						if (avoidV.m_splineZDist > -50.0f && avoidV.m_splineZDist < 75.0f ) // 100.0f
						{

							// work out the rough distances, x and z
							float splineDistX = 0.0f;
							if (useSplineFollower)
							{
								splineDistX = avoidV.m_pRouteInfo->m_pRoute->GetDistanceUsingOffsetParamAndLine(avoidV.m_pISplineFollower->GetOffset(), AITrackDataLines::TRACK_DATA_LINE_RACING, avoidV.m_pRouteInfo->m_index);
							}
							else
							{
								splineDistX = avoidV.m_pRouteInfo->m_pRoute->GetDistanceUsingOffsetParamAndLine(avoidV.m_pRouteInfo->m_offset, AITrackDataLines::TRACK_DATA_LINE_RACING, avoidV.m_pRouteInfo->m_index);
							}

							avoidV.m_splineXDist = splineDistX - splineXDist;
							avoidV.m_actualSplineXDist = splineDistX;

							avoidV.m_leftDistance = Maths::Abs(avoidV.m_pRouteInfo->m_pRoute->GetDistanceUsingOffsetParamAndLine(avoidV.m_pRouteInfo->m_offset, AITrackDataLines::TRACK_DATA_LINE_LEFT_5, avoidV.m_pRouteInfo->m_index));
							avoidV.m_rightDistance = Maths::Abs(avoidV.m_pRouteInfo->m_pRoute->GetDistanceUsingOffsetParamAndLine(avoidV.m_pRouteInfo->m_offset, AITrackDataLines::TRACK_DATA_LINE_RIGHT_5, avoidV.m_pRouteInfo->m_index));


							Maths::HVec3 targetDirection = (avoidV.m_pVehicleState->GetPosition() - MyPosition);					
							avoidV.m_relativeSpeed = pVehicleStateMy->GetSpeed() - avoidV.m_pVehicleState->GetSpeed();	
							avoidV.m_worldZDist	= (MyzAxis | targetDirection);
							avoidV.m_worldXDist	= (MyxAxis | targetDirection);	// was -ve
							avoidV.m_splinePredition = (avoidV.m_splineZDist ) / avoidV.m_relativeSpeed; 

							// vehicle dimensions.......
							avoidV.m_vehicleLength = avoidV.m_pVehicleState->GetVehicleSize().Z() * 0.5f;
							avoidV.m_vehicleWidth = avoidV.m_pVehicleState->GetVehicleSize().X() * 0.5f;


							if (avoidV.m_splineZDist > 0.0f)
							{
								// infront possibly........

								// TODO : Use the world zDist 

								float viewAngle = DEGTORAD(30); //25 // 20
								Maths::HVec3 direction = avoidV.m_pVehicleState->GetPosition() - MyPosition;
								direction.Normalise();
								float angle = acosf(MyzAxis | direction);

								if (Maths::Abs(angle) < viewAngle)
								{
									avoidV.m_AheadBehindAlongside = AvoidCar::Ahead;
								}
								else
								{
									if (avoidV.m_splineZDist < 7.0f)
									{
										// alongside
										avoidV.m_AheadBehindAlongside = AvoidCar::Alongside;
									}
									else
									{
										// infront
										avoidV.m_AheadBehindAlongside = AvoidCar::Ahead;
									}
								}		
							}
							else
							{
								float viewAngle = DEGTORAD(155); // 20
								Maths::HVec3 direction = avoidV.m_pVehicleState->GetPosition() - MyPosition;
								direction.Normalise();
								float angle = acosf(MyzAxis | direction);

								if (Maths::Abs(angle) > viewAngle)
								{
									avoidV.m_AheadBehindAlongside = AvoidCar::Behind;
								}
								else
								{
									if (avoidV.m_splineZDist > -6.0f)
									{
										// alongside
										avoidV.m_AheadBehindAlongside = AvoidCar::Alongside;
									}
									else
									{
										avoidV.m_AheadBehindAlongside = AvoidCar::Behind;
									}
								}
							}


							switch(avoidV.m_AheadBehindAlongside)
							{
								case AvoidCar::Ahead:
									{
										SetAhead(avoidV);
									}
									break;

								case AvoidCar::Alongside:
									{
										SetAlongside(avoidV);
									}
									break;

								case AvoidCar::Behind:
									{
										SetBehind(avoidV);
									}
									break;

								default:
									break;

							}	

						}

					}
					else
					{
						// ignore stuff.......
					}
				}
			}
		}			
	}

	void CAvoidance::SetAhead(AvoidCar &avoidV)
	{
		if (avoidV.m_AheadBehindAlongside == AvoidCar::Ahead)
		{
			float vehicleLength = m_pIData->GetVehicleState()->GetState()->GetVehicleSize().Z() * 0.5f;
			float vehicleWidth = m_pIData->GetVehicleState()->GetState()->GetVehicleSize().X() * 0.5f;

			float dimensions = vehicleWidth + avoidV.m_vehicleWidth + 0.15f; // safety margin.....

			if (Maths::Abs(avoidV.m_worldXDist) < dimensions) // 1.5f
			{
				if (m_InfrontDirectly.IsValid())
				{
					if (avoidV.m_worldZDist < m_InfrontDirectly.m_worldZDist)
					{
						m_InfrontDirectly = avoidV;
					}
				}
				else
				{
					m_InfrontDirectly = avoidV;
				}
			}

			//const float aheadWidth = 2.0f; 
			if (Maths::Abs(avoidV.m_splineXDist) < dimensions)
			{
				// infront
				if (m_Infront.IsValid())
				{
					if (avoidV.m_splineZDist < m_Infront.m_splineZDist)
					{
						m_Infront = avoidV;
					}
				}
				else
				{
					m_Infront = avoidV;
				}
			}
			else if (avoidV.m_splineXDist < -dimensions)
			{
				// infront left
				if (m_InfrontLeft.IsValid())
				{
					if (avoidV.m_splineZDist < m_InfrontLeft.m_splineZDist)
					{
						m_InfrontLeft = avoidV;
					}
				}
				else
				{
					m_InfrontLeft = avoidV;
				}
			}
			else if (avoidV.m_splineXDist > dimensions)
			{
				// infront right
				if (m_InfrontRight.IsValid())
				{
					if (avoidV.m_splineZDist < m_InfrontRight.m_splineZDist)
					{
						m_InfrontRight = avoidV;
					}
				}
				else
				{
					m_InfrontRight = avoidV;
				}
			}

		}
	}

	void CAvoidance::SetAlongside(AvoidCar &avoidV)
	{
		if (avoidV.m_AheadBehindAlongside == AvoidCar::Alongside)
		{
			if (avoidV.m_worldXDist < 0.0f)
			{
				// left
				if (m_AlongsideLeft.IsValid())
				{
					if (avoidV.m_worldXDist	> m_AlongsideLeft.m_worldXDist)
					{
						m_AlongsideLeft = avoidV;
					}
				}
				else
				{
					m_AlongsideLeft = avoidV;
				}
			}
			else 
			{
				// right
				if (m_AlongsideRight.IsValid())
				{
					if (avoidV.m_worldXDist	< m_AlongsideRight.m_worldXDist)
					{
						m_AlongsideRight = avoidV;
					}
				}
				else
				{
					m_AlongsideRight = avoidV;
				}
			}
		}
	}

	void CAvoidance::SetBehind(AvoidCar &avoidV)
	{
		if (avoidV.m_AheadBehindAlongside == AvoidCar::Behind) 
		{
			if (m_Behind.IsValid())
			{
				if (avoidV.m_splineZDist > m_Behind.m_splineZDist)
				{
					m_Behind = avoidV;
				}
			}
			else
			{
				m_Behind = avoidV;
			}
		}

	}

	void CAvoidance::Update()
	{
#ifdef AI_DEBUG
		CObjectId Myid = GetObjectId();
		CObjectId Dataid = m_pIData->GetParams()->GetDriverData()->m_name;
		const char *pName = Dataid.GetHashName().c_str();
#endif	

		if (m_pIData)
		{
			IVehicleState	*pIVehicleStateMy = m_pIData->GetVehicleState();
			assert(pIVehicleStateMy);

			VehicleState	*pVehicleStateMy = pIVehicleStateMy->GetState();
			assert(pVehicleStateMy);


			if (pVehicleStateMy->GetPlayer() == false) // do not run for the player
			{
				m_returnToLineSeconds = 6.0f; // 2.0f
				m_returnToLine = false;

				AvoidVehicles();
				ReturnToLine();
			}
		}
	}

		

	void CAvoidance::AvoidVehicles()
	{
#ifdef AI_DEBUG
		CObjectId Myid = GetObjectId();
		CObjectId Dataid = m_pIData->GetParams()->GetDriverData()->m_name;
		const char *pName = Dataid.GetHashName().c_str();
#endif	

		IVehicleState	*pIVehicleStateMy = m_pIData->GetVehicleState();
		assert(pIVehicleStateMy);

		VehicleState	*pVehicleStateMy = pIVehicleStateMy->GetState();
		assert(pVehicleStateMy);

		// Using it for debug purposes....
		//if (pVehicleStateMy->GetPlayer() == false) // do not run for the player
		{
			// Pattern/Influence matching.....
			int situation = 0;

			if (m_InfrontLeft.IsValid())
				situation += 1;

			if (m_Infront.IsValid())
				situation += 2;

			if (m_InfrontRight.IsValid())
				situation += 4;

			if (m_AlongsideLeft.IsValid())
				situation += 8;

			if (m_AlongsideRight.IsValid())
				situation += 16;

			// ignore behind for now
			//if (m_Behind.IsValid())
			//	situation += 32;


			AvoidanceAction(situation);
		}
	}


	void CAvoidance::AvoidanceAction(int situation)
	{
#ifdef AI_DEBUG
		CObjectId Myid = GetObjectId();
		CObjectId Dataid = m_pIData->GetParams()->GetDriverData()->m_name;
		const char *pName = Dataid.GetHashName().c_str();
#endif

		switch(situation)
		{
			case 0:
				{
					// return to the racing line
					m_returnToLine = true;
				}
				break;

			case 1:
				{
					assert(m_InfrontLeft.IsValid());		// 1

					float threshold = RandomInRange(10.0f, 20.0f);
					if ((m_InfrontLeft.m_relativeSpeed > 0.0f) || (m_InfrontLeft.m_splineZDist < threshold))
					{
						AvoidInfrontLeft(&m_InfrontLeft);
					}
					else 
					{
						// return to the racing line
						m_returnToLine = true;
					}	
				}
				break;

			case 2:	
				{
					assert(m_Infront.IsValid());			// 2

					if (m_Infront.m_splineZDist < 75.0f)
					{
						AvoidInfront(&m_Infront);	
					}
					else
					{
						// return to the racing line
						m_returnToLine = true;
					}
				}
				break;

			case 3:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_Infront.IsValid());			// 2

					// we could use the more detailed prediction times.......

					if (m_InfrontLeft.m_splineZDist < m_Infront.m_splineZDist)
					{
						if (m_InfrontLeft.m_splineZDist < 16.0f)
						{
							if (m_InfrontLeft.m_splineXDist > -4.5f) // this can be changed.....
							{
								AvoidanceAction(1);
								m_Infront.m_leftConstraintSet = true;
							}
							else
							{
								m_Infront.m_leftConstraintSet = false;
							}
						}
					}
				
					// we have set the left constraint.....
					AvoidanceAction(2);
				
				}
				break;

			case 4:
				{
					assert(m_InfrontRight.IsValid());		// 4

					float threshold = RandomInRange(10.0f, 20.0f);
					if ((m_InfrontRight.m_relativeSpeed > 0.0f) || (m_InfrontRight.m_splineZDist < threshold) )
					{
						AvoidInfrontRight(&m_InfrontRight);
					}
					else
					{
						m_returnToLine = true;
					}
				}
				break;

			case 5:
				{	
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_InfrontRight.IsValid());		// 4


					if (m_InfrontLeft.m_splineZDist < 15.0f && m_InfrontRight.m_splineZDist < 15.0f)
					{
						if ((m_InfrontLeft.m_splinePredition > 0.0f) && (m_InfrontLeft.m_splinePredition < 5.0f) &&
							(m_InfrontRight.m_splinePredition > 0.0f) && (m_InfrontRight.m_splinePredition < 5.0f))
						{
							if (m_InfrontLeft.m_splinePredition < m_InfrontRight.m_splinePredition)
							{
								AvoidanceAction(1);
							}
							else
							{
								AvoidanceAction(4);
							}
						}
						else 
						{
							if ((m_InfrontLeft.m_splinePredition > 0.0f) && (m_InfrontLeft.m_splinePredition < 5.0f))
							{
								AvoidanceAction(1);
							}
							else if ((m_InfrontRight.m_splinePredition > 0.0f) && (m_InfrontRight.m_splinePredition < 5.0f))
							{
								AvoidanceAction(4);
							}
							else
							{
								if (m_InfrontLeft.m_splineZDist <= m_InfrontRight.m_splineZDist)
								{
									AvoidanceAction(1);
								}
								else
								{
									AvoidanceAction(4);
								}
							}
						}
					}
					else if (m_InfrontLeft.m_splineZDist < m_InfrontRight.m_splineZDist)
					{
						if (m_InfrontLeft.m_splineZDist < 15.0f)
						{
							AvoidanceAction(1);
						}
					}
					else if (m_InfrontLeft.m_splineZDist > m_InfrontRight.m_splineZDist)
					{
						if (m_InfrontRight.m_splineZDist < 15.0f)
						{
							AvoidanceAction(4);
						}
					}

					
				}
				break;


			case 6:
				{
					assert(m_Infront.IsValid());			// 2
					assert(m_InfrontRight.IsValid());		// 4


					if (m_InfrontRight.m_splineZDist < m_Infront.m_splineZDist)
					{
						if (m_InfrontRight.m_splineZDist < 16.0f)
						{
							if (m_InfrontRight.m_splineXDist < 4.5f) // check this
							{
								m_Infront.m_rightConstraintSet = true;
								AvoidanceAction(4);
							}
							else
							{
								m_Infront.m_rightConstraintSet = false;
							}
						}
					}

					AvoidanceAction(2);
				}
				break;

			case 7:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_Infront.IsValid());			// 2
					assert(m_InfrontRight.IsValid());		// 4


					if (m_InfrontLeft.m_splineZDist > 15.0f)
					{
						if (m_InfrontRight.m_splineZDist > 15.0f)
						{
							AvoidanceAction(2);
						}
						else
						{
							AvoidanceAction(6);
						}
					}
					else if (m_InfrontRight.m_splineZDist > 15.0f)
					{
						if (m_InfrontLeft.m_splineZDist > 15.0f)
						{
							AvoidanceAction(2);
						}
						else 
						{
							AvoidanceAction(3);
						}
					}
					else
					{
						AvoidanceAction(5);

						if (m_InfrontRight.m_splineXDist < 4.5f)
							m_Infront.m_rightConstraintSet = true;

						if (m_InfrontLeft.m_splineXDist > -4.5f)
							m_Infront.m_leftConstraintSet = true;

						AvoidanceAction(2);
					}
				}
				break;

			case 8:
				{
					assert(m_AlongsideLeft.IsValid());		// 8


					
				
					AvoidAlongsideLeft(&m_AlongsideLeft);
				}
				break;

			case 9:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_AlongsideLeft.IsValid());		// 8

					// ignore the infront left
					AvoidanceAction(8);
				}
				break;

			case 10:
				{
					assert(m_AlongsideLeft.IsValid());		// 8
					assert(m_Infront.IsValid());			// 2

					// we can overtake with a left constraint.........
					if (m_AlongsideLeft.m_splineXDist < -5.0f) // too far away
					{
						AvoidanceAction(2); // infront
					}
					else
					{
						AvoidanceAction(8); // alongside left

						// steer infront with a left contstraint
						m_Infront.m_leftConstraintSet = true;
						AvoidanceAction(2);
					}

				}
				break;

			case 11:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_Infront.IsValid());			// 2
					assert(m_AlongsideLeft.IsValid());		// 8


					if (m_AlongsideLeft.m_splineXDist > -5.0f)
					{
						AvoidanceAction(8);

						m_Infront.m_leftConstraintSet = true;
						AvoidanceAction(3);
					}
					else
					{
						AvoidanceAction(3);
					}

				}
				break;

			case 12:
				{
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideLeft.IsValid());		// 8

					if (m_AlongsideLeft.m_splineXDist < -5.0f)
					{
						AvoidanceAction(4);
					}
					else
					{
						AvoidanceAction(8);
					}
				}
				break;

			case 13:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideLeft.IsValid());		// 8

					if (m_AlongsideLeft.m_splineXDist > -5.0f)
					{
						AvoidanceAction(8);
					}
					else
					{
						AvoidanceAction(5);
					}


				}
				break;

			case 14:
				{
					assert(m_Infront.IsValid());			// 2
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideLeft.IsValid());		// 8


					if (m_AlongsideLeft.m_splineXDist < -5.0f)
					{
						AvoidanceAction(6);
					}
					else
					{
						AvoidanceAction(8);
						m_Infront.m_leftConstraintSet = true;
						AvoidanceAction(6);

					}
				}
				break;

			case 15:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_Infront.IsValid());			// 2
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideLeft.IsValid());		// 8

					if (m_AlongsideLeft.m_splineXDist < -5.0f)
					{
						AvoidanceAction(7);
					}
					else
					{
						AvoidanceAction(8);

						m_Infront.m_leftConstraintSet = true;
						AvoidanceAction(7);
					}
				}
				break;

			case 16:
				{
					assert(m_AlongsideRight.IsValid());		// 16
					AvoidAlongsideRight(&m_AlongsideRight);
				}
				break;

			case 17:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_AlongsideRight.IsValid());		// 16

					if (m_AlongsideRight.m_splineXDist < 5.0f)
					{
						AvoidanceAction(16);
					}
					else
					{
						AvoidanceAction(1);
					}
				}
				break;

			case 18:
				{
					assert(m_Infront.IsValid());			// 2
					assert(m_AlongsideRight.IsValid());		// 16

					// we can overtake with a left constraint.........
					if (m_AlongsideRight.m_splineXDist > 5.0f) // too far away
					{
						AvoidanceAction(2);
					}
					else
					{

						AvoidanceAction(16);

						// steer infront with a right contstraint
						m_Infront.m_rightConstraintSet = true;
						AvoidanceAction(2);
					}
				}
				break;

			case 19:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_Infront.IsValid());			// 2
					assert(m_AlongsideRight.IsValid());		// 16

					if (m_AlongsideRight.m_splineXDist > 5.0f)
					{
						AvoidanceAction(3);
					}
					else
					{
						AvoidanceAction(16);

						m_Infront.m_rightConstraintSet = true;
						AvoidanceAction(3);
					}

				}
				break;

			case 20:
				{
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideRight.IsValid());		// 16

					// ignore infront right

					if (m_AlongsideRight.m_splineXDist < 5.0f)
					{
						AvoidanceAction(16);
					}
					else
					{
						AvoidanceAction(4);
					}

				}
				break;

			case 21:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideRight.IsValid());		// 16

					if (m_AlongsideRight.m_splineXDist < 5.0f)
					{
						AvoidanceAction(16);
					}
					else
					{
						AvoidanceAction(5);
					}

				}
				break;

			case 22:
				{
					assert(m_Infront.IsValid());			// 2
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideRight.IsValid());		// 16

					if (m_AlongsideRight.m_splineXDist < 5.0f)
					{
						AvoidanceAction(16);

						m_Infront.m_rightConstraintSet = true;
						AvoidanceAction(6);
					}

				}
				break;

			case 23:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_Infront.IsValid());			// 2
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideRight.IsValid());		// 16

					if (m_AlongsideRight.m_splineXDist < 5.0f)
					{
						AvoidanceAction(16);

						m_Infront.m_rightConstraintSet = true;
						AvoidanceAction(7);
					}
					else
					{
						AvoidanceAction(7);
					}

				}
				break;

			case 24:
				{
					assert(m_AlongsideLeft.IsValid());		// 8
					assert(m_AlongsideRight.IsValid());		// 16

					// Nearest
					if (Maths::Abs(m_AlongsideLeft.m_splineXDist) < Maths::Abs(m_AlongsideRight.m_splineXDist))
					{
						// this might produce weaving.......
						if (m_AlongsideLeft.m_splineXDist > -6.0f)
						{
							AvoidanceAction(8);
						}				
					}
					else
					{
						if (m_AlongsideRight.m_splineXDist < 6.0f)
						{
							AvoidanceAction(16);	
						}
					}
				}
				break;

			case 25:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_AlongsideLeft.IsValid());		// 8
					assert(m_AlongsideRight.IsValid());		// 16

					AvoidanceAction(24);
				}
				break;

			case 26:
				{
					assert(m_Infront.IsValid());			// 2
					assert(m_AlongsideLeft.IsValid());		// 8
					assert(m_AlongsideRight.IsValid());		// 16


					if ((m_AlongsideLeft.m_splineXDist < -5.5f) && (m_AlongsideRight.m_splineXDist > 5.5f))
					{
						AvoidanceAction(2);	
					}
					else
					{
						AvoidanceAction(24);

						m_Infront.m_leftConstraintSet = true;
						m_Infront.m_rightConstraintSet = true;
						AvoidanceAction(2);
					}

				}
				break;

			case 27:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_Infront.IsValid());			// 2
					assert(m_AlongsideLeft.IsValid());		// 8
					assert(m_AlongsideRight.IsValid());		// 16

					AvoidanceAction(26);
				}
				break;

			case 28:
				{
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideLeft.IsValid());		// 8
					assert(m_AlongsideRight.IsValid());		// 16

					// ignoring infront right
					AvoidanceAction(24);
				}
				break;

			case 29:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideLeft.IsValid());		// 8
					assert(m_AlongsideRight.IsValid());		// 16

					// ignoring infront left and right
					AvoidanceAction(24);
				}
				break;

			case 30:
				{
					assert(m_Infront.IsValid());			// 2
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideLeft.IsValid());		// 8
					assert(m_AlongsideRight.IsValid());		// 16

					// ignoring infront right
					AvoidanceAction(26);

				}
				break;



			case 31:
				{
					assert(m_InfrontLeft.IsValid());		// 1
					assert(m_Infront.IsValid());			// 2
					assert(m_InfrontRight.IsValid());		// 4
					assert(m_AlongsideLeft.IsValid());		// 8
					assert(m_AlongsideRight.IsValid());		// 16

					// ignoring infront left and right
					AvoidanceAction(26);
				}
				break;

			// TODO : behind stuff........



			default:
				break;
		}
	}

	void CAvoidance::ReturnToLine()
	{
		//return; // Darren this is causing some wierd behaviour, too abrubt.

		if (m_returnToLine == true && m_startRouteChangeTime == 0.0f)
		{
			ISplineTracker	*pSplineTrackerMy = m_pIData->GetSplineTracker();
			if (pSplineTrackerMy)
			{
				const RouteInfo *pRouteInfoMy = pSplineTrackerMy->GetCurrentRoute();
				if (pRouteInfoMy && pRouteInfoMy->m_pRoute)
				{
					// return to the racing line......
					float dist = m_actualSplineXDist;

					ISplineFollower *pISplineFollower = m_pIData->GetSplineFollower();
					if (pISplineFollower)
					{
						float offset = m_returnToOffset;
						dist = pRouteInfoMy->m_pRoute->GetDistanceUsingOffsetParamAndLine(offset, AITrackDataLines::TRACK_DATA_LINE_RACING, pRouteInfoMy->m_index);
					
						// Darren Hack
						if (Maths::Abs(dist) < 1.0f)
						{
							m_returnToLineSeconds = 5.0f;
						}
						else if (Maths::Abs(dist) < 2.0f)
						{
							m_returnToLineSeconds = 10.0f;
						}
						else if (Maths::Abs(dist) < 3.0f)
						{
							m_returnToLineSeconds = 15.0f;
						}
						else if (Maths::Abs(dist) < 4.0f)
						{
							m_returnToLineSeconds = 20.0f;
						}
						else
						{
							m_returnToLineSeconds = 25.0f;
						}
						

						
						//m_returnToLineSeconds = 50.0f;

					}

					SetTargetOffset(m_returnToOffset, -dist, m_returnToLineSeconds);
				}
			}
		}
	}

	void CAvoidance::AvoidInfront(AvoidCar *pAvoidCar)
	{
#ifdef AI_DEBUG
		CObjectId Myid = GetObjectId();
		CObjectId Dataid = m_pIData->GetParams()->GetDriverData()->m_name;
		const char *pName = Dataid.GetHashName().c_str();
#endif
		AvoidCar *pDebugInfront = pAvoidCar;
		DataState &myDataState = m_pIData->GetState();

		ISplineTracker	*pSplineTrackerMy = m_pIData->GetSplineTracker();
		assert(pSplineTrackerMy);

		const RouteInfo *pRouteInfoMy = pSplineTrackerMy->GetCurrentRoute();
		assert(pRouteInfoMy);

		ISplineFollower *pISplineFollower = m_pIData->GetSplineFollower();
		assert(pISplineFollower);

		// This will be a very crude interpretation of the avoidance for the AI.....
		// Basically... if car infront is on the right, move left and vice-a-verse, checks for the gap, curvature is ignore,
		// Will build in curvature depending upon driver type.......favours inside/outside

		bool turnLeft = false;
		bool turnRight = false;

		float collisionTime = pAvoidCar->m_splinePredition;

		// TODO : Check this function to make sure I haven`t optimised for using 1m radius only...
		float linearPrediction = pAvoidCar->m_linearPrediction = Prediction(pAvoidCar, 5.0f, 2.0f, 2.0f);
		pAvoidCar->m_linearPrediction = linearPrediction;

		const float gap = 3.0f;

		if (linearPrediction > 0.0f && linearPrediction < 4.5f) // 3.0f
		{
			// we are potentially colliding in the future.......
			if ((pAvoidCar->m_splineXDist < 0.0f) && (pAvoidCar->m_rightDistance >= gap) /*&& (m_rightDistance >= gap)*/ && (pAvoidCar->m_rightConstraintSet == false) )
			{
				// if on the left and the right side gap is large enough, go right
				turnRight = true;
			}
			else if ((pAvoidCar->m_splineXDist > 0.0f) && (pAvoidCar->m_leftDistance >= gap) /*&& (myLeftWidthF >= gap)*/ && (pAvoidCar->m_leftConstraintSet == false) )
			{
				// if on the right and the left side gap is large enough, go left
				turnLeft = true;
			}
			else if ((Maths::Abs(pAvoidCar->m_splineXDist) < 0.5f) && (pAvoidCar->m_leftDistance >= gap) && (pAvoidCar->m_leftDistance > pAvoidCar->m_rightDistance) )
			{
				turnLeft = true;
			}
			else if ((Maths::Abs(m_Infront.m_splineXDist) < 0.5f) && (pAvoidCar->m_rightDistance >= gap) && (pAvoidCar->m_rightDistance > pAvoidCar->m_leftDistance) )
			{
				turnRight = true;
			}
			else 
			{
				int stop = 0; // what todo here ?????
			}
		}
		else
		{
			// maybe something based upon distance........
			if (pAvoidCar->m_splineZDist < 12.0f)
			{
				if (pAvoidCar->m_splinePredition > 0.0f)
					collisionTime = pAvoidCar->m_splinePredition;
				else
					collisionTime = 5.0f;

				//if (collisionTime < 0.0f)
				//	collisionTime = 10.0f;
				//else
				//	collisionTime = Maths::Min(collisionTime, 10.0f);

				if (Maths::Abs(pAvoidCar->m_worldZDist) < 5.0f && Maths::Abs(pAvoidCar->m_worldXDist) < 1.25f)
				{
					collisionTime = Maths::Min(collisionTime, 0.1f);
				}

				if ((pAvoidCar->m_splineXDist < 0.0f) && (pAvoidCar->m_rightDistance >= gap) /*&& (m_rightDistance >= gap)*/ && (pAvoidCar->m_rightConstraintSet == false) )
				{
					// if on the left and the right side gap is large enough, go right
					turnRight = true;
				}
				else if ((pAvoidCar->m_splineXDist > 0.0f) && (pAvoidCar->m_leftDistance >= gap) /*&& (myLeftWidthF >= gap)*/ && (pAvoidCar->m_leftConstraintSet == false) )
				{
					// if on the right and the left side gap is large enough, go left
					turnLeft = true;
				}
				else if ((Maths::Abs(pAvoidCar->m_splineXDist) < 0.5f) && (pAvoidCar->m_leftDistance >= gap) && (pAvoidCar->m_leftDistance > pAvoidCar->m_rightDistance) )
				{
					turnLeft = true;
				}
				else if ((Maths::Abs(pAvoidCar->m_splineXDist) < 0.5f) && (pAvoidCar->m_rightDistance >= gap) && (pAvoidCar->m_rightDistance > pAvoidCar->m_leftDistance) )
				{
					turnRight = true;
				}
				else 
				{
					// last resort.........
					
				}


			}

		}

		float sepDist = 3.0f; // for now just to force them out further...

		float myXDist = m_actualSplineXDist;


		if (turnLeft == true)
		{
			float targetDist = pAvoidCar->m_actualSplineXDist - sepDist;//(sepDist - Maths::Abs(pAvoidCar->m_splineXDist));
			float currentTargetDist = targetDist;

			// need return targetDist and a paramatised version, regardless if they are capped
			float targetoffset = 0.0f;
			pRouteInfoMy->m_pRoute->CalculateXOffsetFromXDistance(pAvoidCar->m_pRouteInfo->m_index, AITrackDataLines::TRACK_DATA_LINE_RACING , currentTargetDist, targetoffset );


			// new
			currentTargetDist = (sepDist - Maths::Abs(pAvoidCar->m_splineXDist));

			SetTargetOffset(targetoffset, currentTargetDist, collisionTime);
		}
		else if (turnRight == true)
		{
			float targetDist = pAvoidCar->m_actualSplineXDist + sepDist;//(sepDist - Maths::Abs(pAvoidCar->m_splineXDist));
			float currentTargetDist = targetDist;

			// need return targetDist and a paramatised version, regardless if they are capped
			float targetoffset = 0.0f;
			pRouteInfoMy->m_pRoute->CalculateXOffsetFromXDistance(pAvoidCar->m_pRouteInfo->m_index, AITrackDataLines::TRACK_DATA_LINE_RACING , currentTargetDist, targetoffset );

			// new
			currentTargetDist = (sepDist - Maths::Abs(pAvoidCar->m_splineXDist));

			SetTargetOffset(targetoffset, currentTargetDist, collisionTime);
		}
		else
		{
			m_returnToLine = true;
		}



#if 0
		if (linearPrediction > 0.0f && linearPrediction < 3.0f)
		{
			if (pAvoidCar->m_splineXDist > 0.0f)
			{
				// right side, move left
				turnLeft = true;
			}
			else
			{
				// left side, move right
				turnRight = true;
			}
		}
		else 
		{
			if (pAvoidCar->m_splineZDist < 16.0f)
			{

				if (linearPrediction < 0.0f)
					collisionTime = 1000.0f;
				else
					collisionTime = linearPrediction;

				if (pAvoidCar->m_splineXDist > 0.0f)
				{
					// right side, move left
					turnLeft = true;
				}
				else
				{
					// left side, move right
					turnRight = true;
				}
			}
		}

		float sepDist = 3.0f; // for now just to force them out further...
		
		float myXDist = m_actualSplineXDist;


		if (turnLeft == true && pAvoidCar->m_leftConstraintSet == false)
		{
			float targetDist = pAvoidCar->m_actualSplineXDist - sepDist;//(sepDist - Maths::Abs(pAvoidCar->m_splineXDist));
			float currentTargetDist = targetDist;

			// need return targetDist and a paramatised version, regardless if they are capped
			float targetoffset = 0.0f;
			pRouteInfoMy->m_pRoute->CalculateXOffsetFromXDistance(pAvoidCar->m_pRouteInfo->m_index, AITrackDataLines::TRACK_DATA_LINE_RACING , currentTargetDist, targetoffset );

			SetTargetOffset(targetoffset, currentTargetDist, collisionTime);
		}
		else if (turnRight == true && pAvoidCar->m_rightConstraintSet == false)
		{
			float targetDist = pAvoidCar->m_actualSplineXDist + sepDist;//(sepDist - Maths::Abs(pAvoidCar->m_splineXDist));
			float currentTargetDist = targetDist;

			// need return targetDist and a paramatised version, regardless if they are capped
			float targetoffset = 0.0f;
			pRouteInfoMy->m_pRoute->CalculateXOffsetFromXDistance(pAvoidCar->m_pRouteInfo->m_index, AITrackDataLines::TRACK_DATA_LINE_RACING , currentTargetDist, targetoffset );

			SetTargetOffset(targetoffset, currentTargetDist, collisionTime);
		}

#endif

	}

	void CAvoidance::AvoidInfrontLeft(AvoidCar *pAvoidCar)
	{
#ifdef AI_DEBUG
		CObjectId Myid = GetObjectId();
		CObjectId Dataid = m_pIData->GetParams()->GetDriverData()->m_name;
		const char *pName = Dataid.GetHashName().c_str();
#endif
		AvoidCar *pDebugInfront = pAvoidCar;
		DataState &myDataState = m_pIData->GetState();

		ISplineTracker	*pSplineTrackerMy = m_pIData->GetSplineTracker();
		assert(pSplineTrackerMy);

		const RouteInfo *pRouteInfoMy = pSplineTrackerMy->GetCurrentRoute();
		assert(pRouteInfoMy);

		ISplineFollower *pISplineFollower = m_pIData->GetSplineFollower();
		assert(pISplineFollower);

		bool turnLeft = false;
		bool turnRight = false;

		if (pAvoidCar->m_splineXDist < -3.0f)
		{
			m_returnToLine = true;
		}
		else
		{

		}
	}

	void CAvoidance::AvoidInfrontRight(AvoidCar *pAvoidCar)
	{
#ifdef AI_DEBUG
		CObjectId Myid = GetObjectId();
		CObjectId Dataid = m_pIData->GetParams()->GetDriverData()->m_name;
		const char *pName = Dataid.GetHashName().c_str();
#endif
		AvoidCar *pDebugInfront = pAvoidCar;
		DataState &myDataState = m_pIData->GetState();

		ISplineTracker	*pSplineTrackerMy = m_pIData->GetSplineTracker();
		assert(pSplineTrackerMy);

		const RouteInfo *pRouteInfoMy = pSplineTrackerMy->GetCurrentRoute();
		assert(pRouteInfoMy);

		ISplineFollower *pISplineFollower = m_pIData->GetSplineFollower();
		assert(pISplineFollower);

		// This will be a very crude interpretation of the avoidance for the AI.....
		// Basically... if car infront is on the right, move left and vice-a-verse, regardless
		// of the gap available.

		bool turnLeft = false;
		bool turnRight = false;

		if (pAvoidCar->m_splineXDist > 3.0f)
		{
			m_returnToLine = true;
		}
		else
		{

		}
	}

	void CAvoidance::AvoidAlongsideLeft(AvoidCar *pAvoidCar)
	{
#ifdef AI_DEBUG
		CObjectId Myid = GetObjectId();
		CObjectId Dataid = m_pIData->GetParams()->GetDriverData()->m_name;
		const char *pName = Dataid.GetHashName().c_str();
#endif

		DataState &myDataState = m_pIData->GetState();

		ISplineTracker	*pSplineTrackerMy = m_pIData->GetSplineTracker();
		assert(pSplineTrackerMy);

		const RouteInfo *pRouteInfoMy = pSplineTrackerMy->GetCurrentRoute();
		assert(pRouteInfoMy);

		ISplineFollower *pISplineFollower = m_pIData->GetSplineFollower();
		assert(pISplineFollower);

		// This will be a very crude interpretation of the avoidance for the AI.....
		// Basically... if car infront is on the right, move left and vice-a-verse, regardless
		// of the gap available.

		bool turnLeft = false;
		bool turnRight = false;

		// could alter this based upon some other attribute or value.....
		float sepDist = 4.5f; // 3.0f 
		if (Maths::Abs(pAvoidCar->m_splineXDist) < sepDist)
		{
			// turn right................
			float targetDist = pAvoidCar->m_actualSplineXDist + sepDist;//(sepDist - Maths::Abs(pAvoidCar->m_splineXDist));
			float currentTargetDist = targetDist;

			// need return targetDist and a paramatised version, regardless if they are capped
			float targetoffset = 0.0f;
			pRouteInfoMy->m_pRoute->CalculateXOffsetFromXDistance(pAvoidCar->m_pRouteInfo->m_index, AITrackDataLines::TRACK_DATA_LINE_RACING , currentTargetDist, targetoffset );


			// new
			currentTargetDist = (sepDist - Maths::Abs(pAvoidCar->m_splineXDist));

			SetTargetOffset(targetoffset, currentTargetDist, 0.75f);
		}
		else
		{
			m_returnToLine = true;
		}


	}

	void CAvoidance::AvoidAlongsideRight(AvoidCar *pAvoidCar)
	{
#ifdef AI_DEBUG
		CObjectId Myid = GetObjectId();
		CObjectId Dataid = m_pIData->GetParams()->GetDriverData()->m_name;
		const char *pName = Dataid.GetHashName().c_str();
#endif

		DataState &myDataState = m_pIData->GetState();

		ISplineTracker	*pSplineTrackerMy = m_pIData->GetSplineTracker();
		assert(pSplineTrackerMy);

		const RouteInfo *pRouteInfoMy = pSplineTrackerMy->GetCurrentRoute();
		assert(pRouteInfoMy);

		ISplineFollower *pISplineFollower = m_pIData->GetSplineFollower();
		assert(pISplineFollower);

		// This will be a very crude interpretation of the avoidance for the AI.....
		// Basically... if car infront is on the right, move left and vice-a-verse, regardless
		// of the gap available.

		bool turnLeft = false;
		bool turnRight = false;

		
		float sepDist = 4.5f;//3.0f; // 4.5f 
		if (Maths::Abs(pAvoidCar->m_splineXDist) < sepDist)
		{
			// turn left.....
			float targetDist = pAvoidCar->m_actualSplineXDist - sepDist;//(sepDist - Maths::Abs(pAvoidCar->m_splineXDist));
			float currentTargetDist = targetDist;

			// need return targetDist and a paramatised version, regardless if they are capped
			float targetoffset = 0.0f;
			pRouteInfoMy->m_pRoute->CalculateXOffsetFromXDistance(pAvoidCar->m_pRouteInfo->m_index, AITrackDataLines::TRACK_DATA_LINE_RACING , currentTargetDist, targetoffset );


			// new
			currentTargetDist = (sepDist - Maths::Abs(pAvoidCar->m_splineXDist));


			SetTargetOffset(targetoffset, currentTargetDist, 0.75f);
		}
		else
		{
			m_returnToLine = true;
		}



	}


	float CAvoidance::Prediction(AvoidCar *pAvoid, float tTime, float radiusA, float radiusB)
	{
		// TODO FIX this up......
		// NOTE : for now assume 2d

		float u0,u1;

		IVehicleState *pMyVehicleState = m_pIData->GetVehicleState();


		float speedA = pMyVehicleState->GetState()->GetSpeed();
		float speedB = pAvoid->m_pVehicleState->GetSpeed();

		Maths::HVec3 velocityA = pMyVehicleState->GetState()->GetZaxis() * (tTime * speedA); 
		Maths::HVec3 velocityB = pAvoid->m_pVehicleState->GetZaxis() * (tTime * speedB); 

		velocityA.ZeroY();
		velocityB.ZeroY();

		Maths::HVec3 APos = pMyVehicleState->GetState()->GetPosition(); 
		Maths::HVec3 BPos =  pAvoid->m_pVehicleState->GetPosition();

		APos.ZeroY();
		BPos.ZeroY();

		Maths::HVec3 AfuturePos = APos + velocityA; 
		Maths::HVec3 BfuturePos = BPos + velocityB; 

		bool value = SphereSphereSweep(radiusA, APos, AfuturePos, radiusB, BPos, BfuturePos, u0,u1, tTime);

		if (u0 < 0.0f)
		{
			u0 = 1000.0f;
		}

		return u0;
	}




	void CAvoidance::SetTargetOffset(float targetoffset, float currentTargetDist, float secs)
	{
		ISplineFollower *pISplineFollower = m_pIData->GetSplineFollower();
		ISplineTracker	*pSplineTrackerMy = m_pIData->GetSplineTracker();
		
		if (pISplineFollower && pSplineTrackerMy)
		{
			const RouteInfo *pRouteInfoMy = pSplineTrackerMy->GetCurrentRoute();
			if (pRouteInfoMy && pRouteInfoMy->m_pRoute)
			{
				float lateralMPS = (Maths::Abs(currentTargetDist) / (secs * secs));// / 30.0f ; 
				lateralMPS = Maths::Clamp(lateralMPS, 0.0001f, 1.0f);

				m_TargetOffset = targetoffset;

				m_CurrentTargetOffset = pISplineFollower->GetOffset();
				float coffset = m_CurrentTargetOffset;
				float to = m_TargetOffset;
				float diff = Maths::Abs(m_CurrentTargetOffset - m_TargetOffset);


				if (m_CurrentTargetOffset < m_TargetOffset)
				{
					coffset += (lateralMPS * diff);
					if (coffset >= m_TargetOffset)
						coffset = m_TargetOffset;
				}
				else
				{
					coffset -= (lateralMPS * diff);
					if (coffset <= m_TargetOffset)
						coffset = m_TargetOffset;
				}

				m_CurrentTargetOffset = coffset;
				pISplineFollower->SetOffset(coffset);

				m_returnToLine = false;
			}		
		}
		
	}

}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

