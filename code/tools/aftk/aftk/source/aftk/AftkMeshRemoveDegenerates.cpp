#include "stdafx.h"

using namespace Aftk;

U32 C_MeshRemoveDegenerates::RemoveDegenerates(Aftk::C_SceneMeshNode* pMesh)
{
	U32 deg = 0;
	for (U32 iLodLevel = 0; iLodLevel < pMesh->GetNumLodLevels(); iLodLevel++)
		deg += RemoveDegenerates(pMesh, iLodLevel);

	return deg;
}

U32 C_MeshRemoveDegenerates::RemoveDegenerates(Aftk::C_SceneMeshNode* pMesh, U32 lodLevel)
{
	if (pMesh->GetNumPolygons(lodLevel) == 0)
		return 0;

	pMesh->DirtyGeometryDependantCaches();

	U32 numRepairs = 0;
	Ut::UtVector< Ut::DynamicVector<C_PolygonCluster*> > clusterArray;
	Ut::UtVector< Ut::DynamicVector<C_Polygon> > newPolygonArray;

	Ut::Vector<S32> remapping;
	remapping.Resize(pMesh->GetNumPolygons(lodLevel));
	memset(&remapping[0], -1, sizeof(S32)*remapping.Size());

	for (U32 i = 0; i < pMesh->GetNumPolygonClusters(); i++)
	{
		C_PolygonCluster* pCluster = pMesh->GetPolygonClusterPtr(i);
		AFTK_ASSERT(pCluster->GetMeshNodePtr());
	}
	U32 numPolys = pMesh->GetNumPolygons(lodLevel);
	newPolygonArray.Reserve(numPolys);
	{
		//Remove any polygons with < 3 poly verts
		for (U32 polyIndex = 0; polyIndex < numPolys; polyIndex++)
		{
			C_Polygon* pOriginalPoly = &pMesh->GetPolygon(lodLevel, polyIndex);
			if (pOriginalPoly->GetNumPolygonVertices() < 3)
			{
				#ifdef _DEBUG
//				g_aftkDebugManager << m_name << " Removing polygon " << polyIndex << " with < 3 poly verts\n";
				#endif
				numRepairs++;
				continue;
			}
			Matrix44 worldMatrix = pMesh->GetWorldMatrix();
			if (pOriginalPoly->GetArea(&worldMatrix) == 0)
			{
				#ifdef _DEBUG
//				g_aftkDebugManager << m_name << " Removing polygon " << polyIndex << " with zero area\n";
				#endif
				numRepairs++;
				continue;
			}

			newPolygonArray.PushBack(*pOriginalPoly);
			S32 newPolyIndex = newPolygonArray.Size()-1;
			C_Polygon* pNewPoly = &newPolygonArray[newPolyIndex];
			remapping[polyIndex] = newPolyIndex;
			AFTK_ASSERT(pOriginalPoly->GetClusterPtr());
			clusterArray.PushBack(pOriginalPoly->GetClusterPtr());
		}
	}
	pMesh->GetPolygonArray(lodLevel) = newPolygonArray;

	//Clear and rebuild polygon clusters.
	for (U32 i = 0; i < pMesh->GetNumPolygonClusters(); i++)
		pMesh->GetPolygonClusterPtr(i)->ClearPolygonIndices(lodLevel);

	for (U32 i = 0; i < pMesh->GetNumPolygons(lodLevel); i++)
	{
		C_PolygonCluster* pPolyCluster = clusterArray[i];
		AFTK_ASSERT(pPolyCluster);
		pPolyCluster->AddPolygonIndex(lodLevel, i);

		//Compensate base indices.
		Aftk::C_Polygon& poly = pMesh->GetPolygon(lodLevel, i);

		//The basePolyIndex < remapping.Size() check is to account for existing bad data.
		S32 basePolyIndex = poly.GetBasePolygonIndex();
		if (basePolyIndex != -1 && 
			basePolyIndex < remapping.Size())
		{
			//Remap
			poly.SetBasePolygonIndex(remapping[basePolyIndex]);
		}
	}

	return numRepairs;
}
