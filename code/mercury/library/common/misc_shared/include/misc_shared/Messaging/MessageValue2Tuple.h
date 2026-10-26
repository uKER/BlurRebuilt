#ifndef MESSAGEVALUE2TUPLE_H
#define MESSAGEVALUE2TUPLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/messaging/MessageBase.h>
#include <misc_shared/messaging/MessageValueStorage.h>
#include <misc_shared/messaging/MessageSerialisation.h>


namespace Messaging
{


/**
@brief Base helper class for messages containing two values.
@param T_FirstType type of first value.
@param T_SecondType type of second value.
*/
template <typename T_FirstType, typename T_SecondType>
class C_MessageValue2TupleBase : public C_MessageBase
{
public:
	explicit C_MessageValue2TupleBase(const TMsgType typeId)
		: C_MessageBase(typeId, sizeof(C_MessageValue2TupleBase))
		, m_First()
		, m_Second() 
	{
	}
	
	C_MessageValue2TupleBase(const TMsgType typeId, const T_FirstType& first, const T_SecondType& second)
		: C_MessageBase(typeId, sizeof(C_MessageValue2TupleBase))
		, m_First(first)
		, m_Second(second) 
	{
	}
	
	// auto: C_MessageValue2TupleBase(const C_MessageValue2TupleBase& copy);
	// auto: ~C_MessageValue2TupleBase();
	// auto: C_MessageValue2TupleBase& operator = (const C_MessageValue2TupleBase& copy);

	T_FirstType GetFirst() const {return m_First.Get();} ///< @returns first value for this message.
	void SetFirst(const T_FirstType& value) {m_First.Set(value);} ///< @param first value value for this message.

	T_SecondType GetSecond() const {return m_Second.Get();} ///< @returns second value for this message.
	void SetSecond(const T_SecondType& value) {m_Second.Set(value);} ///< @param second value value for this message.

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
	{
		const C_MessageValue2TupleBase* pThis = static_cast<const C_MessageValue2TupleBase*>(pMessage);
		stream << pThis->m_First;
		stream << pThis->m_Second;
	}
	
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
	{
		C_MessageValue2TupleBase* pThis = static_cast<C_MessageValue2TupleBase*>(pMessage);
		stream >> pThis->m_First;
		stream >> pThis->m_Second;
	}

private:
	C_MessageValueStorage<T_FirstType> m_First; ///< first value for this message
	C_MessageValueStorage<T_SecondType> m_Second; ///< second value for this message

};


/**
@brief Helper class for messages containing two values.

Similar to C_MessageValue but the message carries two values.

@param T_FirstType type of first value.
@param T_SecondType type of second value.
@param t_TypeId TMsgType identifying the message class.
*/
template <typename T_FirstType, typename T_SecondType, TMsgType t_TypeId>
class C_MessageValue2Tuple : public C_MessageValue2TupleBase<T_FirstType, T_SecondType>
{
public:
	C_MessageValue2Tuple() : C_MessageValue2TupleBase<T_FirstType, T_SecondType>(t_TypeId) {s_Serialiser.DummyForceLinkage();}
	explicit C_MessageValue2Tuple(const T_FirstType& first, const T_SecondType& second) : C_MessageValue2TupleBase<T_FirstType, T_SecondType>(t_TypeId, first, second) {s_Serialiser.DummyForceLinkage();}
	
	// auto: C_MessageValue2Tuple(const C_MessageValue2Tuple& copy);
	// auto: ~C_MessageValue2Tuple();
	// auto: C_MessageValue2Tuple& operator=(const C_MessageValue2Tuple& copy);

	static TMsgType StaticType() {return t_TypeId;} ///< @returns type identifier for this message class.

private:
	static C_MessageSerialiser s_Serialiser; ///< registers serialisation functions

};

template <typename T_FirstType, typename T_SecondType, TMsgType t_TypeId>
C_MessageSerialiser C_MessageValue2Tuple<T_FirstType, T_SecondType, t_TypeId>::s_Serialiser(t_TypeId, &C_MessageValue2TupleBase<T_FirstType, T_SecondType>::SerialiseContents, &C_MessageValue2TupleBase<T_FirstType, T_SecondType>::DeserialiseContents);

} // namespace Messaging


#endif // MESSAGEVALUE2TUPLE_H
