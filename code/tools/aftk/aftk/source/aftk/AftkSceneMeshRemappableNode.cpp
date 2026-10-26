#include <stdafx.h>

using namespace Aftk;

C_SceneMeshRemappableNode::C_SceneMeshRemappableNode(C_Scene* pScene) 
	: I_SceneMeshNode(pScene)
{
	OneTimeInit();
}

C_SceneMeshRemappableNode::~C_SceneMeshRemappableNode()
{
	ClearMappingTables();
}

void C_SceneMeshRemappableNode::SetTextureCoordinateOffset(U32 set, const TextureCoordinate4& offset)
{
	if ((U32)m_uvwxOffsets.Size() <= set)
	{
		for (U32 i = (U32)m_uvwxOffsets.Size(); i < set+1; i++)
		{
			m_uvwxOffsets.PushBack(offset);
		}
	}

	m_uvwxOffsets[set] = offset;
}

void C_SceneMeshRemappableNode::SetPolygonBlindDataMapping(
	U16 index,
	const C_BlindDataResource* pResource)
{
	AFTK_ASSERT(m_pScene != 0);

	C_SharedResourceMapping<C_BlindDataNode>* pSharedResourceMapping =
		m_polygonBlindDataMappingArray[index];
	
	//Check if the targeted mapping index has an identical resource to the one specified.
	//If so, abort.
	if (pSharedResourceMapping->GetSharedResourcePtr() == pResource)
		return;

	//Decrement the old resource reference count prior to adding the new resource.
	if (pSharedResourceMapping->GetSharedResourcePtr())
	{
		m_pScene->DecrementResourceReferenceCount(
			pSharedResourceMapping->GetSharedResourcePtr());
	}

	//Set the new resource.
	pSharedResourceMapping->SetSharedResourcePtr(pResource);

	//Increment the scene's reference count for the new resource. Note that we of course
	//use the same reference count as the previous resource.
	if (pResource)
		m_pScene->IncrementResourceReferenceCount(pResource);
}

void C_SceneMeshRemappableNode::SetPolygonMaterialMapping(
	U16 index, 
	const C_MaterialResource* pResource)
{
	AFTK_ASSERT(m_pScene != 0);

	C_SharedResourceMapping<C_MaterialNode>* pSharedResourceMapping =
		m_polygonMaterialMappingArray[index];
	
	//Check if the targeted mapping index has an identical resource to the one specified.
	//If so, abort.
	if (pSharedResourceMapping->GetSharedResourcePtr() == pResource)
		return;

	//Decrement the old resource reference count prior to adding the new resource.
	if (pSharedResourceMapping->GetSharedResourcePtr())
	{
		m_pScene->DecrementResourceReferenceCount(
			pSharedResourceMapping->GetSharedResourcePtr());
	}

	//Remove old uv linkage
	if (pSharedResourceMapping->GetSharedResourcePtr())
		DeleteUVLinks(pSharedResourceMapping->GetSharedResourcePtr());

	//Set the new resource.
	pSharedResourceMapping->SetSharedResourcePtr(pResource);

	//Increment the scene's reference count for the new resource. Note that we of course
	//use the same reference count as the previous resource.
	if (pResource)
		m_pScene->IncrementResourceReferenceCount(pResource);
}

const TextureCoordinate4& C_SceneMeshRemappableNode::GetTextureCoordinateOffset(U32 set) const
{
	return m_uvwxOffsets[set];
}

const C_BlindDataResource* C_SceneMeshRemappableNode::GetPolygonBlindDataMappingPtr(S16 index) const
{
	if (index == -1)
	{
		return 0;
	}
	return m_polygonBlindDataMappingArray[index]->GetSharedResourcePtr();
}

const C_MaterialResource* C_SceneMeshRemappableNode::GetPolygonMaterialMappingPtr(S16 index) const
{
	if (index == -1)
	{
		return 0;
	}
	return m_polygonMaterialMappingArray[index]->GetSharedResourcePtr();
}

void C_SceneMeshRemappableNode::GetPolygonBlindDataMappingArray(
	Ut::UtVector< Ut::DynamicVector<C_SharedResourceMapping<C_BlindDataNode>* > >& mapping) const
{
	mapping = m_polygonBlindDataMappingArray;
}

void C_SceneMeshRemappableNode::GetPolygonMaterialMappingArray(
	Ut::UtVector< Ut::DynamicVector<C_SharedResourceMapping<C_MaterialNode>* > >& mapping) const
{
	mapping = m_polygonMaterialMappingArray;
}

U32 C_SceneMeshRemappableNode::GetPolygonBlindDataMappingReferenceCount(S16 index) const
{
	if (index == -1)
	{
		return 0;
	}
	return m_polygonBlindDataMappingArray[index]->GetReferenceCount();
}

U32 C_SceneMeshRemappableNode::GetPolygonMaterialMappingReferenceCount(S16 index) const
{
	if (index == -1)
	{
		return 0;
	}
	return m_polygonMaterialMappingArray[index]->GetReferenceCount();
}

S16 C_SceneMeshRemappableNode::GetPolygonBlindDataMappingIndex(const C_BlindDataResource* pResource) const
{
	return GetPolygonResourceMappingIndex(
		m_polygonBlindDataMappingArray,	pResource);
}

S16 C_SceneMeshRemappableNode::GetPolygonMaterialMappingIndex(const C_MaterialResource* pResource) const
{
	return GetPolygonResourceMappingIndex(
		m_polygonMaterialMappingArray,
		pResource);
}


U16 C_SceneMeshRemappableNode::GetNumPolygonBlindDataMappings() const
{
	return (U16)m_polygonBlindDataMappingArray.Size();
}

U16 C_SceneMeshRemappableNode::GetNumPolygonMaterialMappings() const
{
	return (U16)m_polygonMaterialMappingArray.Size();
}

E_ObjType C_SceneMeshRemappableNode::GetType() const
{
	return E_ObjType_SceneMeshRemappable;
}

bool C_SceneMeshRemappableNode::HasType(E_ObjType type) const
{
	if (I_SceneMeshNode::HasType(type))
		return true;
	if (type == C_SceneMeshRemappableNode::GetType()) 
		return true;

	return false;
}


void C_SceneMeshRemappableNode::CopyPolygonBlindDataMappings(const C_SceneMeshRemappableNode* rhs)
{
	AFTK_ASSERT(m_pScene);

	for (U16 i = 0 ; i < (U16)m_polygonBlindDataMappingArray.Size(); i++)
	{
		//Decrement the scene's reference count for the existing resources. 
		m_pScene->DecrementResourceReferenceCount(
			m_polygonBlindDataMappingArray[i]->GetSharedResourcePtr());
		delete m_polygonBlindDataMappingArray[i];
	}

	//Clear our map and array representation of the resource mapping data.
	m_polygonBlindDataMappingArray.Clear();

	for (U16 i = 0; i < (U16)rhs->m_polygonBlindDataMappingArray.Size(); i++)
	{
		m_polygonBlindDataMappingArray.PushBack(
			new C_SharedResourceMapping< C_BlindDataNode >(*(rhs->m_polygonBlindDataMappingArray[i])));
		//Don't copy reference counts between source nodes.
		if (IsRemappedSource())
		{
			m_polygonBlindDataMappingArray[i]->SetReferenceCount(0);
		}
		else
		{
			m_polygonBlindDataMappingArray[i]->SetReferenceCount(1);
		}

		//If we're copying between scenes, we need to perform resource copying.
		if (m_pScene != rhs->m_pScene)
		{
			const C_BlindDataResource* pRhsRes = 
				rhs->m_polygonBlindDataMappingArray[i]->GetSharedResourcePtr();
			//Copy blind data resource
			C_BlindDataNode bd = pRhsRes->GetContent();
			//Add blind data to our scene.
			const C_BlindDataResource* pRes = m_pScene->AddBlindDataNode(bd);
			//The function below will take care of our reference counting for us.
			m_polygonBlindDataMappingArray[i]->SetSharedResourcePtr(pRes);
		}

		//Increment the scene's reference count for the new resource. 
		m_pScene->IncrementResourceReferenceCount(
			m_polygonBlindDataMappingArray[i]->GetSharedResourcePtr());
	}
}

void C_SceneMeshRemappableNode::CopyPolygonMaterialMappings(const C_SceneMeshRemappableNode* rhs, const MaterialRemappingHashTable* pMappingTable)
{
	AFTK_ASSERT(m_pScene);

	for (U16 i = 0 ; i < (U16)m_polygonMaterialMappingArray.Size(); i++)
	{
		//Decrement the scene's reference count for the existing resources. 
		m_pScene->DecrementResourceReferenceCount(
			m_polygonMaterialMappingArray[i]->GetSharedResourcePtr());
		delete m_polygonMaterialMappingArray[i];
	}

	//Clear our map and array representation of the resource mapping data.
	m_polygonMaterialMappingArray.Clear();

	for (U16 i = 0; i < (U16)rhs->m_polygonMaterialMappingArray.Size(); i++)
	{
		m_polygonMaterialMappingArray.PushBack(
			new C_SharedResourceMapping< C_MaterialNode >(*(rhs->m_polygonMaterialMappingArray[i])));
		//Don't copy reference counts between source nodes.
		if (IsRemappedSource())
		{
			m_polygonMaterialMappingArray[i]->SetReferenceCount(0);
		}
		else
		{
			m_polygonMaterialMappingArray[i]->SetReferenceCount(1);
		}

		//If we're copying between scenes, we need to perform resource copying.
		if (m_pScene != rhs->m_pScene)
		{
			const C_MaterialResource* pRhsRes = 
				rhs->m_polygonMaterialMappingArray[i]->GetSharedResourcePtr();

			const C_MaterialResource* pRes = 0;

			if (pMappingTable)
			{
				//Old -> new material mapping.
				MaterialRemappingHashTable::ConstIterator it = pMappingTable->Find(pRhsRes);
				if (it != pMappingTable->End())
				{
					//We found an existing record.
					pRes = it->GetValue();
				}
			}
			if (!pRes)
			{
				//If we assert here when we've provided a mapping table it means materials were
				//missing from our table - which is likely to be programmer error. We'll have
				//to resort to material copying (slow) in this case.
				AFTK_ASSERT(!pMappingTable);

				//Copy blind data resource
				C_MaterialNode mtl = pRhsRes->GetContent();
				//Add blind data to our scene.
				pRes = m_pScene->AddMaterialNode(mtl, true);
			}
			m_polygonMaterialMappingArray[i]->SetSharedResourcePtr(pRes);
		}

		//Increment the scene's reference count for the new resource.
		m_pScene->IncrementResourceReferenceCount(
			m_polygonMaterialMappingArray[i]->GetSharedResourcePtr());
	}
}

void C_SceneMeshRemappableNode::CopyTextureCoordinateOffsets(const C_SceneMeshRemappableNode* rhs)
{
	m_uvwxOffsets = rhs->m_uvwxOffsets;
}

/*
void C_SceneMeshRemappableNode::Output() const
{
	AFTK_DO_BEGIN << "Mesh material table:\n" AFTK_DO_INFO_END;
	for (U16 iResource = 0; iResource < (U16)m_polygonMaterialMappingArray.Size(); iResource++)
	{
		AFTK_DO_BEGIN << "[" << iResource << "] Material name [" 
			<< m_polygonMaterialMappingArray[iResource]->GetSharedResourcePtr()->GetContent().GetName()
			<< "] scene reference count [" << m_polygonMaterialMappingArray[iResource]->GetSharedResourcePtr()->GetReferenceCount()
			<< "] poly reference count [" << m_polygonMaterialMappingArray[iResource]->GetReferenceCount() << "]\n"
		AFTK_DO_INFO_END;
	}
}
*/
S16 C_SceneMeshRemappableNode::Exists(const C_MaterialNode& material)
{
	for (U32 iMaterial = 0; iMaterial < (U32)m_polygonMaterialMappingArray.Size(); iMaterial++)
	{
		if (m_polygonMaterialMappingArray[iMaterial]->GetSharedResourcePtr()->GetContent() == material)
		{
			return iMaterial;
		}
	}

	return -1;
}

S16 C_SceneMeshRemappableNode::Exists(const C_BlindDataNode& blindData)
{
	for (U32 iBlindData = 0; iBlindData < (U32)m_polygonBlindDataMappingArray.Size(); iBlindData++)
	{
		if (m_polygonBlindDataMappingArray[iBlindData]->GetSharedResourcePtr()->GetContent() == blindData)
		{
			return iBlindData;
		}
	}

	return -1;
}

U16 C_SceneMeshRemappableNode::DeleteUnusedPolygonMaterialMappings()
{
//#pragma message("TEMPORARY HACK IN DeleteUnusedPolygonMaterialMappings TO GET PGR4 SCRATCH SYSTEM WORKING")
	if (GetNumInstancesOfThisNode() > 0)
		return 0;

	for (U16 iMapping = 0; iMapping < (U16)m_polygonMaterialMappingArray.Size(); iMapping++)
	{
		C_SharedResourceMapping<C_MaterialNode>* pMapping = 
			(m_polygonMaterialMappingArray[iMapping]);

		if (pMapping->GetReferenceCount() == 0)
		{
			DeletePolygonMaterialMapping(iMapping);
			U16 numRemoved = 1;
			numRemoved += DeleteUnusedPolygonMaterialMappings();
			return numRemoved;
		}
	}

	return 0;
}

U16 C_SceneMeshRemappableNode::DeleteUnusedPolygonBlindDataMappings()
{
	for (U16 iMapping = 0; iMapping < (U16)m_polygonBlindDataMappingArray.Size(); iMapping++)
	{
		C_SharedResourceMapping< C_BlindDataNode >* pMapping = 
			m_polygonBlindDataMappingArray[iMapping];

		if (pMapping->GetReferenceCount() == 0)
		{
			DeletePolygonBlindDataMapping(iMapping);
			U16 numRemoved = 1;
			numRemoved += DeleteUnusedPolygonBlindDataMappings();
			return numRemoved;
		}
	}

	return 0;
}

void C_SceneMeshRemappableNode::DeleteUVLinks(const C_MaterialResource* pRes)
{
	//Sync uv-linking information.
	Ut::UtMap<const C_MaterialResource*, C_SceneMeshRemappableNode::UVLinkageList>::Iterator iter;

	iter = m_uvLinkInfo.Find(pRes);
	if (iter != m_uvLinkInfo.End())
		m_uvLinkInfo.Erase(iter);
}

S32 C_SceneMeshRemappableNode::GetUVSetIndex(
	const C_MaterialResource* pRes,
	const C_ShaderImageNode* pShaderImageNode,
	C_Status* pStatus) const {

	Ut::UtMap<const C_MaterialResource*, UVLinkageList>::ConstIterator iter;
	iter = m_uvLinkInfo.Find(pRes);

	if (iter == m_uvLinkInfo.End()) 
	{
		//***********************************
		//Didn't find material - error
		//***********************************
		*pStatus = C_Status(C_Status::E_Status_Failure, "No material node found"); 
		return -1;

	}
	else 
	{
		const UVLinkageList& linkage = iter->GetValue();
		for (unsigned int i = 0; i < (U32)linkage.Size(); i++) 
		{
			if (linkage[i].pImageNode == pShaderImageNode) 
				return linkage[i].uvIndex;
		}
	}

	//***********************************
	//Didn't find UVSet - warning/error
	//***********************************
	*pStatus = C_Status(C_Status::E_Status_Success, "No UV set found"); 
	return -1;

}

void C_SceneMeshRemappableNode::AddUVSetIndex(
	const C_MaterialResource* pRes,
	const C_ShaderImageNode* pShaderImageNode,
	S32 index)
{
	Ut::UtMap<const C_MaterialResource*, UVLinkageList>::Iterator iter;
	iter = m_uvLinkInfo.Find(pRes);

	C_UvLink newUVLink;
	newUVLink.pImageNode = pShaderImageNode;
	newUVLink.uvIndex = index;

	if (iter == m_uvLinkInfo.End()) 
	{
		// Didn't find material: Create a new entry in the map
		UVLinkageList linkage;
		linkage.PushBack(newUVLink);

		m_uvLinkInfo[pRes] = linkage;
	}
	else
	{
		// Found the material: Add the new UVset index to the UVLinkageList
		UVLinkageList& linkage = iter->GetValue();
		linkage.PushBack(newUVLink);
	}

}

void C_SceneMeshRemappableNode::RemapUVLinkageMaterialPtr(
	const C_MaterialResource* pOldMaterialRes, 
	const C_MaterialResource* pNewMaterialRes)
{

	Ut::UtMap<const C_MaterialResource*, UVLinkageList>::Iterator iter;

	iter = m_uvLinkInfo.Find(pOldMaterialRes);

	if (iter == m_uvLinkInfo.End())
		return;
	
	UVLinkageList uvLinkageListToRemap = iter->GetValue();
	
	//Erase old map entry here somewhere
	m_uvLinkInfo.Erase(iter);

	//Remap image node pointers
	for (U32 i = 0; i < (U32)uvLinkageListToRemap.Size(); i++)
	{
		const C_ShaderImageNode* pOldImageNode = uvLinkageListToRemap[i].pImageNode;
		const C_ShaderImageNode* pNewImageNode = 
			(const C_ShaderImageNode*)pNewMaterialRes->GetContent().GetShaderNodePtr(pOldImageNode->GetNodeIndex());

		uvLinkageListToRemap[i].pImageNode = pNewImageNode;
	}

	// We know our new material ptr, and the new image node ptrs
	// Add to our map again.
	m_uvLinkInfo[pNewMaterialRes] = uvLinkageListToRemap;
}

bool C_SceneMeshRemappableNode::operator== (const C_SceneMeshNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneMeshRemappableNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_SceneMeshRemappableNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	if (classToAbortTest == E_ObjType_SceneMeshRemappable)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && I_SceneMeshNode::IsEqualToAll(rhs, classToAbortTest, flags));
}

bool C_SceneMeshRemappableNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	//First check numbers of components as trivial rejection, before
	//testing the component values

	if (!_rhs.HasType(E_ObjType_SceneMeshRemappable))
	{
		return false;
	}

	C_SceneMeshRemappableNode& rhs = (C_SceneMeshRemappableNode&) _rhs;

	if (
		(m_uvwxOffsets.Size() != rhs.m_uvwxOffsets.Size()) ||
		(m_polygonBlindDataMappingArray.Size() != rhs.m_polygonBlindDataMappingArray.Size()) ||
		(m_polygonMaterialMappingArray.Size() != rhs.m_polygonMaterialMappingArray.Size())
		)
		return false;

	if (m_uvwxOffsets.Size() > 0)
	{
		if (memcmp(&m_uvwxOffsets[0], &rhs.m_uvwxOffsets[0], sizeof(TextureCoordinate4)*m_uvwxOffsets.Size()) != 0)
			return false;
	}
	for (U32 i = 0 ; i < (U32)m_polygonBlindDataMappingArray.Size(); i++)
	{
		if (*m_polygonBlindDataMappingArray[i] != *rhs.m_polygonBlindDataMappingArray[i])
			return false;
	}
	for (U32 i = 0 ; i < (U32)m_polygonMaterialMappingArray.Size(); i++)
	{
		if (*m_polygonMaterialMappingArray[i] != *rhs.m_polygonMaterialMappingArray[i])
			return false;
	}

	return true;
}

void C_SceneMeshRemappableNode::Clear()
{
	ClearThis();
	I_SceneMeshNode::Clear();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneMeshRemappableNode::OneTimeInit()
{
}

void C_SceneMeshRemappableNode::ClearThis()
{
	if (IsRemappedSource())
	{
		for (
//			Ut::UtSet<C_SceneNode*>::Iterator it = m_instancesOfThisNode.Begin();
			Ut::UtVector<Ut::DynamicVector<C_SceneNode*> >::Iterator it = m_instancesOfThisNode.Begin();
			it != m_instancesOfThisNode.End(); 
			++it)
		{
			C_SceneNode* pSceneNode = *it;
			if (pSceneNode->HasType(E_ObjType_SceneMeshRemappable))
			{
				C_SceneMeshRemappableNode* pInstanceOfThisNode =
					(C_SceneMeshRemappableNode*)pSceneNode;
				//Instances of this node cannot be remapped sources. Just double check here.
				AFTK_ASSERT(pInstanceOfThisNode->IsRemappedSource());

				//Clear remapping and offset information for instance.
				pInstanceOfThisNode->ClearMappingTables();
				pInstanceOfThisNode->m_uvwxOffsets.Clear();
			}
		}
		//Clear our own remapping and offset information.
		ClearMappingTables();
		m_uvwxOffsets.Clear();
	}
}

U32 C_SceneMeshRemappableNode::ProcessProblems(bool fix)
{
	U32 errorCount = 0;
	errorCount += C_SceneNode::ProcessProblems(fix);

	for (Ut::UtMap<const C_MaterialResource*, C_SceneMeshRemappableNode::UVLinkageList>::Iterator it = 
		m_uvLinkInfo.Begin(); it != m_uvLinkInfo.End(); ++it)
	{
		C_SceneMeshRemappableNode::UVLinkageList& linkageList = it->GetValue();
		for (U32 i = 0; i < (U32)linkageList.Size(); i++)
		{
			const C_UvLink& uvLinkage = linkageList[i];
			if (uvLinkage.uvIndex >= GetNumTextureCoordinateSets())
			{
				AFTK_DO_BEGIN << "Mesh " << GetName() << " UV linkage set >= num sets" AFTK_DO_DEBUG_END;
				#ifdef _DEBUG
				AFTK_ASSERT(!("UV linkage set >= num sets."));
				#endif
				errorCount++;
			}
			//Check material exists for this mesh.
			bool foundMaterial = false;
			for (U32 j = 0; j < (U32)m_polygonMaterialMappingArray.Size(); j++)
				if (m_polygonMaterialMappingArray[j]->GetSharedResourcePtr() == it->GetKey())
					foundMaterial = true;

			if (!foundMaterial)
			{
				AFTK_DO_BEGIN << "Mesh " << GetName() << " UV linkage to bad material." AFTK_DO_DEBUG_END;
				#ifdef _DEBUG
				AFTK_ASSERT(!("UV linkage to bad material."));
				#endif
				errorCount++;
			}
		}
	}

	return errorCount;
}

void C_SceneMeshRemappableNode::ClearMappingTables()
{
	//Decrement scene reference counts for shared resources.
	for(; m_polygonBlindDataMappingArray.Size() > 0;)
	{
		U32 i = (U32)m_polygonBlindDataMappingArray.Size()-1;
		DeletePolygonBlindDataMapping(i);
	}
	m_polygonBlindDataMappingArray.Clear();

	for(; m_polygonMaterialMappingArray.Size() > 0;)
	{
		U32 i = (U32)m_polygonMaterialMappingArray.Size()-1;
		DeletePolygonMaterialMapping(i);
	}
	m_polygonMaterialMappingArray.Clear();
}

C_SceneMeshRemappableNode::C_SceneMeshRemappableNode(const C_SceneMeshRemappableNode& rhs)
: I_SceneMeshNode(0)
{
	OneTimeInit();
	*this = rhs;
}

const C_SceneMeshRemappableNode& C_SceneMeshRemappableNode::Copy(const C_SceneMeshRemappableNode& rhs, const MaterialRemappingHashTable* pMappingTable)
{
	I_SceneMeshNode::operator =(rhs);

	ClearThis();

	//Clear old blind data mapping information
	for (U16 i = 0; i < (U16)m_polygonBlindDataMappingArray.Size(); i++)
	{
		delete m_polygonBlindDataMappingArray[i];
	}
	m_polygonBlindDataMappingArray.Clear();

	//Clear old material mapping information
	for (U32 i = 0 ; i < (U16)m_polygonMaterialMappingArray.Size(); i++)
	{
		delete m_polygonMaterialMappingArray[i];
	}
	m_polygonMaterialMappingArray.Clear();

	//Copy blind data mapping data from rhs
	CopyPolygonBlindDataMappings(&rhs);

	//Copy material mapping data from rhs
	CopyPolygonMaterialMappings(&rhs, pMappingTable);

	CopyTextureCoordinateOffsets(&rhs);

	return *this;
}

const C_SceneMeshRemappableNode& C_SceneMeshRemappableNode::operator=(const C_SceneMeshRemappableNode& rhs)
{
	return Copy(rhs, 0);
}

S16 C_SceneMeshRemappableNode::AddPolygonBlindDataMapping(
	const C_BlindDataResource* pResource, bool forceAddition)
{
	AFTK_ASSERT(m_pScene != 0);

	S16 resourceIndex = AddPolygonResourceMapping(
		m_polygonBlindDataMappingArray, 
		pResource,
		forceAddition);

	return resourceIndex;
}


S16 C_SceneMeshRemappableNode::AddPolygonMaterialMapping(
	const C_MaterialResource* pResource, bool forceAddition)
{
	AFTK_ASSERT(m_pScene != 0);

	S16 resourceIndex = AddPolygonResourceMapping(
		m_polygonMaterialMappingArray, 
		pResource,
		forceAddition);

	return resourceIndex;
}

S16 C_SceneMeshRemappableNode::AddPolygonBlindDataMappingSetCount(
	const C_BlindDataResource* pResource, U32 refCountToSet)
{
	//NOTE: THIS FUNCTION SHOULD _ONLY_ BE CALLED WHEN LOADING FROM DISK
	S32 oldCount = m_polygonBlindDataMappingArray.Size();
	//Call add mapping function...
	S16 resourceIndex =  AddPolygonBlindDataMapping(pResource, true);
	if (oldCount != m_polygonBlindDataMappingArray.Size())
	{
		m_polygonBlindDataMappingArray[resourceIndex]->SetReferenceCount(refCountToSet);
	}

	return resourceIndex;
}

S16 C_SceneMeshRemappableNode::AddPolygonMaterialMappingSetCount(
	const C_MaterialResource* pResource, U32 refCountToSet)
{
	//NOTE: THIS FUNCTION SHOULD _ONLY_ BE CALLED WHEN LOADING FROM DISK
	S32 oldCount = m_polygonMaterialMappingArray.Size();
	//Call add mapping function...
	S16 resourceIndex = AddPolygonMaterialMapping(pResource, true);
	if (oldCount != m_polygonMaterialMappingArray.Size())
	{
		m_polygonMaterialMappingArray[resourceIndex]->SetReferenceCount(refCountToSet);
	}

	return resourceIndex;
}

void C_SceneMeshRemappableNode::IncrementPolygonMaterialMappingReferenceCount(U16 index)
{
	AFTK_ASSERT(IsRemappedSource());
	IncrementPolygonResourceMappingReferenceCount(m_polygonMaterialMappingArray, index);
}

void C_SceneMeshRemappableNode::IncrementPolygonBlindDataMappingReferenceCount(U16 index)
{
	AFTK_ASSERT(IsRemappedSource());
	IncrementPolygonResourceMappingReferenceCount(m_polygonBlindDataMappingArray, index);
}

void C_SceneMeshRemappableNode::DecrementPolygonBlindDataMappingReferenceCount(U16 index)
{
	AFTK_ASSERT(IsRemappedSource());
	DecrementPolygonResourceMappingReferenceCount(m_polygonBlindDataMappingArray, index);
}

void C_SceneMeshRemappableNode::DecrementPolygonMaterialMappingReferenceCount(U16 index)
{
	AFTK_ASSERT(IsRemappedSource());
	DecrementPolygonResourceMappingReferenceCount(m_polygonMaterialMappingArray, index);
}

void C_SceneMeshRemappableNode::DeletePolygonBlindDataMapping(U16 mappingIndex)
{
	AFTK_ASSERT((S32)mappingIndex < m_polygonBlindDataMappingArray.Size());
	//Sync with remapped instances.
	for (
		//Ut::UtSet<Aftk::C_SceneNode*>::Iterator it = m_instancesOfThisNode.Begin(); 
		Ut::UtVector<Ut::DynamicVector<C_SceneNode*> >::Iterator it = m_instancesOfThisNode.Begin();
		it != m_instancesOfThisNode.End(); ++it)
	{
		Aftk::C_SceneNode* pSceneNode = *it;
//		Aftk::C_SceneNode* pSceneNode = it->GetValue();
		if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		{
			C_SceneMeshRemappedInstanceNode* pInst = (C_SceneMeshRemappedInstanceNode*)pSceneNode;
			//Strictly speaking the number of mapping entries should by sync'd
			if (mappingIndex < pInst->GetNumPolygonBlindDataMappings())
				pInst->DeletePolygonBlindDataMapping(mappingIndex);
		}
	}

	DeletePolygonResourceMapping(
		m_polygonBlindDataMappingArray, 
		mappingIndex);
}

void C_SceneMeshRemappableNode::DeletePolygonMaterialMapping(U16 mappingIndex)
{
	AFTK_ASSERT((S32)mappingIndex < m_polygonMaterialMappingArray.Size());
	//Sync with remapped instances.
	for (
	//	Ut::UtSet<Aftk::C_SceneNode*>::Iterator it = m_instancesOfThisNode.Begin(); 
		Ut::UtVector<Ut::DynamicVector<C_SceneNode*> >::Iterator it = m_instancesOfThisNode.Begin();
		it != m_instancesOfThisNode.End(); ++it)
	{
		Aftk::C_SceneNode* pSceneNode = *it;
//		Aftk::C_SceneNode* pSceneNode = it->GetValue();
		if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		{
			C_SceneMeshRemappedInstanceNode* pInst = (C_SceneMeshRemappedInstanceNode*)pSceneNode;
			//Strictly speaking the number of mapping entries should by sync'd
			if (mappingIndex < pInst->GetNumPolygonMaterialMappings())
				pInst->DeletePolygonMaterialMapping(mappingIndex);
		}
	}

	//Remove any uv linkage information
	DeleteUVLinks(m_polygonMaterialMappingArray[mappingIndex]->GetSharedResourcePtr());
	
	DeletePolygonResourceMapping(
		m_polygonMaterialMappingArray, 
		mappingIndex);
}

#endif
