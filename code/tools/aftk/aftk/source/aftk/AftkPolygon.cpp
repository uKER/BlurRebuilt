#include <stdafx.h>

#include <aftk/AftkBuildAdjacency.h>

namespace Aftk
{
	F32 GetTriangleArea(
		Position3 pos1,
		Position3 pos2,
		Position3 pos3,
		const Normal3& normal)
	{
		F32 area = 0;
		F32 ax = fabsf(normal[E_Axis_X]);
		F32 ay = fabsf(normal[E_Axis_Y]);
		F32 az = fabsf(normal[E_Axis_Z]);
		E_Axis axisToIgnore;

		if ((ax > ay) && (ax > az))
			axisToIgnore = E_Axis_X;
		else if (ay > az) 
			axisToIgnore = E_Axis_Y;
		else
		{
			axisToIgnore = E_Axis_Z;
		}

		switch(axisToIgnore)
		{
		case E_Axis_X:
			area = pos2[E_Axis_Y] * (pos3[E_Axis_Z] - pos1[E_Axis_Z]);
			area *= (1 / (2*ax));
			break;
		case E_Axis_Y:
			area += pos2[E_Axis_X] * (pos3[E_Axis_Z] - pos1[E_Axis_Z]);
			area *= (1 / (2*ay));
			break;
		case E_Axis_Z:
			area += pos2[E_Axis_X] * (pos3[E_Axis_Y] - pos1[E_Axis_Y]);
			area *= (1 / (2*az));
			break;
		}

		return area;
	}
}

using namespace Aftk;

C_Polygon::~C_Polygon()
{
	if (m_pAttachedData)
	{
		delete m_pAttachedData;
	}
	m_pAttachedData = 0;

	if (m_pMeshNode)
	{
		//IMPORTANT: Avoid calling 'deletePolygonMapping' functions within mesh destructor.
		//We know we're in the mesh destructor when we're deleting a polygon belonging to a mesh
		//that claims that it has no polygons.
		U32 polyCount = 0;
		for (U32 iLodLevel = 0; iLodLevel < m_pMeshNode->GetNumLodLevels(); iLodLevel++)
		{
			polyCount += m_pMeshNode->GetNumPolygons(iLodLevel);
		}

		if (polyCount > 0)
		{
			if (m_blindDataMappingIndex != -1)
			{
				m_pMeshNode->DecrementPolygonBlindDataMappingReferenceCount(m_blindDataMappingIndex);
			}

			if (m_materialMappingIndex != -1)
			{
				m_pMeshNode->DecrementPolygonMaterialMappingReferenceCount(m_materialMappingIndex);
			}
		}
	}
	m_pMeshNode = 0;
}

C_Polygon::C_Polygon(const C_Polygon &rhs)
{
	OneTimeInit();

	*this = rhs;
}

Aftk::C_Polygon& C_Polygon::operator= (const C_Polygon &rhs)
{
	if (m_pAttachedData != 0)
		delete m_pAttachedData;

	bool replaceCurrentMaterial = true;
	bool replaceCurrentBlindData = true;
	if (m_pMeshNode)
	{
		//Must compare actual resource ptrs to compensate for remapping.
		if (m_pMeshNode->GetPolygonMaterialMappingPtr(m_materialMappingIndex) == 
			rhs.m_pMeshNode->GetPolygonMaterialMappingPtr(rhs.m_materialMappingIndex))
		{
			replaceCurrentMaterial = false;
		}

		//Must compare actual resource ptrs to compensate for remapping.
		if (m_pMeshNode->GetPolygonBlindDataMappingPtr(m_blindDataMappingIndex) ==
			rhs.m_pMeshNode->GetPolygonBlindDataMappingPtr(rhs.m_blindDataMappingIndex))
		{
			replaceCurrentBlindData = false;
		}
	}

	if (m_pMeshNode)
	{
		if (m_blindDataMappingIndex != -1)
		{
			//Must compare actual resource ptrs to compensate for remapping.
			if (replaceCurrentBlindData)
			{
				m_pMeshNode->DecrementPolygonBlindDataMappingReferenceCount(m_blindDataMappingIndex);
			}
		}

		if (m_materialMappingIndex != -1)
		{
			//Must compare actual resource ptrs to compensate for remapping.
			if (replaceCurrentMaterial)
			{
				m_pMeshNode->DecrementPolygonMaterialMappingReferenceCount(m_materialMappingIndex);
			}
		}
	}

	m_polygonVertexIndices	=	rhs.m_polygonVertexIndices;

	//If we're copying within the same mesh, we don't need to perform material remapping
	if (m_pMeshNode == rhs.m_pMeshNode)
	{
		m_blindDataMappingIndex =	rhs.m_blindDataMappingIndex;
		m_materialMappingIndex	 =	rhs.m_materialMappingIndex;

		if (m_pMeshNode)
		{
			if (m_blindDataMappingIndex != -1)
			{
				if (replaceCurrentBlindData)
				{
					//Start using new blind data - increase reference count
					m_pMeshNode->IncrementPolygonBlindDataMappingReferenceCount(m_blindDataMappingIndex);
				}
			}			
			if (m_materialMappingIndex != -1)
			{
				if (replaceCurrentMaterial)
				{
					//Start using new material - increase reference count
					m_pMeshNode->IncrementPolygonMaterialMappingReferenceCount(m_materialMappingIndex);
				}
			}
		}
	}
	else
	{
		//Current mesh is null.
		if (!m_pMeshNode)
		{
			m_blindDataMappingIndex =	rhs.m_blindDataMappingIndex;
			m_materialMappingIndex	 =	rhs.m_materialMappingIndex;

			#ifdef _DEBUG
			AFTK_ASSERT(m_blindDataMappingIndex < rhs.m_pMeshNode->m_polygonBlindDataMappingArray.Size());
			#endif
			if (m_blindDataMappingIndex != -1)
			{
				//Start using new blind data - increase reference count
				rhs.m_pMeshNode->IncrementPolygonBlindDataMappingReferenceCount(m_blindDataMappingIndex);
			}
			#ifdef _DEBUG
			AFTK_ASSERT(m_materialMappingIndex < rhs.m_pMeshNode->m_polygonMaterialMappingArray.Size());
			#endif
			if (m_materialMappingIndex != -1)
			{
				//Start using new material - increase reference count
				rhs.m_pMeshNode->IncrementPolygonMaterialMappingReferenceCount(m_materialMappingIndex);
			}
		}
		//Else, this mesh is _not_ null and we're copying from another mesh.
		else
		{
			if (rhs.m_blindDataMappingIndex != -1)
			{
				//Start using new blind data - increase reference count
				rhs.m_pMeshNode->IncrementPolygonBlindDataMappingReferenceCount(rhs.m_blindDataMappingIndex);
			}
			if (rhs.m_materialMappingIndex != -1)
			{
				//Start using new material - increase reference count
				rhs.m_pMeshNode->IncrementPolygonMaterialMappingReferenceCount(rhs.m_materialMappingIndex);
			}

			//We are already using different blind data, decrease its reference count.
			m_pMeshNode->DecrementPolygonBlindDataMappingReferenceCount(m_blindDataMappingIndex);

			//We are already using a different material, decrease its reference count.
			m_pMeshNode->DecrementPolygonMaterialMappingReferenceCount(m_materialMappingIndex);

			//Copy indices
			m_blindDataMappingIndex =	rhs.m_blindDataMappingIndex;
			m_materialMappingIndex	 =	rhs.m_materialMappingIndex;
		}

		//Associate this poly with the rhs poly mesh.
		m_pPolygonCluster =	rhs.m_pPolygonCluster;
		m_pMeshNode	= rhs.m_pMeshNode;
	}

	m_basePolygonIndex		=	rhs.m_basePolygonIndex;

	// copy source to destination
	if (rhs.m_pAttachedData != 0)
	{
		m_pAttachedData = rhs.m_pAttachedData->CreateClone();
	}
	else
	{
		m_pAttachedData = 0;
	}
	return *this;
}

void C_Polygon::SetPolygonVertexInitialCapacity(U32 capacity)
{
	if (capacity > (U32)m_polygonVertexIndices.Capacity())
	{
		m_polygonVertexIndices.Reserve(capacity);
	}
}

U32 C_Polygon::AddPolygonVertexIndex(U32 index)
{
	m_polygonVertexIndices.PushBack(index);
	return (U32)m_polygonVertexIndices.Size()-1;
}

void C_Polygon::SetPolygonVertexIndex(U32 index, U32 instance)
{
	m_polygonVertexIndices[index] = instance;
}

void C_Polygon::SetMaterialMappingIndex(S16 index)
{
	if (index == -1)
	{
		DeleteMaterial();
		return;
	}

	AFTK_ASSERT(m_pMeshNode != 0);
	AFTK_ASSERT(index < m_pMeshNode->GetNumPolygonMaterialMappings());

	S16 oldIndex = m_materialMappingIndex;

	//Don't do anything if we're already using this material.
	if (m_pMeshNode->GetPolygonMaterialMappingPtr(m_materialMappingIndex) ==
		m_pMeshNode->GetPolygonMaterialMappingPtr(index))
		return;

	//Start using new material - increase reference count
	m_pMeshNode->IncrementPolygonMaterialMappingReferenceCount(index);
	m_materialMappingIndex = index;

	//Stop using the old material - decrease reference count
	if (oldIndex != -1)
		m_pMeshNode->DecrementPolygonMaterialMappingReferenceCount(oldIndex);
}

void C_Polygon::SetBlindDataMappingIndex(S16 index)
{
	if (index == -1)
	{
		DeleteBlindData();
		return;
	}
	AFTK_ASSERT(m_pMeshNode != 0);
	AFTK_ASSERT(index < m_pMeshNode->GetNumPolygonBlindDataMappings());

	S16 oldIndex = m_blindDataMappingIndex;

	//Don't do anything if we're already using this material.
	if (m_pMeshNode->GetPolygonBlindDataMappingPtr(m_blindDataMappingIndex) ==
		m_pMeshNode->GetPolygonBlindDataMappingPtr(index))
		return;

	//Start using new blind data - increase reference count
	m_pMeshNode->IncrementPolygonBlindDataMappingReferenceCount(index);
	m_blindDataMappingIndex = index;

	//Stop using the old blind data - decrease reference count
	if (oldIndex != -1)
		m_pMeshNode->DecrementPolygonBlindDataMappingReferenceCount(oldIndex);
}

void C_Polygon::SetPolygonVertex(U32 instance, const C_PolygonVertex& pVert)
{
	//Check for case where we cannot add a polygon vertex as the polygon not attached to a mesh.
	AFTK_ASSERT(m_pMeshNode != 0);

	m_pMeshNode->SetPolygonVertex(m_polygonVertexIndices[instance], pVert);
}

void C_Polygon::DeleteBlindData()
{
	AFTK_ASSERT(m_pMeshNode);

	if (m_blindDataMappingIndex != -1)
	{
		//Decrease old node reference count
		m_pMeshNode->DecrementPolygonBlindDataMappingReferenceCount(m_blindDataMappingIndex);
		m_blindDataMappingIndex = -1;
	}
}

void C_Polygon::DeleteMaterial()
{
	AFTK_ASSERT(m_pMeshNode);

	if (m_materialMappingIndex != -1)
	{
		//Decrease old node reference count
		m_pMeshNode->DecrementPolygonMaterialMappingReferenceCount(m_materialMappingIndex);
		m_materialMappingIndex = -1;
	}
}

const C_AttachData* C_Polygon::GetAttachDataPtr() const
{
	return m_pAttachedData;
}

C_AttachData* C_Polygon::GetAttachDataPtr()
{
	return m_pAttachedData;
}

U32 C_Polygon::GetNumPolygonVertices() const
{
	return (U32)m_polygonVertexIndices.Size();
}

U32 C_Polygon::GetNumTriangles() const
{
	if (m_polygonVertexIndices.Size() < 3)
	{
		//There should never be < 3 pVerts.
		return 0;
	}

	return U32((U32)m_polygonVertexIndices.Size() - 2);
}

Normal3 C_Polygon::GetNormal(const Matrix44* pMatrix, bool* pIsDegenerate) const
{
	Vector3F32 normal;
	normal[0] = 0.f;
	normal[1] = 0.f;
	normal[2] = 0.f;
	if (m_polygonVertexIndices.Size() < 3)
	{
		normal[1] = 1.f;
		AFTK_ASSERT(m_polygonVertexIndices.Size() >= 3);
	}
	else
	{
		float d = 0.f;

		U32 degCount = 0;
		for (U32 iTri = 0; iTri < GetNumTriangles(); iTri++)
		{
			const  C_PolygonVertex* triPV[3]; 
			GetTrianglePolygonVertices(iTri, triPV);

			const Position3& v0 = m_pMeshNode->GetVertexPosition(triPV[0]->GetPositionIndex());
			const Position3& v1 = m_pMeshNode->GetVertexPosition(triPV[1]->GetPositionIndex());
			const Position3& v2 = m_pMeshNode->GetVertexPosition(triPV[2]->GetPositionIndex());

			(Vector3F32&)normal = 
				(Vector3F32&)normal + 
				Geometry::GetTriangleNormal(v0,v1,v2, false, pMatrix, pIsDegenerate);
			if (pIsDegenerate)
				if (*pIsDegenerate)
					degCount++;
		}

		if (pIsDegenerate)
		{
			if (degCount == GetNumTriangles())
				*pIsDegenerate = true;
			else
				*pIsDegenerate = false;
		}

		UtMaths::SafeNormalise(&normal);
	}
	return (Normal3&)normal;
}

F32 C_Polygon::GetArea(const Matrix44* pMatrix) const
{
	if (m_polygonVertexIndices.Size() < 3)
	{
		return 0.f;
	}

	bool bDegenerate;
	Normal3 normal = GetNormal(pMatrix, &bDegenerate);
	if (bDegenerate)
	{
		return 0.f;
	}

	F32 area = 0.f;
	F32 ax = fabsf(normal[E_Axis_X]);
	F32 ay = fabsf(normal[E_Axis_Y]);
	F32 az = fabsf(normal[E_Axis_Z]);
	E_Axis axisToIgnore;

    if ((ax > ay) && (ax > az))
		axisToIgnore = E_Axis_X;
    else if (ay > az) 
		axisToIgnore = E_Axis_Y;
	else
	{
		axisToIgnore = E_Axis_Z;
	}


	U32 i,j,k;
    for (i=1, j=2, k = 0; i<=(U32)m_polygonVertexIndices.Size(); i++, j++, k++)
	{
		const C_PolygonVertex* pPV1 = &m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[k]);

		const C_PolygonVertex* pPV2;
		if (i == (U32)m_polygonVertexIndices.Size())
			pPV2 = &m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[0]);
		else
			pPV2 = &m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[i]);

		const C_PolygonVertex* pPV3;
		if (j == (U32)m_polygonVertexIndices.Size())
			pPV3 = &m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[0]);
		else if (j == (U32)m_polygonVertexIndices.Size()+1)
			pPV3 = &m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[1]);
		else
			pPV3 = &m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[j]);

		Position3 pos1 = m_pMeshNode->GetVertexPosition(pPV1->GetPositionIndex());
		Position3 pos2 = m_pMeshNode->GetVertexPosition(pPV2->GetPositionIndex());
		Position3 pos3 = m_pMeshNode->GetVertexPosition(pPV3->GetPositionIndex());
		if (pMatrix)
		{
			pos1 = ApplyMatrix(*pMatrix, pos1);
			pos2 = ApplyMatrix(*pMatrix, pos2);
			pos3 = ApplyMatrix(*pMatrix, pos3);
		}

		switch(axisToIgnore)
		{
		case E_Axis_X:
			area += pos2[E_Axis_Y] * (pos3[E_Axis_Z] - pos1[E_Axis_Z]);
			continue;
		case E_Axis_Y:
			area += pos2[E_Axis_X] * (pos3[E_Axis_Z] - pos1[E_Axis_Z]);
			continue;
		case E_Axis_Z:
			area += pos2[E_Axis_X] * (pos3[E_Axis_Y] - pos1[E_Axis_Y]);
			continue;
		}
	}

	//Get area prior to projection into 2D
    switch (axisToIgnore) 
	{
    case E_Axis_X:
        area *= (1 / (2*ax));
        break;
    case E_Axis_Y:
        area *= (1 / (2*ay));
        break;
    case E_Axis_Z:
        area *= (1 / (2*az));
		break;
    }
    return fabsf(area);
}

C_Status C_Polygon::GetTrianglePolygonVertices(U32 instance, const C_PolygonVertex** ppTriangle) const
{
	//Without being attached to a mesh we cannot return the triangle polygon vertices.
	if (!m_pMeshNode)
	{
		AFTK_ASSERT(m_pMeshNode && "C_Polygon is not attached to a mesh!");
		return C_Status::E_Status_Failure;
	}

	if (m_polygonVertexIndices.Size() < 3)
	{
		return C_Status(C_Status::E_Status_Failure, "C_Polygon has less than three polygon vertices.");
	}

	// create new triangle
	ppTriangle[0] = &m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[0]);
	ppTriangle[1] = &m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[instance + 1]);
	ppTriangle[2] = &m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[instance + 2]);

	return C_Status(C_Status::E_Status_Success);
}

C_Status C_Polygon::GetTrianglePolygonVertexIndices(U32 instance, U32* pTriangle) const
{
	if (m_polygonVertexIndices.Size() < 3)
	{
		return C_Status(C_Status::E_Status_Failure, "Polygon has less than three polygon vertices.");
	}

	// create new triangle
	pTriangle[0] = m_polygonVertexIndices[0];
	pTriangle[1] = m_polygonVertexIndices[instance + 1];
	pTriangle[2] = m_polygonVertexIndices[instance + 2];

	return C_Status(C_Status::E_Status_Success);
}

void C_Polygon::SetTrianglePolygonVertexIndices(U32 instance, const U32* pTriangle) 
{
	m_polygonVertexIndices[0] = pTriangle[0];
	m_polygonVertexIndices[instance + 1] = pTriangle[1];
	m_polygonVertexIndices[instance + 2] = pTriangle[2];
}

const C_PolygonVertex& C_Polygon::GetPolygonVertex(U32 instance) const
{
	//We can only return a polygon vertex if we're attached to a mesh.
	AFTK_ASSERT(m_pMeshNode);

	return m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[instance]);
}

C_PolygonVertex& C_Polygon::GetPolygonVertex(U32 instance)
{
	//We can only return a polygon vertex if we're attached to a mesh.
	AFTK_ASSERT(m_pMeshNode);

	return m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[instance]);
}

U32 C_Polygon::GetPolygonVertexIndex(U32 instance) const
{
	return m_polygonVertexIndices[instance];
}

S16 C_Polygon::GetMaterialMappingIndex() const
{
	return m_materialMappingIndex;
}

S16 C_Polygon::GetBlindDataMappingIndex() const
{
	return m_blindDataMappingIndex;
}

void C_Polygon::SetAttachData(const C_AttachData& attachData)
{
	m_pAttachedData = attachData.CreateClone();
}

void C_Polygon::DeletePolygonVertexIndex(U32 instance)
{
	m_polygonVertexIndices.EraseByIndex(instance);
}

void C_Polygon::DeletePolygonVertexIndex()
{
	if (m_polygonVertexIndices.Size() > 0)
		m_polygonVertexIndices.PopBack();
}

void C_Polygon::DeleteAttachData()
{
	if (m_pAttachedData != 0)
	{
		delete m_pAttachedData;
		m_pAttachedData = 0;
	}
}

C_SceneMeshNode* C_Polygon::GetMeshNodePtr()
{
	return m_pMeshNode;
}

const C_SceneMeshNode* C_Polygon::GetMeshNodePtr() const
{
	return m_pMeshNode;
}

U32 C_Polygon::Validate() const
{
	C_Polygon* pThis = const_cast<C_Polygon*>(this);
	return pThis->ProcessProblems(false);
}

U32 C_Polygon::ProcessProblems(bool fix)
{
	if (!m_pMeshNode)
	{
		AFTK_DO_BEGIN << "C_Polygon is not attached to a mesh.\n" AFTK_DO_DEBUG_END;
		AFTK_ASSERT(false);
		return 1;
	}
	U32 errorCount = 0;

	if ((U32)m_polygonVertexIndices.Size() > 256)
	{
		AFTK_DO_BEGIN << "Mesh " << m_pMeshNode->GetName() << " C_Polygon is invalid - too many polygon vertices (max 256).\n" AFTK_DO_DEBUG_END;
		AFTK_ASSERT(false);
		errorCount++;				
	}

	for (U32 i = 0; i < (U32)m_polygonVertexIndices.Size(); i++)
	{
		if (m_polygonVertexIndices[i] > (U32)m_pMeshNode->GetNumPolygonVertices())
		{
			AFTK_DO_BEGIN << "Mesh " << m_pMeshNode->GetName() << " C_Polygon PolyVertexIndex " << i << " is invalid " << m_polygonVertexIndices[i] << "/" << m_pMeshNode->GetNumPolygonVertices() << "\n" AFTK_DO_DEBUG_END;
#ifdef _DEBUG
			AFTK_ASSERT(false);
#endif
			errorCount++;			
		}
	}

	const C_SharedResource<C_MaterialNode>* pMaterialResource = 
		m_pMeshNode->GetPolygonMaterialMappingPtr(m_materialMappingIndex);
	if (pMaterialResource)
	{
		const C_MaterialNode* pMaterialNode;
		pMaterialNode = &m_pMeshNode->GetScenePtr()->GetMaterialResourcePtr(pMaterialResource->GetIndex())->GetContent();
		if (pMaterialNode == 0)
		{
			AFTK_DO_BEGIN << "Mesh " << m_pMeshNode->GetName() << " C_Polygon has an invalid material index of " << pMaterialResource->GetIndex() << " in its shared resource.\n" AFTK_DO_DEBUG_END;
#ifdef _DEBUG
			AFTK_ASSERT(false);
#endif
			errorCount++;
		}
	}

	if (!m_pPolygonCluster)
	{
		AFTK_DO_BEGIN << "Mesh " << m_pMeshNode->GetName() << " C_Polygon has no polygon cluster.\n" AFTK_DO_DEBUG_END;
		errorCount++;
		#ifdef _DEBUG
		AFTK_ASSERT(false);
		#endif
	}

	return errorCount;
}

S32 C_Polygon::GetBasePolygonIndex() const
{
	return m_basePolygonIndex;
}

C_PolygonCluster* C_Polygon::GetClusterPtr()
{
	return m_pPolygonCluster;
}

const C_PolygonCluster* C_Polygon::GetClusterPtr() const
{
	return m_pPolygonCluster;
}

void C_Polygon::ReverseWindingOrder()
{
	Ut::UtVector<Ut::DynamicVector<U32> > indices;
	indices.Resize(m_polygonVertexIndices.Size());
	for (U32 i = 0, newIndex = (U32)m_polygonVertexIndices.Size()-1; i < (U32)m_polygonVertexIndices.Size(); i++, newIndex--)
		indices[newIndex] = m_polygonVertexIndices[i];
	
	for (U32 i = 0; i < (U32)indices.Size(); i++)
		m_polygonVertexIndices[i] = indices[i];
}

// Protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

#pragma optimize ("", off)
void C_Polygon::Clear()
{
	if (m_pAttachedData != 0)
	{
		delete m_pAttachedData;
		m_pAttachedData = 0;
	}

	if (m_pMeshNode != 0)
	{
		if (m_blindDataMappingIndex != -1)
		{
			m_pMeshNode->DecrementPolygonBlindDataMappingReferenceCount(m_blindDataMappingIndex);
		}

		if (m_materialMappingIndex != -1)
		{
			m_pMeshNode->DecrementPolygonMaterialMappingReferenceCount(m_materialMappingIndex);
		}
	}

	m_polygonVertexIndices.Clear();
	m_materialMappingIndex = -1;
	m_blindDataMappingIndex = -1;
	m_pMeshNode = 0;
	m_pPolygonCluster = 0;
}
#pragma optimize ("", on)

void C_Polygon::AttachToMesh(C_SceneMeshNode* pMeshNode)
{
	AFTK_ASSERT(pMeshNode);
	if (m_pMeshNode == pMeshNode) return;

	//Adjust material and blind data reference counts for the mesh we're disassociating with.
	if (m_pMeshNode)
	{
		if (m_blindDataMappingIndex != -1)
		{
			m_pMeshNode->DecrementPolygonBlindDataMappingReferenceCount(m_blindDataMappingIndex);
		}
		if (m_materialMappingIndex != -1)
		{
			m_pMeshNode->DecrementPolygonMaterialMappingReferenceCount(m_materialMappingIndex);
		}		
	}

	//Use new mesh.
	m_pMeshNode = pMeshNode;

	if (m_pMeshNode)
	{
		if (m_materialMappingIndex != -1)
		{
			m_pMeshNode->IncrementPolygonMaterialMappingReferenceCount(m_materialMappingIndex);
		}
		if (m_blindDataMappingIndex != -1)
		{
			m_pMeshNode->IncrementPolygonBlindDataMappingReferenceCount(m_blindDataMappingIndex);
		}		
	}
}

void C_Polygon::SetAdjacencyIndex(S32 index) const
{
	m_iAdjacencyIndex = index;
}

void C_Polygon::SetBasePolygonIndex(S32 index)
{
	m_basePolygonIndex = index;
}

S32 C_Polygon::GetAdjacentPolygonIndex(U32 vert) const
{
	if (m_iAdjacencyIndex == -1)
		m_pMeshNode->BuildAdjacency();

    return m_pMeshNode->GetAdjacencyList(m_iAdjacencyIndex)[vert];
}

void C_Polygon::SetClusterPtr(C_PolygonCluster* pPolyCluster)
{
	AFTK_ASSERT(pPolyCluster);
	m_pPolygonCluster = pPolyCluster;
}

void C_Polygon::ShuffleBlindDataMappingIndex()
{
	AFTK_ASSERT(m_blindDataMappingIndex > 0);
	m_blindDataMappingIndex--;
}

void C_Polygon::ShuffleMaterialMappingIndex()
{
	AFTK_ASSERT(m_materialMappingIndex > 0);
	m_materialMappingIndex--;
}

bool C_Polygon::TriangleIsAnEar(
	const Matrix44* pMatrix,
	const Vector3F32& n0,
	S32 pvi1, S32 pvi2, S32 pvi3)
{
	const F32 earEpsilon = 1e-10f;

	S32 meshRelPVIndex1 = m_polygonVertexIndices[pvi1];
	S32 meshRelPVIndex2 = m_polygonVertexIndices[pvi2];
	S32 meshRelPVIndex3 = m_polygonVertexIndices[pvi3];

	const C_PolygonVertex& pv1 = m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[pvi1]);
	const C_PolygonVertex& pv2 = m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[pvi2]);
	const C_PolygonVertex& pv3 = m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[pvi3]);

	S32 vPos1 = pv1.GetPositionIndex();
	S32 vPos2 = pv2.GetPositionIndex();
	S32 vPos3 = pv3.GetPositionIndex();

	Vector3F32 p1 = (Vector3F32&)m_pMeshNode->GetVertexPosition(vPos1);
	Vector3F32 p2 = (Vector3F32&)m_pMeshNode->GetVertexPosition(vPos2);
	Vector3F32 p3 = (Vector3F32&)m_pMeshNode->GetVertexPosition(vPos3);

	//Transform points into the same space as n0.
	if (pMatrix)
	{
		p1 = ApplyMatrix(*pMatrix, p1);
		p2 = ApplyMatrix(*pMatrix, p2);
		p3 = ApplyMatrix(*pMatrix, p3);
	}

	//Condition 3 check.
	//Condition 3: Triangle does not contain any of the polygons active vertices.
	//Active vertices are those that have not been used in triangles yet.
	Vector3F32 delta1(p2 - p1);
	Vector3F32 delta2(p3 - p2);
	Vector3F32 delta3(p1 - p3);

	Vector3F32 n1 = n0 ^ delta1;
	Vector3F32 n2 = n0 ^ delta2;
	Vector3F32 n3 = n0 ^ delta3;
	UtMaths::SafeNormalise(&n1);
	UtMaths::SafeNormalise(&n2);
	UtMaths::SafeNormalise(&n3);

	//We need to know the orientation of the triangle for our collision detection test later on.
	bool counterClockwise;
	{
		if ((n1 | (p3 - p1)) > 0)
			counterClockwise = true;
		else
			counterClockwise = false;
	}
	for (U32 iPV = 0; iPV < (U32)m_polygonVertexIndices.Size(); iPV++)
	{
		//If this vertex is active, and is not one of the vertices currently under
		//consideration, then...
		if (
			(iPV != pvi1) &&
			(iPV != pvi2) &&
			(iPV != pvi3))
		{
			//Get vertex position q.
			const C_PolygonVertex& pvq = m_pMeshNode->GetPolygonVertex(m_polygonVertexIndices[iPV]);
			S32 vPosq = pvq.GetPositionIndex();
			Vector3F32 q = (const Vector3F32&)m_pMeshNode->GetVertexPosition(vPosq);
			if (pMatrix)
				q = ApplyMatrix(*pMatrix, q);

			Vector3F32 delta1 = (q - p1);
			Vector3F32 delta2 = (q - p2);
			Vector3F32 delta3 = (q - p3);
			UtMaths::SafeNormalise(&delta1);
			UtMaths::SafeNormalise(&delta2);
			UtMaths::SafeNormalise(&delta3);

			if (counterClockwise)
			{
				if (
					((n1 | delta1) > -earEpsilon) &&
					((n2 | delta2) > -earEpsilon) &&
					((n3 | delta3) > -earEpsilon)
				)
				{
					//Triangle is not an ear.
					return false;
				}
			}
			else
			{
				if (
					(!((n1 | delta1) > -earEpsilon)) &&
					(!((n2 | delta2) > -earEpsilon)) &&
					(!((n3 | delta3) > -earEpsilon))
				)
				{
					return false;
				}
			}
		}
	}

	return true;
}

void C_Polygon::AddNewTriangle(
	Ut::UtVector<Ut::DynamicVector<C_Polygon> >& polygonArray,
	S32& firstEmittedTriangleIndex,
	S32 pvi1, S32 pvi2, S32 pvi3)
{
	//std::cout << pvi1 << " " << pvi2 << " " << pvi3 << std::endl;
	const C_PolygonVertex& pv1 = m_pMeshNode->GetPolygonVertex(pvi1);
	const C_PolygonVertex& pv2 = m_pMeshNode->GetPolygonVertex(pvi2);
	const C_PolygonVertex& pv3 = m_pMeshNode->GetPolygonVertex(pvi3);

	S32 vPos1 = pv1.GetPositionIndex();
	S32 vPos2 = pv2.GetPositionIndex();
	S32 vPos3 = pv3.GetPositionIndex();

	Vector3F32 p1 = (Vector3F32&)m_pMeshNode->GetVertexPosition(vPos1);
	Vector3F32 p2 = (Vector3F32&)m_pMeshNode->GetVertexPosition(vPos2);
	Vector3F32 p3 = (Vector3F32&)m_pMeshNode->GetVertexPosition(vPos3);

	U32 newTriIndex = (U32)polygonArray.Size();
	polygonArray.PushBack();
	C_Polygon* pNewPoly = &polygonArray[newTriIndex];
	//Attach polygon to mesh.
	pNewPoly->AttachToMesh(m_pMeshNode);

	if (firstEmittedTriangleIndex == -1)
	{
		pNewPoly->m_basePolygonIndex = -1;
		firstEmittedTriangleIndex = newTriIndex;
	}
	else
		pNewPoly->m_basePolygonIndex = firstEmittedTriangleIndex;
	//Add polygon vertices.
	pNewPoly->AddPolygonVertexIndex(pvi1);
	pNewPoly->AddPolygonVertexIndex(pvi2);
	pNewPoly->AddPolygonVertexIndex(pvi3);

	//Manually add to poly cluster.
	AFTK_ASSERT(m_pPolygonCluster);
	pNewPoly->m_pPolygonCluster = m_pPolygonCluster;

	//Copy materials and blind data.
	pNewPoly->SetMaterialMappingIndex(m_materialMappingIndex);
	pNewPoly->SetBlindDataMappingIndex(m_blindDataMappingIndex);

	//Copy attached data.
	if (m_pAttachedData != 0)
	{
		pNewPoly->SetAttachData(*m_pAttachedData);
	}

	Normal3 temp = pNewPoly->GetNormal(0,0);
}



U32 C_Polygon::Triangulate(
	Ut::UtVector<Ut::DynamicVector<C_Polygon> >& polygonArray,
	Ut::UtVector<Ut::DynamicVector<bool> >& activeVertexArray,
	Ut::UtVector<Ut::DynamicVector<bool> >& vertexClassification,
	const Matrix44* pMatrix)
{
	if (!m_pMeshNode)
		//Without being associated with a mesh, we cannot access the vertex data needed to perform
		//triangulation.
		return 0;

	U32 trianglesEmitted = 0;
	if (m_polygonVertexIndices.Size() == 3) 
	{
		//Add the only triangle of this poly to the new poly list.
		polygonArray.PushBack(*this);
		return 1;
	}

	C_TriangleIterator triIt(*this, pMatrix);
	S32 firstTriIndex = -1;
	while(triIt)
	{
		const U32* t = triIt;
		AddNewTriangle(polygonArray, firstTriIndex, t[0], t[1], t[2]);
		if (firstTriIndex == -1)
		{
			firstTriIndex = (U32)polygonArray.Size();
		}
		triIt++;
		trianglesEmitted++;
	}

	return trianglesEmitted;
}

C_Polygon::C_Polygon()
{
	OneTimeInit();
}

void C_Polygon::OneTimeInit()
{
	m_pMeshNode = 0;
	m_blindDataMappingIndex = -1;
	m_materialMappingIndex = -1;
	m_pPolygonCluster = 0;

	m_pAttachedData = 0;
	m_iAdjacencyIndex = -1;
	m_basePolygonIndex = -1;
}

bool C_Polygon::operator==(const C_Polygon& rhs) const
{
	if (memcmp(&m_polygonVertexIndices[0], &rhs.m_polygonVertexIndices[0], sizeof(U32)*m_polygonVertexIndices.Size()) != 0)
	{
		return false;
	}

	//use this comparison if polys are on same mesh.
	if (m_pMeshNode == rhs.m_pMeshNode)
	{
		if ((m_materialMappingIndex != rhs.m_materialMappingIndex) ||
			(m_blindDataMappingIndex != rhs.m_blindDataMappingIndex) ||
			(m_pPolygonCluster != rhs.m_pPolygonCluster))
		{
			return false;
		}
	}
	//
	else
	{
		if ((m_materialMappingIndex != rhs.m_materialMappingIndex) ||
			(m_blindDataMappingIndex != rhs.m_blindDataMappingIndex))
		{
			return false;
		}
	}

	return true;
}

#endif
