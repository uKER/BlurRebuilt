#include "stdafx.h"

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>

#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Aftk/AftkRef.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Misc/AsyncHelpers.h>

namespace EditorLib
{

C_AftkRefManager::C_AftkRefManager()
{
}

C_AftkRefManager::~C_AftkRefManager()
{
	for (AftkRefRecordList::Iterator it = m_references.Begin(); it != m_references.End(); ++it)
	{
		C_AftkRefRecord& refRecord = *it;
		BIZ_ASSERT(refRecord.refCount == 0);
		refRecord.pRef->Unload();
		delete refRecord.pRef;
	}
}

void C_AftkRefManager::GetRefs(ConstAftkRefList& refList) const
{
	for (AftkRefRecordList::ConstIterator it = m_references.Begin(); it != m_references.End(); ++it)
	{
		const C_AftkRefRecord& refRecord = *it;
		const C_AftkRef* p = ((const C_AftkRef*)refRecord.pRef);
		refList.PushBack(p);
	}
}

const C_AftkRef* C_AftkRefManager::GetRef(const Btl::String& _filePath) const
{
	return const_cast<C_AftkRefManager*>(this)->GetRef(_filePath);
}

C_AftkRef* C_AftkRefManager::GetRef(const Btl::String& _filePath)
{
	Btl::String filePath = Misc::FormatUnixFileString(_filePath.Lower());
	for (AftkRefRecordList::ConstIterator it = m_references.Begin(); it != m_references.End(); ++it)
	{
		const C_AftkRefRecord& refRecord = *it;
		if (refRecord.pRef->GetOriginalFilePath() == filePath)
			return refRecord.pRef;
	}

	return 0;
}

C_AftkRef* C_AftkRefManager::CreateRef(const Btl::String& _filePath, bool isVisualisation)
{
	Btl::String filePath = Misc::FormatUnixFileString(_filePath.Lower());
	for (AftkRefRecordList::Iterator it = m_references.Begin(); it != m_references.End(); ++it)
	{
		C_AftkRefRecord& refRecord = *it;
		if (refRecord.pRef->GetOriginalFilePath() == filePath)
		{
			//Increment reference count;
			refRecord.refCount++;
			return refRecord.pRef;
		}
	}

	m_references.PushBack();
	C_AftkRefRecord& refRecord = m_references.Back();
	//Create a new Aftk reference data container.
	refRecord.pRef = new C_AftkRef(filePath, isVisualisation);
	//Init ref count.
	refRecord.refCount = 1;
	
	return refRecord.pRef;
}

C_Status C_AftkRefManager::DestroyRef(const Btl::String& _filePath)
{
	Btl::String filePath = Misc::FormatUnixFileString(_filePath.Lower());
	for (AftkRefRecordList::Iterator it = m_references.Begin(); it != m_references.End(); ++it)
	{
		C_AftkRefRecord& refRecord = *it;
		if (refRecord.pRef->GetOriginalFilePath() == filePath)
		{
			//If our ref count is about to be zero'd...
			if (refRecord.refCount == 1)
			{
				if (refRecord.pRef->IsLoaded())
					refRecord.pRef->Unload();

				delete refRecord.pRef;
				m_references.Erase(it);
			}
			else
			{
				//Decrement ref count.
				refRecord.refCount--;
			}
			
			C_Core::Instance().GetGlobalSignals().levelAftkRefRemoved(filePath);
			return E_Status_Success;
		}
	}

	return E_Status_Failure;
}

void C_AftkRefManager::SetCacheDirectory(const Btl::String &dir)
{
	m_CacheDirectory = dir;
}

const Btl::String& C_AftkRefManager::GetCacheDirectory() const
{
	return m_CacheDirectory;
}

void C_AftkRefManager::SetConversionTool(const Btl::String& path, const Btl::String& params, const Btl::String& metaQueryParams)
{
	m_ConversionToolPath = path;
	m_ConversionToolParams = params;
	m_ConversionToolMetaQueryParams = metaQueryParams;
}

void C_AftkRefManager::GetConverstionTool(Btl::String& path, Btl::String& params, Btl::String& metaQueryParams) const
{
	path = m_ConversionToolPath;
	params = m_ConversionToolParams;
	metaQueryParams = m_ConversionToolMetaQueryParams;
}

}