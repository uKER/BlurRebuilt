#include "stdafx.h"

#include <introoutroSplineAI/CIntroOutroSplineAI.h>
#include <spline/ispline.h>



#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif


namespace airacingtech
{
	
	CIntroOutroSplineAI::CIntroOutroSplineAI() : m_steering(0.0f), m_pISpline(0), m_speed(0.0f), m_pIData(0)
	{
		m_state = CIntroOutroSplineAI::STATE_NONE;
	}

	CIntroOutroSplineAI::~CIntroOutroSplineAI()
	{

	}

	void CIntroOutroSplineAI::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_INTRO_OUTRO_SPLINE_AI, CID_INTRO_OUTRO_SPLINE_AI);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_INTRO_OUTRO_SPLINE_AI, CIntroOutroSplineAI::CreateMe, CIntroOutroSplineAI::DestroyMe, CHash("IntroOutroSplineAI"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_INTRO_OUTRO_SPLINE_AI, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_INTRO_OUTRO_SPLINE_AI, MT_SET_INTRO_OUTRO_SPLINE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_INTRO_OUTRO_SPLINE_AI, MT_SET_INTRO_OUTRO_SPEED);	
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_INTRO_OUTRO_SPLINE_AI, MT_SET_INTRO_OUTRO_TARGET_POINT);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_INTRO_OUTRO_SPLINE_AI, MT_SET_INTRO_OUTRO_DISABLE_AI);
	}

	IComponent *CIntroOutroSplineAI::CreateMe()
	{
		return new CIntroOutroSplineAI;
	}

	bool CIntroOutroSplineAI::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CIntroOutroSplineAI::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CIntroOutroSplineAI::Deinit(void)
	{
	}

	EMessageResult CIntroOutroSplineAI::HandleMessage(const CMessage &msg)
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


			case MT_SET_INTRO_OUTRO_SPLINE:
				{
					// get the spline info out....
					IntroOutroSetSplineParams *pParams = static_cast<IntroOutroSetSplineParams *>(msg.mpData);
					if (pParams)
					{
						m_pISpline = static_cast<ISplineAI *>(GlobalsComponent::GetObjectManager().QueryInterface(pParams->m_splineName, IID_SPLINE_AI));
					
						m_state = STATE_SPLINE_FOLLOWING;
					}
					
					
				}
				break;

			case MT_SET_INTRO_OUTRO_SPEED:
				{
					IntroOutroSetSpeedParams *pParams = static_cast<IntroOutroSetSpeedParams *>(msg.mpData);
					if (pParams && m_pIData)
					{
						m_speed = MPHTOMS(pParams->m_speed);
						m_pIData->GetState().m_trackSpeed = m_speed;
			
					}
				}
				break;

			case MT_SET_INTRO_OUTRO_TARGET_POINT:
				{
					IntroOutroSetTargetPointParams *pParams = static_cast<IntroOutroSetTargetPointParams *>(msg.mpData);
					if (pParams && m_pIData)
					{
						m_state = CIntroOutroSplineAI::STATE_FOLLOW_TARGET_POINT;
						m_targetPoint = pParams->m_targetPoint;
					}	
				}
				break;

			case MT_SET_INTRO_OUTRO_DISABLE_AI:
				{
					m_state = CIntroOutroSplineAI::STATE_NONE;				
				}
				break;

			default:
				break;
		}

		return msgRet;
	}


	EComponentTypeId CIntroOutroSplineAI::GetComponentTypeId(void)
	{
		return CID_INTRO_OUTRO_SPLINE_AI;
	}

	

	void CIntroOutroSplineAI::CachedInterfaces()
	{
		CObjectId id = GetObjectId();	
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));		
	}


	void CIntroOutroSplineAI::Update()
	{
		//if (m_pISpline && m_pIData && m_pIData->GetSplineFollower())
		if (m_pIData)
		{

			IRubberbandingAI *pRubberBanding = m_pIData->GetRubberBanding();
			if (pRubberBanding)
			{
				pRubberBanding->SetDirectTargetSpeed(-1.0f);
			}


			switch(m_state)
			{
				case CIntroOutroSplineAI::STATE_NONE:
					{
						m_steering = 0.0f;
						m_pIData->GetState().m_trackSpeed = 0.0f;
					}
					break;

				case CIntroOutroSplineAI::STATE_SPLINE_FOLLOWING:
					{
						if (m_pISpline && m_pIData && m_pIData->GetSplineFollower())
						{
							m_pIData->GetSplineFollower()->Update(m_pISpline);
							m_steering = m_pIData->GetSplineFollower()->GetSteering();
							m_pIData->GetState().m_trackSpeed = m_speed;
						}
						else
						{
							m_pIData->GetState().m_trackSpeed = 0.0f;
						}
						
						
						
					}
					break;

				case CIntroOutroSplineAI::STATE_FOLLOW_TARGET_POINT:
					{
						if (m_pISpline && m_pIData && m_pIData->GetSplineFollower())
						{
							m_pIData->GetSplineFollower()->UpdateTargetPoint(m_targetPoint);
							m_steering = m_pIData->GetSplineFollower()->GetSteering();
							m_pIData->GetState().m_trackSpeed = m_speed;
						}
						else
						{
							m_pIData->GetState().m_trackSpeed = 0.0f;
						}
					}
					break;

				default:
					{
						m_steering = 0.0f;
						m_speed = 0.0f;
						
						
					}
					break;

			}

			m_pIData->GetState().m_trackSpeed = m_speed;
			
		}
	}

	float CIntroOutroSplineAI::GetSteering() const
	{
		return m_steering;
	}
	
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif
