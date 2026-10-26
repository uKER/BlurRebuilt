#include "stdafx.h"

#include "GameAudioLayer.h"
//#include "StringHash.h"

#include "AppInterfaceLayer.h"
#include "WheelSubsystem.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF

Debug::Bool gbWheelMessages("Audio.Subsystem.WheelMsgsEnabled", true, Debug::Persist());

Debug::Bool g_doSurfaceInstanceSuspending("Audio.MessageHandler.Wheels.DoSurfaceInstanceSuspend",true);
//-----------------------------------------------------------------------------
// WheelSubsystem::WheelSubsystem
//-----------------------------------------------------------------------------
WheelSubsystem::WheelSubsystem()
{
	m_AllowWheelSuspending = false;
}

//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------
bool WheelSubsystem::ProcessSystemMessage(const U32 u32MessageId)
{

	if(!gbWheelMessages)
		return false;

	switch (u32MessageId)
	{

			case Audio::eBeginLevel:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StartActionIn"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn,false);
				}
			}
			m_AllowWheelSuspending = true;
			break;
		case Audio::eEndLevel:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StopActionIn"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn,false);
				}
			}
			m_AllowWheelSuspending = false;
			break;
		case Audio::eMute:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
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
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
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
				//	Audio::InterfaceController::Get().SuspendEntity(pInstance->GetAudioEntityHandle());
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
bool WheelSubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data, void* pData)
{
	if(!gbWheelMessages)
		return false;

	if( !g_doSurfaceInstanceSuspending || !m_AllowWheelSuspending)
			return false;

	Audio::EntityInstance* pInstance = GetEntityInstance((Audio::EntityInstanceHandle)u32Data);

	if(!pInstance)
		return false;

	switch (u32MessageId)
	{

	//AUDIOTODO: look these instance handles up before hand
	case eFLWheelDry_AudioResumed:
			if (pInstance)
				Audio::InterfaceController::Get().ResumeInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "FLWheelDry"));
		break;

	case eFRWheelDry_AudioResumed:
			if (pInstance)
				Audio::InterfaceController::Get().ResumeInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "FRWheelDry"));
		break;

	case eRLWheelDry_AudioResumed:
			if (pInstance)
				Audio::InterfaceController::Get().ResumeInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "RLWheelDry"));
		break;

	case eRRWheelDry_AudioResumed:
			if (pInstance)
				Audio::InterfaceController::Get().ResumeInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "RRWheelDry"));
		break;

	case eFLWheelDry_AudioStopped:
			if (pInstance)
				Audio::InterfaceController::Get().SuspendInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "FLWheelDry"));
		break;

	case eFRWheelDry_AudioStopped:
			if (pInstance)
				Audio::InterfaceController::Get().SuspendInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "FRWheelDry"));
		break;

	case eRLWheelDry_AudioStopped:
			if (pInstance)
				Audio::InterfaceController::Get().SuspendInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "RLWheelDry"));
		break;

	case eRRWheelDry_AudioStopped:
			if (pInstance)
				Audio::InterfaceController::Get().SuspendInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "RRWheelDry"));
		break;

	case eFLWheelOffRoad_AudioResumed:
			if (pInstance)
				Audio::InterfaceController::Get().ResumeInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "FLWheelOffroad"));
		break;

	case eFRWheelOffRoad_AudioResumed:
			//for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				//Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
		//	if (pInstance)
				Audio::InterfaceController::Get().ResumeInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "FRWheelOffroad"));
			}
		break;

	case eRLWheelOffRoad_AudioResumed:
			if (pInstance)
				Audio::InterfaceController::Get().ResumeInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "RLWheelOffroad"));
		break;

	case eRRWheelOffRoad_AudioResumed:
			if (pInstance)
				Audio::InterfaceController::Get().ResumeInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "RRWheelOffroad"));
		break;

	case eFLWheelOffRoad_AudioStopped:
			if (pInstance)
				Audio::InterfaceController::Get().SuspendInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "FLWheelOffroad"));
		break;

	case eFRWheelOffRoad_AudioStopped:
			if (pInstance)
				Audio::InterfaceController::Get().SuspendInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "FRWheelOffroad"));
		break;

	case eRLWheelOffRoad_AudioStopped:
			if (pInstance)
				Audio::InterfaceController::Get().SuspendInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "RLWheelOffroad"));
		break;

	case eRRWheelOffRoad_AudioStopped:
			if (pInstance)
				Audio::InterfaceController::Get().SuspendInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "RRWheelOffroad"));
		break;


		//ai wheels
	case eAIWheelOffRoad_AudioStopped:
			if (pInstance)
				Audio::InterfaceController::Get().SuspendInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "AIWheelOffroad"));
		break;

	case eAIWheelOffRoad_AudioResumed:
			if (pInstance)
				Audio::InterfaceController::Get().ResumeInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "AIWheelOffroad"));
		break;

	case eAIWheelDry_AudioStopped:
			if (pInstance)
				Audio::InterfaceController::Get().SuspendInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "AIWheelDry"));
		break;

	case eAIWheelDry_AudioResumed:
			if (pInstance)
				Audio::InterfaceController::Get().ResumeInstance(pInstance->GetAudioEntityHandle(),Audio::InterfaceController::Get().LookupInstance(pInstance->GetDefinitionHandle(), "AIWheelDry"));
		break;

		default:
		break;
	}

	return false;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF


