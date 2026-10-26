#include "stdafx.h"

using namespace Aftk;

C_Status C_MeshTriangulate::Triangulate(C_SceneMeshNode* pMesh)
{
	C_Status status;
	for (U32 iLodLevel = 0; iLodLevel < (U32)pMesh->GetNumLodLevels(); iLodLevel++)
	{
		status = Triangulate(pMesh, iLodLevel);
		if (!status) return status;
	}
	return C_Status::E_Status_Success;
}

C_Status C_MeshTriangulate::Triangulate(C_SceneMeshNode* pMesh, U32 lodLevel)
{
	pMesh->DirtyGeometryDependantCaches();
	C_Status status;

	//Find the poly with the largest number of PVs.
	U32 largestPoly = 3;
	for (U32 iPoly = 0; iPoly < pMesh->GetNumPolygons(lodLevel); iPoly++)
	{
		if (pMesh->GetPolygon(lodLevel, iPoly).GetNumPolygonVertices() > largestPoly)
		{
			largestPoly = pMesh->GetPolygon(lodLevel, iPoly).GetNumPolygonVertices();
		}
	}
	if (largestPoly == 3)
	{
		//Lod level already triangulated - abort.
		return C_Status::E_Status_Success;
	}
	//Allocate memory for active vertex array.
	Ut::UtVector<Ut::DynamicVector<bool> > activeVertexArray;
	activeVertexArray.Resize(largestPoly);
	Ut::UtVector<Ut::DynamicVector<bool> > vertexClassification;
	vertexClassification.Resize(largestPoly);

	//Reserve memory for the new triangle list.
	U32 numTriangles = pMesh->GetNumTriangles(lodLevel);
	Ut::UtVector< Ut::DynamicVector<C_Polygon> > newPolygonArray;
	newPolygonArray.Reserve(numTriangles);

	//Remove all of the polygons of this lod level from their clusters
	for (U32 i = 0; i < pMesh->GetNumPolygonClusters(); i++)
		pMesh->GetPolygonClusterPtr(i)->ClearPolygonIndices(lodLevel);

	Matrix44 worldMatrix = pMesh->GetWorldMatrix();
	for (U32 iPoly = 0; iPoly < pMesh->GetNumPolygons(lodLevel); iPoly++)
	{
		C_Polygon* pPoly = &pMesh->GetPolygon(lodLevel, iPoly);
		pPoly->Triangulate(
			newPolygonArray, 
			activeVertexArray,
			vertexClassification,
			&worldMatrix);
	}

	//Overwrite old polygon array for this lod level.
	pMesh->GetPolygonArray(lodLevel) = newPolygonArray;
	for (U32 iPoly = 0; iPoly < pMesh->GetNumPolygons(lodLevel); iPoly++)
	{
		C_Polygon* pPoly = &pMesh->GetPolygon(lodLevel, iPoly);
		pPoly->GetClusterPtr()->AddPolygonIndexDoNotModifyPolygon(lodLevel, iPoly);
	}

	return C_Status::E_Status_Success;
}
