#include <stdafx.h>

using namespace Aftk;

C_SceneMeshBonedInstanceNode::C_SceneMeshBonedInstanceNode(
	C_Scene* pScene)
	:
	C_SceneMeshRemappedInstanceNode(pScene)
{
	OneTimeInit();
}

C_Node* C_SceneMeshBonedInstanceNode::Clone(C_Scene* pScene) const
{
	C_SceneMeshBonedInstanceNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneMeshBonedInstanceNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneMeshBonedInstanceNode(pScene);
		(C_SceneMeshBonedInstanceNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_Node* C_SceneMeshBonedInstanceNode::Clone(C_Scene* pScene, const MaterialRemappingHashTable& mappingTable) const
{
	C_SceneMeshBonedInstanceNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneMeshBonedInstanceNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneMeshBonedInstanceNode(pScene);
		(C_SceneMeshBonedInstanceNode&)(*pNewNode).Copy(*this, &mappingTable);
	}

	return pNewNode;
}

const C_SceneMeshBonedInstanceNode& C_SceneMeshBonedInstanceNode::Copy(const C_SceneMeshBonedInstanceNode& rhs, const MaterialRemappingHashTable* pMappingTable)
{
	C_SceneMeshRemappedInstanceNode::Copy(rhs, pMappingTable);

	ClearThis();

	if (m_pScene == rhs.m_pScene)
	{
		m_pBones[0] = rhs.m_pBones[0];
		m_pBones[1] = rhs.m_pBones[1];
	}

	return *this;
}

C_SceneMeshBonedInstanceNode::C_SceneMeshBonedInstanceNode(
	const C_SceneMeshBonedInstanceNode& rhs) : 
	C_SceneMeshRemappedInstanceNode(0)
{
	OneTimeInit();
	*this = rhs;
}

const C_SceneMeshBonedInstanceNode& C_SceneMeshBonedInstanceNode::operator =(
	const C_SceneMeshBonedInstanceNode& rhs)
{
	return Copy(rhs, 0);
}

C_SceneMeshBonedInstanceNode::~C_SceneMeshBonedInstanceNode()
{
	//Disassociate with bones.
	SetBonePtr(0, 0);
	SetBonePtr(1, 0);
}

S32	C_SceneMeshBonedInstanceNode::GetTextureCoordinateSetIndex(const std::string& name) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetTextureCoordinateSetIndex(name);
}

const std::string& C_SceneMeshBonedInstanceNode::GetTextureCoordinateSetName(U32 set) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetTextureCoordinateSetName(set);
}

const C_VertexDataChannelInfo& C_SceneMeshBonedInstanceNode::GetVertexDataChannelInfo(U32 channel) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetVertexDataChannelInfo(channel);
}
	
const Aftk::C_Polygon& C_SceneMeshBonedInstanceNode::GetPolygon(U32 lodLevel, U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygon(lodLevel, instance);
}

const Aftk::C_Polygon& C_SceneMeshBonedInstanceNode::GetPolygon(U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygon(instance);
}

const TextureCoordinate4& C_SceneMeshBonedInstanceNode::GetTextureCoordinate(U32 set, U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetTextureCoordinate(set, instance);
}

const Vector3F32& C_SceneMeshBonedInstanceNode::GetVertexTangent(U32 set, U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetVertexTangent(set, instance);
}

const Vector3F32& C_SceneMeshBonedInstanceNode::GetVertexBinormal(U32 set, U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetVertexBinormal(set, instance);
}

const C_PolygonVertex& C_SceneMeshBonedInstanceNode::GetPolygonVertex(U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonVertex(instance);
}

const C_PolygonCluster* C_SceneMeshBonedInstanceNode::GetPolygonClusterPtr(U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonClusterPtr(instance);
}

U32 C_SceneMeshBonedInstanceNode::GetNumVertexPositions() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexPositions();
}

U32	C_SceneMeshBonedInstanceNode::GetNumVertexNormals() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexNormals();
}

U32	C_SceneMeshBonedInstanceNode::GetNumVertexTangents(U32 set) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexTangents(set);
}

U32	C_SceneMeshBonedInstanceNode::GetNumVertexBinormals(U32 set) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexBinormals(set);
}

U32	C_SceneMeshBonedInstanceNode::GetNumVertexData() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexData();
}

U32	C_SceneMeshBonedInstanceNode::GetNumTextureCoordinates(U32 set) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumTextureCoordinates(set);
}

U32	C_SceneMeshBonedInstanceNode::GetNumTextureCoordinateSets() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumTextureCoordinateSets();
}

U32	C_SceneMeshBonedInstanceNode::GetNumVertexDataChannels() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexDataChannels();
}

const C_MeshLodLevelInfo& C_SceneMeshBonedInstanceNode::GetLodLevelInfo(U32 lodLevel) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetLodLevelInfo(lodLevel);
}

C_MeshLodLevelInfo& C_SceneMeshBonedInstanceNode::GetLodLevelInfo(U32 lodLevel)
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetLodLevelInfo(lodLevel);
}

U32	C_SceneMeshBonedInstanceNode::GetNumPolygonVertices() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumPolygonVertices();
}

U32	C_SceneMeshBonedInstanceNode::GetNumPolygons(U32 lodLevel) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumPolygons(lodLevel);
}

U32	C_SceneMeshBonedInstanceNode::GetNumTriangles(U32 lodLevel) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumTriangles(lodLevel);
}

U32	C_SceneMeshBonedInstanceNode::GetNumMaterialNodes(U32 lodLevel) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumMaterialNodes(lodLevel);
}

U32 C_SceneMeshBonedInstanceNode::GetNumPolygonClusters() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumPolygonClusters();
}

E_ObjType C_SceneMeshBonedInstanceNode::GetType() const
{
	return E_ObjType_SceneMeshBonedInstance;
}

bool C_SceneMeshBonedInstanceNode::HasType(E_ObjType type) const
{
	if (C_SceneMeshRemappedInstanceNode::HasType(type))
		return true;
	if (type == C_SceneMeshBonedInstanceNode::GetType()) 
		return true;

	return false;
}
/*
void C_SceneMeshBonedInstanceNode::Output() const
{
	AFTK_ASSERT(m_pSourceNode);
	m_pSourceNode->Output();
}
*/
bool C_SceneMeshBonedInstanceNode::IsAdjacencyBuilt() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->IsAdjacencyBuilt();
}

const U32* C_SceneMeshBonedInstanceNode::GetAdjacencyList(U32 iThisPoly) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetAdjacencyList(iThisPoly);
}

U32 C_SceneMeshBonedInstanceNode::Validate() const
{
	C_SceneMeshBonedInstanceNode* pThis = const_cast<C_SceneMeshBonedInstanceNode*>(this);
	return pThis->ProcessProblems(false);
}

U32 C_SceneMeshBonedInstanceNode::ProcessProblems(bool fix)
{
	U32 error = 0;
	error += C_SceneNode::ProcessProblems(fix);
	if (!m_pSourceNode)
	{
		AFTK_DO_BEGIN 
			<< "Remapped instance node " 
			<< m_name
			<< " has no source node.\n" 
		AFTK_DO_ERROR_END_NO_STATUS;
		#ifdef _DEBUG
		AFTK_ASSERT(false);
		#endif
		error++;
	}
	else
	{
		if (!m_pSourceNode->IsSkinned())
		{
			AFTK_DO_BEGIN 
				<< "Remapped instance node " 
				<< m_name 
				<< " has an unskinned source node. Check that the base instance " 
				<< m_pSourceNode->GetName() 
				<< " has its weighting information is correctly set.\n" 
			AFTK_DO_ERROR_END_NO_STATUS;
			#ifdef _DEBUG
			AFTK_ASSERT(false);
			#endif
			error++;
		}
	}

	for (U32 i = 0; i < 2; i++)
		if (!m_pBones[i])
		{
			AFTK_DO_BEGIN 
				<< "Remapped instance node " 
				<< m_name 
				<< " has no bone " 
				<< i 
				<< " node.\n" 
			AFTK_DO_ERROR_END_NO_STATUS;
			#ifdef _DEBUG
			AFTK_ASSERT(false);
			#endif
			error++;
		}

	return error;
}


void C_SceneMeshBonedInstanceNode::SetBonePtr(U32 index, C_SceneNode* pSceneNode)
{
	AFTK_ASSERT(index < 2);
	if (index > 2)
		return;

	if (m_pBones[index])
		//We're no longer instancing the given bone.
		m_pBones[index]->DeleteInstanceOfThisNode(this);

	//Asignment...
	m_pBones[index] = pSceneNode;

	//Make sure that the bone is aware it's being instanced. This also means that when a bone
	//is destroyed it will inform is of its destruction so that we can avoid dangling pointers.
	if (pSceneNode)
	{
		AFTK_ASSERT(pSceneNode->m_pScene == m_pScene);
		pSceneNode->AddInstanceOfThisNode(this);
	}

	m_localBoundsCacheDirtyFlag = true;
	m_worldBoundsCacheDirtyFlag = true;
	m_evaluated = false;
}

C_SceneNode* C_SceneMeshBonedInstanceNode::GetBonePtr(U32 index)
{
	return m_pBones[index];
}

const C_SceneNode* C_SceneMeshBonedInstanceNode::GetBonePtr(U32 index) const
{
	return m_pBones[index];
}

C_SceneNode** C_SceneMeshBonedInstanceNode::GetBoneArray()
{
	return (Aftk::C_SceneNode**)m_pBones;
}

const C_SceneNode** C_SceneMeshBonedInstanceNode::GetBoneArray() const
{
	return (const Aftk::C_SceneNode**)m_pBones;
}

void C_SceneMeshBonedInstanceNode::ApplyMatrix(const Matrix44& matrix)
{
	//noop
}

bool C_SceneMeshBonedInstanceNode::operator== (const C_SceneMeshBonedInstanceNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneMeshBonedInstanceNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_SceneMeshBonedInstanceNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneMeshBonedInstance)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneMeshRemappedInstanceNode::IsEqualToAll(rhs, classToAbort, flags));
}

bool C_SceneMeshBonedInstanceNode::IsEqualToThis(const I_Object& _rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_SceneMeshBonedInstance))
	{
		return false;
	}

	C_SceneMeshBonedInstanceNode& rhs = (C_SceneMeshBonedInstanceNode&) _rhs;

	if ((m_pBones[0] != rhs.m_pBones[0])
		||
		(m_pBones[1] != rhs.m_pBones[1]))
		return false;

	if ((!EqualityTestFMatrix(m_matrixPalette[0].matrix, rhs.m_matrixPalette[0].matrix))
		||
		(!EqualityTestFMatrix(m_worldMatrixInverse, rhs.m_worldMatrixInverse)))
		return false;

	return true;
}

bool C_SceneMeshBonedInstanceNode::IsSkinned() const
{
	return true;
}

bool C_SceneMeshBonedInstanceNode::IsSkinAnimated() const
{
	return false;
}

U32 C_SceneMeshBonedInstanceNode::GetNumLodLevels() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumLodLevels();
}

const C_VertexWeight& C_SceneMeshBonedInstanceNode::GetVertexWeight(U32 instance) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetVertexWeight(instance);
}

U32 C_SceneMeshBonedInstanceNode::GetNumVertexWeights() const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetNumVertexWeights();
}

const C_MatrixPaletteEntry& C_SceneMeshBonedInstanceNode::GetPaletteEntry(U32 instance) 
{
	AFTK_ASSERT(instance < 2);
	return m_matrixPalette[instance];
}

U32 C_SceneMeshBonedInstanceNode::GetNumPaletteEntries() 
{
	return 2;
}

C_Status C_SceneMeshBonedInstanceNode::BuildAnimationHelpers() const
{
	AFTK_ASSERT(m_pSourceNode);
	C_Status status;
	status = m_pSourceNode->BuildAnimationHelpers();
	if (!status) return status;
	//Evaluate matrix palette by default, otherwise boned instance would assume parents position.
	status = Evaluate();
	return status;
}

void C_SceneMeshBonedInstanceNode::ClearAnimationHelpers() const
{
	m_evaluated = false;
}

C_Status C_SceneMeshBonedInstanceNode::Evaluate() const
{
	if (!m_pBones[0])
	{
		AFTK_ASSERT(m_pBones[0]);
		return C_Status::E_Status_Failure;
	}
	if (!m_pBones[1])
	{
		AFTK_ASSERT(m_pBones[1]);
		return C_Status::E_Status_Failure;
	}

	if (m_pSourceNode->GetAnimationHelpersBuilt() == E_AnimHelperState_NotBuilt)
		BuildAnimationHelpers();

	Matrix44 worldMatrix = GetWorldMatrix();
	Matrix44 worldMatrixInv;
	UtMaths::Invert4x4(&worldMatrixInv, worldMatrix);

	m_matrixPalette[0].matrix 
		= m_basePose * m_pBones[0]->GetBasePoseInverse() * m_pBones[0]->GetWorldMatrix() * worldMatrixInv;

	m_matrixPalette[1].matrix 
		= m_basePose * m_pBones[1]->GetBasePoseInverse() * m_pBones[1]->GetWorldMatrix() * worldMatrixInv;

	Matrix44 temp;
	temp = m_matrixPalette[0].matrix;
	UtMaths::Transpose(&temp);
	UtMaths::Invert4x4(&(m_matrixPalette[0].matrixInverseTranspose), temp);

	temp = m_matrixPalette[1].matrix;
	UtMaths::Transpose(&temp);
	UtMaths::Invert4x4(&(m_matrixPalette[1].matrixInverseTranspose), temp);

	m_evaluated = true;
	return C_Status::E_Status_Success;
}

Position3 C_SceneMeshBonedInstanceNode::GetPolygonVertexPosition(
	U32 polyVertIndex) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonVertexPosition(polyVertIndex);
}

Normal3 C_SceneMeshBonedInstanceNode::GetPolygonVertexNormal(
	U32 polyVertIndex) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonVertexNormal(polyVertIndex);
}

Vector3F32 C_SceneMeshBonedInstanceNode::GetPolygonVertexTangent(
	U32 uvSet, 
	U32 polyVertIndex) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonVertexTangent(uvSet, polyVertIndex);
}

Vector3F32 C_SceneMeshBonedInstanceNode::GetPolygonVertexBinormal(
	U32 uvSet, 
	U32 polyVertIndex) const
{
	AFTK_ASSERT(m_pSourceNode);
	return m_pSourceNode->GetPolygonVertexBinormal(uvSet, polyVertIndex);
}

Position3 C_SceneMeshBonedInstanceNode::GetEvaluatedPolygonVertexPosition(
	U32 polyVertIndex) const
{
	if (m_pSourceNode->GetAnimationHelpersBuilt() == E_AnimHelperState_NotBuilt)
		BuildAnimationHelpers();
	if (!m_evaluated)
		Evaluate();

	Position3 position;
	EvaluatePolygonVertexPosition(polyVertIndex, position);
	return position;
}

TextureCoordinate4 C_SceneMeshBonedInstanceNode::GetPolygonVertexTextureCoordinate(U32 uvSet, U32 polyVertIndex) const
{
	const C_PolygonVertex* pPV = &GetPolygonVertex(polyVertIndex);
	S32 uvIndex = pPV->GetTextureCoordinateIndex(uvSet);
	if (uvIndex == -1)
	{
		return TextureCoordinate4(0,0,0,0);
	}
	return GetTextureCoordinate(uvSet, uvIndex);
}

Normal3 C_SceneMeshBonedInstanceNode::GetEvaluatedPolygonVertexNormal(
	U32 polyVertIndex) const
{
	if (m_pSourceNode->GetAnimationHelpersBuilt() == E_AnimHelperState_NotBuilt)
		BuildAnimationHelpers();
	if (!m_evaluated)
		Evaluate();

	Normal3 normal;
	EvaluatePolygonVertexNormal(polyVertIndex, normal);
	return normal;
}

Vector3F32 C_SceneMeshBonedInstanceNode::GetEvaluatedPolygonVertexTangent(
	U32 uvSet, 
	U32 polyVertIndex) const
{
	if (m_pSourceNode->GetAnimationHelpersBuilt() == E_AnimHelperState_NotBuilt)
		BuildAnimationHelpers();
	if (!m_evaluated)
		Evaluate();

	Vector3F32 tangent;
	EvaluatePolygonVertexTangent(uvSet, polyVertIndex, tangent);
	return tangent;
}

Vector3F32 C_SceneMeshBonedInstanceNode::GetEvaluatedPolygonVertexBinormal(
	U32 uvSet, 
	U32 polyVertIndex) const
{
	if (m_pSourceNode->GetAnimationHelpersBuilt() == E_AnimHelperState_NotBuilt)
		BuildAnimationHelpers();
	if (!m_evaluated)
		Evaluate();

	Vector3F32 binormal;
	EvaluatePolygonVertexBinormal(uvSet, polyVertIndex, binormal);
	return binormal;
}

void C_SceneMeshBonedInstanceNode::Clear()
{
	ClearThis();
	C_SceneMeshRemappedInstanceNode::Clear();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneMeshBonedInstanceNode::CalculateBounds(C_Bounds& bounds, const Matrix44* pMatrix) const
{
	AFTK_ASSERT(m_pSourceNode);
	Evaluate();

	bounds.Clear();

	//Evaluate this mesh in order to generate its bounding box.
	for (U32 iPolyVert = 0; iPolyVert < m_pSourceNode->GetNumPolygonVertices(); iPolyVert++)
	{
		Position3 position;
		EvaluatePolygonVertexPosition(iPolyVert, position);
		if (pMatrix)
			bounds.AddVertex(Aftk::ApplyMatrix(*pMatrix, position));
		else
			bounds.AddVertex(position);
	}
}

void C_SceneMeshBonedInstanceNode::EvaluatePolygonVertexPosition(
	U32 instance,
	Position3& positionLocalResult) const
{
	AFTK_ASSERT(m_pSourceNode);

	if (!m_evaluated)
		Evaluate();

	const C_PolygonVertex& originalPV = m_pSourceNode->GetPolygonVertex(instance);

	positionLocalResult[0] = 0;
	positionLocalResult[1] = 0;
	positionLocalResult[2] = 0;

	S32 vPosIndex = originalPV.GetPositionIndex();
	if (vPosIndex == -1)
	{
		return;
	}

	//Move us in to bind space.
	Position3 positionLocalOriginal = m_pSourceNode->GetVertexPosition(vPosIndex);

	if (m_pSourceNode->GetNumVertexWeights() == 0)
	{
		return;
	}

	const C_VertexWeight& weight = m_pSourceNode->GetVertexWeight(vPosIndex);
	if (m_pSourceNode->GetAnimationHelpersBuilt() == Aftk::E_AnimHelperState_NotNeeded || Aftk::E_AnimHelperState_NotBuilt)
	{
		positionLocalResult = positionLocalOriginal;
		return;
	}

	F32 lastWeight = 0.0f;

	U32 weightIndex = 0;
	if (weight.GetNumWeights() > 1)
	{
		//Apply the first influence...
		{
			F32 thisWeight = weight.GetWeight(0);
			U32 matrixIndex = weight.GetInfluenceNodeIndex(weightIndex);
			AFTK_ASSERT(matrixIndex < 2);
			lastWeight += thisWeight;

			//Calculate position
			{
				Position3 tempPosition;
				tempPosition = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrix, 
					positionLocalOriginal);

				positionLocalResult[0] = thisWeight * tempPosition[0];
				positionLocalResult[1] = thisWeight * tempPosition[1];
				positionLocalResult[2] = thisWeight * tempPosition[2];
			}
		}
		weightIndex++;		
	}

	//Apply the last influence...
	{
		lastWeight = 1.0f - lastWeight;
		U32 matrixIndex = weight.GetInfluenceNodeIndex(weightIndex);

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
	}
}

void C_SceneMeshBonedInstanceNode::EvaluatePolygonVertexNormal(
	U32 instance,
	Normal3& normalLocalResult) const
{
	AFTK_ASSERT(m_pSourceNode);

	if (!m_evaluated)
		Evaluate();

	const C_PolygonVertex& originalPV = m_pSourceNode->GetPolygonVertex(instance);
	S32 vPosIndex = originalPV.GetPositionIndex();
	if (vPosIndex == -1)
	{
		normalLocalResult[0] = 0;
		normalLocalResult[1] = 0;
		normalLocalResult[2] = 0;
		return;
	}

	S32 vNormIndex = originalPV.GetNormalIndex();
	if (vNormIndex == -1)
	{
		normalLocalResult[0] = 0;
		normalLocalResult[1] = 0;
		normalLocalResult[2] = 0;
		return;
	}

	Normal3 normalLocalOriginal = m_pSourceNode->GetVertexNormal(vNormIndex);
	if (m_pSourceNode->GetAnimationHelpersBuilt() == Aftk::E_AnimHelperState_NotNeeded || Aftk::E_AnimHelperState_NotBuilt)
	{
		normalLocalResult = normalLocalOriginal;
		return;
	}

	const C_VertexWeight& weight = m_pSourceNode->GetVertexWeight(vPosIndex);

	F32 lastWeight = 0.0f;
	U32 weightIndex = 0;
	if (weight.GetNumWeights() > 1)
	{
		//Apply the first influence...
		{
			F32 thisWeight = weight.GetWeight(0);
			U32 matrixIndex = weight.GetInfluenceNodeIndex(weightIndex);

			lastWeight += thisWeight;

			//Calculate normal
			{
				Normal3 tempNormal;
				if (m_normalInverseTranspose)
					tempNormal = ApplyMatrixToNormal(
						m_matrixPalette[matrixIndex].matrixInverseTranspose, 
						normalLocalOriginal);
				else
					tempNormal = ApplyMatrixToNormal(
						m_matrixPalette[matrixIndex].matrixInverseTranspose, 
						normalLocalOriginal);

				normalLocalResult[0] = thisWeight * tempNormal[0];
				normalLocalResult[1] = thisWeight * tempNormal[1];
				normalLocalResult[2] = thisWeight * tempNormal[2];
			}
		}
		weightIndex++;		
	}
	else
	{
		normalLocalResult[0] = 0;
		normalLocalResult[1] = 0;
		normalLocalResult[2] = 0;
	}

	//Apply the last influence...
	{
		lastWeight = 1.0f - lastWeight;
		U32 matrixIndex = weight.GetInfluenceNodeIndex(weightIndex);

		//Calculate normal
		{
			Normal3 tempNormal;
			if (m_normalInverseTranspose)
			{
				tempNormal = ApplyMatrixToNormal(
					m_matrixPalette[matrixIndex].matrixInverseTranspose, 
					normalLocalOriginal);
			}
			else
			{
				tempNormal = ApplyMatrixToNormal(
					m_matrixPalette[matrixIndex].matrix, 
					normalLocalOriginal);
			}

			normalLocalResult[0] += lastWeight * tempNormal[0];
			normalLocalResult[1] += lastWeight * tempNormal[1];
			normalLocalResult[2] += lastWeight * tempNormal[2];
		}
	}

	Normalise(normalLocalResult);
}

void C_SceneMeshBonedInstanceNode::EvaluatePolygonVertexTangent(
	U32 set,
	U32 instance,
	Vector3F32& tangentLocalResult) const
{
	AFTK_ASSERT(m_pSourceNode);

	if (!m_evaluated)
		Evaluate();

	const C_PolygonVertex& originalPV = m_pSourceNode->GetPolygonVertex(instance);
	S32 vPosIndex = originalPV.GetPositionIndex();
	if (vPosIndex == -1)
	{
		tangentLocalResult[0] = 0;
		tangentLocalResult[1] = 0;
		tangentLocalResult[2] = 0;
		return;
	}

	S32 vTangentIndex = originalPV.GetTangentIndex(set);
	if (vTangentIndex == -1)
	{
		tangentLocalResult[0] = 0;
		tangentLocalResult[1] = 0;
		tangentLocalResult[2] = 0;
		return;
	}

	Vector3F32 tangentLocalOriginal = m_pSourceNode->GetVertexTangent(set, vTangentIndex);
	if (m_pSourceNode->GetAnimationHelpersBuilt() == Aftk::E_AnimHelperState_NotNeeded || Aftk::E_AnimHelperState_NotBuilt)
	{
		tangentLocalResult = tangentLocalOriginal;
		return;
	}
	const C_VertexWeight& weight = m_pSourceNode->GetVertexWeight(vPosIndex);

	F32 lastWeight = 0.0f;

	U32 weightIndex = 0;
	if (weight.GetNumWeights() > 1)
	{
		//Apply the first influence...
		{
			F32 thisWeight = weight.GetWeight(0);
			U32 matrixIndex = weight.GetInfluenceNodeIndex(weightIndex);

			lastWeight += thisWeight;

			//Calculate tangent
			{
				Vector3F32 tempTangent;
				tempTangent = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrixInverseTranspose, 
					tangentLocalOriginal);

				tangentLocalResult[0] = thisWeight * tempTangent[0];
				tangentLocalResult[1] = thisWeight * tempTangent[1];
				tangentLocalResult[2] = thisWeight * tempTangent[2];
			}
		}
		weightIndex++;		
	}
	else
	{
		tangentLocalResult[0] = 0;
		tangentLocalResult[1] = 0;
		tangentLocalResult[2] = 0;
	}

	//Apply the last influence...
	{
		lastWeight = 1.0f - lastWeight;
		U32 matrixIndex = weight.GetInfluenceNodeIndex(weightIndex);

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
	}
}

void C_SceneMeshBonedInstanceNode::EvaluatePolygonVertexBinormal(
	U32 set,
	U32 instance,
	Vector3F32& binormalLocalResult) const
{
	AFTK_ASSERT(m_pSourceNode);

	if (!m_evaluated)
		Evaluate();

	const C_PolygonVertex& originalPV = m_pSourceNode->GetPolygonVertex(instance);
	S32 vPosIndex = originalPV.GetPositionIndex();
	if (vPosIndex == -1)
	{
		binormalLocalResult[0] = 0;
		binormalLocalResult[1] = 0;
		binormalLocalResult[2] = 0;
		return;
	}

	S32 vBinormalIndex = originalPV.GetBinormalIndex(set);
	if (vBinormalIndex == -1)
	{
		binormalLocalResult[0] = 0;
		binormalLocalResult[1] = 0;
		binormalLocalResult[2] = 0;
		return;
	}

	Vector3F32 binormalLocalOriginal = m_pSourceNode->GetVertexBinormal(set, vBinormalIndex);

	const C_VertexWeight& weight = m_pSourceNode->GetVertexWeight(vPosIndex);
	if (m_pSourceNode->GetAnimationHelpersBuilt() == Aftk::E_AnimHelperState_NotNeeded || Aftk::E_AnimHelperState_NotBuilt)
	{
		binormalLocalResult = binormalLocalOriginal;
		return;
	}

	F32 lastWeight = 0.0f;

	U32 weightIndex = 0;
	if (weight.GetNumWeights() > 1)
	{
		//Apply the first influence...
		{
			F32 thisWeight = weight.GetWeight(0);
			U32 matrixIndex = weight.GetInfluenceNodeIndex(weightIndex);

			lastWeight += thisWeight;

			//Calculate normal
			{
				Vector3F32 tempBinormal;
				tempBinormal = Aftk::ApplyMatrix(
					m_matrixPalette[matrixIndex].matrixInverseTranspose, 
					binormalLocalOriginal);

				binormalLocalResult[0] = thisWeight * tempBinormal[0];
				binormalLocalResult[1] = thisWeight * tempBinormal[1];
				binormalLocalResult[2] = thisWeight * tempBinormal[2];
			}
		}
		weightIndex++;		
	}
	else
	{
		binormalLocalResult[0] = 0;
		binormalLocalResult[1] = 0;
		binormalLocalResult[2] = 0;
	}

	//Apply the last influence...
	{
		lastWeight = 1.0f - lastWeight;
		U32 matrixIndex = weight.GetInfluenceNodeIndex(weightIndex);

		//Calculate normal
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

void C_SceneMeshBonedInstanceNode::OneTimeInit()
{
	m_pBones[0] = 0;
	m_pBones[1] = 0;

	m_evaluated = false;

	UtMaths::SetIdentity(&m_matrixPalette[0].matrix);
	UtMaths::SetIdentity(&m_matrixPalette[1].matrix);
	UtMaths::SetIdentity(&m_matrixPalette[0].matrixInverseTranspose);
	UtMaths::SetIdentity(&m_matrixPalette[1].matrixInverseTranspose);
	UtMaths::SetIdentity(&m_worldMatrixInverse);

	m_pSourceNode = 0;
}

void C_SceneMeshBonedInstanceNode::ClearThis()
{
	if (m_pSourceNode)
	{
		//Remove association with instance source.
		m_pSourceNode->DeleteInstanceOfThisNode(this);
		m_pSourceNode = 0;
	}

	m_pBones[0] = 0;
	m_pBones[1] = 0;
	m_evaluated = false;

	UtMaths::SetIdentity(&m_matrixPalette[0].matrix);
	UtMaths::SetIdentity(&m_matrixPalette[1].matrix);
	UtMaths::SetIdentity(&m_matrixPalette[0].matrixInverseTranspose);
	UtMaths::SetIdentity(&m_matrixPalette[1].matrixInverseTranspose);
	UtMaths::SetIdentity(&m_worldMatrixInverse);
}

void C_SceneMeshBonedInstanceNode::DeleteInstanceSource(C_SceneNode* pSource)
{
	//Okay, in some way we're required by a boned instance. We could be one of its bones,
	//or perhaps one of its base-instance.
	if (GetSourceNodePtr() == pSource)
		SetSourceNodePtr(0);
	else if (GetBonePtr(0) == pSource)
		SetBonePtr(0, 0);
	else if (GetBonePtr(1) == pSource)
		SetBonePtr(1, 0);
	else
		AFTK_ASSERT(!"We thought we were instanced by a boned instance but we weren't");
}

#endif
