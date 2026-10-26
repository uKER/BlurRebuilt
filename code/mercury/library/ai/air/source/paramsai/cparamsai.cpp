#include "stdafx.h"

#include <paramsai/cparamsai.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif


namespace airacingtech
{

	CParamsAI::CParamsAI() 
	{
	}

	CParamsAI::~CParamsAI()
	{

	}

	void CParamsAI::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PARAMS_AI, CID_PARAMS_AI);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PARAMS_AI, CParamsAI::CreateMe, CParamsAI::DestroyMe, CHash("ParamsAI"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PARAMS_AI, MT_RELOAD_DRIVER_DATA);
	}

	IComponent *CParamsAI::CreateMe()
	{
		return new CParamsAI;
	}

	bool CParamsAI::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}

	// Virtual IComponent methods
	bool CParamsAI::Init(CObjectId oid, CParameterNode &paramNode)
	{
		DriverData *pData = static_cast<DriverData *>( paramNode.GetData() );
		if (pData)
		{
			CopyDriverData(pData);
		}

		return true;
	}

	void CParamsAI::Deinit(void)
	{
	}

	EMessageResult CParamsAI::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch(msg.mType)
		{
			case MT_RELOAD_DRIVER_DATA:
				{
					DriverData *pNewData = static_cast<DriverData *>(msg.mpData);
					if (pNewData)
					{
						CopyDriverData(pNewData);
					}
				}
				break;

			default:
				break;
		}

		return msgRet;
	}


	EComponentTypeId CParamsAI::GetComponentTypeId(void)
	{
		return CID_PARAMS_AI;
	}

	void CParamsAI::CopyDriverData(DriverData *pNew)
	{
		m_Params = *pNew;
	}
}


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

