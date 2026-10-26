#ifndef __KD_TREE_H__
#define __KD_TREE_H__

#pragma once

#include "spatial_hierarchy/Ray.h"
#include "spatial_hierarchy/RayTraceResult.h"
#include "spatial_hierarchy/Primitive.h"

#include "spatial_hierarchy/KdTreeTemplate.h"

typedef Ut::Vector<Box3> Box3Vector;

// some forward declarations
class KdTree : public KdTreeTemplate<TriAccel>
{
public:
				KdTree() : KdTreeTemplate<TriAccel>(), m_pMeshNodes(NULL), m_pOptionalInfo(NULL), m_nTris(0) {};
				~KdTree();

	int					m_nTris;
	int					m_iLayer;
	int					m_iTreeLayer;
	int					m_iLodLevel;

	Aftk::C_SceneMeshNode	**m_pMeshNodes;
	Aftk::C_Scene			*m_pScene;

	void		Build(Aftk::C_SceneNode *pNode);
    void		RayTrace(const SD::Ray &,SD::RayTraceResult &) const;
	int			RayTrace(SD::RayBatch &) const;
	Box3		*GetBoundingBox() const { return (Box3*)&m_bbox; }

	template<typename Archive>
	void		write(Archive &ar);
	template<typename Archive>
	void		read(Archive &ar, char *pszMemoryTrackerName);

	TriPrimVector			m_Primitives;
	OptionalTriInfo			*m_pOptionalInfo;
	int						m_nOptionalTriInfo;

	SD::SpatialHierarchy	*m_pSpatialHierarchy;
	Aftk::C_SceneNode		*m_pSceneNode;

	bool					m_bAllocatedBySerialization;
	int						m_iFirstGlobalPrimIdx;
};

#endif
