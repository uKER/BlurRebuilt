#ifndef MESSAGEVALUE3TUPLE_H
#define MESSAGEVALUE3TUPLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/messaging/MessageBase.h>
#include <misc_shared/messaging/MessageValueStorage.h>
#include <misc_shared/messaging/MessageSerialisation.h>


namespace Messaging
{


/**
@brief Base helper class for messages containing three values.
@param T_FirstType type of first value.
@param T_SecondType type of second value.
@param T_ThirdType type of third value.
*/
template <typename T_FirstType, typename T_SecondType, typename T_ThirdType>
class C_MessageValue3TupleBase : public C_MessageBase
{
public:
	explicit C_MessageValue3TupleBase(const TMsgType typeId)
		: C_MessageBase(typeId, sizeof(C_MessageValue3TupleBase))
		, m_First()
		, m_Second() 
		, m_Third()
	{
	}
	
	C_MessageValue3TupleBase(const TMsgType typeId, const T_FirstType& first, const T_SecondType& second, const T_ThirdType& third)
		: C_MessageBase(typeId, sizeof(C_MessageValue3TupleBase))
		, m_First(first)
		, m_Second(second) 
		, m_Third(third)
	{
	}
	
	// auto: C_MessageValue3TupleBase(const C_MessageValue3TupleBase& copy);
	// auto: ~C_MessageValue3TupleBase();
	// auto: C_MessageValue3TupleBase& operator = (const C_MessageValue3TupleBase& copy);

	T_FirstType GetFirst() const {return m_First.Get();} ///< @returns first value for this message.
	void SetFirst(const T_FirstType& value) {m_First.Set(value);} ///< @param first value value for this message.

	T_SecondType GetSecond() const {return m_Second.Get();} ///< @returns second value for this message.
	void SetSecond(const T_SecondType& value) {m_Second.Set(value);} ///< @param second value value for this message.

	T_ThirdType GetThird() const {return m_Third.Get();} ///< @returns third value for this message.
	void SetThird(const T_ThirdType& value) {m_Third.Set(value);} ///< @param third value value for this message.

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
	{
		const C_MessageValue3TupleBase* pThis = static_cast<const C_MessageValue3TupleBase*>(pMessage);
		stream << pThis->m_First;
		stream << pThis->m_Second;
		stream << pThis->m_Third;
	}
	
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
	{
		C_MessageValue3TupleBase* pThis = static_cast<C_MessageValue3TupleBase*>(pMessage);
		stream >> pThis->m_First;
		stream >> pThis->m_Second;
		stream >> pThis->m_Third;
	}

private:
	C_MessageValueStorage<T_FirstType> m_First; ///< first value for this message
	C_MessageValueStorage<T_SecondType> m_Second; ///< second value for this message
	C_MessageValueStorage<T_ThirdType> m_Third; ///< second value for this message

};


/**
@brief Helper class for messages containing three values.

Similar to C_MessageValue but the message carries three values.

@param T_FirstType type of first value.
@param T_SecondType type of second value.
@param T_ThirdType type of third value.
@param t_TypeId TMsgType identifying the message class.
*/
template <typename T_FirstType, typename T_SecondType, typename T_ThirdType, TMsgType t_TypeId>
class C_MessageValue3Tuple : public C_MessageValue3TupleBase<T_FirstType, T_SecondType, T_ThirdType>
{
public:
	C_MessageValue3Tuple() : C_MessageValue3TupleBase<T_FirstType, T_SecondType, T_ThirdType>(t_TypeId) {s_Serialiser.DummyForceLinkage();}
	explicit C_MessageValue3Tuple(const T_FirstType& first, const T_SecondType& second, const T_ThirdType& third) : C_MessageValue3TupleBase<T_FirstType, T_SecondType, T_ThirdType>(t_TypeId, first, second, third) {s_Serialiser.DummyForceLinkage();}
	
	// auto: C_MessageValue3Tuple(const C_MessageValue3Tuple& copy);
	// auto: ~C_MessageValue3Tuple();
	// auto: C_MessageValue3Tuple& operator=(const C_MessageValue3Tuple& copy);

	static TMsgType StaticType() {return t_TypeId;} ///< @returns type identifier for this message class.

private:
	static C_MessageSerialiser s_Serialiser; ///< registers serialisation functions

};

template <typename T_FirstType, typename T_SecondType, typename T_ThirdType, TMsgType t_TypeId>
C_MessageSerialiser C_MessageValue3Tuple<T_FirstType, T_SecondType, T_ThirdType, t_TypeId>::s_Serialiser(t_TypeId, &C_MessageValue3TupleBase<T_FirstType, T_SecondType, T_ThirdType>::SerialiseContents, &C_MessageValue3TupleBase<T_FirstType, T_SecondType, T_ThirdType>::DeserialiseContents);

} // namespace Messaging


#endif // MESSAGEVALUE3TUPLE_H
