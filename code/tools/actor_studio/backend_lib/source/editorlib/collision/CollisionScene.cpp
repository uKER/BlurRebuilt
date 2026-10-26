#include "stdafx.h"

#include <Gravity/NarrowPhase/Primitive.h>
#include <Gravity/NarrowPhase/CollisionResult.h>
#include <Gravity/NarrowPhase/PrimitiveTest.h>
#include <Gravity/NarrowPhase/Primitive.inl>
#include <Gravity/NarrowPhase/XenoCollide.h>
#include <Gravity/NarrowPhase/OBBSegment.inl>

#include <EditorLib/Collision/Ray.h>
#include <EditorLib/Collision/CollisionStructs.h>
#include <EditorLib/Collision/CollisionResult.h>
#include <EditorLib/Collision/CollisionScene.h>
#include <EditorLib/Collision/CollisionManager.h>
#include <EditorLib/Collision/CollisionHelpers.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

namespace EditorLib
{

C_CollisionScene::C_CollisionScene()
{
}

C_CollisionScene::~C_CollisionScene()
{
	Clear();
}

void C_CollisionScene::Clear()
{
	for (CollisionInstanceArray::Iterator it = m_instances.Begin();
		it != m_instances.End(); ++it)
		delete (*it).pInstance;
	m_instances.Clear();

	for (CollisionSourceArray::Iterator it = m_sources.Begin();
		it != m_sources.End(); ++it)
		delete *it;
	m_sources.Clear();
}

const C_CollisionScene::CollisionSourceArray& C_CollisionScene::GetCollisionSourceArray() const
{
	return m_sources;
}

const C_CollisionScene::CollisionInstanceArray& C_CollisionScene::GetCollisionInstanceArray() const
{
	return m_instances;
}

const C_CollisionScene::CollisionInstanceMapping& C_CollisionScene::GetCollisionInstanceMapping() const
{
	return m_instanceMapping;
}

C_Status C_CollisionScene::AddCollisionSource(
	I_CollisionSource* pSource,
	const char* aftkNodePath)
{
	m_sources.PushBack(pSource);
	C_Core::Instance().GetCollisionManager().OnAddCollisionSource(pSource, aftkNodePath);
	return E_Status_Success;
}

C_Status C_CollisionScene::RemoveCollisionSource(
	I_CollisionSource* pSource)
{
	for (CollisionSourceArray::Iterator it = m_sources.Begin(); it != m_sources.End(); ++it)
	{
		if (*it == pSource)
		{
			C_Core::Instance().GetCollisionManager().OnRemoveCollisionSource(pSource);
			delete pSource;
			m_sources.Erase(it);
			return E_Status_Success;
		}
	}

	return E_Status_Failure;
}

C_Status C_CollisionScene::AddCollisionInstance(
	I_DgNode* pDgNode,
	I_CollisionInstance* pInstance)
{
	C_CollisionInstance r = {pDgNode, pInstance};
	m_instances.PushBack(r);
	m_instanceMapping[pDgNode] = pInstance;
	return E_Status_Success;
}

C_Status C_CollisionScene::RemoveCollisionInstance(I_DgNode* pDgNode)
{
	CollisionInstanceMapping::Iterator it = m_instanceMapping.Find(pDgNode);
	if (it == m_instanceMapping.End())
		return E_Status_Failure;

	I_CollisionInstance* pInstance = it->GetValue();
	m_instanceMapping.Erase(it);
	for (CollisionInstanceArray::Iterator it = m_instances.Begin(); it != m_instances.End(); ++it)
	{
		if (it->pInstance == pInstance)
		{
			delete pInstance;
			m_instances.Erase(it);
			return E_Status_Success;
		}
	}

	return E_Status_Success;
}

U32 C_CollisionScene::Intersect(
	C_RayResult* pResults,
	U32 resultCapacity,
	const C_Ray& ray,
	E_CollisionQueryType queryType,
	U32 sourceTypeFilter) const
{
	U32 nResults = 0;

	for (C_CollisionScene::CollisionInstanceArray::ConstIterator it = m_instances.Begin();
		it != m_instances.End(); ++it)
	{
		if (!C_Core::Instance().GetCollisionManager().IsCollidable(it->pDgNode))
		{
			continue;
		}

		I_CollisionInstance* pInstance = it->pInstance;
		C_RayResult rayBoundsResult;
		if (pInstance->pSource && 
			_IntersectBounds(rayBoundsResult, ray, pInstance, sourceTypeFilter))
		{
			//Refine query if necessary.
			if (queryType == E_CollisionQueryType_Complex)
			{
				if (_IntersectComplex(
					*pResults, 
					ray, 
					pInstance,
					sourceTypeFilter))
				{
					pResults->pDgNode = it->pDgNode;
					pResults->pCollisionInstance = pInstance;
					pResults = pResults++;
					nResults++;
				}
			}
			else
			{
				//We intersect the object bounds.
				*pResults = rayBoundsResult;
				pResults->pDgNode = it->pDgNode;
				pResults->pCollisionInstance = pInstance;
				pResults = pResults++;
				nResults++;
			}
		}
		if (nResults == resultCapacity)
			break;
	}

	return nResults;
}

bool C_CollisionScene::Intersect(
	C_RayResult& result,
	const C_Ray& ray,
	E_CollisionQueryType queryType,
	U32 sourceTypeFilter) const
{
	bool hasIntersected = false;
	bool intersects = false;

	sourceTypeFilter = sourceTypeFilter & kCollisionSourceType_MaskFaces|kCollisionSourceType_MaskGeometry;
	for (C_CollisionScene::CollisionInstanceArray::ConstIterator it = m_instances.Begin();
		it != m_instances.End(); ++it)
	{
		if (!C_Core::Instance().GetCollisionManager().IsCollidable(it->pDgNode))
		{
			continue;
		}

		I_CollisionInstance* pInstance = it->pInstance;

		C_RayResult tmp;
		if (pInstance->pSource && 
			_IntersectBounds(tmp, ray, pInstance, sourceTypeFilter))
		{
			//Refine query if necessary.
			if (queryType == E_CollisionQueryType_Complex)
			{
				intersects = _IntersectComplex(
					tmp, 
					ray, 
					pInstance,
					sourceTypeFilter);
			}
			else
			{
				//We intersect the object bounds.
				intersects = true;
			}

			if (intersects)
				hasIntersected = true;

			if (intersects && tmp.distanceSquared < result.distanceSquared)
			{
				tmp.pDgNode = it->pDgNode;
				tmp.pCollisionInstance = it->pInstance;
				result = tmp;
			}
		}
	}

	return hasIntersected;
}

bool C_CollisionScene::Intersect(
	C_RayResult& result, 
	const C_Ray& ray,
	const I_CollisionInstance* pInstance,
	E_CollisionQueryType queryType,
	U32 sourceTypeFilter) const
{
	sourceTypeFilter = sourceTypeFilter & kCollisionSourceType_MaskGeometry;
	if (!pInstance->pSource)
	{
		assert(!"Instance source missing!");
		return false;
	}

	if (queryType == E_CollisionQueryType_Complex)
	{
		return _IntersectComplex(
			result,
			ray,
			pInstance,
			sourceTypeFilter);
	}
	else
	{
		return _IntersectBounds(
			result,
			ray,
			pInstance,
			sourceTypeFilter);
	}

	return false;
}

bool C_CollisionScene::NearestPoint(
	C_NearestPointResult& result,
	const C_Ray& ray,
	U32 sourceTypeFilter) const
{
	bool found = false;
	sourceTypeFilter = sourceTypeFilter & kCollisionSourceType_MaskGeometry;
	for (C_CollisionScene::CollisionInstanceArray::ConstIterator it = m_instances.Begin();
		it != m_instances.End(); ++it)
	{
		if (!C_Core::Instance().GetCollisionManager().IsCollidable(it->pDgNode))
		{
			continue;
		}

		C_NearestPointResult tmpResult;
		I_CollisionInstance* pInstance = it->pInstance;
		//C_RayResult rayBoundsResult;
		if (pInstance->pSource)// && 
//			_IntersectBounds(rayBoundsResult, ray, pInstance, sourceTypeFilter))
		{
			//Refine query
			if (_NearestPointComplex(
				tmpResult, 
				ray, 
				pInstance,
				sourceTypeFilter) &&
				tmpResult.distanceSquared < result.distanceSquared)
			{
				tmpResult.pDgNode = it->pDgNode;
				tmpResult.pCollisionInstance = pInstance;
				result = tmpResult;
				found = true;
			}
		}
	}

	return found;
}

bool C_CollisionScene::NearestPoint(
	C_NearestPointResult& result, 
	const Vector3& point) const
{
	bool foundNearest = false;
	for (C_CollisionScene::CollisionInstanceArray::ConstIterator it = m_instances.Begin();
		it != m_instances.End(); ++it)
	{
		if (!C_Core::Instance().GetCollisionManager().IsCollidable(it->pDgNode))
		{
			continue;
		}

		C_NearestPointResult tmpResult;
		tmpResult.distanceSquared = FLT_MAX;

		const I_CollisionInstance* pInstance = it->pInstance;
		if (pInstance->pSource && 
			pInstance->pSource->GetType() == E_CollisionSourceType_Mesh)
		{
			CollisionHelpers::NearestPoint(
				tmpResult,
				point,
				(C_CollisionMesh*)pInstance->pSource,
				pInstance->toObjectSpace,
				pInstance->toWorldSpace);
			if (tmpResult.distanceSquared < result.distanceSquared)
			{
				tmpResult.pCollisionInstance = pInstance;
				result = tmpResult;
				foundNearest = true;
			}
		}
	}
	return foundNearest;
}

bool C_CollisionScene::_IntersectBounds(
	C_RayResult& result,
	const C_Ray& ray,
	const I_CollisionInstance* pInstance,
	U32 sourceTypeFilter) const
{
	U32 filteredType = pInstance->pSource->GetType() & sourceTypeFilter;
	if (filteredType == E_CollisionSourceType_Mesh)
	{
		return _IntersectBoundsMesh(
			result,
			ray,
			(C_CollisionMesh*)pInstance->pSource,
			pInstance->toObjectSpace,
			pInstance->toWorldSpace);
	}
	else if (filteredType == E_CollisionSourceType_Proxy)
	{
		return _IntersectBoundsProxy(
			result,
			ray,
			(C_CollisionProxy*)pInstance->pSource,
			pInstance->toObjectSpace,
			pInstance->toWorldSpace);
	}
	return false;
}

bool C_CollisionScene::_IntersectComplex(
	C_RayResult& result, 
	const C_Ray& ray,
	const I_CollisionInstance* pInstance,
	U32 sourceTypeFilter) const
{
	U32 filteredType = pInstance->pSource->GetType() & sourceTypeFilter;
	if (filteredType == E_CollisionSourceType_Mesh)
	{
		U32 faceMask = 0;
		faceMask |= (sourceTypeFilter & E_CollisionSourceFace_Front) ? kFaceMask_Front : 0;
		faceMask |= (sourceTypeFilter & E_CollisionSourceFace_Back) ? kFaceMask_Back : 0;

		return _IntersectComplexMesh(
			result,
			ray,
			(C_CollisionMesh*)pInstance->pSource,
			pInstance->toObjectSpace,
			pInstance->toWorldSpace,
			faceMask);
	}
//	else if (filteredType == E_CollisionSourceType_Spline)
//	{
//	}
	else if (filteredType == E_CollisionSourceType_Proxy)
	{
		return _IntersectBoundsProxy(
			result,
			ray,
			(C_CollisionProxy*)pInstance->pSource,
			pInstance->toObjectSpace,
			pInstance->toWorldSpace);
	}

	return false;
}

bool C_CollisionScene::_NearestPointComplex(
	C_NearestPointResult& result, 
	const C_Ray& ray,
	const I_CollisionInstance* pInstance,
	U32 sourceTypeFilter) const
{
	U32 filteredType = pInstance->pSource->GetType() & sourceTypeFilter;
	if (filteredType == E_CollisionSourceType_Mesh)
	{
		return CollisionHelpers::NearestPoint(
			result,
			ray,
			(C_CollisionMesh*)pInstance->pSource,
			pInstance->toObjectSpace,
			pInstance->toWorldSpace);
	}
	else if (filteredType == E_CollisionSourceType_Spline)
	{
		return CollisionHelpers::NearestPoint(
			result,
			ray,
			(C_CollisionSpline*)pInstance->pSource,
			pInstance->toObjectSpace,
			pInstance->toWorldSpace);
	}
	else if (filteredType == E_CollisionSourceType_Proxy)
	{
		return CollisionHelpers::NearestPoint(
			result,
			ray,
			(C_CollisionProxy*)pInstance->pSource,
			pInstance->toObjectSpace,
			pInstance->toWorldSpace);
	}

	return false;
}

bool C_CollisionScene::_IntersectBoundsProxy(
	C_RayResult& result, 
	const C_Ray& ray,
	const C_CollisionProxy* pProxy,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace) const
{
	Maths::HMat44 m = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toObjectSpace);
	Maths::HMat44 wM = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toWorldSpace);
	Maths::HVec3 hP(ray.origin.x, ray.origin.y, ray.origin.z);
	Maths::HVec3 hQ(ray.end.x, ray.end.y, ray.end.z);
	hP = hP*m;
	hQ = hQ*m;

	Maths::C_Line line((Maths::C_Vertex&)hP, (Maths::C_Vertex&)hQ);
	Maths::C_QuadraticResult intersect = pProxy->boundingSphere.Intersect(line);

	if (intersect.Hit() != 0)
	{
		result.triIndex = -1;
		Maths::HVec3 intersection = line.Point(intersect.T0());
		//Transform intersection point into world space.
		intersection = intersection * wM;

		Maths::HVec3 delta = 
			intersection - Maths::HVec3(ray.origin.x, ray.origin.y, ray.origin.z);
		result.distanceSquared = delta.LengthSquared();

		Maths::Store(intersection, (MathsStorage::Float3*)&result.intersectionPoint);
		return true;
	}

	return false;
}

bool C_CollisionScene::_IntersectBoundsMesh(
	C_RayResult& result, 
	const C_Ray& ray,
	const C_CollisionMesh* pMesh,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace) const
{
	Maths::HMat44 m = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toObjectSpace);
	Maths::HMat44 wM = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toWorldSpace);
	Maths::HVec3 hP(ray.origin.x, ray.origin.y, ray.origin.z);
	Maths::HVec3 hQ(ray.end.x, ray.end.y, ray.end.z);
	//Create transposed obb matrix
	Maths::HMat44 toOBBSpace(pMesh->obb.transform.m_Rotate.Transposed());
	toOBBSpace.AW((Maths::HVec4)-pMesh->obb.transform.m_Translate);

	hP = hP*m*toOBBSpace;
	hQ = hQ*m*toOBBSpace;

	Maths::HVec3 contactPoint1; 
	Maths::HVec3 contactPoint2;
	U32 faceIndex1; 
	U32 faceIndex2;

	Maths::HVec3 unused(1.f,1.f,1.f);
	U32 res = Collision::IntersectLineBox(
		pMesh->obb.extents, 
		hP, 
		hQ, 
		unused, 
		contactPoint1, 
		contactPoint2, 
		faceIndex1, 
		faceIndex2);

	if (res != 0)
	{
		contactPoint1 = contactPoint1 * pMesh->obb.transform * Maths::HMat43(wM);
		
		Maths::HVec3 delta = 
			contactPoint1 - Maths::HVec3(ray.origin.x, ray.origin.y, ray.origin.z);
		result.distanceSquared = delta.LengthSquared();

		Maths::Store(contactPoint1, (MathsStorage::Float3*)&result.intersectionPoint);
		return true;
	}

	return false;
}

bool C_CollisionScene::_IntersectComplexMesh(
	C_RayResult& result, 
	const C_Ray& ray,
	const C_CollisionMesh* pMesh,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace,
	const U32 faceMask) const
{
	if (!faceMask)
		return false;

	Maths::HMat44 m = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toObjectSpace);
	Maths::HMat44 wM = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toWorldSpace);
	Maths::HVec3 hP(ray.origin.x, ray.origin.y, ray.origin.z);
	Maths::HVec3 hQ(ray.end.x, ray.end.y, ray.end.z);
	hP = hP*m;
	hQ = hQ*m;

//Gravity IntersectTriangleRay function seems to have issues. Either that or I'm going mad.
#ifndef USE_GRAVITY
	Maths::HVec3 dir = hQ - hP;
	dir.Normalise();
#endif

	Maths::HF32 closestDistance = FLT_MAX;

	bool intersects = false;
	U32* pTri = pMesh->pTriIndices;
	for (U32 iTri = 0; iTri < pMesh->nTris; iTri++, pTri+=3)
	{
		Maths::HVec3 hA = Maths::Load<Maths::HVec3>((const MathsStorage::Float3*)&pMesh->pVertices[pTri[0]]);
		Maths::HVec3 hB = Maths::Load<Maths::HVec3>((const MathsStorage::Float3*)&pMesh->pVertices[pTri[1]]);
		Maths::HVec3 hC = Maths::Load<Maths::HVec3>((const MathsStorage::Float3*)&pMesh->pVertices[pTri[2]]);
		Maths::HVec3 res;

		Maths::HVec3 n = (hB - hA) ^ (hC - hA);
		//Compare normal and ray direction.
		F32 b = n | dir;
		if ((b<0.f && 0==(faceMask & kFaceMask_Back)) || (b>0.f && 0==(faceMask & kFaceMask_Front)))
			continue;

#ifdef USE_GRAVITY
		if (Collision::IntersectTriangleRay(hA, hB, hC, hP, hQ, res))
		{
#else
		if (CollisionHelpers::IntersectTriangleRay(hA, hB, hC, hP, dir, res))
		{
#endif
			F32 distanceSquared;
			{
				Maths::HVec3 delta = res - hP;
				distanceSquared = delta.LengthSquared();
			}
			intersects = true;
			//Calculate distance squared.
			if (distanceSquared < closestDistance)
			{
				res = res * wM;
				F32 distanceSquared;
				{
					Maths::HVec3 delta = res - Maths::HVec3(ray.origin.x, ray.origin.y, ray.origin.z);;
					distanceSquared = delta.LengthSquared();
				}
				closestDistance = distanceSquared;
				result.distanceSquared = distanceSquared;
				result.triIndex = iTri;
				Maths::Store(res, (MathsStorage::Float3*)&result.intersectionPoint);
			}
		}
	}

	return intersects;
}

}