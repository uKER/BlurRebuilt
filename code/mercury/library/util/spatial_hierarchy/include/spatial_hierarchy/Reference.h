#pragma once

#include "Spatial Divider/Ray.h"
#include "Spatial Divider/RayTraceResult.h"


class Primitive
{
public:
	virtual box3f &WorldBound() = 0;
	virtual bool Intersect(const SD::Ray &ray, SD::RayTraceResult &result) = 0;
	int		m_iMailBox;
};

typedef Ut::UtVector< Ut::DynamicVector<Primitive*> > PrimVector;

struct SIMDRay;
struct SIMDResult;

class TrianglePrim : public Primitive
{
public:
	int		m_i;
	int		m_iPosVertex[3];
	int		m_iSceneNode;
	int		m_iPolygon;
	float3	m_v[3];
	box3f	m_bbox;
	float	m_fSurfaceArea;
	Aftk::C_SceneMeshNode *m_pMeshNode;
	box3f	&WorldBound() { return m_bbox; }
	bool	Intersect(const SD::Ray &ray, SD::RayTraceResult &result);
};

struct TriAccel
{
	// plane:
	float n_u; //!< == normal.u / normal.k
	float n_v; //!< == normal.v / normal.k
	float n_d; //!< constant of plane equation
	int m_k; // projection dimension
	// line equation for line ac
	float b_nu;
	float b_nv;
	float b_d;
	int m_iMailBox;
	// line equation for line ab
	float c_nu;
	float c_nv;
	float c_d;
	int pad;

	void Initialize(TrianglePrim *pPrim);
	bool Intersect(const SD::Ray &ray, SD::RayTraceResult &result);
	bool IntersectSIMD(SIMDRay &ray, SIMDResult &result, __m128 &mask);
};

void CollectPolygonsFromScene(Aftk::C_Scene *pAftkScene, PrimVector &primitives, box3f sceneBBox);



