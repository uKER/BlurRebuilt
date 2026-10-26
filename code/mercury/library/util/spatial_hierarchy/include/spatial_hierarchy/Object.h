#pragma once

#include "spatial_hierarchy/Ray.h"
#include "spatial_hierarchy/RayTraceResult.h"
#include "spatial_hierarchy/Primitive.h"

#include "spatial_hierarchy/KdTreeTemplate.h"


// object representing a part of the scene mesh
class ObjectPrim
{
public:
	Box3			m_worldbbox;
	::Float44		m_invWorldMatrix;	// world transformation matrix
	int				m_iKdObjRef;		// kd-tree object reference
	int				m_iLayer;
	int				m_iLodLevel;
	bool			m_bEnabled;
	int				m_iAftkSceneNode;
	int				m_iUser;

	bool Initialize(int i, Aftk::C_SceneNode *pNode, Aftk::C_SceneNode *pMesh);
	bool Intersect(const SD::Ray &ray, SD::RayTraceResult &result, void *pTree);
#ifndef DONT_USE_SIMD
	void IntersectSIMD(SIMDRay &ray, SIMDResult &result, __m128 &mask, void *pTree);
#endif

	template<typename Archive>
	void write(Archive &ar)
	{
		ar << m_worldbbox << m_invWorldMatrix << m_iKdObjRef << m_iLayer << m_iAftkSceneNode << m_iUser << m_iLodLevel;
	}
	template<typename Archive>
	void read(Archive &ar)
	{
		ar >> m_worldbbox >> m_invWorldMatrix >> m_iKdObjRef >> m_iLayer >> m_iAftkSceneNode >> m_iUser >> m_iLodLevel;
		m_bEnabled = true;
	}
};

