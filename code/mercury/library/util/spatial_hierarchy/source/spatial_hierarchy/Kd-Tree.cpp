#include "StdAfx.h"

#include "spatial_hierarchy/Tree-Common.h"
#include "spatial_hierarchy/Kd-TreeBuilder.h"
#include "spatial_hierarchy/Kd-Tree.h"
#include "spatial_hierarchy/Primitive.h"
#include "spatial_hierarchy/LpvsUtility.h"

//#define STATS

// build a kd-tree using the KdBuilder class
void KdTree::Build(Aftk::C_SceneNode *pNode)
{
	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server.kdtree");

	m_pSceneNode = pNode;

//	OutputDebugString("-- collision tree\n");

	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server.kdtree.primitives");

	// Compute bounds for kd-tree construction
	Box3Vector primBounds;
	primBounds.Reserve(m_Primitives.Size());
	for (int i = 0; i < m_Primitives.Size(); ++i)
	{
		// create bbox
		Maths::C_AxisBox bbox;
		for (int j=0; j<3; j++)
			bbox.UpdateExtents( Maths::HVec3(&m_Primitives[i].m_v[j]) );

		primBounds.PushBack(bbox);
	}

	MEMORY_TRACKER_POP("mercury.spatialhierarchy.server.kdtree.primitives");

	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server.kdtree.leaves");

	KdBuilder *pBuilder = new KdBuilder (primBounds,
		1,		// intersect cost
		10,		// traversal cost
		5.5f,	// empty bonus
		4,		// min primitives per leafnode
		-1		// max depth
		);
	MEMORY_TRACKER_POP("mercury.spatialhierarchy.server.kdtree.leaves");

	// copy Nodes vector to a cache-aligned memory block
	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server.kdtree.nodes");
	m_nNodes = pBuilder->m_Nodes.Size()+1;
	m_pNodes = (KdNode*)BizAlignedMalloc(sizeof(KdNode) * (m_nNodes), 128);
	memcpy(m_pNodes+1, &pBuilder->m_Nodes[0], sizeof(KdNode) * pBuilder->m_Nodes.Size());
	MEMORY_TRACKER_POP("mercury.spatialhierarchy.server.kdtree.nodes");

	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server.kdtree.triaccel");

	// build TriAccel list
	// + count number of optional info
	int nOptionalInfo = 0;
	m_nTris = m_Primitives.Size();
	m_pTris = (TriAccel*)BizAlignedMalloc(sizeof(TriAccel) * m_nTris, 128);
	for (int i=0; i<m_Primitives.Size(); i++)
	{
		m_pTris[i].Initialize( &m_Primitives[i] );
		if (m_Primitives[i].m_iAlphaTex != -1)
			nOptionalInfo++;
	}

	MEMORY_TRACKER_POP("mercury.spatialhierarchy.server.kdtree.triaccel");

	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server.kdtree.optionalinfo");

	// initialize optional info
	m_nOptionalTriInfo = nOptionalInfo;
	m_pOptionalInfo = new OptionalTriInfo[ nOptionalInfo ];
	nOptionalInfo = 0;
	for (int i=0; i<m_Primitives.Size(); i++)
	{
		TrianglePrim *pTri = &m_Primitives[i];

		int x = -1;
		if (pTri->m_iAlphaTex != -1)
		{
			m_pOptionalInfo[nOptionalInfo].iAlphaTex = pTri->m_iAlphaTex;
			m_pOptionalInfo[nOptionalInfo].m_uv[0]   = pTri->m_uv[0];
			m_pOptionalInfo[nOptionalInfo].m_uv[1]   = pTri->m_uv[1];
			m_pOptionalInfo[nOptionalInfo].m_uv[2]   = pTri->m_uv[2];

			x = nOptionalInfo;

			nOptionalInfo++;
		}

		m_pTris[i].m_k = (m_pTris[i].m_k&3) | (x<<2);
	}

	MEMORY_TRACKER_POP("mercury.spatialhierarchy.server.kdtree.optionalinfo");

	m_pMailboxes = pBuilder->m_pMailboxes;
	pBuilder->m_pMailboxes = NULL;

	// resize the vectors to be exactly the right size
	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server.kdtree.primitives");
	m_Primitives.Reserve( m_Primitives.Size() );
	MEMORY_TRACKER_POP("mercury.spatialhierarchy.server.kdtree.primitives");
	MEMORY_TRACKER_PUSH("mercury.spatialhierarchy.server.kdtree.leaves");
	m_pMailboxes->Reserve( m_pMailboxes->Size() );
	MEMORY_TRACKER_POP("mercury.spatialhierarchy.server.kdtree.leaves");

	delete pBuilder;

	MEMORY_TRACKER_POP("mercury.spatialhierarchy.server.kdtree");
}

KdTree::~KdTree()
{
	if (m_pTris)
		BizAlignedFree(m_pTris);
	if (m_pOptionalInfo)
		delete [] m_pOptionalInfo;
}

// this function call the routine that does the actual traversing of the tree
void KdTree::RayTrace(const SD::Ray &ray, SD::RayTraceResult &result) const
{
	result.iPrimitive = -1;
	result.iPolygon = -1;
	Trace(ray,result);
	if (result.iPrimitive == -1)
	{
		result.bHit = false;
		return;
	}

	result.bHit = true;

	// get more information about the hit
	if (m_Primitives.Size())
	{
		const TrianglePrim *pPrim = &m_Primitives[result.iPrimitive];
		result.iPolygon = pPrim->m_iPolygon;
		result.vVerts[0] = pPrim->m_v[0];
		result.vVerts[1] = pPrim->m_v[1];
		result.vVerts[2] = pPrim->m_v[2];
		result.pMeshNode = pPrim->m_pMeshNode;
	}

	result.iGlobalPrim = result.iPrimitive + m_iFirstGlobalPrimIdx;

	// calculate impact point + normal
	if (!ray.bShadowRay)
	{
		result.vHitPos = ray.origin + ray.direction * result.fDistance;
	}
}

// calculate the normal for the primitive that we hit
Float3 SD::RayTraceResult::GetSurfaceNormal() const
{
	Float3 vNormal;
	int u,v,k;

	k = pTri->m_k & 3;
	u = (k+1) %3; v = (k+2) %3;
	vNormal.v[u] = pTri->n_u;
	vNormal.v[v] = pTri->n_v;
	vNormal.v[k] = 1.0f;
	
	vNormal = Normalize(vNormal);

	return vNormal;
}

