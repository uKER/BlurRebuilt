#include "stdafx.h"

using namespace AftkUt;

struct Point
{
	Aftk::Position3 position;
	S32 node;
	S32 vertex;
	F32 threshold;
};

typedef Ut::UtVector<Ut::DynamicVector<Point> >			PointArray;
typedef Ut::UtVector<Ut::DynamicVector<Point*> >		PointPtrArray;
typedef Ut::UtVector<Ut::DynamicVector<const Point*> >	ConstPointPtrArray;


class C_PointCloud
{
public:

	struct Node
	{
		S32					index;
		Maths::C_AxisBox	bounds;
		PointArray			points;

		Node()
		: index(-1)
		{ }
	};

	typedef Ut::UtVector<Ut::DynamicVector<Node> > NodeArray;
	typedef Ut::UtVector<Ut::DynamicVector<const Node*> > ConstNodePtrArray;

	C_PointCloud(const Maths::C_AxisBox& bounds, F32 minSize);
	void AddPoint(const Aftk::Position3& position, S32 node, S32 vertex, F32 threshold);

	void Find(const Aftk::Position3& p, F32 threshold, ConstNodePtrArray& result);

private:

	bool HasChildren(U32 nodeIndex) const;
	bool Insert(U32 nodeIndex, Point& point);
	void Split(U32 nodeIndex);

	Maths::C_AxisBox	m_worldBounds;
	F32					m_minSize;

	NodeArray			m_Nodes;
};

C_PointCloud::C_PointCloud(const Maths::C_AxisBox& bounds, F32 minSize)
: m_worldBounds(bounds)
, m_minSize(minSize)
{
	m_Nodes.PushBack();
	m_Nodes.Back().index = m_Nodes.Size()-1;
	m_Nodes.Back().bounds = bounds;
}

void C_PointCloud::AddPoint(const Aftk::Position3& position, S32 node, S32 vertex, F32 threshold)
{
	Point p = {position, node, vertex, threshold};
	bool result = Insert(0, p);
	assert(result);
}

void C_PointCloud::Find(const Aftk::Position3 &p, F32 threshold, C_PointCloud::ConstNodePtrArray &result)
{
	Maths::C_Sphere sphere(p.x, p.y, p.z, threshold);

	Ut::UtVector<Ut::DynamicVector<C_PointCloud::Node*> > nodeStack;
	nodeStack.PushBack(&m_Nodes[0]);

	while (nodeStack.Size())
	{
		const C_PointCloud::Node *pNode = nodeStack.Back();
		nodeStack.PopBack();

		if (pNode->bounds.Intersects(sphere))
		{
			if (HasChildren(pNode->index))
			{
				nodeStack.PushBack(&m_Nodes[pNode->index*2+2]);
				nodeStack.PushBack(&m_Nodes[pNode->index*2+1]);
			}
			else if (pNode->points.Size())
				result.PushBack(pNode);
		}
	}
}


bool C_PointCloud::HasChildren(U32 nodeIndex) const
{
	return ((S32)nodeIndex*2+2 < m_Nodes.Size()) ? m_Nodes[nodeIndex*2+1].index!=-1 : false;
}

bool C_PointCloud::Insert(U32 nodeIndex, Point &point)
{
	C_PointCloud::Node *pNode = &m_Nodes[nodeIndex];

	if (point.position[0]+point.threshold < pNode->bounds.m_Min[0] || point.position[0]-point.threshold > pNode->bounds.m_Max[0] ||
		point.position[1]+point.threshold < pNode->bounds.m_Min[1] || point.position[1]-point.threshold > pNode->bounds.m_Max[1] ||
		point.position[2]+point.threshold < pNode->bounds.m_Min[2] || point.position[2]-point.threshold > pNode->bounds.m_Max[2])
		return false;

	if (!pNode->points.Size() && !HasChildren(pNode->index))
		Split(pNode->index);

	//ptrs not guaranteed to be valid as a result of vector resize, grab ptr again.
	pNode = &m_Nodes[nodeIndex];
	if (HasChildren(pNode->index))
	{
		S32 nodeIndex = pNode->index;
		return Insert(nodeIndex*2+1, point) | Insert(nodeIndex*2+2, point);	//no lazy evaluation here
	}
	else
		pNode->points.PushBack(point);

	return true;
}

void C_PointCloud::Split(U32 nodeIndex)
{
	C_PointCloud::Node *pNode = &m_Nodes[nodeIndex];
	assert(!HasChildren(pNode->index));

	U32 splitAxis = -1;
	F32 size = Maths::Constants<F32>::Min();

	//split along longest axis
	for (S32 i=0; i<3; ++i)
	{
		F32 newSize = pNode->bounds.m_Max[i] - pNode->bounds.m_Min[i];
		if (newSize > size)
		{
			splitAxis = i;
			size = newSize;
		}
	}

	if (size > 2.f*m_minSize)
	{
		if (m_Nodes.Size() < pNode->index*2+3)
			m_Nodes.Resize(pNode->index*2+3);

		//grab ptr again
		pNode = &m_Nodes[nodeIndex];
		
		C_PointCloud::Node& l = m_Nodes[pNode->index*2+1];
		C_PointCloud::Node& r = m_Nodes[pNode->index*2+2];

		l.index = pNode->index*2+1;
		l.bounds = pNode->bounds;
		l.bounds.m_Max[splitAxis] = 0.5f*(pNode->bounds.m_Min[splitAxis]+pNode->bounds.m_Max[splitAxis]);

		r.index = pNode->index*2+2;
		r.bounds = pNode->bounds;
		r.bounds.m_Min[splitAxis] = 0.5f*(pNode->bounds.m_Min[splitAxis]+pNode->bounds.m_Max[splitAxis]);
	}
}


//TODO: calculate proper minimum sphere (welzl's algorithm?)
class C_PointCluster
{
public:

	struct PointHashOp
	{
		static U32 GetHash(const Point* p)
		{
			return Ut::Hash((U64)p->node<<32 | (U64)p->vertex);
		}

		static bool Equal(const Point* p1, const Point* p2)
		{
			return p1->node==p2->node && p1->vertex==p2->vertex;
		}
	};

	typedef Ut::OpenHashSet<const Point*, PointHashOp> PointSet;

	C_PointCluster();
	~C_PointCluster();

	const PointSet& GetPoints() const;

	void Merge(const C_PointCluster& cluster);
	void AddPoint(const Point* point);
	void RemovePoint(const Point* point);

	bool IsMember(const Point* point) const;
	bool CanAdd(const Point* point) const;

	bool SetIntersects(const C_PointCluster& cluster) const;

	const Aftk::Position3& ClusterCentre() const;
	F32 ClusterRadius() const;

private:
	void UpdateCache() const;

	struct Cache
	{
		Aftk::Position3 centre;
		F32 radius;
		bool dirty;
	};

	PointSet			m_points;
	mutable Cache		m_cache;
};

typedef Ut::UtVector< Ut::DynamicVector<C_PointCluster> > PointClusterArray;
typedef Ut::UtVector< Ut::DynamicVector<C_PointCluster*> > PointClusterPtrArray;

C_PointCluster::C_PointCluster()
{
	m_cache.centre = Aftk::Position3(0.f, 0.f, 0.f);
	m_cache.radius = 0.f;
	m_cache.dirty  = false;
}

C_PointCluster::~C_PointCluster()
{
}

const C_PointCluster::PointSet& C_PointCluster::GetPoints() const
{
	return m_points;
}

void C_PointCluster::Merge(const C_PointCluster& cluster)
{
	for (PointSet::ConstIterator it = cluster.m_points.Begin();
		 it != cluster.m_points.End();
		 ++it)
	{
		m_points.Insert(it->GetKey());
		m_cache.dirty = true;
	}
}

void C_PointCluster::AddPoint(const Point* point)
{
	m_points.Insert(point);
	m_cache.dirty = true;
}

void C_PointCluster::RemovePoint(const Point* point)
{
	PointSet::Iterator it = m_points.Find(point);
	if (it != m_points.End())
	{
		m_points.Erase(it);
		m_cache.dirty = true;
	}
}

bool C_PointCluster::IsMember(const Point* point) const
{
	return m_points.Find(point) != m_points.End();
}

bool C_PointCluster::CanAdd(const Point* point) const
{
	UpdateCache();
	F32 r = m_cache.radius + point->threshold;
	F32 t = UtMaths::Magnitude2(point->position - m_cache.centre);
	return t < r*r;
}

bool C_PointCluster::SetIntersects(const C_PointCluster& cluster) const
{
	{
		//quick bounds check
		UpdateCache();
		cluster.UpdateCache();
		F32 t = (m_cache.radius+cluster.m_cache.radius);
		if (UtMaths::Magnitude2(m_cache.centre - cluster.m_cache.centre) > t*t)
			return false;
	}

	const PointSet *pSet1=0, *pSet2=0;
	if (m_points.Size() > cluster.m_points.Size())
	{
		pSet1 = &cluster.m_points;
		pSet2 = &m_points;
	}
	else
	{
		pSet1 = &m_points;
		pSet2 = &cluster.m_points;
	}

	for (PointSet::ConstIterator it = pSet1->Begin();
		 it != pSet1->End();
		 ++it)
	{
		if (pSet2->Find(it->GetKey()) != pSet2->End())
			return true;
	}

	return false;
}

const Aftk::Position3& C_PointCluster::ClusterCentre() const
{
	UpdateCache();
	return m_cache.centre;
}

F32 C_PointCluster::ClusterRadius() const
{
	UpdateCache();
	return m_cache.radius;
}

void C_PointCluster::UpdateCache() const
{
	if (m_cache.dirty)
	{
		m_cache.centre = Aftk::Position3(0.f, 0.f, 0.f);
		for (PointSet::ConstIterator it = m_points.Begin();
			it != m_points.End(); ++it)
		{
			m_cache.centre += (const Aftk::Vector3F32&)(it->GetKey()->position);
		}
		m_cache.centre /= m_points.Size();


		m_cache.radius = 0.f;
		for (PointSet::ConstIterator it = m_points.Begin();
			it != m_points.End(); ++it)
		{
			m_cache.radius = Maths::Max(m_cache.radius, UtMaths::Magnitude2(m_cache.centre - it->GetKey()->position));
		}
		m_cache.radius = Maths::Sqrt(m_cache.radius);
		m_cache.dirty = false;
	}
}

void C_SnapVertices::BuildNodesToSnap(Aftk::SceneMeshNodePtrArray& nodes, Aftk::C_Scene& scene)
{
	S32 nMeshes(0);
	
	for(Aftk::C_SceneNodeIterator it(&scene); it; ++it)
	{
		if (it->GetType() != Aftk::E_ObjType_SceneMesh)
			continue;

		Aftk::C_SceneNode* pSceneNode = it;
		Aftk::C_SceneMeshNode* pSceneMeshNode = (Aftk::C_SceneMeshNode*)pSceneNode;

		if (pSceneMeshNode->GetNumInstancesOfThisNode() != 0)
			continue;

		//Ignore nodes that have influence clusters, ie. skinned
		if (pSceneMeshNode->GetNumInfluenceClusters() != 0)
			continue;

		if (pSceneMeshNode->GetNumVertexPositions() == 0)
			continue;

		++nMeshes;
		AftkUt::Query::C_Flags flags = AftkUt::Query::GetConventionFlags(it);
		if (flags.data != 0)
		{
			//LOGDEBUG(g_Log, "Not snapping %s flags %i", it->GetName().c_str(), flags.data);
			continue;
		}
		
		Aftk::C_SceneNode* pNode = it;
		nodes.PushBack((Aftk::C_SceneMeshNode*)pNode);
	}
	LOGDEBUG(g_Log, "Found %i meshes to snap out of %i meshes in the scene.", nodes.Size(), nMeshes);
}

int DepthCompare(const void* pA_, const void *pB_)
{
	Aftk::C_SceneNode* pA = *(Aftk::C_SceneNode**)pA_;
	Aftk::C_SceneNode* pB = *(Aftk::C_SceneNode**)pB_;

	if (pA->GetDepth() == pB->GetDepth())
		return 0;

	if (pA->GetDepth() < pB->GetDepth())
		return -1;

	return 1;
}

Aftk::C_Status C_SnapVertices::Snap(const Aftk::SceneMeshNodePtrArray& nodes, I_SnapEquality& equalityTest)
{
	if (nodes.Size() == 0)
		return Aftk::C_Status::E_Status_Success;

	//In our first pass - freeze transforms, ensuring that children further down the hierarch have their transform preserved by
	//performing the freezing from the deepest hierarchy level upward.
	Aftk::SceneMeshNodePtrArray depthSortedNodes = nodes;
	Aftk::C_SceneMeshNode** ppData = &depthSortedNodes[0];
	qsort(ppData, depthSortedNodes.Size(), sizeof(void*), DepthCompare);
	
	for (S32 i = nodes.Size()-1; i > 0; --i)
	{
		Aftk::C_SceneMeshNode* pNode = depthSortedNodes[i];
		//LOGDEBUG(g_Log, "%s %i", pNode->GetPath().c_str(), pNode->GetDepth());
		{
			//freeze transforms
			Aftk::Matrix44 identity;
			UtMaths::SetIdentity(&identity);
			pNode->ApplyMatrix(pNode->GetWorldMatrix());
			pNode->SetWorldMatrix(identity, true);
		}
	}

	Aftk::C_Bounds bounds;
	U32 numVertices = 0;
	Aftk::SceneMeshNodePtrArray validNodes;

	Ut::OpenHashMap<S32, Aftk::Position3Array> worldPositions;

	for (S32 i=0; i<nodes.Size(); ++i)
	{
		Aftk::C_SceneMeshNode *pSceneNode = nodes[i];

		//Ignore instanced nodes.
		if (pSceneNode->GetNumInstancesOfThisNode() != 0)
			continue;

		//Ignore nodes that have influence clusters, ie. skinned
		if (pSceneNode->GetNumInfluenceClusters() != 0)
			continue;

		if (!pSceneNode->GetNumVertexPositions())
			continue;

		validNodes.PushBack(pSceneNode);
		bounds += pSceneNode->GetEvaluatedWorldBounds();

		Ut::OpenHashMap<S32, Aftk::Position3Array>::InsertionReturnType r = 
			worldPositions.Insert(pSceneNode->GetUniqueId());

		assert(r.second);

		Aftk::Position3Array& positions = r.first->GetValue();
		positions = pSceneNode->GetVertexPositionArray();

		for (S32 j=0; j<positions.Size(); ++j)
			positions[j] = Aftk::ApplyMatrix(pSceneNode->GetWorldMatrix(), positions[j]);			

		numVertices += positions.Size();
	}

	if (!validNodes.Size())
	{
		LOGDEBUG(g_Log, "No valid candidate nodes for vertex snapping found");
		return Aftk::C_Status::E_Status_Success;
	}

	LOGDEBUG(g_Log, "Performing vertex snapping on %d meshes (%d ignored), %d vertices", 
		validNodes.Size(), nodes.Size() - validNodes.Size(), numVertices);

	const Aftk::Position3& min = bounds.GetMin();
	const Aftk::Position3& max = bounds.GetMax();

	F32 minKdNodeSize = 10.f;
	{
		//set minimum kdNodeSize using scene bounds
		F32 l = Maths::Constants<F32>::Min();
		for (U32 i=0; i<3; ++i)
			if (max[i]-min[i] > l)
				l = max[i]-min[i];

		minKdNodeSize = l/50.f;
	}


	C_PointCloud pointCloud(Maths::C_AxisBox(Maths::HVec3(min.x, min.y, min.z), Maths::HVec3(max.x, max.y, max.z)), minKdNodeSize);

	for (S32 i=0; i<validNodes.Size(); ++i)
	{
		Aftk::C_SceneMeshNode *pMeshNode = (Aftk::C_SceneMeshNode*)validNodes[i];

		Ut::OpenHashMap<S32, Aftk::Position3Array>::Iterator it = worldPositions.Find(pMeshNode->GetUniqueId());
		assert(it != worldPositions.End());
		Aftk::Position3Array& positions = it->GetValue();

		for (S32 j=0; j<positions.Size(); ++j)
			pointCloud.AddPoint(positions[j], pMeshNode->GetUniqueId(), j, equalityTest.Threshold(positions[j]));
	}

	//assumption: pointcloud does not change from here on.
	{
		Ut::OpenHashMap<U64, const Point*>	pointMap;
		PointClusterPtrArray				pointClusters;

		{
			C_PointCloud::ConstNodePtrArray		queryResults;
			ConstPointPtrArray					snapCandidates;

			for (S32 i=0; i<validNodes.Size(); ++i)
			{
				Aftk::C_SceneMeshNode *pMeshNode = (Aftk::C_SceneMeshNode*)validNodes[i];

				Ut::OpenHashMap<S32, Aftk::Position3Array>::Iterator it = worldPositions.Find(pMeshNode->GetUniqueId());
				assert(it != worldPositions.End());
				Aftk::Position3Array& positions = it->GetValue();

				for (S32 j=0; j<positions.Size(); ++j)
				{
					U64 vertexIndex = ((U64)pMeshNode->GetUniqueId()<<32) | (U64) j;

					queryResults.Clear();
					snapCandidates.Clear();

					pointCloud.Find(positions[j], equalityTest.Threshold(positions[j]) , queryResults);

					for (S32 u=0; u<queryResults.Size(); ++u)
					{
						for (S32 v=0; v<queryResults[u]->points.Size(); ++v)
						{
							const Point& p = queryResults[u]->points[v];
							if (equalityTest.Equal(positions[j], p.position))
							{
								if (p.node == pMeshNode->GetUniqueId() && p.vertex == j)
								{
									//we've found the current vertex in 'C_PointCloud'
									//we don't care from which kd-tree node it came from
									pointMap[vertexIndex] = &p;
								}
								else
									snapCandidates.PushBack(&p);
							}
						}
					}
				
					if (snapCandidates.Size())
					{
						pointClusters.PushBack(new C_PointCluster());
						assert(pointMap.Find(vertexIndex) != pointMap.End());
						pointClusters.Back()->AddPoint(pointMap[vertexIndex]);
						for (S32 u=0; u<snapCandidates.Size(); ++u)
							pointClusters.Back()->AddPoint(snapCandidates[u]);
					}
				}
			}

			//point cluster merging ... O((n^2)/2) :(
			{
				LOGDEBUG(g_Log, "Merging %d point clusters", pointClusters.Size());
				for (S32 i=0; i<pointClusters.Size(); ++i)
				{
					for (S32 j=i+1; j<pointClusters.Size(); ++j)
					{
						if (pointClusters[i]->SetIntersects(*pointClusters[j]))
						{
							pointClusters[i]->Merge(*pointClusters[j]);
							delete pointClusters[j];
							pointClusters.EraseNonOrdered(j);
							--j;
						}
					}
				}
				LOGDEBUG(g_Log, "%d point clusters remaining after merging", pointClusters.Size());
			}

			//TODO: handle asymmetric merges properly (ie. when a vertex can snap to another, but not the other way around)
			for (S32 i=0; i<pointClusters.Size(); ++i)
			{
				const C_PointCluster::PointSet& points = pointClusters[i]->GetPoints();
				for (C_PointCluster::PointSet::ConstIterator it=points.Begin(); it!=points.End(); ++it)
				{
					const Point& p = *it->GetKey();

					Ut::OpenHashMap<S32, Aftk::Position3Array>::Iterator positionArrayIt = worldPositions.Find(p.node);
					assert(positionArrayIt != worldPositions.End());
					Aftk::Position3Array& positions = positionArrayIt->GetValue();
					positions[p.vertex] = pointClusters[i]->ClusterCentre();
				}
			}

			//clean up
			for (S32 i=0; i<pointClusters.Size(); ++i)
				delete pointClusters[i];
		}
	}


	//assign
	for (S32 i=0; i<validNodes.Size(); ++i)
	{
		Aftk::C_SceneMeshNode *pMeshNode = (Aftk::C_SceneMeshNode*)validNodes[i];

		Ut::OpenHashMap<S32, Aftk::Position3Array>::Iterator it = worldPositions.Find(pMeshNode->GetUniqueId());
		assert(it != worldPositions.End());
		Aftk::Position3Array& positions = it->GetValue();

		for (S32 j=0; j<positions.Size(); ++j)
			positions[j] = Aftk::ApplyMatrix(pMeshNode->GetWorldMatrixInverse(), positions[j]);

		pMeshNode->GetVertexPositionArray() = positions; 
	}

	return Aftk::C_Status::E_Status_Success;
}
