#include "stdafx.h"
#include <racedirector/cracedirector.h>
#include <data/idata.h>
#include <algorithm>
#include <messages/EnterDynJunction.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

static Debug::Logger &g_DebugLoggerRaceDirector = Debug::GetLogger("oldtech.ai.racedirector");


namespace airacingtech
{

	CRaceDirector::CRaceDirector() 
	{
		//m_Frequency = 12;
		//m_FrequencyCounter = 12;
		m_pTrackData = 0;
		m_pIGameServerTime = 0;
		m_JunctionLastTime = 0.0f;
		m_TrackLogicLastTime = 0.0f;
		
	}

	CRaceDirector::~CRaceDirector()
	{
		m_AIEventBalancing.Destroy(); // tobe on the safe side
	}

	void CRaceDirector::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_RACEDIRECTOR, CID_RACEDIRECTOR);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_RACEDIRECTOR, CRaceDirector::CreateMe, CRaceDirector::DestroyMe, CHash("RaceDirector"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RACEDIRECTOR, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RACEDIRECTOR, MT_ADD_AI_DRIVER_TO_LIST);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RACEDIRECTOR, MT_PRE_UPDATE_AI_SYSTEMS);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RACEDIRECTOR, MT_UPDATE_AI_SYSTEMS);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RACEDIRECTOR, MT_POST_UPDATE_AI_SYSTEMS);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RACEDIRECTOR, MT_QUICK_RESTART);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RACEDIRECTOR, MT_INIT_RACE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RACEDIRECTOR, MT_ENTERING_DYNAMIC_JUNCTION_ZONE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RACEDIRECTOR, MT_SET_ROUTE_ACTIVE_INACTIVE);
	}

	IComponent *CRaceDirector::CreateMe()
	{
		return new CRaceDirector;
	}

	bool CRaceDirector::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CRaceDirector::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CRaceDirector::Deinit(void)
	{
		m_AIlist.clear();
		m_AIlistTrackPosition.clear();
		
		m_AIEventBalancing.Destroy();
	}

	EMessageResult CRaceDirector::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;
		switch (msg.mType)
		{

		case MT_QUICK_RESTART:
			{
				QuickRestart();
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
				IData *pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));
				if (pIData)
				{
					m_AIlist.push_back(pIData);
					m_AIlistTrackPosition.push_back(pIData);
				}
				
			}
			break;

		case MT_INIT_RACE:
			{
				InitRace();
			}
			break;

		
		case MT_PRE_UPDATE_AI_SYSTEMS:
			{
				PreUpdate();
			}
			break;

		case MT_UPDATE_AI_SYSTEMS:
			{
				Update();
			}
			break;

		case MT_POST_UPDATE_AI_SYSTEMS:
			{
				PostUpdate();
			}
			break;
		
		case MT_START_AI_RACE:
			{
				
				
			}
			break;

		case MT_ENTERING_DYNAMIC_JUNCTION_ZONE:
			{
				msgRet =  MR_TRUE;

				EnterDynamicJunctionMSG *pJunctionMessage = static_cast<EnterDynamicJunctionMSG *>(msg.mpData);
				assert(pJunctionMessage);
				
				HandleJunctionMessages(pJunctionMessage);
			}
			break;

		case MT_SET_ROUTE_ACTIVE_INACTIVE:
			{
				msgRet =  MR_TRUE;

				SetRouteMSG *pActivateDeActivateMessage = static_cast<SetRouteMSG *>(msg.mpData);
				assert(pActivateDeActivateMessage);
				
				HandleSetRoute(pActivateDeActivateMessage);
						
			}
			break;

		
	

		default:
			break;
		}

		return msgRet;
	}

	void CRaceDirector::QuickRestart()
	{
	}


	EComponentTypeId CRaceDirector::GetComponentTypeId(void)
	{
		return CID_RACEDIRECTOR;
	}

	void CRaceDirector::CachedInterfaces()
	{
		// cache all the interfaces needed for the ai...
		CObjectId id = GetObjectId();

		CObjectId trackdataSystem("TrackDataSystem");
		m_pTrackData = static_cast<ITrackDataAI *>(GlobalsComponent::GetObjectManager().QueryInterface(trackdataSystem, IID_TRACK_DATA_AI));

		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

	}

	void CRaceDirector::HandleSetRoute(SetRouteMSG *pMessage)
	{
		if ( m_pTrackData )
		{
			AITrackData *pAITrackData = m_pTrackData->GetTrackData();
			
			// A couple of problems, what if we are setting a junction that has a rule
			// should we override the rule, once a message is recieved.
			// Add extra data, ie override indefinatly, override for x seconds, will get very complex.

			// In addition, should we override junctions only, or routes also ?

			AITrackDataRoutes *pRoute = pAITrackData->GetRoute(pMessage->m_routeID);
			if (pRoute)
			{
				pRoute->SetActive(pMessage->m_active);
			}
		}
		
	}


	void CRaceDirector::HandleJunctionMessages(EnterDynamicJunctionMSG *pJunctionMessage)
	{
		// Rule 9 for now
		if (pJunctionMessage->m_pRoute->IsDynamicJunction())
		{
			if (pJunctionMessage->m_pRoute->m_pJunctionRule)
			{
				if (pJunctionMessage->m_pRoute->m_pJunctionRule->m_ruleNO == 9)
				{
					JunctionRule9 *pRule9 = static_cast<JunctionRule9*>(pJunctionMessage->m_pRoute->m_pJunctionRule);
					pRule9->m_CarCount++;
				}
			}
		}
	}

	void CRaceDirector::InitRace()
	{
		CalculateAllRouteSpeeds();
	}

	void CRaceDirector::PreUpdate()
	{
		// make sure this is on a frequency of about 1 in every 5 seconds

		if ( m_pTrackData && m_pIGameServerTime && m_pIGameServerTime->GetTime() > m_JunctionLastTime + 0.5f)
		{
			UpdateJunctionRules();
			m_JunctionLastTime = m_pIGameServerTime->GetTime();
		}
		

		/*for (int i=0; i < static_cast<int>(m_AIlist.size()); i++)
		{
			IData *pIData = m_AIlist[i];	

			// do stuff......
			


		}
		*/
	}

	void CRaceDirector::Update()
	{	

		UpdateTrackLogic();

	}	

	void CRaceDirector::PostUpdate()
	{
		/*
		for (int i=0; i < static_cast<int>(m_AIlist.size()); i++)
		{
			IData *pIData = m_AIlist[i];	

			// do stuff......
		}
		*/
	}	

	void CRaceDirector::UpdateTrackLogic()
	{
		/*
		// crude method for  testing if we should update track logic
		bool valid = false;
		for (int i=0; i < static_cast<int>(m_AIlist.size()); i++)
		{
			IData *pIData = m_AIlist[i];	
			if (pIData)
			{
				if (pIData->GetVehicleLogic())
				{
					valid = true;
					break;
				}
			}
		
		}

		if (valid)
		{
			SortByLogic();
		}
		*/



		if (m_pIGameServerTime->GetTime() > m_TrackLogicLastTime)
		{
			SortByLogic();
			m_TrackLogicLastTime = m_pIGameServerTime->GetTime() + 0.25f; // we can have this on a higher time
		}	
		
		SortByTrackPosition();

	}

	void CRaceDirector::SortByTrackPosition()
	{
		sort( m_AIlistTrackPosition.begin(), m_AIlistTrackPosition.end(), SortCarsByTrackPosition() );

		for (int i=0; i < static_cast<int>(m_AIlistTrackPosition.size()); i++)
		{
			IData *pIData = m_AIlistTrackPosition[i];	
			IVehicleLogic *pVehicleLogic = pIData->GetVehicleLogic();
			LogicState *pLogicState = pVehicleLogic->GetLogicState();
			pLogicState->m_positionIndex = i;
		}

	}

	void CRaceDirector::SortByLogic()
	{
		sort( m_AIlist.begin(), m_AIlist.end(), SortCarsByLogic() );

		int RacePos = 1;

		for (int i=0; i < static_cast<int>(m_AIlist.size()); i++)
		{
			IData *pIData = m_AIlist[i];
		
#if 0
			IVehicleLogic *pVehicleLogic = pIData->GetVehicleLogic();
			LogicState *pLogicState = pVehicleLogic->GetLogicState();
			pLogicState->m_Rank = i+1;	
#else
			// to include the eliminated flag in certain game modes.....
			IVehicleLogic *pVehicleLogic = pIData->GetVehicleLogic();
			LogicState *pLogicState = pVehicleLogic->GetLogicState();
				
			if (pIData->GetState().m_DoNotUseInPositionCalculation == true)
			{
				pLogicState->m_Rank = 0;///
			}
			else
			{
				pLogicState->m_Rank = RacePos;
				RacePos++;
			}
#endif
		}
	}


	void CRaceDirector::UpdateJunctionRules()
	{
		

		// get player
		int playerIndex = -1;
		for (int i=0; i < static_cast<int>(m_AIlist.size()); i++)
		{
			IData *pIData = m_AIlist[i];
		
			bool isPlayer = pIData->GetVehicleState()->GetState()->GetPlayer();
			if (isPlayer)
			{
				playerIndex = i;	
			}
		}

		AITrackData *pAITrackData = m_pTrackData->GetTrackData();

		int noRoutes = pAITrackData->GetNumRoutes();

		for(int i=0; i<noRoutes; i++)
		{
			AITrackDataRoutes *pRoute = pAITrackData->GetRoute(i);

			// only interested in junctions........only dynamic ones really
			if ( pRoute->IsJunction() && pRoute->IsDynamicJunction() )
			{
				if (pRoute->m_pJunctionRule)
				{
					switch(pRoute->m_pJunctionRule->m_ruleNO)
					{
					case 2:
						{
							// rule : Once the player enters this rule route, the junction stays open/closed indefinately
							JunctionRule2 *pRule2 = static_cast<JunctionRule2*>(pRoute->m_pJunctionRule);

							bool anyOneIn = false;

							// loop through human players.......
							for (int j=0; j < static_cast<int>(m_AIlist.size()); j++)
							{
								IData *pIData = m_AIlist[j];
								bool isPlayer = pIData->GetVehicleState()->GetState()->GetPlayer();
								if (isPlayer)
								{
									// loop through all the routes.....
									airacingtech::ISplineTracker *pSplineTracker =  pIData->GetSplineTracker();
									if (pSplineTracker)
									{
										const Btl::Vector<RouteInfo> &routeInfo = pSplineTracker->GetRouteInfo();
									
										for(int k=0; k<routeInfo.Size(); k++)
										{
											const airacingtech::RouteInfo *pRouteInfo = &routeInfo[k];
											
											if (pRouteInfo->m_pRoute && pRouteInfo->m_pRoute->m_Name == pRule2->m_routeName)
											{
												if (pRouteInfo->m_in)
												{
													anyOneIn = true;
												}
											}
											
										}
									
									}
								}
							}

							if (anyOneIn == true)
							{
								// set the route to the required rule state....
								if (pRule2->m_state == JunctionRule::JUNCTION_OPEN)
									pRoute->SetActive(true);
								else
									pRoute->SetActive(false);
							}
						}
						break;


					case 3:
						{
							// rule : Once the player enters this rule route, the junction stays open/closed until the player leaves
							JunctionRule3 *pRule3 = static_cast<JunctionRule3*>(pRoute->m_pJunctionRule);

							bool anyOneIn = false;

							// loop through human players.......
							for (int j=0; j < static_cast<int>(m_AIlist.size()); j++)
							{
								IData *pIData = m_AIlist[j];
								bool isPlayer = pIData->GetVehicleState()->GetState()->GetPlayer();
								if (isPlayer)
								{
									// loop through all the routes.....
									airacingtech::ISplineTracker *pSplineTracker =  pIData->GetSplineTracker();
									if (pSplineTracker)
									{
										const Btl::Vector<RouteInfo> &routeInfo = pSplineTracker->GetRouteInfo();

										for(int k=0; k<routeInfo.Size(); k++)
										{
											const airacingtech::RouteInfo *pRouteInfo = &routeInfo[k];

											if (pRouteInfo->m_pRoute && pRouteInfo->m_pRoute->m_Name == pRule3->m_routeName)
											{
												if (pRouteInfo->m_in)
												{
													anyOneIn = true;
												}
											}

										}

									}
								}
							}

							if (anyOneIn == true)
							{
								// set the route to the required rule state....
								if (pRule3->m_state == JunctionRule::JUNCTION_OPEN)
									pRoute->SetActive(true);
								else
									pRoute->SetActive(false);
							}
							else
							{
								// set the route to the opposite rule state....
								if (pRule3->m_state == JunctionRule::JUNCTION_OPEN)
									pRoute->SetActive(false);
								else
									pRoute->SetActive(true);
							}	
						}
						break;

					case 4:
						{
							// TODO : If needed......
						}
						break;


					case 5:
						{
							// Toggle
							JunctionRule5 *pRule5 = static_cast<JunctionRule5*>(pRoute->m_pJunctionRule);

							if (pRule5->m_currentState == JunctionRule::JUNCTION_NOT_INTIALISED)
							{
								pRule5->m_currentState = pRule5->m_state[0];
								pRule5->m_helperState = 0;
								pRule5->m_lastTime = m_pIGameServerTime->GetTime();
							}
							else if (pRule5->m_currentState == JunctionRule::JUNCTION_OPEN)
							{
								pRoute->SetActive(true);

								if (pRule5->m_helperState == 0)
								{
									if (m_pIGameServerTime->GetTime() > pRule5->m_lastTime + pRule5->m_t[0])
									{
										pRule5->m_currentState = pRule5->m_state[1];
										pRule5->m_lastTime = m_pIGameServerTime->GetTime();
										pRule5->m_helperState = 1;
									}
								}
								else
								{
									if (m_pIGameServerTime->GetTime() > pRule5->m_lastTime + pRule5->m_t[1])
									{
										pRule5->m_currentState = pRule5->m_state[0];
										pRule5->m_lastTime = m_pIGameServerTime->GetTime();
										pRule5->m_helperState = 0;
									}
								}
							}
							else if (pRule5->m_currentState == JunctionRule::JUNCTION_CLOSE)
							{
								pRoute->SetActive(false);

								if (pRule5->m_helperState == 0)
								{
									if (m_pIGameServerTime->GetTime() > pRule5->m_lastTime + pRule5->m_t[0])
									{
										pRule5->m_currentState = pRule5->m_state[1];
										pRule5->m_lastTime = m_pIGameServerTime->GetTime();
										pRule5->m_helperState = 1;
									}
								}
								else
								{
									if (m_pIGameServerTime->GetTime() > pRule5->m_lastTime + pRule5->m_t[1])
									{
										pRule5->m_currentState = pRule5->m_state[0];
										pRule5->m_lastTime = m_pIGameServerTime->GetTime();
										pRule5->m_helperState = 0;
									}
								}
							}

						}
						break;

					case 6:
						{
							// open when player is only on lap X
								
							JunctionRule6 *pRule6 = static_cast<JunctionRule6*>(pRoute->m_pJunctionRule);


							// get first human player............

							IData *pIHumanData = 0;
							for (int j=0; j < static_cast<int>(m_AIlist.size()); j++)
							{
								IData *pIData = m_AIlist[j];
								bool isPlayer = pIData->GetVehicleState()->GetState()->GetPlayer();
								if (isPlayer)
								{
									pIHumanData = m_AIlist[j];
									break;
								}
							}

							if (pIHumanData)
							{
								if ( pIHumanData->GetVehicleLogic() )
								{
									int playerLap  = pIHumanData->GetVehicleLogic()->GetLogicState()->GetLap();
									if (pRule6->m_lap == playerLap)
									{
										// open the route......
										if (pRule6->m_state == JunctionRule::JUNCTION_OPEN)
											pRoute->SetActive(true);
										else
											pRoute->SetActive(false);
									}
									else
									{
										if (pRule6->m_state == JunctionRule::JUNCTION_OPEN)
											pRoute->SetActive(false);
										else
											pRoute->SetActive(true);
									}

								}
								
							}


						}
						break;

					case 7:
						{
							// open when player equal to and greater than lap x

							JunctionRule7 *pRule7 = static_cast<JunctionRule7*>(pRoute->m_pJunctionRule);


							// get first human player............

							IData *pIHumanData = 0;
							for (int j=0; j < static_cast<int>(m_AIlist.size()); j++)
							{
								IData *pIData = m_AIlist[j];
								bool isPlayer = pIData->GetVehicleState()->GetState()->GetPlayer();
								if (isPlayer)
								{
									pIHumanData = m_AIlist[j];
									break;
								}
							}

							if (pIHumanData)
							{
								if ( pIHumanData->GetVehicleLogic() )
								{
									int playerLap  = pIHumanData->GetVehicleLogic()->GetLogicState()->GetLap();
									if (pRule7->m_lap >= playerLap)
									{
										// open the route......
										if (pRule7->m_state == JunctionRule::JUNCTION_OPEN)
											pRoute->SetActive(true);
										else
											pRoute->SetActive(false);
									}
									else
									{
										if (pRule7->m_state == JunctionRule::JUNCTION_OPEN)
											pRoute->SetActive(false);
										else
											pRoute->SetActive(true);
									}

								}

							}
						}
						break;


					case 9:
						{
							JunctionRule9 *pRule9 = static_cast<JunctionRule9*>(pRoute->m_pJunctionRule);
	
							if (pRule9->m_currentState == JunctionRule::JUNCTION_NOT_INTIALISED)
							{
								if (pRule9->m_state == JunctionRule::JUNCTION_OPEN)
									pRule9->m_currentState = JunctionRule::JUNCTION_CLOSE;
								else
									pRule9->m_currentState = JunctionRule::JUNCTION_OPEN;

								pRule9->m_lastTime = m_pIGameServerTime->GetTime();
								pRule9->m_CarCount = 0;
								pRule9->m_startCounterReset = false;
							}
							else
							{
								if (pRule9->m_currentState == JunctionRule::JUNCTION_OPEN)
									pRoute->SetActive(true);
								else
									pRoute->SetActive(false);

								if (pRule9->m_CarCount >= pRule9->m_Ncars)
								{
									pRule9->m_currentState = pRule9->m_state;
									
									// start timer
									if (pRule9->m_startCounterReset == false)
									{
										pRule9->m_lastTime = m_pIGameServerTime->GetTime();
										pRule9->m_startCounterReset = true;
									}
									else if (m_pIGameServerTime->GetTime() > pRule9->m_lastTime + pRule9->m_resetTime)
									{
										pRule9->m_currentState = JunctionRule::JUNCTION_NOT_INTIALISED;
									}
									
								}
								else
								{

								}
							}
						}
						break;

					default:
						break;
					}

				}

			}
		}
		
		
		
	}


	void CRaceDirector::CalculateAllRouteSpeeds()
	{
		for (int i=0; i < static_cast<int>(m_AIlist.size()); i++)
		{
			IData *pIData = m_AIlist[i];
			//SetupSpeedData(i);
			CalculateSpeed(i);
		
		}
	}

	/*void CRaceDirector::SetupSpeedData(int carNo)
	{
		IData *pIData = m_AIlist[carNo];
		IVehicleState *pIVehState = pIData->GetVehicleState();
		ISpeedTracker *pISpeedTRacker = pIData->GetSpeedTracker();

		AITrackData *pAITrackData = m_pTrackData->GetTrackData();

		int noRoutes = pAITrackData->GetNumRoutes();

		for(int routeN=0; routeN<noRoutes; routeN++)
		{
			AITrackDataRoutes *pRoute = pAITrackData->GetRoute(routeN);

			if (pRoute->IsChangeableRoute())
			{
				for(int line=0; line<AITrackDataLines::TRACK_DATA_NUM_LINES; line++)
				{
					for(int seg=0; seg<pRoute->GetNumSegments(); seg++)
					{
						pRoute->AddSpeed(seg, line);
					}
				}
			}
		}

	}*/

	void CRaceDirector::CalculateSpeed(int carNo)
	{
		IData *pIData = m_AIlist[carNo];
		IVehicleState *pIVehState = pIData->GetVehicleState();
		ISpeedTracker *pISpeedTRacker = pIData->GetSpeedTracker();


		float speed_Coeff = 0.0f;
		float decel_co_eff = 0.0f;
		pISpeedTRacker->CalculateFrictions(speed_Coeff, decel_co_eff);
		decel_co_eff = -Maths::Abs(decel_co_eff);

		// bugatti
		/*const float assumedAcceleration = 12.5f;//Maths::Sqrt(5.96f);
		const float assumedDeceleration = 23.9f;//19.81f;//18.9f - 7.0f;////Maths::Sqrt(19.9f); 
		const float gravity = 9.81f;
		const float friction = 2.3f;//2.3f;

		const float percentage = 0.25f;
		const float power = 1437.0f;
		const float errorTorque = power * percentage;
		const float hp = 746.0f;
		const float mass = 1950.0f;
		const float bhp = power - errorTorque;
		const float topSpeed = 100.0f;//80.28f; //180mph ... 
		const float CA = 2.1f;
		*/

		// Ferrari F1 355....
		/*const float assumedAcceleration =  5.96f;//Maths::Sqrt(5.96f);
		const float assumedDeceleration = 19.9f;//19.81f;//18.9f - 7.0f;////Maths::Sqrt(19.9f); 
		const float gravity = 9.81f;
		const float friction = 2.3f;//2.3f;

		const float percentage = 0.125f; // 0.25 too high ???
		const float power = 416.0f;
		const float errorTorque = power * percentage;
		const float hp = 746.0f;
		const float mass = 1350.0f;;
		const float bhp = power - errorTorque;
		const float topSpeed = 80.0f;//80.28f; //180mph ... 
		const float CA = 2.1f;
		*/

		float gravity = 9.81f;
		float assumedAcceleration =  gravity;
		float assumedDeceleration = 19.0f;////19.9f;
		float friction = 2.3f;
		float topSpeed = 80.0f;//80.28f; //180mph ... 
		float CA = 2.1f;
		float percentage =  0.05f;//0.15f; // 0.25 too high ???
		float power = 416.0f;
		float hp = 745.699872f; // 1hp = 745.699872 watts......
		float mass = 1350.0f;
		float airFriction = 0.2f;
		float bhp = 0.0f;

		
		mass = pIVehState->GetState()->GetMass();
		mass =  Maths::Clamped(mass, 0.01f, 5000.0f);
		power = pIVehState->GetState()->GetPeakTorque();
		power =  Maths::Clamped(power, 0.01f, 50000.0f);

		float maxRevs = pIVehState->GetState()->GetMaxRevs();
		bhp = (power  * maxRevs) / 9549; 


		topSpeed = pIVehState->GetState()->GetTopSpeed();
		topSpeed =  Maths::Clamped(topSpeed, 0.01f, 1000.0f);

		airFriction = pIVehState->GetState()->GetAmaxCarData().m_AirFriction;
		percentage = 0.0f;//0.17f;//0.15f;
		
		float errorTorque = bhp * percentage; // percentage loss......
		bhp -= errorTorque;
		float ptwratio = (bhp * hp)  / mass;

		float zeroSixtyTime = mass / (bhp * 1.0f); // 0.9f
		float accelSixty = MPHTOMS(60.0f) / zeroSixtyTime;
		assumedAcceleration = accelSixty;

		float zeroHundredTime = mass / (bhp * 1.0f); // 0.9f
		float accelHundred = MPHTOMS(100.0f) / zeroSixtyTime;
		assumedAcceleration = Maths::Max(accelHundred, accelSixty);

		AITrackData *pAITrackData = m_pTrackData->GetTrackData();

		int noRoutes = pAITrackData->GetNumRoutes();

		for(int routeN=0; routeN<noRoutes; routeN++)
		{
			AITrackDataRoutes *pRoute = pAITrackData->GetRoute(routeN);
			
			if (pRoute->IsChangeableRoute())
			{

				//////////////////// next clump of the algorithm
				for(int line=0; line<AITrackDataLines::TRACK_DATA_NUM_LINES; line++)
				{
					Btl::Vector<float>  speeds;
					Btl::Vector<float>  curvatures;

					for(int seg=0; seg<pRoute->GetNumSegments(); seg++)
					{
						float c = pRoute->GetCurvature(seg, line);
						curvatures.PushBack(c);

						float Tspeed = airacingtech::CalculateSpeedMPS(c, speed_Coeff, 9.81f );	
						Tspeed = Maths::Min( Tspeed, pIVehState->GetState()->GetTopSpeed() );

						//U8 speedU = (U8)Tspeed;

						speeds.PushBack(Tspeed);

					}

					//////////////////// next clump of the algorithm
					// we now have the speeds for a line in a route.....
					for(int i=curvatures.Size() - 1; i>=0; i--)
					{
						int nextIndex = i-1;

						if (nextIndex < 0)
						{
							if (pRoute->IsLoopedTrack())
								nextIndex += curvatures.Size();
							else
							{
								speeds[0] = speeds[1];
								break;
							}
						}
					
						float speed1 =  speeds[i];
						float speed2 =  speeds[nextIndex];

						float distBetween = Maths::Abs(pRoute->GetDistance(nextIndex,i,line));

						float distanceToLastCorner = distBetween;//1.0f;
						float extraBraking = 0.0f;
						
						// deceleration in the equations is +ve and not -ve
						float assumedAccel;
						CalculateTheFrictions(speed1, pIData, assumedDeceleration, assumedAccel);
						
						float speedEstimate = Maths::Sqrt( (speed1*speed1) + 2.0f*(assumedDeceleration-extraBraking)*distanceToLastCorner);

						//if (speed1 < speed2)
						{	
							if (speedEstimate  < speed2)
							{
								speeds[nextIndex] =  Maths::Min(topSpeed, speedEstimate);
							}	

						}
					}		

					//////////////////// next clump of the algorithm
					int index = -1;
					float maxCurvature = 0.0f;
					for(int i=0; i<curvatures.Size(); i++)
					{
						if(Maths::Abs(curvatures[i]) > maxCurvature)
						{
							maxCurvature = Maths::Abs(curvatures[i]);
							index = i;
						}
					}

					//////////////////// next clump of the algorithm
					for(int i=index; i<curvatures.Size(); i++)
					{
						int nextIndex = i+1;
						if (nextIndex >=curvatures.Size())
						{
							if (pRoute->IsLoopedTrack())
								nextIndex -= curvatures.Size();
							else
							{
								//speeds[i] = speeds[i] - 1;
								break;
							}
						}
						//float c1 = curvatures[i];
						//float c2 = curvatures[nextIndex];


						float speed1 =  speeds[i];
						float speed2 =  speeds[nextIndex]; 

						

						float distBetween = Maths::Abs(pRoute->GetDistance(nextIndex,i,line));
						float distanceToLastCorner = distBetween;//1.0f;


						float DragF = (airFriction * speed1 * speed1);
						float DragAccel = DragF / mass;

						// accelerate according to power to weight ratio and grip
						float a = ptwratio / speed1;

						//assumedAcceleration = gravity;

						a -= DragAccel;
						if (a > assumedAcceleration)
							a = assumedAcceleration;

						//float temp;

						//CalculateTheFrictions(speed1, pHelper, friction, temp);
						float speedEstimate = Maths::Sqrt( (speed1*speed1) + 2.0f*a*distanceToLastCorner);

						//if (speed1 <= speed2)
						{
							if (speedEstimate < speed2)
							{
								speeds[nextIndex] = Maths::Min(topSpeed,speedEstimate);
							}

						}

					}


					//////////////////// next clump of the algorithm
					if(index > 0)
					{
						for(int i=0; i<index; i++)
						{
							int nextIndex = i+1;
							if (nextIndex >=curvatures.Size())
							{
								if (pRoute->IsLoopedTrack())
									nextIndex -= curvatures.Size();
								else
								{
									//speeds[i] = speeds[i] - 1;
									break;
								}
							}

							float speed1 =  speeds[i];
							float speed2 =  speeds[nextIndex]; 

							// accelerate according to power to weight ratio and grip
							float a = ptwratio / speed1;
							float DragF = (airFriction * speed1 * speed1);
							float DragAccel = DragF / mass;

							//assumedAcceleration = gravity;

							a -= DragAccel;
							if (a > assumedAcceleration)
								a = assumedAcceleration;

							float distBetween = Maths::Abs(pRoute->GetDistance(nextIndex,i,line));
							float distanceToLastCorner = distBetween;//1.0f;

							//CalculateTheFrictions(speed1, pHelper, friction, assumedAcceleration);
							float speedEstimate = Maths::Sqrt( (speed1*speed1) + 2.0f*a*distanceToLastCorner);

							
							//if (speed1 <= speed2)
							{
								if (speedEstimate < speed2)
								{
									speeds[nextIndex] = Maths::Min(topSpeed,speedEstimate);
								}
							}
						}
					}

					//////////////////// next clump of the algorithm
					for(int i=curvatures.Size() - 1; i>=0; i--)
					{
						int nextIndex = i-1;

						if (nextIndex < 0)
						{
							if (pRoute->IsLoopedTrack())
								nextIndex += curvatures.Size();
							else
							{
								speeds[0] = speeds[1];
								break;
							}
						}

						float speed1 =  speeds[i];
						float speed2 =  speeds[nextIndex];

						float distBetween = Maths::Abs(pRoute->GetDistance(nextIndex,i,line));

						float distanceToLastCorner = distBetween;//1.0f;
						float extraBraking = 0.0f;

						// deceleration in the equations is +ve and not -ve
						float assumedAccel;
						CalculateTheFrictions(speed1, pIData, assumedDeceleration, assumedAccel);

						float speedEstimate = Maths::Sqrt( (speed1*speed1) + 2.0f*(assumedDeceleration)*distanceToLastCorner);

						//if (speed1 < speed2)
						{	
							if (speedEstimate  < speed2)
							{
								speeds[nextIndex] =  Maths::Min(topSpeed, speedEstimate);
							}	

						}
					}		


					//////////////////// next clump of the algorithm
					for(int i=0; i<speeds.Size(); i++)
					{
						float speed1 = speeds[i];
						float assumedAccel;
						CalculateTheFrictions(speed1, pIData, assumedDeceleration, assumedAccel);
						float initialBrakingDistance = airacingtech::BrakingDistance(speed1, 10.0f, -assumedDeceleration, 0.0f); // cars length error ?
						initialBrakingDistance = Maths::Max(initialBrakingDistance, 1.0f); // 15.0f	

						int nextplusone = i+1;

						if (nextplusone >= speeds.Size())
						{
							if (pRoute->IsLoopedTrack())
								nextplusone -= speeds.Size();
							else
							{
								break;
							}
						}

						//int next = (int)(i + initialBrakingDistance);
						bool status = false;
						int next = pRoute->GetIndexOffsetByDistance(i, initialBrakingDistance, line, status);

						float nextSpeed = 0.0f;
						if (next < speeds.Size())
						{
							float minSpeed = FLT_MAX;
							for(int j=i; j<next; j++)
							{
								float testSpeed = speeds[j];
								if (testSpeed < minSpeed)
								{
									minSpeed = testSpeed;
								}
							}
							nextSpeed = minSpeed;
						}
						else
						{
							float minSpeed = FLT_MAX;
							for(int j=i; j<speeds.Size(); j++)
							{
								float testSpeed = speeds[j];
								if (testSpeed < minSpeed)
								{
									minSpeed = testSpeed;
								}
							}

							next -= speeds.Size();
							for(int j=0; j<next; j++)
							{
								float testSpeed = speeds[j];
								if (testSpeed < minSpeed)
								{
									minSpeed = testSpeed;
								}
							}

							nextSpeed = minSpeed;
						}

						float nspeed = nextSpeed;
						CalculateTheFrictions(nspeed, pIData, assumedDeceleration, assumedAccel);
						initialBrakingDistance = airacingtech::BrakingDistance(speed1, nspeed, -assumedDeceleration, 0.0f); // cars length error ?
						initialBrakingDistance = Maths::Max(initialBrakingDistance, 1.0f); // 1.0f // 15.0f	

						//next = (int)(i + initialBrakingDistance);
						next = pRoute->GetIndexOffsetByDistance(i, initialBrakingDistance, line, status);

						nextSpeed = 0.0f;
						int distcount = i;
						if (next < speeds.Size())
						{
							float minSpeed = FLT_MAX;
							for(int j=i; j<next; j++)
							{
								float testSpeed = speeds[j];
								if (testSpeed < minSpeed)
								{
									minSpeed = testSpeed;
									distcount = j;
								}
							}
							nextSpeed = minSpeed;
						}
						else
						{
							float minSpeed = FLT_MAX;
							for(int j=i; j<speeds.Size(); j++)
							{
								float testSpeed = speeds[j];
								if (testSpeed < minSpeed)
								{
									minSpeed = testSpeed;
									distcount = j;
								}
							}

							next -= speeds.Size();
							for(int j=0; j<next; j++)
							{
								float testSpeed = speeds[j];
								if (testSpeed < minSpeed)
								{
									minSpeed = testSpeed;
									distcount = j;
								}
							}

							nextSpeed = minSpeed;
						}


						if (nextSpeed < speed1)
						{
							int dist = 0;
							if (distcount < i)
							{
								dist = (speeds.Size() - i) + (distcount);
							}
							else
							{
								dist = distcount - nextplusone;
							}
							//int dist = next - nextplusone;
							for(int j=0; j<=dist; j++ )
							{
								int index = j + i;
								if (index >= speeds.Size())
								{
									if (pRoute->IsLoopedTrack())
										index -= speeds.Size();
									else
									{
										break;
									}
								}
								int indexNext = index + 1;
								if (indexNext >= speeds.Size())
								{
									if (pRoute->IsLoopedTrack())
										indexNext -= speeds.Size();
									else
									{
										break;
									}
								}
								speed1 = speeds[index];

								float distBetween = Maths::Abs(pRoute->GetDistance(index,indexNext,line));
								// work out the deceleration value
								CalculateTheFrictions(speed1, pIData, assumedDeceleration, assumedAccel);
								
								float speedEstimate = Maths::Sqrt( (speed1*speed1) + 2.0f*-assumedDeceleration*distBetween);

								speeds[indexNext] = Maths::Max(speedEstimate, nextSpeed);
							}
						}
					}

					for(int seg=0; seg<pRoute->GetNumSegments(); seg++)
					{
						float sp = speeds[seg];
						U8 sp8 = static_cast<U8>(sp);
						pRoute->SetSpeed(seg, line, sp8, carNo);
					}
					
					speeds.Clear();
					curvatures.Clear();

				}
			}
		}


		int stop = 0;

	}

	void CRaceDirector::CalculateTheFrictions(float speed , IData *pIData, float &assumedDeceleration, float &assumedAccel)
	{
		IVehicleState *pIVehState = pIData->GetVehicleState();
		IParamsAI *pIParams = pIData->GetParams();
		ISpeedTracker *pISpeedTRacker = pIData->GetSpeedTracker();

		pIVehState->GetState()->CalculateCarFrictions(speed); // 0.75
		pISpeedTRacker->CalculateFrictions(assumedAccel, assumedDeceleration);
		assumedDeceleration = Maths::Abs(assumedDeceleration);
	}

	



}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

