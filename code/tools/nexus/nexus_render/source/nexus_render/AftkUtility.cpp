#include <nexus_render/stdafx.h>
#include <nexus_render/AftkUtility.h>

using namespace NexusRender;

Aftk::C_SceneNode* AftkUtility::GetIndirectSourcePtr(Aftk::C_SceneNode* pSceneNode)
{
	if (pSceneNode->GetType() == Aftk::E_ObjType_SceneInstance)
	{
		Aftk::C_SceneInstanceNode* pInst = (Aftk::C_SceneInstanceNode*)pSceneNode;
		Aftk::C_SceneNode* pSource = pInst->GetSourceNodePtr();

		if (pSource && pSource->HasType(Aftk::E_ObjType_SceneInstance))
			return GetIndirectSourcePtr(pSource);

        return pSource;
	}

	return pSceneNode;
}

Aftk::I_SceneMeshNode* AftkUtility::GetMeshInterfacePtr(Aftk::C_SceneNode* pSceneNode)
{
	if (pSceneNode->GetType() == Aftk::E_ObjType_SceneInstance)
	{
		Aftk::C_SceneInstanceNode* pInst = (Aftk::C_SceneInstanceNode*)pSceneNode;
		if (!pInst)
			return 0;
		return GetMeshInterfacePtr(pInst->GetSourceNodePtr());
	}
	else if (pSceneNode->HasType(Aftk::E_ObjType_ISceneMesh))
		return (Aftk::I_SceneMeshNode*)pSceneNode;

	return 0;
}

