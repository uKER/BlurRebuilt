#ifndef HANDLEITEM_H
#define HANDLEITEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/Handle.h>
#include <actor_system/HandleDeclare.h>

namespace Util
{

/**	
@brief Base class for C_HandleItem classes.
Users should not need to use C_HandleItemBase directly and should normally
use the C_HandleItem templated class.
*/
class C_HandleItemBase
{
	friend class C_HandleManagerBase;

public:
	C_HandleItemBase() {}
	// auto: ~C_HandleItemBase();

	const C_HandleBase& GetHandle() const {return m_Handle;} ///< @returns the handle used to refer to this object

private:
	C_HandleBase		m_Handle;		///< handle value that references this object

	C_HandleItemBase(const C_HandleItemBase&); // can not be copied
	C_HandleItemBase& operator = (const C_HandleItemBase&); // can not be copied

};

/**	
@brief Base class for objects wanting to use handles.
@param T_ItemType object class that will derive from us.
@code
class MyClass : public Util::C_HandleItem<MyClass>
{
	// supports GetHandle() and can be used with C_HandleManager<MyClass>
};
@endcode
*/
template <class T_ItemType>
class C_HandleItem : public C_HandleItemBase
{
public:
	// auto: C_HandleItem();
	// auto: ~C_HandleItem();

	typedef C_Handle<T_ItemType> C_HandleType; ///< handle to our object type

	const C_HandleType& GetHandle() const {return *static_cast<const C_HandleType*>(&C_HandleItemBase::GetHandle());} ///< @returns the handle used to refer to this object

};

} // namespace Util

#endif // HANDLEITEM_H
