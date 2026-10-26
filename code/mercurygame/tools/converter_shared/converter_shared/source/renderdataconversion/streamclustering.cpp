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
#include "renderingdata/lods.h"
#include "converter_shared/pvs/pvs.h"
#include <Portal/Builder.h>
#include <iomanip>
#include <converter_shared/renderdataconversion/Common.h>

//#pragma optimize("", off)

namespace RenderDataConversion {
namespace Clustering
{

static Debug::Logger &g_Log = Debug::GetLogger("Graphics.RenderDataConversion.Clustering");

///////////////////////////////////////////////////////////////////////////////////////////
// Stream file clustering

//#pragma optimize("", off)

struct StreamNode
{
	StreamNode() : size(-1), groupBits(0), lowestLod(-1) {bounds.SetEmpty();}
	~StreamNode() {size = -1;}

	RenderingData::Bounds bounds;
	Btl::BitVector32 pvsBits;
	Btl::BitVector roomBits;
	U32 groupBits;
	Btl::Vector<S32> elements;

	typedef Btl::Set<StreamNode*>::Iterator Iterator;
	typedef Btl::Set<StreamNode*>::ConstIterator ConstIterator;

	Btl::Set<StreamNode*> prerequisites;
	Btl::Set<StreamNode*> dependants;
	S32 lowestLod;
	S32 size;
};

struct Counter
{
	Counter() : count(0) {}
	S32 count;
};

class StreamData : public I_StreamData
{
public:
	StreamData() 
	{
		m_bounds.SetEmpty();
		m_nBlocks = 0;
		m_chunk = -1;
		m_totalCombined = 0;
		m_largeCellCount = 0;
		m_debugRun = 0;
	}

	virtual ~StreamData() {}
	virtual void Begin(S32 startChunk, S32 debug);
	virtual void Add(const StreamElement& block);
	virtual void End();
	virtual StreamCluster Find(const StreamElement& element) const;

	static S32 MaxClusterDataSize() {return 1024 * 1024;} // Limit the amount of data in a cluster (bytes).
	static S32 MinClusterDataSize() {return 256 * 1024;} // Limit the amount of data in a cluster (bytes).
	static S32 MaxClusterElements() {return 100;} // Limit the number of textures/buffers in a chunk.
	static S32 MaxBufferIncrease() {return 50 * 1024 * 1024;} // Limit the increase in the streamer buffer from clustering.
	static S32 MaxBufferIncreaseSingle() {return 256 * 1024;} // Limit the increase in the streamer buffer from a single merge.

private:
	float GetSeparation(const Graph::NodeIterator i1, const Graph::NodeIterator i2);
	S32 GetSize(const Graph::NodeIterator it);
	S32 GetNumElements(const Graph::NodeIterator it);
	void AddNodeToClusters(const Graph::NodeIterator it, S32 chunkIndex);
	Btl::AutoPointer<StreamNode> MergeNodes(Btl::AutoPointer<StreamNode> p1, Btl::AutoPointer<StreamNode> p2);
	bool ShouldCombine(const Graph::NodeIterator i1, const Graph::NodeIterator i2, float length);
	void ClusterLod(S32 lod);
	void ConnectLodNodes(S32 lod);
	void ConnectToLowerLod(Graph::NodeIterator graphNode, Graph::NodeIterator lowerLodNode);
	S32 GetMinLod(const Graph::NodeIterator it);
	void GetCellSizesPreCluster();
	void GetCellSizesPostCluster();
	float GetCombinationCost(const StreamNode* p1, const StreamNode* p2);
	float CalculateSize(const StreamNode* p) const;

	RenderingData::Bounds m_bounds;
	Btl::Vector<StreamElement> m_elements;
	Btl::ChainedHashMap<StreamElement, S32> m_elementMap;
	Btl::Vector<StreamCluster> m_clusters;
	typedef Btl::Map<Graph::NodeIterator, Btl::AutoPointer<StreamNode> > NodeMap;
	NodeMap m_nodeMap;
	Graph m_graph;
	S32 m_nBlocks;
	S32 m_chunk;
	float m_totalCombined;

	Btl::Map<S32, Btl::Map<S32, Counter> > m_cellSizesPreCluster;
	Btl::BitVector m_largePvsCells;
	S32 m_largeCellCount;
	S32 m_debugRun;
};

void Create(I_StreamData*& pStreamData)
{
	pStreamData = new StreamData;
}

void Destroy(I_StreamData*& pStreamData)
{
	delete pStreamData;
	pStreamData = 0;
}

U32 Hash(const StreamElement& element)
{
	U32 hash = 0;
	hash = hash = Btl::Hash(element.blockIndex, hash);
	hash = hash = Btl::Hash(element.lod, hash);
	return hash;
}

bool operator == (const StreamElement& a, const StreamElement& b)
{
	return (a.blockIndex == b.blockIndex
			&& a.lod == b.lod);
};

void StreamData::Begin(S32 startChunk, S32 debug)
{
	LOGDEBUG(g_Log, "Clustering::StreamData::Begin");

	m_nBlocks = 0;
	m_chunk = startChunk;
	m_debugRun = debug;
}

void StreamData::Add(const StreamElement& element)
{
	m_elements.PushBack(element);
	m_elementMap.Insert(element, m_elements.Size() - 1);
	m_bounds.SetUnion(element.bounds);
	m_nBlocks = Maths::Max(m_nBlocks, element.blockIndex + 1);

	if (m_elements.Back().sizeInBytes == 0)
		m_elements.Back().pvsBits.ClearBits();
}

S32 CountAAndNotB(const Btl::BitVector32& a, const Btl::BitVector32& b)
{
	assert(a.Size() == b.Size());

	const U32* pa = a.Data();
	const U32* pb = b.Data();
	S32 n = a.Size() / a.BitsPerElement();
	S32 count = 0;

	for (int i = 0; i < n; i++)
	{
		U32 temp = pa[i] & ~(pb[i]);
		count += Btl::BitCount(temp);
	}

	for (int i = n * a.BitsPerElement(); i < a.Size(); i++)
		if (a[i] & !b[i]) 
			count++;

	return count;
}

bool IsPrerequisite(const StreamNode* p1, const StreamNode* p2)
{
	StreamNode::ConstIterator it = p1->prerequisites.Find((StreamNode*)p2);
	return (it != p1->prerequisites.End());
}

float StreamData::CalculateSize(const StreamNode* p) const
{
	float size = 0;

	for (int i = 0; i < p->elements.Size(); i++)
		size += m_elements[p->elements[i]].sizeInBytes;

	return size;
}

float StreamData::GetCombinationCost(const StreamNode* p1, const StreamNode* p2)
{
	// The cost (in streamer memory) of combining p2 with p1 is the size 
	// of p2 times the probability of p1 being required without p2.
	//   cost = prob(p1 & !p2) * size(p2)
	float p;

	if (!p1->pvsBits.Empty() && !p2->pvsBits.Empty())
	{
		//Btl::BitVector32 temp = p2->pvsBits;
		//temp.Invert();
		//temp &= p1->pvsBits;
		//temp &= m_largePvsCells;
		//p = temp.CountBitsSet() / (float) m_largeCellCount;

		p = CountAAndNotB(p1->pvsBits, p2->pvsBits) / (float) p1->pvsBits.Size();
	}
	else if (!p1->pvsBits.Empty())
	{
		//Btl::BitVector32 temp = p1->pvsBits;
		//temp &= m_largePvsCells;
		//p = temp.CountBitsSet() / (float) m_largeCellCount;

		p = p1->pvsBits.CountBitsSet() / (float) p1->pvsBits.Size();
	}

	float total = p * p2->size;

	return total;
}

float StreamData::GetSeparation(const Graph::NodeIterator i1, const Graph::NodeIterator i2)
{
	StreamNode* pStreamNode1 = &*m_nodeMap[i1];
	StreamNode* pStreamNode2 = &*m_nodeMap[i2];

	S32 size1 = pStreamNode1->size;
	S32 size2 = pStreamNode2->size;

	float separation;

	assert(pStreamNode1->size > -1 && pStreamNode2->size > -1);

	if (pStreamNode1->pvsBits.Empty() && pStreamNode2->pvsBits.Empty())
	{
		// No PVS: estimate probabilities from the size of the bounding box of the node compared to the scene.
		float v1 = Magnitude(pStreamNode1->bounds.GetSize());
		float v2 = Magnitude(pStreamNode2->bounds.GetSize());
		float vFull = Magnitude(m_bounds.GetSize());
		float p1 = PVis(v1, vFull) * Maths::Min(pStreamNode1->bounds.distanceMax, vFull) / vFull;
		float p2 = PVis(v2, vFull) * Maths::Min(pStreamNode2->bounds.distanceMax, vFull) / vFull;
		RenderingData::Bounds bounds;
		bounds.SetEmpty();
		bounds.SetUnion(pStreamNode1->bounds);
		bounds.SetUnion(pStreamNode2->bounds);
		float v3 = Magnitude(bounds.GetSize());
		float pCombined = PVis(v3, vFull) * Maths::Min(bounds.distanceMax, vFull) / vFull;

		// separate cost = (size(1) * prob(1) + size(2) * prob(2))
		// combined cost = (size(1) + size(2)) * (prob(1 and 2))
		float separate = (size1 * p1 + size2 * p2);
		float combined = (size1 + size2) * pCombined;

		// Amount of memory added on average to the scene by combining the nodes.
		separation = combined - separate;
	}
	else
	{
		separation = GetCombinationCost(pStreamNode1, pStreamNode2) + GetCombinationCost(pStreamNode2, pStreamNode1);

		// Combining p2 means also combining the lower lods of p2.
		for (StreamNode::ConstIterator it = pStreamNode2->prerequisites.Begin(); it != pStreamNode2->prerequisites.End(); ++it)
		{
			if (!IsPrerequisite(pStreamNode1, *it))
				separation += GetCombinationCost(pStreamNode1, *it);
		}

		for (StreamNode::ConstIterator it = pStreamNode1->prerequisites.Begin(); it != pStreamNode1->prerequisites.End(); ++it)
		{
			if (!IsPrerequisite(pStreamNode2, *it))
				separation += GetCombinationCost(pStreamNode2, *it);
		}
	}

	return separation;
}

S32 StreamData::GetSize(const Graph::NodeIterator it)
{
	StreamNode* pStreamNode = &*m_nodeMap[it];
	return pStreamNode->size;
}

S32 StreamData::GetNumElements(const Graph::NodeIterator it)
{
	S32 total = 0;

	StreamNode* pStreamNode = &*m_nodeMap[it];

	for (int i = 0; i < pStreamNode->elements.Size(); i++)
	{
		if (m_elements[pStreamNode->elements[i]].sizeInBytes > 0)
			total += m_elements[pStreamNode->elements[i]].nElements;
	}

	return total;
}

void StreamData::AddNodeToClusters(const Graph::NodeIterator it, S32 chunkIndex)
{
	for (int i = 0; i < m_nodeMap[it]->elements.Size(); i++)
	{
		S32 element = m_nodeMap[it]->elements[i];
		m_clusters[element].chunkIndex = chunkIndex;
	}
}

void UpdatePvsBits(StreamNode* pNode, const Btl::BitVector32& pvsBits)
{
	if (pNode->pvsBits.Size() == pvsBits.Size())
		pNode->pvsBits |= pvsBits;
}

void UpdatePreviousLods(StreamNode* pNode, StreamNode* pOld, StreamNode* pNew)
{
	StreamNode::Iterator it = pNode->prerequisites.Find(pOld);

	if (it != pNode->prerequisites.End())
	{
		pNode->prerequisites.Erase(it);
		pNode->prerequisites.Insert(pNew);
	}
}

void UpdateNextLods(StreamNode* pNode, StreamNode* pOld, StreamNode* pNew)
{
	StreamNode::Iterator it = pNode->dependants.Find(pOld);

	if (it != pNode->dependants.End())
	{
		pNode->dependants.Erase(it);
		pNode->dependants.Insert(pNew);

		UpdatePvsBits(pNode, pNew->pvsBits);
	}
}

/*void Verify(const StreamNode* pNode)
{
	for (StreamNode::ConstIterator it = pNode->prerequisites.Begin(); it != pNode->prerequisites.End(); ++it)
	{
		for (StreamNode::ConstIterator it2 = it->GetValue()->prerequisites.Begin(); it2 != it->GetValue()->prerequisites.End(); ++it2)
		{
			if (pNode->prerequisites.Find(it2->GetValue()) == pNode->prerequisites.End())
				assert(0);
		}
	}

	for (StreamNode::ConstIterator it = pNode->prerequisites.Begin(); it != pNode->prerequisites.End(); ++it)
	{
		if (pNode->dependants.Find(it->GetValue()) != pNode->dependants.End())
			assert(0);
	}
}*/

Btl::AutoPointer<StreamNode> StreamData::MergeNodes(Btl::AutoPointer<StreamNode> p1, Btl::AutoPointer<StreamNode> p2)
{
	Btl::AutoPointer<StreamNode> node;
	Allocate(node);

	node->bounds.SetEmpty();
	node->bounds.SetUnion(p1->bounds);
	node->bounds.SetUnion(p2->bounds);

	node->pvsBits = p1->pvsBits;
	node->pvsBits |= p2->pvsBits;

	node->roomBits = p1->roomBits;
	node->roomBits |= p2->roomBits;

	node->groupBits = (p1->groupBits) | (p2->groupBits);

	node->lowestLod = Maths::Min(p1->lowestLod, p2->lowestLod);
	
	node->elements = p1->elements;

	for (int i = 0; i < p2->elements.Size(); i++)
		node->elements.PushBack(p2->elements[i]);

	node->size = (S32) CalculateSize(&*node);

	node->prerequisites = p1->prerequisites;

	for (StreamNode::ConstIterator it = p2->prerequisites.Begin(); it != p2->prerequisites.End(); ++it)
		node->prerequisites.Insert(*it);

	node->prerequisites.Erase(&*p1);
	node->prerequisites.Erase(&*p2);

	node->dependants = p1->dependants;

	for (StreamNode::ConstIterator it = p2->dependants.Begin(); it != p2->dependants.End(); ++it)
		node->dependants.Insert(*it);

	node->dependants.Erase(&*p1);
	node->dependants.Erase(&*p2);

	for (StreamNode::ConstIterator it = node->dependants.Begin(); it != node->dependants.End(); ++it)
	{
		UpdatePreviousLods(*it, &*p1, &*node);
		UpdatePreviousLods(*it, &*p2, &*node);
	}

	for (StreamNode::ConstIterator it = node->prerequisites.Begin(); it != node->prerequisites.End(); ++it)
	{
		UpdateNextLods(*it, &*p1, &*node);
		UpdateNextLods(*it, &*p2, &*node);
	}

	for (StreamNode::Iterator d = node->dependants.Begin(); d != node->dependants.End(); ++d)
		for (StreamNode::ConstIterator p = node->prerequisites.Begin(); p != node->prerequisites.End(); ++p)
			d->GetValue()->prerequisites.Insert(p->GetValue());

	for (StreamNode::ConstIterator d = node->dependants.Begin(); d != node->dependants.End(); ++d)
		for (StreamNode::Iterator p = node->prerequisites.Begin(); p != node->prerequisites.End(); ++p)
			p->GetValue()->dependants.Insert(d->GetValue());

	return node;
}

//#pragma optimize("", off)

bool StreamData::ShouldCombine(const Graph::NodeIterator i1, const Graph::NodeIterator i2, float length)
{
	S32 size1 = GetSize(i1);
	S32 size2 = GetSize(i2);

	S32 maxClusterSize = (Maths::Min(size1, size2) < MinClusterDataSize()) ? (MaxClusterDataSize() * 2) : MaxClusterDataSize();

	if (size1 + size2 > maxClusterSize && size1 > 0 && size2 > 0)
		return false;

	S32 nElements1 = GetNumElements(i1);
	S32 nElements2 = GetNumElements(i2);

	if (nElements1 + nElements2 > MaxClusterElements() && size1 > 0 && size2 > 0)
		return false;

	StreamNode* pStreamNode1 = &*m_nodeMap[i1];
	StreamNode* pStreamNode2 = &*m_nodeMap[i2];

	//Don't combine different groups.
	if (pStreamNode1->groupBits != pStreamNode2->groupBits)
		return false;

	if (Maths::Min(size1, size2) < MinClusterDataSize())
		return true;

	if (m_totalCombined > MaxBufferIncrease())
		return false;

	if (length > MaxBufferIncreaseSingle())
	{
		LOGDEBUG(g_Log, "Not merging nodes, length = %i, total merged = %i.", (S32) length, (S32)m_totalCombined);
		return false;
	}

	return true;
}

bool IsNotCyclic(const StreamNode* pNode1, const StreamNode* pNode2)
{
	for (StreamNode::ConstIterator it = pNode1->prerequisites.Begin(); it != pNode1->prerequisites.End(); ++it)
	{
		if (it->GetValue()->prerequisites.Find((StreamNode*)pNode2) != it->GetValue()->prerequisites.End())
			return false;
	}

	for (StreamNode::ConstIterator it = pNode2->prerequisites.Begin(); it != pNode2->prerequisites.End(); ++it)
	{
		if (it->GetValue()->prerequisites.Find((StreamNode*)pNode1) != it->GetValue()->prerequisites.End())
			return false;
	}

	return true;
}

void StreamData::ConnectLodNodes(S32 lod)
{
	float maxConnectionDistance = 32.f;

	Btl::Vector<Graph::NodeIterator> graphNodeIterators;

	PointCloud pointCloud;
	pointCloud.Initialise(m_bounds.aabbMin, m_bounds.aabbMax, maxConnectionDistance);
	int i = 0;

	for (Graph::NodeIterator it = m_graph.NodesBegin(); it != m_graph.NodesEnd(); ++it)
	{
		NodeMap::Iterator pNode = m_nodeMap.Find(it);

		if (pNode->GetValue()->lowestLod == lod)
		{
			assert(pNode != m_nodeMap.End());
			Point3 centre = pNode->GetValue()->bounds.GetCentre();
			pointCloud.Add(centre, i++);
			graphNodeIterators.PushBack(it);
		}
	}

	for (int i = 0; i < graphNodeIterators.Size(); i++)
	{
		Btl::Vector<Connection> connections;

		const int MAX_CONNECTIONS = 100;
		const int MIN_CONNECTIONS = 100;

		/*Point3 centre = m_nodeMap[graphNodeIterators[i]]->bounds.GetCentre();
		
		Vector3 size(maxConnectionDistance, maxConnectionDistance, maxConnectionDistance);
		PointCloud::OutputList output;

		for (int j = 0; j < 4 && output.Size() < MIN_CONNECTIONS; j++)
		{
			pointCloud.Find(centre, centre - size, centre + size, output);
			size *= 2;
		}
		
		for (int j = 0; j < output.Size(); j++)
		{
			S32 nodeIndex = output[j].id;

			if (nodeIndex != i)
			{
				float d = GetSeparation(graphNodeIterators[i], graphNodeIterators[nodeIndex]);

				connections.PushBack();
				connections.Back().node1 = i;
				connections.Back().node2 = nodeIndex;
				connections.Back().distance = d;
			}
		}*/

		for (int j = 0; j < graphNodeIterators.Size(); j++)
		{
			if (i != j)
			{
				float d = GetSeparation(graphNodeIterators[i], graphNodeIterators[j]);

				connections.PushBack();
				connections.Back().node1 = i;
				connections.Back().node2 = j;
				connections.Back().distance = d;
			}
		}

		std::sort(connections.Begin(), connections.End());

		for (int j = 0; j < connections.Size() && j < MAX_CONNECTIONS; j++)
		{
			float d = connections[j].distance;
			m_graph.AddConnection(graphNodeIterators[connections[j].node1], graphNodeIterators[connections[j].node2], d);
		}
	}
}

void StreamData::ConnectToLowerLod(Graph::NodeIterator graphNode, Graph::NodeIterator lowerLodNode)
{
	// Connect to the lower lod node.
	float d = GetSeparation(graphNode, lowerLodNode);
	m_graph.AddConnection(graphNode, lowerLodNode, d);
}

void StreamData::ClusterLod(S32 lod)
{
	Btl::Vector<Graph::NodeIterator> blockNodes;
	blockNodes.Resize(m_nBlocks);

	// Find nodes containing the previous lod.
	if (lod > 0)
	{
		for (Graph::NodeIterator it = m_graph.NodesBegin(); it != m_graph.NodesEnd(); ++it)
		{
			for (int i = 0; i < m_nodeMap[it]->elements.Size(); i++)
			{
				if (m_elements[m_nodeMap[it]->elements[i]].lod == lod - 1)
				{
					S32 blockIndex = m_elements[m_nodeMap[it]->elements[i]].blockIndex;
					blockNodes[blockIndex] = it;
				}
			}
		}
	}

	S32 lodSize = 0;

	// Add element to the graph.
	for (int i = 0; i < m_elements.Size(); i++)
	{
		if (m_elements[i].lod == lod)
		{
			if (m_elements[i].sizeInBytes > 0 || lod == 0)
			{
				// Add to the graph.
				Graph::NodeIterator graphNode = m_graph.AddNode();
				Allocate(m_nodeMap[graphNode]);
				m_nodeMap[graphNode]->elements.PushBack(i);
				m_nodeMap[graphNode]->bounds = m_elements[i].bounds;
				m_nodeMap[graphNode]->pvsBits = m_elements[i].pvsBits;
				m_nodeMap[graphNode]->roomBits = m_elements[i].roomBits;
				m_nodeMap[graphNode]->groupBits = m_elements[i].groupBits;
				m_nodeMap[graphNode]->lowestLod = lod;
				m_nodeMap[graphNode]->size = (S32) CalculateSize(&*m_nodeMap[graphNode]);

				if (lod > 0)
				{
					Graph::NodeIterator lowerLodNode = blockNodes[m_elements[i].blockIndex];
					StreamNode* pLowerLodNode = &*m_nodeMap[lowerLodNode];
					StreamNode* pCurrentNode = &*m_nodeMap[graphNode];

					for (StreamNode::ConstIterator it = pLowerLodNode->prerequisites.Begin(); it != pLowerLodNode->prerequisites.End(); ++it)
					{
						pCurrentNode->prerequisites.Insert(it->GetValue());
						it->GetValue()->dependants.Insert(pCurrentNode);
					}

					pCurrentNode->prerequisites.Insert(pLowerLodNode);
					pLowerLodNode->dependants.Insert(pCurrentNode);

					ConnectToLowerLod(graphNode, lowerLodNode);
				}

				//Verify(&*m_nodeMap[graphNode]);

				lodSize += m_elements[i].sizeInBytes;
			}
			else
			{
				Graph::NodeIterator lowerLodNode = blockNodes[m_elements[i].blockIndex];
				m_nodeMap[lowerLodNode]->elements.PushBack(i);
			}
		}
	}

	ConnectLodNodes(lod);

	float length;
	Graph::Connection* pConnection;

	// Create clusters by merging nodes.
	while ((pConnection = m_graph.GetShortestConnection(&length)))
	{
		if (ShouldCombine(pConnection->first, pConnection->second, length) && IsNotCyclic(&*m_nodeMap[pConnection->first], &*m_nodeMap[pConnection->second]))
		{
			//float test = GetSeparation(pConnection->first, pConnection->second);

			StreamNode* pNode1 = &*m_nodeMap[pConnection->first];
			StreamNode* pNode2 = &*m_nodeMap[pConnection->second];

			Btl::AutoPointer<StreamNode> pMerged = MergeNodes(m_nodeMap[pConnection->first], m_nodeMap[pConnection->second]);
			m_nodeMap.Erase(pConnection->first);
			m_nodeMap.Erase(pConnection->second);
			Graph::NodeIterator mergedNode = m_graph.MergeNodes(pConnection->first, pConnection->second);
			m_nodeMap[mergedNode] = pMerged;

			//Verify(&*m_nodeMap[mergedNode]);

			// Recalculate the connection distances.
			for (int i = 0; i < mergedNode->GetNumConnections(); i++)
			{
				Graph::NodeIterator connected = mergedNode->GetConnection(i);
				float newLength = GetSeparation(mergedNode, connected);
				m_graph.SetConnectionLength(mergedNode, connected, newLength);
			}

			m_totalCombined += length;
		}
		else
		{
			m_graph.RemoveConnection(pConnection->first, pConnection->second);
		}
	}

	LOGDEBUG(g_Log, "Total combined: %i", (S32)m_totalCombined);
}

void StreamData::End()
{
	LOGDEBUG(g_Log, "StreamData: Building clusters.");

	GetCellSizesPreCluster();

	m_clusters.Resize(m_elements.Size());

	m_totalCombined = 0;

	for (int i = 0; i < RenderingData::LodCalculator::GetNumLods(); i++)
	{
		ClusterLod(i);
		S32 nodeCount = m_graph.GetNumNodes();
	}

	GetCellSizesPostCluster();

	// Initial chunk index.
	S32 chunkIndex = m_chunk;

	for (int lod = 0; lod < RenderingData::LodCalculator::GetNumLods(); lod++)
	{
		float pVisMax = -1.f;
		Graph::NodeIterator pCurrentNode = m_graph.NodesEnd();

		// Find the node with the most PVS bits set.
		for (Graph::NodeIterator pGraphNode = m_graph.NodesBegin(); pGraphNode != m_graph.NodesEnd(); ++pGraphNode)
		{
			StreamNode& streamNode = *m_nodeMap[pGraphNode];
			S32 minLod = streamNode.lowestLod;

			//Verify(&streamNode);

			if (minLod == lod)
			{
				float pVis = 1.f;

				if (streamNode.pvsBits.Size() > 0)
					pVis = float(streamNode.pvsBits.CountBitsSet()) / float(streamNode.pvsBits.Size());

				if (pVis > pVisMax)
				{
					pCurrentNode = pGraphNode;
					pVisMax = pVis;
				}
			}
		}

		if (pCurrentNode == m_graph.NodesEnd())
			continue;

		AddNodeToClusters(pCurrentNode, chunkIndex++);
		m_graph.EraseNode(pCurrentNode);

		while (!m_graph.Empty())
		{
			Graph::NodeIterator pNextNode = m_graph.NodesEnd();

			float pMax = -1.f;

			// Find the node with the most similar PVS.
			for (Graph::NodeIterator pGraphNode = m_graph.NodesBegin(); pGraphNode != m_graph.NodesEnd(); ++pGraphNode)
			{
				StreamNode& node1 = *m_nodeMap[pGraphNode];
				StreamNode& node2 = *m_nodeMap[pCurrentNode];

				if (node1.lowestLod == lod)
				{
					float p = 1.f;

					if (!node1.pvsBits.Empty() && !node2.pvsBits.Empty())
					{
						Btl::BitVector32 temp = node1.pvsBits;
						temp ^= node2.pvsBits;
						p = 1.f - float(temp.CountBitsSet()) / float(temp.Size());
					}

					if (p > pMax)
					{
						pNextNode = pGraphNode;
						pMax = p;
					}
				}
			}

			if (pNextNode == m_graph.NodesEnd())
				break;

			//Verify(&*m_nodeMap[pCurrentNode]);

			pCurrentNode = pNextNode;
			AddNodeToClusters(pCurrentNode, chunkIndex++);
			m_graph.EraseNode(pCurrentNode);
		}
	}

	/*Btl::MultiMap<float, Graph::NodeIterator> orderedNodes;

	// to do: order by common visibility
	for (Graph::NodeIterator pGraphNode = m_graph.NodesBegin(); pGraphNode != m_graph.NodesEnd(); ++pGraphNode)
	{
		StreamNode& streamNode = *m_nodeMap[pGraphNode];
		S32 minLod = streamNode.lowestLod;
		float pVis = 1.f;

		if (streamNode.pvsBits.Size() > 0)
			pVis = float(streamNode.pvsBits.CountBitsSet()) / float(streamNode.pvsBits.Size());

		float key = float(minLod) + (1.f - pVis);

		orderedNodes.Insert(key, pGraphNode);
	}

	// Add to clusters.
	for (Btl::MultiMap<float, Graph::NodeIterator>::Iterator pGraphNode = orderedNodes.Begin(); pGraphNode != orderedNodes.End(); ++pGraphNode)
		AddNodeToClusters(pGraphNode->GetValue(), chunkIndex++);*/

	//for (Graph::NodeIterator pGraphNode = m_graph.NodesBegin(); pGraphNode != m_graph.NodesEnd(); ++pGraphNode)
	//	AddNodeToClusters(pGraphNode, chunkIndex++);

	LOGDEBUG(g_Log, "StreamData: %i chunks.\n", chunkIndex);
}

void StreamData::GetCellSizesPreCluster()
{
	float totalMb = 0;
	S32 nCells = 0;

	for (int ielement = 0; ielement < m_elements.Size(); ielement++)
	{
		const StreamElement& element = m_elements[ielement];

		nCells = Maths::Max(nCells, element.pvsBits.Size());

		for (int icell = 0; icell < element.pvsBits.Size(); icell++)
		{
			if (element.pvsBits[icell])
			{
				m_cellSizesPreCluster[icell][element.lod].count += element.sizeInBytes;
				totalMb += element.sizeInBytes / (1024.f * 1024.f);
			}
		}
	}

	float largestCellMb = 0.f;

	for (int icell = 0; icell < nCells; icell++)
	{
		float preTotal = 0;
		
		for (int ilod = 0; ilod < m_cellSizesPreCluster[icell].Size(); ilod++)
		{
			float size = m_cellSizesPreCluster[icell][ilod].count / (1024.f * 1024.f);
			preTotal += size;
		}

		largestCellMb = Maths::Max(largestCellMb, preTotal);
	}

	float averageMb = totalMb / nCells;
	
	for (int icell = 0; icell < nCells; icell++)
	{
		float cellSizeMb = 0.f;

		for (int ilod = 0; ilod < m_cellSizesPreCluster[icell].Size(); ilod++)
		{
			cellSizeMb += m_cellSizesPreCluster[icell][ilod].count / (1024.f * 1024.f);
		}
		
		bool b = (cellSizeMb > largestCellMb * (m_debugRun / 10.f));
		m_largePvsCells.PushBack(b);
	}

	m_largeCellCount = m_largePvsCells.CountBitsSet();
}

void StreamData::GetCellSizesPostCluster()
{
	Btl::Map<S32, Btl::Map<S32, Counter> > cellSizes;
	Btl::Map<S32, Counter> lodSizes;
	S32 totalSize = 0;

	for (Graph::NodeIterator pGraphNode = m_graph.NodesBegin(); pGraphNode != m_graph.NodesEnd(); ++pGraphNode)
	{
		const StreamNode& streamNode = *m_nodeMap[pGraphNode];

		for (int ielement = 0; ielement < streamNode.elements.Size(); ielement++)
		{
			S32 lod = m_elements[streamNode.elements[ielement]].lod;
			S32 size = m_elements[streamNode.elements[ielement]].sizeInBytes;

			lodSizes[lod].count += size;
			totalSize += size;

			for (int icell = 0; icell < streamNode.pvsBits.Size(); icell++)
				if (streamNode.pvsBits[icell])
					cellSizes[icell][lod].count += size;
		}
	}

	float largestCellPreCluster = 0.f;
	float largestCellPostCluster = 0.f;
	float averageCellPreCluster = 0.f;
	float averageCellPostCluster = 0.f;

	for (int icell = 0; icell < cellSizes.Size(); icell++)
	{
		float preTotal = 0;
		std::stringstream preString;
		preString << std::fixed << std::setprecision(2);

		for (int ilod = 0; ilod < m_cellSizesPreCluster[icell].Size(); ilod++)
		{
			float size = m_cellSizesPreCluster[icell][ilod].count / (1024.f * 1024.f);
			preTotal += size;
			preString << " " << size;
		}

		float postTotal = 0;
		std::stringstream postString;
		postString << std::fixed << std::setprecision(2);

		for (int ilod = 0; ilod < cellSizes[icell].Size(); ilod++)
		{
			float size = cellSizes[icell][ilod].count / (1024.f * 1024.f);
			postTotal += size;
			postString << " " << size;
		}

		largestCellPreCluster = Maths::Max(largestCellPreCluster, preTotal);
		largestCellPostCluster = Maths::Max(largestCellPostCluster, postTotal);
		averageCellPreCluster += preTotal;
		averageCellPostCluster += postTotal;

		if (postTotal > 0)
		{
			LOGDEBUG(g_Log, "Cell %i: before %0.2f Mb, after %0.2f Mb (+%0.2f).", icell, preTotal, postTotal, postTotal - preTotal);
			LOGDEBUG(g_Log, "Before clustering:%s", preString.str().c_str());
			LOGDEBUG(g_Log, "After clustering:%s", postString.str().c_str());
		}
	}

	if (cellSizes.Size())
	{
		averageCellPreCluster /= cellSizes.Size();
		averageCellPostCluster /= cellSizes.Size();

		LOGDEBUG(g_Log, "Average cell requires %0.2f Mb before, %0.2f Mb after (+%0.2f, expected %0.2f).", averageCellPreCluster, averageCellPostCluster, averageCellPostCluster - averageCellPreCluster, m_totalCombined / (1024 * 1024));
	}

	std::stringstream lodString;
	lodString << std::fixed << std::setprecision(2);

	for (int ilod = 0; ilod < lodSizes.Size(); ilod++)
		lodString << " " << lodSizes[ilod].count / (1024.f * 1024.f);

	LOGDEBUG(g_Log, "Lod sizes:%s", lodString.str().c_str());
	LOGDEBUG(g_Log, "Total size: %0.2f Mb", totalSize / (1024.f * 1024.f));

	if (largestCellPostCluster > 0.f)
		LOGDEBUG(g_Log, "Scene requires a %0.2f Mb streamer buffer (%0.2f Mb before clustering).", largestCellPostCluster, largestCellPreCluster);
}

StreamCluster StreamData::Find(const StreamElement& element) const
{
	Btl::ChainedHashMap<StreamElement, S32>::ConstIterator find = m_elementMap.Find(element);

	if (find != m_elementMap.End())
	{
		return m_clusters[find->GetValue()];
	}
	else
	{
		StreamCluster cluster;
		return cluster;
	}
}

}}