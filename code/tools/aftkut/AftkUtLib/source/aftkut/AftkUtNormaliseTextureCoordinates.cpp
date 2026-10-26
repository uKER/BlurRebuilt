#include "stdafx.h"
#pragma message("FIX THIS: INCLUSION OF OLD MATHSUTILITY.H")
#include <tools_common/mathsutility.h>

namespace AftkUt
{
	Aftk::C_Status C_NormaliseTextureCoordinates::Normalise(Aftk::C_Scene* pScene)
	{
		Aftk::C_Status status;
		Aftk::C_SceneNodeIterator it(pScene);
		while(it)
		{
			Aftk::C_SceneNode* pSceneNode = it;
			if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				Aftk::C_SceneMeshNode* pMesh = (Aftk::C_SceneMeshNode*)pSceneNode;
				status = Normalise(pMesh);
				if (!status) return status;
			}
			it++;
		}
		return Aftk::C_Status::E_Status_Success;
	}

	void C_NormaliseTextureCoordinates::PolygonTiling(
		U32 uvSet,
		const Aftk::C_Polygon* pPoly, 
		F32& tilingU,
		F32& tilingV)
	{
		F32 uMin = 0;
		F32 vMin = 0;
		F32 uMax = 0;
		F32 vMax = 0;
		for (U32 lPV = 0; lPV < pPoly->GetNumPolygonVertices(); lPV++)
		{
			const Aftk::C_PolygonVertex* pPV = &pPoly->GetPolygonVertex(lPV);

			S32 uvIndex = pPV->GetTextureCoordinateIndex(uvSet);
			if (uvIndex == -1) 
				continue;

			Aftk::TextureCoordinate4 uvwx = 
				pPoly->GetMeshNodePtr()->GetTextureCoordinate(uvSet, uvIndex);

			if (lPV == 0)
			{
				uMin = uvwx[0];
				uMax = uvwx[0];
				vMin = uvwx[1];
				vMax = uvwx[1];
			}
			else
			{
				if (uvwx[0] < uMin)
				{
					uMin = uvwx[0];
				}
				else if (uvwx[0] > uMax)
				{
					uMax = uvwx[0];
				}
				if (uvwx[1] < vMin)
				{
					vMin = uvwx[1];
				}
				else if (uvwx[1] > vMax)
				{
					vMax = uvwx[1];
				}
			}
		}
		F32 du = uMax - uMin;
		F32 dv = vMax - vMin;

		tilingU = du;
		tilingV = dv;
	}

	bool C_NormaliseTextureCoordinates::IsPolygonTiled(U32 uvSet, const Aftk::C_Polygon* pPoly)
	{
		U32 uSign = 0;
		U32 vSign = 0;
		F32 uMin = 0;
		F32 vMin = 0;
		F32 uMax = 0;
		F32 vMax = 0;
		bool equalSgn = true;
		for (U32 lPV = 0; lPV < pPoly->GetNumPolygonVertices(); lPV++)
		{
			const Aftk::C_PolygonVertex* pPV = &pPoly->GetPolygonVertex(lPV);

			S32 uvIndex = pPV->GetTextureCoordinateIndex(uvSet);
			if (uvIndex == -1) 
				continue;

			Aftk::TextureCoordinate4 uvwx = 
				pPoly->GetMeshNodePtr()->GetTextureCoordinate(uvSet, uvIndex);

			if (lPV == 0)
			{
				uSign = Tu::Sgn(uvwx[0]);
				vSign = Tu::Sgn(uvwx[1]);
				uMin = uvwx[0];
				uMax = uvwx[0];
				vMin = uvwx[1];
				vMax = uvwx[1];
			}
			else
			{
				if (uSign !=  Tu::Sgn(uvwx[0]))
				{
					equalSgn = false;
				}
				if (vSign !=  Tu::Sgn(uvwx[1]))
				{
					equalSgn = false;
				}
				if (uvwx[0] < uMin)
				{
					uMin = uvwx[0];
				}
				else if (uvwx[0] > uMax)
				{
					uMax = uvwx[0];
				}
				if (uvwx[1] < vMin)
				{
					vMin = uvwx[1];
				}
				else if (uvwx[1] > vMax)
				{
					vMax = uvwx[1];
				}
			}
		}
		F32 du = uMax - uMin;
		F32 dv = vMax - vMin;

		//If we're not tiled then return false
		if ((du <= 1.f) && (dv <= 1.f) && equalSgn)
		{
			return false;
		}

		//We're tiled
		return true;
	}

	Aftk::C_Status C_NormaliseTextureCoordinates::Normalise(Aftk::C_SceneMeshNode* pMesh)
	{
		//Unshare polygon vertices.
		Aftk::C_MeshExpandPolygonVertices::ExpandPolygonVertices(pMesh);
		
		for (U32 iLodLevel = 0; iLodLevel < pMesh->GetNumLodLevels(); iLodLevel++)
		{
			for (U32 jPoly = 0; jPoly < pMesh->GetNumPolygons(iLodLevel); jPoly++)
			{
				Aftk::C_Polygon* pPoly = &pMesh->GetPolygon(iLodLevel, jPoly);

				for (U32 kSet = 0; kSet < pMesh->GetNumTextureCoordinateSets(); kSet++)
				{
					//If we're not tiled then normalise UVs
					if (!IsPolygonTiled(kSet, pPoly))
					{
						for (U32 lPV = 0; lPV < pPoly->GetNumPolygonVertices(); lPV++)
						{
							Aftk::C_PolygonVertex* pPV = &pPoly->GetPolygonVertex(lPV);

							S32 uvIndex = pPV->GetTextureCoordinateIndex(kSet);
							if (uvIndex == -1) 
								continue;

							//Expand UVs.
							{
								Aftk::TextureCoordinate4 uvwx = 
									pPoly->GetMeshNodePtr()->GetTextureCoordinate(kSet, uvIndex);
								U32 newUVIndex = pPoly->GetMeshNodePtr()->AddTextureCoordinate(kSet, uvwx);
								pPV->SetTextureCoordinateIndex(kSet, newUVIndex);
							}

							Aftk::TextureCoordinate4& uvwx = 
								pMesh->GetTextureCoordinate(kSet, uvIndex);

							//Normalise.
							if ((uvwx[0] > 1.f) || (uvwx[0] < 0.f))
								uvwx[0] -= floor(uvwx[0]);
							if ((uvwx[1] > 1.f) || (uvwx[1] < 0.f))
								uvwx[1] -= floor(uvwx[1]);
						}
					}
				}
			}
		}
		/*
		pMesh->RepairTextureCoordinates();
		pMesh->WeldTextureCoordinates();
		pMesh->WeldPolygonVertices();
		*/
		return Aftk::C_Status::E_Status_Success;
	}
}
