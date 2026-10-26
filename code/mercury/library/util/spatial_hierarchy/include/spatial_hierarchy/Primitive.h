#pragma once
#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include "spatial_hierarchy/Ray.h"
#include "spatial_hierarchy/RayTraceResult.h"

namespace Maths
{
	class Float44;
};
namespace SD
{
	class SpatialHierarchy;
};
class AlphaTextureInfo;
class KdTree;


class Primitive
{
public:
	virtual Box3 &WorldBound() = 0;
//	virtual bool Intersect(const SD::Ray &ray, SD::RayTraceResult &result) = 0;
	int		m_iMailBox;
};

typedef Ut::UtVector< Ut::DynamicVector<Primitive*> > PrimVector;

#ifndef DONT_USE_SIMD
struct SIMDRay;
struct SIMDResult;
#endif

class TrianglePrim
{
public:
	int		m_iSceneNode;
	int		m_iPolygon;
	Float3	m_v[3];
	Float2	m_uv[3];
	int		m_iAlphaTex;
	Aftk::I_SceneMeshNode *m_pMeshNode;
	bool	Intersect(const SD::Ray &ray, SD::RayTraceResult &result);
};

typedef Ut::UtVector< Ut::DynamicVector<TrianglePrim> > TriPrimVector;

struct OptionalTriInfo
{
	int iAlphaTex;

	Float2 m_uv[3];
};

struct TriAccel
{
	// plane:
	float n_u; //!< == normal.u / normal.k
	float n_v; //!< == normal.v / normal.k
	float n_d; //!< constant of plane equation
	int m_k; // projection dimension in lowest 2 bits + optional info index
	// line equation for line ac
	float b_nu;
	float b_nv;
	float b_d;
	// line equation for line ab
	float c_nu;
	float c_nv;
	float c_d;

	void Initialize(TrianglePrim *pPrim);
	bool Intersect(const SD::Ray &ray, SD::RayTraceResult &result, void *pTree);
	bool Intersect(void *p);
	bool CheckAlphaTextureIsTranslucent(float lambda, float mue, const KdTree *pTree);
	
#ifndef DONT_USE_SIMD
	bool IntersectSIMD(SIMDRay &ray, SIMDResult &result, __m128 &mask, void *pTree);
	bool CheckAlphaTextureIsTranslucentSIMD(__m128 lambda, __m128 mue, __m128 &mask, const KdTree *pTree);
#endif
};

void CollectPolygonsFromScene(Aftk::C_SceneNode *pMesh, TriPrimVector &primitives, Box3 *psceneBBox, KdTree *pSH, bool bTransform);


#endif
