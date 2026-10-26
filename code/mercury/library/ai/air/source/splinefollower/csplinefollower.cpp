#include "stdafx.h"
#include <splinefollower/csplinefollower.h>
#include <messages/RouteChange.h>


//#define SPLINE_FOLLOWER_UNOPT

#ifdef _MSC_VER
#ifdef DEBUG_AI
#ifdef SPLINE_FOLLOWER_UNOPT
#pragma optimize("", off)
#endif
#endif
#endif

Debug::Float g_SteerRate("AmaxAI.Car.Steering.Controller.SteeringRate", 30.0f, Debug::Range(0.0f, 60.0f), Debug::Step(0.001f));
Debug::Float g_TimeStep("AmaxAI.Car.Steering.Controller.SteeringTimeStep", 0.03f, Debug::Range(0.0f, 2.0f), Debug::Step(0.001f));

namespace airacingtech
{

#ifdef PLATFORM_PS3
#define BRANCH_HINT_LIKELY( condition )		__builtin_expect( condition, 1 )
#define BRANCH_HINT_UNLIKELY( condition )	__builtin_expect( condition, 0 )
#else
#define BRANCH_HINT_LIKELY( condition )		condition
#define BRANCH_HINT_UNLIKELY( condition )	condition
#endif

	using namespace fuzzy_lib_one;

	CSplineFollower::CSplineFollower()
	{
		m_state.m_pTrackData = 0;
		m_state.m_pIData = 0;
		m_state.m_pIPIDController = 0;
		m_state.m_pIVehicleState = 0;
		m_state.m_pVehState = 0;
		m_state.m_pSplineTracker = 0;
		m_state.m_nearestIndex = -1;
		m_state.m_furthestIndex = -1;
		m_state.m_steering = 0.0f;
		m_state.m_angle = 0.0f;
		m_state.m_LastSteering = 0.0f;
		m_state.m_offset = 0.0f;
		m_state.m_pHeadingSmoother = new Smoother<Maths::HVec3>(3, Maths::HVec3(0.0f,0.0f,0.0f)); 
		m_state.m_pIGameServerTime = 0;
		m_state.m_startRouteChangeTime = 0.0f;
		m_state.m_startRouteChangeTimeThreshold = 2.0f;
		m_state.m_startConstraint = 0.0f;
		m_state.m_endIntroOutroSpline = false;
		m_state.m_distToSpline = 0.0f;
		m_state.m_trackAngle = 0.0f;
		m_state.m_startRaceTime = 0.0f;

		m_state.m_targetSplinePoint[0] = Maths::HVec3(0.0f,0.0f,0.0f);
		m_state.m_targetSplinePoint[1] = Maths::HVec3(0.0f,0.0f,0.0f);
		m_state.m_validTarget = false;
		m_state.m_reachedOffsetLimit = 0;
	}

	CSplineFollower::~CSplineFollower()
	{

	}

	void CSplineFollower::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_SPLINE_FOLLOWER, CID_SPLINE_FOLLOWER);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_SPLINE_FOLLOWER, CSplineFollower::CreateMe, CSplineFollower::DestroyMe, CHash("SplineFollower"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPLINE_FOLLOWER, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPLINE_FOLLOWER, MT_CHANGED_ROUTE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPLINE_FOLLOWER, MT_RELOAD_STEERING_DATA);
	
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPLINE_FOLLOWER, MT_START_AI_RACE);

		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPLINE_FOLLOWER, MT_STOP_AI);

		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPLINE_FOLLOWER, MT_START_AI_ROLLINGSTART_START);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPLINE_FOLLOWER, MT_START_AI_ROLLINGSTART_END);
		
	}

	IComponent *CSplineFollower::CreateMe()
	{
		return new CSplineFollower;
	}

	bool CSplineFollower::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}

	// Virtual IComponent methods
	bool CSplineFollower::Init(CObjectId oid, CParameterNode &paramNode)
	{
		SteeringControllerInitialiseParams *pData = static_cast<SteeringControllerInitialiseParams *>( paramNode.GetData() );
		if (pData)
		{
			CopyData(pData);
		}

		return true;
	}

	void CSplineFollower::CopyData(SteeringControllerInitialiseParams *pData)
	{
		m_state.m_UseVelocity = pData->m_UseVelocity;
		m_state.m_Blend = pData->m_Blend;
		m_state.m_ForwardsDist = pData->m_ForwardsDist;
		m_state.m_AngVelSteering = pData->m_AngVelSteering;
		m_state.m_AlignedSteering = pData->m_AlignedSteering;
		m_state.m_SteeringRate = pData->m_SteeringRate;
		m_state.m_SteeringTimeStep = pData->m_SteeringTimeStep;
		m_state.m_LerpValue = pData->m_LerpValue;
		m_state.m_DelayValue = pData->m_DelayValue;
		m_state.m_SmoothTimeStep = pData->m_SmoothTimeStep;
		m_state.m_UseOffset = pData->m_UseOffset;
		m_state.m_OffsetValue = pData->m_OffsetValue;
	}


	void CSplineFollower::Deinit(void)
	{
		if (m_state.m_pHeadingSmoother)
			delete m_state.m_pHeadingSmoother;

		m_state.m_pHeadingSmoother = 0;
		m_state.m_pTrackData = 0;
		m_state.m_pIData = 0;
		m_state.m_pIPIDController = 0;
		m_state.m_pIVehicleState = 0;
		m_state.m_pVehState = 0;
		m_state.m_pSplineTracker = 0;
		m_state.m_nearestIndex = -1;
		m_state.m_furthestIndex = -1;
		m_state.m_steering = 0.0f;
		m_state.m_angle = 0.0f;
		m_state.m_LastSteering = 0.0f;
		m_state.m_offset = 0.0f;
	}

	EMessageResult CSplineFollower::HandleMessage(const CMessage &msg)
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

		case MT_RELOAD_STEERING_DATA:
			{
				SteeringControllerInitialiseParams *pNewData = static_cast<SteeringControllerInitialiseParams *>(msg.mpData);
				if (pNewData)
				{
					CopyData(pNewData);
				}
			}
			break;

		case MT_START_AI_RACE:
			{

				if (m_state.m_pIGameServerTime)
				{
					m_state.m_startRaceTime = 	0.0f; 
				}

			}
			break;

		case MT_STOP_AI:
			{

			}
			break;


		case MT_START_AI_ROLLINGSTART_START:
			{
				m_rollingStart = true;
			}
			break;

		case MT_START_AI_ROLLINGSTART_END:
			{
				m_rollingStart = false;
			}
			break;


		default:
			break;
		}

		return msgRet;
	}

	void CSplineFollower::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_state.m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

		CObjectId trackdataSystem("TrackDataSystem");
		m_state.m_pTrackData = static_cast<ITrackDataAI *>(GlobalsComponent::GetObjectManager().QueryInterface(trackdataSystem, IID_TRACK_DATA_AI));	

		m_state.m_pIPIDController = static_cast<IPIDControllerAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PID_CONTROLLER));
		
		m_state.m_pIVehicleState = static_cast<IVehicleState *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_VEHICLE_STATE));
		if (m_state.m_pIVehicleState)
		{
			m_state.m_pVehState = m_state.m_pIVehicleState->GetState();
		}

		m_state.m_pSplineTracker = static_cast<ISplineTracker *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_SPLINE_TRACKER));
	
		CObjectId gameservertimeID("GameServerTimeSystem");
		m_state.m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

	
	}


	EComponentTypeId CSplineFollower::GetComponentTypeId(void)
	{
		return CID_SPLINE_FOLLOWER;
	}

	void CSplineFollower::ChangedRoute(RouteChangeMSG *pData)
	{
		m_state.m_offset = pData->m_offset;

		if (m_state.m_pIGameServerTime)
			m_state.m_startRouteChangeTime = m_state.m_pIGameServerTime->GetTime();
	}

	void CSplineFollower::Update()
	{
		if (m_state.m_pIData && m_state.m_pTrackData && m_state.m_pIPIDController && m_state.m_pIVehicleState && m_state.m_pVehState && m_state.m_pSplineTracker)
		{

			// if it is the player, we need to set the offset continually....
			if (m_state.m_pVehState->GetPlayer() || m_rollingStart == true)
			{
				const RouteInfo *pRouteInfo = m_state.m_pSplineTracker->GetCurrentRoute();
				if (pRouteInfo && pRouteInfo->m_pRoute)
				{
					m_state.m_offset = pRouteInfo->m_offset;
				}
			}
			else
			{
				if (m_state.m_pIGameServerTime)
				{
					if (m_state.m_pIGameServerTime->GetTime() > m_state.m_startRouteChangeTime + m_state.m_startRouteChangeTimeThreshold)
					{
						m_state.m_startRouteChangeTime = 0.0f;
					}
				}
			}


			if (!m_state.m_pVehState->GetWheel(0)->GetOnGround() &&
				!m_state.m_pVehState->GetWheel(1)->GetOnGround() &&
				!m_state.m_pVehState->GetWheel(2)->GetOnGround() &&
				!m_state.m_pVehState->GetWheel(3)->GetOnGround() 
				)
			{
				// if in the air, move the offset value 
				const RouteInfo *pRouteInfo = m_state.m_pSplineTracker->GetCurrentRoute();
				if (pRouteInfo && pRouteInfo->m_pRoute)
				{
					m_state.m_offset = pRouteInfo->m_offset;
				}
			}


			m_state.m_reachedOffsetLimit = 0;

		
			// Store a value if we are at a set limit
			if (Maths::Abs(m_state.m_distToSpline) > 2.0f) // need to check this
			{
				const RouteInfo *pRouteInfo = m_state.m_pSplineTracker->GetCurrentRoute();
				if (pRouteInfo && pRouteInfo->m_pRoute)
				{
					m_state.m_offset = pRouteInfo->m_offset;
					
					if (m_state.m_distToSpline > 0.0f)
					{
						m_state.m_reachedOffsetLimit = 1;
					}
					else
					{
						m_state.m_reachedOffsetLimit = -1;
					}
				}
			}
			


			
			Maths::HVec3 lastHeading = m_state.m_pVehState->GetVelocityLast();
			lastHeading.Y(0.0f);
			lastHeading.Normalise();

			Maths::HVec3 heading = m_state.m_pVehState->GetZaxis();
			Maths::HVec3 pos = m_state.m_pVehState->GetPosition();
			Maths::HVec3 vel = m_state.m_pVehState->GetVelocity();
			vel.Y(0.0f);
			vel.Normalise();	
			

			float speedMPS = m_state.m_pVehState->GetSpeed();

			if (m_state.m_UseVelocity)// || speedMPS > 110.0f)
			{
				heading = m_state.m_pVehState->GetVelocity();
			}

#if 0		// Darren take out this Hack for now using 24/07/2008
			float lv = g_SteeringLerpValue;
			heading.Lerp(vel,lv);
#endif	

			heading.Y(0.0f);
			heading.Normalise();	

			BuildCubicSpline2();
			

			//if (m_nearestIndex != -1) // is this check really needed ????
			//if (m_state.m_nearestIndex > 0 && m_state.m_nearestIndex < m_state.m_samplesDebug.Size() &&
			//	m_state.m_furthestIndex < m_state.m_samplesDebug.Size())
			{
				//Maths::HVec3 p1 = m_state.m_samplesDebug[m_state.m_nearestIndex];
				//Maths::HVec3 p2 = m_state.m_samplesDebug[m_state.m_furthestIndex];

				Maths::HVec3 p1 = m_state.m_targetSplinePoint[0];
				Maths::HVec3 p2 = m_state.m_targetSplinePoint[1];

				float distFollowingLine = p1.DistanceSquared(p2);
				const float minFollowingLine =  9.0f;

				if (distFollowingLine <= minFollowingLine)
				{
					m_state.m_validTarget = false;
				}
				

				if (m_state.m_validTarget == true)
				{
					FollowSpline(pos, heading, p1, p2);
					m_state.m_trackAngle = m_state.m_angle;

					FollowSpline(pos, heading, pos, p2);
					m_state.m_distToSpline = DistToLine(pos, p1, p2);
				}
				else
				{
					m_state.m_trackAngle = m_state.m_angle;
					//m_state.m_angle = 0.0f;
					//m_state.m_trackAngle = 0.0f;
					//m_state.m_distToSpline = 0.0f;
				}
				


				float angle = GetAngle();
				if (m_state.m_pVehState->GetSpeedMPH() < 2.0f)
				{
					angle = 0.0f;
				}

				//const float pid_time_step =  0.01666f;
				float pid_time_step = m_state.m_pIPIDController->GetTimeStep(); // 0.01666

				//if (angle < 0.05f && angle > -0.05f)
				//		angle = 0.0f;

				float fzsmoothOutput = FuzzyRightShoulder(Maths::Abs(angle), 0.0f, 0.02f) * 0.5f;
				
				m_state.m_pIPIDController->Record(-angle, pid_time_step);
				float steeringTest = m_state.m_pIPIDController->GetOutput() * fzsmoothOutput; //0.25f;//0.5f;


				float yVel = m_state.m_pIData->GetState().m_angVelY;

				// steering correction
				float alignedDirection = m_state.m_pIData->GetState().m_alignedAngleDirection;
				//printf("Aligned Angle :%f\n",alignedDirection );

				float yVelD = (yVel * m_state.m_AngVelSteering * (Maths::Abs(m_state.m_steering) + Maths::Abs(m_state.m_steering)));
				float alignD = (alignedDirection * m_state.m_AlignedSteering * (Maths::Abs(m_state.m_steering) + Maths::Abs(m_state.m_steering)));
				steeringTest += (alignD + yVelD);


				// update the current steering angle
				float steerDelta;
				steerDelta = steeringTest - m_state.m_LastSteering;

				if(steerDelta > g_SteerRate*g_TimeStep)
				{
					steerDelta = g_SteerRate*g_TimeStep;
				}
				else if (steerDelta < -g_SteerRate*g_TimeStep)
				{
					steerDelta = -g_SteerRate*g_TimeStep;
				}

				steeringTest = m_state.m_LastSteering + steerDelta;


				m_state.m_steering = steeringTest;
				
			}
			
			/*if (m_state.m_pIGameServerTime->GetTime() < m_state.m_startRaceTime) 
			{
				m_state.m_steering = 0.0f;	
				const RouteInfo *pRouteInfo = m_state.m_pSplineTracker->GetCurrentRoute();
				if (pRouteInfo && pRouteInfo->m_pRoute)
				{
					m_state.m_offset = pRouteInfo->m_offset;
				}
			}
			*/
			

			m_state.m_steering = Maths::Clamped(m_state.m_steering, -1.0f, 1.0f);
			m_state.m_LastSteering = m_state.m_steering;

		}
	}

	void CSplineFollower::BuildCubicSpline2()
	{
		m_state.m_validTarget = false;
		float speedMPS = m_state.m_pVehState->GetSpeed();
		float sp = m_state.m_pVehState->GetSpeedMPH();

		int startSample = 15;
		int intval =  startSample / 3;

		const RouteInfo *pRouteInfo = m_state.m_pSplineTracker->GetCurrentRoute();
		int currentLane = m_state.m_pSplineTracker->GetCurrentLane();
		
		
		if (pRouteInfo && pRouteInfo->m_in)
		{
			m_state.m_validTarget = true;
			float offset = m_state.m_offset;

			if (BRANCH_HINT_UNLIKELY(m_state.m_UseOffset))
			{
				offset = m_state.m_OffsetValue;
			}
		
			AITrackDataRoutes *pRoute = pRouteInfo->m_pRoute;
			int index1 = pRouteInfo->m_index;

			m_state.m_samplesDebug.Clear();
			m_state.m_ctrlPoints.Clear();
			
			//Btl::Vector<Maths::HVec3, Btl::VectorParameters_Static< 32 > > ctrlPoints;


			AIVEC ctrlPoints[15] = 
			{
				AIVEC(Maths::UNDEFINED), AIVEC(Maths::UNDEFINED), AIVEC(Maths::UNDEFINED),
				AIVEC(Maths::UNDEFINED), AIVEC(Maths::UNDEFINED), AIVEC(Maths::UNDEFINED),
				AIVEC(Maths::UNDEFINED), AIVEC(Maths::UNDEFINED), AIVEC(Maths::UNDEFINED),
				AIVEC(Maths::UNDEFINED), AIVEC(Maths::UNDEFINED), AIVEC(Maths::UNDEFINED),
				AIVEC(Maths::UNDEFINED), AIVEC(Maths::UNDEFINED), AIVEC(Maths::UNDEFINED)
			};

			int ctrlPointsCount = 0;


			//Maths::HVec3 pnt;

			for(int i=3; BRANCH_HINT_LIKELY(i>0); i--)
			{
				int cindex = pRoute->GetIndexOffset(index1, -1*i);	
				pRoute->GetPositionOffsetLine(offset, cindex , ctrlPoints[ctrlPointsCount], AITrackDataLines::TRACK_DATA_LINE_RACING);	
				//m_state.m_ctrlPoints.PushBack(pnt);
				//ctrlPoints.PushBack(pnt);
				//ctrlPoints[ctrlPointsCount] = pnt;
				ctrlPointsCount++;
			}

		
			for(int i=0; BRANCH_HINT_LIKELY(i<9); i++)
			{
				int cindex = pRoute->GetIndexOffset(index1, i);
				pRoute->GetPositionOffsetLine(offset, cindex , ctrlPoints[ctrlPointsCount], AITrackDataLines::TRACK_DATA_LINE_RACING);
				//m_state.m_ctrlPoints.PushBack(pnt);
				//ctrlPoints.PushBack(pnt);

				//ctrlPoints[ctrlPointsCount] = pnt;
				ctrlPointsCount++;
			}

			ctrlPointsCount--;

			
			//B_Spline_ApproxNonLooped_Opt(m_state.m_ctrlPoints, m_state.m_samplesDebug, 1.0f, 0.05f); //  allows us to get to 1m samples
			

			float fzSpeedMult = FuzzyRightShoulder(sp, 30.0f, 90.0f);
			fzSpeedMult *= 12.0f;//10.0f; // 4
			//int extraSpeedDist = (int)fzSpeedMult;

			//int extraSpeedDist2 = 0;
		

			//int forwardsDist = (int)(m_state.m_ForwardsDist+ extraSpeedDist + extraSpeedDist2);
			float fForwardsDist = m_state.m_ForwardsDist + fzSpeedMult;
			Maths::HVec3 position = m_state.m_pVehState->GetPosition();
			AIVEC startOnSpline;
			AIVEC targetPoint = B_Spline_EvaluatePointAheadOfPoint(position, &ctrlPoints[0], ctrlPointsCount, fForwardsDist,1.f,&startOnSpline);
			m_state.m_targetSplinePoint[0] = startOnSpline;
			m_state.m_targetSplinePoint[1] = targetPoint;

			// debug output only, so can be wrapped in ifdefs.......
			m_state.m_samplesDebug.Clear();
			m_state.m_samplesDebug.PushBack(m_state.m_targetSplinePoint[0]);
			m_state.m_samplesDebug.PushBack(m_state.m_targetSplinePoint[1]);

			
			/*
			m_state.m_nearestIndex = TrackSpline(position);
			m_state.m_furthestIndex = m_state.m_nearestIndex + forwardsDist;
			if (BRANCH_HINT_UNLIKELY(m_state.m_furthestIndex >= m_state.m_samplesDebug.Size()))
			{
				m_state.m_furthestIndex = m_state.m_samplesDebug.Size() - 1;
			}

			*/

		}
		else 
		{
			m_state.m_validTarget = false;
		}
	}




	


	int CSplineFollower::TrackSpline(Maths::HVec3& position)
	{
		int nSamples;
		Maths::HVec3 pos = position;
		int sample = 0;

		if (sample != -1)
		{
			nSamples = m_state.m_samplesDebug.Size()- 1;
			int     originalPoint = sample;

			bool updated	= false;
			bool endSpline = false;

			//	Track forwards
			do
			{
				int next = sample + 1;
				if ( next > nSamples)
				{
					updated = false;	
				}
				else
				{
					Maths::HVec3 pp = m_state.m_samplesDebug[sample];    
					Maths::HVec3 pd = m_state.m_samplesDebug[next];

					pd.Y(0.0f);
					pp.Y(0.0f);
					pos.Y(0.0f);
					
					pd -= pp;
					pp -= pos;

					float dot = pp | pd;

					if ( dot < 0.0f )
					{
						sample = next;
						updated = true;
					}
					else
					{
						updated = false;
					}
				}
			}
			while (updated);
		}

		return sample;

	}


	void CSplineFollower::FollowSpline(Maths::HVec3& pos, Maths::HVec3& heading, Maths::HVec3& p1, Maths::HVec3& p2)
	{
		Maths::HVec3 vehicleHeading = heading;
		vehicleHeading.Y(0.0f);
	
		Maths::HVec3 splineDirection = p2 - p1; 
		splineDirection.Y(0.0f);

		Maths::HVec3 mySideDirection(splineDirection.Z(), splineDirection.Y(), -splineDirection.X());
		mySideDirection.Normalise();

		Maths::HVec3 offset = pos - p2;
		offset.Y(0.0f);

		float distToSpline = (offset | mySideDirection);
		m_state.m_distToSpline = distToSpline;

		float blend = m_state.m_Blend;

		m_state.m_shouldMove = false;
		
#ifdef _CONSTRAINT_BY_VELOCITY
		float diff = Maths::Abs(distToSpline) - 0.3f;
		
		Maths::HVec3 vel = m_state.m_pVehState->GetVelocity();
		float velY = vel.Y();
		vel.Y(0.0f);
		float l = vel.Length();
		vel.Y(velY);

		Maths::HVec3 splineDirectionVel = p2 - pos;//p1; 
		splineDirectionVel.Y(0.0f);
		splineDirectionVel.Normalise();
		splineDirectionVel *= l;
		splineDirectionVel.Y(velY);
		
		Maths::HVec3 v1 = splineDirectionVel;
		Maths::HVec3 v2 = vel;
		
		int side = Vector2DSign(v1,v2);

		m_state.m_constraintAngVelocity = m_state.m_pVehState->GetAngularVelocity();

		float ang = m_state.m_constraintAngVelocity.Y();
		
		if ((distToSpline < 0.0f) && (Maths::Abs(distToSpline) > 0.3f && Maths::Abs(distToSpline) < 1.0f))
		{
			//velocity.............
			if (side < 0)
			{
				ang += 0.05f;
				ang = Maths::Min(ang, 2.0f);
				m_state.m_constraintAngVelocity.Y(ang);

				if (m_state.m_pIGameServerTime)
					m_state.m_startConstraint = m_state.m_pIGameServerTime->GetTime();
			}
		}
		else
		{
			if ((side > 0) && (Maths::Abs(distToSpline) > 0.3f && Maths::Abs(distToSpline) < 1.0f))
			{
				ang -= 0.05f;
				ang = Maths::Max(ang, -2.0f);
				m_state.m_constraintAngVelocity.Y(ang);

				if (m_state.m_pIGameServerTime)
					m_state.m_startConstraint = m_state.m_pIGameServerTime->GetTime();
			}
		}

		if (m_state.m_pIGameServerTime)
		{
			float time = m_state.m_pIGameServerTime->GetTime();
			float diffTime = time - m_state.m_startConstraint;
			if (diffTime < 1.0f) // 0.5f
			{
				m_state.m_constraintPosition = splineDirectionVel;//vel;
				m_state.m_constraintPosition = SmoothExp(vel, splineDirectionVel, g_SteeringTimeStep, g_SteeringDecayValue);	
				m_state.m_shouldMove = true;
			}
			else
			{
				m_state.m_shouldMove = false;
				m_state.m_startConstraint = 0.0f;
			}

		}
#endif
		


	/*	if (m_pIGameServerTime)
		{
			float currentTime = m_pIGameServerTime->GetTime();
			float diff = m_pIGameServerTime->GetTime() - m_startRouteChangeTime;
			if (diff > m_startRouteChangeTimeThreshold)
				blend = g_SteeringBlend;
			else
				blend = blend * 0.5f;
		}
		*/

		//float distBlend = FuzzyLeftShoulder(Maths::Abs(distToSpline), 0.5f, 4.0f); // 3.0f
		//distBlend = Maths::Max(distBlend, 0.2f);
		float distBlend = FuzzyRightShoulder(Maths::Abs(distToSpline), 0.0f, 1.0f); // 4.0f
		float distToSpline2 = distBlend * distToSpline;
		
		distToSpline = distToSpline2;

		mySideDirection *= ( blend * distToSpline); 
		splineDirection -= mySideDirection;

		// calc turn from here
		Maths::HVec3 dirTan(splineDirection.Z(), 0.0f, -splineDirection.X());
		//dirTan.Normalise();
		//splineDirection.Normalise();

		float x = -(vehicleHeading | dirTan);
		float y = vehicleHeading | splineDirection;

		m_state.m_angle = atan2f( x, y );	

	}


	////////////////////////////////////////////////////////////////////////////////
	// INTRO and OUTRO following a simple spline.. uses ISplineAI not the ITRackData class.....
	// NASTY code here..put what the hell. Should really be a different component.
	////////////////////////////////////////////////////////////////////////////////

	void CSplineFollower::UpdateTargetPoint(Maths::HVec3 point)
	{
		if (m_state.m_pIData && m_state.m_pIPIDController && m_state.m_pIVehicleState && m_state.m_pVehState)
		{
			Maths::HVec3 heading = m_state.m_pVehState->GetZaxis();
			Maths::HVec3 pos = m_state.m_pVehState->GetPosition();
			m_state.m_offset = 0.0f;
			heading.Y(0.0f);
			heading.Normalise();	

			FollowSpline(pos, heading, pos, point);

			float angle = GetAngle();
			if (m_state.m_pVehState->GetSpeedMPH() < 2.0f)
			{
				angle = 0.0f;
			}

			float pid_time_step = m_state.m_pIPIDController->GetTimeStep(); // 0.01666

			m_state.m_pIPIDController->Record(-angle, pid_time_step);
			float steeringTest = m_state.m_pIPIDController->GetOutput() * 0.25f;//0.5f;


			float yVel = m_state.m_pIData->GetState().m_angVelY;

			// steering correction
			float alignedDirection = m_state.m_pIData->GetState().m_alignedAngleDirection;
			//printf("Aligned Angle :%f\n",alignedDirection );

			float yVelD = (yVel * m_state.m_AngVelSteering * (Maths::Abs(m_state.m_steering) + Maths::Abs(m_state.m_steering)));
			float alignD = (alignedDirection * m_state.m_AlignedSteering * (Maths::Abs(m_state.m_steering) + Maths::Abs(m_state.m_steering)));
			steeringTest += (alignD + yVelD);


			// update the current steering angle
			float steerDelta;
			steerDelta = steeringTest - m_state.m_LastSteering;

			if(steerDelta > g_SteerRate*g_TimeStep)
			{
				steerDelta = g_SteerRate*g_TimeStep;
			}
			else if (steerDelta < -g_SteerRate*g_TimeStep)
			{
				steerDelta = -g_SteerRate*g_TimeStep;
			}

			steeringTest = m_state.m_LastSteering + steerDelta;


			m_state.m_steering = steeringTest;

			m_state.m_steering = Maths::Clamped(m_state.m_steering, -1.0f, 1.0f);
			m_state.m_LastSteering = m_state.m_steering;

			m_state.m_samplesDebug.Clear();
			m_state.m_samplesDebug.PushBack(point);
			m_state.m_samplesDebug.PushBack(m_state.m_pVehState->GetPosition());


		}
	}

	void CSplineFollower::Update(class ISplineAI *pISpline)
	{
		if (pISpline && m_state.m_pIData && m_state.m_pIPIDController && m_state.m_pIVehicleState && m_state.m_pVehState)
		{
			m_state.m_samplesDebug.Clear();

			// we can now update the steering on this spline.....
			m_state.m_offset = 0.0f;
			Maths::HVec3 lastHeading = m_state.m_pVehState->GetVelocityLast();
			lastHeading.Y(0.0f);
			lastHeading.Normalise();

			Maths::HVec3 heading = m_state.m_pVehState->GetZaxis();
			Maths::HVec3 pos = m_state.m_pVehState->GetPosition();
			Maths::HVec3 vel = m_state.m_pVehState->GetVelocity();
			vel.Y(0.0f);
			vel.Normalise();	


			float speedMPS = m_state.m_pVehState->GetSpeed();
			float sp = m_state.m_pVehState->GetSpeedMPH();

			if (m_state.m_UseVelocity)// || speedMPS > 110.0f)
			{
				heading = m_state.m_pVehState->GetVelocity();
			}

			heading.Y(0.0f);
			heading.Normalise();	

			
			float fzSpeedMult = FuzzyRightShoulder(sp, 40.0f, 90.0f);
			fzSpeedMult *= 10.0f; // 4
			int extraSpeedDist = (int)fzSpeedMult;

			int extraSpeedDist2 = 0;
			
			int forwardsDist = (int)(m_state.m_ForwardsDist+ extraSpeedDist + extraSpeedDist2);
			Maths::HVec3 position = m_state.m_pVehState->GetPosition();
			m_state.m_nearestIndex = TrackSpline(position, pISpline, m_state.m_nearestIndex);
			m_state.m_furthestIndex = m_state.m_nearestIndex + forwardsDist;
			m_state.m_endIntroOutroSpline = false;
			if (m_state.m_furthestIndex >= pISpline->GetLength())
			{
				// we are at the end of the spline.. broadcast a message........
				m_state.m_furthestIndex = pISpline->GetLength() - 1;
				m_state.m_endIntroOutroSpline = true;
			}

			if (m_state.m_nearestIndex >= 0)
			{
				assert(m_state.m_nearestIndex < pISpline->GetLength());
				assert(m_state.m_furthestIndex < pISpline->GetLength());
				Maths::HVec3 p1 = pISpline->GetSamplePoint(m_state.m_nearestIndex);
				Maths::HVec3 p2 = pISpline->GetSamplePoint(m_state.m_furthestIndex);

				FollowSpline(pos, heading, p1, p2);

				float angle = GetAngle();
				if (m_state.m_pVehState->GetSpeedMPH() < 2.0f)
				{
					angle = 0.0f;
				}

				//const float pid_time_step =  0.01666f;
				float pid_time_step = m_state.m_pIPIDController->GetTimeStep(); // 0.01666

				m_state.m_pIPIDController->Record(-angle, pid_time_step);
				float steeringTest = m_state.m_pIPIDController->GetOutput() * 0.25f;


				float yVel = m_state.m_pIData->GetState().m_angVelY;

				// steering correction
				float alignedDirection = m_state.m_pIData->GetState().m_alignedAngleDirection;
				//printf("Aligned Angle :%f\n",alignedDirection );

				float yVelD = (yVel * m_state.m_AngVelSteering * (Maths::Abs(m_state.m_steering) + Maths::Abs(m_state.m_steering)));
				float alignD = (alignedDirection * m_state.m_AlignedSteering * (Maths::Abs(m_state.m_steering) + Maths::Abs(m_state.m_steering)));
				steeringTest += (alignD + yVelD);


				// update the current steering angle
				float steerDelta;
				steerDelta = steeringTest - m_state.m_LastSteering;

				if(steerDelta > g_SteerRate*g_TimeStep)
				{
					steerDelta = g_SteerRate*g_TimeStep;
				}
				else if (steerDelta < -g_SteerRate*g_TimeStep)
				{
					steerDelta = -g_SteerRate*g_TimeStep;
				}

				steeringTest = m_state.m_LastSteering + steerDelta;


				m_state.m_steering = steeringTest;

			}
			else
			{
				m_state.m_steering = 0.0f;
			}

			m_state.m_steering = Maths::Clamped(m_state.m_steering, -1.0f, 1.0f);
			m_state.m_LastSteering = m_state.m_steering;

			int max = Maths::Min(pISpline->GetLength(), 100);
			for(int i=0;i<max; i++)
			{
				m_state.m_samplesDebug.PushBack(pISpline->GetSamplePoint(i));
			}
			

		}
	}


	int CSplineFollower::TrackSpline(Maths::HVec3& position, class ISplineAI *pISpline, int nearIndex)
	{
		int nSamples;
		Maths::HVec3 pos = position;
		int sample = nearIndex;

		if (pISpline)
		{
		
			if (sample != -1)
			{
				nSamples = pISpline->GetLength()- 1;
				int     originalPoint = sample;

				bool updated	= false;
				bool endSpline = false;

				//	Track forwards
				do
				{
					int next = sample + 1;
					if ( next > nSamples)
					{
						updated = false;	
					}
					else
					{
						Maths::HVec3 pp = pISpline->GetSamplePoint(sample);    
						Maths::HVec3 pd = pISpline->GetSamplePoint(next);

						pd.Y(0.0f);
						pp.Y(0.0f);
						pos.Y(0.0f);

						pd -= pp;
						pp -= pos;

						float dot = pp | pd;

						if ( dot < 0.0f )
						{
							sample = next;
							updated = true;
						}
						else
						{
							updated = false;
						}
					}
				}
				while (updated);
			}
			else
			{
				// find nearest
				sample = pISpline->GetNearestSamplePointIndexXZ(pos);
			}
		}

		return sample;
	}

}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


