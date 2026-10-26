#include "StdAfx.h"

#include "spatial_hierarchy/Tree-Common.h"
#include "spatial_hierarchy/Kd-TreeBuilder.h"
#include "spatial_hierarchy/Kd-Tree.h"
#include "spatial_hierarchy/Primitive.h"
#include "spatial_hierarchy/Kd-Tree-batch.h"

using namespace SD;

// some _m128 constants
float xfEpsilon = 0.003f; // this epsilon will get modified from outside, depending on the texel density
float xfMinusOne = -1.0f;
float xfOne = 1.0f;
unsigned int xfMinusOneInv = ~0x7f7fffff; //FLT_MAX
#ifndef DONT_USE_SIMD
__m128 fOne = _mm_load1_ps( &xfOne );
__m128 fZero = _mm_setzero_ps();
__m128 fMinusOne  = _mm_load1_ps( &xfMinusOne );
__m128 fMinusOneInv = _mm_load1_ps( (float*)&xfMinusOneInv );
__m128 fEpsilon = _mm_load1_ps( &xfEpsilon );
#endif

// this function calls the routine that does the actual traversing of the tree
int KdTree::RayTrace(SD::RayBatch &batch) const
{
	int nRaysLeft = batch.m_nRays;
	int iRay = 0;
#ifndef DONT_USE_SIMD
	SIMDRay ray;
	SIMDResult result;
#endif
	Ray *pRay = batch.m_Ray;
	int nHit = 0;

	// check that all the sign bits match
//	assert(DirectionSignsMatch(batch));

#ifndef DONT_USE_SIMD
	// split into batches of 4
	while (nRaysLeft >= 4)
	{
		Ray *pRay = batch.m_Ray+iRay;

		// swizzle to SIMD format
		ray.bShadowRay = pRay->bShadowRay;
		for (int i=0; i<4; i++, pRay++)
		{
			for (int j=0; j<3; j++)
			{
				ray.fOrigin[j][i] = pRay->origin.v[j];
				ray.fDirection[j][i] = pRay->direction.v[j];
			}

			ray.fLength[i] = pRay->length;
		}

		// trace 4 rays in one go
		TraceSIMD( ray, result );

		// read SIMDResult and store into m_Result[x]
		for (int i=0; i<4; i++)
		{
			batch.m_Result[iRay+i].bHit = (result.bHit[i]==0);
			if (batch.m_Result[iRay+i].bHit)
				nHit++;

			result.bHit[i] = 0;
		}

		iRay += 4;
		nRaysLeft -= 4;
	}
#endif

	// process any single leftover rays with the normal raytracer
	pRay = batch.m_Ray+iRay;
	for (; iRay<batch.m_nRays; iRay++, pRay++)
	{
		RayTrace(*pRay, batch.m_Result[iRay]);
		if (batch.m_Result[iRay].bHit)
			nHit++;
	}

	return nHit;
}

