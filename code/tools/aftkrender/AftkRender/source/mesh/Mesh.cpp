#include "AftkRenderPch.h"

#include "mesh/MeshElements.h"
#include "mesh/Mesh.h"

using namespace AftkRender;


C_MeshClusterResource::C_MeshClusterResource()
: m_pMesh(0)
, m_NonIndexedPrimitiveType(E_PrimitiveType_Unknown)
, m_nVertices(0)
, m_VertexDataElements(0)
{
}

C_MeshClusterResource::~C_MeshClusterResource()
{
	for (S32 i=0; i<m_VertexData.Size(); ++i)
		delete[] m_VertexData[i].pData;

	for (S32 i=0; i<m_IndexData.Size(); ++i)
		delete[] m_IndexData[i].pData;
}

C_MeshResource* C_MeshClusterResource::GetMesh()
{
	return m_pMesh;
}

const C_MeshResource* C_MeshClusterResource::GetMesh() const
{
	return m_pMesh;
}

bool C_MeshClusterResource::AddIndexData(const U32* pData, U32 count, E_PrimitiveType type, S16 id)
{
	if (!pData || !count)
		return false;

	U32 *pNewData = new U32[count*sizeof(U32)];
	if (!pNewData)
		return false;

	memcpy(pNewData, pData, count*sizeof(U32));

	m_IndexData.PushBack();
	m_IndexData.Back().pData = pNewData;
	m_IndexData.Back().count = count;
	m_IndexData.Back().type = type;
	m_IndexData.Back().materialId = id;
	
	return true;
}

bool C_MeshClusterResource::AttachIndexData(U32* pData, U32 count, E_PrimitiveType type, S16 id)
{
	if (!pData || !count)
		return false;

	m_IndexData.PushBack();
	m_IndexData.Back().pData = pData;
	m_IndexData.Back().count = count;
	m_IndexData.Back().type = type;
	m_IndexData.Back().materialId = id;
	
	return true;
}

U32 C_MeshClusterResource::GetNumIndexDataElements() const
{
	return m_IndexData.Size();
}

const C_MeshClusterResource::IndexDataElement* C_MeshClusterResource::GetIndexDataElement(U32 index) const
{
	if (index < (U32)m_IndexData.Size())
		return &m_IndexData[index];
	else
		return 0;
}

void C_MeshClusterResource::SetNumVertices(U32 nVertices)
{
	assert(m_VertexData.Size() == 0);
	m_nVertices = nVertices;
}

U32 C_MeshClusterResource::GetNumVertices() const
{
	return m_nVertices;
}

bool C_MeshClusterResource::AddVertexDataElement(const void* pData, E_ElementType type)
{
	if (m_nVertices == 0 || HasVertexDataElement(type))
		return false;

	U8 *pNewData = new U8[m_nVertices*GetElementSize(type)];
	if (!pData)
		return false;

	memcpy(pNewData, pData, m_nVertices*GetElementSize(type));

	m_VertexData.PushBack();
	m_VertexData.Back().pData = pNewData;
	m_VertexData.Back().type = type;

	//invalidate cached value
	m_VertexDataElements = 0;

	return true;
}

bool C_MeshClusterResource::AttachVertexDataElement(void* pData, E_ElementType type)
{
	if (m_nVertices == 0 || HasVertexDataElement(type))
		return false;

	m_VertexData.PushBack();
	m_VertexData.Back().pData = pData;
	m_VertexData.Back().type = type;

	//invalidate cached value
	m_VertexDataElements = 0;

	return true;
}

bool C_MeshClusterResource::HasVertexDataElement(E_ElementType type) const
{
	if (!m_VertexDataElements)
	{
		for (S32 i=0; i<m_VertexData.Size(); ++i)
			m_VertexDataElements |= m_VertexData[i].type;
	}
	return (m_VertexDataElements & (U32)type) > 0;
}

const void* C_MeshClusterResource::GetVertexDataElement(E_ElementType type) const
{
	for (S32 i=0; i<m_VertexData.Size(); ++i)
		if (m_VertexData[i].type == type)
			return m_VertexData[i].pData;

	return 0;
}

void C_MeshClusterResource::SetNonIndexedPrimitiveType(E_PrimitiveType type)
{
	m_NonIndexedPrimitiveType = type;
}

E_PrimitiveType C_MeshClusterResource::GetNonIndexedPrimitiveType() const
{
	return m_NonIndexedPrimitiveType;
}

C_MeshResource::C_MeshResource()
: m_VertexDataElements(0)
{
}

C_MeshResource::~C_MeshResource()
{
	FreeMeshClusters();
}

U32 C_MeshResource::GetNumClusters() const
{
	return m_Clusters.Size();
}

void C_MeshResource::AddCluster(C_MeshClusterResource *pCluster)
{
	//make sure it's not already owned by another mesh
	assert(pCluster->m_pMesh == 0);

	pCluster->m_pMesh = this;
	m_Clusters.PushBack(pCluster);
}

C_MeshClusterResource* C_MeshResource::GetCluster(U32 index)
{
	if (index>=(U32)m_Clusters.Size())
		return 0;

	return m_Clusters[index];
}

const C_MeshClusterResource* C_MeshResource::GetCluster(U32 index) const
{
	if (index>=(U32)m_Clusters.Size())
		return 0;

	return m_Clusters[index];
}

void C_MeshResource::FreeMeshClusters()
{
	for (S32 i=0; i<m_Clusters.Size(); ++i)
		delete m_Clusters[i];

	m_VertexDataElements = 0;
}

C_MeshClusterResource* C_MeshResource::DetachCluster(U32 index)
{
	if (index>=(U32)m_Clusters.Size())
		return 0;

	C_MeshClusterResource *pReturn = m_Clusters[index];
	m_Clusters.EraseByIndex(index);
	pReturn->m_pMesh = 0;
	return pReturn;
}
