#include "stdafx.h"

#include "../XsiIoUtility.h"

#include "XsiIoSceneImport.h"
#include "XsiIoSceneNodeImport.h"
#include "XsiIoSceneMeshImport.h"
#include "XsiIoImportOptions.h"

using namespace XsiIo;

Aftk::C_Status C_SceneMeshNodeImport::Import(
	C_SceneImport* pSceneImport,
	CSLModel* pXsiNode, 
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;
	status = C_SceneNodeImport::Import(
		pSceneImport,
		pXsiNode,
		pAftkSceneNode);
	AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;

	//Get the XSI mesh
	CSLMesh* pXsiMesh = (CSLMesh*)pXsiNode->Primitive();
	CSLBaseShape* pBaseShape = pXsiMesh->Shape();

	//Get the AFTK mesh
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)pAftkSceneNode;

	// copy vertices
	{
		int nVertices = pBaseShape->GetVertexCount();
		CSLBaseShape::CSLVector3DArray& array = *pBaseShape->GetVertexList();

		pAftkSceneMeshNode->SetVertexPositionInitialCapacity(nVertices);
		for (int i = 0; i < nVertices; i++)
		{
			Aftk::Position3 p(array[i].GetX(), array[i].GetY(), array[i].GetZ());
			pAftkSceneMeshNode->AddVertexPosition(p);
		}
	}

	// copy normals
	{
		int nNormals = pBaseShape->GetNormalCount();
		CSLBaseShape::CSLVector3DArray& array = *pBaseShape->GetNormalList();

		pAftkSceneMeshNode->SetVertexNormalInitialCapacity(nNormals);
		for (int i = 0; i < nNormals; i++)
		{
			Aftk::Vector3F32 n(Aftk::Vector3F32(array[i].GetX(), array[i].GetY(), array[i].GetZ()));
			pAftkSceneMeshNode->AddVertexNormal((Aftk::Normal3&)n);
		}
	}

	switch (pBaseShape->Type()) 
	{
		case CSLTemplate::SI_SHAPE35:
		{
			CSLShape_35* pShape35 = (CSLShape_35*) pBaseShape;

			// copy texture coordinates
			int nTextureCoordArrays = pShape35->GetUVCoordArrayCount();
			if (nTextureCoordArrays == 0)
			{
				break;
			}

			pAftkSceneMeshNode->SetTextureCoordinateSetSize(nTextureCoordArrays);
			for (U32 set = 0; set < (U32)nTextureCoordArrays; set++)
			{
				CSLUVCoordArray** uvCoordArrays = pShape35->UVCoordArrays();
				CSLBaseShape::CSLVector2DArray& array = *uvCoordArrays[set]->GetUVCoordList();
				int nTextureCoords = uvCoordArrays[set]->GetUVCoordCount();

				pAftkSceneMeshNode->SetTextureCoordinateInitialCapacity(set, nTextureCoords);
				pAftkSceneMeshNode->SetTextureCoordinateSetName(set, uvCoordArrays[set]->GetTextureProjection());
				for (int i = 0; i < nTextureCoords; i++)
				{
					Aftk::TextureCoordinate4 v(array[i].GetX(), 1.f - array[i].GetY(), 0.f, 0.f);
					pAftkSceneMeshNode->AddTextureCoordinate(set, v);
				}
			}

			break;
		}
		case CSLTemplate::SI_SHAPE:
		{
			CSLShape* pShape = (CSLShape*) pBaseShape;
			int nTextureCoords = pShape->GetUVCoordCount();
			if (nTextureCoords == 0)
			{
				break;
			}
			CSLBaseShape::CSLVector2DArray& array = *pShape->GetUVCoordList();

			pAftkSceneMeshNode->SetTextureCoordinateInitialCapacity(0, nTextureCoords);
			for (int i = 0; i < nTextureCoords; i++)
			{
				Aftk::TextureCoordinate4 v(array[i].GetX(), 1.f - array[i].GetY(), 0.f, 0.f);
				pAftkSceneMeshNode->AddTextureCoordinate(0, v);
			}
		}
	}

	{
	}

	// copy polygons
	{
		int nPolygonLists = pXsiMesh->GetPolygonListCount();
		CSLPolygonList** ppPolygonLists = pXsiMesh->PolygonLists();
		S32 nTriangleLists = pXsiMesh->GetTriangleListCount();
		CSLTriangleList** ppTriangleLists = pXsiMesh->TriangleLists();
		
		//Pre-allocate polygon storage
		{
			U32 numPolys = 0;
			for (S32 iTriangleList = 0; iTriangleList < nTriangleLists; iTriangleList++)
			{
				numPolys += ppPolygonLists[iTriangleList]->GetPolygonCount();
			}
			for (S32 iPolyList = 0; iPolyList < nPolygonLists; iPolyList++)
			{
				numPolys += ppPolygonLists[iPolyList]->GetPolygonCount();
			}
			pAftkSceneMeshNode->SetPolygonInitialCapacity(0, numPolys);
		}

		{
			for (S32 iTriList = 0; iTriList < nTriangleLists; iTriList++)
			{
				//Add a poly cluster for this list and set its name.
				status = ImportTriangleCluster(
					pSceneImport,
					ppTriangleLists[iTriList],
					pAftkSceneMeshNode);
				AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
			}
		}
		{
			for (S32 iPolyList = 0; iPolyList < nPolygonLists; iPolyList++)
			{
				//Add a poly cluster for this list and set its name.
				status = ImportPolygonCluster(
					pSceneImport,
					ppPolygonLists[iPolyList],
					pAftkSceneMeshNode);
				AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
			}
		}
	}

	if (pSceneImport->GetOptionsPtr()->cleanUVs)
	{
		status = DeleteUnusedTextureCoordinates(pAftkSceneMeshNode);
		AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneMeshNodeImport::ImportTriangleCluster(
	C_SceneImport* pSceneImport,
	CSLTriangleList* pTriangleList,
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode)
{
	Aftk::C_Status status;

	CSLTriangleList::CSLTriangleArray* pPositionList= pTriangleList->GetVertexIndices();
	CSLTriangleList::CSLTriangleArray* pNormalList	= pTriangleList->GetNormalIndices();
	S32 nTriangles									= pTriangleList->GetTriangleCount();
	S32	nUVSets										= pTriangleList->GetUVArrayCount();

	//Add a new cluster and set its name
	S32 polyClusterIndex = pAftkSceneMeshNode->AddPolygonCluster();
	Aftk::C_PolygonCluster* pAftkPolygonCluster = pAftkSceneMeshNode->GetPolygonClusterPtr(polyClusterIndex);
	pAftkPolygonCluster->SetName(pTriangleList->GetName());

	S32 count = 0;
	U32 vertexCount = 3;
	for (S32 j = 0; j < nTriangles; j++)
	{
		// add polygon
		U32 aftkPolyIndex = pAftkSceneMeshNode->AddPolygon(0, polyClusterIndex, Aftk::C_Polygon());
		Aftk::C_Polygon* pPolygon = &pAftkSceneMeshNode->GetPolygon(0, aftkPolyIndex);
        count += vertexCount - 1;

		//Add first polygon vertex
		{
			SLTriangle* pXsiTrianglePositionIndices = &(*pPositionList)[count];
			Aftk::C_PolygonVertex polygonVertex;
			polygonVertex.SetPositionIndex(pXsiTrianglePositionIndices->m_iVtx1);

			if (pNormalList)
			{
				SLTriangle* pXsiTriangleNormalIndices = &(*pNormalList)[count];
				polygonVertex.SetNormalIndex(pXsiTriangleNormalIndices->m_iVtx1);
			}

			for (U32 set = 0; set < (U32)pAftkSceneMeshNode->GetNumTextureCoordinateSets(); set++)
			{
				if ((S32)set >= nUVSets)
				{
					polygonVertex.SetTextureCoordinateIndex(set, -1);
				}
				else
				{
					CSLTriangleList::CSLTriangleArray* pUVList = pTriangleList->GetUVIndices(set);
					if (pUVList)
					{
						SLTriangle* pXsiTriangleUVIndices = &(*pUVList)[count];
						polygonVertex.SetTextureCoordinateIndex(set, pXsiTriangleUVIndices->m_iVtx1);
					}
					else
					{
						polygonVertex.SetTextureCoordinateIndex(set, -1);
					}
				}
			}
			//Polys must ALWAYS have the same number of UV indices as there are UV sets.
			AFTK_ASSERT(polygonVertex.GetNumTextureCoordinateIndices() == pAftkSceneMeshNode->GetNumTextureCoordinateSets());
			{
				S32 pvIndex = pAftkSceneMeshNode->AddPolygonVertex(polygonVertex);
				if (pvIndex == -1)
				{
					return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to add polygon vertex.");
				}
				pPolygon->AddPolygonVertexIndex(pvIndex);
			}
			
			if (!status) return status;
			count--;
		}
		//Add second polygon vertex
		{
			SLTriangle* pXsiTrianglePositionIndices = &(*pPositionList)[count];
			Aftk::C_PolygonVertex polygonVertex;
			polygonVertex.SetPositionIndex(pXsiTrianglePositionIndices->m_iVtx2);

			if (pNormalList)
			{
				SLTriangle* pXsiTriangleNormalIndices = &(*pNormalList)[count];
				polygonVertex.SetNormalIndex(pXsiTriangleNormalIndices->m_iVtx2);
			}
			for (U32 set = 0; set < (U32)pAftkSceneMeshNode->GetNumTextureCoordinateSets(); set++)
			{
				if ((S32)set >= nUVSets)
				{
					polygonVertex.SetTextureCoordinateIndex(set, -1);
				}
				else
				{
					CSLTriangleList::CSLTriangleArray* pUVList = pTriangleList->GetUVIndices(set);
					if (pUVList)
					{
						SLTriangle* pXsiTriangleUVIndices = &(*pUVList)[count];
						polygonVertex.SetTextureCoordinateIndex(set, pXsiTriangleUVIndices->m_iVtx2);
					}
					else
					{
						polygonVertex.SetTextureCoordinateIndex(set, -1);
					}
				}
			}

			//Polys must ALWAYS have the same number of UV indices as there are UV sets.
			AFTK_ASSERT(polygonVertex.GetNumTextureCoordinateIndices() == pAftkSceneMeshNode->GetNumTextureCoordinateSets());

			{
				S32 pvIndex = pAftkSceneMeshNode->AddPolygonVertex(polygonVertex);
				if (pvIndex == -1)
				{
					return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to add polygon vertex.");
				}
				pPolygon->AddPolygonVertexIndex(pvIndex);
			}
			
			if (!status) return status;
			count--;
		}
		//Add third polygon vertex
		{
			SLTriangle* pXsiTrianglePositionIndices = &(*pPositionList)[count];
			Aftk::C_PolygonVertex polygonVertex;
			polygonVertex.SetPositionIndex(pXsiTrianglePositionIndices->m_iVtx3);

			if (pNormalList)
			{
				SLTriangle* pXsiTriangleNormalIndices = &(*pNormalList)[count];
				polygonVertex.SetNormalIndex(pXsiTriangleNormalIndices->m_iVtx3);
			}
			for (U32 set = 0; set < (U32)pAftkSceneMeshNode->GetNumTextureCoordinateSets(); set++)
			{
				if ((S32)set >= nUVSets)
				{
					polygonVertex.SetTextureCoordinateIndex(set, -1);
				}
				else
				{
					CSLTriangleList::CSLTriangleArray* pUVList = pTriangleList->GetUVIndices(set);
					if (pUVList)
					{
						SLTriangle* pXsiTriangleUVIndices = &(*pUVList)[count];
						polygonVertex.SetTextureCoordinateIndex(set, pXsiTriangleUVIndices->m_iVtx3);
					}
					else
					{
						polygonVertex.SetTextureCoordinateIndex(set, -1);
					}
				}
				//Polys must ALWAYS have the same number of UV indices as there are UV sets.
				AFTK_ASSERT(polygonVertex.GetNumTextureCoordinateIndices() == pAftkSceneMeshNode->GetNumTextureCoordinateSets());
			}

			{
				S32 pvIndex = pAftkSceneMeshNode->AddPolygonVertex(polygonVertex);
				if (pvIndex == -1)
				{
					return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to add polygon vertex.");
				}
				pPolygon->AddPolygonVertexIndex(pvIndex);
			}
			
			if (!status) return status;
			count--;
		}

		count += vertexCount + 1;

		const Aftk::C_SharedResource<Aftk::C_MaterialNode>*	pAftkMaterialResource = 
			pSceneImport->GetAftkMaterialFromXsi(
			pTriangleList->GetMaterial(), status);
		AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;

//		pPolygon->SetMaterialResourcePtr(pAftkMaterialResource);
		S16 localMaterialIndex = 
			pAftkSceneMeshNode->AddPolygonMaterialMapping(pAftkMaterialResource);
		pPolygon->SetMaterialMappingIndex(localMaterialIndex);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneMeshNodeImport::ImportPolygonCluster(
	C_SceneImport* pSceneImport,
	CSLPolygonList* pPolygonList,
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode)
{
	Aftk::C_Status status;

	CSLPolygonList::CSLIntArray* pVertexCount	= pPolygonList->GetPolygonVertexCountList();
	CSLPolygonList::CSLIntArray* pPositionList	= pPolygonList->GetVertexIndices();
	CSLPolygonList::CSLIntArray* pNormalList	= pPolygonList->GetNormalIndices();
	S32 nPolygons								= pPolygonList->GetPolygonCount();
	S32	nUVSets									= pPolygonList->GetUVArrayCount();

	//Add a new cluster and set its name
	S32 polyClusterIndex = pAftkSceneMeshNode->AddPolygonCluster();
	Aftk::C_PolygonCluster* pAftkPolygonCluster = pAftkSceneMeshNode->GetPolygonClusterPtr(polyClusterIndex);
	pAftkPolygonCluster->SetName(pPolygonList->GetName());

	S32 count = 0;
	for (S32 j = 0; j < nPolygons; j++)
	{
		// add polygon
		U32 aftkPolyIndex = pAftkSceneMeshNode->AddPolygon(0, polyClusterIndex, Aftk::C_Polygon());
		Aftk::C_Polygon* pPolygon = &pAftkSceneMeshNode->GetPolygon(0, aftkPolyIndex);

		U32 vertexCount = (*pVertexCount)[j];
        count += vertexCount - 1;

		for (U32 k = 0; k < vertexCount; k++)
		{
			{
				S32 pvIndex = pAftkSceneMeshNode->AddPolygonVertex();
				if (pvIndex == -1)
				{
					return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to add polygon vertex.");
				}
				pPolygon->AddPolygonVertexIndex(pvIndex);
			}
			
			if (!status) return status;

			Aftk::C_PolygonVertex* pPolygonVertex = &pPolygon->GetPolygonVertex(k);

			pPolygonVertex->SetPositionIndex((*pPositionList)[count]);

			if (pNormalList)
			{
				pPolygonVertex->SetNormalIndex((*pNormalList)[count]);
			}

			for (U32 set = 0; set < (U32)pAftkSceneMeshNode->GetNumTextureCoordinateSets(); set++)
			{
				if ((S32)set >= nUVSets)
				{
					pPolygonVertex->SetTextureCoordinateIndex(set, -1);
				}
				else
				{
					CSLPolygonList::CSLIntArray* pUVList = pPolygonList->GetUVIndices(set);
					if (pUVList)
					{
						pPolygonVertex->SetTextureCoordinateIndex(set, (*pUVList)[count]);
					}
					else
					{
						pPolygonVertex->SetTextureCoordinateIndex(set, -1);
					}
				}
			}

			count--;

			//Polys must ALWAYS have the same number of UV indices as there are UV sets.
			AFTK_ASSERT(pPolygonVertex->GetNumTextureCoordinateIndices() == pAftkSceneMeshNode->GetNumTextureCoordinateSets());
		}

		count += vertexCount + 1;

		const Aftk::C_SharedResource<Aftk::C_MaterialNode>*	pAftkMaterialResource = 
			pSceneImport->GetAftkMaterialFromXsi(pPolygonList->GetMaterial(), status);
		AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;

		S16 localMaterialIndex = 
			pAftkSceneMeshNode->AddPolygonMaterialMapping(pAftkMaterialResource);
		pPolygon->SetMaterialMappingIndex(localMaterialIndex);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneMeshNodeImport::DeleteUnusedTextureCoordinates(
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode)
{
	//Build a list of the uv sets used by each material.
	//[material][uv set]
	Ut::UtVector<Ut::DynamicVector<
		Ut::UtVector<Ut::DynamicVector<U32> > > > materialUVSetUsage;
	materialUVSetUsage.Resize(pAftkSceneMeshNode->GetNumPolygonMaterialMappings());
	//For each material...
	for (U16 iMaterial = 0; iMaterial < pAftkSceneMeshNode->GetNumPolygonMaterialMappings(); iMaterial++)
	{
		const Aftk::C_MaterialResource* pMaterialRes = 
			pAftkSceneMeshNode->GetPolygonMaterialMappingPtr(iMaterial);

		const Aftk::C_MaterialNode* pMaterial = &pMaterialRes->GetContent();
		//Find all "tspace_id" occurances
		for (U32 jShaderNode = 0; jShaderNode < pMaterial->GetNumShaderNodes(); jShaderNode++)
		{
			const Aftk::C_ShaderNode* pShaderNode = pMaterial->GetShaderNodePtr(jShaderNode);

			const Aftk::I_Plug* pFound = pShaderNode->GetPlugPtr(std::string("tspace_id"));
			if (pFound)
			{
				//Append this uv set.
				std::string uvSetName;
				pFound->GetValue(uvSetName);
				S32 uvSetIndex = pAftkSceneMeshNode->GetTextureCoordinateSetIndex(uvSetName);
				if (uvSetIndex == -1)
				{
						std::string errorString("Could not find UV set during UV set cleanup. Material [");
						errorString +=
							pMaterial->GetName() + std::string("] uvset [") +
							uvSetName +
							std::string("]. Mesh [") +
							pAftkSceneMeshNode->GetName() +
							std::string("]. Shader C_Node [") +
							pShaderNode->GetName() + 
							std::string("].");

						OutputDebugString(errorString.c_str());
						OutputDebugString("\n");
						for (U32 set = 0; set < pAftkSceneMeshNode->GetNumTextureCoordinateSets(); set++)
						{
							OutputDebugString("    ");
							OutputDebugString(pAftkSceneMeshNode->GetTextureCoordinateSetName(set).c_str());
							OutputDebugString("\n");
						}

					continue;
				}
				materialUVSetUsage[iMaterial].PushBack(uvSetIndex);
			}
		}
	}

	//For each poly...
	for (U32 iCluster = 0; iCluster < pAftkSceneMeshNode->GetNumPolygonClusters(); iCluster++)
	{
		Aftk::C_PolygonCluster* pPolyCluster = pAftkSceneMeshNode->GetPolygonClusterPtr(iCluster);

		for (U32 iLodLevel = 0; iLodLevel < pPolyCluster->GetNumLodLevels(); iLodLevel++)
		{
			for (U32 iPoly = 0; iPoly < pPolyCluster->GetNumPolygonIndices(iLodLevel); iPoly++)
			{
				U32 polyIndex = pPolyCluster->GetPolygonIndex(iLodLevel, iPoly);
				Aftk::C_Polygon* pPoly = &pAftkSceneMeshNode->GetPolygon(iLodLevel, iPoly);
				//Get the materal index
				S16 materialIndex = pPoly->GetMaterialMappingIndex();
				
				if (materialIndex == -1)
				{
					for (U32 jPV = 0; jPV < pPoly->GetNumPolygonVertices(); jPV++)
					{
						Aftk::C_PolygonVertex* pPV = &pPoly->GetPolygonVertex(jPV);
						for (U32 jUVSet = 0; jUVSet < pPV->GetNumTextureCoordinateIndices(); jUVSet++)
						{
							//Mark this texture coodinate as unused.
							pPV->SetTextureCoordinateIndex(jUVSet, -1);
						}
					}
				}
				else
				{
					for (U32 jPV = 0; jPV < pPoly->GetNumPolygonVertices(); jPV++)
					{
						Aftk::C_PolygonVertex* pPV = &pPoly->GetPolygonVertex(jPV);
						//Mark texture coordinate indices unused by material as -1
						for (U32 jUVSet = 0; jUVSet < pPV->GetNumTextureCoordinateIndices(); jUVSet++)
						{
							bool used = false;
							for (U32 kTest = 0; kTest < (U32)materialUVSetUsage[materialIndex].Size(); kTest++)
							{
								if (jUVSet == materialUVSetUsage[materialIndex][kTest])
								{
									used = true;
									break;
								}
							}
							if (!used)
							{
								//Mark this texture coodinate as unused.
								pPV->SetTextureCoordinateIndex(jUVSet, -1);
							}
						}
					}
				}
			}
		}
	}

	//Remove unused texture coordinates.
	U32 numUVsRemoved = pAftkSceneMeshNode->RepairTextureCoordinates();
	return Aftk::C_Status::E_Status_Success;
}

