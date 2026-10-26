#ifndef HANDLEMANAGER_H
#define HANDLEMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/Handle.h>
#include <actor_system/HandleDeclare.h>
#include <actor_system/HandleItem.h>

#include <Btl/Containers/list.h>
#include <Btl/Containers/vector.h>

namespace Util
{

/**
@brief Base class for all C_HandleManager classes.
User should not use this class directly, but should instead use the
C_HandleManager templated class.
C_HandleManagerBase exists to reduce code-bloat.
*/
class C_HandleManagerBase
{
public:
	explicit C_HandleManagerBase(const U32 maxHandles, const U32 fixedHandleValue);
	~C_HandleManagerBase();

	U32 Count() const; ///< @returns the number of handles used

protected:
	C_HandleBase AllocateHandleBase(C_HandleItemBase* pItem); ///< attempts to allocate a handle to this object
	void FreeHandleBase(C_HandleItemBase* pItem); ///< frees the handle to this object

	inline const C_HandleItemBase* GetBaseItemPtr(const C_HandleBase& handle) const; ///< access the base item referred to by this handle
	inline C_HandleItemBase* GetBaseItemPtr(const C_HandleBase& handle); ///< access the base item referred to by this handle

	inline bool IsValid(const C_HandleBase& handle) const; ///< checks if this handle referes to a valid object

	template <class T_ItemType, class T_VisitorType>
	bool VisitValidItems(T_VisitorType& visitor) const;

private:
	/// one for each handle slot, stored in a contiguous array
	struct C_HandleIndexInfo : public Btl::ListElementDouble<C_HandleIndexInfo>
	{
		C_HandleData	m_HandleData;		///< handle currently represented by this slot
		void*			m_pItemOrNextFree;	///< if this slot is used then it's C_HandleItemBase* for the item referred to, otherwise it's C_HandleIndexInfo* for the next item in the free list
	};

	typedef Btl::Vector<C_HandleIndexInfo> C_HandleIndexArray;
	typedef Btl::List<C_HandleIndexInfo> C_HandleIndexList;

	C_HandleIndexArray		m_HandleIndexArray;		///< storage for handle info slots
	C_HandleIndexList		m_AssignedHandles;		///< list of assigned handled
    C_HandleIndexInfo*		m_pFirstFreeIndex;		///< head of singly-linked list of free slots
	C_HandleIndexInfo*		m_pLastFreeIndex;		///< tail of singly-linked list of free slots

	C_HandleManagerBase(); // no default ctor
	C_HandleManagerBase(const C_HandleManagerBase&); // can not be copied
	C_HandleManagerBase& operator = (const C_HandleManagerBase&); // can not be copied

	inline const C_HandleItemBase* GetBaseItemPtrImpl(const C_HandleBase& handle) const; ///< access the base item referred to by this handle

	inline bool IsValidIndexInfoNextFree(const void* const pMem) const; ///< used for debug assertions

	inline C_HandleItemBase* GetIndexInfoItem(const C_HandleIndexInfo* pInfo) const;
	inline void SetIndexInfoItem(C_HandleIndexInfo* pInfo, C_HandleItemBase* const pItem) const;
	inline C_HandleIndexInfo* GetIndexInfoNextFree(const C_HandleIndexInfo* pInfo) const;
	inline void SetIndexInfoNextFree(C_HandleIndexInfo* pInfo, C_HandleIndexInfo* const pNext) const;

};


/// Interface for visiting handle items.
template <class T_ItemType>
struct I_HandleVisitor
{
	/**
	@brief Called for each handle item.
	@param pItem item being visited.
	@returns true to continue, false to abort.
	*/
	virtual bool VisitHandleItem(T_ItemType* pItem) = 0;
};


/**
@brief Base class for managing object handles.
User should derive from C_HandleManager to create a class that manages handles
to instances of T_ItemType. The deriving class can then use the GetItemPtr()
methods to translate handles into pointers internally.
@param T_ItemType object class that will be assigned handles.
*/
template <class T_ItemType>
class C_HandleManager : public C_HandleManagerBase
{
public:
	typedef C_Handle<T_ItemType> C_Handle; ///< handle type
	typedef C_HandleItem<T_ItemType> C_Item; ///< item type base class
	typedef I_HandleVisitor<T_ItemType> I_Visitor; ///< visitor type interface
	typedef I_HandleVisitor<const T_ItemType> I_ConstVisitor; ///< visitor type interface

	inline explicit C_HandleManager(const U32 maxHandles);
	inline ~C_HandleManager();

	inline bool IsValidHandle(const C_Handle& handle) const; ///< checks if this is a valid handle

	inline C_Handle AllocateHandle(C_Item* pItem); ///< allocates a handle to this object
	inline void FreeHandle(T_ItemType* pItem); ///< frees a handle assigned by AllocateHandle()

	inline const T_ItemType* GetItemPtr(const C_Handle& handle) const; ///< gets the item referred to by this handle
	inline T_ItemType* GetItemPtr(const C_Handle& handle); ///< gets the item referred to by this handle

	inline bool Visit(I_Visitor& visitor); ///< visits valid handle items
	inline bool Visit(I_ConstVisitor& visitor) const; ///< visits valid handle items

private:
	static U32 s_NextFixedHandleValue; ///< fixed handle value to use for next instance of this type

};

} // namespace Util

#define HANDLEMANAGER_INL
#include "HandleManager.inl"
#undef HANDLEMANAGER_INL

#endif // HANDLEMANAGER_H
