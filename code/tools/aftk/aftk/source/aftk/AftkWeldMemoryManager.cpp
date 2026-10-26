#include <stdafx.h>

using namespace Aftk;

//1 of each elemnt = 104 bytes worth of data
//so, preallocating 100,000 of each = ~10Mb
#define AFTK_WELD_MEMORY_MANAGER_PRESET_CAPACITY 100000

C_WeldMemoryManager::C_WeldMemoryManager()
{
	m_sortedIndexList.Reserve(AFTK_WELD_MEMORY_MANAGER_PRESET_CAPACITY);
	m_remappingArray.Reserve(AFTK_WELD_MEMORY_MANAGER_PRESET_CAPACITY);
	m_componentsToSkipArray.Reserve(AFTK_WELD_MEMORY_MANAGER_PRESET_CAPACITY);

	m_intermediateVector3F32Array.Reserve(AFTK_WELD_MEMORY_MANAGER_PRESET_CAPACITY);
	m_transformedVector3F32Array.Reserve(AFTK_WELD_MEMORY_MANAGER_PRESET_CAPACITY);

	m_intermediateVector4F32Array.Reserve(AFTK_WELD_MEMORY_MANAGER_PRESET_CAPACITY);
	m_transformedVector4F32Array.Reserve(AFTK_WELD_MEMORY_MANAGER_PRESET_CAPACITY);

	m_intermediatePolygonVertexArray.Reserve(AFTK_WELD_MEMORY_MANAGER_PRESET_CAPACITY);
}

C_WeldMemoryManager::~C_WeldMemoryManager()
{
}

void C_WeldMemoryManager::GetSortedIndexArray(
	Ut::UtVector< Ut::DynamicVector<C_ComponentAxis> >** ppArray)
{
	m_sortedIndexList.Clear();
	*ppArray = &m_sortedIndexList;
}

void C_WeldMemoryManager::GetRemappingArray(
	Ut::UtVector< Ut::DynamicVector<U32> >** ppArray)
{
	m_remappingArray.Clear();
	*ppArray = &m_remappingArray;
}

void C_WeldMemoryManager::GetComponentsToSkipArray(
	Ut::UtVector< Ut::DynamicVector<bool> >** ppArray)
{
	m_componentsToSkipArray.Clear();
	*ppArray = &m_componentsToSkipArray;
}

void C_WeldMemoryManager::GetTransformedArray(
	Ut::UtVector< Ut::DynamicVector<Vector3F32> >** ppArray)
{
	m_transformedVector3F32Array.Clear();
	*ppArray = (Ut::UtVector< Ut::DynamicVector<Vector3F32> >*)&m_transformedVector3F32Array;
}

void C_WeldMemoryManager::GetIntermediateArray(
	Ut::UtVector< Ut::DynamicVector<Vector3F32> >** ppArray)
{
	m_intermediateVector3F32Array.Clear();
	*ppArray = (Ut::UtVector< Ut::DynamicVector<Vector3F32> >*)&m_intermediateVector3F32Array;
}

void C_WeldMemoryManager::GetTransformedArray(
	Ut::UtVector< Ut::DynamicVector<Vector4F32> >** ppArray)
{
	m_transformedVector4F32Array.Clear();
	*ppArray = (Ut::UtVector< Ut::DynamicVector<Vector4F32> >*)&m_transformedVector4F32Array;
}

void C_WeldMemoryManager::GetIntermediateArray(
	Ut::UtVector< Ut::DynamicVector<Vector4F32> >** ppArray)
{
	m_intermediateVector4F32Array.Clear();
	*ppArray = (Ut::UtVector< Ut::DynamicVector<Vector4F32> >*)&m_intermediateVector4F32Array;
}

void C_WeldMemoryManager::GetIntermediateArray(
	Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >** ppArray)
{
	m_intermediatePolygonVertexArray.Clear();
	*ppArray = (Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >*)&m_intermediatePolygonVertexArray;
}
