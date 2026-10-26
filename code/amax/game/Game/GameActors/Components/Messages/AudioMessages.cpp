#include "stdafx.h"

#include <misc_shared/Messaging/MessageBase.h>
#include "Game/GameActors/Components/Messages/AudioMessages.h"

#include <actor_system/actor.h>
#include <shared_actors/Components/Creation/MessageComponentsCreate.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorMessages.h>
#include <shared_actors/XtSharedLevelActorParams.h>

namespace ActorMsg
{
// C_AudioSetParameterMsg Serialisation Functions
Messaging::C_MessageSerialiser C_AudioSetParameterMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

void C_AudioSetParameterMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const ActorMsg::C_AudioSetParameterMsg* pMsg = Messaging::MessageCast<C_AudioSetParameterMsg>(pMessage);
	stream << pMsg->MsgID;
	stream << pMsg->fValue;
}

void C_AudioSetParameterMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	ActorMsg::C_AudioSetParameterMsg* pMsg = static_cast<C_AudioSetParameterMsg*>(pMessage);
	stream >> pMsg->MsgID;
	stream >> pMsg->fValue;
}

// C_AudioSetVectorMsg Serialisation Functions
Messaging::C_MessageSerialiser C_AudioSetVectorMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

void C_AudioSetVectorMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const ActorMsg::C_AudioSetVectorMsg* pMsg = Messaging::MessageCast<C_AudioSetVectorMsg>(pMessage);
	stream << pMsg->MsgID;
	stream << pMsg->vValue;
}

void C_AudioSetVectorMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	ActorMsg::C_AudioSetVectorMsg* pMsg = static_cast<C_AudioSetVectorMsg*>(pMessage);
	stream >> pMsg->MsgID;
	stream >> pMsg->vValue;
}

// C_AudioTriggerActionFromHandleMsg Serialisation Functions
Messaging::C_MessageSerialiser C_AudioTriggerActionFromHandleMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

void C_AudioTriggerActionFromHandleMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const ActorMsg::C_AudioTriggerActionFromHandleMsg* pMsg = Messaging::MessageCast<C_AudioTriggerActionFromHandleMsg>(pMessage);
	stream << pMsg->m_uEntityDataMemberHnd;
}

void C_AudioTriggerActionFromHandleMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_AudioTriggerActionFromHandleMsg* pMsg = static_cast<C_AudioTriggerActionFromHandleMsg*>(pMessage);
	stream >> pMsg->m_uEntityDataMemberHnd;
}

// C_AudioTriggerActionFromStrHashMsg Serialisation Functions
Messaging::C_MessageSerialiser C_AudioTriggerActionFromStrHashMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

void C_AudioTriggerActionFromStrHashMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const ActorMsg::C_AudioTriggerActionFromStrHashMsg* pMsg = Messaging::MessageCast<C_AudioTriggerActionFromStrHashMsg>(pMessage);
	stream << pMsg->m_uStringHash;
}

void C_AudioTriggerActionFromStrHashMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	ActorMsg::C_AudioTriggerActionFromStrHashMsg* pMsg = static_cast<C_AudioTriggerActionFromStrHashMsg*>(pMessage);
	stream >> pMsg->m_uStringHash;
}


// C_AudioSetParamFromStrHashMsg Serialisation Functions
Messaging::C_MessageSerialiser C_AudioSetParamFromStrHashMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

void C_AudioSetParamFromStrHashMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const ActorMsg::C_AudioSetParamFromStrHashMsg* pMsg = Messaging::MessageCast<C_AudioSetParamFromStrHashMsg>(pMessage);
	stream << pMsg->m_uStringHash;
	stream << pMsg->m_fParam;
}

void C_AudioSetParamFromStrHashMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	ActorMsg::C_AudioSetParamFromStrHashMsg* pMsg = static_cast<C_AudioSetParamFromStrHashMsg*>(pMessage);
	stream >> pMsg->m_uStringHash;
	stream >> pMsg->m_fParam;
}

// C_AudioSetParamFromStrHashMsg Serialisation Functions
Messaging::C_MessageSerialiser C_AudioConnectEntityMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

void C_AudioConnectEntityMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const ActorMsg::C_AudioConnectEntityMsg* pMsg = Messaging::MessageCast<C_AudioConnectEntityMsg>(pMessage);
	//stream << pMsg->m_EntityInstanceHandle;
}

void C_AudioConnectEntityMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	ActorMsg::C_AudioConnectEntityMsg* pMsg = static_cast<C_AudioConnectEntityMsg*>(pMessage);
	//stream >> pMsg->m_EntityInstanceHandle;
}

// C_AudioPlayDialogueMsg Serialisation Functions
Messaging::C_MessageSerialiser C_InstanceAudioStoppedMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

void C_InstanceAudioStoppedMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_InstanceAudioStoppedMsg* pMsg = Messaging::MessageCast<C_InstanceAudioStoppedMsg>(pMessage);
	stream << pMsg->m_sInstanceId;
}

void C_InstanceAudioStoppedMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_InstanceAudioStoppedMsg* pMsg = static_cast<C_InstanceAudioStoppedMsg*>(pMessage);
	stream >> pMsg->m_sInstanceId;
}

// Xt Message Annotations
class C_XtAudioActionMessage : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtAudioActionMessage(){ }
	virtual const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext) 
	{
		if(m_Msg.m_uStringHash == Audio::StringHash::Invalid)
			m_Msg.m_uStringHash = Audio::StringHash(m_sz8Name).GetValue();

		return &m_Msg;
	}

	XtSz8 m_sz8Name;
	C_AudioTriggerActionFromStrHashMsg m_Msg;
};

class C_XtAudioMessageSetParameter : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtAudioMessageSetParameter(){ }
	virtual const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext) 
	{
		if(m_Msg.m_uStringHash == Audio::StringHash::Invalid)
			m_Msg.m_uStringHash = Audio::StringHash(m_sz8Name).GetValue();

		if (m_ParamValue)
			m_Msg.m_fParam = m_ParamValue->GetFloat(pActorContext);

		return &m_Msg;
	}

	XtSz8 m_sz8Name;
	Actors::C_XtParamFloat*  m_ParamValue;

	C_AudioSetParamFromStrHashMsg m_Msg;
};

// Audio Wait for Event Action

// Play Spoken Dialogue Script Action
class C_MessageScriptWaitForAudioEventAction : public Components::C_MessageScriptWaitActionItem
{
public:
	virtual bool	DoAction(const Actor::C_Actor *pActorContext) const
	{
		// Send message to audio actor
		C_AudioWaitForEventMsg waitForEventMsg;
		waitForEventMsg.SetValue(Audio::StringHash(m_EventName).GetValue());
		waitForEventMsg.SetActorHandle(pActorContext->GetHandle());

		pActorContext->SendMessageToActor(m_pActionActor->GetActorHandle(), &waitForEventMsg);


		return true;
	}

	XtSz8								m_EventName;	// event name to wait for
};

}//end namespace ActorMsg

XT_TYPE(ActorMsg::C_XtAudioActionMessage, "AudioMessageAction")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_sz8Name, "ActionName")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtAudioMessageSetParameter, "AudioMessageSetParameter")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_sz8Name, "ParameterName")
XT_FIELD(m_ParamValue, "ParamValue")
XT_END_TYPE

XT_TYPE(ActorMsg::C_MessageScriptWaitForAudioEventAction, "MessageScriptWaitForAudioEventAction")
XT_BASE(Components::C_MessageScriptWaitActionItem)
XT_FIELD(m_EventName, "EventName")
XT_END_TYPE


XT_LIBRARY(AudioActorMessages)
XT_REGISTER_TYPE(ActorMsg::C_XtAudioActionMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtAudioMessageSetParameter)
XT_REGISTER_TYPE(ActorMsg::C_MessageScriptWaitForAudioEventAction)
XT_END_LIBRARY