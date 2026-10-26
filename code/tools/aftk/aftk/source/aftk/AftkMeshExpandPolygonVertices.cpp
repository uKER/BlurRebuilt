#include "stdafx.h"

using namespace Aftk;

U32 C_MeshExpandPolygonVertices::ExpandPolygonVertices(C_SceneMeshNode* pMesh)
{
	//Animation helpers are being invalidated here.
	pMesh->DirtyGeometryDependantCaches();

	U32 expansions = 0;
	for (U32 iLodLevel = 0; iLodLevel < pMesh->GetNumLodLevels(); iLodLevel++)
	{
		expansions += ExpandPolygonVertices(
			pMesh,
			iLodLevel);
	}
	return expansions;
}

U32 C_MeshExpandPolygonVertices::ExpandPolygonVertices(C_SceneMeshNode* pMesh, U32 lodLevel)
{
	PolygonVertexArray& polygonVertices = pMesh->GetPolygonVertexArray();
	PolygonVertexArray newPolygonVertices;

	//Calculate the number of unshared polygon vertices.
	U32 numNewPolygonVertices = 0;
	for (U32 iPoly = 0; iPoly < (U32)pMesh->GetNumPolygons(lodLevel); iPoly++)
	{
		C_Polygon* pPoly = &pMesh->GetPolygon(lodLevel, iPoly);
		numNewPolygonVertices += pPoly->GetNumPolygonVertices();
	}
	//Reserve memory
	newPolygonVertices.Reserve(numNewPolygonVertices);

	//Copy polygon vertices
	for (U32 iPoly = 0; iPoly < pMesh->GetNumPolygons(lodLevel); iPoly++)
	{
		C_Polygon* pPoly = &pMesh->GetPolygon(lodLevel, iPoly);
		for (U32 iPolyVert = 0; iPolyVert < pPoly->GetNumPolygonVertices(); iPolyVert++)
		{
			//Copy polygon vertex
			newPolygonVertices.PushBack(pPoly->GetPolygonVertex(iPolyVert));
			//Index new polygon vertex
			pPoly->SetPolygonVertexIndex(iPolyVert, newPolygonVertices.Size()-1);
		}
	}

	//Overwrite old polygon vertex array.
	S32 numUnshared = newPolygonVertices.Size() - polygonVertices.Size();
	if (numUnshared < 0)
	{
		//I can only see this happening if we had unused polygon vertex data. In this case, our
		//new polygon vertex array could be, oddly enough, smaller than our original polygon vertex array.
		numUnshared = 0;
	}

	Ut::MoveRaw(polygonVertices, newPolygonVertices);
	return (U32)numUnshared;
}
