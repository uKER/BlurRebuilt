#include <stdafx.h>

using namespace Aftk;

C_SceneMeshRemappedInstanceNode::C_SceneMeshRemappedInstanceNode(
	C_Scene* pScene)
	:
	C_SceneMeshRemappableNode(pScene)
{
	OneTimeInit();
}

C_Node* C_SceneMeshRemappedInstanceNode::Clone(C_Scene* pScene) const
{
	C_SceneMeshRemappedInstanceNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneMeshRemappedInstanceNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneMeshRemappedInstanceNode(pScene);
		(C_SceneMeshRemappedInstanceNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_Node* C_SceneMeshRemappedInstanceNode::Clone(C_Scene* pScene, const MaterialRemappingHashTable& mappingTable) const
{
	C_SceneMeshRemappedInstanceNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneMeshRemappedInstanceNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneMeshRemappedInstanceNode(pScene);
		(C_SceneMeshRemappedInstanceNode&)(*pNewNode).Copy(*this, &mappingTable);
	}

	return pNewNode;
}

const C_SceneMeshRemappedInstanceNode& C_SceneMeshRemappedInstanceNode::Copy(const C_SceneMeshRemappedInstanceNode& rhs, const MaterialRemappingHashTable* pMappingTable)
{
	C_SceneMeshRemappableNode::Copy(rhs, pMappingTable);

	ClearThis();

	if (rhs.m_pScene == m_pScene)
	{
		if (m_pSourceNode != rhs.m_pSourceNode)
		{
			//Remove association with previous instance source.
			if (m_pSourceNode)
			{
				m_pSourceNode->DeleteInstanceOfThisNode(this);
			}
			m_pSourceNode = rhs.m_pSourceNode;
			//Add association with new instance source.
			if (m_pSourceNode)
			{
				m_pSourceNode->AddInstanceOfThisNode(this);
			}
		}
	}
	else
	{
		//Remove association with previous instance source.
		if (m_pSourceNode)
		{
			m_pSourceNode->DeleteInstanceOfThisNode(this);
		}
		m_pSourceNode = 0;
	}

	return *this;
}

C_SceneMeshRemappedInstanceNode::C_SceneMeshRemappedInstanceNode(
	const C_SceneMeshRemappedInstanceNode& rhs)
:
C_SceneMeshRemappableNode(0)
{
	OneTimeInit();
	*this = rhs;
}

const C_SceneMeshRemappedInstanceNode& C_SceneMeshRemappedInstanceNode::operator=(
	const C_SceneMeshRemappedInstanceNode& rhs)
{
	return Copy(rhs, 0);
}

C_SceneMeshRemappedInstanceNode::~C_SceneMeshRemappedInstanceNode()
{
	//Remove association with instance source here.
	if (m_pSourceNode)
		SetSourceNodePtr(0);
}

S32	C_SceneMeshRemappedInstanceNode::GetTextureCoordinateSetIndex(const std::string& name) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetTextureCoordinateSetIndex(name);
}

const std::string& C_SceneMeshRemappedInstanceNode::GetTextureCoordinateSetName(U32 set) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetTextureCoordinateSetName(set);
}

const C_VertexDataChannelInfo& C_SceneMeshRemappedInstanceNode::GetVertexDataChannelInfo(U32 channel) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetVertexDataChannelInfo(channel);
}
	
const Aftk::C_Polygon& C_SceneMeshRemappedInstanceNode::GetPolygon(U32 lodLevel, U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygon(lodLevel, instance);
}

const Aftk::C_Polygon& C_SceneMeshRemappedInstanceNode::GetPolygon(U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygon(instance);
}

const TextureCoordinate4& C_SceneMeshRemappedInstanceNode::GetTextureCoordinate(U32 set, U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetTextureCoordinate(set, instance);
}

const Vector3F32& C_SceneMeshRemappedInstanceNode::GetVertexTangent(U32 set, U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetVertexTangent(set, instance);
}

const Vector3F32& C_SceneMeshRemappedInstanceNode::GetVertexBinormal(U32 set, U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetVertexBinormal(set, instance);
}

const C_PolygonVertex& C_SceneMeshRemappedInstanceNode::GetPolygonVertex(U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonVertex(instance);
}

const C_PolygonCluster* C_SceneMeshRemappedInstanceNode::GetPolygonClusterPtr(U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonClusterPtr(instance);
}

U32 C_SceneMeshRemappedInstanceNode::GetNumVertexPositions() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexPositions();
}

U32	C_SceneMeshRemappedInstanceNode::GetNumVertexNormals() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexNormals();
}

U32	C_SceneMeshRemappedInstanceNode::GetNumVertexTangents(U32 set) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexTangents(set);
}

U32	C_SceneMeshRemappedInstanceNode::GetNumVertexBinormals(U32 set) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexBinormals(set);
}

U32	C_SceneMeshRemappedInstanceNode::GetNumVertexData() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexData();
}

U32	C_SceneMeshRemappedInstanceNode::GetNumTextureCoordinates(U32 set) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumTextureCoordinates(set);
}

U32	C_SceneMeshRemappedInstanceNode::GetNumTextureCoordinateSets() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumTextureCoordinateSets();
}

U32	C_SceneMeshRemappedInstanceNode::GetNumVertexDataChannels() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexDataChannels();
}

const C_MeshLodLevelInfo& C_SceneMeshRemappedInstanceNode::GetLodLevelInfo(U32 lodLevel) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetLodLevelInfo(lodLevel);
}

C_MeshLodLevelInfo& C_SceneMeshRemappedInstanceNode::GetLodLevelInfo(U32 lodLevel)
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetLodLevelInfo(lodLevel);
}

U32	C_SceneMeshRemappedInstanceNode::GetNumPolygonVertices() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumPolygonVertices();
}

U32	C_SceneMeshRemappedInstanceNode::GetNumLodLevels() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumLodLevels();
}

U32	C_SceneMeshRemappedInstanceNode::GetNumPolygons(U32 lodLevel) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumPolygons(lodLevel);
}

U32	C_SceneMeshRemappedInstanceNode::GetNumTriangles(U32 lodLevel) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumTriangles(lodLevel);
}

U32	C_SceneMeshRemappedInstanceNode::GetNumMaterialNodes(U32 lodLevel) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumMaterialNodes(lodLevel);
}

U32 C_SceneMeshRemappedInstanceNode::GetNumPolygonClusters() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumPolygonClusters();
}

E_ObjType C_SceneMeshRemappedInstanceNode::GetType() const
{
	return E_ObjType_SceneMeshRemappedInstance;
}

bool C_SceneMeshRemappedInstanceNode::HasType(E_ObjType type) const
{
	if (C_SceneMeshRemappableNode::HasType(type))
		return true;
	if (type == C_SceneMeshRemappedInstanceNode::GetType()) 
		return true;

	return false;
}

C_Status C_SceneMeshRemappedInstanceNode::BuildAnimationHelpers() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->BuildAnimationHelpers();
}

void C_SceneMeshRemappedInstanceNode::ClearAnimationHelpers() const
{
	//Don't do anything.
}

bool C_SceneMeshRemappedInstanceNode::IsAdjacencyBuilt() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->IsAdjacencyBuilt();
}

const U32* C_SceneMeshRemappedInstanceNode::GetAdjacencyList(U32 iThisPoly) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetAdjacencyList(iThisPoly);
}

U32 C_SceneMeshRemappedInstanceNode::Repair()
{
	return 0;
}

U32 C_SceneMeshRemappedInstanceNode::Validate() const
{
	C_SceneMeshRemappedInstanceNode* pThis = const_cast<C_SceneMeshRemappedInstanceNode*>(this);
	return pThis->ProcessProblems(false);
}

U32 C_SceneMeshRemappedInstanceNode::ProcessProblems(bool fix) 
{
	U32 error = 0;
	error += C_SceneNode::Validate();
	if (!m_pSourceNode)
	{
		AFTK_DO_BEGIN << "Remapped instance node " << m_name << " has no source node.\n" AFTK_DO_ERROR_END_NO_STATUS;
#ifdef _DEBUG
		AFTK_ASSERT(false);
#endif
		error++;
	}

	return error;
}

void C_SceneMeshRemappedInstanceNode::SetSourceNodePtr(C_SceneMeshNode* pSource, bool copyMappings)
{
	SetOnlySourceNodePtr(pSource);
	if (!copyMappings)
		return;

	if (m_pSourceNode)
	{
		//Copy blind data mapping data from rhs
		CopyPolygonBlindDataMappings(m_pSourceNode);

		//Copy material mapping data from rhs
		CopyPolygonMaterialMappings(m_pSourceNode, 0);

		CopyTextureCoordinateOffsets(m_pSourceNode);

		AFTK_ASSERT(GetNumPolygonMaterialMappings() == pSource->GetNumPolygonMaterialMappings());
		AFTK_ASSERT(GetNumPolygonBlindDataMappings() == pSource->GetNumPolygonBlindDataMappings());
	}
	if (!pSource)
	{
		//Clear mapping tables and offsets if we're not instancing anything.
		ClearMappingTables();
		m_uvwxOffsets.Clear();
	}
}

C_SceneMeshNode* C_SceneMeshRemappedInstanceNode::GetSourceNodePtr()
{
	return m_pSourceNode;
}

const C_SceneMeshNode* C_SceneMeshRemappedInstanceNode::GetSourceNodePtr() const
{
	return m_pSourceNode;
}

void C_SceneMeshRemappedInstanceNode::ApplyMatrix(const Matrix44& matrix)
{
}

bool C_SceneMeshRemappedInstanceNode::operator== (const C_SceneMeshRemappedInstanceNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneMeshRemappedInstanceNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_SceneMeshRemappedInstanceNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	if (classToAbortTest == E_ObjType_SceneMeshRemappedInstance)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneMeshRemappableNode::IsEqualToAll(rhs, classToAbortTest, flags));
}

bool C_SceneMeshRemappedInstanceNode::IsEqualToThis(const I_Object& _rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_SceneMeshRemappedInstance))
	{
		return false;
	}

	C_SceneMeshRemappedInstanceNode& rhs = (C_SceneMeshRemappedInstanceNode&) _rhs;

	if (m_pSourceNode != rhs.m_pSourceNode)
		return false;

	return true;
}

U32 C_SceneMeshRemappedInstanceNode::GetPolygonBlindDataMappingReferenceCount(S16 index) const
{
	AFTK_ASSERT(m_pSourceNode);
	AFTK_ASSERT(index < GetNumPolygonBlindDataMappings());
	return 1;
}

U32 C_SceneMeshRemappedInstanceNode::GetPolygonMaterialMappingReferenceCount(S16 index) const
{
	AFTK_ASSERT(m_pSourceNode);
	AFTK_ASSERT(index < GetNumPolygonMaterialMappings());
	return 1;
}

bool C_SceneMeshRemappedInstanceNode::IsSkinned() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->IsSkinned();
}

bool C_SceneMeshRemappedInstanceNode::IsSkinAnimated() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->IsSkinAnimated();
}

const C_MatrixPaletteEntry& C_SceneMeshRemappedInstanceNode::GetPaletteEntry(U32 polyIndex)
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPaletteEntry(polyIndex);
}

U32 C_SceneMeshRemappedInstanceNode::GetNumPaletteEntries()
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumPaletteEntries();
}

U32 C_SceneMeshRemappedInstanceNode::GetNumVertexWeights() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexWeights();
}

const C_VertexWeight& C_SceneMeshRemappedInstanceNode::GetVertexWeight(U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetVertexWeight(instance);
}

Position3 C_SceneMeshRemappedInstanceNode::GetPolygonVertexPosition(
	U32 polyVertIndex) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonVertexPosition(polyVertIndex);
}

Normal3 C_SceneMeshRemappedInstanceNode::GetPolygonVertexNormal(
	U32 polyVertIndex) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonVertexNormal(polyVertIndex);
}

Vector3F32 C_SceneMeshRemappedInstanceNode::GetPolygonVertexTangent(
	U32 uvSet, 
	U32 polyVertIndex) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonVertexTangent(uvSet, polyVertIndex);
}

Vector3F32 C_SceneMeshRemappedInstanceNode::GetPolygonVertexBinormal(
	U32 uvSet, 
	U32 polyVertIndex) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonVertexBinormal(uvSet, polyVertIndex);
}

Position3 C_SceneMeshRemappedInstanceNode::GetEvaluatedPolygonVertexPosition(
	U32 polyVertIndex) const
{
	return m_pSourceNode->GetEvaluatedPolygonVertexPosition(polyVertIndex);
}

TextureCoordinate4 C_SceneMeshRemappedInstanceNode::GetPolygonVertexTextureCoordinate(U32 uvSet, U32 polyVertIndex) const
{
	return m_pSourceNode->GetPolygonVertexTextureCoordinate(uvSet, polyVertIndex);
}

Normal3 C_SceneMeshRemappedInstanceNode::GetEvaluatedPolygonVertexNormal(
	U32 polyVertIndex) const
{
	return m_pSourceNode->GetEvaluatedPolygonVertexNormal(polyVertIndex);
}

Vector3F32 C_SceneMeshRemappedInstanceNode::GetEvaluatedPolygonVertexTangent(
	U32 uvSet, 
	U32 polyVertIndex) const
{
	return m_pSourceNode->GetEvaluatedPolygonVertexTangent(uvSet, polyVertIndex);
}

Vector3F32 C_SceneMeshRemappedInstanceNode::GetEvaluatedPolygonVertexBinormal(
	U32 uvSet, 
	U32 polyVertIndex) const
{
	return m_pSourceNode->GetEvaluatedPolygonVertexBinormal(uvSet, polyVertIndex);
}

void C_SceneMeshRemappedInstanceNode::Clear()
{
	ClearThis();
	C_SceneMeshRemappableNode::Clear();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneMeshRemappedInstanceNode::CalculateBounds(C_Bounds& bounds, const Matrix44* pMatrix) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->CalculateBounds(bounds, pMatrix);
}

void C_SceneMeshRemappedInstanceNode::SetOnlySourceNodePtr(C_SceneMeshNode* pSource)
{
	if (m_pSourceNode != pSource)
	{
		//Remove association with previous instance source.
		if (m_pSourceNode)
		{
			m_pSourceNode->DeleteInstanceOfThisNode(this);
		}
		m_pSourceNode = pSource;
		//Add association with new instance source.
		if (m_pSourceNode)
		{
			AFTK_ASSERT(pSource->m_pScene == m_pScene);
			m_pSourceNode->AddInstanceOfThisNode(this);
		}
	}
}

void C_SceneMeshRemappedInstanceNode::OneTimeInit()
{
	m_pSourceNode = 0;
}

void C_SceneMeshRemappedInstanceNode::ClearThis()
{
	if (m_pSourceNode)
	{
		//Remove association with instance source.
		m_pSourceNode->DeleteInstanceOfThisNode(this);
	}

	m_pSourceNode = 0;
}

void C_SceneMeshRemappedInstanceNode::RepairMappingTables()
{
	if (!m_pSourceNode)
		return;

	for (U16 i = GetNumPolygonMaterialMappings(); i < m_pSourceNode->GetNumPolygonMaterialMappings(); i++)
	{
		//Add missing entry.
		AddPolygonMaterialMappingSetCount(m_pSourceNode->GetPolygonMaterialMappingPtr(i), 1);
	}
	for (U16 i = GetNumPolygonBlindDataMappings(); i < m_pSourceNode->GetNumPolygonBlindDataMappings(); i++)
	{
		//Add missing entry.
		AddPolygonBlindDataMappingSetCount(m_pSourceNode->GetPolygonBlindDataMappingPtr(i), 1);
	}
}

bool C_SceneMeshRemappedInstanceNode::IsRemappedSource() const
{
	return false;
}

void C_SceneMeshRemappedInstanceNode::DeleteInstanceSource(C_SceneNode* pSource)
{
	//Okay, in some way we're required by a boned instance. We could be one of its bones,
	//or perhaps one of its base-instance.
	if (GetSourceNodePtr() == pSource)
		SetSourceNodePtr(0);
}

#endif
