#include "stdafx.h"

#include "GameAudioLayer.h"
//#include "StringHash.h"

#include "AppInterfaceLayer.h"
#include "VehicleSubsystem.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF
//#pragma optimize ("", off)

Debug::Bool gbVehicleMessages("Audio.Subsystem.VehicleMsgsEnabled", true, Debug::Persist());
//-----------------------------------------------------------------------------
// VehicleSubsystem::VehicleSubsystem
//-----------------------------------------------------------------------------
VehicleSubsystem::VehicleSubsystem()
{
}

//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------
bool VehicleSubsystem::ProcessSystemMessage(const U32 u32MessageId)
{

	if(!gbVehicleMessages)
		return false;

	switch (u32MessageId)
	{

			case Audio::eBeginLevel:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StartActionIn"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn,false);
				}
			}
			break;
		case Audio::eEndLevel:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StopActionIn"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn,false);
				}
			}
			break;
		case Audio::eMute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,0);
				}
			}
			break;

		case Audio::eUnmute:
			for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,1);
				}
			}
			break;

		case Audio::eSuspend:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					//Audio::InterfaceController::Get().SuspendEntity(pInstance->GetAudioEntityHandle());
				}
			}

			break;

		case Audio::eResume:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					//Audio::InterfaceController::Get().ResumeEntity(pInstance->GetAudioEntityHandle());
				}
			}

			break;
		
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ProcessCustomMessage
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VehicleSubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data, void* pData)
{
	if(!gbVehicleMessages)
		return false;

	//switch (u32MessageId)
	//{
	//	/*case eSetCallback:
	//	{
	//		Audio::EntityInstance* pInstance = GetEntityInstance((Audio::EntityInstanceHandle)u32Data);

	//		if (pInstance)
	//		{
	//			Audio::InterfaceController::Get().SetCallback(pInstance->GetAudioEntityHandle(), m_hTriggerExhaustActionOut, eTriggerExhaustAction);
	//			return true;
	//		}
	//	}
	//	break;*/

	//	default:
	//	break;
	//}

	return false;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF


