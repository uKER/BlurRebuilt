#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_MESH_WELD_MEMORY_MANAGER_H
#define AFTK_MESH_WELD_MEMORY_MANAGER_H

//1 of each elemnt = 104 bytes worth of data
//so, preallocating 100,000 of each = ~10Mb
#define AFTK_MESH_WELD_MEMORY_MANAGER_LARGE_CAPACITY 100000
#define AFTK_MESH_WELD_MEMORY_MANAGER_PRESET_CAPACITY 1000

namespace Aftk
{
	class C_MeshWeldMemoryManager
	{
	public:
		C_MeshWeldMemoryManager(U32 capacity = AFTK_MESH_WELD_MEMORY_MANAGER_PRESET_CAPACITY);

		~C_MeshWeldMemoryManager();

		void GetSortedIndexArray(
			Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >** ppArray);

		void GetRemappingArray(
			Ut::UtVector< Ut::DynamicVector<S32> >** ppArray);

		void GetComponentsToSkipArray(
			Ut::UtVector< Ut::DynamicVector<bool> >** ppArray);

		void GetIntermediateArray(
			Ut::UtVector< Ut::DynamicVector<Vector3F32> >** ppArray);

		void GetTransformedArray(
			Ut::UtVector< Ut::DynamicVector<Vector3F32> >** ppArray);

		void GetIntermediateArray(
			Ut::UtVector< Ut::DynamicVector<Vector4F32> >** ppArray);

		void GetTransformedArray(
			Ut::UtVector< Ut::DynamicVector<Vector4F32> >** ppArray);

		void GetIntermediateArray(
			Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >** ppArray);
		
	private:
		Ut::UtVector< Ut::DynamicVector<C_MeshWeldComponentAxis> >	m_sortedIndexList;
		Ut::UtVector< Ut::DynamicVector<S32> >				m_remappingArray;
		Ut::UtVector< Ut::DynamicVector<bool> >				m_componentsToSkipArray;

		Ut::UtVector< Ut::DynamicVector<Vector3F32> >		m_intermediateVector3F32Array;
		Ut::UtVector< Ut::DynamicVector<Vector3F32> >		m_transformedVector3F32Array;

		Ut::UtVector< Ut::DynamicVector<Vector4F32> >		m_intermediateVector4F32Array;
		Ut::UtVector< Ut::DynamicVector<Vector4F32> >		m_transformedVector4F32Array;

		Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >	m_intermediatePolygonVertexArray;
	};
}

#endif
