#include "stdafx.h"
#include "MakeNoPVSPolygons.h"
#include <Polyflags.h>
#include <aftk/Aftk.h>
#include <aftkut/AftkUt.h>
#include <converter_shared/helpers/AftkHelpers.h>


/// Set doNotPVS flag on any polygons marked with invisible flag.
void C_MakeNoPVSPolygons::MakeNoPVSPolygons(Aftk::C_SceneMeshNode* pAftkSceneMeshNode, const Objectflags::C_Config* pObjectFlagConfig)
{

	// Set doNotPVS flag on any polygons marked with invisible flag.
	for (U32 iCurrentPoly=0; iCurrentPoly<pAftkSceneMeshNode->GetNumPolygons(0); ++iCurrentPoly)
	{
		Aftk::C_Polygon *pCurrentPolygon = &pAftkSceneMeshNode->GetPolygon(iCurrentPoly);
		if(HasBlindDataAttribute(pCurrentPolygon, "polySet_invisible") &&
			!HasBlindDataAttribute(pCurrentPolygon, "polySet_PVSOccluder"))
			AddBlindDataAttribute(pCurrentPolygon, "polySet_doNotPVS");
	}

	// Set doNotPVS flag on any platform polygons.
	bool isPlatform = IsPlatform(pAftkSceneMeshNode, pObjectFlagConfig);
	if(isPlatform)
	{
		for (U32 iCurrentPoly=0; iCurrentPoly<pAftkSceneMeshNode->GetNumPolygons(0); ++iCurrentPoly)
		{
			Aftk::C_Polygon *pCurrentPolygon = &pAftkSceneMeshNode->GetPolygon(iCurrentPoly);
			if(!HasBlindDataAttribute(pCurrentPolygon, "polySet_doNotPVS"))
				AddBlindDataAttribute(pCurrentPolygon, "polySet_doNotPVS");
		}
	}

	pAftkSceneMeshNode->DeleteUnusedPolygonBlindDataMappings();
	return;
}

// --------------------------------------------------------------------------

/// Return a pointer to the blind data resource.
const Aftk::C_BlindDataResource* C_MakeNoPVSPolygons::GetBlindDataResourcePtr(const Aftk::C_Polygon* pPoly)
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
void C_MakeNoPVSPolygons::SetBlindDataResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_BlindDataResource* pRef)
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
void C_MakeNoPVSPolygons::AddBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& rkAttributeName)
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
bool C_MakeNoPVSPolygons::HasBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& rkAttributeName)
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

bool C_MakeNoPVSPolygons::IsPlatform( Aftk::C_SceneMeshNode* pAftkSceneMeshNode, const Objectflags::C_Config* pObjectFlagConfig )
{
	const Aftk::C_SceneNode*  pCurrAftkSceneNode = pAftkSceneMeshNode;
	while(pCurrAftkSceneNode)
	{
		Objectflags::C_Objectflags objectFlags( *pObjectFlagConfig, pCurrAftkSceneNode );
		if( objectFlags.GetValue( E_Objectflag_Model ) || objectFlags.GetValue( E_Objectflag_Element ))
		{
			return true;
		}

		pCurrAftkSceneNode = pCurrAftkSceneNode->GetParentPtr();
	}

	return false;
}


