#include <stdafx.h>

using namespace Aftk;

C_MeshWeldMemoryManager::C_MeshWeldMemoryManager(U32 capacity)
{
	m_sortedIndexList.Reserve(capacity);
	m_remappingArray.Reserve(capacity);
	m_componentsToSkipArray.Reserve(capacity);

	m_intermediateVector3F32Array.Reserve(capacity);
	m_transformedVector3F32Array.Reserve(capacity);

	m_intermediateVector4F32Array.Reserve(capacity);
	m_transformedVector4F32Array.Reserve(capacity);

	m_intermediatePolygonVertexArray.Reserve(capacity);
}

C_MeshWeldMemoryManager::~C_MeshWeldMemoryManager()
{
}

void C_MeshWeldMemoryManager::GetSortedIndexArray(
	Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >** ppArray)
{
	m_sortedIndexList.Clear();
	*ppArray = &m_sortedIndexList;
}

void C_MeshWeldMemoryManager::GetRemappingArray(
	Ut::UtVector< Ut::DynamicVector<S32> >** ppArray)
{
	m_remappingArray.Clear();
	*ppArray = &m_remappingArray;
}

void C_MeshWeldMemoryManager::GetComponentsToSkipArray(
	Ut::UtVector< Ut::DynamicVector<bool> >** ppArray)
{
	m_componentsToSkipArray.Clear();
	*ppArray = &m_componentsToSkipArray;
}

void C_MeshWeldMemoryManager::GetTransformedArray(
	Ut::UtVector< Ut::DynamicVector<Vector3F32> >** ppArray)
{
	m_transformedVector3F32Array.Clear();
	*ppArray = (Ut::UtVector< Ut::DynamicVector<Vector3F32> >*)&m_transformedVector3F32Array;
}

void C_MeshWeldMemoryManager::GetIntermediateArray(
	Ut::UtVector< Ut::DynamicVector<Vector3F32> >** ppArray)
{
	m_intermediateVector3F32Array.Clear();
	*ppArray = (Ut::UtVector< Ut::DynamicVector<Vector3F32> >*)&m_intermediateVector3F32Array;
}

void C_MeshWeldMemoryManager::GetTransformedArray(
	Ut::UtVector< Ut::DynamicVector<Vector4F32> >** ppArray)
{
	m_transformedVector4F32Array.Clear();
	*ppArray = (Ut::UtVector< Ut::DynamicVector<Vector4F32> >*)&m_transformedVector4F32Array;
}

void C_MeshWeldMemoryManager::GetIntermediateArray(
	Ut::UtVector< Ut::DynamicVector<Vector4F32> >** ppArray)
{
	m_intermediateVector4F32Array.Clear();
	*ppArray = (Ut::UtVector< Ut::DynamicVector<Vector4F32> >*)&m_intermediateVector4F32Array;
}

void C_MeshWeldMemoryManager::GetIntermediateArray(
	Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >** ppArray)
{
	m_intermediatePolygonVertexArray.Clear();
	*ppArray = (Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >*)&m_intermediatePolygonVertexArray;
}
