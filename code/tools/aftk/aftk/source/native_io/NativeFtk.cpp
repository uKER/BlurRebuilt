#include "stdafx.h"
#include "../aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeScene.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeSceneNode.h"
#include "NativeLegacyShaderConnection.h"
#include "NativeInfluenceClusterNode.h"

namespace Aftk
{
	Ut::SSSMultiMemoryPool* g_pNativeFtkMemoryPool = 0;
}

using namespace Aftk;

C_NativeFtk::C_NativeFtk()
{
	m_pScene = 0;
	m_pSceneConst = 0;
	g_pNativeFtkMemoryPool = new Ut::SSSMultiMemoryPool();
	g_pNativeFtkMemoryPool->Create();

	m_pPtrSerialisationMapping =
		new Ut::UtMap<const void*, S32>();
	m_pPtrDeserialisationMapping =
		new Ut::UtVector<Ut::DynamicVector<void*> >();

	m_pBlindDataResToIthIndexMapping = new
		Ut::UtMap<const C_BlindDataResource*, U32>();
}

C_NativeFtk::~C_NativeFtk()
{
	Clear();
	delete m_pPtrSerialisationMapping;
	delete m_pPtrDeserialisationMapping;
	delete m_pBlindDataResToIthIndexMapping;
	g_pNativeFtkMemoryPool->Destroy();
	delete g_pNativeFtkMemoryPool;
}
	
void C_NativeFtk::Clear()
{
	m_pPtrSerialisationMapping->Clear();
	m_pPtrDeserialisationMapping->Clear();
    
	m_objectsToResolve.Clear();

	m_blindDataMappingArray.Clear();
	m_pBlindDataResToIthIndexMapping->Clear();
    
	m_pScene = 0;
	m_pSceneConst = 0;
}

U32 C_NativeFtk::GetHighestSupportedFileVersion()
{
	return NATIVE_FILE_VERSION;
}

U32 C_NativeFtk::GetLowestSupportedFileVersion()
{
	return NATIVE_LOWEST_FILE_VERSION;
}

C_Status C_NativeFtk::GetFileVersion(
	const std::string& fileName, 
	U32& version) 
{
	C_Status status;

	C_NativeDiskReader reader;
	try
	{
		reader.Open(fileName);
	}
	catch (C_NativeDiskReader::C_ReadException e)
	{
		return e.GetStatus();
	}
	version = reader.GetVersion();
	reader.Close();
	return C_Status::E_Status_Success;
}
	
C_Status C_NativeFtk::Read(
	const std::string& fileName, 
	C_Scene *pScene, 
	const char* pOptions,
	I_Progress* pProgressCallback)
{
	try
	{
		C_Status status;

		if (pProgressCallback)
		{
			if (pProgressCallback->OnProgress(0))
			{
				return C_Status(C_Status::E_Status_Failure, "C_Scene loading aborted.");
			}
			pProgressCallback->OnPrintProgress(std::string("Reading ") + fileName + std::string("..."));
		}

		m_pScene = pScene;
		m_diskReader.Open(fileName);

		status = C_NativeScene::Deserialise(
			&m_diskReader,
			this, 
			pScene, 
			pProgressCallback);
		if (!status)
		{
			m_diskReader.Close();
			return status;
		}

		m_diskReader.Close();

		if (m_diskReader.GetVersion() < 200)
		{
			LOGERROR(g_aftkMainLogger, "Unsupported A3D version.\n");
			return C_Status::E_Status_Failure;
		}

		RebuildSceneNodeArray();

		//Repair mapping tables for remapped instances.
		{
			C_SceneNodeIterator it(pScene);
			while(it)
			{
				C_SceneNode* pSceneNode = it;
				if (pSceneNode->HasType(E_ObjType_SceneMeshRemappedInstance))
				{
					C_SceneMeshRemappedInstanceNode* pInstance = 
						(C_SceneMeshRemappedInstanceNode*)pSceneNode;
					pInstance->RepairMappingTables();
				}
				it++;
			}
		}
		
		if (pProgressCallback)
		{
			bool onCancel = pProgressCallback->OnProgress(100);
			if (onCancel)
			{
				return C_Status(C_Status::E_Status_Failure, "C_Scene reading aborted.");
			}
			pProgressCallback->OnPrintProgress("File read successfully.");
		}
	}
	catch (C_NativeDiskReader::C_ReadException ex)
	{
		//Stop warning 4101 - unreferenced local var.
		return ex.GetStatus();
	}

	//Repair polygon clusters, as some invalid a3ds were generated at some point that had
	//orphaned polygons.
	C_SceneNodeIterator it(pScene);
	while(it)
	{
		if ((&(*it))->GetType() == E_ObjType_SceneMesh)
		{
			C_SceneMeshNode* pSceneMeshNode = (C_SceneMeshNode*)(&(*it));
			pSceneMeshNode->RepairPolyClusters();
		}
		it++;
	}

	C_MemoryManager::CollectGarbage();
	return C_Status::E_Status_Success;
}

C_Status C_NativeFtk::Write(
	const std::string& fileName,
	U32 version, 
	const C_Scene *pScene,
	const char* pOptions,
	I_Progress* pProgressCallback)
{
	C_Status status;

	m_pSceneConst = pScene;

	if (pProgressCallback)
	{
		if (pProgressCallback->OnProgress(0))
		{
			return C_Status(C_Status::E_Status_Failure, "C_Scene writing aborted.");
		}
		pProgressCallback->OnPrintProgress(std::string("Writing ") + fileName + std::string("..."));
	}

	std::string metaData;
/*	for (U32 i = 0; i < exportParameters.GetNumAttributes(); i++)
	{
		const C_GenericAttribute* pAttr = &exportParameters.GetAttribute(i);
		if (pAttr->GetName() == "metadata")
		{
			if (pAttr->GetDataType() != E_GenericAttributeType_String)
			{
				return C_Status(C_Status::E_Status_Failure, "Invalid meta data type, must be a string.", 1);
			}
			pAttr->GetValue(metaData);
		}
	}
*/
	try
	{
		m_diskWriter.Open(
			fileName, 
			metaData);
		if (!status) return status;
		
		{
			status = C_NativeScene::Serialise(
				&m_diskWriter, 
				this, 
				pScene, 
				pProgressCallback);
			if (!status) 
			{
				m_diskWriter.Close();
				return status;
			}
			m_diskWriter.Close();
		}
	}
	catch (C_NativeDiskWriter::WriteException e)
	{
		return e.GetStatus();
	}

	if (pProgressCallback)
	{
		if (pProgressCallback->OnProgress(100))
		{
			return C_Status(C_Status::E_Status_Failure, "C_Scene writing aborted.");
		}
		pProgressCallback->OnPrintProgress("File written successfully.");
	}

	return C_Status::E_Status_Success;
}

bool C_NativeFtk::AddToIdPtrMapping(S32 id, void* pObject)
{
	//Assert that this is the first time that id has been encountered. If this is not the case
	//then an error has occurred somewhere.
	#ifdef _DEBUG
	AFTK_ASSERT (GetPtrFromId(id) == 0);
	#endif

	//Increment pointer hash-table size in large increments.
	U32 magicSizeIncrement = 1000000;

	if (id < 0)
		return true;

	if (id >= m_pPtrDeserialisationMapping->Size())
	{
		S32 oldSize = m_pPtrDeserialisationMapping->Size();
		S32 newSize = id + magicSizeIncrement;
		S32 memsetRange = newSize - oldSize;
		m_pPtrDeserialisationMapping->Resize(newSize);
		void* pStart = &(*m_pPtrDeserialisationMapping)[0 + oldSize];
		memset(pStart, 0, memsetRange * sizeof(void*));
	}

	///Build a mapping table that integer into pointers.
	(*m_pPtrDeserialisationMapping)[id] = pObject;
	m_objectsToResolve.PushBack(id);

	return true;
}

void* C_NativeFtk::GetPtrFromId(S32 id)
{
	if (id < 0 || id >= m_pPtrDeserialisationMapping->Size())
		// we failed to find the pointer.
		return 0;

	return (*m_pPtrDeserialisationMapping)[id];
}

S32 C_NativeFtk::GetIdFromPtr(const void* p) 
{
	if (!p)
		return -1;
#ifdef AFTK_USE_MEMORY_POOL
	Ut::UtMap<const void*, S32>::Iterator it;
//	std::map<const void*, S32, std::less<const void*>, C_NativeStdPoolAllocator<std::pair<const void*, S32> > >	::iterator it;
#else
	Ut::UtMap<const void*, S32>::Iterator it;
#endif
	it = (*m_pPtrSerialisationMapping).Find(p);
	if (it != (*m_pPtrSerialisationMapping).End())
	{
		//We found the pointer.
		return it->GetValue();
	}
	else
	{
		// we failed to find the pointer. Add it.
		S32 id = (S32)(*m_pPtrSerialisationMapping).Size();
		(*m_pPtrSerialisationMapping)[p] = id;
		return id;
	}
}

void C_NativeFtk::RebuildSceneNodeArray()
{
	//Fix orphan tracking array, this automatically allocates instance id's when nodes are created.
	//We want to override this. Therefore we delete the entire array and rebuild it from scratch
	m_pScene->m_sceneNodes.Clear();

	C_SceneNodeIterator it(m_pScene);
	while (it)
	{
		C_SceneNode* pSceneNode = it;
		m_pScene->m_sceneNodes[pSceneNode->GetUniqueId()] = pSceneNode;
		it++;
	}
}

C_Scene* C_NativeFtk::GetScenePtr()
{
	return m_pScene;
}

const C_Scene* C_NativeFtk::GetScenePtr() const
{
	return m_pSceneConst;
}

const Ut::UtVector< Ut::DynamicVector<S32> >& C_NativeFtk::GetObjectsToResolve()
{
	return m_objectsToResolve;
}


