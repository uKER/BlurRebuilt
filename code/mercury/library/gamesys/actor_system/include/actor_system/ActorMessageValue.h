#ifndef ACTORMESSAGEVALUE_H
#define ACTORMESSAGEVALUE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorMessageData.h>
#include <misc_shared/messaging/MessageSerialisation.h>
#include <misc_shared/messaging/MessageValueStorage.h>


namespace Actor
{

/**
@brief Base helper class for messages carrying an actor handle and a value.
@param T_ValueType type of the value.
*/
template <typename T_ValueType>
class C_MessageActorValueBase : public C_MessageActorData
{
public:
	explicit C_MessageActorValueBase(const Messaging::TMsgType typeId) : C_MessageActorData(typeId, sizeof(C_MessageActorValueBase)), m_Value() {}
	C_MessageActorValueBase(const Messaging::TMsgType typeId, const C_ActorHandle actorHandle, const T_ValueType& value) : C_MessageActorData(typeId, sizeof(C_MessageActorValueBase), actorHandle), m_Value(value) {}
	// auto: C_MessageActorValueBase(const C_MessageActorValueBase& copy);
	// auto: ~C_MessageActorValueBase();
	// auto: C_MessageActorValueBase& operator = (const C_MessageActorValueBase& copy);

	T_ValueType GetValue() const {return m_Value.Get();} ///< @returns value for this message.
	void SetValue(const T_ValueType& value) {m_Value.Set(value);} ///< @param value value for this message.

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
	{
		C_MessageActorData::SerialiseContents(stream, pMessage);
		stream << static_cast<const C_MessageActorValueBase*>(pMessage)->m_Value;
	}
	
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
	{
		C_MessageActorData::DeserialiseContents(stream, pMessage);
		stream >> static_cast<C_MessageActorValueBase*>(pMessage)->m_Value;
	}

private:
	Messaging::C_MessageValueStorage<T_ValueType> m_Value; ///< value for this message

};


/**
@brief Base class for messages carrying an actor handle and a data value.
@ingroup actor_group_messages

For messages that carry an actor handle and an additional value as data, this 
class can be used to automate the class generation.
@code
typedef Actor::C_MessageActorValue<U32, E_MyMessageTypes_MyMessage> C_MyMessageClass;

bool MyMessageHandler::MessageHandler(const C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case E_MyMessageTypes_MyMessage:
		{
			const C_MyMessageClass* pMyMessage = Messaging::MessageCast<C_MyMessageClass>(pMessage);
			Actor::C_ActorHandle actorHandle = pMyMessage->GetActorHandle();
			U32 value = pMyMessage->GetValue();
			// do something with actor handle and value
		}
		return true;

	default:
		return MyBaseClass::MessageHandler(pMessage);
	}
}
@endcode

@param T_ValueType type of the value.
@param t_TypeId TMsgType identifying the message class.
*/
template <typename T_ValueType, Messaging::TMsgType t_TypeId>
class C_MessageActorValue : public C_MessageActorValueBase<T_ValueType>
{
public:
	C_MessageActorValue() : C_MessageActorValueBase<T_ValueType>(t_TypeId) {s_Serialiser.DummyForceLinkage();}
	explicit C_MessageActorValue(const C_ActorHandle actorHandle, const T_ValueType& value) : C_MessageActorValueBase<T_ValueType>(t_TypeId, actorHandle, value) {s_Serialiser.DummyForceLinkage();}
	// auto: C_MessageActorValue(const C_MessageActorValue& copy);
	// auto: ~C_MessageActorValue();
	// auto: C_MessageActorValue& operator=(const C_MessageActorValue& copy);

	static Messaging::TMsgType StaticType() {return t_TypeId;} ///< @returns type identifier for this message class.

private:
	static Messaging::C_MessageSerialiser s_Serialiser; ///< registers serialisation functions

};

template <typename T_ValueType, Messaging::TMsgType t_TypeId>
Messaging::C_MessageSerialiser C_MessageActorValue<T_ValueType, t_TypeId>::s_Serialiser(t_TypeId, &C_MessageActorValueBase<T_ValueType>::SerialiseContents, &C_MessageActorValueBase<T_ValueType>::DeserialiseContents);

} // namespace Actor


#endif // ACTORMESSAGEVALUE_H
