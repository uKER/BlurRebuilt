#include "stdafx.h"
#include <btl/containers/map.h>
#include <btl/utility/string.h>
#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <AftkUt/AftkUtNodeFlagParser.h>

#include <process/PreprocessAftkExecuterLods.h>

U32 GetNumScenes(const C_DataConversionOptions& options)
{
	S32 nScenes(0);

	Sys::C_XmlDom dom;
	const char* xmlFilePath = options.GetExecuterXmlFilePath().Chars();
	bool success = dom.Read(xmlFilePath);
	if(!success)
	{
		LOGERROR(Tools::g_pMyApplication->GetLogger(), " Problem reading '%s'\n", xmlFilePath);
		return 1;
	}

	const Sys::C_XmlDomBaseNode* pRootNode = dom.GetRootNodePtr();
	if(!pRootNode)
		return 1;

	Ut::Vector<const Sys::C_XmlDomBaseNode*> assetNodes;
	Sys::C_XmlDomUtility::XpathEvaluate(pRootNode, "**/container[@shim_tag=\"shim_aftk_scene_type\"]", assetNodes);

	for (int i=0; i<assetNodes.Size(); i++)
	{
		{
			const Sys::C_XmlDomBaseNode* pNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(assetNodes[i], "shim_aftk_dont_export");
			const Sys::C_XmlAttribute* pAttr = ((Sys::C_XmlDomElement*)pNode)->GetAttributes().GetAttributeByName("value");
			if( _stricmp( pAttr->value.Chars(), "true" ) == 0)
				//Skip this node.
				continue;
		}

		Util::C_String fileName;
		{
			const Sys::C_XmlDomBaseNode* pNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(assetNodes[i], "shim_aftk_scene_filename");
			const Sys::C_XmlAttribute* pAttr = ((Sys::C_XmlDomElement*)pNode)->GetAttributes().GetAttributeByName("value");
			fileName = Util::ExtractFilename(pAttr->value);
		}

		LOGDEBUG(Tools::g_pMyApplication->GetLogger(), "PreprocessAftkExecuterLods: Found scene %s.\n", 
			fileName.Chars());
		++nScenes;
	}

	return nScenes;
}

typedef Btl::Map<Btl::String, Btl::Vector<Aftk::C_SceneMeshNode*> > LodGroupCandidates;

void BuildLodGroupCandidates(LodGroupCandidates& lodGroupCandidates, Aftk::C_SceneNode* pNode)
{
	if (pNode->GetType() == Aftk::E_ObjType_SceneMesh)
	{
		AftkUt::C_NodeFlagParser parser(*pNode);
		if (parser.GetFlagGroupName() == "ModelLodLevel")
		{
			Btl::String path = pNode->GetPath().c_str();
			S32 idx = path.RevSearch('_');
			S32 namingConventionLength = strlen("_LOD#");
			if (idx != -1 && idx == path.Length()-namingConventionLength)
			{
				//Truncate lod node paths so we can group them together.
				Btl::String lodSubStr = path(idx, idx+namingConventionLength-1);
				if (!stricmp(lodSubStr.Chars(), "_LOD"))
					path = path(0, path.Length()-namingConventionLength);
			}
			Btl::Vector<Aftk::C_SceneMeshNode*>& lodGroupMembers = lodGroupCandidates[path];
			lodGroupMembers.PushBack((Aftk::C_SceneMeshNode*)pNode);
		}
	}

	for (U32 i = 0; i < pNode->GetNumChildren(); ++i)
	{
		Aftk::C_SceneNode* pChild = pNode->GetChildPtr(i);
		BuildLodGroupCandidates(lodGroupCandidates, pChild);
	}
}

Aftk::C_SceneNode* FindImportRoot(Aftk::C_SceneNode* pSceneNode)
{
	if (!pSceneNode) 
		return 0;

	Aftk::C_SceneNode* pParent = pSceneNode->GetParentPtr();
	if (!pParent)
		return 0;

	while(pParent)
	{
		if (pParent->GetName() == "AftkRoot")
			return pParent;

		pParent = pParent->GetParentPtr();
	}

	return pSceneNode->GetScenePtr()->GetRootSceneNodePtr();
}

bool BuildLodGroups(LodGroupCandidates& lodGroupCandidates)
{
	for (LodGroupCandidates::Iterator it(lodGroupCandidates.Begin()); it != lodGroupCandidates.End(); ++it)
	{
		Btl::Vector<Aftk::C_SceneMeshNode*>& lodGroupMembers = it->GetValue();
		if (lodGroupMembers.Size() < 2)
			continue;

		Aftk::C_SceneNode* pImportRoot = FindImportRoot(lodGroupMembers[0]);
		if (!pImportRoot)
		{
			LOGERROR(Tools::g_pMyApplication->GetLogger(), "PreprocessAftkExecuterLods: Failed to LOD group parent for %s.\n", it->GetKey().Chars());
			continue;
		}

		Aftk::C_SceneLodGroupNode* pLodGroup = new Aftk::C_SceneLodGroupNode(pImportRoot->GetScenePtr());
		pImportRoot->AddChildPtr(pLodGroup);
		pLodGroup->SetName(lodGroupMembers[0]->GetName() + "_LodGroup");

		for (S32 i = 0; i < lodGroupMembers.Size(); ++i)
		{
			Aftk::C_SceneNode* pLod = lodGroupMembers[i];
			pImportRoot->GetScenePtr()->ExtractSceneNodeAndReparentChildren(pLod, true);
			pLodGroup->AddChildPtr(pLod);

			AftkUt::C_NodeFlagParser parser(*pLod);
			F32 lodThreshold = 0;
			if (!parser.GetParameterValue("LODThreshold", lodThreshold))
			{
				LOGERROR(Tools::g_pMyApplication->GetLogger(), "PreprocessAftkExecuterLods: Failed to find LOD group threshold for %s.\n", pLod->GetPath().c_str());
				continue;
			}

			Aftk::C_GroupLodLevelInfo info;
			//Last lod member has distance of 0.
			if (i == lodGroupMembers.Size()-1)
				info.SetThreshold(0);
			else
				info.SetThreshold(lodThreshold);
			info.SetSceneNodePtr(pLod);
			pLodGroup->AddLodLevelInfo(info);
		}

		LOGINFO(Tools::g_pMyApplication->GetLogger(), "PreprocessAftkExecuterLods: Generated LOD group %s with %i levels.\n", pLodGroup->GetName().c_str(), lodGroupMembers.Size());
	}

	return true;
}

bool PreprocessAftkExecuterLods(Aftk::C_Scene& scene, const C_DataConversionOptions& options)
{
	S32 nScenes = GetNumScenes(options);
	if (nScenes < 2)
	{
		LOGDEBUG(Tools::g_pMyApplication->GetLogger(), "PreprocessAftkExecuterLods: Skipping LOD merging. Only one scene provided.\n");
		return true;
	}
	else
		LOGDEBUG(Tools::g_pMyApplication->GetLogger(), "PreprocessAftkExecuterLods: Will attempt LOD merging for %i scenes.\n", nScenes);

	Aftk::C_SceneNode* pRoot = scene.GetRootSceneNodePtr();
	if (!pRoot) return false;

	//Build a list of base meshes to attempt lod group building for.
	LodGroupCandidates lodGroupCandidates;
	BuildLodGroupCandidates(lodGroupCandidates, pRoot);

	//Build the actual lod groups.
	return BuildLodGroups(lodGroupCandidates);
}
