#include "SharedActorsPch.h"

#include <algorithm>

#include <actor_system/Actor.h>

#include <shared_actors/Components/Impl/AnimationController.h>
#include <shared_actors/Components/Messages/AnimationMessages.h>
#include <shared_actors/XtLevelAnimation.h>

Debug::Bool 	g_DebugAnimationControllerComponentRenderDebug		("ActorDebug.Components.AnimationController.DebugRender.Enable",				false);

// uncomment this if you have problems
#define USE_NEW_MESSAGES

namespace Components
{


bool C_AnimationControllerMessageItem::Send(const Actor::C_Actor *pActorContext) const
{
	//pActorContext->SendActorMessage()
	//return m_pMessageItem->m_Message.Send(pActorContext);
	BIZ_ASSERT(pActorContext);
	if(m_pMessageItem && m_pMessageItem->m_Message && m_DestActor)
		pActorContext->SendMessageToActor(m_DestActor->GetActorHandle(), m_pMessageItem->m_Message->GetMessage(pActorContext));

	return false;	// TODO
}

C_AnimationController::C_AnimationController():m_TValue(0),m_CurrentMsgIndex(0)
{

}

bool C_AnimationController::CreateImpl()
{
	if(GetAnimationClip() == NULL)
		return false;

	// Send messages to all the slaves telling them who their master is
	// TODO: implement translation table from component creation data
	for(U32 slaveNo = 0; slaveNo < GetCreationData()->GetNumSlaves(); slaveNo++)
	{
		//const Actors::C_GameActorCreate *pActor = GetAnimationClip()->m_AnimationObjects[slaveNo].m_Actor;

		//if(pActor)
		{
			ActorMsg::C_AnimationSetControllerMsg setControllerMsg;
			setControllerMsg.SetActorHandle(GetActorHandle());
			setControllerMsg.SetValue(slaveNo);

			SendMessageToActor(GetCreationData()->GetSlaveActorHandle(slaveNo), &setControllerMsg);
		}
	}

	// setup message list
	for (S32 objectNumber = 0; objectNumber < GetAnimationClip()->m_AnimationObjects.count; objectNumber++)
	{

		for(S32 channelNo =0; channelNo < GetAnimationClip()->m_AnimationObjects[objectNumber].m_AnimationChannels.count; channelNo++)
		{
			if( 
				GetAnimationClip()->m_AnimationObjects[objectNumber].m_AnimationChannels[channelNo] &&
				GetAnimationClip()->m_AnimationObjects[objectNumber].m_AnimationChannels[channelNo]->m_ChannelID == Level::E_AnimationChannelID_Message
				)
			{
				//const Level::C_XtAnimationMessageChannel * pMessageChannel = (const Level::C_XtAnimationMessageChannel * )GetAnimationClip()->m_AnimationObjects[objectNumber].GetAnimationChannel(Level::E_AnimationChannelID_Message);
				const Level::C_XtAnimationMessageChannel * pMessageChannel = (const Level::C_XtAnimationMessageChannel * )GetAnimationClip()->m_AnimationObjects[objectNumber].m_AnimationChannels[channelNo];
				if(pMessageChannel)
				{
					// add messages to list
					for(S32 messageNo = 0;messageNo < pMessageChannel->m_Messages.count; messageNo++)
					{
						m_MessageItems.PushBack(C_AnimationControllerMessageChannelItem(&pMessageChannel->m_Messages[messageNo]));

						m_NewMessageItems.PushBack(
							C_AnimationMessageListItem(new C_AnimationControllerMessageChannelItem(&pMessageChannel->m_Messages[messageNo]))
							);
					}
				}
			}
		}
	}


	// sort list
	std::sort(m_MessageItems.Begin(),m_MessageItems.End());

	// TODO: Setup & sort new message list
	for (S32 objectNumber = 0; objectNumber < GetAnimationClip()->m_AnimationObjects.count; objectNumber++)
	{
		const XtArray<Level::C_XtAnimationMessageItem> & messageArray = GetAnimationClip()->m_AnimationObjects[objectNumber].m_AnimationMessages;

		// add messages to list
		for(S32 messageNo = 0;messageNo < messageArray.count; messageNo++)
		{
			m_NewMessageItems.PushBack(
				C_AnimationMessageListItem(
					new C_AnimationControllerMessageItem(
						&messageArray[messageNo],
						GetAnimationClip()->m_AnimationObjects[objectNumber].m_Actor)
					)
			);
		}
	}


	// sort list
	std::sort(m_NewMessageItems.Begin(),m_NewMessageItems.End());

	m_CurrentMsgIndex = GetMessageIndexFromTValue(m_TValue);

	return true;
}

void C_AnimationController::KillImpl()
{
	// TODO: Clean up new messages
	for(S32 messageNo = 0; messageNo < m_NewMessageItems.Size(); messageNo++)
	{
		SAFE_DELETE_PTR(m_NewMessageItems[messageNo].m_pItem);
	}
}


// Send the Current Tvalue to all animation slaves
bool C_AnimationController::SendSlaveObjectsTValues(bool bMoveTo)
{
	bool slaveSent = false;

	for(U32 objectNo = 0; objectNo < GetCreationData()->GetNumSlaves(); objectNo++)
	{
		Level::C_AnimTValue Tvalue;
		const Level::C_XtAnimationObject &animObj = GetAnimationClip()->m_AnimationObjects[objectNo];

		if(animObj.GetSlaveTValueAtTValue(m_TValue,Tvalue))
		{
			if(animObj.m_Actor)
			{
				if(bMoveTo)
				{
					const ActorMsg::C_AnimationMoveToTValueMsg moveToTValueMsg(Tvalue);
					SendMessageToActor(GetCreationData()->GetSlaveActorHandle(objectNo), &moveToTValueMsg);
				}
				else
				{
					const ActorMsg::C_AnimationSetTValueMsg setTValueMsg(Tvalue);
					SendMessageToActor(GetCreationData()->GetSlaveActorHandle(objectNo), &setTValueMsg);
				}
	
				slaveSent = true;
			}
		}
	}

	return slaveSent;
}

bool C_AnimationController::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
		case ActorMsg::E_AnimationMsg_SetTValue:
			m_TValue = Messaging::MessageCast<ActorMsg::C_AnimationSetTValueMsg>(pMessage)->GetValue();
			m_CurrentMsgIndex = GetMessageIndexFromTValue(m_TValue);
			SendSlaveObjectsTValues(false);
			break;
		case ActorMsg::E_AnimationMsg_MoveToTValue:
		{
			 F32 newTValue = Messaging::MessageCast<ActorMsg::C_AnimationMoveToTValueMsg>(pMessage)->GetValue();
#ifdef USE_NEW_MESSAGES
			 if(m_CurrentMsgIndex != m_NewMessageItems.Size())	// at the end
			 {
				 while(m_NewMessageItems[m_CurrentMsgIndex].GetTValue() <= newTValue)
				 {		
					 m_NewMessageItems[m_CurrentMsgIndex].Send(GetActor());
					 m_CurrentMsgIndex++;
					 // have we reached the end?
					 if(m_CurrentMsgIndex == m_NewMessageItems.Size())
						 break;
				 }
			 }

#else
			 if(m_CurrentMsgIndex != m_MessageItems.Size())	// at the end
			 {
				 while(m_MessageItems[m_CurrentMsgIndex].GetTValue() <= newTValue)
				 {		
					 m_MessageItems[m_CurrentMsgIndex].Send(GetActor());
					 m_CurrentMsgIndex++;
					 // have we reached the end?
					 if(m_CurrentMsgIndex == m_MessageItems.Size())
						 break;
				 }
			 }
#endif
			 m_TValue = newTValue;
			 SendSlaveObjectsTValues(true);
		}
		break;
		default:
			return I_AnimationController::OnMessage(pMessage);
	}
	return true;
}

U32	C_AnimationController::GetMessageIndexFromTValue(F32 tValue) const
{
#ifdef USE_NEW_MESSAGES
	for(S32 messageNo = 0; messageNo < m_NewMessageItems.Size(); messageNo++)
	{
		if(m_NewMessageItems[messageNo].GetTValue() >= tValue)
			return messageNo;
	}
#else
	for(S32 messageNo = 0; messageNo < m_MessageItems.Size(); messageNo++)
	{
		if(m_MessageItems[messageNo].GetTValue() >= tValue)
			return messageNo;
	}
#endif

	return 0;
}


// Interface Implementation
F32	C_AnimationController::GetAnimationLength() const
{
	return GetAnimationClip()->m_ClipLength;
}

// const Level::C_XtAnimationChannel*	C_AnimationController::GetObjectChannel(const S32 objectID, U32 channelID) const
// {
// 	return GetAnimationClip()->GetObjectChannel(objectID,channelID);
// }

const bool	C_AnimationController::GetObjectTranslation(S32 objectID, Maths::C_Vertex &translation) const
{
	if(GetAnimationClip()->GetObjectTranslation(objectID,m_TValue,translation))
	{
		translation = translation * GetActor()->GetMatrix();
		return true;
	}
	else
		return false;

}

const bool	C_AnimationController::GetObjectLocalTranslation(const S32 objectID, Maths::C_Vertex &translation) const
{
	return GetAnimationClip()->GetObjectTranslation(objectID,m_TValue,translation);
}

const bool	C_AnimationController::GetObjectRotation(const S32 objectID, Maths::C_Quaternion &rotation) const
{
	if(GetAnimationClip()->GetObjectRotation(objectID,m_TValue,rotation))
	{
		rotation = rotation * Maths::C_Quaternion(GetActor()->GetMatrix().Rotation());
		return true;
	}
	else
		return false;
}

const bool	C_AnimationController::GetObjectLocalRotation(const S32 objectID, Maths::C_Quaternion &rotation) const
{
	return GetAnimationClip()->GetObjectRotation(objectID,m_TValue,rotation);
}

const bool	C_AnimationController::GetCameraFOV(const S32 objectID, F32 &fValue) const
{
	return GetAnimationClip()->GetCameraFOV(objectID,m_TValue,fValue);
}

const S32	C_AnimationController::GetNamedFloatCurveChannelID(const S32 objectID, const Util::C_String &name) const
{
	const Level::C_XtAnimationObject *pAnimObject = GetAnimationClip()->GetAnimationObject(objectID);

	if(pAnimObject)
		return pAnimObject->GetNamedFloatAnimationChannelID(name);
	else
		return -1;
}

const bool	C_AnimationController::GetNamedFloatCurveValue(const S32 objectID, const S32 channelID, F32 &fValue) const
{
	const Level::C_XtAnimationObject *pAnimObject = GetAnimationClip()->GetAnimationObject(objectID);

	if(pAnimObject)
		return pAnimObject->GetNamedFloatValueAtTValue(m_TValue,channelID,fValue);
	else
		return false;
}

const U32	C_AnimationController::GetNumberOfNamedCurves(const S32 objectID) const
{
	const Level::C_XtAnimationObject *pAnimObject = GetAnimationClip()->GetAnimationObject(objectID);

	if(pAnimObject)
		return pAnimObject->GetNumNamedFloatAnimationChannels();

	return 0;
}

const char*	C_AnimationController::GetNamedCurveName(const S32 objectID, U32 index) const
{
	const Level::C_XtAnimationObject *pAnimObject = GetAnimationClip()->GetAnimationObject(objectID);

	if(pAnimObject)
	{
		const Level::C_XtNamedFloatAnimationChannel *pChannel = pAnimObject->GetNamedFloatAnimationChannelFromIndex(index);

		if(pChannel)
			return pChannel->m_Name.Text();
	}

	return NULL;
}


#ifndef CFG_GOLD
void C_AnimationController::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if(g_DebugAnimationControllerComponentRenderDebug == false)
		return;

	const Level::C_XtAnimationClip* clip = GetAnimationClip();
	if (!clip)
		return;
	
	Maths::HMat43 matrix;
	if (GetActor())
		matrix = GetActor()->GetMatrix();

	clip->DebugRender(debugRenderPtr, matrix);
}
#endif

// Serialisation Functions
void C_AnimationController::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_AnimationController::SerialiseForSave(stream);
	stream << m_TValue;
	stream << m_CurrentMsgIndex;

}

void C_AnimationController::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_AnimationController::DeserialiseForSave(stream);
	stream >> m_TValue;
	stream >> m_CurrentMsgIndex;
}

void C_AnimationController::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_AnimationController::SerialiseForSync(stream);
	stream << m_TValue;
	stream << m_CurrentMsgIndex;
}

void C_AnimationController::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_AnimationController::DeserialiseForSync(stream);
	stream >> m_TValue;
	stream >> m_CurrentMsgIndex;
}


}//namespace Components
