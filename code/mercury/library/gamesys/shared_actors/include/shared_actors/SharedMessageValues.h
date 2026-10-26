#ifndef SHAREDMESSAGEVALUES_CA5FFDD7_3430_40DE_BEBF_A689F237E989_H
#define SHAREDMESSAGEVALUES_CA5FFDD7_3430_40DE_BEBF_A689F237E989_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/messaging/MessageValue.h>


namespace Messaging
{

// specialise C_MessageValueStorage for Maths::C_Vertex
template <>
class C_MessageValueStorage<Maths::C_Vertex>
{
public:
	C_MessageValueStorage() : m_Storage() {}
	explicit C_MessageValueStorage(const Maths::C_Vertex& data) : m_Storage(data) {}

	void Set(const Maths::C_Vertex& data) {m_Storage = data;}
	Maths::C_Vertex Get() const {return Maths::Load<Maths::C_Vertex>(&m_Storage);}

	void Serialise(Sys::C_ContextualStream& stream) const {stream << m_Storage;}
	void Deserialise(Sys::C_ContextualStream& stream) {stream >> m_Storage;}

private:
	MathsStorage::Float3 m_Storage;

};

// specialise C_MessageValueStorage for Maths::C_Vector
template <>
class C_MessageValueStorage<Maths::C_Vector>
{
public:
	C_MessageValueStorage() : m_Storage() {}
	explicit C_MessageValueStorage(const Maths::C_Vector& data) : m_Storage(data) {}

	void Set(const Maths::C_Vector& data) {m_Storage = data;}
	Maths::C_Vector Get() const {return Maths::Load<Maths::C_Vector>(&m_Storage);}

	void Serialise(Sys::C_ContextualStream& stream) const {stream << m_Storage;}
	void Deserialise(Sys::C_ContextualStream& stream) {stream >> m_Storage;}

private:
	MathsStorage::Float3 m_Storage;

};

// specialise C_MessageValueStorage for Maths::C_Normal
template <>
class C_MessageValueStorage<Maths::C_Normal>
{
public:
	C_MessageValueStorage() : m_Storage() {}
	explicit C_MessageValueStorage(const Maths::C_Normal& data) : m_Storage(data) {}

	void Set(const Maths::C_Normal& data) {m_Storage = data;}
	Maths::C_Normal Get() const {return Maths::Load<Maths::C_Normal>(&m_Storage);}

	void Serialise(Sys::C_ContextualStream& stream) const {stream << m_Storage;}
	void Deserialise(Sys::C_ContextualStream& stream) {stream >> m_Storage;}

private:
	MathsStorage::Float3 m_Storage;

};

// partially specialise C_MessageValueStorage for Maths::C_SiScalar
template <typename T_SiUnits>
class C_MessageValueStorage< Maths::C_SiScalar<T_SiUnits> >
{
public:
	typedef Maths::C_SiScalar<T_SiUnits> C_SiScalarType;

	C_MessageValueStorage() : m_Storage() {}
	explicit C_MessageValueStorage(const C_SiScalarType& data) : m_Storage(static_cast<F32>(data.Get())) {}

	void Set(const C_SiScalarType& data) {m_Storage = static_cast<F32>(data.Get());}
	C_SiScalarType Get() const {return C_SiScalarType(m_Storage);}

	void Serialise(Sys::C_ContextualStream& stream) const {stream << m_Storage;}
	void Deserialise(Sys::C_ContextualStream& stream) {stream >> m_Storage;}

private:
	F32 m_Storage;

};

// partially-specialise C_MessageValueStorage for C_SiVector types
template <typename T_SiUnits>
class C_MessageValueStorage< Maths::C_SiVector<T_SiUnits> >
{
public:
	typedef Maths::C_SiVector<T_SiUnits> C_SiVectorType;

	C_MessageValueStorage() : m_Storage() {}
	explicit C_MessageValueStorage(const C_SiVectorType& data) {Maths::Store(data.Get(), &m_Storage);}

	void Set(const C_SiVectorType& data) {Maths::Store(data.Get(), &m_Storage);}
	C_SiVectorType Get() const {return Maths::Load<Maths::HVec3>(&m_Storage);}

	void Serialise(Sys::C_ContextualStream& stream) const {stream << m_Storage;}
	void Deserialise(Sys::C_ContextualStream& stream) {stream >> m_Storage;}

private:
	MathsStorage::Float3 m_Storage;

};

} // namespace Messaging


#endif // SHAREDMESSAGEVALUES_CA5FFDD7_3430_40DE_BEBF_A689F237E989_H
