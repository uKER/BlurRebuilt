#ifndef AFTK_RAY_MESH_RESULT_H
#define AFTK_RAY_MESH_RESULT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_RayMeshResult
	{
	public:
		C_RayMeshResult();

		C_RayMeshResult(
			const Position3& intersectionPoint,
			I_SceneMeshNode* pISceneMeshNode, 
			S32 polygonIndex,
			S32 polygonTriangleIndex);

		C_RayMeshResult(const C_RayMeshResult& rhs);

		bool Hit() const;

		const Position3& GetIntersectionPoint() const;

		S32 GetPolygonTriangleIndex() const;

		S32 GetPolygonIndex() const;

		I_SceneMeshNode* GetMeshNodePtr();

		const I_SceneMeshNode* GetMeshNodePtr() const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		void Set(
			const Position3& intersectionPoint,
			I_SceneMeshNode* pISceneMeshNode, 
			S32 polygonIndex,
			S32 polygonTriangleIndex);

		I_SceneMeshNode* m_pISceneMeshNode;
		S32 m_polygonIndex;
		S32 m_polygonTriangleIndex;
		Position3 m_intersectionPoint;

		friend class C_Ray;
	#endif
	};

	FORCE_INLINE C_RayMeshResult::C_RayMeshResult()
	{
		m_pISceneMeshNode = 0;
		m_polygonIndex = -1;
		m_polygonTriangleIndex = -1;
		memset(&m_intersectionPoint, 0, sizeof(Position3));
	}

	FORCE_INLINE C_RayMeshResult::C_RayMeshResult(
		const Position3& intersectionPoint,
		I_SceneMeshNode* pISceneMeshNode, 
		S32 polygonIndex,
		S32 polygonTriangleIndex)
	{
		Set(
			intersectionPoint,
			pISceneMeshNode,
			polygonIndex,
			polygonTriangleIndex);
	}

	FORCE_INLINE C_RayMeshResult::C_RayMeshResult(const C_RayMeshResult& rhs)
	{
		*this = rhs;
	}

	FORCE_INLINE bool C_RayMeshResult::Hit() const
	{
		if ((m_pISceneMeshNode != 0) && (m_polygonIndex != -1) && (m_polygonTriangleIndex != -1))
			return true;
		return false;
	}

	FORCE_INLINE const Position3& C_RayMeshResult::GetIntersectionPoint() const
	{
		return m_intersectionPoint;
	}

	FORCE_INLINE S32 C_RayMeshResult::GetPolygonTriangleIndex() const
	{
		return m_polygonTriangleIndex;
	}

	FORCE_INLINE S32 C_RayMeshResult::GetPolygonIndex() const
	{
		return m_polygonIndex;
	}

	FORCE_INLINE I_SceneMeshNode* C_RayMeshResult::GetMeshNodePtr()
	{
		return m_pISceneMeshNode;
	}

	FORCE_INLINE const I_SceneMeshNode* C_RayMeshResult::GetMeshNodePtr() const
	{
		return m_pISceneMeshNode;
	}

	FORCE_INLINE void C_RayMeshResult::Set(
		const Position3& intersectionPoint,
		I_SceneMeshNode* pISceneMeshNode, 
		S32 polygonIndex,
		S32 polygonTriangleIndex)
	{
		m_intersectionPoint = intersectionPoint;
		m_pISceneMeshNode = pISceneMeshNode;
		m_polygonIndex = polygonIndex;
		m_polygonTriangleIndex = polygonTriangleIndex;
	}
}

#endif
