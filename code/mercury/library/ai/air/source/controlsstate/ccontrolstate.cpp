#include "stdafx.h"
#include <controlsstate/icontrolstate.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

namespace airacingtech
{
	CControlState::CControlState()
	{
	}

	CControlState::~CControlState()
	{

	}

	void CControlState::RegisterComponentType()
	{
		
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_CONTROL_STATE, CID_CONTROL_STATE);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_CONTROL_STATE, CControlState::CreateMe, CControlState::DestroyMe, CHash("ControlState"));

	}

	IComponent *CControlState::CreateMe()
	{
		return new CControlState;
	}

	bool CControlState::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CControlState::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CControlState::Deinit(void)
	{
		return;
	}

	EMessageResult CControlState::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		return msgRet;
	}


	EComponentTypeId CControlState::GetComponentTypeId(void)
	{
		return CID_CONTROL_STATE;
	}

	void CControlState::PreUpdate()
	{
		SetPreviousState();
		m_State.Reset();
	}
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


