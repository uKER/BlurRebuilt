#include "stdafx.h"

#include <recovery/crecoveryai.h>
#include <data/idata.h>
#include <controlsstate/icontrolstate.h>
#include <vehiclestate/ivehiclestate.h>
#include <gameservertime/igameservertime.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

//static Debug::Logger &g_DebugLoggerRecovery = Debug::GetLogger("oldtech.ai.recovery");

namespace airacingtech
{
	// simple recovery... just reverse
	CRecoveryAI::CRecoveryAI() :m_pIData(0), m_pIVehicle(0),m_pIControl(0), m_state(DETECT_RECOVERY), 
								m_lastTime(0.0f), m_ruleFired(0), m_pResetInfo(0), m_lastPollTime(0.0f),
								m_NoRouteLastTime(0.0f), m_NoRoute(false)
	
	{
	}

	CRecoveryAI::~CRecoveryAI()
	{

	}

	void CRecoveryAI::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_RECOVERY_AI, CID_RECOVERY_AI);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_RECOVERY_AI, CRecoveryAI::CreateMe, CRecoveryAI::DestroyMe, CHash("RecoveryAI"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RECOVERY_AI, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_RECOVERY_AI, MT_QUICK_RESTART);	
	}

	IComponent *CRecoveryAI::CreateMe()
	{
		return new CRecoveryAI;
	}

	bool CRecoveryAI::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CRecoveryAI::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CRecoveryAI::Deinit(void)
	{
	}

	EMessageResult CRecoveryAI::HandleMessage(const CMessage &msg)
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
		default:
			break;
		}

		return msgRet;
	}


	EComponentTypeId CRecoveryAI::GetComponentTypeId(void)
	{
		return CID_RECOVERY_AI;
	}

	void CRecoveryAI::QuickRestart()
	{

	}


	void CRecoveryAI::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));
		m_pIVehicle = static_cast<IVehicleState *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_VEHICLE_STATE));
		m_pIControl =  static_cast<IControlState *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_CONTROL_STATE));
	
		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

	
	}


	void CRecoveryAI::Update()
	{
		if (m_pIGameServerTime && m_pIControl && m_pIVehicle && m_pIData)
		{
	
			switch(m_state)
			{
				case DETECT_RECOVERY:
					{
						DetectRecovery();
					}
					break;

				case RECOVERY:
					{
						m_state = RECOVERY_COMPLETE;
					}
					break;

				case RECOVERY_COMPLETE:
					{
						m_state = DETECT_RECOVERY;
						m_ruleFired = 0;
						m_NoRouteLastTime = 0.0f;
						m_NoRoute = false;
					}
					break;

			}
	
		}
	}


	void CRecoveryAI::DetectRecovery()
	{
		if (m_pIGameServerTime->GetTime() >= m_lastTime + 3.0f)
		{
			Maths::HVec3 currentPos = m_pIVehicle->GetState()->GetPosition();

			// we are potentially less than our required speed.....
			if (Maths::Abs(m_pIVehicle->GetState()->GetSpeed()) < 1.0f)
			{
				
				if (currentPos.DistanceSquared(m_lastPosition) < 1.0f)
				{
					if (m_pIData->GetSplineTracker())
					{
						m_pResetInfo = m_pIData->GetSplineTracker()->GetResetInfo();
						if (m_pResetInfo)
						{
							// rule 1, we can be reset to the current route....
							m_ruleFired = 1;
						}
						else
						{
							assert(0);
						}
						
					}
				}
			}
			
			if (m_pIData->GetSplineTracker() && !m_pIData->GetSplineTracker()->GetCurrentRoute())
			{
				// we are not in a route, reset to track in 5 seconds if not back on a route....
				if (m_NoRouteLastTime == 0.0f)
				{
					m_NoRouteLastTime = m_pIGameServerTime->GetTime() + 5.0f;
				}
				else
				{
					if (m_pIGameServerTime->GetTime() > m_NoRouteLastTime)
					{
						m_pResetInfo = m_pIData->GetSplineTracker()->GetResetInfo();
						if (m_pResetInfo)
						{
							// rule 1, we can be reset to the current route....
							m_ruleFired = 2;
							m_NoRouteLastTime = 0.0f;
						}
						else
						{
							assert(0);
						}
					}
				}


				
			}
			else
			{
				m_NoRouteLastTime = 0.0f;
			}

			

			m_lastPosition = m_pIVehicle->GetState()->GetPosition();
			m_lastTime = m_pIGameServerTime->GetTime(); 
		}	
	}




	

	
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif
