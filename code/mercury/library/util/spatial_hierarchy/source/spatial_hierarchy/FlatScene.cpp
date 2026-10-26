#include <StdAfx.h>

#include <spatial_hierarchy/common.h>
#include <spatial_hierarchy/Ray.h>
#include <spatial_hierarchy/RayTraceResult.h>
#include <spatial_hierarchy/HierachyNode.h>
#include <spatial_hierarchy/HierachyNodeIterator.h>
#include <spatial_hierarchy/ViewerCellIterator.h>
#include <spatial_hierarchy/SpatialHierachy.h>
#include <spatial_hierarchy/FlatScene.h>

using namespace SD;

void FlatScene::Build(Aftk::C_Scene *pAftkScene)
{
	// TODO: program this
}


FlatScene::FlatScene()
{
	m_type = SPATIAL_DIVIDER_STRUCTURE_FLAT;
	Reset();
}

FlatScene::~FlatScene()
{
	Clear();
}

void FlatScene::Clear()
{
	this->SpatialHierachy::Clear();

	if (m_pRootNode != 0)
	{
		delete m_pRootNode;
	}
}

HierachyNode* FlatScene::GetRootNodePtr() const
{
	return m_pRootNode;
}

void FlatScene::Reset()
{
	this->SpatialHierachy::Reset();

	m_pRootNode = 0;
}

void FlatScene::SetInitialPolygonCapacity(U32 capacity)
{
	if (m_pRootNode == 0)
	{
		m_pRootNode = new HierachyNode;
		m_pRootNode->SetNumPolygons(capacity);
	}
}

void FlatScene::AddPolygon(const CPolygon& polygon, HierachyNode** ppHierachyNode, U32& instance)
{
	if (m_pRootNode == 0)
	{
		m_pRootNode = new HierachyNode;
	}

	*ppHierachyNode = m_pRootNode;
	
	//As this is a flat scene, we simply add this polygon to our list of polys
	instance = m_pRootNode->AddPolygon(polygon);
}

void FlatScene::RayTrace(const Ray& ray, RayTraceResult& result) const
{
}

bool FlatScene::OcclusionRayTrace(const Ray& ray) const
{
	return false;
}
