#ifndef RENDERDATACONVERSION_CLUSTERING_H
#define RENDERDATACONVERSION_CLUSTERING_H

#include "renderingdata/bounds.h"
#include "btl/utility/autopointer.h"
#include "btl/containers/bitvector.h"
#include "converter_shared/renderdataconversion/Common.h"

namespace Pvs
{
	namespace Build
	{
		class PvsData;
	}
}
namespace Portal
{
	class C_Builder;
}

namespace RenderDataConversion {
namespace Clustering
{

typedef RenderingData::Point3 Point3;
typedef RenderingData::Vector3 Vector3;
typedef RenderingData::Normal3 Normal3;
typedef RenderingData::Vector4 Vector4;

//////////////////////////////////////////////////////////
// Scene clustering

struct SceneElement
{
	SceneElement() : baseModelIndex(-1), instanceMtlIndex(-1), groupIndex(-1), pvsIndex(-1), portalIndex(-1), renderPass(-1) {bounds.SetEmpty();}

	Point3 centroid;
	RenderingData::Bounds bounds;
	S32 baseModelIndex;
	Btl::Vector<S32, Btl::VectorParameters_Static<MAX_MATERIAL_STREAMS> > materialIndices;
	S32 instanceMtlIndex;
	S32 groupIndex;
	S32 pvsIndex;
	S32 portalIndex; // index to a bitfield of rooms
	S32 renderPass;
};

struct Polygon : public SceneElement
{
	Normal3 normal;
};

struct Instance : public SceneElement
{
	Instance() : nPolygons(0) {}

	S32 nPolygons;
};

struct SceneCluster
{
	SceneCluster() : itemIndex(-1), blockIndex(-1) {}

	enum ClusterType
	{
		CLUSTER_TYPE_BASE_MODEL,
		CLUSTER_TYPE_MODEL,
		CLUSTER_TYPE_INSTANCE,
	};

	ClusterType clusterType;
	S32 itemIndex; // Which model / instance it is.
	S32 blockIndex; // Which block the resources should go into.
	Btl::BitVector pvsBits;
	Btl::BitVector roomBits;
};

// top level node (->renderlist)
struct SceneNode
{
	SceneNode() : blockIndex(-1) {}

	S32 blockIndex; // Block index for the node resources.
	S32 groupIndex;
	S32 renderPass;
	RenderingData::Bounds bounds;
	Btl::BitVector pvsBits;
	Btl::BitVector roomBits;

	Btl::Vector<SceneCluster> models;
	Btl::Vector<SceneCluster> instances;
	Btl::Vector<Btl::AutoPointer<SceneNode> > subNodes;
};

class I_Scene
{
public:
	I_Scene() {m_pvsData = 0; m_pPortalBuilder = 0;}
	virtual ~I_Scene() {}
	virtual void Begin() = 0;
	virtual void Add(const Polygon& polygon) = 0;
	virtual void Add(const Instance& instance) = 0;
	virtual void End() = 0;
	virtual const SceneCluster& Find(const Polygon& polygon) const = 0;
	virtual const SceneCluster& Find(const Instance& instance) const = 0;
	virtual S32 GetNumModels() const = 0;
	virtual const SceneNode& GetRootNode() const = 0;
	void SetPVSData(const Pvs::Build::PvsData* pvsData) {m_pvsData = pvsData;}
	void SetPortalData(Portal::C_Builder* pPortalBuilder) {m_pPortalBuilder = pPortalBuilder;}

protected:
	const Pvs::Build::PvsData* m_pvsData;
	Portal::C_Builder* m_pPortalBuilder;
};

void Create(I_Scene*& pScene);
void Destroy(I_Scene*& pScene);

/////////////////////////////////////////////////////////////////////
// Stream file clustering

struct StreamElement
{
	Point3 centroid;
	RenderingData::Bounds bounds;
	Btl::BitVector pvsBits; // 1 bit per cell
	Btl::BitVector roomBits; // 1 bit per room
	S32 blockIndex;
	S32 lod;
	S32 sizeInBytes;
	S32 nElements;
	U32 groupBits; // Elements in the same group are merged into clusters first.
};

struct StreamCluster
{
	StreamCluster() : chunkIndex(-1) {}

	S32 chunkIndex;
};

class I_StreamData
{
public:
	virtual ~I_StreamData() {}
	virtual void Begin(S32 startChunk, S32 debug) = 0;
	virtual void Add(const StreamElement& element) = 0;
	virtual void End() = 0;
	virtual StreamCluster Find(const StreamElement& element) const = 0;
};

void Create(I_StreamData*& pScene);
void Destroy(I_StreamData*& pScene);

float PVis(float size, float sceneSize);

struct Connection
{
	S32 node1, node2;
	float distance;
};

inline bool operator < (const Connection& a, const Connection& b)
{
	return a.distance < b.distance;
}

}}

#endif
