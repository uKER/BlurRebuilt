#include "stdafx.h"

#include <btl/containers/map.h>

using namespace AftkUt;

void C_PolyFlagLowestLods::PolyFlagLowestLod(Aftk::C_SceneLodGroupNode* pAftkLodGroupNode,
											 const Btl::Vector<Btl::Pair<Util::C_String, Util::C_String> >& polyFlags)
{
	if (!pAftkLodGroupNode->GetNumLodLevels())
		return;

	U32 lowestLod = pAftkLodGroupNode->GetNumLodLevels() - 1;

	if (lowestLod == 0)
		return;

	Aftk::C_SceneNode* pSceneNode = pAftkLodGroupNode->GetLodLevelInfo(lowestLod).GetSceneNodePtr();

	if (!pSceneNode)
		return;

	if (pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
	{
		Aftk::C_SceneMeshNode* pSceneMeshNode = (Aftk::C_SceneMeshNode*)pSceneNode;

		U32 numBlindDataMappings = pSceneMeshNode->GetNumPolygonBlindDataMappings();

		Btl::Map<const Aftk::C_BlindDataResource*, S16> blindDataResourceMap;
		S16 defaultBlindDataResource = CreateBlindDataResource(*pSceneMeshNode, polyFlags);

		if (numBlindDataMappings)
		{
			for (U32 i = 0, count = numBlindDataMappings; i < count; ++i)
			{
				S16 blindDataMappingIndex = CreateBlindDataResource(*pSceneMeshNode, polyFlags, i);
				
				if (blindDataMappingIndex > -1)
					blindDataResourceMap.Insert(pSceneMeshNode->GetPolygonBlindDataMappingPtr(i), blindDataMappingIndex);
			}
		}

		for (U32 i = 0, count = pSceneMeshNode->GetNumPolygons(0); i < count; ++i)
		{
			Aftk::C_Polygon *pPoly = &pSceneMeshNode->GetPolygon(i);

			const Aftk::C_BlindDataResource* pOriginalBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(pPoly->GetBlindDataMappingIndex());

			if (!pOriginalBlindDataResource)
			{
				if (defaultBlindDataResource >= 0)
				{
					pPoly->SetBlindDataMappingIndex(defaultBlindDataResource);
				}
			}
			else
			{
				Btl::Map<const Aftk::C_BlindDataResource*, S16>::ConstIterator iter
					= blindDataResourceMap.Find(pOriginalBlindDataResource);

				if (iter != blindDataResourceMap.End())
				{
					pPoly->SetBlindDataMappingIndex(iter->GetValue());
				}
			}
		}

		pSceneMeshNode->DeleteUnusedPolygonBlindDataMappings();
	}
}

S16 C_PolyFlagLowestLods::CreateBlindDataResource(Aftk::C_SceneMeshNode& sceneMeshNode,
												  const Btl::Vector<Btl::Pair<Util::C_String, Util::C_String> >& polyFlags,
												  const S16 blindDataResourceIndex)
{
	const Aftk::C_BlindDataResource* pPolyFlagsBlindData = NULL;

	if (blindDataResourceIndex >= 0)
	{
		const Aftk::C_BlindDataResource* pBlindDataResource = sceneMeshNode.GetPolygonBlindDataMappingPtr(blindDataResourceIndex);
	
		if (pBlindDataResource)
		{
			const Aftk::C_BlindDataNode& existingBlindDataNode = pBlindDataResource->GetContent();

			Aftk::C_BlindDataNode newBlindDataNode;

			Aftk::C_GenericAttribute attr;

			for (U32 i = 0, count = existingBlindDataNode.GetNumAttributes(); i < count; ++i)
			{
				attr.SetName(existingBlindDataNode.GetAttribute(i).GetName());
				attr.SetValue(existingBlindDataNode.GetAttribute(i));
				newBlindDataNode.AddAttribute(attr);
				attr.Clear();
			}
			
			bool success = false;

			for (U32 i = 0, count = static_cast<U32>(polyFlags.Size()); i < count; ++i)
			{
				bool allowAdd = true;

				for (U32 j = 0, attrCount = existingBlindDataNode.GetNumAttributes(); j < attrCount; ++j)
				{
					if (_stricmp(existingBlindDataNode.GetAttribute(j).GetName().c_str(), polyFlags[i].first.Chars()) == 0
						|| _stricmp(existingBlindDataNode.GetAttribute(j).GetName().c_str(), polyFlags[i].second.Chars()) == 0)
					{
						allowAdd = false;
						break;
					}
				}

				if (allowAdd)
				{
					attr.SetName(polyFlags[i].first.Chars());
					newBlindDataNode.AddAttribute(attr);
					attr.Clear();

					success = true;
				}
			}

			if (success)
				pPolyFlagsBlindData = sceneMeshNode.GetScenePtr()->AddBlindDataNode(newBlindDataNode);
		}
	}
	else
	{
		Aftk::C_BlindDataNode newBlindDataNode;

		Aftk::C_GenericAttribute attr;

		for (U32 i = 0, count = static_cast<U32>(polyFlags.Size()); i < count; ++i)
		{
			attr.SetName(polyFlags[i].first.Chars());
			newBlindDataNode.AddAttribute(attr);
			attr.Clear();
		}

		pPolyFlagsBlindData = sceneMeshNode.GetScenePtr()->AddBlindDataNode(newBlindDataNode);
	}

	if (pPolyFlagsBlindData)
		return sceneMeshNode.AddPolygonBlindDataMapping(pPolyFlagsBlindData);

	return -1;
}
