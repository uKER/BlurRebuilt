#include "StdAfx.h"

#include "spatial_hierarchy/Tree-Common.h"
#include "spatial_hierarchy/Kd-TreeBuilder.h"
#include "spatial_hierarchy/Ray.h"
#include "spatial_hierarchy/RayTraceResult.h"
#include "spatial_hierarchy/SpatialHierarchy.h"
#include "spatial_hierarchy/Primitive.h"
#include "spatial_hierarchy/KD-Tree-batch.h"
#include "spatial_hierarchy/Kd-Tree.h"
#include "spatial_hierarchy/LpvsUtility.h"

using namespace SD;

extern Maths::HVec3 AftkPos3ToHVec3(const Aftk::Position3 &v);

inline Maths::HMat44 ConvertMatrix(const Aftk::Matrix44 &matrix)
{
	return Maths::HMat44(
		Maths::HVec4(matrix.At<0>().X(), matrix.At<0>().Y(), matrix.At<0>().Z(),0),
		Maths::HVec4(matrix.At<1>().X(), matrix.At<1>().Y(), matrix.At<1>().Z(),0), 
		Maths::HVec4(matrix.At<2>().X(), matrix.At<2>().Y(), matrix.At<2>().Z(),0), 
		Maths::HVec4(matrix.At<3>().X(), matrix.At<3>().Y(), matrix.At<3>().Z(),1));
}

// calculate bounding box for a given meshnode and world matrix
bool CalculateBoundingBox(Aftk::C_SceneNode *pAftkSceneNode, Maths::HMat44 &mtx, Box3 &bboxref)
{
	Maths::C_AxisBox bbox;
	bool bInvalidFloats = false;

	assert( pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh);
	Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*) pAftkSceneNode;

	// collect polygons
	int nFaces = pMesh->GetNumPolygons(0);
	for (int iFace=0; iFace<nFaces; iFace++)
	{
		Aftk::C_Polygon *pPoly = &pMesh->GetPolygon(iFace);

		assert(pPoly->GetNumPolygonVertices() == 3);

		for (int i=0; i<3; i++)
		{
			int iPosVertex = pPoly->GetPolygonVertex(i).GetPositionIndex();

			Maths::HVec3 v = AftkPos3ToHVec3(pMesh->GetVertexPosition( iPosVertex )) * mtx;

			if (!(CheckBuggeredFloat(v.X()) || CheckBuggeredFloat(v.Y()) || CheckBuggeredFloat(v.Z())))
				bbox.UpdateExtents( v );
			else
				bInvalidFloats = true;
		}
	}

	if (bInvalidFloats)
	{
		LOGWARN( g_Log, "Invalid vertices found on on instance '%s'. This instance will not cast shadows.\n", pAftkSceneNode->GetPath().c_str());
	}

	bboxref = bbox;

	if (bInvalidFloats)
		return false;
	return true;
}

bool ObjectPrim::Initialize(int i, Aftk::C_SceneNode *pNode, Aftk::C_SceneNode *pMesh)
{
	m_iKdObjRef = i;
	Maths::HMat44 worldMatrix = ConvertMatrix( pNode->GetWorldMatrix() );
	Maths::HMat44 invWorldMatrix = worldMatrix.Inverse();
	Maths::Store(invWorldMatrix, &m_invWorldMatrix);
	m_iLayer = GetLayerIndexFromSceneNodePtr(pNode, 0);
	m_iLodLevel = GetModelLodLevel(pNode);
	m_iAftkSceneNode = pNode->GetUniqueId();
	m_bEnabled = true;	

	Box3 bbox;
	return CalculateBoundingBox( pMesh, worldMatrix, m_worldbbox );
}

#define GET_KD_TEMPLATE(X) ((*(KdTreeTemplate<TriAccel>*)(&*((KdTree*)X))))


bool ObjectPrim::Intersect(const SD::Ray &ray, SD::RayTraceResult &result, void *pTree)
{
	SpatialHierarchy *pSH = (SpatialHierarchy*)pTree;

	if (!m_bEnabled)
		return false;

	// check whether this object is in a layer that's disabled
	if (m_iLayer != -1 && !pSH->m_pbLayer[m_iLayer])
		return false;

	if (m_iLodLevel != pSH->m_iCurLodLevel)
		return false;

	Float3 vRayDir		= ray.direction;

	// precalculate ray direction signs
	int	iRayDir[3];
	iRayDir[0] = (vRayDir.v[0]>=0) ? 0:-1;
	iRayDir[1] = (vRayDir.v[1]>=0) ? 0:-1;
	iRayDir[2] = (vRayDir.v[2]>=0) ? 0:-1;

	// clip ray to the object's world bounding box
	float fNear = 0;
	float fFar = result.fDistance;
	ClipRayVsBBox(ray.origin, 1.f / vRayDir, iRayDir, &fNear, &fFar, m_worldbbox);
	if (fNear > fFar)
		return false;

	SD::Ray myRay;

	// transform ray into this object's local coordinate space
	float *pf = (float*)&m_invWorldMatrix;

	myRay.origin.x = ( (pf[0] * ray.origin.x) + (pf[4] * ray.origin.y) + (pf[8] * ray.origin.z) + pf[12] );
	myRay.origin.y = ( (pf[1] * ray.origin.x) + (pf[5] * ray.origin.y) + (pf[9] * ray.origin.z) + pf[13] );
	myRay.origin.z = ( (pf[2] * ray.origin.x) + (pf[6] * ray.origin.y) + (pf[10] * ray.origin.z) + pf[14] );

	myRay.direction.x = ( (pf[0] * ray.direction.x) + (pf[4] * ray.direction.y) + (pf[8] * ray.direction.z) );
	myRay.direction.y = ( (pf[1] * ray.direction.x) + (pf[5] * ray.direction.y) + (pf[9] * ray.direction.z) );
	myRay.direction.z = ( (pf[2] * ray.direction.x) + (pf[6] * ray.direction.y) + (pf[10] * ray.direction.z) );

	float fScale = Length(myRay.direction);

	myRay.direction = Normalize(myRay.direction);

	myRay.length	= ray.length * fScale;
	myRay.bShadowRay= ray.bShadowRay;

	// trace into this object's kd-tree
	KdTree *pTTree = &pSH->m_KdTrees[m_iKdObjRef];
	pTTree->m_xfEpsilon = GET_KD_TEMPLATE(pTree).m_xfEpsilon;
	int iOldPrim = result.iPrimitive;
	result.iPrimitive = -1;
	bool bOldHit = result.bHit;
	result.bHit = false;

	float fOldResult = result.fDistance;

	// if any previous raytraces obtained a result, then scale the resulting distance into the object's space
	if (result.fDistance < FLT_MAX)
		result.fDistance = result.fDistance * fScale;
	else
		result.fDistance = FLT_MAX;

	pTTree->Trace(myRay, result);
	// scale resulting distance back into world-space
	if (result.fDistance < FLT_MAX)
		result.fDistance = result.fDistance / fScale;

	// retrieve the info for the triangle that we hit
	if (result.iPrimitive != -1 && result.bHit)
	{
		result.iGlobalSubPrim = result.iPrimitive + pTTree->m_iFirstGlobalPrimIdx;

		if (pTTree->m_Primitives.Size())
		{
			const TrianglePrim *pPrim = &pTTree->m_Primitives[result.iPrimitive];
			result.iPolygon = pPrim->m_iPolygon;
			result.vVerts[0] = pPrim->m_v[0];
			result.vVerts[1] = pPrim->m_v[1];
			result.vVerts[2] = pPrim->m_v[2];
			result.pMeshNode = pPrim->m_pMeshNode;
		}

		result.bHit = true;
		return true;
	}
	else
	{
		result.iPrimitive = iOldPrim;
		result.bHit = bOldHit;
		result.fDistance = fOldResult;
		return false;
	}
}
#ifndef DONT_USE_SIMD
void ObjectPrim::IntersectSIMD(SIMDRay &ray, SIMDResult &result, __m128 &mask, void *pTree)
{
	if (!m_bEnabled)
		return;

	// trace the 4 rays using the normal tracing code
	// TODO: check for signs, and use proper SIMD code if we can
	for (int i=0; i<4; i++)
	{
		SD::Ray sray;
		SD::RayTraceResult sresult;

		sray.origin		= Float3(ray.fOrigin[0][i], ray.fOrigin[1][i], ray.fOrigin[2][i]);
		sray.direction  = Float3(ray.fDirection[0][i], ray.fDirection[1][i], ray.fDirection[2][i]);
		sray.length     = ray.fLength[i];

		if (Intersect(sray, sresult, pTree))
		{
			result.bHit[i] = true;
			result.hit.m128_f32[i] = 0;
		}
	}

	return;
/*


	SpatialHierarchy *pSH = (SpatialHierarchy*)pTree;

	// check whether this object is in a layer that's disabled
	if (m_iLayer != -1 && !pSH->m_pbLayer[m_iLayer])
		return;

	// TODO: write this whole thing in SIMD code


/*
	SIMDRay myRay;

	myRay.bShadowRay	= ray.bShadowRay;
	myRay.length		= ray.length;

	float *pf = (float*)&m_invWorldMatrix;
	__m128 *in = ray.origin;

	// transform ray by world matrix using SSE
	myRay.origin[0] = _mm_add_ps( _mm_add_ps( _mm_add_ps( _mm_load1_ps( pf+12 ), _mm_mul_ps( in[0], _mm_load1_ps( pf+0 ) ) ), _mm_mul_ps( in[1], _mm_load1_ps( pf+4 ) ) ), _mm_mul_ps( in[2], _mm_load1_ps( pf+8 ) ) );
	myRay.origin[1] = _mm_add_ps( _mm_add_ps( _mm_add_ps( _mm_load1_ps( pf+13 ), _mm_mul_ps( in[0], _mm_load1_ps( pf+1 ) ) ), _mm_mul_ps( in[1], _mm_load1_ps( pf+5 ) ) ), _mm_mul_ps( in[2], _mm_load1_ps( pf+9 ) ) );
	myRay.origin[2] = _mm_add_ps( _mm_add_ps( _mm_add_ps( _mm_load1_ps( pf+14 ), _mm_mul_ps( in[0], _mm_load1_ps( pf+2 ) ) ), _mm_mul_ps( in[1], _mm_load1_ps( pf+6 ) ) ), _mm_mul_ps( in[2], _mm_load1_ps( pf+10 ) ) );

	pf = (float*)&m_worldMatrix;

	in = ray.direction;
	myRay.direction[0] = _mm_add_ps( _mm_add_ps( _mm_mul_ps( in[0], _mm_load1_ps( pf+0 ) ), _mm_mul_ps( in[1], _mm_load1_ps( pf+1 ) ) ), _mm_mul_ps( in[2], _mm_load1_ps( pf+2 ) ) );
	myRay.direction[1] = _mm_add_ps( _mm_add_ps( _mm_mul_ps( in[0], _mm_load1_ps( pf+4 ) ), _mm_mul_ps( in[1], _mm_load1_ps( pf+5 ) ) ), _mm_mul_ps( in[2], _mm_load1_ps( pf+6 ) ) );
	myRay.direction[2] = _mm_add_ps( _mm_add_ps( _mm_mul_ps( in[0], _mm_load1_ps( pf+8 ) ), _mm_mul_ps( in[1], _mm_load1_ps( pf+9 ) ) ), _mm_mul_ps( in[2], _mm_load1_ps( pf+10 ) ) );

	// trace into this object's kd-tree
	KdTree *pTTree  = &pSH->m_KdTrees[m_iKdObjRef];
	pTTree->m_xfEpsilon = GET_KD_TEMPLATE(pTree).m_xfEpsilon;
	pTTree->TraceSIMD(myRay, result);
	// TODO: pass mask into TraceSIMD
	// TODO: check for matching directional signs
*/
}
#endif
