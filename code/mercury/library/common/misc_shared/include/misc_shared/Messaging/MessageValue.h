#ifndef MESSAGEVALUE_H
#define MESSAGEVALUE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/messaging/MessageBase.h>
#include <misc_shared/messaging/MessageValueStorage.h>
#include <misc_shared/messaging/MessageSerialisation.h>


namespace Messaging
{


/**
@brief Base helper class for messages containing a single value.
@param T_ValueType type of the value.
*/
template <typename T_ValueType>
class C_MessageValueBase : public C_MessageBase
{
public:
	explicit C_MessageValueBase(const TMsgType typeId) : C_MessageBase(typeId, sizeof(C_MessageValueBase)), m_Value() {}
	C_MessageValueBase(const TMsgType typeId, const T_ValueType& value) : C_MessageBase(typeId, sizeof(C_MessageValueBase)), m_Value(value) {}
	// auto: C_MessageValueBase(const C_MessageValueBase& copy);
	// auto: ~C_MessageValueBase();
	// auto: C_MessageValueBase& operator = (const C_MessageValueBase& copy);

	T_ValueType GetValue() const {return m_Value.Get();} ///< @returns value for this message.
	void SetValue(const T_ValueType& value) {m_Value.Set(value);} ///< @param value value for this message.

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage) {stream << static_cast<const C_MessageValueBase*>(pMessage)->m_Value;}
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage) {stream >> static_cast<C_MessageValueBase*>(pMessage)->m_Value;}

private:
	C_MessageValueStorage<T_ValueType> m_Value; ///< value for this message

};


/**
@brief Helper class for messages containing a single value.

Where a message carries a single piece of additional data, say an integer, 
the C_MessageValue class can be used to automate the generate of the message
class that holds this data.
@code
typedef Messaging::C_MessageValue<U32, E_MyGameMessageTypes_MyMessage> C_MyMessageClass;

bool MyMessageHandler::MessageHandler(const C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case E_MyGameMessageTypes_MyMessage:
		{
			U32 value = Messaging::MessageCast<C_MyMessageClass>(pMessage)->GetValue();
			// do something with value
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
template <typename T_ValueType, TMsgType t_TypeId>
class C_MessageValue : public C_MessageValueBase<T_ValueType>
{
public:
	C_MessageValue() : C_MessageValueBase<T_ValueType>(t_TypeId) {s_Serialiser.DummyForceLinkage();}
	explicit C_MessageValue(const T_ValueType& value) : C_MessageValueBase<T_ValueType>(t_TypeId, value) {s_Serialiser.DummyForceLinkage();}
	// auto: C_MessageValue(const C_MessageValue& copy);
	// auto: ~C_MessageValue();
	// auto: C_MessageValue& operator=(const C_MessageValue& copy);

	static TMsgType StaticType() {return t_TypeId;} ///< @returns type identifier for this message class.

private:
	static C_MessageSerialiser s_Serialiser; ///< registers serialisation functions

};

template <typename T_ValueType, TMsgType t_TypeId>
C_MessageSerialiser C_MessageValue<T_ValueType, t_TypeId>::s_Serialiser(t_TypeId, &C_MessageValueBase<T_ValueType>::SerialiseContents, &C_MessageValueBase<T_ValueType>::DeserialiseContents);

} // namespace Messaging


#endif // MESSAGEVALUE_H
