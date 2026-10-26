#ifndef EDITOR_LIB_COLLISION_SCENE_H
#define EDITOR_LIB_COLLISION_SCENE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Misc/Status.h>

namespace EditorLib
{
	struct I_CollisionInstance;
	struct C_RayResult;

	struct C_CollisionInstance
	{
		I_DgNode* pDgNode;
		I_CollisionInstance* pInstance;
	};

	//This class can also host accelerators such as grids and trees. TODO
	class C_CollisionScene
	{
	public:
		static const U32 kFaceMask_Front = 0x1;
		static const U32 kFaceMask_Back	 = 0x2;
		static const U32 kFaceMask_All	 = 0x3;

		typedef Btl::Vector<I_CollisionSource*> CollisionSourceArray;
		typedef Btl::Vector<C_CollisionInstance> CollisionInstanceArray;
		typedef Btl::OpenHashMap<I_DgNode*, I_CollisionInstance*> CollisionInstanceMapping;

		C_CollisionScene();
		~C_CollisionScene();

		void Clear();

		const CollisionSourceArray& GetCollisionSourceArray() const;
		const CollisionInstanceArray& GetCollisionInstanceArray() const;
		const CollisionInstanceMapping& GetCollisionInstanceMapping() const;

		//Sources in local space. Takes ownership of shape data passed in.
		C_Status AddCollisionSource(
			I_CollisionSource* pCollisionSource,
			const char* aftkNodePath = 0); //aftk node path if originating from aftk

		C_Status RemoveCollisionSource(
			I_CollisionSource* pCollisionSource);

		C_Status AddCollisionInstance(
			I_DgNode* pDgNode,
			I_CollisionInstance* pSource);

		C_Status RemoveCollisionInstance(
			I_DgNode* pDgNode);

		//Find all intersections (at most one per object).
		U32 Intersect(
			C_RayResult* pResults,
			U32 resultCapacity,
			const C_Ray& ray,
			E_CollisionQueryType queryType,
			U32 sourceTypeFilter = kCollisionSourceType_MaskAll) const;

		bool Intersect(
			C_RayResult& result,
			const C_Ray& ray,
			E_CollisionQueryType queryType,
			U32 sourceTypeFilter = kCollisionSourceType_MaskAll) const;

		//Find nearest object intersection.
		bool Intersect(
			C_RayResult& result,
			const C_Ray& ray,
			const I_CollisionInstance* pInstance,
			E_CollisionQueryType queryType,
			U32 sourceTypeFilter = kCollisionSourceType_MaskAll) const;

		//Nearest point to ray.
		bool NearestPoint(
			C_NearestPointResult& result,
			const C_Ray& ray,
			U32 sourceTypeFilter = kCollisionSourceType_MaskAll) const;

		bool NearestPoint(
			C_NearestPointResult& result, 
			const Vector3& point) const;

	private:
		C_CollisionScene(const C_CollisionScene& rhs);
		const C_CollisionScene& operator=(const C_CollisionScene& rhs);

		bool _IntersectBounds(
			C_RayResult& result, 
			const C_Ray& ray,
			const I_CollisionInstance* pInstance,
			U32 sourceTypeFilter) const;

		bool _IntersectComplex(
			C_RayResult& result, 
			const C_Ray& ray,
			const I_CollisionInstance* pInstance,
			U32 sourceTypeFilter) const;
			
		bool _NearestPointComplex(
			C_NearestPointResult& result, 
			const C_Ray& ray,
			const I_CollisionInstance* pInstance,
			U32 sourceTypeFilter) const;

		bool _IntersectBoundsProxy(
			C_RayResult& result, 
			const C_Ray& ray,
			const C_CollisionProxy* pProxy,
			const Matrix44& toObjectSpace,
			const Matrix44& toWorldSpace) const;

		bool _IntersectBoundsMesh(
			C_RayResult& result, 
			const C_Ray& ray,
			const C_CollisionMesh* pMesh,
			const Matrix44& toObjectSpace,
			const Matrix44& toWorldSpace) const;

		bool _IntersectComplexMesh(
			C_RayResult& result, 
			const C_Ray& ray,
			const C_CollisionMesh* pMesh,
			const Matrix44& toObjectSpace,
			const Matrix44& toWorldSpace,
			const U32 faceMask = kFaceMask_Front) const;

		CollisionSourceArray		m_sources;
		CollisionInstanceArray		m_instances;
		CollisionInstanceMapping	m_instanceMapping;
	};
}

#endif
