#include "stdafx.h"

using namespace AftkUt;


void C_BlindDataHelper::SetBlindDataResourcePtr(
	Aftk::C_Polygon& poly, 
	const Aftk::C_BlindDataResource* pRef)
{
	Aftk::C_SceneMeshNode* pMesh = poly.GetMeshNodePtr();

	if (pRef)
	{
		S16 bdLocalIndex = pMesh->AddPolygonBlindDataMapping(pRef);
		assert(bdLocalIndex != -1);
		poly.SetBlindDataMappingIndex(bdLocalIndex);
	}
	else
	{
		poly.SetBlindDataMappingIndex(-1);
	}
}

const Aftk::C_BlindDataResource* C_BlindDataHelper::GetBlindDataResourcePtr(
	const Aftk::C_Polygon& poly)
{
	if (poly.GetBlindDataMappingIndex() == -1)
		return 0;

	const Aftk::C_SceneMeshNode* pMesh = poly.GetMeshNodePtr();
	return pMesh->GetPolygonBlindDataMappingPtr(poly.GetBlindDataMappingIndex());
}

void C_BlindDataHelper::DeleteBlindDataAttribute(
	Aftk::C_Polygon& poly, 
	const char* attributeName)
{
	Aftk::C_SceneMeshNode* pSceneMeshNode = poly.GetMeshNodePtr();
	Aftk::C_Scene* pAftkScene = pSceneMeshNode->GetScenePtr();
	S16 blindDataMappingIndex = poly.GetBlindDataMappingIndex();

	const Aftk::C_BlindDataResource* pBlindDataResource = 
		pSceneMeshNode->GetPolygonBlindDataMappingPtr(blindDataMappingIndex);
	if(pBlindDataResource)
	{
		const Aftk::C_BlindDataNode& bd = pBlindDataResource->GetContent();

		Aftk::C_BlindDataNode newBlindDataNode(bd);

		for (U32 i = 0; i < newBlindDataNode.GetNumAttributes(); ) 
		{
			const Aftk::C_GenericAttribute& aftkAttribute = newBlindDataNode.GetAttribute( i ); 

			if (!stricmp(aftkAttribute.GetName().c_str(), attributeName ))
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
		poly.SetBlindDataMappingIndex(blindDataMapping);
	}  
}

const Aftk::C_GenericAttribute* C_BlindDataHelper::GetBlindDataAttribute(
	const Aftk::C_Polygon& poly, 
	const char* attributeName)
{
	const Aftk::C_SceneMeshNode* pSceneMeshNode = poly.GetMeshNodePtr();
	const Aftk::C_Scene* pAftkScene = pSceneMeshNode->GetScenePtr();
	S16 blindDataMappingIndex = poly.GetBlindDataMappingIndex();

	const Aftk::C_BlindDataResource* pBlindDataResource = 
		pSceneMeshNode->GetPolygonBlindDataMappingPtr(blindDataMappingIndex);
	if(pBlindDataResource)
	{
		const Aftk::C_BlindDataNode& bd = pBlindDataResource->GetContent();

		for (U32 i = 0; i < bd.GetNumAttributes(); i++) 
		{
			const Aftk::C_GenericAttribute& aftkAttribute = bd.GetAttribute( i ); 

			if (!stricmp(aftkAttribute.GetName().c_str(), attributeName ))
			{
				return &aftkAttribute;
			}
		}
	}  

	return 0;
}

void C_BlindDataHelper::SetBlindDataAttribute(
	Aftk::C_Polygon& poly, 
	const Aftk::C_GenericAttribute& attr)
{
	Aftk::C_SceneMeshNode* pSceneMeshNode = poly.GetMeshNodePtr();
	Aftk::C_Scene* pAftkScene = pSceneMeshNode->GetScenePtr();

	const Aftk::C_BlindDataResource* pRes = GetBlindDataResourcePtr(poly);

	Aftk::C_BlindDataNode newBlindData;

	if (pRes)
	{
		//Start with existing blind data.
		newBlindData = pRes->GetContent();
	}

	bool found = false;
	for (U32 i = 0; i < newBlindData.GetNumAttributes(); i++)
	{
		Aftk::C_GenericAttribute& aftkAttribute = newBlindData.GetAttribute( i ); 
		if (!stricmp(aftkAttribute.GetName().c_str(), attr.GetName().c_str() ))
		{
			aftkAttribute.SetValue(attr);
			found = true;
			break;
		}
	}
	if (!found)
	{
		newBlindData.AddAttribute(attr);
	}

	//Add blind data to scene.
	pRes = pAftkScene->AddBlindDataNode(newBlindData);
	assert(pRes);
	//Add blind data to mesh.
	S16 resIndex = pSceneMeshNode->AddPolygonBlindDataMapping(pRes);
	assert(resIndex != -1);
	//Add blind data to polygon.
	poly.SetBlindDataMappingIndex(resIndex);
}
