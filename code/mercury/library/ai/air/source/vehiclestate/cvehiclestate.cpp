#include "stdafx.h"
#include <vehiclestate/cvehiclestate.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

namespace airacingtech
{
	CVehicleState::CVehicleState()
	{
	}

	CVehicleState::~CVehicleState()
	{

	}

	void CVehicleState::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_VEHICLE_STATE, CID_VEHICLE_STATE);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_VEHICLE_STATE, CVehicleState::CreateMe, CVehicleState::DestroyMe, CHash("VehicleState"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_STATE, MT_QUICK_RESTART);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_VEHICLE_STATE, MT_STOP_AI);
		
	}

	IComponent *CVehicleState::CreateMe()
	{
		return new CVehicleState;
	}

	bool CVehicleState::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CVehicleState::Init(CObjectId oid, CParameterNode &paramNode)
	{
		std::string *name = static_cast<std::string*>( paramNode.GetData() );
		
		m_vehicleState.SetName(*name);
		return true;
	}

	void CVehicleState::Deinit(void)
	{
	}

	EMessageResult CVehicleState::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;
	
		switch (msg.mType)
		{


			case MT_QUICK_RESTART:
				{
					Maths::HVec3 zeroVec(0.0f,0.0f,0.0f);
					m_vehicleState.SetAngularVelocity(zeroVec);
					m_vehicleState.SetVelocity(zeroVec);
					m_vehicleState.SetTrackSpeed(0.0f);
					m_vehicleState.SetHaveSentConstantsOver(false);
				}
				break;

			case MT_STOP_AI:
				{
					Maths::HVec3 zeroVec(0.0f,0.0f,0.0f);
					m_vehicleState.SetAngularVelocity(zeroVec);
					m_vehicleState.SetVelocity(zeroVec);
					m_vehicleState.SetTrackSpeed(0.0f);
					m_vehicleState.SetHaveSentConstantsOver(false);
				}
				break;




			default:
				break;
		}

		

		return msgRet;
	}


	EComponentTypeId CVehicleState::GetComponentTypeId(void)
	{
		return CID_VEHICLE_STATE;
	}
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


