#ifndef MESSAGEVALUESTORAGE_H
#define MESSAGEVALUESTORAGE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/utility/string.h>
#include <core/containers/String.h>
#include <core/Maths.h>
#include <core/utility/CoreDebug.h>
#include <misc_shared/Context/ContextualStream.h>
#include <misc_shared/messaging/MessageBase.h>
#include <misc_shared/messaging/MessageSerialisation.h>


namespace Messaging
{


// TODO: [Oscar] replace this with a common alignof macro, we must have one somewhere
#if defined(COMPILER_VC)
 #define Messaging_AlignOf __alignof
#else
 #define Messaging_AlignOf __alignof__
#endif


/**
@brief Utility class for storing data in messages.

Provides a templated class with a default implementation that works for simple
types with up to 4-byte alignment. Complex types, or those with higher 
alignment requirements, require template specialisations.
*/
template <typename T_DataType>
class C_MessageValueStorage
{
public:
	C_MessageValueStorage() : m_Storage() {StaticAssert(Messaging_AlignOf(T_DataType) <= 4, storage_type_must_be_no_more_than_4_byte_aligned);}
	explicit C_MessageValueStorage(const T_DataType data) : m_Storage(data) {} ///< @param data data value to store.

	void Set(const T_DataType data) {m_Storage = data;} ///< @param data data value to store.
	T_DataType Get() const {return m_Storage;} ///< @returns stored data value.

	void Serialise(Sys::C_ContextualStream& stream) const {stream << m_Storage;} ///< @param stream stream to write into.
	void Deserialise(Sys::C_ContextualStream& stream) {stream >> m_Storage;} ///< @param stream stream to read from.

private:
	T_DataType m_Storage;	///< storage for data value.

};

// specialise C_MessageValueStorage for Maths::HVec3
template <>
class C_MessageValueStorage<Maths::HVec3>
{
public:
	C_MessageValueStorage() : m_Storage() {}
	explicit C_MessageValueStorage(const Maths::HVec3& data) : m_Storage(data) {}

	void Set(const Maths::HVec3& data) {m_Storage = data;}
	Maths::HVec3 Get() const {return Maths::Load<Maths::HVec3>(&m_Storage);}

	void Serialise(Sys::C_ContextualStream& stream) const {stream << m_Storage;}
	void Deserialise(Sys::C_ContextualStream& stream) {stream >> m_Storage;}

private:
	MathsStorage::Float3 m_Storage;

};

// specialise C_MessageValueStorage for Maths::HMat43
template <>
class C_MessageValueStorage<Maths::HMat43>
{
public:
	C_MessageValueStorage() : m_Storage() {}
	explicit C_MessageValueStorage(const Maths::HMat43& data) : m_Storage() {Set(data);}

	void Set(const Maths::HMat43& data) {Maths::Store(data, &m_Storage);}
	Maths::HMat43 Get() const {return Maths::Load<Maths::HMat43>(&m_Storage);}

	void Serialise(Sys::C_ContextualStream& stream) const {stream << m_Storage;}
	void Deserialise(Sys::C_ContextualStream& stream) {stream >> m_Storage;}

private:
	MathsStorage::Float43 m_Storage;

};

// specialise C_MessageValueStorage for Maths::C_QuatTrans
template <>
class C_MessageValueStorage<Maths::C_QuatTrans>
{
public:
	C_MessageValueStorage() : m_Quat(), m_Trans() {}
	explicit C_MessageValueStorage(const Maths::C_QuatTrans& data) : m_Quat(), m_Trans() {Set(data);}

	void Set(const Maths::C_QuatTrans& data)
	{
		Maths::Store(static_cast<Maths::HVec4>(data.m_Quaternion), &m_Quat);
		Maths::Store(data.m_Translation, &m_Trans);
	}

	Maths::C_QuatTrans Get() const 
	{
		Maths::C_QuatTrans data;
		data.m_Quaternion = Maths::C_Quaternion(Maths::Load<Maths::HVec4>(&m_Quat));
		data.m_Translation = Maths::Load<Maths::HVec3>(&m_Trans);
		return data;
	}

	void Serialise(Sys::C_ContextualStream& stream) const
	{
		stream << m_Quat;
		stream << m_Trans;
	}
	
	void Deserialise(Sys::C_ContextualStream& stream)
	{
		stream >> m_Quat;
		stream >> m_Trans;
	}

private:
	MathsStorage::Float4 m_Quat;
	MathsStorage::Float3 m_Trans;

};

// specialise C_MessageValueStorage for Util::C_String, should never be used!
template <>
class C_MessageValueStorage<Util::C_String>
{
private:
	// Util::C_String is not safe to memcpy, use Util::C_FixedString<> if you really need a string in a message
	C_MessageValueStorage();

};

// specialise C_MessageValueStorage for Btl::String, should never be used!
template <>
class C_MessageValueStorage<Btl::String>
{
private:
	// Btl::String is not safe to memcpy, use Btl::StringN if you really need a string in a message
	C_MessageValueStorage();

};


} // namespace Messaging


#endif // MESSAGEVALUESTORAGE_H
