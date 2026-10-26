#include "stdafx.h"
#include <pidcontrollerai/cpidcontrollerai.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif


namespace airacingtech
{
	CPIDControllerAI::CPIDControllerAI()
	{
	}

	CPIDControllerAI::~CPIDControllerAI()
	{

	}

	void CPIDControllerAI::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PID_CONTROLLER, CID_PID_CONTROLLER);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PID_CONTROLLER, CPIDControllerAI::CreateMe, CPIDControllerAI::DestroyMe, CHash("PIDController"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PID_CONTROLLER, MT_QUICK_RESTART);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PID_CONTROLLER, MT_RELOAD_PID_DATA);
		
	}

	IComponent *CPIDControllerAI::CreateMe()
	{
		return new CPIDControllerAI;
	}

	bool CPIDControllerAI::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPIDControllerAI::Init(CObjectId oid, CParameterNode &paramNode)
	{
		PIDControllerInitialiseParams *pParams = static_cast<PIDControllerInitialiseParams *>( paramNode.GetData() );

		m_Error.Clear();

		SetCoefficients(pParams->p,pParams->i,pParams->d);
		SetNumSlots(pParams->m_numSlots);
		m_Timestep = pParams->m_timeStep;

		m_Error.Reserve(pParams->m_numSlots);

		for(int i=0;i<m_NUM_SLOTS; i++)
		{
			m_Error.PushBack(0.0f);
		}

		Clear();


		return true;
	}

	void CPIDControllerAI::Deinit(void)
	{
		m_Error.Destroy();
	}

	EMessageResult CPIDControllerAI::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

#if 0
		switch (msg.mType)
		{
			//case MT_SET_PID_DATA:
			//	{
			//		PIDControllerInitialiseParams *pData = static_cast<PIDControllerInitialiseParams *>(msg.mpData);
			//		SetCoefficients(pData->p, pData->i, pData->d);
			//		msgRet =  MR_TRUE;
			//	}
			//	break;

			default:
				break;
		}
#endif

		switch (msg.mType)
		{
			case MT_QUICK_RESTART:
				{
					QuickRestart();
				}
				break;

			case MT_RELOAD_PID_DATA:
				{
					PIDControllerInitialiseParams *pNewData = static_cast<PIDControllerInitialiseParams *>(msg.mpData);
					if (pNewData)
					{
						CopyData(pNewData);
					}
				}
				break;

			default:
				break;
		}

		return msgRet;
	}


	void CPIDControllerAI::CopyData(PIDControllerInitialiseParams *pData)
	{
		m_Timestep = pData->m_timeStep;
		SetCoefficients(pData->p,pData->i,pData->d);
	}


	EComponentTypeId CPIDControllerAI::GetComponentTypeId(void)
	{
		return CID_PID_CONTROLLER;
	}

	void CPIDControllerAI::QuickRestart()
	{

	}
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


