#include "StdAfx.h"

#include "spatial_hierarchy/Tree-Common.h"
#include "spatial_hierarchy/Ray.h"
#include "spatial_hierarchy/RayTraceResult.h"
#include "spatial_hierarchy/Primitive.h"
#include "spatial_hierarchy/Kd-TreeBuilder.h"
#include "spatial_hierarchy/RayTraceResult.h"
#include "spatial_hierarchy/Ray.h"
#include "spatial_hierarchy/AlphaTextureManager.h"
#include "spatial_hierarchy/SpatialHierarchy.h"
#include "spatial_hierarchy/kd-TreeBuilder.h"
#include "spatial_hierarchy/kd-Tree-batch.h"
#include "spatial_hierarchy/Primitive.h"
#include "spatial_hierarchy/LpvsUtility.h"
#include "spatial_hierarchy/PolyFlags.h"
#include "spatial_hierarchy/UserDataIterator.h"

using namespace SD;

/// constructor
UserDataIterator::UserDataIterator(SpatialHierarchy *pSH)
{
	m_mode = Mode_BaseInstances;
	m_iCurPrim = 0;
	m_iCurTree = 0;
	m_iCurGlobalPrim = 0;
	m_pSH = pSH;
	GetValues();
}

/// destructor
UserDataIterator::~UserDataIterator()
{
}

/// get the global index of this primitive
int	UserDataIterator::GetPrimitiveIndex()
{
	return m_iCurGlobalPrim;
}

/// get the face number of the index (-1 if the current node is an instance)
int	UserDataIterator::GetFaceIndex()
{
	return m_iCurTriangle;
}

/// get the scene node containing the current primitive
Aftk::C_SceneNode *UserDataIterator::GetSceneNode()
{
	return m_pCurSceneNode;
}

/// returns true if the current primitive is a triangle in a base instance
UserDataIterator::Mode UserDataIterator::GetMode()
{
	return m_mode;
}

/// returns true if we've reached the end of the list
bool UserDataIterator::End()
{
	return m_mode == Mode_Finished;
}

/// check whether the iterator has reached the end of a part and advance if needed
void UserDataIterator::Advance()
{
	// check whether m_iCurPrim is too high, and if it is advance to the next tree etc.
	switch (m_mode)
	{
	case Mode_BaseInstances:
		for (; m_iCurTree<m_pSH->m_KdTrees.Size(); m_iCurTree++)
		{
			if (m_iCurPrim == 0)
			{
				assert(m_pSH->m_KdTrees[m_iCurTree].m_iFirstGlobalPrimIdx == m_iCurGlobalPrim);
			}
			int n = m_pSH->m_KdTrees[m_iCurTree].m_Primitives.Size();
			if (m_iCurPrim<n)
				return;
			m_iCurPrim = 0;
		}
		m_iCurTree = 0;
		m_mode = Mode_MainTrees;
		// fall through...

	case Mode_MainTrees:
		for (; m_iCurTree<m_pSH->m_MainKdTrees.Size(); m_iCurTree++)
		{
			if (m_iCurPrim == 0)
			{
				assert(m_pSH->m_MainKdTrees[m_iCurTree].m_iFirstGlobalPrimIdx == m_iCurGlobalPrim);
			}
			int n = m_pSH->m_MainKdTrees[m_iCurTree].m_Primitives.Size();
			if (m_iCurPrim<n)
				return;
			m_iCurPrim = 0;
		}
		m_iCurTree = 0;
		m_mode = Mode_Instances;
		// fall through...

	case Mode_Instances:
		if (m_iCurPrim < m_pSH->m_Objects.Size())
			return;
		m_iCurPrim = 0;
		m_mode = Mode_Finished;
		// fall through...

	default:
		break;
	}
}

/// retrieve the values for the current primitive
void UserDataIterator::GetValues()
{
	Advance();

	switch (m_mode)
	{
	case Mode_BaseInstances:
		GetValuesFromTrianglePrimitive(&m_pSH->m_KdTrees[m_iCurTree].m_Primitives[m_iCurPrim]);
		break;
	case Mode_MainTrees:
		GetValuesFromTrianglePrimitive(&m_pSH->m_MainKdTrees[m_iCurTree].m_Primitives[m_iCurPrim]);
		break;
	case Mode_Instances:
		m_iCurTriangle = -1;
		m_pCurSceneNode = m_pSH->m_pAftkScene->GetSceneNodePtrByUniqueID(m_pSH->m_pTris[m_iCurPrim].m_iAftkSceneNode);
		break;
	}
}

// retrive the values from a given Primitive structure
void UserDataIterator::GetValuesFromTrianglePrimitive(TrianglePrim *pPrim)
{
	m_iCurTriangle = pPrim->m_iPolygon;
	m_pCurSceneNode = pPrim->m_pMeshNode;
}

/// increment iterator
void UserDataIterator::operator ++()
{
	m_iCurPrim++;
	m_iCurGlobalPrim++;
	GetValues();
}
