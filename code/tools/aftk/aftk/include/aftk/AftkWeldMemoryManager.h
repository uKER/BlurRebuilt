#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_WELD_MEMORY_MANAGER_H
#define AFTK_WELD_MEMORY_MANAGER_H

namespace Aftk
{
	struct C_ComponentAxis
	{
		C_ComponentAxis() {};
		bool operator < (const C_ComponentAxis &rhs) const
		{
			if (axis < rhs.axis)
			{
				return true;
			}
			return false;
		}

		F32 axis;
		U32 index;
	};

	class C_WeldMemoryManager
	{
	public:
		C_WeldMemoryManager();

		~C_WeldMemoryManager();

		void GetSortedIndexArray(
			Ut::UtVector< Ut::DynamicVector<C_ComponentAxis> >** ppArray);

		void GetRemappingArray(
			Ut::UtVector< Ut::DynamicVector<U32> >** ppArray);

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
		Ut::UtVector< Ut::DynamicVector<C_ComponentAxis> >	m_sortedIndexList;
		Ut::UtVector< Ut::DynamicVector<U32> >				m_remappingArray;
		Ut::UtVector< Ut::DynamicVector<bool> >				m_componentsToSkipArray;

		Ut::UtVector< Ut::DynamicVector<Vector3F32> >		m_intermediateVector3F32Array;
		Ut::UtVector< Ut::DynamicVector<Vector3F32> >		m_transformedVector3F32Array;

		Ut::UtVector< Ut::DynamicVector<Vector4F32> >		m_intermediateVector4F32Array;
		Ut::UtVector< Ut::DynamicVector<Vector4F32> >		m_transformedVector4F32Array;

		Ut::UtVector< Ut::DynamicVector<C_PolygonVertex> >	m_intermediatePolygonVertexArray;
	};
}

#endif
