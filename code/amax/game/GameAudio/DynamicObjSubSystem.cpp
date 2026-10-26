#include "stdafx.h"

#include "GameAudioLayer.h"
//#include "StringHash.h"

#include "AppInterfaceLayer.h"
#include "DynamicObjSubsystem.h"
#include "Game\GameActors\Components\AudioComponents.h"
#include "Game/GameActors/Components/Messages/AudioMessages.h"
#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include "GameLogic/Layers/AmaxWorldLayer.h"
#include <camera2/Camera.h>
#include <camera2/stock/DebugFeed.h>

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off)
#endif	//	AUDIO_OPTIMIZE_OFF
//#pragma optimize ("", off)

Debug::Bool gbDynamicObjMessages("Audio.Subsystem.DynamicObjMsgsEnabled", true, Debug::Persist());

Debug::Bool gbTriggerEntities("Audio.Subsystem.DynamicObj.TriggerEntities", false);

Debug::Bool gbDynObjDebug("Audio.Subsystem.DynamicObj.OnscreenDebug", false);


//-----------------------------------------------------------------------------
// DynamicObjSubsystem::DynamicObjSubsystem
//-----------------------------------------------------------------------------
DynamicObjSubsystem::DynamicObjSubsystem()
{
}

//-----------------------------------------------------------------------------
// ProcessSystemMessage
//-----------------------------------------------------------------------------
bool DynamicObjSubsystem::ProcessSystemMessage(const U32 u32MessageId)
{

	if(!gbDynamicObjMessages)
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
					//Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn,false);
				}
			}
			break;
			
		case Audio::eEndLevel:
			for (int i=0; i<C_DynamicEntityList::MAX_NUM_DYNOBJ_INSTANCES; i++)
			{
				if (C_DynamicEntityList::Get().mEntities[i].hEntity.IsValid())
				{
					// instead of sending a message to the actor to ask for the entity to be released just release it here.
					// if an actor with an entity on the list gets destroyed after this point then it will look up the slot
					// but it will return nothing so will do nothing (ie it wont try and release the entity twice)
					Audio::InterfaceController::Get().ReleaseEntity(C_DynamicEntityList::Get().mEntities[i].hEntity);

					C_DynamicEntityList::Get().mEntities[i].Clear();
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
					Audio::InterfaceController::Get().SuspendEntity(pInstance->GetAudioEntityHandle());
				}
			}

			break;

		case Audio::eResume:
			for (U32 i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
			{
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];
				if (pInstance)
				{
					Audio::InterfaceController::Get().ResumeEntity(pInstance->GetAudioEntityHandle());
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
bool DynamicObjSubsystem::ProcessCustomMessage(const U32 u32MessageId, U32 u32Data, void* pData)
{
	if(!gbDynamicObjMessages)
		return false;

	switch (u32MessageId)
	{
		case eSetCallback:
		{
			Audio::EntityInstance* pInstance = GetEntityInstance((Audio::EntityInstanceHandle)u32Data);

			if (pInstance)
			{
				ActiveDynamicEntity* pSlot = C_DynamicEntityList::Get().Lookup(const_cast<AudioEntityHandle&>(pInstance->GetAudioEntityHandle()));

				// if we did not find it in the list it means it will have already been released

				if (pSlot)
				{
					// if this flag is set then the actor has been destroyed
					if (pSlot->bToDestroy)
					{
						//printf("releasing entity 0x%x in subsystem\n", pInstance->GetAudioEntityHandle().ArrayHandle());

						C_DynamicEntityList::Get().RemoveFromList(const_cast<AudioEntityHandle&>(pInstance->GetAudioEntityHandle()));
						Audio::InterfaceController::Get().ReleaseEntity(pInstance->GetAudioEntityHandle());
					}
				}
				else
				{
					//LOGWARN(Game::g_Log, DBSTR" could not lookup entity 0x%x to release", pInstance->GetAudioEntityHandle().ArrayHandle());
				}

				return true;
			}
		}
		break;

		default:
		break;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ProcessCustomMessage
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
void DynamicObjSubsystem::Update()
{
#ifndef CFG_GOLD
	if (gbDynObjDebug)
	{
		// render nice little green square.
		// without this the text doesnt render. dont know why! answers on a postcard...
		Maths::HVec2 pos( 0, 0);
		Maths::HVec2 size( 10.0f,  10.f);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dBar(pos,size,Maths::RGBA(0x00, 0xff, 0x00, 0xff));

		for (int i=0; i<C_DynamicEntityList::MAX_NUM_DYNOBJ_INSTANCES; i++)
		{
			Btl::String actName;
			Btl::String entName;

			Audio::EntityInstance* pInstance = Audio::InterfaceController::Get().GetEntityInstanceFromHandle(C_DynamicEntityList::Get().mEntities[i].hEntity);
			if (pInstance)
			{
				Audio::EntityDefinition* pDef =  Audio::InterfaceController::Get().GetDefinition(pInstance);
				if (pDef)
					entName = pDef->Name();
			}

			ActiveDynamicEntity& entity= C_DynamicEntityList::Get().mEntities[i];

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(100,150+i*20.f), Util::C_String().Printf("hEnt %8x, %8s, doomd %d, t %4.2f, hAct %4x, %20s, d %4.2f", 
				entity.hEntity.ArrayHandle().GetValue(),
				entity.callbackState == 0 ? "unplayed" : entity.callbackState == 1 ? "playing" : "stopped" , 
				entity.bToDestroy, 
				entity.fTimeCreated == FLT_MAX ? 0 : entity.fTimeCreated, 
				entity.hActor.GetIndex(),
				entName.Chars(),
				entity.fDistance), 
				Maths::RGBA(0xff,0xff,0xff,0xff), 14.f);
		}
	}
#endif

	if(!gbDynamicObjMessages)
		return;

	if(gbTriggerEntities)
	{
		for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
		{
			Audio::EntityInstance* pInstance = m_aEntityInstance[i];
			if (pInstance)
			{
				Audio::EntityDataMemberHandle hActionIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), Audio::StringHash("StartActionIn"));
				Audio::InterfaceController::Get().TriggerEntityActionIn(pInstance->GetAudioEntityHandle(), hActionIn,true);
			}
		}
		gbTriggerEntities = false;

	}


	Maths::HMat43 camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Mat43();
	Maths::HVec3 velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed().Live().Velocity();


	//if(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() > 1)//max 4 views
	//{
	//	//int closestView = 0;
	//	int currentView = 0;
	//	//set closest view
	//	
	//	F32 nearestDistance = 1000000; //big value
	//	
	//	for(int i = 0 ; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); i++)
	//	{

	//		Maths::HVec3 entityDistance = ((Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(i).DebugFeed().Mat43().AW())  - (info.pActor->GetMatrix().AW()));
	//		F32 length = Maths::Length(entityDistance);
	//		
	//		if(length < nearestDistance)
	//		{
	//			nearestDistance = length;
	//			currentView = i;
	//		}

	//	}

	//	camMatrix =  Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Mat43();
	//	velocity = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(currentView).DebugFeed().Live().Velocity();

	//}

	if(Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount() == 1)
	{
		for (unsigned int i=0; i<m_aEntityInstance.GetHighestNodeIndex(); i++)
		{
				//Audio::EntityInstance* pInstance = GetEntityInstance((Audio::EntityInstanceHandle)u32Data);
				Audio::EntityInstance* pInstance = m_aEntityInstance[i];

				if (pInstance)
				{
					ActiveDynamicEntity* pSlot = C_DynamicEntityList::Get().Lookup(const_cast<AudioEntityHandle&>(pInstance->GetAudioEntityHandle()));

					// if we did not find it in the list it means it will have already been released

					if (pSlot)
					{
						// if this flag is set then the actor has been destroyed
						if (pSlot->bToDestroy)
						{

							static Audio::StringHash listenerPos = Audio::StringHash("ListenerPosition");
							Audio::EntityDataMemberHandle hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(),listenerPos );
							Audio::InterfaceController::Get().SetEntityVectorIn(pInstance->GetAudioEntityHandle(),hParamIn, (F32*)(&camMatrix.AW()));
							
							static Audio::StringHash listenerDir = Audio::StringHash("ListenerDirection");
							hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), listenerDir);
							Audio::InterfaceController::Get().SetEntityVectorIn(pInstance->GetAudioEntityHandle(), hParamIn, (F32*)(&camMatrix.AZ()));					
							
							static Audio::StringHash listenerUp = Audio::StringHash("ListenerUp");
							hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), listenerUp);
							Audio::InterfaceController::Get().SetEntityVectorIn(pInstance->GetAudioEntityHandle(), hParamIn, (F32*)(&camMatrix.AY()));
							
							static Audio::StringHash listenerVel = Audio::StringHash("ListenerVelocity");
							hParamIn = Audio::InterfaceController::Get().LookupInstanceComponent(pInstance->GetDefinitionHandle(), listenerVel);
							Audio::InterfaceController::Get().SetEntityVectorIn(pInstance->GetAudioEntityHandle(), hParamIn, (F32*)(&velocity));
		
						}
					}
				}
		}
	}



}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif	//	AUDIO_OPTIMIZE_OFF


