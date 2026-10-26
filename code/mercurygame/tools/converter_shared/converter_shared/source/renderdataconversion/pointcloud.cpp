#include "stdafx.h"
#include "converter_shared/renderdataconversion/pointcloud.h"
#include <algorithm>

//#pragma optimize("", off)

namespace RenderDataConversion {
namespace Clustering
{

bool operator < (const PointCloud::Point& a, const PointCloud::Point& b)
{
	return a.distance < b.distance;
}

PointCloud::PointCloud()
{
	m_count = 0;
}

void PointCloud::Initialise(const Point3& min, const Point3& max, const Vector3& minNodeSize)
{
	assert(m_count == 0);
	Allocate(m_root);
	m_root->min = min;
	m_root->max = max;
	m_minNodeSize = minNodeSize;
}

void PointCloud::Initialise(const Point3& min, const Point3& max, float minNodeSize)
{
	Initialise(min, max, Vector3(minNodeSize, minNodeSize, minNodeSize));
}

void PointCloud::Clear()
{
	Free(m_root);
	m_count = 0;
}

S32 PointCloud::Size() const
{
	return m_count;
}

void PointCloud::Add(const Point3& p, S32 pointID)
{
	S32 oldCount = m_count;
	Insert(*m_root, p, pointID);
	assert(m_count == oldCount + 1);
}

void PointCloud::Find(const Point3& p, const Point3& min, const Point3& max, OutputList& output) const
{
	output.Clear();
	Find(*m_root, min, max, output);

	for (int i = 0; i < output.Size(); i++)
		output[i].distance = Magnitude(output[i].position - p);

	std::sort(output.Begin(), output.End());
}

S32 PointCloud::Count(const Point3& min, const Point3& max) const
{
	OutputList output;
	Find(*m_root, min, max, output);
	return output.Size();
}

void PointCloud::Find(const Node& node, const Point3& min, const Point3& max, OutputList& output) const
{
	if (max.X() < node.min.X() || max.Y() < node.min.Y() || max.Z() < node.min.Z()
		|| min.X() > node.max.X() || min.Y() > node.max.Y() || min.Z() > node.max.Z())
		return;

	if (node.pSubNodes != 0)
	{
		assert(node.points.Size() == 0);
		Find(node.pSubNodes->first, min, max, output);
		Find(node.pSubNodes->second, min, max, output);
	}
	else
	{
		for (int i = 0; i < node.points.Size(); i++)
		{
			const Point3& p = node.points[i].position;

			if (p.X() > min.X() && p.Y() > min.Y() && p.Z() > min.Z()
				&& p.X() < max.X() && p.Y() < max.Y() && p.Z() < max.Z())
				output.PushBack(node.points[i]);
		}
	}
}

void PointCloud::Split(PointCloud::Node& node)
{
	assert(node.pSubNodes == 0);

	int splitAxis = -1;
	float size = 0.f;

	for (int i = 0; i < 3; i++)
	{
		float newSize = (node.max[i] - node.min[i]) / m_minNodeSize[i];

		if (splitAxis == -1 || newSize > size)
		{
			splitAxis = i;
			size = newSize;
		}
	}

	if (size > 1.f)
	{
		Allocate(node.pSubNodes);

		float centre = (node.min[splitAxis] + node.max[splitAxis]) / 2.f;

		node.pSubNodes->first.min = node.min;
		node.pSubNodes->first.max = node.max;
		node.pSubNodes->second.min = node.min;
		node.pSubNodes->second.max = node.max;

		node.pSubNodes->first.max[splitAxis] = centre;
		node.pSubNodes->second.min[splitAxis] = centre;
	}
}

bool PointCloud::Insert(PointCloud::Node& node, const Point3& p, S32 pointID)
{
	if (p.X() < node.min.X() || p.Y() < node.min.Y() || p.Z() < node.min.Z()
		|| p.X() > node.max.X() || p.Y() > node.max.Y() || p.Z() > node.max.Z())
		return false;

	if (node.pSubNodes == 0)
		Split(node);

	if (node.pSubNodes != 0)
	{
		if (!Insert(node.pSubNodes->first, p, pointID))
			Insert(node.pSubNodes->second, p, pointID);
	}
	else
	{
		node.points.PushBack(Point(p, pointID));
		m_count++;
	}

	return true;
}

////////////////////////////////////////////////////////////

const Point3& PointCloud::Node::GetMin() const
{
	return min;
}

const Point3& PointCloud::Node::GetMax() const
{
	return max;
}

S32 PointCloud::Node::GetNumSubNodes() const
{
	return (pSubNodes != 0) ? 2 : 0;
}

const PointCloud::Node& PointCloud::Node::GetSubNode(S32 i) const 
{
	assert(i >= 0 && i <= GetNumSubNodes());
	return (i == 0 ? pSubNodes->first : pSubNodes->second);
}

S32 PointCloud::Node::GetNumPoints() const
{
	return points.Size();
}

const PointCloud::Point& PointCloud::Node::GetPoint(S32 i) const
{
	return points[i];
}

}}