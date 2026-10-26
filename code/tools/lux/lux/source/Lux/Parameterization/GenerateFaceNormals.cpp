//-----------------------------------------------------------------------------
// File: GenerateFaceNormals.cpp
//
// Desc: 'exactly what it says on the tin'
//		 
//-----------------------------------------------------------------------------
#include "stdafx.h"

#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/Parameterization/GenerateFaceNormals.h"

using namespace LUX;

#pragma optimize( "", off ) // turn off all optimizations

// generate average normal for each face
bool CGenerateFaceNormals::GenerateFaceNormals(Aftk::C_SceneMeshNode *pMesh)
{
	m_pMesh = pMesh;

	Aftk::Matrix44 mtx = pMesh->GetWorldMatrix();

    for (U32 i=0; i<m_pMesh->GetNumPolygons(0); i++)
	{
		Vector3 vNormal(0,0,0);

		Aftk::C_Polygon *pPoly = &m_pMesh->GetPolygon(i);

		for (int j=0; j<3; j++)
		{
			vNormal += POLYVERT_NORMAL(pPoly->GetPolygonVertex(j));
		}

		vNormal = LPVS::ApplyMatrix(mtx, *(Aftk::Normal3*)&vNormal );

		Normalise(&vNormal);

		POLY_DATA(i)->vNormal[0] = vNormal.x;
		POLY_DATA(i)->vNormal[1] = vNormal.y;
		POLY_DATA(i)->vNormal[2] = vNormal.z;
	}

	return true;
}

