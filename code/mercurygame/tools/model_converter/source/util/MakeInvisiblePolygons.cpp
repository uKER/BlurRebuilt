#include "stdafx.h"
#include "MakeInvisiblePolygons.h"
#include <Polyflags.h>
#include <aftk/Aftk.h>
#include <aftkut/AftkUt.h>
#include <converter_shared/helpers/AftkHelpers.h>


/// Static method to perform tagging of polygons with additional invisible poly flag if 
/// the node is either a portal room, part of the nav mesh or has a collision type that
/// is anything but NULL.
void C_MakeInvisiblePolygons::MakeInvisiblePolygons(Aftk::C_SceneMeshNode* pAftkSceneMeshNode, const Polyflags::C_Config* pPolyConfig, const Objectflags::C_Config* pObjectConfig)
{
	U32 iPolygonCount = pAftkSceneMeshNode->GetNumPolygons(0);
	bool bSetFlags    = false;

	// Figure out if we are a portal room or nav mesh object.
	Objectflags::C_Objectflags objectFlags(*pObjectConfig, pAftkSceneMeshNode);
	if(objectFlags.GetValue(E_Objectflag_PortalRooms) || objectFlags.GetValue(E_Objectflag_NavMesh))
		bSetFlags = true;

	// Check collision type.
	if(AftkHelpers::GetCollisionType(pAftkSceneMeshNode) != AftkHelpers::E_CollisionType_None)
		bSetFlags = true;

	if(bSetFlags)
	{
		for (U32 iCurrentPoly=0; iCurrentPoly<iPolygonCount; ++iCurrentPoly)
		{
			Aftk::C_Polygon *pCurrentPolygon = &pAftkSceneMeshNode->GetPolygon(iCurrentPoly);
			const S16 iBlindDataIndex = pCurrentPolygon->GetBlindDataMappingIndex();

			// Add any additional flags here:
			AddBlindDataAttribute(pCurrentPolygon, "polySet_invisible");
			AddBlindDataAttribute(pCurrentPolygon, "polySet_doNotCastShadows");
			AddBlindDataAttribute(pCurrentPolygon, "polySet_doNotLight");
			AddBlindDataAttribute(pCurrentPolygon, "polySet_doNotPVS");
		}
	}

	pAftkSceneMeshNode->DeleteUnusedPolygonBlindDataMappings();
	return;
}

// --------------------------------------------------------------------------

/// Return a pointer to the blind data resource.
const Aftk::C_BlindDataResource* C_MakeInvisiblePolygons::GetBlindDataResourcePtr(const Aftk::C_Polygon* pPoly)
{
	assert(pPoly);
	const Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();
	const S16 iLocalBdIndex = pPoly->GetBlindDataMappingIndex();

	if(iLocalBdIndex == -1)
		return 0;

	return pMesh->GetPolygonBlindDataMappingPtr(iLocalBdIndex);
}

// --------------------------------------------------------------------------

/// Set the blind data resource pointer.
void C_MakeInvisiblePolygons::SetBlindDataResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_BlindDataResource* pRef)
{
	Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

	if(pRef)
	{
		S16 bdLocalIndex = pMesh->AddPolygonBlindDataMapping(pRef);
		assert(bdLocalIndex != -1);
		pPoly->SetBlindDataMappingIndex(bdLocalIndex);
	}
	else
		pPoly->SetBlindDataMappingIndex(-1);

	return;
}

// --------------------------------------------------------------------------

/// Add an attribute to the polygons blind data.
void C_MakeInvisiblePolygons::AddBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& rkAttributeName)
{
	// Make sure this attribute doesn't already exist in polygon's blind data.
	if(HasBlindDataAttribute(pPolygon, rkAttributeName))
		return;

	Aftk::C_SceneMeshNode* pSceneMeshNode = pPolygon->GetMeshNodePtr();
	Aftk::C_Scene* pAftkScene = pSceneMeshNode->GetScenePtr();
	S16 iBlindDataMappingIndex = pPolygon->GetBlindDataMappingIndex();
	const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(iBlindDataMappingIndex);
	if(pBlindDataResource)
	{
		const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();
		Aftk::C_BlindDataNode newBlindDataNode(aftkBlindDataNode);	// Replicate the blind data node.
	
		// Add additional generic attribute from argument.
		Aftk::C_GenericAttribute newAttribute;
		newAttribute.SetName(rkAttributeName.Chars());
		newBlindDataNode.AddAttribute(newAttribute);

		// Add data to the node and set the index on polygon.
 		const Aftk::C_BlindDataResource* pResource = pAftkScene->AddBlindDataNode(newBlindDataNode);
 		S16 iBlindDataMapping = pSceneMeshNode->AddPolygonBlindDataMapping(pResource);
 		pPolygon->SetBlindDataMappingIndex(iBlindDataMapping);
 	}
	else
	{
		Aftk::C_BlindDataNode newBlindDataNode;	// No duplication on create.
	
		// Add additional generic attribute from argument.
		Aftk::C_GenericAttribute newAttribute;
		newAttribute.SetName(rkAttributeName.Chars());
		newBlindDataNode.AddAttribute(newAttribute);

		// Add data to the node and set the index on polygon.
		const Aftk::C_BlindDataResource* pResource = pAftkScene->AddBlindDataNode(newBlindDataNode);
		S16 iBlindDataMapping = pSceneMeshNode->AddPolygonBlindDataMapping(pResource);
		pPolygon->SetBlindDataMappingIndex(iBlindDataMapping);
	}

	return;
}

// --------------------------------------------------------------------------

/// Return true if a specified polygon has a particular attribute in it's blind data.
bool C_MakeInvisiblePolygons::HasBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& rkAttributeName)
{
	assert(pPolygon);
	const Aftk::C_SceneMeshNode* pSceneMeshNode = pPolygon->GetMeshNodePtr();
	const Aftk::C_Scene* pAftkScene = pSceneMeshNode->GetScenePtr();
	const S16 iBlindDataMappingIndex = pPolygon->GetBlindDataMappingIndex();
	const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(iBlindDataMappingIndex);

	if(pBlindDataResource)
	{
		const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();

		for (U32 i=0; i<aftkBlindDataNode.GetNumAttributes(); ++i) 
		{
			const Aftk::C_GenericAttribute& aftkAttribute = aftkBlindDataNode.GetAttribute(i); 
			if (!stricmp(aftkAttribute.GetName().c_str(), rkAttributeName.Chars()))
				return true;
		}
	}  

	return false;
}
