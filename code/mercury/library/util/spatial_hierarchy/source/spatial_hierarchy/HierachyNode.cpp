#include <StdAfx.h>

#include <spatial_hierarchy/RayTraceResult.h>
#include <spatial_hierarchy/Ray.h>
#include <spatial_hierarchy/HierachyNode.h>

using namespace SD;

HierachyNode::HierachyNode()
{
	Reset();
}

HierachyNode::~HierachyNode()
{
	Clear();
}

void HierachyNode::SetNumPolygons(U32 numPolygons)
{
	m_polygons.Reserve(numPolygons);
}

S32 HierachyNode::AddPolygon(const CPolygon& polygon)
{
	m_polygons.PushBack(polygon);

	if (polygon.GetNumPolygonVertices() == 0)
	{
		return -1;
	}

	if (m_boundingBoxUnknown)
	{
		m_bounds.AddVertex(polygon.GetPolygonVertex(0).GetPosition());        
		m_boundingBoxUnknown = false;
	}

	for (U32 polyVertex = 0; polyVertex < polygon.GetNumPolygonVertices(); polyVertex++)
	{
		m_bounds.AddVertex(polygon.GetPolygonVertex(polyVertex).GetPosition());    
	}

	return m_polygons.Size() - 1;
}

const LPVS::Bounds& HierachyNode::GetBounds() const
{
	return m_bounds;
}

Position3 HierachyNode::GetCentre() const
{
	return m_bounds.GetCentre();
}

U32 HierachyNode::GetNumPolygons() const
{
	return m_polygons.Size();
}

U32 HierachyNode::GetNumChildren() const
{
	// Fetch the number of children from our GTL container node
	return m_numChildren;
}

HierachyNode* HierachyNode::GetParentPtr()
{
	// TODO, fetch parent node from our GTL container node
	return 0;
}

HierachyNode* HierachyNode::GetChildPtr(U32 instance) 
{
	// TODO, fetch child node from our GTL container node
	return 0;
}

void HierachyNode::Clear()
{
	Reset();
}

const CPolygon& HierachyNode::GetPolygon(U32 instance) const
{
	return m_polygons[instance];
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
void HierachyNode::Reset()
{
	m_boundingBoxUnknown = true;
	m_polygons.Clear();
}

#endif
