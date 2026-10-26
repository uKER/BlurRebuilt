#include "AudioPch.h"
#include "SystemHeaders.h"
#include <system/serialise/FilePath.h>
#include <System/memory/MemoryTracker.h>

Debug::Bool g_AudioDefManRenderDebug("Audio.System.InstanceStats.ShowOnScreen", false);
Debug::Float g_AudioObjectStatsYOffset("Audio.System.InstanceStats.YOffset", 140.f, Debug::Range(-3000.0f, 600.f));
Debug::Int g_AudioObjectStats1InstMemoryThreshold("Audio.System.InstanceStats.IndividualInstanceMemoryThreshold", 0);
Debug::Int g_AudioObjectStatsAllInstMemoryThreshold("Audio.System.InstanceStats.AllInstancesMemoryThreshold", 0);
Debug::Bool g_AudioDefManShowObjectsWithNoInstances("Audio.System.InstanceStats.ListObjectsWithNoInstances", true);

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
////////////////////////////////////////////////////////////////////////////////
//	Create
////////////////////////////////////////////////////////////////////////////////

// turn this into LoadResourceData somehow?
Audio::Definition* Audio::DefinitionManager::Create(const char* pchFilename)
{
	Sys::C_FilePath filePath(pchFilename);
	Sys::C_FilePath fileName = Sys::ExtractFilename(filePath);
	// has it already been loaded?
	Definition* pDef = Lookup(fileName.Chars());
	if (pDef)
	{
		#ifndef	CFG_GOLD
		// took this out as its now a legitimate scenario for a definition to exist without an object.
		//if(!pDef->GetObject())
		//	AUDIO_SYS_FATAL("Dynamic Object Not previously released");

		if (pDef->GetObject() && pDef->GetObject()->IsDoomed())
			AUDIO_SYS_FATAL("DefinitionManager::Create() - Trying to returning doomed object '%s'", pDef->Name());
		#endif
	
		pDef->IncRefCount();

		return pDef;
	}

	//	open
	Sys::I_File* file = Sys::GetDefaultFileSystem()->Open(filePath.Chars());
	if (!file)
		return NULL;

	Sys::C_FileInfo info;
    if (!Sys::GetDefaultFileSystem()->FileInfo(filePath.Chars(), &info))
	{
		delete file;
		return NULL;
	}
	U32 uSize = info.m_FileSize;

	//	once allocated, the Definition owns the memory
	System::Get()->PushCurrentMemoryAllocationId(5);
	MEMORY_TRACKER_PUSH("Audio.Objects.Definitions");

	U8* pData = (U8*)System::Alloc(uSize, 16);

	System::Get()->PopCurrentMemoryAllocationId();

    bool bSuccess = true;
    bSuccess = file->Read(pData, uSize);

////	close
	delete file;

    //  success
    if (bSuccess)
    {
        //  create new definition
		System::Get()->PushCurrentMemoryAllocationId(5);

        Definition* pNewDefinition = new Definition;

        if (Success == pNewDefinition->Init(fileName.Chars(), pData, uSize))
		{
			GetLock()->Acquire();
            m_apDefinitions.PushBack(pNewDefinition);
			GetLock()->Release();
			pNewDefinition->IncRefCount();
		
			AUDIO_SYS_DEBUG("Read audio object dat file '%s' ok", filePath.Chars());
		}
        else
        {
            delete pNewDefinition;
            pNewDefinition = NULL;
        }


		System::Get()->PopCurrentMemoryAllocationId();
		MEMORY_TRACKER_POP("Audio.Objects.Definitions");
		
        return pNewDefinition;
    }
    else
	{
		MEMORY_TRACKER_POP("Audio.Objects.Definitions");
		System::Free(pData);
        return NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Lookup
////////////////////////////////////////////////////////////////////////////////
Audio::Definition* Audio::DefinitionManager::Lookup(const char* pchFilename)
{
	Definition* pDef = NULL;

	GetLock()->Acquire();
	for (U32 uCount = 0; uCount < m_apDefinitions.Size(); uCount++)
	{
		if (m_apDefinitions[uCount]->FileName())
		{
			if (stricmp(m_apDefinitions[uCount]->FileName(), pchFilename) == 0)
			{
				pDef = m_apDefinitions[uCount];
				break;
			}
		}
	}
	GetLock()->Release();

	return pDef;
}

////////////////////////////////////////////////////////////////////////////////
//	RemoveFromList
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::DefinitionManager::RemoveFromList(Definition* pDefinition)
{
	Result res = Success;

    //  get index
	GetLock()->Acquire();
    S32 sIndex = m_apDefinitions.LinearSearch(pDefinition);
 
    if (sIndex == -1)
	{
		res = DefinitionNotFoundError;
	}
	else
	{
		//  remove it from array (reallocates)
		m_apDefinitions.Erase(sIndex);
	}
	GetLock()->Release();

    return res;
}

////////////////////////////////////////////////////////////////////////////////
//	Destroy
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::DefinitionManager::Destroy(Definition*& pDefinition)
{
	//if (pDefinition->GetObject() && (! pDefinition->GetObject()->IsDoomed()))
	//	AUDIO_SYS_FATAL("Trying to destroy definition '%s' when not flagged as doomed", pDefinition->Name());

	#ifndef	CFG_GOLD
	if (pDefinition->RefCount())
	{
		AUDIO_SYS_FATAL("Trying to destroy definition '%s' when still being referenced %d times", pDefinition->Name(), pDefinition->RefCount());
		return Failed;
	}
	#endif

	MEMORY_TRACKER_PUSH("Audio.Objects.Definitions");

	AUDIO_SYS_DEBUG("Destroying definition %s\n", pDefinition->Name());

    //  delete object
    delete pDefinition;
	pDefinition = NULL;

	MEMORY_TRACKER_POP("Audio.Objects.Definitions");

    return Success;
}

////////////////////////////////////////////////////////////////////////////////
//	Release
////////////////////////////////////////////////////////////////////////////////
Audio::Result Audio::DefinitionManager::Release(Definition*& pDefinition)
{
	if (!pDefinition)
		return Failed;

	if (!pDefinition->RefCount())
		return Failed;

	pDefinition->DecRefCount();

	if (!pDefinition->RefCount())
	{
		Result res = RemoveFromList(pDefinition);

		if (res != Success)
			return res;

		res = Destroy(pDefinition);

		if (res != Success)
			return res;
	}

    return Success;
}

#ifndef CFG_GOLD
namespace Audio 
{
	struct DefinitionEntry
	{
	public:
		DefinitionEntry(Audio::Definition* pDef) { m_pDef = pDef; }
		Audio::Definition* m_pDef;
		bool operator <= ( const DefinitionEntry& other ) const { return (strcmp(m_pDef->Name(), other.m_pDef->Name()) <= 0) ? true : false; }
	};
}
#endif

void Audio::DefinitionManager::RenderStats()
{
	#ifndef CFG_GOLD
	if (!g_AudioDefManRenderDebug)
		return;

	Btl::List<DefinitionEntry> list;
	for (U32 uCount = 0; uCount < m_apDefinitions.Size(); uCount++)
	{
		if (g_AudioDefManShowObjectsWithNoInstances || (!g_AudioDefManShowObjectsWithNoInstances && m_apDefinitions[uCount]->GetObject()->GetTotalInstanceCount()))
			list.PushBack(DefinitionEntry(m_apDefinitions[uCount]));
	}

	list.Sort();
	
	Btl::String str;
	static F32 fXCol1 = 40.f;
	static F32 fXCol2 = 400.f;
	static F32 fXCol3 = 475.f;
	static F32 fXCol4 = 550.f;
	static F32 fXCol5 = 650.f;
	static F32 fYInc = 20.f;
	
	F32 fY = g_AudioObjectStatsYOffset;
	U32 c=0;

	Audio::System::DrawBox(fXCol1, fY-fYInc/2.f, 800.f, fYInc*2.f, 0x802020a0);

	Audio::System::PrintAt(fXCol1, fY, 0xffffffff, 16.f, "OBJECT");
	Audio::System::PrintAt(fXCol2, fY, 0xffffffff, 16.f, "TOT.");
	Audio::System::PrintAt(fXCol2, fY+fYInc, 0xffffffff, 16.f, "INST.");
	Audio::System::PrintAt(fXCol3, fY, 0xffffffff, 16.f, "ACT.");
	Audio::System::PrintAt(fXCol3, fY+fYInc, 0xffffffff, 16.f, "INST.");
	Audio::System::PrintAt(fXCol4, fY, 0xffffffff, 16.f, "1 INST.");
	Audio::System::PrintAt(fXCol5, fY, 0xffffffff, 16.f, "ALL INST.");

	fY += fYInc*2.f;

	U32 uInstanceCount = 0;

	Btl::List<DefinitionEntry>::Iterator it;
	for (it = list.Begin(); it != list.End(); ++it, c++)
	{
		bool bShow = true;
		Object* pObject = it->m_pDef->GetObject();

		if (pObject)
		{
			if (g_AudioObjectStats1InstMemoryThreshold)
			{
				if (pObject->GetInstanceFootprint()/1024.f < g_AudioObjectStats1InstMemoryThreshold)
					bShow = false;
			}
			else
			if (g_AudioObjectStatsAllInstMemoryThreshold)
			{
				if (pObject->GetTotalInstanceFootprint()/1024.f < g_AudioObjectStatsAllInstMemoryThreshold)
					bShow = false;
			}

			uInstanceCount += pObject ? pObject->GetTotalInstanceCount() : 0;
		}

		if (bShow)
		{
			Audio::System::DrawBox(fXCol1, fY-fYInc/2.f, 800.f, fYInc, c&0x1 ? 0xc0404040 : 0xc0202020);

			Audio::System::PrintAt(fXCol1, fY, 0xffffffff, 16.f, "%s", it->m_pDef->Name());
			if (pObject)
			{
				Audio::System::PrintAt(fXCol2, fY, 0xffffffff, 16.f, "%d", pObject->GetTotalInstanceCount());
				Audio::System::PrintAt(fXCol3, fY, 0xffffffff, 16.f, "%d", pObject->GetActiveInstanceCount() - pObject->GetSuspendedInstanceCount());
				Audio::System::PrintAt(fXCol4, fY, 0xffffffff, 16.f, "%.2f k", pObject->GetInstanceFootprint()/1024.f);
				Audio::System::PrintAt(fXCol5, fY, 0xffffffff, 16.f, "%.2f k", pObject->GetTotalInstanceFootprint()/1024.f);
			}

			fY += fYInc;
		}
	}

	//fY += fYInc;
	Audio::System::DrawBox(fXCol1, fY-fYInc/2.f, 800.f, fYInc*2, c&0x1 ? 0xc0404040 : 0xc0202020);

	Audio::System::PrintAt(fXCol1, fY, 0xffffffff, 16.f, "----------------------------------------------------------------------------------------------");
	fY += fYInc;
	Audio::System::PrintAt(fXCol1, fY, 0xffffffff, 16.f, "%d objects loaded, %d instances active,", list.Size(), uInstanceCount );

	#endif
}

////////////////////////////////////////////////////////////////////////////////
//	Dump
////////////////////////////////////////////////////////////////////////////////
void Audio::DefinitionManager::DumpStats()
{
	#ifndef CFG_GOLD
	GetLock()->Acquire();

	Btl::List<DefinitionEntry> list;
	for (U32 uCount = 0; uCount < m_apDefinitions.Size(); uCount++)
	{
		if (g_AudioDefManShowObjectsWithNoInstances || (!g_AudioDefManShowObjectsWithNoInstances && m_apDefinitions[uCount]->GetObject()->GetTotalInstanceCount()))
			list.PushBack(DefinitionEntry(m_apDefinitions[uCount]));
	}

	list.Sort();

	//U32 uSize = m_apDefinitions.Size();
	U32 uInstanceCount=0;
	U32 uTotalInstanceMem=0;
	U32 uTotalDefinitionsMem=0;
	U32 uTotalObjectMem=0;
	U32 uTotalActiveInstanceCount=0;
	AUDIO_SYS_INFO("==========Definition Dump Start==========");
	AUDIO_SYS_INFO("[Please note: Memory usage figures are approximate (reasonably accurate but not 100%%)]]");
	AUDIO_SYS_INFO("----------------------------------------------------------------------------------------------------------------------------------------------------");
	AUDIO_SYS_INFO("    |Name                              | Defin- | Object |Compo-|Total |Active|Individual       |SPU                 |Total               | Ref");
	AUDIO_SYS_INFO("    |                                  | ition  | Bytes  |nents |Inst- |Inst- |Instance         |Instances           |Instances           | Count");
	AUDIO_SYS_INFO("    |                                  | Bytes  |        |      |ances |ances |Bytes            |Bytes               |Bytes               |");
	AUDIO_SYS_INFO("----------------------------------------------------------------------------------------------------------------------------------------------------");
	
    //for (U32 uCount = 0; uCount < uSize; uCount++)
	U32 uCount = 0;
	Btl::List<DefinitionEntry>::Iterator it;
	for (it = list.Begin(); it != list.End(); ++it, uCount++)
	{
		Object* pObject = it->m_pDef->GetObject();
		
		AUDIO_SYS_INFO("%3d | %-32s | %6d | %6d | %4d |  %3d |  %3d | %7d (%4dk) |    %7d (%4dk) |    %7d (%4dk) | %3d ", 
			uCount, 
			it->m_pDef->Name(), 
			it->m_pDef->GetDataSize(), 
			pObject ? pObject->GetComponentFootprint() : 0,
			it->m_pDef->NumComponents(), 
			pObject ? pObject->GetTotalInstanceCount() : 0,
			pObject ? pObject->GetActiveInstanceCount() - pObject->GetSuspendedInstanceCount() : 0,
			pObject ? pObject->GetInstanceFootprint() : 0,
			pObject ? pObject->GetInstanceFootprint()/1024 : 0,
			pObject ? pObject->GetSpuInstanceFootprint() : 0,
			pObject ? pObject->GetSpuInstanceFootprint()/1024 : 0,
			pObject ? pObject->GetTotalInstanceFootprint() : 0,
			pObject ? pObject->GetTotalInstanceFootprint()/1024 : 0,
			it->m_pDef->RefCount());

		uTotalInstanceMem += pObject ? pObject->GetTotalInstanceFootprint() : 0;
		uTotalDefinitionsMem += it->m_pDef->GetDataSize();
		uTotalObjectMem += pObject ? pObject->GetComponentFootprint() : 0;
		uInstanceCount += pObject ? pObject->GetTotalInstanceCount() : 0;
		uTotalActiveInstanceCount += pObject ? pObject->GetActiveInstanceCount() - pObject->GetSuspendedInstanceCount() : 0;
	}

	AUDIO_SYS_INFO("-----------------------------------------------------------------------------------------------------");
	AUDIO_SYS_INFO("%d definitions loaded, %d instances, %d instances active", m_apDefinitions.Size(), uInstanceCount, uTotalActiveInstanceCount);
	AUDIO_SYS_INFO("%d bytes used for instances (%d k)", uTotalInstanceMem, uTotalInstanceMem/1024);
	AUDIO_SYS_INFO("%d bytes used for definitions(%d k)", uTotalDefinitionsMem, uTotalDefinitionsMem/1024);
	AUDIO_SYS_INFO("%d bytes used for objects(%d k)", uTotalObjectMem, uTotalObjectMem/1024);
	AUDIO_SYS_INFO("%d bytes used for everything (%d k)", uTotalObjectMem + uTotalInstanceMem + uTotalDefinitionsMem, (uTotalObjectMem + uTotalInstanceMem + uTotalDefinitionsMem)/1024);
	AUDIO_SYS_INFO("==========Definition Dump End==========\n");
	GetLock()->Release();
	#endif
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
