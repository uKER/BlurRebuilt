#ifndef RENDERDATACONVERSION_POINT_CLOUD_H
#define RENDERDATACONVERSION_POINT_CLOUD_H

#include "clustering.h"
#include "btl/utility/autopointer.h"
#include "btl/utility/pair.h"

namespace RenderDataConversion {
namespace Clustering
{

// KD tree with the following build strategy:
// Bounds for the entire tree are given at the start.
// Each node is split halfway along the largest axis.
//	-> tree independent of the order items are added.
class PointCloud
{
public:
	PointCloud();

	// Create the root node.
	// All items subsequently added must fall within the bounds given.
	// minNodeSize controls max depth of the tree.
	void Initialise(const Point3& min, const Point3& max, float minNodeSize);

	void Clear();

	void Add(const Point3& p, S32 pointID);

	struct Point
	{
		Point(const Point3& p, S32 i) : position(p), id(i) {}

		Point3 position;
		S32 id;
		float distance;
	};

	typedef Btl::Vector<Point> OutputList;

	void Find(const Point3& p, const Point3& min, const Point3& max, OutputList& output) const;

	S32 Size() const;

private:
	struct Node
	{
		Node() {}

		Point3 min;
		Point3 max;
		Btl::Vector<Point> points;
		Btl::AutoPointer<Btl::Pair<Node, Node> > pSubNodes;
	};

	bool Insert(Node& node, const Point3& p, S32 pointID);
	void Split(Node& node);
	void Find(const Node& node, const Point3& min, const Point3& max, OutputList& output) const;

	Btl::AutoPointer<Node> m_root;
	float m_minNodeSize;
	S32 m_count;
};



}}

#endif