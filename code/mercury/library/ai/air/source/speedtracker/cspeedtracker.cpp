#include "stdafx.h"
#include <speedtracker/cspeedtracker.h>
#include <system/Perf/PerfEvent.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif


namespace airacingtech
{

	Debug::Logger& g_Log = Debug::GetLogger( "air.speedtracker" );

	CSpeedTracker::CSpeedTracker() : m_pTrackData(0), m_pIData(0), m_pSplineTracker(0), m_pVehState(0), m_pIVehicleState(0)
	{
		m_pSpeedSmoother = 0;//new Smoother<float>(30, 0.0f); 
	}

	CSpeedTracker::~CSpeedTracker()
	{
		if (m_pSpeedSmoother)
			delete m_pSpeedSmoother;
	}

	void CSpeedTracker::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_SPEED_TRACKER, CID_SPEED_TRACKER);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_SPEED_TRACKER, CSpeedTracker::CreateMe, CSpeedTracker::DestroyMe, CHash("SpeedTracker"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPEED_TRACKER, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_SPEED_TRACKER, MT_STOP_AI);

	
		
	}

	IComponent *CSpeedTracker::CreateMe()
	{
		return new CSpeedTracker;
	}

	bool CSpeedTracker::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}

	// Virtual IComponent methods
	bool CSpeedTracker::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}


	void CSpeedTracker::Deinit(void)
	{

	}

	EMessageResult CSpeedTracker::HandleMessage(const CMessage &msg)
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

		case MT_STOP_AI:
			{
				m_pIData->GetState().m_trackSpeed = 0.0f;
				m_pIData->GetState().m_pIVehicleState->GetState()->SetTrackSpeed(m_pIData->GetState().m_trackSpeed );		
			}
			break;


		default:
			break;
		}

		return msgRet;
	}

	void CSpeedTracker::CachedInterfaces()
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
	
	}


	EComponentTypeId CSpeedTracker::GetComponentTypeId(void)
	{
		return CID_SPEED_TRACKER;
	}

	void CSpeedTracker::Update()
	{

		PERF_EVENT("AI.CSpeedTracker.Update");

		if (m_pIData && m_pTrackData && m_pSplineTracker && m_pVehState)
		{
			m_pIData->GetState().m_trackSpeed = 999.0f; //30.0f; was this to get around a bug ?????

			//float decel_co_eff; // rough ranges are -17
			//float speed_Coeff;  // rough range is 1.8

			//CalculateFrictions(speed_Coeff, decel_co_eff);
			//decel_co_eff *= 2.0f; // 3.0f..Testing
			//decel_co_eff = -Maths::Abs(decel_co_eff);

			// Darren HACK OLD, VERY OLD
			//decel_co_eff -= 3;	// makes the car slide out.....
			//speed_Coeff *= 1.1f; //1.0 - 1.2 makes them make slight mistakes, ie slide out

		
			//float initialBrakingDistance = airacingtech::BrakingDistance(m_pVehState->GetSpeed(), 15.0f, decel_co_eff, 0.0f); // cars length error ?	
			//initialBrakingDistance = Maths::Clamped(initialBrakingDistance, 1.0f/*5.0f*/, 250.0f);// + 15; // 10.0f // 15.0f
			//int initBrakeDist = (int)initialBrakingDistance;
	
			const RouteInfo *pRouteInfo = m_pSplineTracker->GetCurrentRoute();

			if (pRouteInfo)
			{
				float offset = 0.0f;
				ISplineFollower *pSplineFollower = m_pIData->GetSplineFollower();
				if (pSplineFollower)
				{
					offset = pSplineFollower->GetOffset();
				}
				else
				{
					offset = pRouteInfo->m_offset;
				}

				// Darren Hack, always use the racing line for the speed
				//const int currentLane = AITrackDataLines::TRACK_DATA_LINE_RACING; 
				const int currentLane = pRouteInfo->m_pRoute->NearestLane(offset); 


				AITrackDataRoutes *pRoute = pRouteInfo->m_pRoute;
				int index1 = pRouteInfo->m_index;
				int line = currentLane; 
/*				// Darren 01/07/09...don`t need this code to run, legacy.......			

				// TODO : This needs to be optimised.....
				bool status = false;
				int index2 = pRoute->GetIndexOffsetByDistance(index1, initialBrakingDistance, line, status);
	
				int index3 = -1;
				float maxCurvature = pRoute->GetMaxCurvature(index1, index2, line, index3, status);
				float speed = airacingtech::CalculateSpeedMPS(maxCurvature, speed_Coeff, 9.81f );
				speed = Maths::Min(speed, m_pVehState->GetTopSpeed() );

				initialBrakingDistance = airacingtech::BrakingDistance(m_pVehState->GetSpeed(), speed, decel_co_eff, 0.0f); // cars length error ?	
				initialBrakingDistance = Maths::Clamped(initialBrakingDistance, 1.0f, 250.0f);// + 15.0f; // 10.0f // 15.0f

				int index5 = -1;
				int index4 = pRoute->GetIndexOffsetByDistance(index1, initialBrakingDistance, line, status);	
				maxCurvature = pRoute->GetMaxCurvature(index1, index4, line, index5, status);
				speed = airacingtech::CalculateSpeedMPS(maxCurvature, speed_Coeff, 9.81f );	
				speed = Maths::Min(speed, m_pVehState->GetTopSpeed() );

				//float speedSmoothed = m_pSpeedSmoother->Update(speed);

*/

				int carNO = m_pIData->GetState().m_index;
				float actualTrackSpeed = static_cast<float>( pRoute->GetSpeed(pRouteInfo->m_index, line, carNO) );
				float surfaceFriction = m_pVehState->GetSurfaceFriction();
				actualTrackSpeed *= surfaceFriction;
				m_pIData->GetState().m_trackSpeedPreprocessed = actualTrackSpeed;
				
				

				float actualTrackPercenatge = ((m_pVehState->GetSpeed() / actualTrackSpeed)*100.0f);
			
				//if (m_pVehState->GetPlayer())
				//{
					//LOGINFO(g_Log, "TrackSpeed : [%f], TrackPercentageSpeed : [%f]", speed, ((m_pVehState->GetSpeed() / speed)*100.0f));
				//	LOGINFO(g_Log, "TrackSpeedPreProcessed : [%f], TrackPercentageSpeedPreProcessed : [%f]", actualTrackSpeed, actualTrackPercenatge);
				//}


				m_pIData->GetState().m_trackSpeed = actualTrackSpeed;// * 0.8f;//speed;

				// Darren : This is very lazy coding......
				
				if (m_pIData->GetRubberBanding() && m_pIData->GetRubberBanding()->GetEventBalancingSegmentData() && !m_pIData->GetState().m_pIVehicleState->GetState()->GetPlayer())
				{
					// factor in the rubber banding for the track speed.
					// clamp between 0.2f and 1.0f....
					float maxTrackSpeedMultiplier = m_pIData->GetRubberBanding()->GetEventBalancingSegmentData()->m_TrackSpeedPercentageMax;
					maxTrackSpeedMultiplier = Maths::Clamped(maxTrackSpeedMultiplier, 0.2f, 1.0f);
					m_pIData->GetState().m_trackSpeed *= maxTrackSpeedMultiplier;
				}



				m_pIData->GetState().m_pIVehicleState->GetState()->SetTrackSpeed(m_pIData->GetState().m_trackSpeed );
			}
			else
			{
				// no route....
				int stop = 0;
			}


		}
	}

	void CSpeedTracker::CalculateFrictions(float &speed_Coeff, float &decel_co_eff)
	{
		DataState &myDataState = m_pIData->GetState();
		VehicleState *pVehicleState = myDataState.m_pIVehicleState->GetState();
		assert(pVehicleState);

		// calculate track frictions with AI data......
		IParamsAI *pIParams = m_pIData->GetParams();
		if (pIParams)
		{
			VehicleState *pVehicleStateMy = myDataState.m_pIVehicleState->GetState();
			assert(pVehicleStateMy);

			float vehicleBrakingGrip = pVehicleStateMy->GetBrakingFriction();
			float vehicleLateralGrip = pVehicleStateMy->GetLateralFriction();

			const DriverData *pDriverData = pIParams->GetDriverData();
			speed_Coeff = (pDriverData->m_speed.m_frictionCoefficient) * vehicleLateralGrip;
			decel_co_eff = (pDriverData->m_speed.m_brakingCoefficient) * vehicleBrakingGrip * 9.81f;

			// NOTE : This is not needed for this game, even though the weather is defaulted to 1, just being safe..
			// now add in the wetness
			//if (pVehicleStateMy->GetSurfaceFriction() < 1.0f)
			//{
			// wet track
			//	speed_Coeff *= pDriverData->m_weather;
			//	decel_co_eff *= pDriverData->m_weather;
			//}

			myDataState.m_speedCoeff = speed_Coeff;
			myDataState.m_brakingCoeff = decel_co_eff;

			//m_speedCoeff = speed_Coeff;
			//m_brakingCoeff = decel_co_eff;
		}
	}

}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

