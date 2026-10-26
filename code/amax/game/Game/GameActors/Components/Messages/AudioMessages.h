#ifndef AUDIO_MESSAGES_H
#define AUDIO_MESSAGES_H

#ifndef NOT_USING_AUDIO_COMPONENETS

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/uid/FourCC.h>
#include "misc_shared/Messaging/MessageValue.h"
#include "StringHash.h"
#include "AppInterfaceLayer.h"

#include <actor_system/ActorMessageValue.h>

#include <xt/xtgame.h>

namespace Audio
{
	struct XtEntityProxy;
}

namespace ActorMsg
{

enum E_AudioMsg
{
	E_AudioMsg_HiBits = FOURCC_HI20('AUDM'), 

	E_AudioMsg_Event,
	E_AudioMsg_SetParameter,
	E_AudioMsg_SetVector,

	E_AudioMsg_TriggerActionFromHandle,

	// used only from actor studio
	E_AudioMsg_TriggerActionFromStrHash,
	E_AudioMsg_SetParamFromStrHash,
	E_AudioMsg_ConnectEntity,
	E_AudioMsg_DialogueFinish,
	E_AudioMsg_DialogueLoadEntity,
	E_AudioMsg_GraphicalEQ,

	//E_AudioMsg_Pickups_Fire,
	//E_AudioMsg_Pickups_Ended,
	//E_AudioMsg_Pickups_Destroy,
	//E_AudioMsg_Pickups_Hit,
	//E_AudioMsg_Pickups_Missed,
	//E_AudioMsg_Pickups_Blocked,
	//E_AudioMsg_Pickups_Charging,
	//E_AudioMsg_Pickups_Update,
	//E_AudioMsg_Pickups_Awarded,

	//E_AudioMsg_Pickups_AudioFinished,

	E_AudioMsg_EntityAudioStopped,			// the entire entity's audio has stopped
	E_AudioMsg_InstanceAudioStopped,		// the audio has stopped of an instance within the entity

	E_AudioMsg_WaitForEvent,				// tell the audio actor to wait for an event
	
	E_AudioMsg_HotSwitchEntity,				// change entity from one type to another. eg ai gun -> player gun when pickup weapon

	E_AudioMsg_PlayUIDialogue,
	E_AudioMsg_StopUIDialogue,
	E_AudioMsg_UIDialogueFinish,

	E_AudioMsg_ReleaseEntity,				// tell the actor to release it's entity

	//E_AudioMsg_Pickups_UpdateMatrix,
	//E_AudioMsg_Pickups_Fire_Projectile,

	//E_AudioMsg_Collisions_Impact,				// tell the actor to release it's entity

	E_AudioMsg_End, 
	E_AudioMsg_Begin = E_AudioMsg_HiBits + 1, 
	E_AudioMsg_Count = E_AudioMsg_End - E_AudioMsg_Begin
};

typedef Messaging::C_MessageValue<U32, E_AudioMsg_Event> C_AudioEventMsg;

class C_AudioSetParameterMsg: public Messaging::C_MessageDerived<C_AudioSetParameterMsg, E_AudioMsg_SetParameter>
{
public:
	C_AudioSetParameterMsg() {}
	explicit C_AudioSetParameterMsg(const U32 msgID, F32 val) : MsgID(msgID),fValue(val) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	U32			MsgID;
	F32			fValue;
};

class C_AudioSetVectorMsg: public Messaging::C_MessageDerived<C_AudioSetVectorMsg, E_AudioMsg_SetVector>
{
public:
	C_AudioSetVectorMsg() {}
	explicit C_AudioSetVectorMsg(const U32 msgID, Maths::HVec3 vec) : MsgID(msgID),vValue(vec) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	U32				MsgID;
	Maths::HVec3	vValue;
};


class C_AudioTriggerActionFromHandleMsg: public Messaging::C_MessageDerived<C_AudioTriggerActionFromHandleMsg, E_AudioMsg_TriggerActionFromHandle>
{
public:
	C_AudioTriggerActionFromHandleMsg() {}
	explicit C_AudioTriggerActionFromHandleMsg(U32 uEntityDataMemberHnd) : m_uEntityDataMemberHnd(uEntityDataMemberHnd) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	U32	m_uEntityDataMemberHnd;
};

class C_AudioTriggerActionFromStrHashMsg: public Messaging::C_MessageDerived<C_AudioTriggerActionFromStrHashMsg, E_AudioMsg_TriggerActionFromStrHash>
{
public:
	C_AudioTriggerActionFromStrHashMsg() : m_uStringHash(Audio::StringHash::Invalid) {}
	explicit C_AudioTriggerActionFromStrHashMsg(U32 uStringHash) : m_uStringHash(uStringHash) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	U32 m_uStringHash;
};

class C_AudioSetParamFromStrHashMsg: public Messaging::C_MessageDerived<C_AudioSetParamFromStrHashMsg, E_AudioMsg_SetParamFromStrHash>
{
public:
	C_AudioSetParamFromStrHashMsg() : m_fParam(0.f),
									  m_uStringHash(Audio::StringHash::Invalid) {}

	explicit C_AudioSetParamFromStrHashMsg(U32 uStringHash, F32 fParam) : m_fParam(fParam),
																		  m_uStringHash(uStringHash) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	U32 m_uStringHash;
	F32 m_fParam;
};

class C_AudioConnectEntityMsg: public Messaging::C_MessageDerived<C_AudioConnectEntityMsg, E_AudioMsg_ConnectEntity>
{
public:
	C_AudioConnectEntityMsg(){}

	explicit C_AudioConnectEntityMsg(const AudioEntityHandle EntityInstanceHandle) : m_EntityInstanceHandle(EntityInstanceHandle){}
	

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	AudioEntityHandle	m_EntityInstanceHandle;
};

class C_AudioDialogueFinishMsg: public Messaging::C_MessageDerived<C_AudioDialogueFinishMsg, E_AudioMsg_DialogueFinish>
{
public:
	C_AudioDialogueFinishMsg() {}
	explicit C_AudioDialogueFinishMsg(const U32 msgID, F32 val) : MsgID(msgID),fValue(val) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	U32			MsgID;
	F32			fValue;
};

class C_AudioDialogueLoadEntity: public Messaging::C_MessageDerived<C_AudioDialogueLoadEntity, E_AudioMsg_DialogueLoadEntity>
{
public:
	C_AudioDialogueLoadEntity() {}
	explicit C_AudioDialogueLoadEntity(const U32 waveBank, U32 index) : uWaveBank(waveBank),fSampleIndex(index) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	U32			uWaveBank;
	U32			fSampleIndex;
};

class C_AudioPlayUIDialogue: public Messaging::C_MessageDerived<C_AudioPlayUIDialogue, E_AudioMsg_PlayUIDialogue>
{
public:
	C_AudioPlayUIDialogue() {}
	explicit C_AudioPlayUIDialogue( U32 index) : fSampleIndex(index) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	U32			fSampleIndex;
};

class C_AudioStopUIDialogue: public Messaging::C_MessageDerived<C_AudioStopUIDialogue, E_AudioMsg_StopUIDialogue>
{
public:
	C_AudioStopUIDialogue() {}
	explicit C_AudioStopUIDialogue( U32 index) : fSampleIndex(index) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	U32			fSampleIndex;
};

class C_AudioUIDialogueFinishMsg: public Messaging::C_MessageDerived<C_AudioUIDialogueFinishMsg, E_AudioMsg_UIDialogueFinish>
{
public:
	C_AudioUIDialogueFinishMsg() {}
	explicit C_AudioUIDialogueFinishMsg(const U32 msgID, F32 val) : MsgID(msgID),fValue(val) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	U32			MsgID;
	F32			fValue;
};

class C_AudioGraphicalEQMsg: public Messaging::C_MessageDerived<C_AudioGraphicalEQMsg, E_AudioMsg_GraphicalEQ>
{
public:
	C_AudioGraphicalEQMsg() {}
	explicit C_AudioGraphicalEQMsg(const U32 msgID) : MsgID(msgID) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	U32			MsgID;
	F32			fBin1;
	F32			fBin2;
	F32			fBin3;
	F32			fBin4;
	F32			fBin5;
	F32			fBin6;
	F32			fBin7;
	F32			fBin8;
};

// Wait for audio event
// Value - Event Name String Hash
// Actor Handle - Actor to notify on event
typedef Actor::C_MessageActorValue<U32, E_AudioMsg_WaitForEvent> C_AudioWaitForEventMsg;

class C_InstanceAudioStoppedMsg: public Messaging::C_MessageDerived<C_InstanceAudioStoppedMsg, E_AudioMsg_InstanceAudioStopped>
{
public:
	C_InstanceAudioStoppedMsg() : m_sInstanceId(-1) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	S32 m_sInstanceId;	// the id of the instance which has just finished
};

class C_HotSwitchEntityMsg: public Messaging::C_MessageDerived<C_HotSwitchEntityMsg, E_AudioMsg_HotSwitchEntity>
{
public:
	C_HotSwitchEntityMsg() {}
	C_HotSwitchEntityMsg(const XtHandle<Audio::XtEntityProxy>& hEntity) : m_hXtEntity(hEntity) {}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);
	static Messaging::C_MessageSerialiser s_Serialiser;

	XtHandle<Audio::XtEntityProxy> m_hXtEntity;
};

} //actor message
#endif
#endif