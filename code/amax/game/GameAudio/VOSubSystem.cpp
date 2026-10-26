#include "stdafx.h"

#include "GameAudioLayer.h"

#include <GameApp.h>
#include "AppInterfaceLayer.h"
#include "VOSubsystem.h"
#include "XtAudio.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF

Debug::Bool gbVOMessages("Audio.Subsystem.VOMsgsEnabled", true, Debug::Persist());

//-----------------------------------------------------------------------------
// VOSubsystem::VOSubsystem
//-----------------------------------------------------------------------------
VOSubsystem::VOSubsystem()
{
}

//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------
bool VOSubsystem::ProcessSystemMessage(const U32 u32MessageId)
{
	if(!gbVOMessages)
		return false;

	switch (u32MessageId)
	{
		////case Audio::eInit:
		////{
		////	// create a VO entity
		////	Util::C_String string = "VO";
		////	Audio::XtEntityDef* pXtEntity = Audio::C_XtAudioDb::Get()->LookupXtEntity(string);
		////	if (pXtEntity)
		////	{
		////		pXtEntity->SetupEntityType();

		////		if (pXtEntity->GetEntityTypeHnd().IsValid())
		////		{
		////			AudioEntityHandle hEntity = Audio::InterfaceController::Get().CreateEntity(pXtEntity->GetEntityTypeHnd());
		////			Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(hEntity, "StartActionIn");

		////			// connect to renderer
		////			Audio::EntityDefinition* pDef = Audio::InterfaceController::Get().GetEntityManager().GetDefinitionFromHandle(pXtEntity->GetEntityDefHnd());
		////			if (pDef && pDef->LookupInstanceComponent("OutputToRenderer") != 0xffffffff)
		////				Audio::InterfaceController::Get().SetMaster(hEntity, "OutputToRenderer");

		////			Audio::InterfaceController::Get().TriggerEntityActionIn(hEntity, hActionIn);

		////			// temp. store handle in AudioSystem class
		////			C_GameApp::GetInstance()->GetGameAudioLayer()->SetActorEntityHnd(Game::C_GameAudioLayer::eVOSubsystem,hEntity);
		////		}
		////	}
		////	
		////}
		////break;
		case Audio::eBeginLevel:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::InterfaceController::Get().ResumeEntity(pInstance->GetAudioEntityHandle());
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StartActionIn"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);
				}
			}
			break;
		case Audio::eEndLevel:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("RaceFinished"));
					Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn);
				}
			}
			break;
		case Audio::eMute:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,0);
					Audio::InterfaceController::Get().SuspendEntity(pInstance->GetAudioEntityHandle());
				}
			}
			break;

		case Audio::eUnmute:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::InterfaceController::Get().ResumeEntity(pInstance->GetAudioEntityHandle());
					Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("GamePaused"));
					Audio::InterfaceController::Get().SetEntityValue(pInstance->GetAudioEntityHandle(), hParamIn,1);
				}
			}
			break;
		default:
			break;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ProcessCustomMessage
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool VOSubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data, void* pData)
{
	if(!gbVOMessages)
		return false;

	return true;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF


