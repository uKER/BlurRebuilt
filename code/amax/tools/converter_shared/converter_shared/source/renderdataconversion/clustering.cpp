#include "stdafx.h"
#include "converter_shared/renderdataconversion/clustering.h"

#include "btl/containers/map.h"
#include "btl/containers/list.h"
#include "btl/utility/autopointer.h"
#include "btl/containers/chainedhashmap.h"
#include "converter_shared/renderdataconversion/pointcloud.h"
#include "converter_shared/renderdataconversion/graphcluster.h"
#include "converter_shared/renderdataconversion/clustertree.h"
#include <algorithm>

//#pragma optimize("", off)

namespace RenderDataConversion {
namespace Clustering
{

static Debug::Logger &g_Log = Debug::GetLogger("Graphics.RenderDataConversion.Clustering");

class Scene : public I_Scene
{
public:
	Scene() : m_nModels(0) {m_bounds.SetEmpty();}
	~Scene() {}

	void Begin();
	void Add(const Polygon& polygon);
	void Add(const Instance& instance);
	void End();
	Cluster FindCluster(const Polygon& polygon) const;
	Cluster FindCluster(const Instance& instance) const;
	S32 GetNumModels() const {return m_nModels;}
	const Node& GetRootNode() const {return m_rootNode;}

	struct Key
	{
		Key(const Polygon& polygon);

		S32 baseModelIndex;
		S32 materialIndex;
		S32 lightmapIndex;
		S32 groupIndex;
	};

private:
	S32 GetNumElements(const Node* pNode);
	S32 GetNumElementsRecursive(const Node* pNode);
	S32 CountUnassignedPolygons(const Graph::NodeIterator it);
	S32 EstimateUnassignedSize(const Graph::NodeIterator it);
	void AddNodeToClusters(const Graph::NodeIterator it);
	void AssignNodeToBlock(const Graph::NodeIterator it, S32 blockIndex);
	float GetSeparation(const Graph::NodeIterator i1, const Graph::NodeIterator i2);
	bool ShouldCombine(const Graph::NodeIterator i1, const Graph::NodeIterator i2);

	Btl::Map<Key, ClusterTree> m_treeMap;
	Btl::Map<const ClusterTree::Node*, Cluster> m_clusterMap;
	S32 m_nModels;
	RenderingData::Bounds m_bounds;
	Btl::Vector<Instance> m_instances;
	Btl::ChainedHashMap<Instance, S32> m_instanceMap;
	Btl::Vector<Cluster> m_instanceClusters;
	Node m_rootNode;
	Btl::Map<Graph::NodeIterator, Btl::AutoPointer<Node> > m_nodeMap;

	// temporary
	Btl::Vector<const ClusterTree::Node*> m_nodePointers;
	Btl::Vector<Key> m_nodeKeys;
	S32 m_instanceOffset;
};

I_Scene* CreateScene()
{
	return new Scene;
}

void DestroyScene(I_Scene*& pScene)
{
	delete pScene;
	pScene = 0;
}

//////////////////////////////////////////////////////////////

bool operator < (const Scene::Key& a, const Scene::Key& b)
{
	if (a.baseModelIndex < b.baseModelIndex) return true;
	if (a.baseModelIndex > b.baseModelIndex) return false;
	if (a.materialIndex < b.materialIndex) return true;
	if (a.materialIndex > b.materialIndex) return false;
	if (a.lightmapIndex < b.lightmapIndex) return true;
	if (a.lightmapIndex > b.lightmapIndex) return false;
	if (a.groupIndex < b.groupIndex) return true;
	if (a.materialIndex > b.materialIndex) return false;
	return false;
}

bool operator == (const Instance& a, const Instance& b)
{
	if (a.centroid != b.centroid) return false;
	//if (a.bounds != b.bounds) return false; // Can probably get away without this. 
	if (a.baseModelIndex != b.baseModelIndex) return false;
	if (a.materialIndex != b.materialIndex) return false;
	if (a.lightmapIndex != b.lightmapIndex) return false;
	if (a.groupIndex != b.groupIndex) return false;
	if (a.pvsIndex != b.pvsIndex) return false;
	if (a.nPolygons != b.nPolygons) return false;
	return true;
}

U32 Hash(const Instance& instance)
{
	U32 hash = 0;
	hash = Btl::Hash(instance.centroid.X(), hash);
	hash = Btl::Hash(instance.centroid.Y(), hash);
	hash = Btl::Hash(instance.centroid.Z(), hash);
	hash = Btl::Hash(instance.baseModelIndex, hash);
	hash = Btl::Hash(instance.materialIndex, hash);
	hash = Btl::Hash(instance.lightmapIndex, hash);
	hash = Btl::Hash(instance.groupIndex, hash);
	hash = Btl::Hash(instance.pvsIndex, hash);
	hash = Btl::Hash(instance.nPolygons, hash);
	return hash;
}

/////////////////////////////////////////////////////////////////

Scene::Key::Key(const Polygon& polygon)
{
	baseModelIndex = polygon.baseModelIndex;
	materialIndex = polygon.materialIndex;
	lightmapIndex = polygon.lightmapIndex;
	groupIndex = polygon.groupIndex;
}

ClusterTree::Element::Element(const Polygon& polygon)
{
	centroid = polygon.centroid;
	normal = polygon.normal;
	bounds = polygon.bounds;
	pvsIndex = polygon.pvsIndex;
}

void Scene::Add(const Polygon& polygon)
{
	Key key(polygon);
	ClusterTree::Element element(polygon);
	m_treeMap[key].Add(element);
	m_bounds.SetUnion(polygon.bounds);
}

void Scene::Add(const Instance& instance)
{
	m_instances.PushBack(instance);
	m_instanceMap.Insert(instance, m_instances.Size() - 1);
	m_bounds.SetUnion(instance.bounds);
}

void Scene::Begin()
{
	LOGINFO(g_Log, "Clustering::Scene::Begin");
}

S32 Scene::GetNumElements(const Node* pNode)
{
	S32 nElements = 0;

	// Don't count base models.
	for (int i = 0; i < pNode->models.Size(); i++)
		if (pNode->models[i].clusterType == Cluster::CLUSTER_TYPE_MODEL)
			nElements++;
	
	nElements += pNode->instances.Size();
	return nElements;
}

S32 Scene::GetNumElementsRecursive(const Node* pNode)
{
	S32 nElements = GetNumElements(pNode);

	for (int i = 0; i < pNode->subNodes.Size(); i++)
		nElements += GetNumElementsRecursive(&*pNode->subNodes[i]);
	
	return nElements;
}

// Count the number of polygons in a graph node that have not been assigned to a block.
S32 Scene::CountUnassignedPolygons(const Graph::NodeIterator it)
{
	S32 nPolygons = 0;

	for (int i = 0; i < m_nodeMap[it]->models.Size(); i++)
	{
		if (m_nodeMap[it]->models[i].blockIndex == -1)
		{
			S32 nodeIndex = m_nodeMap[it]->models[i].itemIndex;
			const ClusterTree::Node* pNode = m_nodePointers[nodeIndex];
			nPolygons += pNode->elements.Size();
		}
	}

	return nPolygons;
}

// Estimate the size in bytes of the vertex data in a graph node that has not been assigned to a block.
S32 Scene::EstimateUnassignedSize(const Graph::NodeIterator it)
{
	// Don't know the actual size of the vertices,
	// but approximate figures are enough for clustering.
	const int BASE_VERTEX_SIZE = 24;
	const int INSTANCE_VERTEX_SIZE = 4;

	S32 size = 0;

	for (int i = 0; i < m_nodeMap[it]->models.Size(); i++)
	{
		if (m_nodeMap[it]->models[i].blockIndex == -1)
		{
			S32 nodeIndex = m_nodeMap[it]->models[i].itemIndex;
			const ClusterTree::Node* pNode = m_nodePointers[nodeIndex];
			size += pNode->elements.Size() * BASE_VERTEX_SIZE;
		}
	}

	for (int i = 0; i < m_nodeMap[it]->instances.Size(); i++)
	{
		if (m_nodeMap[it]->instances[i].blockIndex == -1)
		{
			S32 instanceIndex = m_nodeMap[it]->instances[i].itemIndex;
			size += m_instances[instanceIndex].nPolygons * INSTANCE_VERTEX_SIZE;
		}
	}

	return size;
}

Btl::AutoPointer<Node> MergeNodes(Btl::AutoPointer<Node> p1, Btl::AutoPointer<Node> p2)
{
	if (p1 == 0) return p2;
	if (p2 == 0) return p1;

	for (int i = 0; i < p2->subNodes.Size(); i++)
		p1->subNodes.PushBack(p2->subNodes[i]);

	for (int i = 0; i < p2->models.Size(); i++)
		p1->models.PushBack(p2->models[i]);

	for (int i = 0; i < p2->instances.Size(); i++)
		p1->instances.PushBack(p2->instances[i]);

	p1->bounds.SetUnion(p2->bounds);

	return p1;
}

// Set the block index of anything in the node that is not yet in a block.
void Scene::AssignNodeToBlock(const Graph::NodeIterator it, S32 blockIndex)
{
	if (m_nodeMap[it]->blockIndex == -1)
		m_nodeMap[it]->blockIndex = blockIndex;

	S32 nModels = m_nodeMap[it]->models.Size();

	for (int i = 0; i < nModels; i++)
	{
		if (m_nodeMap[it]->models[i].blockIndex == -1)
			m_nodeMap[it]->models[i].blockIndex = blockIndex;
	}

	S32 nInstances = m_nodeMap[it]->instances.Size();

	for (int i = 0; i < nInstances; i++)
	{
		if (m_nodeMap[it]->instances[i].blockIndex == -1)
			m_nodeMap[it]->instances[i].blockIndex = blockIndex;
	}
}

// Add the contents of a node to the cluster maps (should only be done once per node).
void Scene::AddNodeToClusters(const Graph::NodeIterator it)
{
	S32 nModels = m_nodeMap[it]->models.Size();

	for (int i = 0; i < nModels; i++)
	{
		S32 nodeIndex = m_nodeMap[it]->models[i].itemIndex;
		const ClusterTree::Node* pNode = m_nodePointers[nodeIndex];
		m_clusterMap[pNode] = m_nodeMap[it]->models[i];
	}

	S32 nInstances = m_nodeMap[it]->instances.Size();

	for (int i = 0; i < nInstances; i++)
	{
		S32 nodeIndex = m_nodeMap[it]->instances[i].itemIndex;
		m_instanceClusters[nodeIndex] = m_nodeMap[it]->instances[i];
	}
}

float Scene::GetSeparation(const Graph::NodeIterator i1, const Graph::NodeIterator i2)
{
	RenderingData::Bounds& b1 = m_nodeMap[i1]->bounds;
	RenderingData::Bounds& b2 = m_nodeMap[i2]->bounds;

	float d = Magnitude(b1.aabbMax - b2.aabbMax) + Magnitude(b1.aabbMin - b2.aabbMin);
	return d;
}

struct Connection
{
	S32 node1, node2;
	float distance;
};

bool operator < (const Connection& a, const Connection& b)
{
	return a.distance < b.distance;
}

// Probability of being visible.
// Could use PVS for more accurate result.
float PVis(float size, float sceneSize)
{
	float f = size / sceneSize;
	float pInside = f;
	float pFacing = Maths::Lerp(0.25f, 0.75f, f);

	float pVis = pInside + (1.f - pInside) * pFacing;
	return pVis;
}

bool Scene::ShouldCombine(const Graph::NodeIterator i1, const Graph::NodeIterator i2)
{
	// separate cost = 2 * (node cost) + (n(1) * p(1) + n(2) * p(2)) * (element cost)
	// combined cost = 1 * (node cost) + (n(1) + n(2)) * (p(1 and 2)) * (element cost)

	S32 nElements1 = GetNumElements(&*m_nodeMap[i1]);
	S32 nElements2 = GetNumElements(&*m_nodeMap[i2]);

	float nodeCost = 50.f;
	float v1 = Magnitude(m_nodeMap[i1]->bounds.GetSize());
	float v2 = Magnitude(m_nodeMap[i2]->bounds.GetSize());
	float vFull = Magnitude(m_bounds.GetSize());
	float p1 = PVis(v1, vFull);
	float p2 = PVis(v2, vFull);
	RenderingData::Bounds bounds;
	bounds.SetEmpty();
	bounds.SetUnion(m_nodeMap[i1]->bounds);
	bounds.SetUnion(m_nodeMap[i2]->bounds);
	float v3 = Magnitude(bounds.GetSize());
	float pCombined = PVis(v3, vFull);
	float separate = 2.f * nodeCost + (nElements1 * p1 + nElements2 * p2);
	float combined = nodeCost + (nElements1 + nElements2) * pCombined;

	return separate > combined;
}

void Scene::End()
{
	LOGINFO(g_Log, "Clustering: Building clusters.");

	float maxConnectionDistance = 32.f;
	m_instanceClusters.Resize(m_instances.Size());

	// For building the hierarchy.
	Graph graph;

	{
		// Add clusters to the graph.
		for (Btl::Map<Key, ClusterTree>::Iterator i = m_treeMap.Begin(); i != m_treeMap.End(); ++i)
		{
			ClusterTree& tree = i->GetValue();
			tree.Build();
			
			for (int j = 0; j != tree.nodes.Size(); ++j)
			{
				const ClusterTree::Node* pNode = tree.nodes[j];
				Graph::NodeIterator graphNode = graph.NodesEnd();

				if (pNode->elements.Size() > 0) // leaf node
				{
					S32 nodeIndex = m_nodePointers.Size();

					// Add to the graph.
					Graph::NodeIterator graphNode = graph.AddNode();
					Allocate(m_nodeMap[graphNode]);
					Cluster cluster;
					cluster.itemIndex = nodeIndex;

					if (i->GetKey().baseModelIndex > -1)
						cluster.clusterType = Cluster::CLUSTER_TYPE_BASE_MODEL;
					else
						cluster.clusterType = Cluster::CLUSTER_TYPE_MODEL;

					m_nodeMap[graphNode]->models.PushBack(cluster);
					m_nodeMap[graphNode]->bounds = pNode->bounds;

					// Add to the vectors.
					m_nodePointers.PushBack(pNode);
					m_nodeKeys.PushBack(i->GetKey());
				}
			}
		}

		m_instanceOffset = m_nodePointers.Size();

		// Add instances to the graph.
		for (int i = 0; i < m_instances.Size(); i++)
		{
			S32 nodeIndex = m_instanceOffset + i;

			// Add to the graph.
			Graph::NodeIterator graphNode = graph.AddNode();
			Allocate(m_nodeMap[graphNode]);
			Cluster cluster;
			cluster.itemIndex = nodeIndex - m_instanceOffset;
			cluster.clusterType = Cluster::CLUSTER_TYPE_INSTANCE;
			m_nodeMap[graphNode]->instances.PushBack(cluster);
			m_nodeMap[graphNode]->bounds = m_instances[i].bounds;
		}

		// Connect graph nodes.
		{
			Btl::Vector<Graph::NodeIterator> graphNodeIterators;

			PointCloud pointCloud;
			pointCloud.Initialise(m_bounds.aabbMin, m_bounds.aabbMax, maxConnectionDistance);
			int i = 0;

			// Add a node the size of the scene.
			Graph::NodeIterator sceneNode = graph.AddNode();
			Allocate(m_nodeMap[sceneNode]);
			m_nodeMap[sceneNode]->bounds = m_bounds;

			for (Graph::NodeIterator it = graph.NodesBegin(); it != graph.NodesEnd(); ++it)
			{
				Point3 centre = m_nodeMap[it]->bounds.GetCentre();
				pointCloud.Add(centre, i++);
				graphNodeIterators.PushBack(it);

				// Connect all nodes to the scene node.
				float d = GetSeparation(it, sceneNode);
				graph.AddConnection(it, sceneNode, d);
			}

			for (int i = 0; i < graphNodeIterators.Size(); i++)
			{
				const int MAX_CONNECTIONS = 20;
				const int MIN_CONNECTIONS = 10;

				Point3 centre = m_nodeMap[graphNodeIterators[i]]->bounds.GetCentre();
				S32 baseModelIndex = -1;

				if (i < m_instanceOffset)
					baseModelIndex = m_nodeKeys[i].baseModelIndex;

				Vector3 size(maxConnectionDistance, maxConnectionDistance, maxConnectionDistance);
				PointCloud::OutputList output;

				for (int j = 0; j < 4 && output.Size() < MIN_CONNECTIONS; j++)
				{
					pointCloud.Find(centre, centre - size, centre + size, output);
					size *= 2;
				}

				Btl::Vector<Connection> m_connections;
				
				for (int j = 0; j < output.Size(); j++)
				{
					S32 nodeIndex = output[j].id;

					if (nodeIndex != i)
					{
						S32 baseModelIndex2 = -1;

						if (nodeIndex < m_instanceOffset)
							baseModelIndex2 = m_nodeKeys[nodeIndex].baseModelIndex;

						Point3 centre2 = m_nodeMap[graphNodeIterators[nodeIndex]]->bounds.GetCentre();

						// Base models don't get clustered with the rest of the scene.
						if (baseModelIndex == -1 && baseModelIndex2 == -1
							|| baseModelIndex > -1 && baseModelIndex2 > -1)
						{
							float d = GetSeparation(graphNodeIterators[i], graphNodeIterators[nodeIndex]);

							m_connections.PushBack();
							m_connections.Back().node1 = i;
							m_connections.Back().node2 = nodeIndex;
							m_connections.Back().distance = d;
						}
					}
				}

				std::sort(m_connections.Begin(), m_connections.End());

				for (int j = 0; j < m_connections.Size() && j < MAX_CONNECTIONS; j++)
				{
					float d = m_connections[j].distance;
					graph.AddConnection(graphNodeIterators[m_connections[j].node1], graphNodeIterators[m_connections[j].node2], d);
				}
			}
		}
	}

	S32 block = 1; // Don't use block 0

	float length;
	Graph::Connection* pConnection;

	// Create clusters by merging nodes.
	while ((pConnection = graph.GetShortestConnection(&length)) /*&& length < ClusterTree::MaxNodeSize()*/)
	{
		S32 nPolygons1 = CountUnassignedPolygons(pConnection->first);
		S32 nPolygons2 = CountUnassignedPolygons(pConnection->second);

		S32 size1 = EstimateUnassignedSize(pConnection->first);
		S32 size2 = EstimateUnassignedSize(pConnection->second);

		// Create block.
		if (size1 + size2 > ClusterTree::MaxClusterVertexDataSize()
			|| nPolygons1 + nPolygons2 > ClusterTree::MaxPolygonsPerModel())
		{
			S32 blockIndex = block++;

			Graph::NodeIterator larger;
			
			if (nPolygons1 + nPolygons2 > ClusterTree::MaxPolygonsPerModel())
				larger = (nPolygons1 > nPolygons2) ? pConnection->first : pConnection->second;
			else if (size1 + size2 > ClusterTree::MaxClusterVertexDataSize())
				larger = (size1 > size2) ? pConnection->first : pConnection->second;

			AssignNodeToBlock(larger, blockIndex);
		}

		if (ShouldCombine(pConnection->first, pConnection->second))
		{
			Btl::AutoPointer<Node> pMerged = MergeNodes(m_nodeMap[pConnection->first], m_nodeMap[pConnection->second]);
			m_nodeMap.Erase(pConnection->first);
			m_nodeMap.Erase(pConnection->second);
			Graph::NodeIterator mergedNode = graph.MergeNodes(pConnection->first, pConnection->second);
			m_nodeMap[mergedNode] = pMerged;

			// Recalculate the connection distances.
			for (int i = 0; i < mergedNode->GetNumConnections(); i++)
			{
				Graph::NodeIterator connected = mergedNode->GetConnection(i);
				float newLength = GetSeparation(mergedNode, connected);
				float oldLength = graph.RemoveConnection(mergedNode, connected);
				graph.AddConnection(mergedNode, connected, newLength);
			}
		}
		else
		{
			graph.RemoveConnection(pConnection->first, pConnection->second);
		}
	}

	// Add the remaining nodes.
	for (Graph::NodeIterator pGraphNode = graph.NodesBegin(); pGraphNode != graph.NodesEnd(); ++pGraphNode)
	{
		S32 blockIndex = block++;
		AssignNodeToBlock(pGraphNode, blockIndex);
		AddNodeToClusters(pGraphNode);

		if (GetNumElementsRecursive(&*m_nodeMap[pGraphNode]) > 0)
			m_rootNode.subNodes.PushBack(m_nodeMap[pGraphNode]);
		else
			int breakhere = 0;
	}

	m_nModels = m_instanceOffset;

	m_rootNode.blockIndex = -1;

	LOGINFO(g_Log, "Clustering: %i models, %i blocks.\n", m_nModels, block);
}

Cluster Scene::FindCluster(const Polygon& polygon) const
{
	Key key(polygon);
	Btl::Map<Key, ClusterTree>::ConstIterator i = m_treeMap.Find(key);

	if (i != m_treeMap.End())
	{
		ClusterTree::Element element(polygon);
		const ClusterTree::Node* pNode = i->GetValue().Find(element);
		const Cluster& cluster = m_clusterMap.Find(pNode)->GetValue();
		return cluster;
	}
	else
	{
		assert(0);
		Cluster cluster;
		return cluster;
	}
}

Cluster Scene::FindCluster(const Instance& instance) const
{
	Btl::ChainedHashMap<Instance, S32>::ConstIterator find = m_instanceMap.Find(instance);

	if (find != m_instanceMap.End())
	{
		return m_instanceClusters[find->GetValue()];
	}
	else
	{
		assert(0);
		Cluster cluster;
		return cluster;
	}
}

//////////////////////////////////////////////////////////////////////

void ClusterTree::Add(const Element& element)
{
	m_root.elements.PushBack(element);
}

void ClusterTree::Build(Node& node)
{
	S32 nPolygons = node.elements.Size();

	// Calculate centroid and bounds.
	{
		Vector3 sum = Maths::Constants<Vector3>::Zero();
		RenderingData::Bounds bounds;
		bounds.SetEmpty();

		for (Btl::List<Element>::Iterator i = node.elements.Begin(); i != node.elements.End(); ++i)
		{
			sum += MakeVector(i->centroid);
			bounds.SetUnion(i->bounds);
		}

		node.centroid = MakePoint(sum / float(nPolygons));
		node.bounds = bounds;
	}

	// No split if less than min number of polygons.
	if (nPolygons < MinPolygonsPerCluster() * 2)
		return;

	// Separate large polygons.
	S32 nLarge = 0;
	S32 nSmall = 0;

	{
		for (Btl::List<Element>::Iterator i = node.elements.Begin(); i != node.elements.End(); ++i)
		{
			ClusterTree::Element& element = *i;

			if (Largest(element.bounds.GetSize()) > MaxClusterSize())
				nLarge++;
			else 
				nSmall++;
		}

		if (nLarge > 0 && nSmall > 0)
			SeparateLargePolygons(node, MaxClusterSize());
	}

	if (node.pFront == 0 
		&& nPolygons > MaxPolygonsPerModel())
	{
		Normal3 axis;

		// Find a good splitting plane.
		{
			Vector3 x, y, z;
			x = y = z = Maths::Constants<Vector3>::Zero();

			for (Btl::List<Element>::Iterator i = node.elements.Begin(); i != node.elements.End(); ++i)
			{
				Normal3 d = Normalised(i->centroid - node.centroid);
				x += ((d.X()) > 0.f) ? d : -d;
				y += ((d.Y()) > 0.f) ? d : -d;
				z += ((d.Z()) > 0.f) ? d : -d;
			}

			float sx = Magnitude(x);
			float sy = Magnitude(y);
			float sz = Magnitude(z);

			if (sx > sy && sx > sz)
				axis = Normalised(x);
			else if (sy > sz)
				axis = Normalised(y);
			else
				axis = Normalised(z);

			Vector3 sum = Maths::Constants<Vector3>::Zero();

			for (Btl::List<Element>::Iterator i = node.elements.Begin(); i != node.elements.End(); ++i)
			{
				Normal3 d = Normalised(i->centroid - node.centroid);
				sum += ((d | axis) > 0.f) ? d : -d;
			}

			float s = Magnitude(sum);
			axis = Normalised(sum);
		}

		node.boundary.axis = axis;
		node.boundary.h = MakeVector(node.centroid) | axis;

		Split(node);
	}

	// size split
	if (node.pFront == 0 && nLarge == 0)
	{
		RenderingData::Bounds bounds;
		bounds.SetEmpty();

		for (Btl::List<Element>::Iterator i = node.elements.Begin(); i != node.elements.End(); ++i)
			bounds.SetUnion(i->bounds);

		Vector3 size = bounds.GetSize();

		bool split = true;

		if (size.X() > MaxClusterSize() && size.X() > size.Y() && size.X() > size.Z())
			node.boundary.axis = Maths::Constants<Normal3>::X();
		else if (size.Y() > MaxClusterSize() && size.Y() > size.Z())
			node.boundary.axis = Maths::Constants<Normal3>::Y();
		else if (size.Z() > MaxClusterSize())
			node.boundary.axis = Maths::Constants<Normal3>::Z();
		else
			split = false;

		if (split)
		{
			Point3 centroid = bounds.GetCentre();
			node.boundary.h = MakeVector(node.centroid) | node.boundary.axis;

			Split(node);
		}
	}

	if (node.elements.Size() > MaxPolygonsPerModel())
		int breakhere = 0;

	if (node.pFront != 0) Build(*node.pFront);
	if (node.pBack != 0) Build(*node.pBack);
}

void ClusterTree::Split(Node& node)
{
	Btl::AutoPointer<Node> pFront;
	Btl::AutoPointer<Node> pBack;

	Allocate(pFront);
	Allocate(pBack);

	for (Btl::List<Element>::Iterator i = node.elements.Begin(); i != node.elements.End(); ++i)
	{
		ClusterTree::Element& element = *i;

		if ((MakeVector(element.centroid) | node.boundary.axis) >= node.boundary.h)
			pFront->elements.PushBack(element);
		else 
			pBack->elements.PushBack(element);
	}

	S32 nFront = pFront->elements.Size();
	S32 nBack = pBack->elements.Size();

	if (pFront->elements.Size() > 0 && pBack->elements.Size() > 0)
	{
		node.boundary.type = BOUNDARY_TYPE_POSITION;
		node.elements.Clear();

		node.pFront = pFront;
		node.pBack = pBack;

		nodes.PushBack(&*node.pFront);
		nodes.PushBack(&*node.pBack);
	}
	else
		int breakhere = 0;
}

void ClusterTree::SeparateLargePolygons(Node& node, float largest)
{
	Btl::AutoPointer<Node> pFront;
	Btl::AutoPointer<Node> pBack;

	Allocate(pFront);
	Allocate(pBack);

	for (Btl::List<Element>::Iterator i = node.elements.Begin(); i != node.elements.End(); ++i)
	{
		ClusterTree::Element& element = *i;

		if (Largest(element.bounds.GetSize()) > largest)
			pFront->elements.PushBack(element);
		else 
			pBack->elements.PushBack(element);
	}

	if (pFront->elements.Size() > 0 && pBack->elements.Size() > 0)
	{
		node.boundary.type = BOUNDARY_TYPE_SIZE;
		node.boundary.h = largest;
		node.elements.Clear();

		node.pFront = pFront;
		node.pBack = pBack;

		nodes.PushBack(&*node.pFront);
		nodes.PushBack(&*node.pBack);
	}
}

const ClusterTree::Node* ClusterTree::Find(const Element& element) const
{
	const Node* pNode = &m_root;

	while (1)
	{
		if (pNode->pFront != 0)
		{
			if (pNode->boundary.type == BOUNDARY_TYPE_SIZE)
			{
				if (Largest(element.bounds.GetSize()) > pNode->boundary.h)
					pNode = &*pNode->pFront;
				else
					pNode = &*pNode->pBack;
			}
			else 
			{
				if ((MakeVector(element.centroid) | pNode->boundary.axis) >= pNode->boundary.h)
					pNode = &*pNode->pFront;
				else
					pNode = &*pNode->pBack;
			}
		}
		else
		{
			return pNode;
		}
	}
}

///////////////////////////////////////////////////////////////////////

}}