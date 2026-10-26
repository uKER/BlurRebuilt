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
#ifndef DONT_USE_SIMD
#include "spatial_hierarchy/xmmoperators.h"
#endif

// JAN:
//   Microsoft compiler somehow manages to screw up this function when optimizing it... so don't optimize it
//   this bug only shows up on some models, if you created DirectX using D3DCREATE_FPU_PRESERVE
#pragma optimize( "", off ) // turn off all optimizations

// initialize TriAccel structure with a triangle primitive
void TriAccel::Initialize(TrianglePrim *pPrim)
{
using namespace Maths;
	Float3 A = pPrim->m_v[0];
	Float3 B = pPrim->m_v[1];
	Float3 C = pPrim->m_v[2];

	// calculate normal
	Float3 b = C-A;
	Float3 c = B-A;
	Float3 N = CrossProduct(c,b);
	float f;
	Float3 N2;

	int u,v,k;

	if (fabs(N.x) > fabs(N.y))
		if (fabs(N.x) > fabs(N.z)) k = 0; else k = 2;
	else
		if (fabs(N.y) > fabs(N.z)) k = 1; else k = 2;
	u = (k+1) %3; v = (k+2) %3;

	N2.x = N.x / N.v[k];
	N2.y = N.y / N.v[k];
	N2.z = N.z / N.v[k];

	n_u = N.v[u] / N.v[k];
	n_v = N.v[v] / N.v[k];
	m_k = k;

	n_d = DotProduct(N2, A); // dot product of one of the vertices and the projected normal - gives plane equation's constant

	f = (b.v[u]*c.v[v]-b.v[v]*c.v[u]);
    b_nu =  b.v[u] / f;
	b_nv = -b.v[v] / f;
	b_d  = -(b.v[u]*A.v[v] - b.v[v]*A.v[u]) / f;

    c_nu = -c.v[u] / f;
	c_nv =  c.v[v] / f;
	c_d  = -(c.v[v]*A.v[u] - c.v[u]*A.v[v]) / f;
}
#pragma optimize( "", on )	// restore optimizations

#define GET_KD_TEMPLATE(X) ((*(KdTreeTemplate<TriAccel>*)(&*((KdTree*)X))))

unsigned int modulo[] = {0,1,2,0,1};
extern float xfEpsilon;
bool TriAccel::Intersect(const SD::Ray &ray, SD::RayTraceResult &result, void *pTree)
{
	int k = m_k&3;
	int ku = modulo[k+1];
	int kv = modulo[k+2];

	// don’t prefetch here, assume data has already been prefetched
	// start high-latency division as early as possible
	const float nd = 1.0f/(ray.direction.v[k] + n_u * ray.direction.v[ku] + n_v * ray.direction.v[kv]);
	// ^-- DOT PRODUCT (kinda) between Normal and ray direction

	const float f = (n_d - ray.origin.v[k] - n_u * ray.origin.v[ku] - n_v * ray.origin.v[kv]) * nd;
	// ^-- DOT PRODUCT (kind) between Normal and .. Ray origin - A???

	// check for valid distance.
	if (!(result.fDistance > f && f > GET_KD_TEMPLATE(pTree).m_xfEpsilon ))
		return false;

	// compute hitpoint positions on uv plane
	const float hxu = (ray.origin.v[ku] + f * ray.direction.v[ku]);
	const float hxv = (ray.origin.v[kv] + f * ray.direction.v[kv]);

	// check first barycentric coordinate
	const float lambda = (b_nu * hxv + b_nv * hxu + b_d);
	if (lambda < 0.0f) return false;

	// check second barycentric coordinate
	const float mue = (c_nv * hxu + c_nu * hxv + c_d);
	if (mue < 0.f)	return false;

	// check third barycentric coordinate
	if (lambda+mue > 1.0f) return false;

	if (CheckAlphaTextureIsTranslucent(lambda, mue, (const KdTree *)pTree))
		return false;

	// have a valid hitpoint here. store it.
	result.fDistance = f;
	result.bHit = true;
	result.pTri = this;
	result.u = lambda;
	result.v = mue;
	return true;
}

#ifndef DONT_USE_SIMD

// intersect 4 rays with one triangle using SIMD
bool TriAccel::IntersectSIMD(SIMDRay &ray, SIMDResult &result, __m128 &inmask, void *pTree)
{
	int k = m_k&3;
	int ku = modulo[k+1];
	int kv = modulo[k+2];
	__m128 nd;
	__m128 mask		= inmask;
	__m128 fEpsilon = _mm_load1_ps( &GET_KD_TEMPLATE(pTree).m_xfEpsilon );

	// load this triangle's attributes into SIMD registers
	__m128 xn_u = _mm_load1_ps( &n_u );
	__m128 xn_v = _mm_load1_ps( &n_v );

	// don’t prefetch here, assume data has already been prefetched
	// start high-latency division as early as possible
	nd   = fOne / (ray.direction[k] + xn_u * ray.direction[ku] + xn_v * ray.direction[kv]);
	// ^-- DOT PRODUCT (kinda) between Normal and ray direction

	__m128 xn_d = _mm_load1_ps( &n_d );

	__m128 f = (xn_d - ray.origin[k] - xn_u * ray.origin[ku] - xn_v * ray.origin[kv]) * nd;
	// ^-- DOT PRODUCT (kind) between Normal and .. Ray origin - A???

	// check for valid distance.
	mask = (result.fDistance > f) & (f > fEpsilon) & mask;
	if (!_mm_movemask_ps(mask))
		return false;

	__m128 xb_nu = _mm_load1_ps( &b_nu );
	__m128 xb_nv = _mm_load1_ps( &b_nv );
	__m128 xb_d  = _mm_load1_ps( &b_d );

	// compute hitpoint positions on uv plane
	__m128 hxu = (ray.origin[ku] + f * ray.direction[ku] );
	__m128 hxv = (ray.origin[kv] + f * ray.direction[kv] );

	// check first barycentric coordinate
	__m128 lambda = (xb_nu * hxv + xb_nv * hxu + xb_d);

	mask = (lambda >= fZero) & mask;
	if (!_mm_movemask_ps(mask))
		return false;

	__m128 xc_nu = _mm_load1_ps( &c_nu );
	__m128 xc_nv = _mm_load1_ps( &c_nv );
	__m128 xc_d  = _mm_load1_ps( &c_d );

	// check second barycentric coordinate
	__m128 mue = (xc_nv * hxu + xc_nu * hxv) + xc_d;

	mask = (mue >= fZero) & mask;
	if (!_mm_movemask_ps(mask))
		return false;

	// check third barycentric coordinate
	mask = (lambda + mue <= fOne) & mask;
	if (!_mm_movemask_ps(mask))
		return false;

	if (CheckAlphaTextureIsTranslucentSIMD(lambda, mue, mask, (const KdTree *)pTree))
		return false;

	// at least one of the rays has a valid hitpoint here. store it.
	result.hit = _mm_andnot_ps( mask, result.hit );
	inmask = _mm_cmplt_ps( result.hit, fZero ); // mask out the rays that got a hit so they don't get processed again
	return true;
}
#endif

/*
*/

SD::Ray::Ray (const Float3 &o, const Float3 &d, bool bDirection)
{
	if (!bDirection)
	{
		origin = o;
		direction = d - o;
		length = Length(direction);
		direction *= 1.0f/length;
	}
	else
	{
		origin = o;
		direction = d;
		length = FLT_MAX;
		direction = Normalize(direction);
	}
}

bool TriAccel::CheckAlphaTextureIsTranslucent(float lambda, float mue, const KdTree *pTree)
{
	Float2 uv;
	int x = m_k&~3;
	if (x == ~3)
		return false;

	OptionalTriInfo *pInfo = &pTree->m_pOptionalInfo[x>>2];
	AlphaTextureInfo *pAlphaTex = &pTree->m_pSpatialHierarchy->m_pAlphaTextureList->At(pInfo->iAlphaTex);
	int cX = pAlphaTex->m_cX;
	int cY = pAlphaTex->m_cY;

	// calculate the UV
	uv.x = pInfo->m_uv[1].x * lambda + pInfo->m_uv[2].x * mue + pInfo->m_uv[0].x * (1 - lambda - mue);
	uv.y = pInfo->m_uv[1].y * lambda + pInfo->m_uv[2].y * mue + pInfo->m_uv[0].y * (1 - lambda - mue);

	int u,v;

	// lookup in texture
	u = (int) (uv.x * cX);
	v = (int) (uv.y * cY);

	u &= (cX-1);
	v &= (cY-1);

	U8 r = pAlphaTex->m_pData[cX * v + u];

	return (r<128);
}
#ifndef DONT_USE_SIMD
bool TriAccel::CheckAlphaTextureIsTranslucentSIMD(__m128 lambda, __m128 mue, __m128 &mask, const KdTree *pTree)
{
	int x = m_k&~3;
	if (x == ~3)
		return false;

	OptionalTriInfo *pInfo = &pTree->m_pOptionalInfo[x>>2];
	AlphaTextureInfo *pAlphaTex = &pTree->m_pSpatialHierarchy->m_pAlphaTextureList->At(pInfo->iAlphaTex);
	__m128 cX = _mm_cvtsi32_ss( fOne, pAlphaTex->m_cX );
	cX = _mm_shuffle_ps( cX, cX, _MM_SHUFFLE( 0,0,0,0 ) );
	__m128 cY = _mm_cvtsi32_ss( fOne, pAlphaTex->m_cY );
	cY = _mm_shuffle_ps( cY, cY, _MM_SHUFFLE( 0,0,0,0 ) );

	// calculate the current UV
	__m128 fu, fv, oneminlm;
	__m128 infou[3], infov[3];
	infou[0] = _mm_load1_ps( &pInfo->m_uv[0].x );	infov[0] = _mm_load1_ps( &pInfo->m_uv[0].y );
	infou[1] = _mm_load1_ps( &pInfo->m_uv[1].x );	infov[1] = _mm_load1_ps( &pInfo->m_uv[1].y );
	infou[2] = _mm_load1_ps( &pInfo->m_uv[2].x );	infov[2] = _mm_load1_ps( &pInfo->m_uv[2].y );

	oneminlm = _mm_sub_ps( _mm_sub_ps(fOne, lambda), mue);

	fu = _mm_add_ps(_mm_add_ps(_mm_mul_ps(infou[1], lambda), _mm_mul_ps(infou[2], mue)), _mm_mul_ps(infou[0], oneminlm));
	fv = _mm_add_ps(_mm_add_ps(_mm_mul_ps(infov[1], lambda), _mm_mul_ps(infov[2], mue)), _mm_mul_ps(infov[0], oneminlm));

	// lookup in texture
	fu = _mm_mul_ps(fu, cX);
	fv = _mm_mul_ps(fv, cY);

	// read the texture and set the mask
	int u,v;

	for (int i=0; i<4; i++)
	{
		u = (int)((float*)&fu)[i] & (pAlphaTex->m_cX-1);
		v = (int)((float*)&fv)[i] & (pAlphaTex->m_cY-1);
		if (pAlphaTex->m_pData[pAlphaTex->m_cX * v + u] < 128)
			((int*)&mask)[i] &= 0;
	}

	return false;
}
#endif
