#ifndef RENDERDATACONVERSION_CLUSTER_TREE_H
#define RENDERDATACONVERSION_CLUSTER_TREE_H

#include "btl\containers\vector.h"
#include "btl\utility\autopointer.h"

namespace RenderDataConversion {
namespace Clustering
{

// Tree for finding which cluster a polygon belongs to.
// Built top-down with splitting planes.
class ClusterTree
{
public:
	static S32 MaxPolygonsPerModel() {return 10000;} // 65k vertices allowed in index buffer, but tri-stripping can be slow.
	static S32 MinPolygonsPerCluster() {return 100;} // Takes precedence over the maximum size of a cluster.
	static S32 MaxClusterVertexDataSize() {return 1000000;} // Limit the amount of vertex data in a cluster.
	static S32 MaxClustersPerNode() {return 1000;} // = Number of draw calls in a render list.
	static float MaxNodeSize() {return 1000.f;} // Maximum size of a render node.
	static float MaxClusterSize() {return 128.f;} // Limit the maximum size of a cluster in distance units.

	enum BoundaryType
	{
		BOUNDARY_TYPE_POSITION,
		BOUNDARY_TYPE_SIZE
	};

	struct Boundary
	{
		BoundaryType type;
		Normal3 axis;
		float h;
	};

	struct Element
	{
		Element(const Polygon& polygon);

		Point3 centroid;
		Normal3 normal;
		RenderingData::Bounds bounds;
		S32 pvsIndex;
	};

	struct Node
	{
		Boundary boundary;
		Btl::AutoPointer<Node> pFront;
		Btl::AutoPointer<Node> pBack;
		Btl::List<Element> elements;
		Point3 centroid;
		RenderingData::Bounds bounds;
	};

	ClusterTree() {nodes.PushBack(&m_root);}

	void Add(const Element& element);
	void Build() {Build(m_root);}
	void Split(Node& node);
	const Node* Find(const Element& element) const;
	void SeparateLargePolygons(Node& node, float largest);

	Btl::Vector<const Node*> nodes;

private:
	void Build(Node& node);

	Node m_root;
};

}}

#endif