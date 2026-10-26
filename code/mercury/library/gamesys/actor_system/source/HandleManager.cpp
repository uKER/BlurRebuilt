#include "ActorSystemPch.h"

#include <actor_system/HandleManager.h>
#include <system/misc/Random.h>


/**
@page HandleSystem Handle System.

Handles provide a way to refer to objects without maintaining pointers, which
can be useful in a variety of situations. They also guard against accessing
destroyed objects, as the handle will detect that the object has been freed.

To use the handle system, the object that you wish to reference by handles 
must derive from the C_HandleItem templated base class.

You should provide factory-like methods for creating and destroying objects, 
which perform the allocation/deallocation of storage for those objects, and 
call the AllocateHandle()/FreeHandle() methods to manage handles for them.

Example:
@code
class ZipFile : public Util::C_HandleItem<ZipFile>
{
public:
	ZipFile(const Btl::String& path) : m_Path(path) {}

	File* CreateFile(const Btl::String& fileName);

private:
	Btl::String m_Path;

};

class FileSystem : public Util::C_HandleManager<ZipFile>
{
public:
	C_Handle CreateArchive(const Btl::String& path)
	{
		ZipFile* pZipFile = new ZipFile(path);
		return AllocateHandle(pZipFile);
	}

	void DestroyArchive(const C_Handle& handle)
	{
		ZipFile* pZipFile = GetItemPtr(handle);

		if (pZipFile != NULL)
		{
			FreeHandle(pZipFile);
			delete pZipFile;
		}
	}

	File* CreateFile(const C_Handle& archiveHandle, const Btl::String& fileName)
	{
		ZipFile* pZipFile = GetItemPtr(handle);

		if (pZipFile != NULL)
			return pZipFile->CreateFile(fileName);

		return NULL;
	}

};
@endcode
*/


namespace Util
{

/**
@param maxHandles maximum number of handles that can be allocated 
simultaneously, must be greater than zero.
@param fixedHandleValue uniquely identifies this handle manager instance.
*/
C_HandleManagerBase::C_HandleManagerBase(const U32 maxHandles, const U32 fixedHandleValue)
	: m_HandleIndexArray()
	, m_AssignedHandles()
	, m_pFirstFreeIndex(NULL)
	, m_pLastFreeIndex(NULL)
{
	BIZ_ASSERT((maxHandles > 0) && (maxHandles <= C_HandleData::kIndexSize));

	// allocate storage for index slots
	m_HandleIndexArray.Resize(maxHandles);

	// initialise the slots
	Util::C_Random rng;
	rng.Seed(fixedHandleValue);

    for (S32 i = 0; i < m_HandleIndexArray.Size(); ++i)
    {
		C_HandleIndexInfo* const pIndexInfo = &m_HandleIndexArray[i];
		pIndexInfo->m_HandleData.Init(static_cast<U32>(i), rng.Rand(), fixedHandleValue);
		SetIndexInfoNextFree(pIndexInfo, (i < (m_HandleIndexArray.Size() - 1)) ? &m_HandleIndexArray[i + 1] : NULL);
    }

	// initialise the list
    m_pFirstFreeIndex = &m_HandleIndexArray[0];
	m_pLastFreeIndex = &m_HandleIndexArray[m_HandleIndexArray.Size() - 1];
}

C_HandleManagerBase::~C_HandleManagerBase()
{
	// intentionally blank
}

U32 C_HandleManagerBase::Count() const
{
	return m_AssignedHandles.Size();
}

/**
The item being allocated a handle must not already have a handle. We will set
it's handle and store a reference to it that can be retrieved from the handle.
@param pItem item to allocate a handle for.
@returns allocated handle.
*/
C_HandleBase C_HandleManagerBase::AllocateHandleBase(C_HandleItemBase* pItem)
{
    if (pItem == NULL)
    {
        return C_HandleBase();
    }

	BIZ_ASSERT(static_cast<U32>(pItem->m_Handle) == 0); // pItem handle has already been allocated?

    if (m_pFirstFreeIndex == NULL)
    {
		LOGERROR(Debug::GetRootLogger(), "No Free Handles\n");
		return C_HandleBase();
    }

    // get the next free index slot and update the list
    C_HandleIndexInfo* const pIndexInfo = m_pFirstFreeIndex;
    m_pFirstFreeIndex = GetIndexInfoNextFree(pIndexInfo);

	if (m_pFirstFreeIndex == NULL)
	{
		m_pLastFreeIndex = NULL;
	}

	SetIndexInfoItem(pIndexInfo, pItem);
	m_AssignedHandles.PushBack(*pIndexInfo);

	// create the handle, update the pItem and we're done
	pItem->m_Handle.m_Data = pIndexInfo->m_HandleData;
	return pItem->m_Handle;
}

/**
Frees the handle to the item that was previously passed to 
AllocateHandleBase().
@param pItem to free handle for.
*/
void C_HandleManagerBase::FreeHandleBase(C_HandleItemBase* pItem)
{
	if ((pItem == NULL) || pItem->m_Handle.m_Data.IsNull())
	{
		return;
	}

	// get the slot that the pItem thinks it's stored in
	C_HandleIndexInfo* const pIndexInfo = &m_HandleIndexArray[pItem->m_Handle.m_Data.GetIndex()]; // if this asserts then we've got a bad handle

	if ((pItem->m_Handle.m_Data.AsU32() != pIndexInfo->m_HandleData.AsU32()) || (pItem != GetIndexInfoItem(pIndexInfo)))
	{
		// this is a bad handle, has already been freed, or maybe it was allocated by a different manager?
		DebugBreak();
		return;
	}

	// clear the pItem handle
	pItem->m_Handle = C_HandleBase();

	// clear the slot and return to the free list
	m_AssignedHandles.Erase(C_HandleIndexList::Iterator(pIndexInfo));
	pIndexInfo->m_HandleData.SpinCycle();

	SetIndexInfoNextFree(pIndexInfo, NULL);

	if (m_pFirstFreeIndex == NULL)
	{
		m_pFirstFreeIndex = pIndexInfo;
		m_pLastFreeIndex = pIndexInfo;
	}
	else
	{
		SetIndexInfoNextFree(m_pLastFreeIndex, pIndexInfo);
		m_pLastFreeIndex = pIndexInfo;
	}
}

} // namespace Util
