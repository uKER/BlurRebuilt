#ifndef HANDLE_H
#define HANDLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/HandleData.h>
#include <actor_system/HandleDeclare.h>

namespace Util
{

/**	
@brief Base class for all C_Handle types.
Users should not need to use this type directly, but should instead use the
C_Handle templated class for the object type referred to.
*/
class C_HandleBase
{
	friend class C_HandleManagerBase;

public:
	C_HandleBase();
	~C_HandleBase();

	C_HandleBase(const C_HandleBase& copy);
	C_HandleBase& operator = (const C_HandleBase&);

	const U32 GetIndex() const {return m_Data.GetIndex();} // TODO: [Oscar] why is this publically accessible?

	/// converts the handle to a unique uint32 value, eg. for comparison or hashing
	operator U32 () const {return m_Data.AsU32();}

private:
    C_HandleData m_Data; ///< internal data

};

inline C_HandleBase::C_HandleBase()
	: m_Data() 
{
}

inline C_HandleBase::~C_HandleBase()
{
}

inline C_HandleBase::C_HandleBase(const C_HandleBase& copy)
	: m_Data(copy.m_Data)
{
}

inline C_HandleBase& C_HandleBase::operator = (const C_HandleBase& copy)
{
	m_Data = copy.m_Data;
	return *this;
}

/**	
@brief Handle to a specific data type.
Objects deriving from C_HandleItem should be referenced using C_Handle.
@param T_ItemType object class referred to.
*/
template <class T_ItemType>
class C_Handle : public C_HandleBase
{
	friend class C_HandleManager<T_ItemType>;

public:
	C_Handle();
	~C_Handle();

	C_Handle(const C_Handle& copy);
	C_Handle& operator = (const C_Handle& copy);

private:
	C_Handle(const C_HandleBase& copy);
	C_Handle& operator = (const C_HandleBase& copy);

};

template <class T_ItemType>
inline C_Handle<T_ItemType>::C_Handle()
	: C_HandleBase() 
{
}

template <class T_ItemType>
inline C_Handle<T_ItemType>::~C_Handle()
{
}

template <class T_ItemType>
inline C_Handle<T_ItemType>::C_Handle(const C_Handle<T_ItemType>& copy)
	: C_HandleBase(copy) 
{
}

template <class T_ItemType>
inline C_Handle<T_ItemType>::C_Handle(const C_HandleBase& copy)
	: C_HandleBase(copy) 
{
}

template <class T_ItemType>
inline C_Handle<T_ItemType>& C_Handle<T_ItemType>::operator = (const C_Handle<T_ItemType>& copy)
{
	C_HandleBase::operator = (copy);
	return *this;
}

template <class T_ItemType>
inline C_Handle<T_ItemType>& C_Handle<T_ItemType>::operator = (const C_HandleBase& copy)
{
	C_HandleBase::operator = (copy);
	return *this;
}

template <class T_ItemType>
bool operator == (const C_Handle<T_ItemType>& lhs, const C_Handle<T_ItemType>& rhs)
{
	return static_cast<U32>(lhs) == static_cast<U32>(rhs);
}

template <class T_ItemType>
bool operator != (const C_Handle<T_ItemType>& lhs, const C_Handle<T_ItemType>& rhs)
{
	return static_cast<U32>(lhs) != static_cast<U32>(rhs);
}

template <class T_ItemType>
bool operator <= (const C_Handle<T_ItemType>& lhs, const C_Handle<T_ItemType>& rhs)
{
	return static_cast<U32>(lhs) <= static_cast<U32>(rhs);
}

template <class T_ItemType>
bool operator >= (const C_Handle<T_ItemType>& lhs, const C_Handle<T_ItemType>& rhs)
{
	return static_cast<U32>(lhs) >= static_cast<U32>(rhs);
}

template <class T_ItemType>
bool operator < (const C_Handle<T_ItemType>& lhs, const C_Handle<T_ItemType>& rhs)
{
	return static_cast<U32>(lhs) < static_cast<U32>(rhs);
}

template <class T_ItemType>
bool operator > (const C_Handle<T_ItemType>& lhs, const C_Handle<T_ItemType>& rhs)
{
	return static_cast<U32>(lhs) > static_cast<U32>(rhs);
}

} // namespace Util

#endif // HANDLE_H
