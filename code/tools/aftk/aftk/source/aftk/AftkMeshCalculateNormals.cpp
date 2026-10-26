#include "stdafx.h"

using namespace Aftk;

C_Status C_MeshCalculateNormals::CalculateNormals(C_SceneMeshNode* pMesh)
{
	C_MeshExpandPolygonVertices::ExpandPolygonVertices(pMesh);

	for (U32 iLodLevel = 0; iLodLevel < pMesh->GetNumLodLevels(); iLodLevel++)
		for (U32 jPoly = 0; jPoly < pMesh->GetNumPolygons(iLodLevel); jPoly++)
		{
			C_Polygon* pPoly = &pMesh->GetPolygon(iLodLevel, jPoly);
			U32 faceNormalIndex = pMesh->AddVertexNormal(pPoly->GetNormal());

			for (U32 kPV = 0; kPV < pPoly->GetNumPolygonVertices(); kPV++)
			{
				C_PolygonVertex* pPV = &pPoly->GetPolygonVertex(kPV);
				pPV->SetNormalIndex(faceNormalIndex);
			}
		}

	pMesh->RepairVertexNormals();

	return C_Status::E_Status_Success;
}


