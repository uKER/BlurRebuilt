#include <stdafx.h>

//#pragma message("TODO: C_PolygonCluster The currently poly-cluster system is overcomplicated and difficult to use in the context of mesh editing. It needs improving.")
//#pragma message("TODO: C_PolygonCluster Implement mesh weld with public interface only.")
//#pragma message("TODO: C_PolygonCluster Implement mesh degenerate removal with public interface only.")
//#pragma message("TODO: C_PolygonCluster Implement mesh triangulation with public interface only.")

#define AFTK_RESIZE_CLUSTER_LOD_LEVELS_IF_NEEDED if (lodLevel >= (U32)m_polygonIndices.Size()) {	m_polygonIndices.Resize(lodLevel+1); }

using namespace Aftk;

C_PolygonCluster::C_PolygonCluster(C_SceneMeshNode* pMesh)
:
C_ComponentCluster(pMesh)
{
	OneTimeInit();
}

C_PolygonCluster::~C_PolygonCluster()
{
}

const Aftk::C_PolygonCluster& C_PolygonCluster::operator=(const Aftk::C_PolygonCluster& rhs)
{
	Aftk::C_ComponentCluster::operator=(rhs);

	m_polygonIndices = rhs.m_polygonIndices;
	return *this;
}

E_ComponentClusterType C_PolygonCluster::GetType() const
{
	return E_ComponentClusterType_Polygon;
}

S32 C_PolygonCluster::AddPolygonIndex(U32 lodLevel, U32 index)
{
	AFTK_ASSERT(m_pMeshNode);
	AFTK_RESIZE_CLUSTER_LOD_LEVELS_IF_NEEDED

	S32 newIndex = AddPolygonIndexDoNotModifyPolygon(lodLevel, index);
	//Attach poly to cluster.
	if (newIndex != -1)
	{
		//Remove the polygon from its old cluster.
		Aftk::C_Polygon* pPoly = &m_pMeshNode->GetPolygon(lodLevel, index);
		C_PolygonCluster* pPolyCluster = pPoly->GetClusterPtr();
		if (pPolyCluster == this)
		{
			//Already a member of this cluster!
			return -1;
		}
		if (pPolyCluster)
		{
			//Detach from poly cluster of another mesh if needed.
			if (pPolyCluster->GetMeshNodePtr() == m_pMeshNode)
			{
				pPolyCluster->DeletePolygonIndex(lodLevel, index);
			}
			else
			{
				pPoly->m_pPolygonCluster = 0;
			}
		}
		//Attach this polygon to this cluster.
		pPoly->SetClusterPtr(this);
	}

	return newIndex;
}
/*
void C_PolygonCluster::SetPolygonIndex(U32 instance, U32 lodLevel, U32 index)
{
	AFTK_ASSERT(lodLevel >= (m_polygonIndices.Size());
	AFTK_ASSERT(instance >= (m_polygonIndices[lodLevel].Size());
	m_polygonIndices[lodLevel][instance] = index;
}
*/
void C_PolygonCluster::SetPolygonIndexInitialCapacity(U32 lodLevel, U32 capacity)
{
	AFTK_RESIZE_CLUSTER_LOD_LEVELS_IF_NEEDED
	m_polygonIndices[lodLevel].Reserve(capacity);
}

S32 C_PolygonCluster::GetPolygonIndex(U32 lodLevel, U32 instance) const
{
	if (lodLevel >= (U32)m_polygonIndices.Size())
	{
		AFTK_ASSERT(lodLevel >= (U32)m_polygonIndices.Size());
		return -1;
	}
	if (instance >= (U32)m_polygonIndices[lodLevel].Size())
	{
		AFTK_ASSERT(instance >= (U32)m_polygonIndices[lodLevel].Size());
		return -1;
	}

	return m_polygonIndices[lodLevel][instance];
}

U32 C_PolygonCluster::GetNumPolygonIndices(U32 lodLevel) const
{
	return m_polygonIndices[lodLevel].Size();
}

void C_PolygonCluster::DeletePolygonIndex(U32 lodLevel, U32 index)
{
	DeletePolygonIndex(lodLevel, index, false);
}

void C_PolygonCluster::DeletePolygonIndex(U32 lodLevel, U32 index, bool forceDelete)
{
	if (lodLevel >= (U32)m_polygonIndices.Size())
	{
		AFTK_ASSERT(lodLevel >= (U32)m_polygonIndices.Size());
		return;
	}

	for (S32 i = 0; i < (S32)m_polygonIndices[lodLevel].Size(); i++)
	{
		if (m_polygonIndices[lodLevel][i] == index)
		{
			m_polygonIndices[lodLevel].EraseByIndex(i);
			//Account for our size being decremented.
			i--;

			if (index < m_pMeshNode->GetNumPolygons(lodLevel))
			{
				//Remove mesh node from cluster.
				Aftk::C_Polygon* pPolygon = &m_pMeshNode->GetPolygon(lodLevel, index);
				if (!forceDelete && m_pMeshNode->GetPolygonClusterPtr(0) == this)
				{
					//Cannot remove a polygon from the default cluster.
					return;
				}

				//Add to default cluster, if not forcing deletion from default cluster.
				if (!forceDelete && pPolygon->GetClusterPtr() == this)
				{
					//Add to default cluster.
					m_pMeshNode->GetPolygonClusterPtr(0)->AddPolygonIndex(lodLevel, index);
				}
			}
		}
	}
}


void C_PolygonCluster::DeletePolygonIndexDoNotModifyPolygon(U32 lodLevel, U32 index)
{
	if (lodLevel >= (U32)m_polygonIndices.Size())
	{
		AFTK_ASSERT(lodLevel >= (U32)m_polygonIndices.Size());
		return;
	}

	for (U32 i = 0; i < (U32)m_polygonIndices.Size(); i++)
	{
		if (m_polygonIndices[lodLevel][i] == index)
		{
			m_polygonIndices[lodLevel].EraseByIndex(i);
		}
	}
}

void C_PolygonCluster::DeletePolygonIndicesDoNotModifyPolygons(U32 lodLevel)
{
	if (lodLevel >= (U32)m_polygonIndices.Size())
	{
		AFTK_ASSERT(lodLevel >= (U32)m_polygonIndices.Size());
		return;
	}

	m_polygonIndices[lodLevel].Clear();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_PolygonCluster::ClearPolygonIndices()
{
	//Remove polygon indices for all lod levels.
	m_polygonIndices.Clear();
}

void C_PolygonCluster::ClearPolygonIndices(U32 lodLevel)
{
	if (lodLevel >= (U32)m_polygonIndices.Size())
	{
		AFTK_ASSERT(lodLevel >= (U32)m_polygonIndices.Size());
		return;
	}
	//Remove polygon indices for a particular lod level.
	m_polygonIndices[lodLevel].Clear();
}

void C_PolygonCluster::OneTimeInit()
{
	m_name = "default";
}

void C_PolygonCluster::Clear()
{
	ClearThis();
	C_ComponentCluster::Clear();
}

void C_PolygonCluster::ClearThis()
{
	m_name = "default";
	m_polygonIndices.Clear();
}

U32 C_PolygonCluster::Repair()
{
	U32 fixCount = 0;

	Ut::UtVector<Ut::DynamicVector<
		Ut::UtSet<U32> > > polyIndexSets;
	polyIndexSets.Resize(m_pMeshNode->GetNumLodLevels());

	{
		Aftk::C_PolygonCluster clusterCopy(*this);
		//Remove duplicate entries.
		for (U32 iLodLevel = 0; iLodLevel < clusterCopy.GetNumLodLevels(); iLodLevel++)
		{
			for (U32 jPoly = 0; jPoly < clusterCopy.GetNumPolygonIndices(iLodLevel); jPoly++)
			{
				U32 polygonIndex = clusterCopy.GetPolygonIndex(iLodLevel, jPoly);
				if (polygonIndex < m_pMeshNode->GetNumPolygons(iLodLevel))
				{
					Ut::UtSet<U32>::Iterator iter;
					iter = polyIndexSets[iLodLevel].Find(polygonIndex);
					if (iter == polyIndexSets[iLodLevel].End())
						polyIndexSets[iLodLevel].Insert(polygonIndex);
					else
						DeletePolygonIndexDoNotModifyPolygon(iLodLevel, polygonIndex);
				}
			}
		}
	}

	{
		C_PolygonCluster clusterCopy(*this);
		for (U32 iLodLevel = 0; iLodLevel < clusterCopy.GetNumLodLevels(); iLodLevel++)
		{
			for (U32 jPoly = 0; jPoly < clusterCopy.GetNumPolygonIndices(iLodLevel); jPoly++)
			{
				U32 polygonIndex = clusterCopy.GetPolygonIndex(iLodLevel, jPoly);
				if (iLodLevel >= (U32)m_pMeshNode->GetNumLodLevels())
				{
					DeletePolygonIndex(iLodLevel, polygonIndex);
					fixCount++;
				}
				else
				{
					if (polygonIndex >= m_pMeshNode->GetNumPolygons(iLodLevel))
					{
						DeletePolygonIndex(iLodLevel, polygonIndex);
						fixCount++;
					}
				}
			}
		}
	}

	return fixCount;
}

U32 C_PolygonCluster::GetNumLodLevels() const
{
	return (U32)m_polygonIndices.Size();
}

S32 C_PolygonCluster::AddPolygonIndexDoNotModifyPolygon(U32 lodLevel, U32 index)
{
	AFTK_RESIZE_CLUSTER_LOD_LEVELS_IF_NEEDED

	m_polygonIndices[lodLevel].PushBack(index);

	return m_polygonIndices.Size()-1;
}

bool C_PolygonCluster::operator== (const Aftk::C_PolygonCluster& rhs) const
{
	const U32 nLods = (U32)m_polygonIndices.Size(); 
	const U32 nLodsOther = (U32)rhs.m_polygonIndices.Size(); 

	if (nLods != nLodsOther)
	{
		return false;
	}

	for (U32 lod = 0; lod < nLods; lod++)
	{
		if (!Aftk::EqualityTestVector(m_polygonIndices[lod], rhs.m_polygonIndices[lod]))
		{
			return false;
		}
	}

	return true;
}

void C_PolygonCluster::ShufflePolygonIndices(U32 lod, U32 polyIndex)
{
	if (lod >= (U32)m_polygonIndices.Size())
		return;

	for (S32 kPoly = 0; kPoly < m_polygonIndices[lod].Size(); kPoly++)
	{
		if (m_polygonIndices[lod][kPoly] > polyIndex)
			m_polygonIndices[lod][kPoly]--;
	}
}

#endif
