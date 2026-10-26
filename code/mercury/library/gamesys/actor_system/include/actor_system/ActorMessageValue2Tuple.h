#ifndef ACTORMESSAGEVALUE2TUPLE_H
#define ACTORMESSAGEVALUE2TUPLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorMessageData.h>
#include <misc_shared/messaging/MessageSerialisation.h>
#include <misc_shared/messaging/MessageValueStorage.h>


namespace Actor
{

/**
@brief Base helper class for messages carrying an actor handle and two values.
@param T_FirstType type of first value.
@param T_SecondType type of second value.
*/
template <typename T_FirstType, typename T_SecondType>
class C_MessageActorValue2TupleBase : public C_MessageActorData
{
public:
	explicit C_MessageActorValue2TupleBase(const Messaging::TMsgType typeId)
		: C_MessageActorData(typeId, sizeof(C_MessageActorValue2TupleBase))
		, m_First() 
		, m_Second() 
	{
	}
	
	C_MessageActorValue2TupleBase(const Messaging::TMsgType typeId, const C_ActorHandle actorHandle, const T_FirstType& first, const T_SecondType& second)
		: C_MessageActorData(typeId, sizeof(C_MessageActorValue2TupleBase), actorHandle)
		, m_First(first) 
		, m_Second(second) 
	{
	}
	
	// auto: C_MessageActorValue2TupleBase(const C_MessageActorValue2TupleBase& copy);
	// auto: ~C_MessageActorValue2TupleBase();
	// auto: C_MessageActorValue2TupleBase& operator = (const C_MessageActorValue2TupleBase& copy);

	T_FirstType GetFirst() const {return m_First.Get();} ///< @returns first value for this message.
	void SetFirst(const T_FirstType& value) {m_First.Set(value);} ///< @param first value value for this message.

	T_SecondType GetSecond() const {return m_Second.Get();} ///< @returns second value for this message.
	void SetSecond(const T_SecondType& value) {m_Second.Set(value);} ///< @param second value value for this message.

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
	{
		C_MessageActorData::SerialiseContents(stream, pMessage);
		const C_MessageActorValue2TupleBase* pThis = static_cast<const C_MessageActorValue2TupleBase*>(pMessage);
		stream << pThis->m_First;
		stream << pThis->m_Second;
	}
	
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
	{
		C_MessageActorData::DeserialiseContents(stream, pMessage);
		C_MessageActorValue2TupleBase* pThis = static_cast<C_MessageActorValue2TupleBase*>(pMessage);
		stream >> pThis->m_First;
		stream >> pThis->m_Second;
	}

private:
	Messaging::C_MessageValueStorage<T_FirstType> m_First; ///< first value for this message
	Messaging::C_MessageValueStorage<T_SecondType> m_Second; ///< second value for this message

};


/**
@brief Base class for messages carrying an actor handle and two data values.
@ingroup actor_group_messages

Similar to C_MessageActorValue.

@param T_FirstType type of first value.
@param T_SecondType type of second value.
@param t_TypeId TMsgType identifying the message class.
*/
template <typename T_FirstType, typename T_SecondType, Messaging::TMsgType t_TypeId>
class C_MessageActorValue2Tuple : public C_MessageActorValue2TupleBase<T_FirstType, T_SecondType>
{
public:
	C_MessageActorValue2Tuple() : C_MessageActorValue2TupleBase<T_FirstType, T_SecondType>(t_TypeId) {s_Serialiser.DummyForceLinkage();}
	explicit C_MessageActorValue2Tuple(const C_ActorHandle actorHandle, const T_FirstType& first, const T_SecondType& second) : C_MessageActorValue2TupleBase<T_FirstType, T_SecondType>(t_TypeId, actorHandle, first, second) {s_Serialiser.DummyForceLinkage();}
	
	// auto: C_MessageActorValue2Tuple(const C_MessageActorValue2Tuple& copy);
	// auto: ~C_MessageActorValue2Tuple();
	// auto: C_MessageActorValue2Tuple& operator=(const C_MessageActorValue2Tuple& copy);

	static Messaging::TMsgType StaticType() {return t_TypeId;} ///< @returns type identifier for this message class.

private:
	static Messaging::C_MessageSerialiser s_Serialiser; ///< registers serialisation functions

};

template <typename T_FirstType, typename T_SecondType, Messaging::TMsgType t_TypeId>
Messaging::C_MessageSerialiser C_MessageActorValue2Tuple<T_FirstType, T_SecondType, t_TypeId>::s_Serialiser(t_TypeId, &C_MessageActorValue2TupleBase<T_FirstType, T_SecondType>::SerialiseContents, &C_MessageActorValue2TupleBase<T_FirstType, T_SecondType>::DeserialiseContents);

} // namespace Actor


#endif // ACTORMESSAGEVALUE2TUPLE_H
