#include "stdafx.h"

namespace AftkSpeedTree
{
	extern Debug::Logger &g_Log;
}

using namespace AftkSpeedTree;

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
void CleanupActorForLightingTool(Aftk::C_SceneNode* pSceneNode)
{
	bool status;
	AftkUt::C_NodeFlagParser parser(*pSceneNode, &status);
	if (status)
	{
		if (parser.GetFlagGroupName() == "BaseInstance")
		{
			if (parser.DeleteParameter("UseSHLighting"))
			{
				LOGINFO( g_Log, "Deleted UseSHLighting param from tree %s.\n", 
					pSceneNode->GetName().c_str());
			}
			if (parser.DeleteParameter("UseLightmapLighting"))
			{
				LOGINFO( g_Log, "Deleted UseLightmapLighting param from tree %s.\n", 
					pSceneNode->GetName().c_str());
			}
		}
		parser.StoreFlags(*pSceneNode);
	}
}

bool HasSPTData(Aftk::C_SceneNode* pSceneNode)
{
	if (!pSceneNode)
		return false;

	if (!pSceneNode->GetBlindDataResourcePtr())
		return false;

	const Aftk::C_BlindDataNode& blindData = pSceneNode->GetBlindDataResourcePtr()->GetContent();
	for (U32 jAttr=0; jAttr<blindData.GetNumAttributes(); ++jAttr)
	{
		if (blindData.GetAttribute(jAttr).GetName() == "SPTData")
		{
			return true;
		}
	}

	return false;
}