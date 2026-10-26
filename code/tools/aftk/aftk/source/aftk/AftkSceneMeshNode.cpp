#include <stdafx.h>
#include <aftk/AftkBuildAdjacency.h>
#include <aftk/AftkStringTable.h>
#include <core/containers/bitvector.h>

#define AFTK_RESIZE_LOD_LEVELS_IF_NEEDED if (lodLevel >= (U32)m_lodLevels.Size()) {	m_lodLevels.Resize(lodLevel+1); }

//#define MERCURY_WELD
//#define WELD_POLYGONS

namespace Aftk
{
	struct C_UVSetOffsets
	{
		U32 uvIndexOffset;
		U32 tanIndexOffset;
		U32 binIndexOffset;
	};
}

using namespace Aftk;

C_Node* C_SceneMeshNode::Clone(C_Scene* pScene) const
{
	C_SceneMeshNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneMeshNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneMeshNode(pScene);
		(C_SceneMeshNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_Node* C_SceneMeshNode::Clone(C_Scene* pScene, const MaterialRemappingHashTable& mappingTable) const
{
	C_SceneMeshNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneMeshNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneMeshNode(pScene);
		(C_SceneMeshNode&)(*pNewNode).Copy(*this, &mappingTable);
	}

	return pNewNode;
}

const C_SceneMeshNode& C_SceneMeshNode::Copy(const C_SceneMeshNode& rhs, const MaterialRemappingHashTable* pMappingTable)
{
	// base-class asignment operator
	C_SceneMeshRemappableNode::Copy(rhs, pMappingTable);

	ClearThis();

	U16 numMaterialMappings = GetNumPolygonMaterialMappings();
	AFTK_ASSERT(rhs.GetNumPolygonMaterialMappings() == numMaterialMappings);
	U16 numBlindDataMappings = GetNumPolygonBlindDataMappings();
	AFTK_ASSERT(rhs.GetNumPolygonBlindDataMappings() == numBlindDataMappings);

	m_polygonVertices = rhs.m_polygonVertices;

	m_vertexPositions = rhs.m_vertexPositions;

	m_vertexNormals = rhs.m_vertexNormals;
    
	m_lodLevels = rhs.m_lodLevels;

	//Remap polygon mesh ptrs
	for (U32 iLodLevel = 0; iLodLevel < (U32)m_lodLevels.Size(); iLodLevel++)
	{
		for (U32 jPoly = 0; jPoly < (U32)rhs.m_lodLevels[iLodLevel].GetNumPolygons(); jPoly++)
		{
			C_Polygon* pPoly = &m_lodLevels[iLodLevel].GetPolygon(jPoly);
			pPoly->AttachToMesh(this);
			pPoly->m_pPolygonCluster = 0;
			AFTK_ASSERT(pPoly->GetMaterialMappingIndex() < numMaterialMappings);
			AFTK_ASSERT(pPoly->GetBlindDataMappingIndex() < numBlindDataMappings);
		}
	}

	// outer loop iterates thru UV sets
	m_textureCoordinateSets.Reserve(rhs.m_textureCoordinateSets.Size());
	for (U32 uvSet = 0; uvSet < (U32)rhs.m_textureCoordinateSets.Size(); uvSet++)
	{
		// inner loop iterates thru the UVs of this set
		m_textureCoordinateSets.PushBack(rhs.m_textureCoordinateSets[uvSet]);
	}

	m_vertexDataChannelInfo = rhs.m_vertexDataChannelInfo;

	//Delete poly clusters
	for (U32 iPolyCluster = 0; iPolyCluster < (U32)m_polygonClusters.Size(); iPolyCluster++)
	{
		delete m_polygonClusters[iPolyCluster];
	}
	m_polygonClusters.Clear();

	//Copy new poly clusters and remap polygon pointers.
	for (U32 iPolyCluster = 0; iPolyCluster < (U32)rhs.m_polygonClusters.Size(); iPolyCluster++)
	{
		const C_PolygonCluster* pRhsPolyCluster = rhs.m_polygonClusters[iPolyCluster];
		C_PolygonCluster* pPolyCluster = new C_PolygonCluster(this);
		*pPolyCluster = *pRhsPolyCluster;
//		pPolyCluster->AttachToMesh(this);

		m_polygonClusters.PushBack(pPolyCluster);
		for (U32 jLodLevel = 0; jLodLevel < pRhsPolyCluster->GetNumLodLevels(); jLodLevel++)
		{
			for (U32 kPoly = 0; kPoly < pRhsPolyCluster->GetNumPolygonIndices(jLodLevel); kPoly++)
			{
				U32 polyIndex = pRhsPolyCluster->GetPolygonIndex(jLodLevel, kPoly);
				C_Polygon* pPoly = &GetPolygon(jLodLevel, polyIndex);
				pPoly->m_pPolygonCluster = pPolyCluster;
			}
		}
	}

	//Rebuild animation helpers if needed.
	if ((rhs.GetAnimationHelpersBuilt() == E_AnimHelperState_WeightBuilt) ||
		(rhs.GetAnimationHelpersBuilt() == E_AnimHelperState_GeoCacheWeightBuilt))
	{
		BuildAnimationHelpers();
	}

	return *this;
}

C_SceneMeshNode::C_SceneMeshNode(C_Scene* pScene) : C_SceneMeshRemappableNode(pScene)
{
	OneTimeInit();
}

///\brief Builds a new mesh from the given boned instance.
///
///@param rhs The boned instance node.
C_SceneMeshNode::C_SceneMeshNode(const C_SceneMeshBonedInstanceNode& rhs)
: C_SceneMeshRemappableNode(0)
{
	OneTimeInit();

	*this = rhs;
}

C_SceneMeshNode::~C_SceneMeshNode()
{
	ClearThis();

	//Delete the default cluster, this is not removed by invoking 'clear'.
	delete m_polygonClusters[0];
}

E_ObjType C_SceneMeshNode::GetType() const
{
	return E_ObjType_SceneMesh;
}

bool C_SceneMeshNode::HasType(E_ObjType type) const
{
	if (C_SceneMeshRemappableNode::HasType(type))
		return true;
	if (type == C_SceneMeshNode::GetType()) 
		return true;

	return false;
}

void C_SceneMeshNode::SetVertexPositionInitialCapacity(U32 capacity)
{
	if (capacity > (U32)m_vertexPositions.Capacity())
	{
		m_vertexPositions.Reserve(capacity);
	}
}

void C_SceneMeshNode::SetVertexNormalInitialCapacity(U32 capacity)
{
	if (capacity > (U32)m_vertexNormals.Capacity())
	{
		m_vertexNormals.Reserve(capacity);
	}
}

S32 C_SceneMeshNode::AddTextureCoordinateSet()
{
	SetTextureCoordinateSetSize(m_textureCoordinateSets.Size() + 1);
	return m_textureCoordinateSets.Size()-1;
}

void C_SceneMeshNode::SetTextureCoordinateSetSize(U32 size)
{
	m_textureCoordinateSets.Resize(size);

	if (m_uvwxOffsets.Size() != size)
	{
		m_uvwxOffsets.Resize(size);
		for (U32 i = 0; i < size; i++)
		{
			m_uvwxOffsets[i] = TextureCoordinate4(0,0,0,0);
		}
		for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
		{
			C_PolygonVertex* pPolyVert = &m_polygonVertices[iPolyVert];
			PadPolygonVertexTextureCoordinateSetIndices(pPolyVert);
		}
	}
}

void C_SceneMeshNode::SetTextureCoordinateInitialCapacity(U32 set, U32 capacity)
{
	ResizeUVSetsIfNeeded(set);
	m_textureCoordinateSets[set].SetTextureCoordinateInitialCapacity(capacity);
}

void C_SceneMeshNode::SetVertexTangentInitialCapacity(U32 set, U32 capacity)
{
	ResizeUVSetsIfNeeded(set);
	m_textureCoordinateSets[set].SetTangentInitialCapacity(capacity);
}

void C_SceneMeshNode::SetVertexBinormalInitialCapacity(U32 set, U32 capacity)
{
	ResizeUVSetsIfNeeded(set);
	m_textureCoordinateSets[set].SetBinormalInitialCapacity(capacity);
}

void C_SceneMeshNode::SetPolygonInitialCapacity(U32 lodLevel, U32 capacity)
{
	AFTK_RESIZE_LOD_LEVELS_IF_NEEDED;

	m_lodLevels[lodLevel].SetPolygonInitialCapacity(capacity);
}

void C_SceneMeshNode::SetPolygonInitialCapacity(U32 capacity)
{
	U32 lodLevel = 0;
	AFTK_RESIZE_LOD_LEVELS_IF_NEEDED;

	m_lodLevels[0].SetPolygonInitialCapacity(capacity);
}

void C_SceneMeshNode::SetPolygonClusterInitialCapacity(U32 capacity)
{
	if (capacity > (U32)m_polygonClusters.Capacity())
	{
		m_polygonClusters.Reserve(capacity);
	}
}

void C_SceneMeshNode::SetPolygonVertexInitialCapacity(U32 capacity)
{
	if (capacity > (U32)m_polygonVertices.Capacity())
	{
		m_polygonVertices.Reserve(capacity);
	}
}

// Addition
S32 C_SceneMeshNode::AddVertexPosition(const Position3& position)
{
	DirtyGeometryDependantCaches();

	m_vertexPositions.PushBack(position);
	return (S32)m_vertexPositions.Size()-1;
}

S32 C_SceneMeshNode::AddVertexNormal(const Normal3& vertexNormal)
{
	DirtyGeometryDependantCaches();
	m_vertexNormals.PushBack(vertexNormal);
	return (S32)m_vertexNormals.Size()-1;
}

S32	C_SceneMeshNode::AddVertexTangent(U32 set, const Vector3F32& tangent)
{
	DirtyGeometryDependantCaches();
	ResizeUVSetsIfNeeded(set);	
	return (S32)m_textureCoordinateSets[set].AddTangent(tangent);
}

S32	C_SceneMeshNode::AddVertexBinormal(U32 set, const Vector3F32& binormal)
{
	DirtyGeometryDependantCaches();
	ResizeUVSetsIfNeeded(set);	
	return (S32)m_textureCoordinateSets[set].AddBinormal(binormal);
}

void C_SceneMeshNode::ResizeUVSetsIfNeeded(U32 set)
{
	if (set >= (U32)m_textureCoordinateSets.Size())
	{
//		Ut::UtSet<C_SceneNode*>::Iterator it;
		Ut::UtVector<Ut::DynamicVector<C_SceneNode*> >::Iterator it;
		for (it = m_instancesOfThisNode.Begin(); it != m_instancesOfThisNode.End(); it++)
		{
		//	if (it->GetKey()->HasType(E_ObjType_SceneMeshRemappable))
			if ((*it)->HasType(E_ObjType_SceneMeshRemappable))
			{
				//This is a remappable instance of this mesh - update its uv info.
				C_SceneMeshRemappableNode* pInstance = (C_SceneMeshRemappableNode*)*it;
//				C_SceneMeshRemappableNode* pInstance = (C_SceneMeshRemappableNode*)it->GetKey();
				for (U32 i = m_uvwxOffsets.Size(); i < set+1; i++)
				{
					//Add uv offsets to instance.
					pInstance->m_uvwxOffsets.PushBack(TextureCoordinate4(0,0,0,0));
				}
			}
		}

		m_textureCoordinateSets.Resize(set+1);
		for (U32 i = m_uvwxOffsets.Size(); i < set+1; i++)
		{
			m_uvwxOffsets.PushBack(TextureCoordinate4(0,0,0,0));
		}

		for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
		{
			C_PolygonVertex* pPolyVert = &m_polygonVertices[iPolyVert];
			PadPolygonVertexTextureCoordinateSetIndices(pPolyVert);
		}
	}
}

S32 C_SceneMeshNode::AddTextureCoordinate(U32 set, const TextureCoordinate4& uvwx)
{
	DirtyGeometryDependantCaches();
	ResizeUVSetsIfNeeded(set);
	return (S32)m_textureCoordinateSets[set].AddTextureCoordinate(uvwx);
}

S32 C_SceneMeshNode::AddPolygonVertex(const C_PolygonVertex& polyVert)
{
	DirtyGeometryDependantCaches();
	m_polygonVertices.PushBack(polyVert);
	C_PolygonVertex* pPolyVert = &m_polygonVertices[m_polygonVertices.Size()-1];

	if (pPolyVert->GetNumTextureCoordinateIndices() < GetNumTextureCoordinateSets())
	{
		PadPolygonVertexTextureCoordinateSetIndices(pPolyVert);
	}
	if (pPolyVert->GetNumData() < GetNumVertexDataChannels())
	{
		PadPolygonVertexDataChannels(pPolyVert);
	}

	return (U32)m_polygonVertices.Size()-1;
}

S32 C_SceneMeshNode::AddPolygonCluster()
{
	DirtyGeometryDependantCaches();
	m_polygonClusters.PushBack(new C_PolygonCluster(this));
	return m_polygonClusters.Size()-1;
}

void C_SceneMeshNode::SetTextureCoordinateSetName(U32 set, const std::string& name)
{
	DirtyGeometryDependantCaches();
	ResizeUVSetsIfNeeded(set);
	m_textureCoordinateSets[set].SetName(name);
}

S32 C_SceneMeshNode::AddVertexDataChannelInfo(const C_VertexDataChannelInfo& info)
{
	DirtyGeometryDependantCaches();
	SetVertexDataChannelInfo(m_vertexDataChannelInfo.Size(), info);
	return (S32)m_vertexDataChannelInfo.Size()-1;
}

void C_SceneMeshNode::DeleteVertexDataChannelInfo(U32 channel)
{
	DirtyGeometryDependantCaches();
	m_vertexDataChannelInfo.EraseByIndex(channel);
	for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
	{
		C_PolygonVertex* pPolyVert = &m_polygonVertices[iPolyVert];
		pPolyVert->DeleteData(channel);
	}
}

S32 C_SceneMeshNode::SetVertexDataChannelInfo(U32 channel, const C_VertexDataChannelInfo& info)
{
	DirtyGeometryDependantCaches();

	//Don't allow gaps in vertex data channels.
	AFTK_ASSERT(channel < (U32)m_vertexDataChannelInfo.Size()+1);
	if (channel >= (U32)m_vertexDataChannelInfo.Size())
	{
		m_vertexDataChannelInfo.Resize(channel+1);
		m_vertexDataChannelInfo[channel] = info;

		for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
		{
			C_PolygonVertex* pPolyVert = &m_polygonVertices[iPolyVert];
			PadPolygonVertexDataChannels(pPolyVert);
		}
	}
	else
	{
		if (m_vertexDataChannelInfo[channel].GetDataType() != info.GetDataType())
		{
			//This will remove any existing data for that channel on the polygon vertices.
			DeleteVertexDataChannelInfo(channel);
			//This will padd out the 
			return AddVertexDataChannelInfo(info);
		}
		else
		{
			m_vertexDataChannelInfo[channel] = info;
		}
	}

	return channel;
}

C_Status C_SceneMeshNode::BuildVertexChannelMergeTable(
	Ut::Vector<S32>& vertexChannelMergeSources,
	const C_SceneMeshNode* pMesh,
	bool mergeVertexChannelsByName)
{
	U32 nFinalChannels = Maths::Max(
		GetNumVertexDataChannels(),
		pMesh->GetNumVertexDataChannels());
	vertexChannelMergeSources.Resize(nFinalChannels);
	if (nFinalChannels == 0)
		return C_Status::E_Status_Success;

	memset(&vertexChannelMergeSources[0], -1, sizeof(S32)*vertexChannelMergeSources.Size());

	//Perform vertex channel sanity checks.
	if (mergeVertexChannelsByName)
	{
		Ut::BitVector sourceVertexChannelUsage;
		sourceVertexChannelUsage.Resize(nFinalChannels);
		sourceVertexChannelUsage.ClearBits();

		Ut::UtSet<std::string> vertexChannelNames;
		for (U32 i = 0; i < GetNumVertexDataChannels(); i++)
		{
			const C_VertexDataChannelInfo& lhsInfo = GetVertexDataChannelInfo(i);
			if (vertexChannelNames.Find(lhsInfo.GetName()) != vertexChannelNames.End())
			{
				assert(!"LHS Vertex channel names must be unique when merging channels by name."); 
				return C_Status::E_Status_Failure;
			}
			vertexChannelNames.Insert(lhsInfo.GetName());
		}
		vertexChannelNames.Clear();
		for (U32 i = 0; i < pMesh->GetNumVertexDataChannels(); i++)
		{
			const C_VertexDataChannelInfo& rhsInfo = pMesh->GetVertexDataChannelInfo(i);
			if (vertexChannelNames.Find(rhsInfo.GetName()) != vertexChannelNames.End())
			{
				assert(!"RHS Vertex channel names must be unique when merging channels by name."); 
				return C_Status::E_Status_Failure;
			}
			vertexChannelNames.Insert(rhsInfo.GetName());
		}

		for (U32 i = 0; i < GetNumVertexDataChannels(); i++)
		{
			for (U32 j = 0; j < pMesh->GetNumVertexDataChannels(); j++)
			{
				const C_VertexDataChannelInfo& lhsInfo = GetVertexDataChannelInfo(i);
				const C_VertexDataChannelInfo& rhsInfo = pMesh->GetVertexDataChannelInfo(j);
				if (lhsInfo.GetName() == rhsInfo.GetName())
				{
					vertexChannelMergeSources[i] = j;
					sourceVertexChannelUsage.SetBit(j);
				}
			}
		}

		//Add merge sources for channels we've not yet matched up by name. These are new channels
		//previously unknown in the target mesh.
		for (U32 iTarget = 0; iTarget < (U32)vertexChannelMergeSources.Size(); iTarget++)
		{
			bool found = false;
			for (U32 jSource = 0; jSource < (U32)vertexChannelMergeSources.Size(); jSource++)
			{
				//If this source is in use for this target.
				if (vertexChannelMergeSources[iTarget] == jSource)
					found = true;
			}
			if (found)
				//We're already added this to our source array.
				continue;

			//Set source to be the first unused one we encounter.
			for (U32 jSource = 0; jSource < (U32)pMesh->GetNumVertexDataChannels(); jSource++)
			{
				//If we've found an unused source channel.
				if (!sourceVertexChannelUsage.GetBitValue(jSource))
				{
					//Let's use it!
					vertexChannelMergeSources[iTarget] = jSource;
					sourceVertexChannelUsage.SetBit(jSource);
					break;
				}
			}
		}
	}
	else
	{
		for (U32 i = 0; i < pMesh->GetNumVertexDataChannels(); i++)
		{
			vertexChannelMergeSources[i] = i;
		}
	}

	//Sanity check channel types.
	{
		U32 lhsNumDataChannels = GetNumVertexDataChannels();
		U32 rhsNumDataChannels = pMesh->GetNumVertexDataChannels();
		for (U32 lhsTarget = 0; lhsTarget < lhsNumDataChannels; lhsTarget++)
		{
			S32 rhsSource = vertexChannelMergeSources[lhsTarget];
			if (rhsSource == -1)
				continue;
			
			const C_VertexDataChannelInfo& lhsInfo = GetVertexDataChannelInfo(lhsTarget);
			const C_VertexDataChannelInfo& rhsInfo = pMesh->GetVertexDataChannelInfo(rhsSource);
			if (lhsInfo.GetDataType() != rhsInfo.GetDataType())
			{
				assert(!"Cannot merge vertex channels with differing types.");
				return C_Status::E_Status_Failure;
			}
		}
	}

	return C_Status::E_Status_Success;
}


C_Status C_SceneMeshNode::AddMesh(
	const C_SceneMeshNode* pMesh,
	bool mergeVertexChannelsByName)
{
	//We cannot add a mesh to itself.
	if (pMesh == this)
		return C_Status::E_Status_Failure;
	DirtyGeometryDependantCaches();

	C_Status status;

	//Define lhs -> rhs vertex channel targeting.
	Ut::Vector<S32> vertexChannelMergeSources;
	status = BuildVertexChannelMergeTable(
		vertexChannelMergeSources,
		pMesh,
		mergeVertexChannelsByName);
	if (!status) return status;

	for (U32 i = 0; i < pMesh->GetNumLodLevels(); i++)
	{
		status = AddMeshPolygonData(pMesh, i);
		if (!status) return status;
	}

	status = AddMeshVertexData(
		pMesh, 
		false,
		vertexChannelMergeSources);
	if (!status) return status;

	return C_Status::E_Status_Success;
}

C_Status C_SceneMeshNode::AddMeshVertexData(
	const C_SceneMeshNode* pMesh, 
	bool mergeWeightsByEnvIndex,
	const Ut::Vector<S32>& vertexChannelMergeSources)
{
	//We cannot add a mesh to itself.
	if (pMesh == this)
		return C_Status::E_Status_Failure;

	U32 positionIndexOffset			= GetNumVertexPositions();
	U32 normalIndexOffset			= GetNumVertexNormals();
	U32 polygonVertexIndexOffset	= GetNumPolygonVertices();
	U32 numUVSetOffsets				= GetNumTextureCoordinateSets();

	//Generate a mapping table from our incoming mesh (rhs) to this mesh (lhs).
	U32* pRHSUVSetToLHSUVSetMapping = new U32[pMesh->GetNumTextureCoordinateSets()];
	for (U32 iSet = 0; iSet < pMesh->GetNumTextureCoordinateSets(); iSet++)
	{
		S32 targetIndex = GetTextureCoordinateSetIndex(pMesh->GetTextureCoordinateSetName(iSet));
		if (targetIndex == -1)
		{
			//We couldn't find an existing uv set in to which to merge.
			//Generate a new uv set.
			U32 newSetIndex = GetNumTextureCoordinateSets();
			SetTextureCoordinateSetSize(newSetIndex+1);
			//Set the uv set name.
			SetTextureCoordinateSetName(
				newSetIndex,
				pMesh->GetTextureCoordinateSetName(iSet));
			//Copy uv offset info.
			SetTextureCoordinateOffset(
				newSetIndex,
				pMesh->GetTextureCoordinateOffset(iSet));
			
			//Store mapping.
			pRHSUVSetToLHSUVSetMapping[iSet] = newSetIndex;
		}
		else
		{
			//Store mapping.
			pRHSUVSetToLHSUVSetMapping[iSet] = (U32)targetIndex;
		}
	}

	//These offsets are used to adjust the incoming uv indices per set.
	Ut::UtVector< Ut::DynamicVector<C_UVSetOffsets> > uvSetOffsets;
	if (numUVSetOffsets > 0)
	{
		uvSetOffsets.Resize(numUVSetOffsets);
		for (U32 iSet = 0; iSet < numUVSetOffsets; iSet++)
		{
			uvSetOffsets[iSet].uvIndexOffset = GetNumTextureCoordinates(iSet);
			uvSetOffsets[iSet].tanIndexOffset = GetNumVertexTangents(iSet);
			uvSetOffsets[iSet].binIndexOffset = GetNumVertexBinormals(iSet);
		}
	}
	{
		for (U32 iVPos = 0; iVPos < pMesh->GetNumVertexPositions(); iVPos++)
		{
			AddVertexPosition(pMesh->GetVertexPosition(iVPos));
		}
	}
	//Copy vertex normals.
	{
		for (U32 iVNorm = 0; iVNorm < pMesh->GetNumVertexNormals(); iVNorm++)
		{
			AddVertexNormal(pMesh->GetVertexNormal(iVNorm));
		}
	}

	for (U32 iSet = 0; iSet < pMesh->GetNumTextureCoordinateSets(); iSet++)
	{
		U32 targetUVSet = pRHSUVSetToLHSUVSetMapping[iSet];
		//Copy texture coordinates, tangents and binormals.
		for (U32 jUV  = 0; jUV < pMesh->GetNumTextureCoordinates(iSet); jUV++)
		{
			AddTextureCoordinate(targetUVSet, pMesh->GetTextureCoordinate(iSet, jUV));
		}
		for (U32 jTan  = 0; jTan < pMesh->GetNumVertexTangents(iSet); jTan++)
		{
			AddVertexTangent(targetUVSet, pMesh->GetVertexTangent(iSet, jTan));
		}
		for (U32 jBin  = 0; jBin < pMesh->GetNumVertexBinormals(iSet); jBin++)
		{
			AddVertexBinormal(targetUVSet, pMesh->GetVertexBinormal(iSet, jBin));
		}
	}

	U32 firstNewVertexIndex = GetNumPolygonVertices();
	for (U32 iPolyVert = 0; iPolyVert < pMesh->GetNumPolygonVertices(); iPolyVert++)
	{
		const C_PolygonVertex& rhsPolyVert = pMesh->GetPolygonVertex(iPolyVert);
		C_PolygonVertex newPolyVert;
		
		if (rhsPolyVert.GetPositionIndex() != -1)
		{
			newPolyVert.SetPositionIndex(positionIndexOffset + rhsPolyVert.GetPositionIndex());
		}

		if (rhsPolyVert.GetNormalIndex() != -1)
		{
			newPolyVert.SetNormalIndex(normalIndexOffset + rhsPolyVert.GetNormalIndex());
		}

		for (U32 jSet = 0; jSet < rhsPolyVert.GetNumTextureCoordinateIndices(); jSet++)
		{
			U32 targetUVSet = pRHSUVSetToLHSUVSetMapping[jSet];

			U32 uvIndexOffset = 0;
			U32 tanIndexOffset = 0;
			U32 binIndexOffset = 0;

			if (targetUVSet < numUVSetOffsets)
			{
				uvIndexOffset = uvSetOffsets[targetUVSet].uvIndexOffset;
				tanIndexOffset = uvSetOffsets[targetUVSet].tanIndexOffset;
				binIndexOffset = uvSetOffsets[targetUVSet].binIndexOffset;
			}

			//Copy uv coordinate index.
			if (rhsPolyVert.GetTextureCoordinateIndex(jSet) != -1)
				newPolyVert.SetTextureCoordinateIndex(targetUVSet, uvIndexOffset + rhsPolyVert.GetTextureCoordinateIndex(jSet));
			else
				newPolyVert.SetTextureCoordinateIndex(targetUVSet, -1);

			//Copy tangent index.
			if (rhsPolyVert.GetTangentIndex(jSet) != -1)
				newPolyVert.SetTangentIndex(targetUVSet, tanIndexOffset + rhsPolyVert.GetTangentIndex(jSet));
			else
				newPolyVert.SetTangentIndex(targetUVSet, -1);

			//Copy binormal index.
			if (rhsPolyVert.GetBinormalIndex(jSet) != -1)
				newPolyVert.SetBinormalIndex(targetUVSet, binIndexOffset + rhsPolyVert.GetBinormalIndex(jSet));
			else
				newPolyVert.SetBinormalIndex(targetUVSet, -1);
		}

		AFTK_ASSERT(newPolyVert.GetPositionIndex() != -1);
		AddPolygonVertex(newPolyVert);
	}

	//Process vertex channel data.
	{
		//Add and pad channel data as required.
		for (U32 iTargetChannel = GetNumVertexDataChannels(); 
			iTargetChannel < (U32)vertexChannelMergeSources.Size(); 
			iTargetChannel++)
		{
			//Set vertex channel descriptions for channels we don't yet have.
			U32 sourceChannel = vertexChannelMergeSources[iTargetChannel];
			const C_VertexDataChannelInfo& info = pMesh->GetVertexDataChannelInfo(sourceChannel);
			SetVertexDataChannelInfo(iTargetChannel, info);
		}

		U32 lhsIndex = firstNewVertexIndex;
		for (U32 iPolyVert = 0; iPolyVert < pMesh->GetNumPolygonVertices(); iPolyVert++, lhsIndex++)
		{
			const C_PolygonVertex& rhsPolyVert = pMesh->GetPolygonVertex(iPolyVert);
			C_PolygonVertex& newPolyVert = GetPolygonVertex(lhsIndex);
			
			//Copy data channels across for pv.
			for (U32 iTargetChannel = 0; iTargetChannel < (U32)vertexChannelMergeSources.Size(); iTargetChannel++)
			{
				S32 sourceChannel = vertexChannelMergeSources[iTargetChannel];
				if (sourceChannel == -1)
					continue;

				const C_VertexDataChannelInfo& info = GetVertexDataChannelInfo(iTargetChannel);
				newPolyVert.SetData(
					iTargetChannel,
					info.GetDataType(), 
					rhsPolyVert.GetDataPtr(sourceChannel));
			}
		}
	}

	//We don't need our uv offset info any more.
	delete[] pRHSUVSetToLHSUVSetMapping;

	//Find the new meshes skin cluster (if any)
	C_InfluenceClusterNode* pIncomingSkinCluster = 0;
	for(U32 iInfluence = 0; iInfluence < (U32)pMesh->m_influenceClusters.Size(); iInfluence++)
	{
		C_InfluenceClusterNode* pInfluenceCluster = pMesh->m_influenceClusters[iInfluence];

		//Only remap vertex weighting information.
		if (pInfluenceCluster->GetUsage() == AFTK_INFLUENCE_CLUSTER_USAGE_SKIN)
		{
			pIncomingSkinCluster = pInfluenceCluster;
		}
	}
	if (!pIncomingSkinCluster)
	{
		//No extra work to do if our incoming mesh isn't skinned.
		return C_Status::E_Status_Success;
	}

	//Find the current meshes skin cluster (if any).
	C_InfluenceClusterNode* pExistingSkinCluster = 0;
	for(U32 iInfluence = 0; iInfluence < (U32)m_influenceClusters.Size(); iInfluence++)
	{
		C_InfluenceClusterNode* pInfluenceCluster = m_influenceClusters[iInfluence];

		//Only remap vertex weighting information.
		if (pInfluenceCluster->GetUsage() == AFTK_INFLUENCE_CLUSTER_USAGE_SKIN)
		{
			pExistingSkinCluster = pInfluenceCluster;
		}
	}

	if (!pExistingSkinCluster)
	{
		//If this mesh doesn't currently have a skin cluster, then just copy the skin cluster
		//from the incoming mesh.
		C_InfluenceClusterNode* pNewSkinCluster = new C_InfluenceClusterNode(m_pScene);
		*pNewSkinCluster = *pIncomingSkinCluster;
		//Connect the new cluster to this mesh.
		pNewSkinCluster->SetSceneNodePtr(this);

		//Adjust the vertex position indices for the new cluster.
		for (U32 iEnvelope = 0; iEnvelope < pNewSkinCluster->GetNumEnvelopes(); iEnvelope++)
		{
			C_WeightEnvelope* pEnvelope = &pNewSkinCluster->GetEnvelope(iEnvelope);
			for (U32 iVtx = 0; iVtx < pEnvelope->GetNumWeights(); iVtx++)
			{
				C_Weight<F32> weight;
				pEnvelope->GetWeight(iVtx, weight);
				//old vertex position index
				U32 posIndex = weight.GetIndex();
				//new vertex position index
				U32 newPosIndex = positionIndexOffset + posIndex;
				weight.SetIndex(newPosIndex);

				//Set vertex weight
				pEnvelope->SetWeight(iVtx, weight);
			}
		}

		return C_Status::E_Status_Success;
	}

	//If we get here, then _both_ meshes have a skin cluster. These clusters must now be merged.
		
	if (mergeWeightsByEnvIndex)
	{
		AFTK_ASSERT(pIncomingSkinCluster->GetNumEnvelopes() == pExistingSkinCluster->GetNumEnvelopes());
		if (pIncomingSkinCluster->GetNumEnvelopes() != pExistingSkinCluster->GetNumEnvelopes())
		{
			return C_Status(C_Status::E_Status_Failure, AFTK_E_MERGE_WEIGHTS_BY_ENV_FAIL);
		}

		for (U32 iEnvelope = 0; iEnvelope < pIncomingSkinCluster->GetNumEnvelopes(); iEnvelope++)
		{
			C_WeightEnvelope* pIncomingEnvelope = &pIncomingSkinCluster->GetEnvelope(iEnvelope);
			C_WeightEnvelope* pExistingEnvelope = &pExistingSkinCluster->GetEnvelope(iEnvelope);

			U32 numWeights = pIncomingEnvelope->GetNumWeights();
			//Both envelopes use the same bone! We now need to add our offset weights.
			for (U32 iVtx = 0; iVtx < numWeights; iVtx++)
			{
				C_Weight<F32> weight;
				pIncomingEnvelope->GetWeight(iVtx, weight);
				//Old vertex index.
				U32 posIndex = weight.GetIndex();
				//New vertex index.
				U32 newPosIndex = positionIndexOffset + posIndex;
				weight.SetIndex(newPosIndex);

				//Add vertex weight to existing envelope.
				pExistingEnvelope->AddWeight(weight);
			}
		}
	}
	else
	{
		//For each envelope in the incoming cluster, find if there is an envelope for the same bone in the
		//existing cluster. If there is, then add to that envelope. If not, then copy in the new envelope.

		for (U32 iEnvelope = 0; iEnvelope < pIncomingSkinCluster->GetNumEnvelopes(); iEnvelope++)
		{
			C_WeightEnvelope* pIncomingEnvelope = &pIncomingSkinCluster->GetEnvelope(iEnvelope);

			U32 jEnvelope;
			for (jEnvelope = 0; jEnvelope < pExistingSkinCluster->GetNumEnvelopes(); jEnvelope++)
			{
				C_WeightEnvelope* pExistingEnvelope = &pExistingSkinCluster->GetEnvelope(jEnvelope);
				if (pIncomingEnvelope->GetInfluenceNodePtr() == pExistingEnvelope->GetInfluenceNodePtr())
				{
					//Both envelopes use the same bone! We now need to add our offset weights.
					for (U32 iVtx = 0; iVtx < pIncomingEnvelope->GetNumWeights(); iVtx++)
					{
						C_Weight<F32> weight;
						pIncomingEnvelope->GetWeight(iVtx, weight);
						//Old vertex index.
						U32 posIndex = weight.GetIndex();
						//New vertex index.
						U32 newPosIndex = positionIndexOffset + posIndex;
						weight.SetIndex(newPosIndex);

						//Add vertex weight to existing envelope.
						pExistingEnvelope->AddWeight(weight);
					}
					break;
				}
			}

			if (jEnvelope == pExistingSkinCluster->GetNumEnvelopes())
			{
				//If this condition is met, then we did not find an existing envelope in which to insert
				//our vertex weights. In this case we must add a new one.
				U32 envelopeIndex = pExistingSkinCluster->AddEnvelope();
				//Get a pointer to our new envelope.
				C_WeightEnvelope* pNewEnvelope = &pExistingSkinCluster->GetEnvelope(envelopeIndex);
				
				//We now need to add our offset weights.
				for (U32 iVtx = 0; iVtx < pIncomingEnvelope->GetNumWeights(); iVtx++)
				{
					C_Weight<F32> weight;
					pIncomingEnvelope->GetWeight(iVtx, weight);
					//Old vertex index.
					U32 posIndex = weight.GetIndex();
					//New vertex index.
					U32 newPosIndex = positionIndexOffset + posIndex;
					weight.SetIndex(newPosIndex);

					//Add vertex weight to new envelope.
					pNewEnvelope->AddWeight(weight);
				}
				pNewEnvelope->SetInfluenceNodePtr(pIncomingEnvelope->GetInfluenceNodePtr());
			}
		}
	}

	return C_Status::E_Status_Success;
}

C_Status C_SceneMeshNode::AddMeshPolygonData(
	const C_SceneMeshNode* pMesh, 
	U32 targetLodLevel)
{
	//We cannot add a mesh to itself.
	if (pMesh == this)
		return C_Status::E_Status_Failure;

	U32 polygonVertexIndexOffset = GetNumPolygonVertices();

	//Add polygons to our new level of detail.
	Ut::Vector<U16> materialMappingTable;
	materialMappingTable.Resize(pMesh->GetNumPolygonMaterialMappings());
	Ut::Vector<U16> blindDataMappingTable;
	blindDataMappingTable.Resize(pMesh->GetNumPolygonBlindDataMappings());

	Ut::Vector<S32> oldToNewPolygonMappingTable;
	oldToNewPolygonMappingTable.Resize(pMesh->GetNumPolygons(targetLodLevel));
	if (oldToNewPolygonMappingTable.Size() > 0)
		memset(&oldToNewPolygonMappingTable[0], -1, sizeof(S32)*oldToNewPolygonMappingTable.Size());
	
	Ut::UtMap<const C_PolygonCluster*, U32> clusterMap;

	for (U32 iPoly = 0; iPoly < pMesh->GetNumPolygons(targetLodLevel); iPoly++)
	{
		const C_Polygon* pExistingPoly = &pMesh->GetPolygon(targetLodLevel, iPoly);
		S16 existingMaterialIndex = pExistingPoly->GetMaterialMappingIndex();
		S16 existingBlindDataIndex = pExistingPoly->GetBlindDataMappingIndex();
		
		if (existingMaterialIndex != -1)
		{
			S16 newMaterialIndex = AddPolygonMaterialMapping(
				pMesh->GetPolygonMaterialMappingPtr(existingMaterialIndex));
			materialMappingTable[existingMaterialIndex] = newMaterialIndex;
		}
		if (existingBlindDataIndex != -1)
		{
			S16 newBlindDataIndex = AddPolygonBlindDataMapping(
				pMesh->GetPolygonBlindDataMappingPtr(existingBlindDataIndex));
			blindDataMappingTable[existingBlindDataIndex] = newBlindDataIndex;
		}
	}

	//We need to copy polygon cluster information.
	for (U32 iPolyCluster = 0; iPolyCluster < pMesh->GetNumPolygonClusters(); iPolyCluster++)
	{
		const C_PolygonCluster* pRhsPolyCluster = pMesh->GetPolygonClusterPtr(iPolyCluster);
		bool found = false;
		for (U32 jPolyCluster = 0; jPolyCluster < GetNumPolygonClusters(); jPolyCluster++)
		{
			C_PolygonCluster* pLhsPolyCluster = GetPolygonClusterPtr(jPolyCluster);
			if (pLhsPolyCluster->GetName() == pRhsPolyCluster->GetName())
			{
				found = true;
				clusterMap[pRhsPolyCluster] = (U32)jPolyCluster;
			}
		}
		if (!found)
		{
			//We don't currently have this poly cluster from the incoming mesh - add it.
			S32 newClusterIndex = AddPolygonCluster();
			if (newClusterIndex == -1)
			{
				return C_Status(C_Status::E_Status_Failure, AFTK_E_CREATE_POLY_CLUSTER_FAIL);
			}
			clusterMap[pRhsPolyCluster] = (U32)newClusterIndex;

			C_PolygonCluster* pNewPolygonCluster = GetPolygonClusterPtr(newClusterIndex);
			pNewPolygonCluster->SetName(pRhsPolyCluster->GetName());
		}
	}

	//We're merging the first level of detail from the given mesh in to the specified level of detail.
	for (U32 iPoly = 0; iPoly < pMesh->GetNumPolygons(targetLodLevel); iPoly++)
	{
		const C_Polygon* pExistingPoly = &pMesh->GetPolygon(0, iPoly);
		S16 existingMaterialIndex = pExistingPoly->GetMaterialMappingIndex();
		S16 existingBlindDataIndex = pExistingPoly->GetBlindDataMappingIndex();

		U32 targetCluster = clusterMap[pExistingPoly->GetClusterPtr()];
		
		S32 newPolyIndex = AddPolygon(targetLodLevel, targetCluster, C_Polygon());
		oldToNewPolygonMappingTable[iPoly] = newPolyIndex;
		C_Polygon* pNewPoly = &GetPolygon(targetLodLevel, newPolyIndex);

		//Copy polygon vertex indices.
		for (U32 iPolyVert = 0; iPolyVert < pExistingPoly->GetNumPolygonVertices(); iPolyVert++)
		{
			pNewPoly->AddPolygonVertexIndex(
				polygonVertexIndexOffset + pExistingPoly->GetPolygonVertexIndex(iPolyVert));
		}

		if (existingMaterialIndex != -1)
		{
			S16 newMaterialIndex = materialMappingTable[existingMaterialIndex];
			pNewPoly->SetMaterialMappingIndex(newMaterialIndex);
		}
		if (existingBlindDataIndex != -1)
		{
			S16 newBlindDataIndex = blindDataMappingTable[existingBlindDataIndex];
			pNewPoly->SetBlindDataMappingIndex(newBlindDataIndex);
		}
	}

	//Remap polygon base indices.
	for (U32 iPoly = 0; iPoly < (U32)oldToNewPolygonMappingTable.Size(); iPoly++)
	{
		AFTK_ASSERT(oldToNewPolygonMappingTable[iPoly] != -1);
		const C_Polygon* pOldPoly = &pMesh->GetPolygon(targetLodLevel, iPoly);
		C_Polygon* pNewPoly = &GetPolygon(targetLodLevel, oldToNewPolygonMappingTable[iPoly]);
		if (pOldPoly->GetBasePolygonIndex() == -1)
			pNewPoly->SetBasePolygonIndex(-1);
		else
			pNewPoly->SetBasePolygonIndex(
				oldToNewPolygonMappingTable[pOldPoly->GetBasePolygonIndex()]);
	}

	return C_Status::E_Status_Success;
}

void C_SceneMeshNode::SetLodLevelInfo(U32 lodLevel, const C_MeshLodLevelInfo& info)
{
	DirtyGeometryDependantCaches();
	AFTK_RESIZE_LOD_LEVELS_IF_NEEDED;
	m_lodLevels[lodLevel].SetInfo(info);
}

void C_SceneMeshNode::InsertLodLevelInfo(U32 lodLevel, const C_MeshLodLevelInfo& info)
{
	DirtyGeometryDependantCaches();
	C_LodLevel lodLevelStructure;
	lodLevelStructure.SetInfo(info);
	m_lodLevels.InsertBefore(lodLevelStructure, m_lodLevels.Begin()+lodLevel);

	//Shuffle polygon cluster Lod indices up by one where needed.
	for (U32 iPolyCluster = 0; iPolyCluster < (U32)m_polygonClusters.Size(); iPolyCluster++)
	{
		C_PolygonCluster* pPolyCluster = m_polygonClusters[iPolyCluster];
		AFTK_ASSERT(pPolyCluster);
		for (U32 iLodLevel = 0; iLodLevel < pPolyCluster->GetNumLodLevels(); iLodLevel++)
		{
			for (U32 jPoly = 0; jPoly < pPolyCluster->GetNumPolygonIndices(iLodLevel); jPoly++)
			{
				U32 polygonIndex = pPolyCluster->GetPolygonIndex(iLodLevel, jPoly);
				//If the existing lod level index is equal to or greater than our insertion point,
				//then shuffle up the lod level index fot this poly.
				if (iLodLevel >= lodLevel)
				{
					pPolyCluster->DeletePolygonIndex(iLodLevel, polygonIndex);
					pPolyCluster->AddPolygonIndex(iLodLevel+1, polygonIndex);
				}
			}
		}
	}
}

C_Status C_SceneMeshNode::SetLodLevelMesh(
	U32 lodLevel, 
	const C_SceneMeshNode* pMeshLodLevel,
	bool mergeWeightsByEnvIndex,
	bool mergeVertexChannelsByName)
{
	DirtyGeometryDependantCaches();
	C_Status status;

	if (pMeshLodLevel == this)
	{
		AFTK_ASSERT(pMeshLodLevel != this);
		return C_Status(C_Status::E_Status_Failure, "An attempt was made to assign a mesh as a lod level of itself.");
	}

	//Define rhs -> lhs vertex channel targeting.
	Ut::Vector<S32> vertexChannelMergeSources;
	status = BuildVertexChannelMergeTable(
		vertexChannelMergeSources,
		pMeshLodLevel,
		mergeVertexChannelsByName);
	if (!status) return status;

	AFTK_RESIZE_LOD_LEVELS_IF_NEEDED;

	m_lodLevels[lodLevel].Clear();

	//Note that all of the indexed data from the incoming mesh must be offset by the current ammount
	//of mesh data.

	status = AddMeshPolygonData(pMeshLodLevel, lodLevel);
	if (!status) return status;

	status = AddMeshVertexData(
		pMeshLodLevel, 
		mergeWeightsByEnvIndex,
		vertexChannelMergeSources);
	if (!status) return status;

	return C_Status::E_Status_Success;
}

S32 C_SceneMeshNode::AddPolygon(const Aftk::C_Polygon& polygon)
{
	DirtyGeometryDependantCaches();
	return AddPolygon(0,0, polygon);
}

void C_SceneMeshNode::DeletePolygon(U32 lodLevel, U32 index)
{
	C_Polygon& polyToDelete = m_lodLevels[lodLevel].GetPolygon(index);
	//Remove poly from cluster.
	polyToDelete.GetClusterPtr()->DeletePolygonIndex(lodLevel, index, true);
	//Remove poly from lod level poly array.
	m_lodLevels[lodLevel].DeletePolygon(index);

	//Shuffle indices to cope with deletion.
	for (U32 jCluster = 0; jCluster < (U32)m_polygonClusters.Size(); jCluster++)
		m_polygonClusters[jCluster]->ShufflePolygonIndices(lodLevel, index);

	//Compensate base indices.
	for (U32 kPoly = 0; kPoly < GetNumPolygons(lodLevel); kPoly++)
	{
		Aftk::C_Polygon& poly = GetPolygon(lodLevel, kPoly);
		S32 basePolyIndex = poly.GetBasePolygonIndex();
		if (basePolyIndex == index)
		{
			//This poly has been removed.
			poly.SetBasePolygonIndex(-1);
		}
		else if (basePolyIndex > (S32)index)
		{
			basePolyIndex--;
			poly.SetBasePolygonIndex(basePolyIndex);
		}
	}
}

S32 C_SceneMeshNode::AddPolygon(U32 lodLevel, U32 cluster, const Aftk::C_Polygon& polygon)
{
	DirtyGeometryDependantCaches();
	S32 newPolyIndex = AddPolygonNoCluster(lodLevel, polygon);
	if (newPolyIndex == -1)
	{
		return -1;
	}

	//Make this polygon a member of the correct cluster.
	m_polygonClusters[cluster]->AddPolygonIndex(lodLevel, newPolyIndex);

	return (S32)newPolyIndex;
}

// Sets
void C_SceneMeshNode::SetVertexPosition(U32 instance, const Position3& position)
{
	DirtyGeometryDependantCaches();
	m_vertexPositions[instance] = position;
}

void C_SceneMeshNode::SetVertexNormal(U32 instance, const Normal3& vertexNormal)
{
	DirtyGeometryDependantCaches();
	m_vertexNormals[instance] = vertexNormal;
}

void C_SceneMeshNode::SetTextureCoordinate(U32 set, U32 instance, const TextureCoordinate4& uvwx)
{
	DirtyGeometryDependantCaches();
	m_textureCoordinateSets[set].SetTextureCoordinate(instance, uvwx);
}

void C_SceneMeshNode::SetVertexTangent(U32 set, U32 instance, const Vector3F32& tangent)
{
	DirtyGeometryDependantCaches();
	m_textureCoordinateSets[set].SetTangent(instance, tangent);
}

void C_SceneMeshNode::SetVertexBinormal(U32 set, U32 instance, const Vector3F32& binormal)
{
	DirtyGeometryDependantCaches();
	m_textureCoordinateSets[set].SetBinormal(instance, binormal);
}

void C_SceneMeshNode::SetPolygonVertex(U32 instance, const C_PolygonVertex& polyVert)
{
	DirtyGeometryDependantCaches();
	m_polygonVertices[instance] = polyVert;
	C_PolygonVertex* pPolyVert = &m_polygonVertices[m_polygonVertices.Size()-1];
	
	if (pPolyVert->GetNumTextureCoordinateIndices() < GetNumTextureCoordinateSets())
	{
		PadPolygonVertexTextureCoordinateSetIndices(pPolyVert);
	}
	if (pPolyVert->GetNumData() < GetNumVertexDataChannels())
	{
		PadPolygonVertexDataChannels(pPolyVert);
	}
}

void C_SceneMeshNode::DirtyGeometryDependantCaches() const
{
	C_SceneGeometryNode::DirtyGeometryDependantCaches();
	ClearAnimationHelpers();
}

void C_SceneMeshNode::SetPolygon(U32 instance, const Aftk::C_Polygon& polygon)
{
	DirtyGeometryDependantCaches();
	m_lodLevels[0].SetPolygon(instance, polygon);
	m_lodLevels[0].GetPolygon(instance).AttachToMesh(this);
}

void C_SceneMeshNode::SetPolygon(U32 lodLevel, U32 instance, const Aftk::C_Polygon& polygon)
{
	DirtyGeometryDependantCaches();
	m_lodLevels[lodLevel].SetPolygon(instance, polygon);
	m_lodLevels[lodLevel].GetPolygon(instance).AttachToMesh(this);
}

const Position3& C_SceneMeshNode::GetVertexPosition(U32 instance) const
{
	return m_vertexPositions[instance];
}

Position3& C_SceneMeshNode::GetVertexPosition(U32 posIndex)
{
	return m_vertexPositions[posIndex];
}

const Normal3& C_SceneMeshNode::GetVertexNormal(U32 normIndex) const
{
	return m_vertexNormals[normIndex];
}

Normal3& C_SceneMeshNode::GetVertexNormal(U32 normIndex)
{
	return m_vertexNormals[normIndex];
}
const TextureCoordinate4& C_SceneMeshNode::GetTextureCoordinate(U32 set, U32 instance) const
{
	return m_textureCoordinateSets[set].GetTextureCoordinate(instance);
}

const Vector3F32& C_SceneMeshNode::GetVertexTangent(U32 set, U32 instance) const
{
	return m_textureCoordinateSets[set].GetTangent(instance);
}

const Vector3F32& C_SceneMeshNode::GetVertexBinormal(U32 set, U32 instance) const
{
	return m_textureCoordinateSets[set].GetBinormal(instance);
}

TextureCoordinate4& C_SceneMeshNode::GetTextureCoordinate(U32 set, U32 instance)
{
	return m_textureCoordinateSets[set].GetTextureCoordinate(instance);
}

Vector3F32& C_SceneMeshNode::GetVertexTangent(U32 set, U32 instance)
{
	return m_textureCoordinateSets[set].GetTangent(instance);
}

Vector3F32& C_SceneMeshNode::GetVertexBinormal(U32 set, U32 instance)
{
	return m_textureCoordinateSets[set].GetBinormal(instance);
}

S32	C_SceneMeshNode::GetTextureCoordinateSetIndex(const std::string& name) const
{
	for (U32 i = 0; i < GetNumTextureCoordinateSets(); i++)
	{
		if (m_textureCoordinateSets[i].GetName() == name)
		{
			return i;
		}
	}

	return -1;
}

const std::string& C_SceneMeshNode::GetTextureCoordinateSetName(U32 set) const
{
	return m_textureCoordinateSets[set].GetName();
}

const C_PolygonVertex& C_SceneMeshNode::GetPolygonVertex(U32 instance) const
{
	return m_polygonVertices[instance];
}

C_PolygonVertex& C_SceneMeshNode::GetPolygonVertex(U32 instance)
{
	return m_polygonVertices[instance];
}

const C_VertexDataChannelInfo& C_SceneMeshNode::GetVertexDataChannelInfo(U32 channel) const
{
	return m_vertexDataChannelInfo[channel];
}

C_VertexDataChannelInfo& C_SceneMeshNode::GetVertexDataChannelInfo(U32 channel)
{
	return m_vertexDataChannelInfo[channel];
}

const C_MeshLodLevelInfo& C_SceneMeshNode::GetLodLevelInfo(U32 lodLevel) const
{
	return m_lodLevels[lodLevel].GetInfo();
}

C_MeshLodLevelInfo& C_SceneMeshNode::GetLodLevelInfo(U32 lodLevel)
{
	return m_lodLevels[lodLevel].GetInfo();
}

U32	C_SceneMeshNode::GetNumTextureCoordinateSets() const
{
	return (U32)m_textureCoordinateSets.Size();
}

U32	C_SceneMeshNode::GetNumVertexDataChannels() const
{
	return (U32)m_vertexDataChannelInfo.Size();
}

const Aftk::C_Polygon& C_SceneMeshNode::GetPolygon(U32 instance) const
{
	return m_lodLevels[0].GetPolygon(instance);
}

Aftk::C_Polygon& C_SceneMeshNode::GetPolygon(U32 instance)
{
	return m_lodLevels[0].GetPolygon(instance);
}

const Aftk::C_Polygon& C_SceneMeshNode::GetPolygon(U32 lodLevel, U32 instance) const
{
	return m_lodLevels[lodLevel].GetPolygon(instance);
}

Aftk::C_Polygon& C_SceneMeshNode::GetPolygon(U32 lodLevel, U32 instance)
{
	return m_lodLevels[lodLevel].GetPolygon(instance);
}

S32 C_SceneMeshNode::GetPolygonClusterIndex(const C_PolygonCluster* pPolygonCluster) const
{
	for (U32 i = 0; i < (U32)m_polygonClusters.Size(); i++)
	{
		if (pPolygonCluster == m_polygonClusters[i])
		{
			return i;
		}
	}

	//We didn't find the poly cluster.
	return -1;
}

C_PolygonCluster* C_SceneMeshNode::GetPolygonClusterPtr(U32 instance)
{
	return m_polygonClusters[instance];
}

const C_PolygonCluster* C_SceneMeshNode::GetPolygonClusterPtr(U32 instance) const
{
	return m_polygonClusters[instance];
}

U32	C_SceneMeshNode::GetNumVertexPositions() const
{
	return (U32)m_vertexPositions.Size();
}

U32 C_SceneMeshNode::GetNumVertexNormals() const
{
	return (U32)m_vertexNormals.Size();
}

U32 C_SceneMeshNode::GetNumVertexTangents(U32 set) const
{
	AFTK_ASSERT(set < (U32)m_textureCoordinateSets.Size());
	return (U32)m_textureCoordinateSets[set].GetNumTangents();
}

U32 C_SceneMeshNode::GetNumVertexBinormals(U32 set) const
{
	AFTK_ASSERT(set < (U32)m_textureCoordinateSets.Size());
	return (U32)m_textureCoordinateSets[set].GetNumBinormals();
}

U32 C_SceneMeshNode::GetNumVertexData() const
{
	U32 count = 0;
	for (U32 i = 0; i < (U32)m_polygonVertices.Size(); i++)
	{
		const C_PolygonVertex* pVert = &m_polygonVertices[i];
		count += pVert->GetNumData();
	}
	return count;
}

U32 C_SceneMeshNode::GetNumTextureCoordinates(U32 set) const
{
	return m_textureCoordinateSets[set].GetNumTextureCoordinates();
}

U32 C_SceneMeshNode::GetNumPolygonVertices() const
{
	return (U32)m_polygonVertices.Size();
}

U32 C_SceneMeshNode::GetNumLodLevels() const
{
	return (U32)m_lodLevels.Size();
}

U32 C_SceneMeshNode::GetNumPolygons(U32 lodLevel) const
{
	if (lodLevel >= (U32)m_lodLevels.Size())
	{
		return 0;
	}
	return (U32)m_lodLevels[lodLevel].GetNumPolygons();
}

U32 C_SceneMeshNode::GetNumMaterialNodes(U32 lodLevel) const
{
	// Need to loop through the array of materials and store the unique resourceIDs encountered.
	// The number of unique resources is the number of material nodes used by this mesh.

	// Note: I suspect that this is very, very inefficient. A _much_ better solution would be to
	// replace the stl set with a hashset.

	Ut::UtSet< const C_SharedResource<C_MaterialNode>* > uniqueResourceIDs;

	for (U32 iLod = 0; iLod < (U32)m_lodLevels.Size(); iLod++)
	{
		for (U32 jPoly = 0; jPoly < (U32)m_lodLevels[lodLevel].GetNumPolygons(); jPoly++)
		{
			const C_Polygon* pPolygon = &m_lodLevels[lodLevel].GetPolygon(jPoly);
			const C_SharedResource<C_MaterialNode>* pMaterialResource = 
				GetPolygonMaterialMappingPtr(pPolygon->GetMaterialMappingIndex());

			if (pMaterialResource != 0)
			{
				uniqueResourceIDs.Insert(pMaterialResource);
			}
		}
	}

	return (U32)uniqueResourceIDs.Size();
}

U32 C_SceneMeshNode::GetNumPolygonClusters() const
{
	return m_polygonClusters.Size();
}

C_Status C_SceneMeshNode::DeletePolygonCluster(U32 instance)
{
	if (instance == 0)
		return C_Status(C_Status::E_Status_Failure, "Cannot delete default cluster.");

	//Remove polygons from the cluster to be deleted.
	for (U32 iLodLevel = 0; iLodLevel < m_polygonClusters[instance]->GetNumLodLevels(); iLodLevel++)
	{
		for (U32 jPoly = 0; jPoly < m_polygonClusters[instance]->GetNumPolygonIndices(iLodLevel); jPoly++)
		{
			U32 polygonIndex = 
				m_polygonClusters[instance]->GetPolygonIndex(iLodLevel, jPoly);
			m_polygonClusters[0]->AddPolygonIndex(iLodLevel, polygonIndex);
		}
	}

	delete m_polygonClusters[instance];
	m_polygonClusters.EraseByIndex(instance);

	return C_Status::E_Status_Success;
}

C_Status C_SceneMeshNode::DeletePolygonCluster(C_PolygonCluster* pPolygonCluster)
{
	for (U32 i = 0; i < (U32)m_polygonClusters.Size(); i++)
	{
		if (m_polygonClusters[i] == pPolygonCluster)
		{
			if (i == 0)
			{
				return C_Status(C_Status::E_Status_Failure, "Cannot delete default cluster.");
			}

			for (U32 jLodLevel = 0; jLodLevel < m_polygonClusters[i]->GetNumLodLevels(); jLodLevel++)
			{
				//Remove polygons from the cluster to be deleted.
				for (U32 kPoly = 0; kPoly < m_polygonClusters[i]->GetNumPolygonIndices(jLodLevel); kPoly++)
				{
					U32 polyIndex = 
						m_polygonClusters[i]->GetPolygonIndex(jLodLevel, kPoly);
					m_polygonClusters[0]->AddPolygonIndex(jLodLevel, polyIndex);
				}
			}

			delete m_polygonClusters[i];
			m_polygonClusters.EraseByIndex(i);
			return C_Status::E_Status_Success;
		}
	}

	return C_Status(C_Status::E_Status_Failure, "Could not find cluster to delete.");
}

U32 C_SceneMeshNode::Repair()
{
	DirtyGeometryDependantCaches();

	U32 numErrors = ProcessProblems(true);
	//The paragraphs below are extracts from the Maya MFnMesh documentation.

	//Circumstances under which a mesh is altered upon creation:

	//If the vertex connection list for a polygon specifies an edge with the same start and end verticies, 
	//those edges will be removed. (ie. a polygon specified as 1 2 2 3 4 will be transformed into 1 2 3 4).

	//If the same vertex appears in the vertex connection list for a polygon more than once, another vertex
	//will be created at the same location, and the list will be altered to use the copy. (ie. a polygon 
	//specified as 1 2 3 1 4 will be transformed into 1 2 3 5 4, where 5 is in the same location as 1).

	if (m_lodLevels.Size() == 0)
	{
		if ((m_vertexPositions.Size() + 
			m_vertexNormals.Size() + 
			m_textureCoordinateSets.Size() + 
			m_vertexDataChannelInfo.Size()) > 0)
		{
#ifdef _DEBUG
			AFTK_DO_BEGIN << "Deleting contents of mesh with no polys " << m_name << "\n" AFTK_DO_DEBUG_END;
#endif
			m_vertexPositions.Clear();
			m_vertexNormals.Clear();
			m_textureCoordinateSets.Clear();
			m_vertexDataChannelInfo.Clear();
		}
	}

	if (m_vertexPositions.Size() == 0)
	{
		if ((m_lodLevels.Size() + 
			m_vertexNormals.Size() + 
			m_textureCoordinateSets.Size() + 
			m_vertexDataChannelInfo.Size()) > 0)
		{
#ifdef _DEBUG
			AFTK_DO_BEGIN << "Deleting contents of mesh with no vertex positions " << m_name << "\n" AFTK_DO_DEBUG_END;
#endif
			m_lodLevels.Clear();
			m_vertexNormals.Clear();
			m_textureCoordinateSets.Clear();
			m_vertexDataChannelInfo.Clear();
		}
	}

	for (U32 iLodLevel = 0; iLodLevel < (U32)m_lodLevels.Size(); iLodLevel++)
	{
		//Check for case 1
		for (U32 jPoly = 0; jPoly < (U32)m_lodLevels[iLodLevel].GetNumPolygons(); jPoly++)
		{
			C_Polygon* pPoly = &m_lodLevels[iLodLevel].GetPolygon(jPoly);
			//The value of -2 is never used for anything
			S32 vertexPositionIndex = -2;
			U32 numPolygonVertices = (U32)pPoly->GetNumPolygonVertices();
			for (U32 kPolyVert = 0; kPolyVert < numPolygonVertices; kPolyVert++)
			{
				C_PolygonVertex* pPolygonVertex = &pPoly->GetPolygonVertex(kPolyVert);
				AFTK_ASSERT(pPolygonVertex->GetPositionIndex() != -2);
				if (pPolygonVertex->GetPositionIndex() == vertexPositionIndex)
				{
					//This position index is the same as the previous, remove this edge.
					pPoly->DeletePolygonVertexIndex(kPolyVert);

					//Now we've one less polygon vertex to check
					numPolygonVertices--;
				}
				else
				{
					vertexPositionIndex = pPolygonVertex->GetPositionIndex();
				}
			}
		}
	}

	for (U32 iLodLevel = 0; iLodLevel < (U32)m_lodLevels.Size(); iLodLevel++)
	{
		//Check for case 2
		for (U32 jPoly = 0; jPoly < (U32)m_lodLevels[iLodLevel].GetNumPolygons(); jPoly++)
		{
			C_Polygon* pPoly = &m_lodLevels[iLodLevel].GetPolygon(jPoly);

			Ut::UtSet<S32> uniqueVertexPositionIndices;

			U32 numPolygonVertices = (U32)pPoly->GetNumPolygonVertices();
			for (U32 kPolyVert = 0; kPolyVert < numPolygonVertices; kPolyVert++)
			{
				C_PolygonVertex* pPolygonVertex = &pPoly->GetPolygonVertex(kPolyVert);
				S32 positionIndex = pPolygonVertex->GetPositionIndex();

				Ut::UtSet<S32>::Iterator it;
				it = uniqueVertexPositionIndices.Find(positionIndex);
				if (it != uniqueVertexPositionIndices.End())
				{
					//We've got a vertex position used more than once here. Create a new vertex position and
					//index this instead
					S32 newVertexPositionIndex = AddVertexPosition(
						Position3(GetVertexPosition(pPolygonVertex->GetPositionIndex())));

					pPolygonVertex->SetPositionIndex(newVertexPositionIndex);
				}
				//Add to our set of unique position indices
				uniqueVertexPositionIndices.Insert(pPolygonVertex->GetPositionIndex());
			}
		}
	}

	RepairPolyClusters();

	RepairPolygonVertices();

	bool needsTriangulation = false;
	for (U32 iLodLevel = 0; iLodLevel < GetNumLodLevels(); iLodLevel++)
	{
		for (U32 jPoly = 0; jPoly < GetNumPolygons(iLodLevel); jPoly++)
		{
			const C_Polygon* pPoly = &GetPolygon(iLodLevel, jPoly);

			if (pPoly->GetNumPolygonVertices() > 256)
				needsTriangulation = true;
		}
	}
	if (needsTriangulation)
		C_MeshTriangulate::Triangulate(this);

	RepairVertexNormals();
	RepairTextureCoordinates();
	RepairVertexTangents();
	RepairVertexTangents();
	RepairVertexPositions();

	//Pad polygon vertices.
	for (U32 iLodLevel = 0; iLodLevel < GetNumLodLevels(); iLodLevel++)
	{
		for (U32 jPoly = 0; jPoly < GetNumPolygons(iLodLevel); jPoly++)
		{
			C_Polygon* pPoly = &GetPolygon(iLodLevel, jPoly);
			if (pPoly->GetNumPolygonVertices() < 3)
			{
				S32 numNewPVsNeeded = 3 - pPoly->GetNumPolygonVertices();
				for (S32 kNewPV = 0; kNewPV < numNewPVsNeeded; kNewPV++)
				{
					U32 pv0Index;
					if (numNewPVsNeeded == 3)
					{
						//Add dummy pv.
						pv0Index = AddPolygonVertex();
						C_PolygonVertex* pPV = &GetPolygonVertex(pv0Index);
						U32 pv0PosIndex = AddVertexPosition(Position3(0,0,0));
						U32 pv0NormIndex = AddVertexNormal((Normal3&)Vector3F32(0,0,0));
						pPV->SetPositionIndex(pv0PosIndex);
						pPV->SetNormalIndex(pv0NormIndex);
					}
					else
						pv0Index = pPoly->GetPolygonVertexIndex(0);

					pPoly->AddPolygonVertexIndex(pv0Index);
				}
			}
		}
	}

	//Ensure that no adjacent polygon vertex indices are the same. This would break the triangulation
	//routine.
	for (U32 iLodLevel = 0; iLodLevel < (U32)m_lodLevels.Size(); iLodLevel++)
	{
		for (S32 jPoly = 0; jPoly < (S32)m_lodLevels[iLodLevel].GetNumPolygons(); jPoly++)
		{
			C_Polygon* pPoly = &m_lodLevels[iLodLevel].GetPolygon(jPoly);

			if (pPoly->GetNumPolygonVertices() >= 3)
			{
				for (U32 kPolyVert = 0; kPolyVert < (U32)pPoly->GetNumPolygonVertices(); kPolyVert++)
				{
					S32 pvIdx = pPoly->GetPolygonVertexIndex(kPolyVert);
					if (pvIdx == -1)
						continue;

					S32 nextPvIdx;
					U32 nextIndex;
					if (kPolyVert < pPoly->GetNumPolygonVertices()-1)
					{
						nextIndex = kPolyVert+1;
						nextPvIdx = pPoly->GetPolygonVertexIndex(nextIndex);
					}
					else
					{
						nextIndex = 0;
						nextPvIdx = pPoly->GetPolygonVertexIndex(nextIndex);
					}
					
					if (pvIdx == nextPvIdx)
					{
						pPoly->DeletePolygonVertexIndex(nextIndex);
					}
				}
			}
			if (pPoly->GetNumPolygonVertices() < 3)
			{
				//Remove degenerate polygon.
				DeletePolygon(iLodLevel, jPoly);
				//Compensate for the fact that we've just removed a polygon.
				jPoly--;
			}
		}
	}

	return numErrors;
}

U32 C_SceneMeshNode::RepairPolygonVertices()
{
	DirtyGeometryDependantCaches();

	U32 numRepairs = 0;

	//////////////////////////////////////////////////////////////////
	//1) Create shuffle buffers for unused component clean-up.
	//////////////////////////////////////////////////////////////////

	//Create the shuffle buffer for removing unused polygon vertices
	Ut::UtVector< Ut::DynamicVector<S32> > polygonVertexShuffleBuffer;
	polygonVertexShuffleBuffer.Resize(m_polygonVertices.Size());
	for (U32 i = 0; i < (U32)polygonVertexShuffleBuffer.Size(); i++)
	{
		polygonVertexShuffleBuffer[i] = -2;
	}
	
	//////////////////////////////////////////////////////////////////
	//2) Create storage for our new component arrays.
	//////////////////////////////////////////////////////////////////

	//Create storage for our new polygon vertex array
	Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> > newPolygonVertices;
	newPolygonVertices.Reserve(m_polygonVertices.Size());

	//////////////////////////////////////////////////////////////////
	//3) Generate component usage information and build new component
	//   arrays.
	//////////////////////////////////////////////////////////////////

	//Get polygon vertex usage
	for (U32 iLodLevel = 0; iLodLevel < GetNumLodLevels(); iLodLevel++)
	{
		for (U32 jPoly = 0; jPoly < (U32)m_lodLevels[iLodLevel].GetNumPolygons(); jPoly++)
		{
			C_Polygon* pPoly = &m_lodLevels[iLodLevel].GetPolygon(jPoly);

			U32 numPolygonVertices = (U32)pPoly->GetNumPolygonVertices();
			for (U32 kPolyVert = 0; kPolyVert < numPolygonVertices; kPolyVert++)
			{
				U32 pvIndex = pPoly->GetPolygonVertexIndex(kPolyVert);
				//Flag this as being used. -1 indicates used -2 indicates unused.
				polygonVertexShuffleBuffer[pvIndex] = -1;
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	//4) Copy mesh components that are flagged as being used
	//////////////////////////////////////////////////////////////////
	{
		for (U32 pvIndex = 0 ; pvIndex < (U32)m_polygonVertices.Size(); pvIndex++)
		{
			if (polygonVertexShuffleBuffer[pvIndex] == -1)
			{
				//Copy this vertex position into our new polygon vertex array
				newPolygonVertices.PushBack(m_polygonVertices[pvIndex]);
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	//5) Generate the shuffle offset information.
	//////////////////////////////////////////////////////////////////

	//Generate polygon vertex shuffle information.
	{
		U32 polygonVertexShuffleAccumulator = 0;
		for (U32 pvIndex = 0; pvIndex < (U32)m_polygonVertices.Size(); pvIndex++)
		{
			//-2 indicates that this vertex position is unused.
			if (polygonVertexShuffleBuffer[pvIndex] == -2)
			{
				polygonVertexShuffleAccumulator++;
				numRepairs++;
			}
			//Insert our accumulated shuffle value.
			polygonVertexShuffleBuffer[pvIndex] = polygonVertexShuffleAccumulator;
		}
	}

	if (numRepairs == 0)
		return 0;

	//////////////////////////////////////////////////////////////////
	//6) Remap our component indices.
	//////////////////////////////////////////////////////////////////

	//Remap component indices using our shuffle buffer.
	for (U32 iLodLevel = 0; iLodLevel < GetNumLodLevels(); iLodLevel++)
	{
		for (U32 iPoly = 0; iPoly < (U32)m_lodLevels[iLodLevel].GetNumPolygons(); iPoly++)
		{
			C_Polygon* pPoly = &m_lodLevels[iLodLevel].GetPolygon(iPoly);
			
			//Remap polygon vertices
			for (U32 jPolyVert = 0; jPolyVert < pPoly->GetNumPolygonVertices(); jPolyVert++)
			{
				S32 pvIndexOld = pPoly->GetPolygonVertexIndex(jPolyVert);
				//Ignore unasigned vertex positions.
				if (pvIndexOld != -1)
				{
					S32 polygonVertexShuffleAccumulator = polygonVertexShuffleBuffer[pvIndexOld];
					S32 pvIndexNew = pvIndexOld - polygonVertexShuffleAccumulator;
					pPoly->SetPolygonVertexIndex(jPolyVert, pvIndexNew);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	//7) Overwrite the old mesh component arrays.
	//////////////////////////////////////////////////////////////////

	//Move new polygon vertices to destination.
	Ut::MoveRaw(m_polygonVertices, newPolygonVertices);

	return numRepairs;
}

U32 C_SceneMeshNode::RepairVertexPositions()
{
	DirtyGeometryDependantCaches();

	U32 numRepairs = 0;

	//////////////////////////////////////////////////////////////////
	//1) Create shuffle buffers for unused component clean-up.
	//////////////////////////////////////////////////////////////////

	//Create the shuffle buffer for removing unused vertex positions
	Ut::UtVector< Ut::DynamicVector<S32> > vertexPositionShuffleBuffer;
	vertexPositionShuffleBuffer.Resize(m_vertexPositions.Size());
	for (U32 i = 0; i < (U32)vertexPositionShuffleBuffer.Size(); i++)
	{
		vertexPositionShuffleBuffer[i] = -2;
	}

	//////////////////////////////////////////////////////////////////
	//2) Create storage for our new component arrays.
	//////////////////////////////////////////////////////////////////

	//Create storage for our new vertex position array
	Ut::UtVector< Ut::DynamicVector<Position3> > newVertexPositions;
	newVertexPositions.Reserve(m_vertexPositions.Size());

	//////////////////////////////////////////////////////////////////
	//3) Generate component usage information and build new component
	//   arrays.
	//////////////////////////////////////////////////////////////////

	//Get polygon vertex component usage
	for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
	{
		C_PolygonVertex* pPolygonVertex = &m_polygonVertices[iPolyVert];

		S32 vPos = pPolygonVertex->GetPositionIndex();
		if (vPos != -1)
		{
			if (vPos >= (S32)GetNumVertexPositions())
			{
				//Mark index as invalid.
				pPolygonVertex->SetPositionIndex(-1);
				numRepairs++;
			}
			else
			{
				//Flag this as being used. -1 indicates used -2 indicates unused.
				vertexPositionShuffleBuffer[vPos] = -1;
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	//4) Copy mesh components that are flagged as being used
	//////////////////////////////////////////////////////////////////
	{
		for (U32 vPos = 0 ; vPos < (U32)m_vertexPositions.Size(); vPos++)
		{
			if (vertexPositionShuffleBuffer[vPos] == -1)
			{
				//Copy this vertex position into our new vertex position array.
				newVertexPositions.PushBack(m_vertexPositions[vPos]);
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	//5) Generate the shuffle offset information.
	//////////////////////////////////////////////////////////////////

	//Generate vertex position shuffle information.
	{
		U32 vertexPositionShuffleAccumulator = 0;
		for (U32 iVPos = 0; iVPos < (U32)m_vertexPositions.Size(); iVPos++)
		{
			//-2 indicates that this vertex position is unused.
			if (vertexPositionShuffleBuffer[iVPos] == -2)
			{
				vertexPositionShuffleAccumulator++;
				numRepairs++;
			}
			//Insert our accumulated shuffle value.
			vertexPositionShuffleBuffer[iVPos] = vertexPositionShuffleAccumulator;
		}
	}

	if (numRepairs == 0)
		return 0;

	//////////////////////////////////////////////////////////////////
	//6) Remap our component indices.
	//////////////////////////////////////////////////////////////////

	Ut::UtVector<Ut::DynamicVector<S32> > remapping;
	remapping.Resize(vertexPositionShuffleBuffer.Size());
	memset(&remapping[0], -1, sizeof(S32) * remapping.Size());
	//Remap component indices using our shuffle buffer.
	{
		for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
		{
			C_PolygonVertex* pPolyVert = &m_polygonVertices[iPolyVert];
			
			//Remap vertex position
			{
				S32 vPosOld = pPolyVert->GetPositionIndex();

				//Ignore unasigned vertex positions.
				if (vPosOld != -1)
				{
					S32 vertexPositionShuffleAccumulator = vertexPositionShuffleBuffer[vPosOld];
					S32 vPosNew = vPosOld - vertexPositionShuffleAccumulator;
					pPolyVert->SetPositionIndex(vPosNew);
					//Store remapping entry.
					remapping[vPosOld] = vPosNew;
				}
			}
		}
	}

	//Remap vertex weighting information.
	C_MeshWeld::RemapVertexWeights(
		this,
		remapping);

	//////////////////////////////////////////////////////////////////
	//7) Overwrite the old mesh component arrays.
	//////////////////////////////////////////////////////////////////

	//Move new vertex positions to destination.
	Ut::MoveRaw(m_vertexPositions, newVertexPositions);
	return numRepairs;
}

U32 C_SceneMeshNode::RepairVertexNormals()
{
	DirtyGeometryDependantCaches();
	U32 numRepairs = 0;

	//////////////////////////////////////////////////////////////////
	//1) Create shuffle buffers for unused component clean-up.
	//////////////////////////////////////////////////////////////////

	//Create the shuffle buffer for removing unused vertex normals.
	Ut::UtVector< Ut::DynamicVector<S32> > vertexNormalShuffleBuffer;
	vertexNormalShuffleBuffer.Resize(m_vertexNormals.Size());
	for (U32 i = 0; i < (U32)vertexNormalShuffleBuffer.Size(); i++)
	{
		vertexNormalShuffleBuffer[i] = -2;
	}

	//////////////////////////////////////////////////////////////////
	//2) Create storage for our new component arrays.
	//////////////////////////////////////////////////////////////////

	//Create storage for our new vertex position array
	Ut::UtVector< Ut::DynamicVector<Normal3> > newVertexNormals;
	newVertexNormals.Reserve(m_vertexNormals.Size());

	//////////////////////////////////////////////////////////////////
	//3) Generate component usage information and build new component
	//   arrays.
	//////////////////////////////////////////////////////////////////

	//Get polygon vertex component usage
	for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
	{
		C_PolygonVertex* pPolygonVertex = &m_polygonVertices[iPolyVert];

		S32 vNorm = pPolygonVertex->GetNormalIndex();
		if (vNorm != -1)
		{
			if (vNorm >= (S32)GetNumVertexNormals())
			{
				//Mark index as invalid.
				pPolygonVertex->SetPositionIndex(-1);
				numRepairs++;
			}
			else
			{
				//Flag this as being used. -1 indicates used -2 indicates unused.
				vertexNormalShuffleBuffer[vNorm] = -1;
			}
		}
	}
	//////////////////////////////////////////////////////////////////
	//4) Copy mesh components that are flagged as being used
	//////////////////////////////////////////////////////////////////
	{
		for (U32 norm = 0 ; norm < (U32)m_vertexNormals.Size(); norm++)
		{
			if (vertexNormalShuffleBuffer[norm] == -1)
			{
				//Copy this vertex position into our new vertex normal array.
				newVertexNormals.PushBack(m_vertexNormals[norm]);
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	//5) Generate the shuffle offset information.
	//////////////////////////////////////////////////////////////////

	//Generate vertex normal shuffle information.
	{
		U32 vertexNormalShuffleAccumulator = 0;
		for (U32 iNorm = 0; iNorm < (U32)m_vertexNormals.Size(); iNorm++)
		{
			//-2 indicates that this vertex normal is unused.
			if (vertexNormalShuffleBuffer[iNorm] == -2)
			{
				vertexNormalShuffleAccumulator++;
				numRepairs++;
			}
			//Insert our accumulated shuffle value.
			vertexNormalShuffleBuffer[iNorm] = vertexNormalShuffleAccumulator;
		}
	}

	if (numRepairs == 0)
		return 0;

	//////////////////////////////////////////////////////////////////
	//6) Remap our component indices.
	//////////////////////////////////////////////////////////////////

	//Remap component indices using our shuffle buffer.
	{
		for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
		{
			C_PolygonVertex* pPolyVert = &m_polygonVertices[iPolyVert];
			
			//Remap vertex normals
			{
				S32 normOld = pPolyVert->GetNormalIndex();

				//Ignore unasigned vertex positions.
				if (normOld != -1)
				{
					S32 vertexNormalShuffleAccumulator = vertexNormalShuffleBuffer[normOld];
					S32 normNew = normOld - vertexNormalShuffleAccumulator;
					pPolyVert->SetNormalIndex(normNew);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	//7) Overwrite the old mesh component arrays.
	//////////////////////////////////////////////////////////////////

	Ut::MoveRaw(m_vertexNormals, newVertexNormals);
	return numRepairs;
}

U32 C_SceneMeshNode::RepairTextureCoordinates()
{
	U32 numRepairs = 0;
	for (U32 iSet = 0; iSet < GetNumTextureCoordinateSets(); iSet++)
	{
		numRepairs += RepairTextureCoordinates(iSet);
	}
	return numRepairs;
}

U32 C_SceneMeshNode::RepairVertexTangents()
{
	DirtyGeometryDependantCaches();
	U32 numRepairs = 0;
	for (U32 iSet = 0; iSet < GetNumTextureCoordinateSets(); iSet++)
	{
		numRepairs += RepairVertexTangents(iSet);
	}
	return numRepairs;
}

U32 C_SceneMeshNode::RepairVertexBinormals()
{
	DirtyGeometryDependantCaches();
	U32 numRepairs = 0;
	for (U32 iSet = 0; iSet < GetNumTextureCoordinateSets(); iSet++)
	{
		numRepairs += RepairVertexBinormals(iSet);
	}
	return numRepairs;
}

U32 C_SceneMeshNode::RepairTextureCoordinates(U32 set)
{
	U32 numRepairs = 0;

	//////////////////////////////////////////////////////////////////
	//1) Create shuffle buffers for unused component clean-up.
	//////////////////////////////////////////////////////////////////

	//Create the shuffle buffer for removing unused texture coordinates.
	Ut::UtVector< Ut::DynamicVector<S32> >  uvShuffleBuffers;
	uvShuffleBuffers.Resize(m_textureCoordinateSets[set].GetNumTextureCoordinates());
	for (U32 iUV = 0; iUV < (U32)m_textureCoordinateSets[set].GetNumTextureCoordinates(); iUV++)
	{
		uvShuffleBuffers[iUV] = -2;
	}

	//////////////////////////////////////////////////////////////////
	//2) Create storage for our new component arrays.
	//////////////////////////////////////////////////////////////////

	Ut::UtVector< Ut::DynamicVector<TextureCoordinate4> >  newUVs;
	newUVs.Reserve(m_textureCoordinateSets[set].GetNumTextureCoordinates());

	//////////////////////////////////////////////////////////////////
	//3) Generate component usage information and build new component
	//   arrays.
	//////////////////////////////////////////////////////////////////

	//Get polygon vertex component usage
	for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
	{
		C_PolygonVertex* pPolygonVertex = &m_polygonVertices[iPolyVert];

		S32 uvIndex = pPolygonVertex->GetTextureCoordinateIndex(set);
		if (uvIndex != -1)
		{
			if (uvIndex >= (S32)GetNumTextureCoordinates(set))
			{
				//Mark index as invalid.
				pPolygonVertex->SetTextureCoordinateIndex(set, -1);
				numRepairs++;
			}
			else
			{
				//Flag this as being used. -1 indicates used -2 indicates unused.
				uvShuffleBuffers[uvIndex] = -1;
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	//4) Copy mesh components that are flagged as being used
	//////////////////////////////////////////////////////////////////
	{
		for (U32 iUV = 0; iUV < (U32)m_textureCoordinateSets[set].GetNumTextureCoordinates(); iUV++)
		{
			if (uvShuffleBuffers[iUV] == -1)
			{
				//Copy this vertex position into our new vertex position array.
				newUVs.PushBack(m_textureCoordinateSets[set].GetTextureCoordinate(iUV));
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	//5) Generate the shuffle offset information.
	//////////////////////////////////////////////////////////////////

	//Generate uv shuffle information.
	{
		U32 uvShuffleAccumulator = 0;
		for (U32 iUV = 0; iUV < (U32)m_textureCoordinateSets[set].GetNumTextureCoordinates(); iUV++)
		{
			//-2 indicates that this uv is unused.
			if (uvShuffleBuffers[iUV] == -2)
			{
				uvShuffleAccumulator++;
				numRepairs++;
			}
			//Insert our accumulated shuffle value.
			uvShuffleBuffers[iUV] = uvShuffleAccumulator;
		}
	}

	if (numRepairs == 0)
		return 0;

	//////////////////////////////////////////////////////////////////
	//6) Remap our component indices.
	//////////////////////////////////////////////////////////////////

	//Remap component indices using our shuffle buffer.
	{
		for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
		{
			C_PolygonVertex* pPolyVert = &m_polygonVertices[iPolyVert];
			
			//Remap uvs
			S32 uvOld = pPolyVert->GetTextureCoordinateIndex(set);

			//Ignore unasigned vertex positions.
			if (uvOld != -1)
			{
				S32 uvShuffleAccumulator = uvShuffleBuffers[uvOld];
				S32 uvNew = uvOld - uvShuffleAccumulator;
				pPolyVert->SetTextureCoordinateIndex(set, uvNew);
			}
		}
	}

	//////////////////////////////////////////////////////////////////
	//7) Overwrite the old mesh component arrays.
	//////////////////////////////////////////////////////////////////

	Ut::MoveRaw(m_textureCoordinateSets[set].GetTextureCoordinateArray(), newUVs);
	return numRepairs;
}

U32 C_SceneMeshNode::RepairVertexTangents(U32 set)
{
	ClearAnimationHelpers();
	U32 numRepairs = 0;
	//////////////////////////////////////////////////////////////////
	//1) Create shuffle buffers for unused component clean-up.
	//////////////////////////////////////////////////////////////////
	//Create the shuffle buffer for removing unused texture coordinates.
	Ut::UtVector< Ut::DynamicVector<S32> >  shuffleBuffers;
	shuffleBuffers.Resize(m_textureCoordinateSets[set].GetNumTangents());
	for (U32 i = 0; i < (U32)m_textureCoordinateSets[set].GetNumTangents(); i++)
	{
		shuffleBuffers[i] = -2;
	}
	//////////////////////////////////////////////////////////////////
	//2) Create storage for our new component arrays.
	//////////////////////////////////////////////////////////////////
	Ut::UtVector< Ut::DynamicVector<Vector3F32> >  newData;
	newData.Reserve(m_textureCoordinateSets[set].GetNumTangents());
	//////////////////////////////////////////////////////////////////
	//3) Generate component usage information and build new component
	//   arrays.
	//////////////////////////////////////////////////////////////////
	//Get polygon vertex component usage
	for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
	{
		C_PolygonVertex* pPolygonVertex = &m_polygonVertices[iPolyVert];

		S32 index = pPolygonVertex->GetTangentIndex(set);
		if (index != -1)
		{
			if (index >= (S32)GetNumVertexTangents(set))
			{
				//Mark index as invalid.
				pPolygonVertex->SetTangentIndex(set, -1);
				numRepairs++;
			}
			else
			{
				//Flag this as being used. -1 indicates used -2 indicates unused.
				shuffleBuffers[index] = -1;
			}
		}
	}
	//////////////////////////////////////////////////////////////////
	//4) Copy mesh components that are flagged as being used
	//////////////////////////////////////////////////////////////////
	{
		for (U32 i = 0; i < (U32)m_textureCoordinateSets[set].GetNumTangents(); i++)
		{
			if (shuffleBuffers[i] == -1)
			{
				//Copy this data into our new data array
				newData.PushBack(m_textureCoordinateSets[set].GetTangent(i));
			}
		}
	}
	//////////////////////////////////////////////////////////////////
	//5) Generate the shuffle offset information.
	//////////////////////////////////////////////////////////////////
	//Generate uv shuffle information.
	{
		U32 shuffleAccumulator = 0;
		for (U32 i = 0; i < (U32)m_textureCoordinateSets[set].GetNumTangents(); i++)
		{
			//-2 indicates that this uv is unused.
			if (shuffleBuffers[i] == -2)
			{
				shuffleAccumulator++;
				numRepairs++;
			}
			//Insert our accumulated shuffle value.
			shuffleBuffers[i] = shuffleAccumulator;
		}
	}

	if (numRepairs == 0)
		return 0;

	//////////////////////////////////////////////////////////////////
	//6) Remap our component indices.
	//////////////////////////////////////////////////////////////////
	//Remap component indices using our shuffle buffer.
	{
		for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
		{
			C_PolygonVertex* pPolyVert = &m_polygonVertices[iPolyVert];
			
			//Remap 
			S32 oldDataIndex = pPolyVert->GetTangentIndex(set);

			//Ignore unasigned data.
			if (oldDataIndex != -1)
			{
				S32 shuffleAccumulator = shuffleBuffers[oldDataIndex];
				S32 newDataIndex = oldDataIndex - shuffleAccumulator;
				pPolyVert->SetTangentIndex(set, newDataIndex);
			}
		}
	}
	//////////////////////////////////////////////////////////////////
	//7) Overwrite the old mesh component arrays.
	//////////////////////////////////////////////////////////////////
	Ut::MoveRaw(m_textureCoordinateSets[set].GetTangentArray(), newData);
	return numRepairs;
}

U32 C_SceneMeshNode::RepairVertexBinormals(U32 set)
{
	ClearAnimationHelpers();

	U32 numRepairs = 0;
	//////////////////////////////////////////////////////////////////
	//1) Create shuffle buffers for unused component clean-up.
	//////////////////////////////////////////////////////////////////
	//Create the shuffle buffer for removing unused texture coordinates.
	Ut::UtVector< Ut::DynamicVector<S32> >  shuffleBuffers;
	shuffleBuffers.Resize(m_textureCoordinateSets[set].GetNumBinormals());
	for (U32 i = 0; i < (U32)m_textureCoordinateSets[set].GetNumBinormals(); i++)
	{
		shuffleBuffers[i] = -2;
	}
	//////////////////////////////////////////////////////////////////
	//2) Create storage for our new component arrays.
	//////////////////////////////////////////////////////////////////
	Ut::UtVector< Ut::DynamicVector<Vector3F32> >  newData;
	newData.Reserve(m_textureCoordinateSets[set].GetNumBinormals());
	//////////////////////////////////////////////////////////////////
	//3) Generate component usage information and build new component
	//   arrays.
	//////////////////////////////////////////////////////////////////
	//Get polygon vertex component usage
	for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
	{
		C_PolygonVertex* pPolygonVertex = &m_polygonVertices[iPolyVert];
		S32 index = pPolygonVertex->GetBinormalIndex(set);
		if (index != -1)
		{
			if (index >= (S32)GetNumVertexBinormals(set))
			{
				//Mark index as invalid.
				pPolygonVertex->SetBinormalIndex(set, -1);
				numRepairs++;
			}
			else
			{
				//Flag this as being used. -1 indicates used -2 indicates unused.
				shuffleBuffers[index] = -1;
			}
		}
	}
	//////////////////////////////////////////////////////////////////
	//4) Copy mesh components that are flagged as being used
	//////////////////////////////////////////////////////////////////
	{
		for (U32 i = 0; i < (U32)m_textureCoordinateSets[set].GetNumBinormals(); i++)
		{
			if (shuffleBuffers[i] == -1)
			{
				//Copy this data into our new data array
				newData.PushBack(m_textureCoordinateSets[set].GetBinormal(i));
			}
		}
	}
	//////////////////////////////////////////////////////////////////
	//5) Generate the shuffle offset information.
	//////////////////////////////////////////////////////////////////
	//Generate uv shuffle information.
	{
		U32 shuffleAccumulator = 0;
		for (U32 i = 0; i < (U32)m_textureCoordinateSets[set].GetNumBinormals(); i++)
		{
			//-2 indicates that this uv is unused.
			if (shuffleBuffers[i] == -2)
			{
				shuffleAccumulator++;
				numRepairs++;
			}
			//Insert our accumulated shuffle value.
			shuffleBuffers[i] = shuffleAccumulator;
		}
	}

	if (numRepairs == 0)
		return 0;

	//////////////////////////////////////////////////////////////////
	//6) Remap our component indices.
	//////////////////////////////////////////////////////////////////
	//Remap component indices using our shuffle buffer.
	{
		for (U32 iPolyVert = 0; iPolyVert < (U32)m_polygonVertices.Size(); iPolyVert++)
		{
			C_PolygonVertex* pPolyVert = &m_polygonVertices[iPolyVert];
			
			//Remap 
			S32 oldDataIndex = pPolyVert->GetBinormalIndex(set);

			//Ignore unasigned data.
			if (oldDataIndex != -1)
			{
				S32 shuffleAccumulator = shuffleBuffers[oldDataIndex];
				S32 newDataIndex = oldDataIndex - shuffleAccumulator;
				pPolyVert->SetBinormalIndex(set, newDataIndex);
			}
		}
	}
	//////////////////////////////////////////////////////////////////
	//7) Overwrite the old mesh component arrays.
	//////////////////////////////////////////////////////////////////
	Ut::MoveRaw(m_textureCoordinateSets[set].GetBinormalArray(), newData);
	return numRepairs;
}

const C_VertexWeight& C_SceneMeshNode::GetVertexWeight(U32 instance) const
{
	return m_vertexWeightsList[instance];
}

U32 C_SceneMeshNode::GetNumVertexWeights() const
{
	return (U32)m_vertexWeightsList.Size();
}

C_Status C_SceneMeshNode::BuildAnimationHelpers() const
{
	//Abort if we don't need to rebuild the animation helpers.
	if (
		(m_animationHelpersBuilt == E_AnimHelperState_WeightBuilt)
		||
		(m_animationHelpersBuilt == E_AnimHelperState_GeoCacheWeightBuilt)
		||
		(m_animationHelpersBuilt == E_AnimHelperState_NotNeeded))
	{
		return C_Status::E_Status_Success;
	}

	if (!IsSkinned())
	{
		m_animationHelpersBuilt = E_AnimHelperState_NotNeeded;
		return C_Status::E_Status_Success;
	}

	C_Status status;
	status = BuildInfluenceNodeList();
	if (!status) return status;

	status = BuildVertexWeightList();
	if (!status) return status;

	//Build geo cache.
	if (IsSkinAnimated())
	{
		m_pPolygonVertexPositionCache = new Position3[GetNumPolygonVertices()];
		if (!m_pPolygonVertexPositionCache)
		{
			return C_Status::E_Status_OutOfMemory;
		}
		m_pPolygonVertexNormalCache = new Normal3[GetNumPolygonVertices()];
		if (!m_pPolygonVertexNormalCache)
		{
			return C_Status::E_Status_OutOfMemory;
		}

		m_ppPolygonVertexTangentCache = new Vector3F32*[GetNumTextureCoordinateSets()];
		if (!m_ppPolygonVertexTangentCache)
		{
			return C_Status::E_Status_OutOfMemory;
		}
		m_ppPolygonVertexBinormalCache = new Vector3F32*[GetNumTextureCoordinateSets()];
		if (!m_ppPolygonVertexBinormalCache)
		{
			return C_Status::E_Status_OutOfMemory;
		}
		for (U32 iUVSet = 0; iUVSet < GetNumTextureCoordinateSets(); iUVSet++)
		{
			m_ppPolygonVertexTangentCache[iUVSet] = new Vector3F32[GetNumPolygonVertices()];
			if (!m_ppPolygonVertexTangentCache[iUVSet])
			{
				return C_Status::E_Status_OutOfMemory;
			}
			m_ppPolygonVertexBinormalCache[iUVSet] = new Vector3F32[GetNumPolygonVertices()];
			if (!m_ppPolygonVertexBinormalCache[iUVSet])
			{
				return C_Status::E_Status_OutOfMemory;
			}
		}
		m_animationHelpersBuilt = E_AnimHelperState_GeoCacheWeightBuilt;
	}
	else
	{
		m_animationHelpersBuilt = E_AnimHelperState_WeightBuilt;
	}

	return Evaluate();
}

E_AnimHelperState C_SceneMeshNode::GetAnimationHelpersBuilt() const
{
	return m_animationHelpersBuilt;
}

C_Status C_SceneMeshNode::Evaluate() const
{
	C_Status status;
	if (m_animationHelpersBuilt == E_AnimHelperState_NotBuilt)
	{
		return C_Status(C_Status::E_Status_Failure, AFTK_E_ANIMATION_HELPERS_NOT_BUILT);
	}
	if (m_animationHelpersBuilt != Aftk::E_AnimHelperState_GeoCacheWeightBuilt)
	{
#ifndef AFTK_LEAN_AND_MEAN
		m_localEvaluatedBoundsCache = GetLocalBounds();
		m_staticWorldEvaluatedBoundsCache = GetWorldBounds(false);
		m_dynamicWorldEvaluatedBoundsCache = GetWorldBounds(true);
#endif
		return C_Status::E_Status_Success;
	}

	Matrix44 worldMatrix = GetWorldMatrix();
	Matrix44 worldMatrixInv;
	UtMaths::Invert4x4(&worldMatrixInv, worldMatrix);

	if (m_influenceNodeList.Size() == 0)
	{
		return C_Status::E_Status_Success;
	}

	for (U32 iBone = 0; iBone < (U32)m_influenceNodeList.Size(); iBone++)
	{
		const C_SceneNode* pBoneNode = m_influenceNodeList[iBone];

		//Transform this bone from bind space to bone space.
		//Final transform F = M C where M = offset matrix and C = combined (aka world) matrix.
		//Combined transform is evaluated at the current time.

		//Vertices start in 'bind space' - base pose transform moves them into bone space.
		//This can then be multiplied by the world matrix for the bone to give the new vertex transform

		m_matrixPalette[iBone].matrix = 
			GetBasePose() * 
			pBoneNode->GetBasePoseInverse() * 
			pBoneNode->GetWorldMatrix() * worldMatrixInv;

		Matrix44 matrixTranspose;
		matrixTranspose = m_matrixPalette[iBone].matrix;
		UtMaths::Transpose(&matrixTranspose);
		UtMaths::Invert4x4(&(m_matrixPalette[iBone].matrixInverseTranspose), matrixTranspose);
	}

#ifndef AFTK_LEAN_AND_MEAN
	m_localEvaluatedBoundsCache.Clear();
	m_staticWorldEvaluatedBoundsCache.Clear();
	m_dynamicWorldEvaluatedBoundsCache.Clear();
#endif
	for (U32 iPolyVert = 0; iPolyVert < GetNumPolygonVertices(); iPolyVert++)
	{
		EvaluatePolygonVertexPositionAndNormal(
			iPolyVert, 
			m_pPolygonVertexPositionCache[iPolyVert],
			m_pPolygonVertexNormalCache[iPolyVert]);
#ifndef AFTK_LEAN_AND_MEAN
		//Recalculate our deformed bounding box.
		m_localEvaluatedBoundsCache.AddVertex(m_pPolygonVertexPositionCache[iPolyVert]);
		m_staticWorldEvaluatedBoundsCache.AddVertex(
			Aftk::ApplyMatrix(GetWorldMatrix(false), m_pPolygonVertexPositionCache[iPolyVert]));
		m_dynamicWorldEvaluatedBoundsCache.AddVertex(
			Aftk::ApplyMatrix(GetWorldMatrix(true), m_pPolygonVertexPositionCache[iPolyVert]));
#endif
	}

	for (U32 iUVSet = 0; iUVSet < GetNumTextureCoordinateSets(); iUVSet++)
	{
		for (U32 iPolyVert = 0; iPolyVert < GetNumPolygonVertices(); iPolyVert++)
		{
			EvaluatePolygonVertexTangentAndBinormal(
				iPolyVert, 
				iUVSet,
				(m_ppPolygonVertexTangentCache[iUVSet])[iPolyVert],
				(m_ppPolygonVertexBinormalCache[iUVSet])[iPolyVert]);
		}
	}

	return C_Status::E_Status_Success;
}

const C_MatrixPaletteEntry& C_SceneMeshNode::GetPaletteEntry(U32 instance) 
{
	if (m_animationHelpersBuilt == E_AnimHelperState_NotBuilt)
	{
		BuildAnimationHelpers();
	}
	AFTK_ASSERT(m_animationHelpersBuilt != E_AnimHelperState_NotBuilt);
	return m_matrixPalette[instance];
}

U32 C_SceneMeshNode::GetNumPaletteEntries() 
{
	if (m_animationHelpersBuilt == E_AnimHelperState_NotBuilt)
	{
		BuildAnimationHelpers();
	}
	AFTK_ASSERT(m_animationHelpersBuilt != E_AnimHelperState_NotBuilt);
	return (U32)m_matrixPalette.Size();
}

Position3 C_SceneMeshNode::GetPolygonVertexPosition(U32 polyVertIndex) const
{
	AFTK_ASSERT(polyVertIndex < GetNumPolygonVertices());
	const C_PolygonVertex* pPV = &GetPolygonVertex(polyVertIndex);
	if (pPV->GetPositionIndex() == -1)
	{
		Position3 tmp;
		tmp[0] = 0.0f;
		tmp[1] = 0.0f;
		tmp[2] = 0.0f;
		return tmp;
	}

	return GetVertexPosition(pPV->GetPositionIndex());
}

Normal3 C_SceneMeshNode::GetPolygonVertexNormal(U32 polyVertIndex) const
{
	AFTK_ASSERT(polyVertIndex < GetNumPolygonVertices());
	const C_PolygonVertex* pPV = &GetPolygonVertex(polyVertIndex);
	if (pPV->GetNormalIndex() == -1)
	{
		Normal3 tmp;
		tmp[0] = 0.0f;
		tmp[1] = 0.0f;
		tmp[2] = 0.0f;
		return tmp;
	}

	return GetVertexNormal(pPV->GetNormalIndex());
}

Vector3F32 C_SceneMeshNode::GetPolygonVertexTangent(U32 uvSet, U32 polyVertIndex) const
{
	AFTK_ASSERT(polyVertIndex < GetNumPolygonVertices());
	const C_PolygonVertex* pPV = &GetPolygonVertex(polyVertIndex);
	if (pPV->GetTangentIndex(uvSet) == -1)
	{
		Vector3F32 tmp;
		tmp[0] = 0.0f;
		tmp[1] = 0.0f;
		tmp[2] = 0.0f;
		return tmp;
	}

	return GetVertexTangent(uvSet, pPV->GetTangentIndex(uvSet));
}

Vector3F32 C_SceneMeshNode::GetPolygonVertexBinormal(U32 uvSet, U32 polyVertIndex) const
{
	AFTK_ASSERT(polyVertIndex < GetNumPolygonVertices());
	const C_PolygonVertex* pPV = &GetPolygonVertex(polyVertIndex);
	if (pPV->GetBinormalIndex(uvSet) == -1)
	{
		Vector3F32 tmp;
		tmp[0] = 0.0f;
		tmp[1] = 0.0f;
		tmp[2] = 0.0f;
		return tmp;
	}
	return GetVertexBinormal(uvSet, pPV->GetBinormalIndex(uvSet));
}

C_Bounds C_SceneMeshNode::GetEvaluatedLocalBounds() const
{
	BuildAnimationHelpers();
	Evaluate();

#ifdef AFTK_LEAN_AND_MEAN
	C_Bounds bounds;
	for (U32 iPolyVert = 0; iPolyVert < GetNumPolygonVertices(); iPolyVert++)
	{
		//Recalculate our deformed bounding box.
		bounds.AddVertex(GetEvaluatedPolygonVertexPosition(iPolyVert));
	}
	return bounds;
#else
	return m_localEvaluatedBoundsCache;
#endif
}
	
C_Bounds C_SceneMeshNode::GetEvaluatedWorldBounds(bool evaluateTransform) const
{
	BuildAnimationHelpers();
	Evaluate();

#ifdef AFTK_LEAN_AND_MEAN
	C_Bounds bounds;
	if (evaluateTransform)
	{
		for (U32 iPolyVert = 0; iPolyVert < GetNumPolygonVertices(); iPolyVert++)
		{
			bounds.AddVertex(
				Aftk::ApplyMatrix(GetWorldMatrix(true), GetEvaluatedPolygonVertexPosition(iPolyVert)));
		}
		return bounds;
	}
	else
	{
		for (U32 iPolyVert = 0; iPolyVert < GetNumPolygonVertices(); iPolyVert++)
		{
			bounds.AddVertex(
				Aftk::ApplyMatrix(GetWorldMatrix(false), GetEvaluatedPolygonVertexPosition(iPolyVert)));
		}
		return bounds;
	}
#else
	if (evaluateTransform)
		return m_dynamicWorldEvaluatedBoundsCache;
	
	return m_staticWorldEvaluatedBoundsCache;
#endif
}

Position3 C_SceneMeshNode::GetEvaluatedPolygonVertexPosition(U32 polyVertIndex) const
{
	if (m_animationHelpersBuilt == E_AnimHelperState_NotBuilt)
		BuildAnimationHelpers();

	AFTK_ASSERT(m_animationHelpersBuilt != E_AnimHelperState_NotBuilt);
	AFTK_ASSERT(polyVertIndex < GetNumPolygonVertices());

	if (m_animationHelpersBuilt != E_AnimHelperState_GeoCacheWeightBuilt)
	{
		const C_PolygonVertex* pPV = &GetPolygonVertex(polyVertIndex);
		if (pPV->GetPositionIndex() == -1)
		{
			Position3 tmp;
			tmp[0] = 0.0f;
			tmp[1] = 0.0f;
			tmp[2] = 0.0f;
			return tmp;
		}
		else
		{
			return GetVertexPosition(pPV->GetPositionIndex());
		}		
	}
	return m_pPolygonVertexPositionCache[polyVertIndex];
}

TextureCoordinate4 C_SceneMeshNode::GetPolygonVertexTextureCoordinate(U32 uvSet, U32 polyVertIndex) const
{
	const C_PolygonVertex* pPV = &GetPolygonVertex(polyVertIndex);
	S32 uvIndex = pPV->GetTextureCoordinateIndex(uvSet);
	if (uvIndex == -1)
	{
		return TextureCoordinate4(0,0,0,0);
	}
	return GetTextureCoordinate(uvSet, uvIndex);
}

Normal3 C_SceneMeshNode::GetEvaluatedPolygonVertexNormal(U32 polyVertIndex) const
{
	if (m_animationHelpersBuilt == E_AnimHelperState_NotBuilt)
	{
		BuildAnimationHelpers();
	}
	AFTK_ASSERT(m_animationHelpersBuilt != E_AnimHelperState_NotBuilt);
	AFTK_ASSERT(polyVertIndex < GetNumPolygonVertices());
	if (m_animationHelpersBuilt != E_AnimHelperState_GeoCacheWeightBuilt)
	{
		const C_PolygonVertex* pPV = &GetPolygonVertex(polyVertIndex);
		if (pPV->GetNormalIndex() == -1)
		{
			Normal3 tmp;
			tmp[0] = 0.0f;
			tmp[1] = 0.0f;
			tmp[2] = 0.0f;
			return tmp;
		}
		else
		{
			return GetVertexNormal(pPV->GetNormalIndex());
		}
	}

	return m_pPolygonVertexNormalCache[polyVertIndex];
}

Vector3F32 C_SceneMeshNode::GetEvaluatedPolygonVertexTangent(U32 uvSet, U32 polyVertIndex) const
{
	if (m_animationHelpersBuilt == E_AnimHelperState_NotBuilt)
	{
		BuildAnimationHelpers();
	}
	AFTK_ASSERT(m_animationHelpersBuilt != E_AnimHelperState_NotBuilt);
	AFTK_ASSERT(polyVertIndex < GetNumPolygonVertices());

	if (m_animationHelpersBuilt != E_AnimHelperState_GeoCacheWeightBuilt)
	{
		const C_PolygonVertex* pPV = &GetPolygonVertex(polyVertIndex);
		if (pPV->GetTangentIndex(uvSet) == -1)
		{
			Vector3F32 tmp;
			tmp[0] = 0.0f;
			tmp[1] = 0.0f;
			tmp[2] = 0.0f;
			return tmp;
		}
		else
		{
			return GetVertexTangent(uvSet, pPV->GetTangentIndex(uvSet));
		}
	}

	return m_ppPolygonVertexTangentCache[uvSet][polyVertIndex];
}

Vector3F32 C_SceneMeshNode::GetEvaluatedPolygonVertexBinormal(U32 uvSet, U32 polyVertIndex) const
{
	if (m_animationHelpersBuilt != E_AnimHelperState_NotBuilt)
	{
		BuildAnimationHelpers();
	}
	AFTK_ASSERT(m_animationHelpersBuilt != E_AnimHelperState_NotBuilt);
	AFTK_ASSERT(polyVertIndex < GetNumPolygonVertices());

	if (m_animationHelpersBuilt != E_AnimHelperState_GeoCacheWeightBuilt)
	{
		const C_PolygonVertex* pPV = &GetPolygonVertex(polyVertIndex);
		if (pPV->GetBinormalIndex(uvSet) == -1)
		{
			Vector3F32 tmp;
			tmp[0] = 0.0f;
			tmp[1] = 0.0f;
			tmp[2] = 0.0f;
			return tmp;
		}
		else
		{
			return GetVertexBinormal(uvSet, pPV->GetBinormalIndex(uvSet));
		}
	}

	return m_ppPolygonVertexBinormalCache[uvSet][polyVertIndex];
}

// generate adjacency information for this mesh
bool C_SceneMeshNode::BuildAdjacency() const
{
	C_BuildAdjacency ab;

	return ab.GenerateAdjacency(this);
}

void C_SceneMeshNode::ApplyMatrix(const Matrix44& matrix)
{
	Matrix44 temp, normalTransform;
	if (m_normalInverseTranspose)
	{
		temp = matrix;
		UtMaths::Transpose(&temp);
		Invert4x4(&normalTransform, temp);
	}
	else
	{
		normalTransform = matrix;
	}

	for (U32 i = 0; i < GetNumVertexPositions(); i++)
	{
		Position3 temp = Aftk::ApplyMatrix(matrix, GetVertexPosition(i));
		SetVertexPosition(i, temp);
	}

	for (U32 i = 0; i < GetNumVertexNormals(); i++)
	{
		//Note that the below function normalises the result.
		Normal3 temp = ApplyMatrixToNormal(normalTransform, GetVertexNormal(i));
		SetVertexNormal(i, temp);
	}

	for (U32 set = 0; set < GetNumTextureCoordinateSets(); set++)
	{
		for (U32 i = 0; i < GetNumVertexTangents(set); i++)
		{
			Vector3F32 temp = Aftk::ApplyMatrix(normalTransform, GetVertexTangent(set, i));
			SetVertexTangent(set, i, temp);
		}
		for (U32 i = 0; i < GetNumVertexBinormals(set); i++)
		{
			Vector3F32 temp = Aftk::ApplyMatrix(normalTransform, GetVertexBinormal(set, i));
			SetVertexBinormal(set, i, temp);
		}
	}

	DirtyGeometryDependantCaches();
}

bool C_SceneMeshNode::IsAdjacencyBuilt() const
{
	if (m_adjacencyList.Size() > 0)
	{
		return true;
	}

	return false;
}

// get list of ints that have the adjacency info
const U32 *C_SceneMeshNode::GetAdjacencyList(U32 iThisPoly) const
{
	if (!IsAdjacencyBuilt())
		BuildAdjacency();
	return &m_adjacencyList[iThisPoly];
}

U32 C_SceneMeshNode::Validate() const
{
	C_SceneMeshNode* pThis = const_cast<C_SceneMeshNode*>(this);
	return pThis->ProcessProblems(false);
}

U32 C_SceneMeshNode::ProcessProblems(bool fix) 
{
	U32 errorCount = 0;
	errorCount += C_SceneMeshRemappableNode::ProcessProblems(fix);

	for (U32 i = 0; i < GetNumPolygonVertices(); i++)
	{
		C_PolygonVertex* pPv = &GetPolygonVertex(i);
	
		S32 posIndex = pPv->GetPositionIndex();
		if (posIndex != -1)
		{
			if (posIndex >= (S32)GetNumVertexPositions())
			{
				AFTK_DO_BEGIN << "Mesh " << GetName() << " C_Polygon Vertex " << i << " has an invalid position index = " << posIndex << "/" << GetNumVertexPositions() << "\n" AFTK_DO_DEBUG_END;
				#ifdef _DEBUG
				AFTK_ASSERT(!("posIndex >= (S32)GetNumVertexPositions()."));
				#endif
				errorCount++;
			}
		}
		else
		{
			AFTK_DO_BEGIN << "Mesh " << GetName() << " C_Polygon Vertex " << i << " has no position index.\n" AFTK_DO_DEBUG_END;
			#ifdef _DEBUG
			AFTK_ASSERT(!("C_Polygon vertex has no position index."));
			#endif
			errorCount++;
		}
		S32 normIndex = pPv->GetNormalIndex();
		if (normIndex != -1)
		{
			if (normIndex >= (S32)GetNumVertexNormals())
			{
				AFTK_DO_BEGIN << "Mesh " << GetName() << " C_Polygon Vertex " << i << " has an invalid normal index = " << normIndex << "/" << GetNumVertexNormals() << "\n" AFTK_DO_DEBUG_END;
				#ifdef _DEBUG
				AFTK_ASSERT(!("normIndex >= (S32)GetNumVertexNormals()"));
				#endif
				errorCount++;
			}
		}

		if (pPv->GetNumTextureCoordinateIndices() > GetNumTextureCoordinateSets())
		{
			AFTK_DO_BEGIN << "Mesh " << GetName() << " C_Polygon Vertex " << i << " has too many texture coordinates\n" AFTK_DO_DEBUG_END;
			#ifdef _DEBUG
			AFTK_ASSERT(!("pPv->GetNumTextureCoordinateIndices() > GetNumTextureCoordinateSets()"));
			#endif
			errorCount++;
		}
		else
		{
			for (U32 set = 0; set < pPv->GetNumTextureCoordinateIndices(); set++)
			{
				if (pPv->GetTextureCoordinateIndex(set) >= (S32)GetNumTextureCoordinates(set))
				{
					AFTK_DO_BEGIN << "Mesh " << GetName() << " C_Polygon Vertex " << i << " has an invalid texture coordinate (set " << set << ") index = " << pPv->GetTextureCoordinateIndex(set) << "/" << GetNumTextureCoordinates(set) << "\n" AFTK_DO_DEBUG_END;
					#ifdef _DEBUG
					AFTK_ASSERT(!("pPv->GetTextureCoordinateIndex(set) >= (S32)GetNumTextureCoordinates(set)"));
					#endif
					if (fix)
						pPv->SetTextureCoordinateIndex(set, 0);
					else
						errorCount++;
				}
				if (pPv->GetTangentIndex(set) != -1)
				{
					if (pPv->GetTangentIndex(set) >= (S32)GetNumVertexTangents(set))
					{
						AFTK_DO_BEGIN << "Mesh " << GetName() << " C_Polygon Vertex " << i << " has an invalid tangent index (set " << set << ") index = " << pPv->GetTangentIndex(set) << "/" << GetNumVertexTangents(set) << "\n" AFTK_DO_DEBUG_END;
						#ifdef _DEBUG
						AFTK_ASSERT(!("pPv->GetTangentIndex(set) >= (S32)GetNumVertexTangents(set)"));
						#endif
						if (fix)
							pPv->SetTangentIndex(set, -1);
						else
							errorCount++;
					}
				}

				if (pPv->GetBinormalIndex(set) != -1)
				{
					if (pPv->GetBinormalIndex(set) >= (S32)GetNumVertexBinormals(set))
					{
						AFTK_DO_BEGIN << "Mesh " << GetName() << " C_Polygon Vertex " << i << " has an invalid binormal index (set " << set << ") index = " << pPv->GetBinormalIndex(set) << "/" << GetNumVertexBinormals(set) << "\n" AFTK_DO_DEBUG_END;
						#ifdef _DEBUG
						AFTK_ASSERT(!("pPv->GetBinormalIndex(set) >= (S32)GetNumVertexBinormals(set)"));
						#endif
						if (fix)
							pPv->SetBinormalIndex(set, -1);
						else
							errorCount++;
					}
				}
			}
			if (pPv->GetNumTextureCoordinateIndices() < GetNumTextureCoordinateSets())
			{
				U32 numUVIndices = pPv->GetNumTextureCoordinateIndices();
				U32 numUVSets = GetNumTextureCoordinateSets();
				AFTK_DO_BEGIN << "Mesh " << GetName() << " C_Polygon Vertex " << i << " does not have enough texture coordinates\n" AFTK_DO_DEBUG_END;
				#ifdef _DEBUG
				AFTK_ASSERT(!("pPv->GetNumTextureCoordinateIndices() < GetNumTextureCoordinateSets()"));
				#endif
				errorCount++;
			}
		}

		U32 numData = pPv->GetNumData();
		U32 numChannels = GetNumVertexDataChannels();
		if (numData > numChannels)
		{
			AFTK_DO_BEGIN << "Mesh " << GetName() << " C_Polygon Vertex " << i << " has too many vertex data channels\n" AFTK_DO_DEBUG_END;
			#ifdef _DEBUG
			AFTK_ASSERT(!("pPv->GetNumData() > GetNumVertexDataChannels()"));
			#endif
			errorCount++;
		}
		else if (pPv->GetNumTextureCoordinateIndices() < GetNumTextureCoordinateSets())
		{
			if (pPv->GetNumData() < GetNumVertexDataChannels())
			{
				AFTK_DO_BEGIN << "Mesh " << GetName() << " C_Polygon Vertex " << i << " does not have enough vertex data channels\n" AFTK_DO_DEBUG_END;
				#ifdef _DEBUG
				AFTK_ASSERT(!("pPv->GetNumData() < GetNumVertexDataChannels()"));
				#endif
				errorCount++;
			}
		}
	}

	U32 nPolysInCluster = 0;
	Ut::UtSet<const C_PolygonCluster*> clusterSet;
	for (U32 iCluster = 0; iCluster < GetNumPolygonClusters(); iCluster++)
	{
		const C_PolygonCluster* pPolyCluster = GetPolygonClusterPtr(iCluster);
		clusterSet.Insert(pPolyCluster);

		for (U32 jLodLevel = 0; jLodLevel < pPolyCluster->GetNumLodLevels(); jLodLevel++)
		{
			nPolysInCluster += pPolyCluster->GetNumPolygonIndices(jLodLevel);
			for (U32 kPoly = 0; kPoly < pPolyCluster->GetNumPolygonIndices(jLodLevel); kPoly++)
			{
				U32 polygonIndex =
					pPolyCluster->GetPolygonIndex(jLodLevel, kPoly);

				if (polygonIndex >= GetNumPolygons(jLodLevel))
				{
					AFTK_DO_BEGIN << "Mesh " << GetName() << " has a poly cluster entry with an invalid poly index.\n" AFTK_DO_DEBUG_END;
					#ifdef _DEBUG
					AFTK_ASSERT(!("Poly cluster invalid poly index."));
					#endif
					errorCount++;
				}
			}
		}
	}

	U32 nPolys = 0;
	for (U32 iLodLevel = 0; iLodLevel < GetNumLodLevels(); iLodLevel++)
	{
		nPolys += GetNumPolygons(iLodLevel);
		for (U32 jPoly = 0; jPoly < GetNumPolygons(iLodLevel); jPoly++)
		{
			const C_Polygon* pPoly = &GetPolygon(iLodLevel, jPoly);
			if (pPoly->GetMaterialMappingIndex() >= GetNumPolygonMaterialMappings())
			{
				AFTK_DO_BEGIN << "Mesh " << GetName() << " invalid material index on poly.\n" AFTK_DO_DEBUG_END;
			}
			if (pPoly->GetBlindDataMappingIndex() >= GetNumPolygonBlindDataMappings())
			{
				AFTK_DO_BEGIN << "Mesh " << GetName() << " invalid material index on poly.\n" AFTK_DO_DEBUG_END;
			}

			
			errorCount += pPoly->Validate();

			Ut::UtSet<const C_PolygonCluster*>::Iterator it;
			it = clusterSet.Find(pPoly->GetClusterPtr());
			if (it == clusterSet.End())
			{
					AFTK_DO_BEGIN << "Mesh " << GetName() << " has a polygon that references a missing polygon cluster.\n" AFTK_DO_DEBUG_END;
					#ifdef _DEBUG
					AFTK_ASSERT(!("Mesh has a polygon that references a missing polygon cluster."));
					#endif
					errorCount++;
			}
		}
	}

	//All of our polygons MUST be within a cluster.
	if (nPolysInCluster < nPolys)
	{
//		#ifdef _DEBUG
		AFTK_DO_BEGIN << "Mesh " << GetName() << " has polygons that are not within a cluster.\n" AFTK_DO_DEBUG_END;
		AFTK_DO_BEGIN << "nPolysInCluster " << nPolysInCluster << " nPolys " << nPolys << "\n" AFTK_DO_DEBUG_END;

		AFTK_DO_BEGIN << "POLY DUMP\n" AFTK_DO_DEBUG_END;
		for (U32 iLodLevel = 0; iLodLevel < GetNumLodLevels(); iLodLevel++)
		{
			for (U32 jPoly = 0; jPoly < GetNumPolygons(iLodLevel); jPoly++)
			{
				const C_Polygon* pPoly = &GetPolygon(iLodLevel, jPoly);
				if (pPoly->GetClusterPtr())
				{
					AFTK_DO_BEGIN << "LOD " << iLodLevel << " poly idx " << jPoly << " cluster name " << pPoly->GetClusterPtr()->GetName() << "\n" AFTK_DO_DEBUG_END;
				}
				else
				{
					AFTK_DO_BEGIN << "LOD " << iLodLevel << " poly idx " << jPoly << " NO CLUSTER" << "\n" AFTK_DO_DEBUG_END;
				}
			}
		}
		AFTK_DO_BEGIN << "END POLY DUMP\n" AFTK_DO_DEBUG_END;

		for (U32 iCluster = 0; iCluster < GetNumPolygonClusters(); iCluster++)
		{
			const C_PolygonCluster* pPolyCluster = GetPolygonClusterPtr(iCluster);
			AFTK_DO_BEGIN << "Cluster DUMP\n" AFTK_DO_DEBUG_END;
			AFTK_DO_BEGIN << "Cluster : " << pPolyCluster->GetName() << "\n" AFTK_DO_DEBUG_END;

			for (U32 iLodLevel = 0; iLodLevel < pPolyCluster->GetNumLodLevels(); iLodLevel++)
			{
				for (U32 jPoly = 0; jPoly < pPolyCluster->GetNumPolygonIndices(iLodLevel); jPoly++)
					AFTK_DO_BEGIN << "Poly idx " << pPolyCluster->GetPolygonIndex(iLodLevel, jPoly) << "\n" AFTK_DO_DEBUG_END;
			}
			AFTK_DO_BEGIN << "END CLUSTER DUMP\n" AFTK_DO_DEBUG_END;
		}

		#ifdef _DEBUG
		AFTK_ASSERT(!("Mesh has polygons that are not within a cluster."));
		#endif
		errorCount++;
	}

//	#ifdef _DEBUG
//	g_aftkDebugManager << errorCount << " errors encountered during validation.\n";
//	#endif
	return errorCount;
}

U32 C_SceneMeshNode::GetNumTriangles(U32 lodLevel) const
{
	U32 triCount = 0;

	for (U32 iPoly = 0; iPoly < GetNumPolygons(lodLevel); iPoly++)
	{
		const C_Polygon* pPoly = &GetPolygon(lodLevel, iPoly);
		U32 numPVerts = pPoly->GetNumPolygonVertices();

		if (numPVerts < 3)
		{
			//There should never be < 3 pVerts anyhow.
			continue;
		}

		triCount += numPVerts - 2;
	}

	return triCount;
}

U32 C_SceneMeshNode::GetNumTriangles() const
{
	U32 triCount = 0;

	for (U32 iLodLevel = 0; iLodLevel < GetNumLodLevels(); iLodLevel++)
	{
		for (U32 jPoly = 0; jPoly < GetNumPolygons(iLodLevel); jPoly++)
		{
			const C_Polygon* pPoly = &GetPolygon(iLodLevel, jPoly);
			U32 numPVerts = pPoly->GetNumPolygonVertices();

			if (numPVerts < 3)
			{
				//There should never be < 3 pVerts anyhow.
				continue;
			}

			triCount += numPVerts - 2;
		}
	}

	return triCount;
}

void C_SceneMeshNode::ClearAnimationHelpers() const
{
	if ((m_animationHelpersBuilt != E_AnimHelperState_WeightBuilt) &&
		(m_animationHelpersBuilt != E_AnimHelperState_GeoCacheWeightBuilt))
	{
		return;
	}

	m_matrixPalette.Clear();
	m_vertexWeightsList.Clear();
	m_influenceNodeList.Clear();

	if (m_animationHelpersBuilt == E_AnimHelperState_GeoCacheWeightBuilt)
	{
		delete[] m_pPolygonVertexPositionCache;
		m_pPolygonVertexPositionCache = 0;

		delete[] m_pPolygonVertexNormalCache;
		m_pPolygonVertexNormalCache = 0;
		
		if (m_ppPolygonVertexTangentCache)
		{
			for (U32 iUVSet = 0; iUVSet < GetNumTextureCoordinateSets(); iUVSet++)
			{
				delete[] m_ppPolygonVertexTangentCache[iUVSet];
			}
			delete[] m_ppPolygonVertexTangentCache;
			m_ppPolygonVertexTangentCache = 0;
		}
		
		if (m_ppPolygonVertexBinormalCache)
		{
			for (U32 iUVSet = 0; iUVSet < GetNumTextureCoordinateSets(); iUVSet++)
			{
				delete[] m_ppPolygonVertexBinormalCache[iUVSet];
			}
			delete[] m_ppPolygonVertexBinormalCache;
			m_ppPolygonVertexBinormalCache = 0;
		}
	}

	if ((!IsSkinned()) || (!IsTransformAnimated()))
	{
		m_animationHelpersBuilt = E_AnimHelperState_NotNeeded;
	}
    
	m_animationHelpersBuilt = E_AnimHelperState_NotBuilt;
}

bool C_SceneMeshNode::IsSkinned() const
{
	for (U32 iInfluenceCluster = 0; iInfluenceCluster < (U32)m_influenceClusters.Size(); iInfluenceCluster++)
	{
		const C_InfluenceClusterNode* pInfluenceCluster = m_influenceClusters[iInfluenceCluster];
		if (pInfluenceCluster)
		{
			if (pInfluenceCluster->GetUsage() != AFTK_INFLUENCE_CLUSTER_USAGE_SKIN)
			{
				//Skip irrelevant influence clusters
				continue;
			}
			return true;
		}
	}

	return false;
}

bool C_SceneMeshNode::IsSkinAnimated() const
{
	for (U32 iInfluenceCluster = 0; iInfluenceCluster < (U32)m_influenceClusters.Size(); iInfluenceCluster++)
	{
		const C_InfluenceClusterNode* pInfluenceCluster = m_influenceClusters[iInfluenceCluster];
		if (!pInfluenceCluster)
		{
			//Skip null influence clusters.
			continue;
		}
		for (U32 jEnv = 0; jEnv < pInfluenceCluster->GetNumEnvelopes(); jEnv++)
		{
			const C_WeightEnvelope* pWeightEnvelope = &pInfluenceCluster->GetEnvelope(jEnv);
			if (!pWeightEnvelope)
			{
				//Skip null weighting envelopes.
				continue;
			}
			const C_SceneNode* pInfluenceNode = pWeightEnvelope->GetInfluenceNodePtr();
			if (!pInfluenceNode)
			{
				//Skip null influence nodes.
				continue;
			}
			if (pInfluenceNode->IsTransformAnimated())
			{
				//At least one of the skins influences is animated - return true.
				return true;
			}
		}
	}

	return false;
}

void C_SceneMeshNode::Clear()
{
	ClearThis();
	C_SceneMeshRemappableNode::Clear();
}

PolygonVertexArray& C_SceneMeshNode::GetPolygonVertexArray()
{
	return m_polygonVertices;
}

const PolygonVertexArray& C_SceneMeshNode::GetPolygonVertexArray() const
{
	return m_polygonVertices;
}

C_TextureCoordinateSet& C_SceneMeshNode::GetTextureCoordinateSet(U32 set) 
{
	return m_textureCoordinateSets[set];
}

const C_TextureCoordinateSet& C_SceneMeshNode::GetTextureCoordinateSet(U32 set) const
{
	return m_textureCoordinateSets[set];
}

PolygonArray& C_SceneMeshNode::GetPolygonArray(U32 lodLevel)
{
	return m_lodLevels[lodLevel].GetPolygonArray();
}

const PolygonArray& C_SceneMeshNode::GetPolygonArray(U32 lodLevel) const
{
	return m_lodLevels[lodLevel].GetPolygonArray();
}

Position3Array& C_SceneMeshNode::GetVertexPositionArray()
{
	return m_vertexPositions;
}

const Position3Array& C_SceneMeshNode::GetVertexPositionArray() const
{
	return m_vertexPositions;
}

Normal3Array& C_SceneMeshNode::GetVertexNormalArray()
{
	return m_vertexNormals;
}

const Normal3Array& C_SceneMeshNode::GetVertexNormalArray() const
{
	return m_vertexNormals;
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneMeshNode::CalculateBounds(C_Bounds& bounds, const Matrix44* pMatrix) const
{
	bounds.Clear();

	U32 nPolys = 0;
	//Note how our mesh bounding box takes in to account all of the levels of detail.
	for (U32 iLodLevel = 0; iLodLevel < (U32)m_lodLevels.Size(); iLodLevel++)
	{
		nPolys += m_lodLevels[iLodLevel].GetNumPolygons();
		for (U32 jPoly = 0; jPoly < m_lodLevels[iLodLevel].GetNumPolygons(); jPoly++)
		{
			const C_Polygon* pPoly = &m_lodLevels[iLodLevel].GetPolygon(jPoly);

			for (U32 kPolyVert = 0; kPolyVert < pPoly->GetNumPolygonVertices(); kPolyVert++)
			{
				const C_PolygonVertex* pVertex = &pPoly->GetPolygonVertex(kPolyVert);

				S32 posIndex = pVertex->GetPositionIndex();
				if (posIndex != -1)
				{
					if (pMatrix)
						bounds.AddVertex(Aftk::ApplyMatrix(*pMatrix, GetVertexPosition(posIndex)));
					else
						bounds.AddVertex(GetVertexPosition(posIndex));
				}
			}
		}
	}

	if (nPolys == 0)
		bounds.AddVertex(Position3(0.f,0.f,0.f));
}

void C_SceneMeshNode::DeletePolygonBlindDataMapping(U16 mappingIndex)
{
	for (U32 iLodLevel = 0; iLodLevel < (U32)m_lodLevels.Size(); iLodLevel++)
	{
		//Shuffle polygons.
		for (U32 jPoly = 0; jPoly < (U32)m_lodLevels[iLodLevel].GetNumPolygons(); jPoly++)
		{
			//Shuffle down polygon resource index as needed.
			C_Polygon* pPoly = &m_lodLevels[iLodLevel].GetPolygon(jPoly);
			S16 mappingIndexToPossiblyShuffle = pPoly->GetBlindDataMappingIndex();
			if (mappingIndexToPossiblyShuffle > mappingIndex)
			{
				pPoly->ShuffleBlindDataMappingIndex();
			}
			else if (pPoly->GetBlindDataMappingIndex() == mappingIndex)
			{
				pPoly->SetBlindDataMappingIndex(-1);
			}
		}
	}

	C_SceneMeshRemappableNode::DeletePolygonBlindDataMapping(mappingIndex);
}

S16 C_SceneMeshNode::AddPolygonBlindDataMapping(const C_BlindDataResource* pResource, bool forceAddition)
{
	S16 newIndex = C_SceneMeshRemappableNode::AddPolygonBlindDataMapping(pResource, forceAddition);

	//Assert that all of our instances have the same number of entries in their mapping table.
	//TODO: Make sure that all instances have their mapping tables padded as needed.
	for (U32 i = 0; i < GetNumInstancesOfThisNode(); i++)
	{
		C_SceneNode* pNode = GetInstanceOfThisNode(i);
		if (pNode->HasType(E_ObjType_SceneMeshRemappable))
		{
			C_SceneMeshRemappableNode* pRemappableNode =
				(C_SceneMeshRemappableNode*)pNode;
			if (pRemappableNode->GetNumPolygonBlindDataMappings() < GetNumPolygonBlindDataMappings())
			{
				for (U32 i = pRemappableNode->GetNumPolygonBlindDataMappings(); i < GetNumPolygonBlindDataMappings(); i++)
				{
					//Copy new remapping entries.
					pRemappableNode->AddPolygonBlindDataMappingSetCount(GetPolygonBlindDataMappingPtr(i), 1);
				}
			}
			AFTK_ASSERT(pRemappableNode->GetNumPolygonBlindDataMappings() ==
				GetNumPolygonBlindDataMappings());
		}
	}

	return newIndex;
}

S16 C_SceneMeshNode::AddPolygonMaterialMapping(const C_MaterialResource* pResource, bool forceAddition)
{
	S16 newIndex = C_SceneMeshRemappableNode::AddPolygonMaterialMapping(pResource, forceAddition);

	//Assert that all of our instances have the same number of entries in their mapping table.
	//TODO: Make sure that all instances have their mapping tables padded as needed.
	for (U32 i = 0; i < GetNumInstancesOfThisNode(); i++)
	{
		C_SceneNode* pNode = GetInstanceOfThisNode(i);
		if (pNode->HasType(E_ObjType_SceneMeshRemappable))
		{
			C_SceneMeshRemappableNode* pRemappableNode =
				(C_SceneMeshRemappableNode*)pNode;
			if (pRemappableNode->GetNumPolygonMaterialMappings() < GetNumPolygonMaterialMappings())
			{
				for (U32 i = pRemappableNode->GetNumPolygonMaterialMappings(); i < GetNumPolygonMaterialMappings(); i++)
				{
					//Copy new remapping entries.
					pRemappableNode->AddPolygonMaterialMappingSetCount(GetPolygonMaterialMappingPtr(i), 1);
				}
			}
			AFTK_ASSERT(pRemappableNode->GetNumPolygonMaterialMappings() ==
				GetNumPolygonMaterialMappings());
		}
	}

	return newIndex;
}

void C_SceneMeshNode::DeletePolygonMaterialMapping(U16 mappingIndex)
{
	for (U32 iLodLevel = 0; iLodLevel < (U32)m_lodLevels.Size(); iLodLevel++)
	{
		//Shuffle polygons.
		for (U32 jPoly = 0; jPoly < (U32)m_lodLevels[iLodLevel].GetNumPolygons(); jPoly++)
		{
			//Shuffle down polygon resource index as needed.
			C_Polygon* pPoly = &m_lodLevels[iLodLevel].GetPolygon(jPoly);
			S16 mappingIndexToPossiblyShuffle = pPoly->GetMaterialMappingIndex();
			if (mappingIndexToPossiblyShuffle > mappingIndex)
			{
				pPoly->ShuffleMaterialMappingIndex();
			}
			else if (pPoly->GetMaterialMappingIndex() == mappingIndex)
			{
				pPoly->SetMaterialMappingIndex(-1);
			}
		}
	}

	C_SceneMeshRemappableNode::DeletePolygonMaterialMapping(mappingIndex);
}

C_SceneMeshNode::C_SceneMeshNode(const C_SceneMeshNode& rhs)
: C_SceneMeshRemappableNode(0)
{
	//Add the default cluster.
	OneTimeInit();
	*this = rhs;
}

const C_SceneMeshNode& C_SceneMeshNode::operator =(const C_SceneMeshBonedInstanceNode& rhs)
{
	// base-class asignment operator
	C_SceneMeshRemappableNode::operator=(rhs);

	ClearThis();

	//Count the number of expanded pvs.
	U32 numExpandedPVs = 0;
	for (U32 iLodLevel = 0; iLodLevel < rhs.GetNumLodLevels(); iLodLevel++)
	{
		for (U32 jPoly = 0; jPoly < rhs.GetNumPolygons(iLodLevel); jPoly++)
		{
			numExpandedPVs += rhs.GetPolygon(iLodLevel, jPoly).GetNumPolygonVertices();
		}
	}

	//Copy vertex data channel descriptions.
	for (U32 iChannel = 0; iChannel < rhs.GetNumVertexDataChannels(); iChannel++)
	{
		AddVertexDataChannelInfo(rhs.GetVertexDataChannelInfo(iChannel));
	}

	//Reserve memory for expanded pvs.
	SetPolygonVertexInitialCapacity(numExpandedPVs);
	for (U32 iLodLevel = 0; iLodLevel < rhs.GetNumLodLevels(); iLodLevel++)
	{
		for (U32 jPoly = 0; jPoly < rhs.GetNumPolygons(iLodLevel); jPoly++)
		{
			const C_Polygon* pPoly = &rhs.GetPolygon(iLodLevel, jPoly);
			for (U32 kPV = 0; kPV < pPoly->GetNumPolygonVertices(); kPV++)
			{
				U32 oldPVIndex = pPoly->GetPolygonVertexIndex(kPV);
				const C_PolygonVertex* pOldPV = &rhs.GetPolygonVertex(oldPVIndex);

				S32 positionIndex = AddVertexPosition(rhs.GetEvaluatedPolygonVertexPosition(oldPVIndex));
				S32 normalIndex = AddVertexNormal(rhs.GetEvaluatedPolygonVertexNormal(oldPVIndex));

   				S32 newPVIndex = AddPolygonVertex();
				
				C_PolygonVertex* pNewPV = &GetPolygonVertex(newPVIndex);
				pNewPV->SetPositionIndex(positionIndex);
				pNewPV->SetNormalIndex(normalIndex);

				//Copy texture coordinate indices.
				for (U32 lUVSet = 0; lUVSet < rhs.GetNumTextureCoordinateSets(); lUVSet++)
				{
					pNewPV->SetTextureCoordinateSetIndices(
						lUVSet,
						pOldPV->GetTextureCoordinateIndex(lUVSet),
						pOldPV->GetTangentIndex(lUVSet),
						pOldPV->GetBinormalIndex(lUVSet));
				}

				//Copy data channels across for pv.
				for (U32 lChannel = 0; lChannel < GetNumVertexDataChannels(); lChannel++)
				{
					const C_VertexDataChannelInfo& info = GetVertexDataChannelInfo(lChannel);
					pNewPV->SetData(lChannel, info.GetDataType(), pOldPV->GetDataPtr(lChannel));
				}
			}
		}
	}

//#pragma message("Todo: Poly cluster support for boned instances when copying.")
	/*
	for (U32 iPolyCluster = 0; iPolyCluster < rhs.GetNumPolyClusters(); iPolyCluster++)
	{
	}
	*/

	//Remap pv indices for polygons.
	U32 newPVIndex = 0;
	for (U32 iLodLevel = 0; iLodLevel < (U32)rhs.GetNumLodLevels(); iLodLevel++)
	{
		SetLodLevelInfo(iLodLevel, rhs.GetLodLevelInfo(iLodLevel));

		for (U32 jPoly = 0; jPoly < rhs.GetNumPolygons(iLodLevel); jPoly++)
		{
			const C_Polygon* pOldPoly = &rhs.GetPolygon(iLodLevel, jPoly);
			AddPolygon(iLodLevel, 0, *pOldPoly);
			C_Polygon* pNewPoly = &GetPolygon(iLodLevel, jPoly);

			for (U32 kPV = 0; kPV < pNewPoly->GetNumPolygonVertices(); kPV++)
			{
				pNewPoly->SetPolygonVertexIndex(kPV, newPVIndex);
				newPVIndex++;
			}
		}
	}

	// outer loop iterates thru UV sets
	m_textureCoordinateSets.Reserve(rhs.GetNumTextureCoordinateSets());
	for (U32 uvSet = 0; uvSet < rhs.GetNumTextureCoordinateSets(); uvSet++)
	{
		// inner loop iterates thru the UVs of this set
		SetTextureCoordinateSetName(uvSet, rhs.GetTextureCoordinateSetName(uvSet));
		for (U32 j = 0; j < rhs.GetNumTextureCoordinates(uvSet); j++)
			AddTextureCoordinate(uvSet, rhs.GetTextureCoordinate(uvSet, j));

		for (U32 j = 0; j < rhs.GetNumVertexTangents(uvSet); j++)
			AddVertexTangent(uvSet, rhs.GetVertexTangent(uvSet, j));

		for (U32 j = 0; j < rhs.GetNumVertexBinormals(uvSet); j++)
			AddVertexBinormal(uvSet, rhs.GetVertexBinormal(uvSet, j));
	}

	return *this;
}

const C_SceneMeshNode& C_SceneMeshNode::operator =(const C_SceneMeshNode& rhs)
{
	return Copy(rhs, 0);
}

C_LodLevel& C_SceneMeshNode::GetLodLevel(U32 lodLevel)
{
	AFTK_ASSERT(lodLevel < (U32)m_lodLevels.Size());
	return m_lodLevels[lodLevel];
}

const C_LodLevel& C_SceneMeshNode::GetLodLevel(U32 lodLevel) const
{
	AFTK_ASSERT(lodLevel < (U32)m_lodLevels.Size());
	return m_lodLevels[lodLevel];
}

C_Status C_SceneMeshNode::BuildInfluenceNodeList() const
{
	m_influenceNodeList.Clear();
	m_matrixPalette.Clear();

	for (U32 iInfluenceCluster = 0; iInfluenceCluster < (U32)m_influenceClusters.Size(); iInfluenceCluster++)
	{
		C_InfluenceClusterNode* pInfluenceCluster = m_influenceClusters[iInfluenceCluster];
		if (pInfluenceCluster->GetUsage() == AFTK_INFLUENCE_CLUSTER_USAGE_SKIN)
		{
			U32 numEnvelopes = pInfluenceCluster->GetNumEnvelopes();
			for (U32 envIndex = 0; envIndex < numEnvelopes; envIndex++)
			{
				C_WeightEnvelope* pEnvelope = &pInfluenceCluster->GetEnvelope(envIndex);
				C_SceneNode* pInfluenceNode = pEnvelope->GetInfluenceNodePtr();

				m_influenceNodeList.PushBack(pInfluenceNode);
			}
		}
	}

	//Allocate space for the matrix palette
	if (m_influenceNodeList.Size() > 0)
	{
		U32 numPaletteMatrices = m_influenceNodeList.Size();
		m_matrixPalette.Reserve(numPaletteMatrices);
		Matrix44 id;
		UtMaths::SetIdentity(&id);
		for (U32 i = 0; i < numPaletteMatrices; i++)
		{
			C_MatrixPaletteEntry paletteEntry;
			paletteEntry.matrix = id;
			paletteEntry.matrixInverseTranspose = id;
			m_matrixPalette.PushBack(paletteEntry);
		}
	}

	return C_Status::E_Status_Success;
}

C_Status C_SceneMeshNode::BuildVertexWeightList() const
{
	C_InfluenceClusterNode* pInfluenceCluster = 0;
	for (U32 iInfluenceCluster = 0; iInfluenceCluster < (U32)m_influenceClusters.Size(); iInfluenceCluster++)
	{
		pInfluenceCluster = m_influenceClusters[iInfluenceCluster];
		if (pInfluenceCluster)
		{
			if (pInfluenceCluster->GetUsage() != AFTK_INFLUENCE_CLUSTER_USAGE_SKIN)
			{
				//Skip irrelevant influence clusters
				continue;
			}

			//Rebuild vertex weight table.

			//Resize vertex weight table if needed.
			U32 numVPos = GetNumVertexPositions();
			if (m_vertexWeightsList.Size() != numVPos)
			{
				m_vertexWeightsList.Resize(numVPos);
			}

			//Loop thru all bones for each vertex, up to four bones per vertex
			for (U32 iEnvelope = 0; iEnvelope < pInfluenceCluster->GetNumEnvelopes(); iEnvelope++)
			{
				C_WeightEnvelope* pEnvelope = &pInfluenceCluster->GetEnvelope(iEnvelope);
				U32 boneIndex = GetInfluenceNodeIndexFromSceneNode(pEnvelope->GetInfluenceNodePtr());
				AFTK_ASSERT(boneIndex != -1);

				/*
				//Keep track of the matrix for this influence node via a pointer to it.
				const C_SceneNode* pInfluenceNode = pEnvelope->GetInfluenceNodePtr();
				if (!pInfluenceNode)
				{
					#ifdef _DEBUG
					AFTK_ASSERT(false);
					#endif
					break;
				}
*/
				if (pEnvelope->GetNumWeights() != 0)
				{
					//Check that we've got the correct weight format.
					if (pEnvelope->GetDataType() != E_GenericAttributeType_F32)
					{
						#ifdef _DEBUG
						AFTK_ASSERT(!("Invalid weight format."));
						#endif
						break;
					}

					//Copy the weighting information.
					for (U32 iWeight = 0; iWeight < pEnvelope->GetNumWeights(); iWeight++)
					{
						C_Weight<F32> weight;
						pEnvelope->GetWeight(iWeight, weight);
						AFTK_ASSERT(weight.GetIndex() != -1);
						
						//Set the correct weight value for the current bone
						AFTK_ASSERT(weight.GetIndex() < (S32)m_vertexPositions.Size());
						AFTK_ASSERT(weight.GetIndex() != -1);

						AFTK_ASSERT(weight.GetIndex() < m_vertexWeightsList.Size());
						m_vertexWeightsList[weight.GetIndex()].AddWeight(weight.GetWeight(), boneIndex);
					}

					//Fill any empty gaps in the weighting table.
					for (U32 iVPos = 0; iVPos < (U32)m_vertexWeightsList.Size(); iVPos++)
					{
						if (m_vertexWeightsList[iVPos].GetNumWeights() == 0)
						{
							//Create an empty bogus weight.
							m_vertexWeightsList[iVPos].AddWeight(0, 0);
						}
					}
				}
			}
		}
	}

	for (U32 i = 0; i < (U32)m_vertexWeightsList.Size(); i++)
	{
		m_vertexWeightsList[i].Normalise();
	}

	return C_Status::E_Status_Success;
}

S32 C_SceneMeshNode::GetInfluenceNodeIndexFromSceneNode(C_SceneNode* pSceneNode) const
{
	for (U32 i = 0; i < (U32)m_influenceNodeList.Size(); i++)
	{
		if (m_influenceNodeList[i] == pSceneNode)
		{
			return i;
		}
	}
	return -1;
}

void C_SceneMeshNode::EvaluatePolygonVertexPosition(
	U32 polyVertIndex,
	Position3& positionLocalResult) const
{
	const C_PolygonVertex& originalPV = GetPolygonVertex(polyVertIndex);
	
	S32 vPosIndex = originalPV.GetPositionIndex();
	AFTK_ASSERT(vPosIndex != -1);
	
	const Position3& positionLocalOriginal = GetVertexPosition(vPosIndex);

	if (m_vertexWeightsList.Size() == 0)
	{
		positionLocalResult = positionLocalOriginal;
		return;
	}

	const C_VertexWeight& weight = m_vertexWeightsList[vPosIndex];

	//Initialise variables to store our evaluated result.
	positionLocalResult[0] = 0;
	positionLocalResult[1] = 0;
	positionLocalResult[2] = 0;

	F32 lastWeight = 0.0f;
	S32 n = weight.GetNumWeights()-1;

	if (n > -1)
	{
		for(S32 iWeight = 0; iWeight < n; iWeight++)
		{
			F32 thisWeight = weight.GetWeight(iWeight);
			U32 matrixIndex = weight.GetInfluenceNodeIndex(iWeight);
			const Matrix44& thisMatrix = m_matrixPalette[matrixIndex].matrix;
			lastWeight += thisWeight;

			//Calculate position
			{
				Position3 tempPosition;
				tempPosition = Aftk::ApplyMatrix(
					thisMatrix, 
					positionLocalOriginal);

				positionLocalResult[0] += thisWeight * tempPosition[0];
				positionLocalResult[1] += thisWeight * tempPosition[1];
				positionLocalResult[2] += thisWeight * tempPosition[2];
			}
		}
	
		//Apply the last influence...
		{
			lastWeight = 1.0f - lastWeight;
			U32 matrixIndex = weight.GetInfluenceNodeIndex(n);
			const Matrix44& lastMatrix = m_matrixPalette[matrixIndex].matrix;

			//Calculate position
			{
				Position3 tempPosition;
				tempPosition = Aftk::ApplyMatrix(
					lastMatrix, 
					positionLocalOriginal);

				positionLocalResult[0] += lastWeight * tempPosition[0];
				positionLocalResult[1] += lastWeight * tempPosition[1];
				positionLocalResult[2] += lastWeight * tempPosition[2];
			}
		}
	}
}
		
void C_SceneMeshNode::EvaluatePolygonVertexPosition(
	U32 instance,
	const Matrix44& worldMatrix,
	Position3& positionResult) const
{
	EvaluatePolygonVertexPosition(
		instance, 
		positionResult);

	positionResult = Aftk::ApplyMatrix(worldMatrix, positionResult);
}

void C_SceneMeshNode::EvaluatePolygonVertexPositionAndNormal(
	U32 polyVertIndex,
	Position3& positionLocalResult,
	Normal3& normalLocalResult) const
{
	const C_PolygonVertex& originalPV =  GetPolygonVertex(polyVertIndex);
	
	S32 vPosIndex = originalPV.GetPositionIndex();
	AFTK_ASSERT(vPosIndex != -1);
	S32 vNormalIndex = originalPV.GetNormalIndex();
	AFTK_ASSERT(vNormalIndex != -1);
	
	const Position3& positionLocalOriginal = GetVertexPosition(vPosIndex);
	const Normal3& normalLocalOriginal = GetVertexNormal(vNormalIndex);

	if (m_vertexWeightsList.Size() == 0)
	{
		positionLocalResult = positionLocalOriginal;
		normalLocalResult = normalLocalOriginal;
		return;
	}

	const C_VertexWeight& weight = m_vertexWeightsList[vPosIndex];

	//Initialise variables to store our evaluated result.
	positionLocalResult[0] = 0;
	positionLocalResult[1] = 0;
	positionLocalResult[2] = 0;

	normalLocalResult[0] = 0;
	normalLocalResult[1] = 0;
	normalLocalResult[2] = 0;

	F32 lastWeight = 0.0f;
	S32 n = weight.GetNumWeights()-1;

	if (n > -1)
	{
		for(S32 iWeight = 0; iWeight < n; iWeight++)
		{
			F32 thisWeight = weight.GetWeight(iWeight);
			U32 matrixIndex = weight.GetInfluenceNodeIndex(iWeight);
			lastWeight += thisWeight;

			//Calculate position
			{
				Position3 tempPosition;
				tempPosition = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrix, 
					positionLocalOriginal);

				positionLocalResult[0] += thisWeight * tempPosition[0];
				positionLocalResult[1] += thisWeight * tempPosition[1];
				positionLocalResult[2] += thisWeight * tempPosition[2];
			}
			//Calculate normal
			{
				Normal3 tempNormal;
				if (m_normalInverseTranspose)
					tempNormal = Aftk::ApplyMatrixToNormal(
						m_matrixPalette[matrixIndex].matrixInverseTranspose, 
						normalLocalOriginal);
				else
					tempNormal = Aftk::ApplyMatrixToNormal(
						m_matrixPalette[matrixIndex].matrix, 
						normalLocalOriginal);

				Normalise(tempNormal);
		
				normalLocalResult[0] += thisWeight * tempNormal[0];
				normalLocalResult[1] += thisWeight * tempNormal[1];
				normalLocalResult[2] += thisWeight * tempNormal[2];
			}
		}
	
		//Apply the last influence...
		{
			lastWeight = 1.0f - lastWeight;
			U32 matrixIndex = weight.GetInfluenceNodeIndex(n);

			//Calculate position
			{
				Position3 tempPosition;
				tempPosition = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrix, 
					positionLocalOriginal);

				positionLocalResult[0] += lastWeight * tempPosition[0];
				positionLocalResult[1] += lastWeight * tempPosition[1];
				positionLocalResult[2] += lastWeight * tempPosition[2];
			}
			//Calculate normal
			{
				Normal3 tempNormal;
				if (m_normalInverseTranspose)
					tempNormal = Aftk::ApplyMatrixToNormal(
						m_matrixPalette[matrixIndex].matrixInverseTranspose,
						normalLocalOriginal);
				else
					tempNormal = Aftk::ApplyMatrixToNormal(
						m_matrixPalette[matrixIndex].matrix,
						normalLocalOriginal);

				Normalise(tempNormal);

				normalLocalResult[0] += lastWeight * tempNormal[0];
				normalLocalResult[1] += lastWeight * tempNormal[1];
				normalLocalResult[2] += lastWeight * tempNormal[2];
			}
		}
	}

	Normalise(normalLocalResult);
}
		
void C_SceneMeshNode::EvaluatePolygonVertexPositionAndNormal(
	U32 polyVertIndex,
	const Matrix44& worldMatrix,
	const Matrix44& worldmatrixInverseTranspose,
	Position3& positionResult,
	Normal3& normalResult) const
{
	EvaluatePolygonVertexPositionAndNormal(
		polyVertIndex, 
		positionResult, 
		normalResult);

	positionResult = Aftk::ApplyMatrix(worldMatrix, positionResult);
	if (m_normalInverseTranspose)
		normalResult = Aftk::ApplyMatrixToNormal(worldmatrixInverseTranspose, normalResult);
	else
		normalResult = Aftk::ApplyMatrixToNormal(worldMatrix, normalResult);
	Normalise(normalResult);
}

void C_SceneMeshNode::EvaluatePolygonVertexTangentAndBinormal(
	U32 instance,
	U32 set,
	Vector3F32& tangentLocalResult,
	Vector3F32& binormalLocalResult) const
{
	const C_PolygonVertex& originalPV = GetPolygonVertex(instance);
	
	S32 vTangentIndex = originalPV.GetTangentIndex(set);
	S32 vBinormalIndex = originalPV.GetBinormalIndex(set);
	if ((vTangentIndex == -1) || (vBinormalIndex == -1))
	{
		return;
	}
	
	const Vector3F32& tangentLocalOriginal = GetVertexTangent(set, vTangentIndex);
	const Vector3F32& binormalLocalOriginal = GetVertexBinormal(set, vBinormalIndex);

	if (m_vertexWeightsList.Size() == 0)
	{
		tangentLocalResult = tangentLocalOriginal;
		binormalLocalResult = binormalLocalOriginal;
		return;
	}

	const C_VertexWeight& weight = m_vertexWeightsList[originalPV.GetPositionIndex()];

	tangentLocalResult[0] = 0;
	tangentLocalResult[1] = 0;
	tangentLocalResult[2] = 0;

	binormalLocalResult[0] = 0;
	binormalLocalResult[1] = 0;
	binormalLocalResult[2] = 0;

	F32 lastWeight = 0.0f;
	S32 n = weight.GetNumWeights()-1;

	if (n > -1)
	{
		for(S32 iWeight = 0; iWeight < n; iWeight++)
		{
			F32 thisWeight = weight.GetWeight(iWeight);
			U32 matrixIndex = weight.GetInfluenceNodeIndex(iWeight);
			lastWeight += thisWeight;

			//Calculate tangent
			{
				Vector3F32 tempTangent;
				tempTangent = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrixInverseTranspose, 
					tangentLocalOriginal);

				tangentLocalResult[0] += thisWeight * tempTangent[0];
				tangentLocalResult[1] += thisWeight * tempTangent[1];
				tangentLocalResult[2] += thisWeight * tempTangent[2];
			}
			//Calculate binormal
			{
				Vector3F32 tempBinormal;
				tempBinormal = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrixInverseTranspose, 
					binormalLocalOriginal);
				
				binormalLocalResult[0] += thisWeight * tempBinormal[0];
				binormalLocalResult[1] += thisWeight * tempBinormal[1];
				binormalLocalResult[2] += thisWeight * tempBinormal[2];
			}
		}
	
		//Apply the last influence...
		{
			lastWeight = 1.0f - lastWeight;
			U32 matrixIndex = weight.GetInfluenceNodeIndex(n);

			//Calculate tangent
			{
				Vector3F32 tempTangent;
				tempTangent = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrixInverseTranspose,
					tangentLocalOriginal);
			
				tangentLocalResult[0] += lastWeight * tempTangent[0];
				tangentLocalResult[1] += lastWeight * tempTangent[1];
				tangentLocalResult[2] += lastWeight * tempTangent[2];
			}
			//Calculate binormal
			{
				Vector3F32 tempBinormal;
				tempBinormal = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrixInverseTranspose, 
					binormalLocalOriginal);
			
				binormalLocalResult[0] += lastWeight * tempBinormal[0];
				binormalLocalResult[1] += lastWeight * tempBinormal[1];
				binormalLocalResult[2] += lastWeight * tempBinormal[2];
			}
		}
	}
}

void C_SceneMeshNode::EvaluatePolygonVertexTangentAndBinormal(
	U32 instance,
	const Matrix44& worldmatrixInverseTranspose,
	U32 set,
	Vector3F32& tangentResult,
	Vector3F32& binormalResult) const
{
	EvaluatePolygonVertexTangentAndBinormal(
		instance, 
		set, 
		tangentResult,
		binormalResult);

	tangentResult = Aftk::ApplyMatrix(worldmatrixInverseTranspose, tangentResult);
	binormalResult = Aftk::ApplyMatrix(worldmatrixInverseTranspose, binormalResult);
}

void C_SceneMeshNode::EvaluatePolygonVertexPositionTangentAndBinormal(
	U32 instance,
	U32 set,
	Position3& positionLocalResult,
	Vector3F32& tangentLocalResult,
	Vector3F32& binormalLocalResult) const
{
	const C_PolygonVertex& originalPV = GetPolygonVertex(instance);
	
	S32 vPosIndex = originalPV.GetPositionIndex();
	AFTK_ASSERT(vPosIndex != -1);
	S32 vTangentIndex = originalPV.GetTangentIndex(set);
	S32 vBinormalIndex = originalPV.GetBinormalIndex(set);
	if ((vTangentIndex == -1) || (vBinormalIndex == -1))
	{
		return;
	}
	
	const Position3& positionLocalOriginal = GetVertexPosition(vPosIndex);
	const Vector3F32& tangentLocalOriginal = GetVertexTangent(set, vTangentIndex);
	const Vector3F32& binormalLocalOriginal = GetVertexBinormal(set, vBinormalIndex);

	if (m_vertexWeightsList.Size() == 0)
	{
		positionLocalResult = positionLocalOriginal;
		tangentLocalResult = tangentLocalOriginal;
		binormalLocalResult = binormalLocalOriginal;
		return;
	}

	const C_VertexWeight& weight = m_vertexWeightsList[vPosIndex];

	//Initialise variables to store our evaluated result.
	positionLocalResult[0] = 0;
	positionLocalResult[1] = 0;
	positionLocalResult[2] = 0;

	tangentLocalResult[0] = 0;
	tangentLocalResult[1] = 0;
	tangentLocalResult[2] = 0;

	binormalLocalResult[0] = 0;
	binormalLocalResult[1] = 0;
	binormalLocalResult[2] = 0;

	F32 lastWeight = 0.0f;
	S32 n = weight.GetNumWeights()-1;

	if (n > -1)
	{
		for(S32 iWeight = 0; iWeight < n; iWeight++)
		{
			F32 thisWeight = weight.GetWeight(iWeight);
			U32 matrixIndex = weight.GetInfluenceNodeIndex(iWeight);
			lastWeight += thisWeight;

			//Calculate position
			{
				Position3 tempPosition;
				tempPosition = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrix, 
					positionLocalOriginal);

				positionLocalResult[0] += thisWeight * tempPosition[0];
				positionLocalResult[1] += thisWeight * tempPosition[1];
				positionLocalResult[2] += thisWeight * tempPosition[2];
			}
			//Calculate tangent
			{
				Vector3F32 tempTangent;
				tempTangent =Aftk:: ApplyMatrix(
					m_matrixPalette[matrixIndex].matrixInverseTranspose, 
					tangentLocalOriginal);

				tangentLocalResult[0] += thisWeight * tempTangent[0];
				tangentLocalResult[1] += thisWeight * tempTangent[1];
				tangentLocalResult[2] += thisWeight * tempTangent[2];
			}
			//Calculate binormal
			{
				Vector3F32 tempBinormal;
				tempBinormal = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrixInverseTranspose, 
					binormalLocalOriginal);

				binormalLocalResult[0] += thisWeight * tempBinormal[0];
				binormalLocalResult[1] += thisWeight * tempBinormal[1];
				binormalLocalResult[2] += thisWeight * tempBinormal[2];
			}
		}
	
		//Apply the last influence...
		{
			lastWeight = 1.0f - lastWeight;
			U32 matrixIndex = weight.GetInfluenceNodeIndex(n);

			//Calculate position
			{
				Position3 tempPosition;
				tempPosition = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrix, 
					positionLocalOriginal);

				positionLocalResult[0] += lastWeight * tempPosition[0];
				positionLocalResult[1] += lastWeight * tempPosition[1];
				positionLocalResult[2] += lastWeight * tempPosition[2];
			}
			//Calculate tangent
			{
				Vector3F32 tempTangent;
				tempTangent = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrixInverseTranspose,
					tangentLocalOriginal);
			
				tangentLocalResult[0] += lastWeight * tempTangent[0];
				tangentLocalResult[1] += lastWeight * tempTangent[1];
				tangentLocalResult[2] += lastWeight * tempTangent[2];
			}
			//Calculate binormal
			{
				Vector3F32 tempBinormal;
				tempBinormal = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrixInverseTranspose, 
					binormalLocalOriginal);
			
				binormalLocalResult[0] += lastWeight * tempBinormal[0];
				binormalLocalResult[1] += lastWeight * tempBinormal[1];
				binormalLocalResult[2] += lastWeight * tempBinormal[2];
			}
		}
	}
}
		
void C_SceneMeshNode::EvaluatePolygonVertexPositionTangentAndBinormal(
	U32 instance,
	const Matrix44& worldMatrix,
	const Matrix44& worldmatrixInverseTranspose,
	U32 set,
	Position3& positionResult,
	Vector3F32& tangentResult,
	Vector3F32& binormalResult) const
{
	EvaluatePolygonVertexPositionTangentAndBinormal(
		instance, 
		set, 
		positionResult, 
		tangentResult,
		binormalResult);

	positionResult = Aftk::ApplyMatrix(worldMatrix, positionResult);
	tangentResult = Aftk::ApplyMatrix(worldmatrixInverseTranspose, tangentResult);
	binormalResult = Aftk::ApplyMatrix(worldmatrixInverseTranspose, binormalResult);
}

void C_SceneMeshNode::PadPolygonVertexTextureCoordinateSetIndices(C_PolygonVertex* pPolyVert)
{
	//Make sure we have enough storage for the data on the poly vertex.
	if (pPolyVert->GetNumTextureCoordinateIndices() >= GetNumTextureCoordinateSets())
		return; // No need to pad

	for (U32 iSet = pPolyVert->GetNumTextureCoordinateIndices(); iSet < GetNumTextureCoordinateSets(); iSet++)
	{
		pPolyVert->SetTextureCoordinateSetIndices(iSet, -1, -1, -1);
	}
}

void C_SceneMeshNode::ClearGeometry()
{
	DirtyGeometryDependantCaches();

	//Clear polygon clusters
	for (U32 i = 1; i < (U32)m_polygonClusters.Size(); i++)
	{
		delete m_polygonClusters[i];
	}
	//Remove clusters from array.
	U32 toEraseCount = (U32)m_polygonClusters.Size();
	for (U32 i = 1; i < toEraseCount; i++)
	{
		m_polygonClusters.PopBack();
	}
	m_polygonClusters[0]->Clear();
	m_polygonClusters[0]->SetName("default");

	m_adjacencyList.Clear();
	m_vertexPositions.Clear();
	m_vertexNormals.Clear();
	m_textureCoordinateSets.Clear();
	m_polygonVertices.Clear();

		U32 numPolys = 0;
		for (U32 iLod = 0; iLod < (U32)m_lodLevels.Size(); iLod++)
		{
			for (U32 jPoly = 0; jPoly < (U32)m_lodLevels[iLod].GetNumPolygons(); jPoly++)
			{
				C_Polygon* pPoly = &m_lodLevels[iLod].GetPolygon(jPoly);
				pPoly->Clear();
				numPolys++;
			}
		}

	//A cunning trick to _really_ erase the contents of the polygon list.
	{
		Ut::UtVector< Ut::DynamicVector<C_LodLevel> > emptyLodLevelArray;
		Ut::MoveRaw(m_lodLevels, emptyLodLevelArray);
	}

	m_vertexDataChannelInfo.Clear();
}

void C_SceneMeshNode::DeleteVertexDataChannels()
{
	for (U32 i = 0; i < (U32)m_polygonVertices.Size(); i++)
		m_polygonVertices[i].DeleteData();

	m_vertexDataChannelInfo.Clear();
}

void C_SceneMeshNode::DeleteTextureCoordinateSets()
{
	for (U32 i = 0; i < (U32)m_polygonVertices.Size(); i++)
		m_polygonVertices[i].DeleteTextureCoordinateIndices();

	m_textureCoordinateSets.Clear();
}

void C_SceneMeshNode::DeleteMaterials()
{
	for (U32 iLod = 0; iLod < GetNumLodLevels(); iLod++)
	{
		for (U32 jPoly = 0; jPoly < GetNumPolygons(iLod); jPoly++)
		{
			C_Polygon* pPoly = &GetPolygon(iLod, jPoly);
			pPoly->SetMaterialMappingIndex(-1);
		}
	}
	DeleteUnusedPolygonMaterialMappings();

	for (U32 i = 0; i < GetNumInstancesOfThisNode(); i++)
	{
		C_SceneNode* pInst = GetInstanceOfThisNode(i);
		if (pInst->HasType(E_ObjType_SceneMeshRemappedInstance))
		{
			C_SceneMeshRemappedInstanceNode* pInstRemapped = 
				(C_SceneMeshRemappedInstanceNode*)pInst;

			//Sync mapping table.
			pInstRemapped->CopyPolygonMaterialMappings(this, 0);
		}
	}

//	AFTK_ASSERT(m_polygonMaterialMappingArray.Size() == 0);
}

void C_SceneMeshNode::DeleteBlindData()
{
	for (U32 iLod = 0; iLod < GetNumLodLevels(); iLod++)
	{
		for (U32 jPoly = 0; jPoly < GetNumPolygons(iLod); jPoly++)
		{
			C_Polygon* pPoly = &GetPolygon(iLod, jPoly);
			pPoly->SetBlindDataMappingIndex(-1);
		}
	}
	DeleteUnusedPolygonBlindDataMappings();

	for (U32 i = 0; i < GetNumInstancesOfThisNode(); i++)
	{
		C_SceneNode* pInst = GetInstanceOfThisNode(i);
		if (pInst->HasType(E_ObjType_SceneMeshRemappedInstance))
		{
			C_SceneMeshRemappedInstanceNode* pInstRemapped = 
				(C_SceneMeshRemappedInstanceNode*)pInst;

			//Sync mapping table.
			pInstRemapped->CopyPolygonBlindDataMappings(this);
		}
	}

	AFTK_ASSERT(m_polygonBlindDataMappingArray.Size() == 0);
}

void C_SceneMeshNode::PadPolygonVertexDataChannels(C_PolygonVertex* pPolyVert)
{
	//Make sure we have enough storage for the data on the poly vertex.
	AFTK_ASSERT(pPolyVert->GetNumData() < GetNumVertexDataChannels());

	//Pad out arbitrary data, if needed
	for (U32 iChannel = pPolyVert->GetNumData(); iChannel < GetNumVertexDataChannels(); iChannel++)
	{
		C_VertexDataChannelInfo info = GetVertexDataChannelInfo(iChannel);
		switch(info.GetDataType())
		{
		case E_GenericAttributeType_U8:
			pPolyVert->SetData(iChannel, U8(0));
			break;
		case E_GenericAttributeType_U16:
			pPolyVert->SetData(iChannel,U16(0));
			break;
		case E_GenericAttributeType_U32:
			pPolyVert->SetData(iChannel,U32(0));
			break;
		case E_GenericAttributeType_U64:
			pPolyVert->SetData(iChannel,U64(0));
			break;
		case E_GenericAttributeType_S8:
			pPolyVert->SetData(iChannel,S8(0));
			break;
		case E_GenericAttributeType_S16:
			pPolyVert->SetData(iChannel,S16(0));
			break;
		case E_GenericAttributeType_S32:
			pPolyVert->SetData(iChannel,S32(0));
			break;
		case E_GenericAttributeType_S64:
			pPolyVert->SetData(iChannel,S64(0));
			break;
		case E_GenericAttributeType_F32:
			pPolyVert->SetData(iChannel,F32(0));
			break;
		case E_GenericAttributeType_F64:
			pPolyVert->SetData(iChannel,F64(0));
			break;
		case E_GenericAttributeType_Vector4F32:
			pPolyVert->SetData(iChannel,Vector4F32(0,0,0,0));
			break;
		case E_GenericAttributeType_Vector3F32:
			pPolyVert->SetData(iChannel,Vector3F32(0,0,0));
			break;
		case E_GenericAttributeType_Vector2F32:
			pPolyVert->SetData(iChannel,Vector2F32(0,0));
			break;
		case E_GenericAttributeType_Matrix44F32:
		{
			Matrix44 matrix;
			UtMaths::SetIdentity(&matrix);
			pPolyVert->SetData(iChannel,matrix);
			break;
		}
		case E_GenericAttributeType_Boolean:
			pPolyVert->SetData(iChannel,bool(false));
			break;
		case E_GenericAttributeType_Vector2F64:
			pPolyVert->SetData(iChannel,Vector2F64(0,0));
			break;
		case E_GenericAttributeType_Vector2S32:
			pPolyVert->SetData(iChannel,Vector2S32(0,0));
			break;
		case E_GenericAttributeType_Vector2S16:
			pPolyVert->SetData(iChannel,Vector2S16(0,0));
			break;
		case E_GenericAttributeType_Vector2S8:
			pPolyVert->SetData(iChannel,Vector2S8(0,0));
			break;
		case E_GenericAttributeType_Vector2U32:
			pPolyVert->SetData(iChannel,Vector2U32(0,0));
			break;
		case E_GenericAttributeType_Vector2U16:
			pPolyVert->SetData(iChannel,Vector2U16(0,0));
			break;
		case E_GenericAttributeType_Vector2U8:
			pPolyVert->SetData(iChannel,Vector2U8(0,0));
			break;
		case E_GenericAttributeType_Vector3F64:
			pPolyVert->SetData(iChannel,Vector3F64(0,0,0));
			break;
		case E_GenericAttributeType_Vector3S32:
			pPolyVert->SetData(iChannel,Vector3S32(0,0,0));
			break;
		case E_GenericAttributeType_Vector3S16:
			pPolyVert->SetData(iChannel,Vector3S16(0,0,0));
			break;
		case E_GenericAttributeType_Vector3S8:
			pPolyVert->SetData(iChannel,Vector3S8(0,0,0));
			break;
		case E_GenericAttributeType_Vector3U32:
			pPolyVert->SetData(iChannel,Vector3U32(0,0,0));
			break;
		case E_GenericAttributeType_Vector3U16:
			pPolyVert->SetData(iChannel,Vector3U16(0,0,0));
			break;
		case E_GenericAttributeType_Vector3U8:
			pPolyVert->SetData(iChannel,Vector3U8(0,0,0));
			break;
		case E_GenericAttributeType_Vector4F64:
			pPolyVert->SetData(iChannel,Vector4F64(0,0,0,0));
			break;
		case E_GenericAttributeType_Vector4S32:
			pPolyVert->SetData(iChannel,Vector4S32(0,0,0,0));
			break;
		case E_GenericAttributeType_Vector4S16:
			pPolyVert->SetData(iChannel,Vector4S16(0,0,0,0));
			break;
		case E_GenericAttributeType_Vector4S8:
			pPolyVert->SetData(iChannel,Vector4S8(0,0,0,0));
			break;
		case E_GenericAttributeType_Vector4U32:
			pPolyVert->SetData(iChannel,Vector4U32(0,0,0,0));
			break;
		case E_GenericAttributeType_Vector4U16:
			pPolyVert->SetData(iChannel,Vector4U16(0,0,0,0));
			break;
		case E_GenericAttributeType_Vector4U8:
			pPolyVert->SetData(iChannel,Vector4U8(0,0,0,0));
			break;
			case E_GenericAttributeType_Vector4U64:
			pPolyVert->SetData(iChannel,Vector4U64(0,0,0,0));
			break;
		case E_GenericAttributeType_Vector4S64:
			pPolyVert->SetData(iChannel,Vector4S64(0,0,0,0));
			break;
		case E_GenericAttributeType_Vector3U64:
			pPolyVert->SetData(iChannel,Vector3U64(0,0,0));
			break;
		case E_GenericAttributeType_Vector3S64:
			pPolyVert->SetData(iChannel,Vector3S64(0,0,0));
			break;
		case E_GenericAttributeType_Vector2U64:
			pPolyVert->SetData(iChannel,Vector2U64(0,0));
			break;
		case E_GenericAttributeType_Vector2S64:
			pPolyVert->SetData(iChannel,Vector2S64(0,0));
			break;

		default:
			//Unknown data type.
			AFTK_DO_BEGIN << "Unknown data type for [" << m_name << "]\n" AFTK_DO_DEBUG_END;
			AFTK_ASSERT(!("Unknown data channel type. Probably set to NULL."));
			break;
		}
	}
}

bool C_SceneMeshNode::IsRemappedSource() const
{
	return true;
}

S32 C_SceneMeshNode::AddPolygonNoCluster(U32 lodLevel, const Aftk::C_Polygon& polygon)
{
	DirtyGeometryDependantCaches();
	AFTK_RESIZE_LOD_LEVELS_IF_NEEDED;

	//Store the polygon
	S32 newPolyIndex = m_lodLevels[lodLevel].AddPolygon(polygon);
	if (newPolyIndex == -1)
	{
		return -1;
	}
	m_lodLevels[lodLevel].GetPolygon(newPolyIndex).AttachToMesh(this);

	return (S32)newPolyIndex;
}

void C_SceneMeshNode::OneTimeInit()
{
	//Add the default cluster.
	C_PolygonCluster* pPolyCluster = new C_PolygonCluster(this);
//	pPolyCluster->AttachToMesh(this);
	m_polygonClusters.PushBack(pPolyCluster);

	//Cache last evaluation of postions, normals, tangents and binormals.
	m_pPolygonVertexPositionCache = 0;
	m_pPolygonVertexNormalCache = 0;
	//Cache tangents and binormals per UV set.
	m_ppPolygonVertexTangentCache = 0;
	m_ppPolygonVertexBinormalCache = 0;
	m_animationHelpersBuilt = E_AnimHelperState_NotBuilt;
}

void C_SceneMeshNode::ClearThis()
{
	ClearGeometry();
}

U32 C_SceneMeshNode::RepairPolyClusters()
{
	U32 fixCount = 0;

	for (U32 iLodLevel = 0; iLodLevel < GetNumLodLevels(); iLodLevel++)
	{
		for (U32 jPoly = 0; jPoly < GetNumPolygons(iLodLevel); jPoly++)
		{
			const C_Polygon* pPoly = &GetPolygon(iLodLevel, jPoly);
			if (!pPoly->GetClusterPtr())
			{
				//Orphaned - add to cluster.
				m_polygonClusters[0]->AddPolygonIndex(iLodLevel, jPoly);
				fixCount++;
			}
		}
	}

	return fixCount;
}

bool C_SceneMeshNode::operator== (const C_SceneMeshNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneMeshNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_SceneMeshNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	if (classToAbortTest == E_ObjType_SceneMesh)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneMeshRemappableNode::IsEqualToAll(rhs, classToAbortTest, flags));
}

bool C_SceneMeshNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	//First check numbers of components as trivial rejection, before
	//testing the component values

	if (!_rhs.HasType(E_ObjType_SceneMesh))
	{
		return false;
	}

	C_SceneMeshNode& rhs = (C_SceneMeshNode&) _rhs;

	if ((GetNumVertexPositions() != rhs.GetNumVertexPositions()) ||
	(GetNumVertexNormals()!=  rhs.GetNumVertexNormals()) ||
	(GetNumVertexWeights() !=  rhs.GetNumVertexWeights()) ||
	(GetNumVertexDataChannels() !=  rhs.GetNumVertexDataChannels()) ||
	(GetNumVertexData() !=  rhs.GetNumVertexData()) ||
	(GetNumPolygonVertices() !=  rhs.GetNumPolygonVertices()) ||
	(GetNumLodLevels() !=  rhs.GetNumLodLevels()) ||
	(GetNumPolygonBlindDataMappings() !=  rhs.GetNumPolygonBlindDataMappings()) ||
	(GetNumPolygonClusters() !=  rhs.GetNumPolygonClusters()) ||
	(GetNumPolygonMaterialMappings() !=  rhs.GetNumPolygonMaterialMappings()) ||
	(GetNumTextureCoordinateSets() !=  rhs.GetNumTextureCoordinateSets()))
	{
		return false;
	}

	const U32 nLodLevels = GetNumLodLevels();

	for (U32 lod = 0; lod < nLodLevels; lod++)
	{
		if ((GetNumMaterialNodes(lod) !=  rhs.GetNumMaterialNodes(lod)) ||
			(GetNumPolygons(lod) !=  rhs.GetNumPolygons(lod)))
		{
			return false;
		}
	}

	const U32 nSets = GetNumTextureCoordinateSets();

	for (U32 set = 0; set < nSets; set++)
	{
		if ((GetNumVertexBinormals(set) !=  rhs.GetNumVertexBinormals(set)) ||
			(GetNumVertexTangents(set) !=  rhs.GetNumVertexTangents(set)) ||
			(GetNumTextureCoordinates(set) !=  rhs.GetNumTextureCoordinates(set)))
		{
			return false;
		}
	}

	//If we get to here, then we need to test the actual components are the same
	//We can just memcmp or vector test the simple components:

	if (m_vertexPositions.Size() > 0)
	{
		if (memcmp(&m_vertexPositions[0], &rhs.m_vertexPositions[0], sizeof(Position3)*m_vertexPositions.Size()) != 0)
			return false;
	}
	if (m_vertexNormals.Size() > 0)
	{
		if (memcmp(&m_vertexNormals[0], &rhs.m_vertexNormals[0], sizeof(Normal3)*m_vertexNormals.Size()) != 0)
            return false;
	}
	
	//The more complex components are tested through equality overloads

	if ((!EqualityTestVector(m_lodLevels, rhs.m_lodLevels)) || 
		(!EqualityTestVector(m_textureCoordinateSets, rhs.m_textureCoordinateSets)) ||
		(!EqualityTestVector(m_vertexDataChannelInfo, rhs.m_vertexDataChannelInfo)) ||
		(!EqualityTestVector(m_polygonVertices, rhs.m_polygonVertices)))
	{
		return false;
	}

	//Check polygon clusters by dereferencing them first

	const U32 nClusters = GetNumPolygonClusters();

	for (U32 cluster = 0; cluster < nClusters; cluster++)
	{
		if (!(*GetPolygonClusterPtr(cluster) == *rhs.GetPolygonClusterPtr(cluster)))
		{
			return false;
		}
	}

	return true;
}

U32 C_SceneMeshNode::GetHashOfThis() const
{
	U32 hash = 1;

	U32 nComponents = 0;
	if (nComponents = GetNumPolygonVertices())
	{
		hash*=nComponents;
	}
	if (nComponents = GetNumTextureCoordinateSets())
	{
		hash*=nComponents;
		if (nComponents = GetNumTextureCoordinates(0))
		{
			hash*=nComponents;
		}
	}
	if (nComponents = GetNumVertexNormals())
	{
		hash*=nComponents;
	}
	if (nComponents = (U32)GetNumPolygonMaterialMappings())
	{
		hash*=nComponents;
	}
	if (nComponents = GetNumVertexDataChannels())
	{
		hash*=nComponents;
		if (nComponents = GetNumVertexData())
		{
			hash*=nComponents;
		}
	}

	return hash;
}

U32 C_SceneMeshNode::GetHash(E_ObjType highestClassToTest) const
{
	if (highestClassToTest = GetType())
		return C_SceneMeshNode::GetHashOfThis();
	return C_SceneMeshNode::GetHashOfThis()*C_SceneMeshRemappableNode::GetHash(highestClassToTest);
}
#endif
