#include "stdafx.h"
#include <core/maths/conversion.h>

using namespace AftkUt;

void C_MakeDoubleSidedPolygons::MakeDoubleSidedPolygons(Aftk::C_SceneMeshNode* pAftkSceneMeshNode, const Util::C_String& attributeName)
{
	U32 nPolys = pAftkSceneMeshNode->GetNumPolygons(0);

	// look for double-sided polygons
	for (U32 i = 0; i < nPolys; i++)
	{
		Aftk::C_Polygon *pPoly = &pAftkSceneMeshNode->GetPolygon(i);

		if(!HasBlindDataAttribute(pPoly, attributeName))
			continue;

		// only process the first tri of a polygon
		if (pPoly->GetBasePolygonIndex() != -1)
			continue;

		int iBaseIdx = pAftkSceneMeshNode->GetNumPolygons(0);
		bool bFirst = true;

		do
		{
			// make a copy of this polygon
			// add the new triangle to the mesh
			U32 newPolyIndex = pAftkSceneMeshNode->AddPolygon(Aftk::C_Polygon());
			pPoly = &pAftkSceneMeshNode->GetPolygon(i);
			Aftk::C_Polygon* pNewPoly = &pAftkSceneMeshNode->GetPolygon(newPolyIndex);

			// copy over the vertices in reverse order
			int nPV = pPoly->GetNumPolygonVertices();
			for (int iPV=0; iPV<nPV; iPV++)
			{
				// copy this polygon vertex
				Aftk::C_PolygonVertex *pPV = &pPoly->GetPolygonVertex(nPV-iPV-1);
				Aftk::C_PolygonVertex newPV = *pPV;

				// create a new vertex normal that points in the opposite direction
				int iNormal = pPV->GetNormalIndex();

				Maths::HVec3 polyNormal, originalNormal, reflectedNormal;
				Maths::Convert(polyNormal, (Aftk::Vector3F32&)pPoly->GetNormal());
				Maths::Convert(originalNormal, (Aftk::Vector3F32&)pAftkSceneMeshNode->GetVertexNormal(iNormal));

				reflectedNormal = originalNormal - (2*(originalNormal | polyNormal)*polyNormal);

				Aftk::Vector3F32 result;
				Maths::Convert(result, reflectedNormal);
				newPV.SetNormalIndex(pAftkSceneMeshNode->AddVertexNormal((Aftk::Normal3&)reflectedNormal));

				S32 iNewVtx = pAftkSceneMeshNode->AddPolygonVertex(newPV);
				pNewPoly->AddPolygonVertexIndex(iNewVtx);
			}

			// set base triangle index
			if (bFirst)
			{
				pNewPoly->SetBasePolygonIndex(-1);
				bFirst = false;
			}
			else
				pNewPoly->SetBasePolygonIndex(iBaseIdx);

			// copy material
			SetMaterialResourcePtr(pNewPoly, GetMaterialResourcePtr(pPoly));                

			// copy polyflags
			SetBlindDataResourcePtr(pNewPoly, GetBlindDataResourcePtr(pPoly));

			// clear the double sided flag from the new and original poly. 			
			RemoveBlindDataAttribute(pNewPoly, attributeName);
			RemoveBlindDataAttribute(pPoly, attributeName);

			i++;
			pPoly = &pAftkSceneMeshNode->GetPolygon(i);
		}

		while (pPoly->GetBasePolygonIndex() != -1);      // also process the other tris that belong to this poly
		i--;
	}

	pAftkSceneMeshNode->DeleteUnusedPolygonBlindDataMappings();
}

const Aftk::C_MaterialResource* C_MakeDoubleSidedPolygons::GetMaterialResourcePtr(const Aftk::C_Polygon* pPoly)
{
	const Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();
	S16 localMaterialIndex = pPoly->GetMaterialMappingIndex();

	if (localMaterialIndex == -1)
	{
		return 0;
	}

	return pMesh->GetPolygonMaterialMappingPtr(localMaterialIndex);
}

const Aftk::C_BlindDataResource* C_MakeDoubleSidedPolygons::GetBlindDataResourcePtr(const Aftk::C_Polygon* pPoly)
{
	const Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();
	S16 localBdIndex = pPoly->GetBlindDataMappingIndex();

	if (localBdIndex == -1)
	{
		return 0;
	}

	return pMesh->GetPolygonBlindDataMappingPtr(localBdIndex);
}

void C_MakeDoubleSidedPolygons::SetMaterialResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_MaterialResource* pRef)
{
	Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

	if (pRef)
	{
		S16 materialLocalIndex = pMesh->AddPolygonMaterialMapping(pRef);
		assert(materialLocalIndex != -1);
		pPoly->SetMaterialMappingIndex(materialLocalIndex);
	}
	else
	{
		pPoly->SetMaterialMappingIndex(-1);
	}
}

void C_MakeDoubleSidedPolygons::SetBlindDataResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_BlindDataResource* pRef)
{
	Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

	if (pRef)
	{
		S16 bdLocalIndex = pMesh->AddPolygonBlindDataMapping(pRef);
		assert(bdLocalIndex != -1);
		pPoly->SetBlindDataMappingIndex(bdLocalIndex);
	}
	else
	{
		pPoly->SetBlindDataMappingIndex(-1);
	}
}

void C_MakeDoubleSidedPolygons::RemoveBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& attributeName)
{
	Aftk::C_SceneMeshNode* pSceneMeshNode = pPolygon->GetMeshNodePtr();
	Aftk::C_Scene* pAftkScene = pSceneMeshNode->GetScenePtr();
	S16 blindDataMappingIndex = pPolygon->GetBlindDataMappingIndex();

	const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(blindDataMappingIndex);
	if(pBlindDataResource != NULL)
	{
		const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();

		Aftk::C_BlindDataNode newBlindDataNode(aftkBlindDataNode);

		for (U32 i = 0; i < newBlindDataNode.GetNumAttributes(); ) 
		{
			const Aftk::C_GenericAttribute& aftkAttribute = newBlindDataNode.GetAttribute( i ); 

			if (!stricmp(aftkAttribute.GetName().c_str(), attributeName.Chars() ))
			{
				newBlindDataNode.DeleteAttribute(i);
			}
			else
			{
				i++;
			}
		}
   
		const Aftk::C_BlindDataResource* pResource = pAftkScene->AddBlindDataNode(newBlindDataNode);
		S16 blindDataMapping = pSceneMeshNode->AddPolygonBlindDataMapping(pResource);
		pPolygon->SetBlindDataMappingIndex(blindDataMapping);
	}  
}

bool C_MakeDoubleSidedPolygons::HasBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& attributeName)
{
	Aftk::C_SceneMeshNode* pSceneMeshNode = pPolygon->GetMeshNodePtr();
	Aftk::C_Scene* pAftkScene = pSceneMeshNode->GetScenePtr();
	S16 blindDataMappingIndex = pPolygon->GetBlindDataMappingIndex();

	const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(blindDataMappingIndex);
	if(pBlindDataResource != NULL)
	{
		const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();

		for (U32 i = 0; i < aftkBlindDataNode.GetNumAttributes(); i++) 
		{
			const Aftk::C_GenericAttribute& aftkAttribute = aftkBlindDataNode.GetAttribute( i ); 

			if (!stricmp(aftkAttribute.GetName().c_str(), attributeName.Chars() ))
			{
				return true;
			}
		}
	}  

	return false;
}
