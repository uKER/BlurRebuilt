/// Lux specific derivation of spatial hierarchy
#include "stdafx.h"
#include <system/memory/memorytracker.h>
#include "Lux/SpatialHierarchy.h"
#include <spatial_hierarchy\SerializeKdTree.h>

namespace LUX
{

/// Build a spatial hierarchy from a given Aftk::C_Scene
void SpatialHierarchy::Build(Aftk::C_Scene *pAftkScene, LayerMode *pLayerModes)
{
	SD::SpatialHierarchy::Build(pAftkScene, pLayerModes);

	int nPrims = SD::SpatialHierarchy::m_iGlobalPrimOffset + SD::SpatialHierarchy::m_pMailboxes->Size();
	m_PrimInfo.Resize(nPrims);

	if (m_PrimInfo.Size())
		memset(&m_PrimInfo[0], -1, sizeof(int) * nPrims);
}

/// Serialize a spatial hierarchy to an ost::Engine class for loading/saving.
void SpatialHierarchy::write(ost::Engine &ar)
{
	ar << m_PrimInfo;

	return SD::SpatialHierarchy::write(ar);
}

void SpatialHierarchy::read(ost::Engine &ar)
{
	MEMORY_TRACKER_PUSH("tools.lux.client.spatialhierarchy");

	ar >> m_PrimInfo;

	MEMORY_TRACKER_POP("tools.lux.client.spatialhierarchy");

	return SD::SpatialHierarchy::read(ar);
}

/// Retrieve extra info for a particular primitive
const PrimInfo &SpatialHierarchy::GetAdditionalPrimInfo(int iPrim) const
{
	if (iPrim >= m_PrimInfo.Size())
	{
		LOGERROR( g_Log, "Internal error in GetAdditionalPrimInfo\n");
		return m_PrimInfo[0];
	}

	return m_PrimInfo[iPrim];
}

/// Set extra info for a particular primitive
void SpatialHierarchy::SetAdditionalPrimInfo(int iPrim, const PrimInfo &info)
{
	m_PrimInfo[iPrim] = info;
}

}; // namespace LUX
