#ifndef HANDLEMANAGER_INL
#error HandleManager.inl should only be included by HandleManager.h
#endif

namespace Util
{

/** 
@param pMem memory address to check.
@returns true if this is a legitimate free-list pointer (can be NULL).
*/
inline bool C_HandleManagerBase::IsValidIndexInfoNextFree(const void* const pMem) const
{
	if (pMem == NULL)
		return true;

	const uintptr_t address = reinterpret_cast<uintptr_t>(pMem);
	const uintptr_t startOfArray = reinterpret_cast<uintptr_t>(&m_HandleIndexArray[0]);
	const uintptr_t sizeOfArray = static_cast<uintptr_t>(m_HandleIndexArray.Size() * sizeof(C_HandleIndexInfo));

	return (address >= startOfArray) && (address < (startOfArray + sizeOfArray));
}

/** 
@param info index info, must be a valid handle.
@returns item referred to.
*/
inline C_HandleItemBase* C_HandleManagerBase::GetIndexInfoItem(const C_HandleIndexInfo* pInfo) const
{
	BIZ_ASSERT(!IsValidIndexInfoNextFree(pInfo->m_pItemOrNextFree));
	return reinterpret_cast<C_HandleItemBase*>(pInfo->m_pItemOrNextFree);
}

/** 
@param info index into to set, will become a valid handle.
@param pItem item that the handle should refer to.
*/
inline void C_HandleManagerBase::SetIndexInfoItem(C_HandleIndexInfo* pInfo, C_HandleItemBase* const pItem) const
{
	BIZ_ASSERT(!IsValidIndexInfoNextFree(pItem));
	pInfo->m_pItemOrNextFree = pItem;
}

/** 
@param info index info, must not be a valid handle.
@returns next item in the free list.
*/
inline C_HandleManagerBase::C_HandleIndexInfo* C_HandleManagerBase::GetIndexInfoNextFree(const C_HandleIndexInfo* pInfo) const
{
	BIZ_ASSERT(IsValidIndexInfoNextFree(pInfo->m_pItemOrNextFree));
	return reinterpret_cast<C_HandleIndexInfo*>(pInfo->m_pItemOrNextFree);
}

/** 
@param info index into to set, will cease to be a valid handle.
@param pNext next item in free list.
*/
inline void C_HandleManagerBase::SetIndexInfoNextFree(C_HandleIndexInfo* pInfo, C_HandleIndexInfo* const pNext) const
{
	BIZ_ASSERT(IsValidIndexInfoNextFree(pNext));
	pInfo->m_pItemOrNextFree = pNext;
}

/**
@param handle handle to check.
@returns true if GetBaseItemPtr() will return non-NULL.
*/
inline bool C_HandleManagerBase::IsValid(const C_HandleBase& handle) const
{
	const C_HandleIndexInfo* const pIndexInfo = &m_HandleIndexArray[handle.m_Data.GetIndex()]; // if this asserts then we've got a bad handle
	BIZ_ASSERT(handle.m_Data.IsNull() || (handle.m_Data.GetFixed() == pIndexInfo->m_HandleData.GetFixed())); // if this asserts then the handle was created by a different manager
	return (handle.m_Data.AsU32() == pIndexInfo->m_HandleData.AsU32());
}

/**
Provides a common implementation shared by const and non-const accessors.
@param handle handle to retrieve the item for.
@returns NULL if the handle is not valid.
*/
FORCE_INLINE const C_HandleItemBase* C_HandleManagerBase::GetBaseItemPtrImpl(const C_HandleBase& handle) const
{
	const C_HandleIndexInfo* const pIndexInfo = &m_HandleIndexArray[handle.m_Data.GetIndex()]; // if this asserts then we've got a bad handle
	BIZ_ASSERT(handle.m_Data.IsNull() || (handle.m_Data.GetFixed() == pIndexInfo->m_HandleData.GetFixed())); // if this asserts then the handle was created by a different manager
	return (handle.m_Data.AsU32() == pIndexInfo->m_HandleData.AsU32()) ? GetIndexInfoItem(pIndexInfo) : NULL;
}

/** 
@param handle handle to desired object.
@returns NULL if the handle does not refer to a valid object.
*/
FORCE_INLINE const C_HandleItemBase* C_HandleManagerBase::GetBaseItemPtr(const C_HandleBase& handle) const 
{
	return GetBaseItemPtrImpl(handle);
}

/** 
@param handle handle to desired object.
@returns NULL if the handle does not refer to a valid object.
*/
FORCE_INLINE C_HandleItemBase* C_HandleManagerBase::GetBaseItemPtr(const C_HandleBase& handle) 
{
	return const_cast<C_HandleItemBase*>(GetBaseItemPtrImpl(handle));
}

/**
@brief Visits all valid items.
@param visitor visitor object, must implement <b>bool VisitHandleItem(T_ItemType*)</b>.
@returns false if the visitor aborted.
*/
template <class T_ItemType, class T_VisitorType> 
bool C_HandleManagerBase::VisitValidItems(T_VisitorType& visitor) const
{
	for (C_HandleIndexList::ConstIterator it = m_AssignedHandles.Begin(); it != m_AssignedHandles.End(); ++it)
		if (!visitor.VisitHandleItem(static_cast<T_ItemType*>(GetIndexInfoItem(&(*it)))))
			return false;

	return true;
}







template <class T_ItemType>
U32 C_HandleManager<T_ItemType>::s_NextFixedHandleValue = 0;

/**
@param maxHandles maximum number of handles that can be allocated.
*/
template <class T_ItemType>
inline C_HandleManager<T_ItemType>::C_HandleManager(const U32 maxHandles) : 
	C_HandleManagerBase(maxHandles, s_NextFixedHandleValue) 
{
	++s_NextFixedHandleValue;
}

template <class T_ItemType>
inline C_HandleManager<T_ItemType>::~C_HandleManager()
{
}

/**
User must call FreeHandle() before the item is destroyed.
@param pItem object to allocate handle to.
@returns handle that can be used to retrieve the object.
*/
template <class T_ItemType>
inline C_Handle<T_ItemType> C_HandleManager<T_ItemType>::AllocateHandle(C_Item* pItem) 
{
	return static_cast<C_Handle>(AllocateHandleBase(pItem));
}

/**
@param pItem item to remove the handle from.
*/
template <class T_ItemType>
inline void C_HandleManager<T_ItemType>::FreeHandle(T_ItemType* pItem) 
{
	FreeHandleBase(pItem);
}


/**
@param handle handle to test.
@returns true if this handle refers to a valid object.
*/
template <class T_ItemType>
inline bool C_HandleManager<T_ItemType>::IsValidHandle(const C_Handle& handle) const
{
	return IsValid(handle);
}

/**
@param handle handle to desired object.
@returns NULL if the handle does not refer to a valid object.
*/
template <class T_ItemType>
FORCE_INLINE const T_ItemType* C_HandleManager<T_ItemType>::GetItemPtr(const C_Handle& handle) const 
{
	return static_cast<const T_ItemType*>(GetBaseItemPtr(handle));
}

/**
@param handle handle to desired object.
@returns NULL if the handle does not refer to a valid object.
*/
template <class T_ItemType>
FORCE_INLINE T_ItemType* C_HandleManager<T_ItemType>::GetItemPtr(const C_Handle& handle) 
{
	return static_cast<T_ItemType*>(GetBaseItemPtr(handle));
}

/**
@param visitor object to visit the handle items.
@returns false if the visitor aborted.
*/
template <class T_ItemType>
inline bool C_HandleManager<T_ItemType>::Visit(I_Visitor& visitor)
{
	return VisitValidItems<T_ItemType, I_Visitor>(visitor);
}

/**
@param visitor object to visit the handle items.
@returns false if the visitor aborted.
*/
template <class T_ItemType>
inline bool C_HandleManager<T_ItemType>::Visit(I_ConstVisitor& visitor) const
{
	return VisitValidItems<const T_ItemType, I_ConstVisitor>(visitor);
}

} // namespace Util
