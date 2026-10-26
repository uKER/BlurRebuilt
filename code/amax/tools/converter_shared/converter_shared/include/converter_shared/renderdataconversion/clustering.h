#ifndef RENDERDATACONVERSION_CLUSTERING_H
#define RENDERDATACONVERSION_CLUSTERING_H

#include "renderingdata/bounds.h"
#include "btl/utility/autopointer.h"

namespace RenderDataConversion {
namespace Clustering
{

typedef RenderingData::Point3 Point3;
typedef RenderingData::Vector3 Vector3;
typedef RenderingData::Normal3 Normal3;
typedef RenderingData::Vector4 Vector4;

struct ClusterItem
{
	ClusterItem() : baseModelIndex(-1), materialIndex(-1), lightmapIndex(-1), groupIndex(-1), pvsIndex(-1) {}

	Point3 centroid;
	RenderingData::Bounds bounds;
	S32 baseModelIndex;
	S32 materialIndex;
	S32 lightmapIndex;
	S32 groupIndex;
	S32 pvsIndex;
};

struct Polygon : public ClusterItem
{
	Normal3 normal;
};

struct Instance : public ClusterItem
{
	Instance() : nPolygons(0) {}

	S32 nPolygons;
};

struct Cluster
{
	Cluster() : itemIndex(-1), blockIndex(-1) {}

	enum ClusterType
	{
		CLUSTER_TYPE_BASE_MODEL,
		CLUSTER_TYPE_MODEL,
		CLUSTER_TYPE_INSTANCE,
	};

	ClusterType clusterType;
	S32 itemIndex; // Which model / instance it is.
	S32 blockIndex; // Which block the resources should go into.
};

struct Node
{
	Node() : blockIndex(-1) {}

	S32 blockIndex;
	RenderingData::Bounds bounds;

	Btl::Vector<Cluster> models;
	Btl::Vector<Cluster> instances;
	Btl::Vector<Btl::AutoPointer<Node> > subNodes;
};

class I_Scene
{
public:
	virtual ~I_Scene() {}
	virtual void Begin() = 0;
	virtual void Add(const Polygon& polygon) = 0;
	virtual void Add(const Instance& instance) = 0;
	virtual void End() = 0;
	virtual Cluster FindCluster(const Polygon& polygon) const = 0;
	virtual Cluster FindCluster(const Instance& instance) const = 0;
	virtual S32 GetNumModels() const = 0;
	virtual const Node& GetRootNode() const = 0;
};

I_Scene* CreateScene();
void DestroyScene(I_Scene*& pScene);

}}

#endif
