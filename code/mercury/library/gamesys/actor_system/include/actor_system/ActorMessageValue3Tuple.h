#ifndef ACTORMESSAGEVALUE3TUPLE_H
#define ACTORMESSAGEVALUE3TUPLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorMessageData.h>
#include <misc_shared/messaging/MessageSerialisation.h>
#include <misc_shared/messaging/MessageValueStorage.h>


namespace Actor
{

/**
@brief Base helper class for messages carrying an actor handle and three values.
@param T_FirstType type of first value.
@param T_SecondType type of second value.
@param T_ThirdType type of third value.
*/
template <typename T_FirstType, typename T_SecondType, typename T_ThirdType>
class C_MessageActorValue3TupleBase : public C_MessageActorData
{
public:
	explicit C_MessageActorValue3TupleBase(const Messaging::TMsgType typeId)
		: C_MessageActorData(typeId, sizeof(C_MessageActorValue3TupleBase))
		, m_First() 
		, m_Second() 
		, m_Third()
	{
	}
	
	C_MessageActorValue3TupleBase(const Messaging::TMsgType typeId, const C_ActorHandle actorHandle, const T_FirstType& first, const T_SecondType& second, const T_ThirdType& third)
		: C_MessageActorData(typeId, sizeof(C_MessageActorValue3TupleBase), actorHandle)
		, m_First(first) 
		, m_Second(second) 
		, m_Third(third)
	{
	}
	
	// auto: C_MessageActorValue3TupleBase(const C_MessageActorValue3TupleBase& copy);
	// auto: ~C_MessageActorValue3TupleBase();
	// auto: C_MessageActorValue3TupleBase& operator = (const C_MessageActorValue3TupleBase& copy);

	T_FirstType GetFirst() const {return m_First.Get();} ///< @returns first value for this message.
	void SetFirst(const T_FirstType& value) {m_First.Set(value);} ///< @param first value value for this message.

	T_SecondType GetSecond() const {return m_Second.Get();} ///< @returns second value for this message.
	void SetSecond(const T_SecondType& value) {m_Second.Set(value);} ///< @param second value value for this message.

	T_ThirdType GetThird() const {return m_Third.Get();} ///< @returns third value for this message.
	void SetThird(const T_ThirdType& value) {m_Third.Set(value);} ///< @param third value value for this message.

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
	{
		C_MessageActorData::SerialiseContents(stream, pMessage);
		const C_MessageActorValue3TupleBase* pThis = static_cast<const C_MessageActorValue3TupleBase*>(pMessage);
		stream << pThis->m_First;
		stream << pThis->m_Second;
		stream << pThis->m_Third;
	}
	
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
	{
		C_MessageActorData::DeserialiseContents(stream, pMessage);
		C_MessageActorValue3TupleBase* pThis = static_cast<C_MessageActorValue3TupleBase*>(pMessage);
		stream >> pThis->m_First;
		stream >> pThis->m_Second;
		stream >> pThis->m_Third;
	}

private:
	Messaging::C_MessageValueStorage<T_FirstType> m_First; ///< first value for this message
	Messaging::C_MessageValueStorage<T_SecondType> m_Second; ///< second value for this message
	Messaging::C_MessageValueStorage<T_ThirdType> m_Third; ///< third value for this message

};


/**
@brief Base class for messages carrying an actor handle and three data values.
@ingroup actor_group_messages

Similar to C_MessageActorValue.

@param T_FirstType type of first value.
@param T_SecondType type of second value.
@param T_ThirdType type of third value.
@param t_TypeId TMsgType identifying the message class.
*/
template <typename T_FirstType, typename T_SecondType, typename T_ThirdType, Messaging::TMsgType t_TypeId>
class C_MessageActorValue3Tuple : public C_MessageActorValue3TupleBase<T_FirstType, T_SecondType, T_ThirdType>
{
public:
	C_MessageActorValue3Tuple() : C_MessageActorValue3TupleBase<T_FirstType, T_SecondType, T_ThirdType>(t_TypeId) {s_Serialiser.DummyForceLinkage();}
	explicit C_MessageActorValue3Tuple(const C_ActorHandle actorHandle, const T_FirstType& first, const T_SecondType& second, const T_SecondType& third) : C_MessageActorValue3TupleBase<T_FirstType, T_SecondType, T_ThirdType>(t_TypeId, actorHandle, first, second, third) {s_Serialiser.DummyForceLinkage();}
	
	// auto: C_MessageActorValue3Tuple(const C_MessageActorValue3Tuple& copy);
	// auto: ~C_MessageActorValue3Tuple();
	// auto: C_MessageActorValue3Tuple& operator=(const C_MessageActorValue3Tuple& copy);

	static Messaging::TMsgType StaticType() {return t_TypeId;} ///< @returns type identifier for this message class.

private:
	static Messaging::C_MessageSerialiser s_Serialiser; ///< registers serialisation functions

};

template <typename T_FirstType, typename T_SecondType, typename T_ThirdType, Messaging::TMsgType t_TypeId>
Messaging::C_MessageSerialiser C_MessageActorValue3Tuple<T_FirstType, T_SecondType, T_ThirdType, t_TypeId>::s_Serialiser(t_TypeId, &C_MessageActorValue3TupleBase<T_FirstType, T_SecondType, T_ThirdType>::SerialiseContents, &C_MessageActorValue3TupleBase<T_FirstType, T_SecondType, T_ThirdType>::DeserialiseContents);

} // namespace Actor


#endif // ACTORMESSAGEVALUE3TUPLE_H
