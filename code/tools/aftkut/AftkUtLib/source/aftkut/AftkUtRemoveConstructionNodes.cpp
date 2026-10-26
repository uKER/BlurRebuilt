#include "stdafx.h"
#include <AftkUt/AftkUtRemoveConstructionNodes.h>

namespace AftkUt
{
namespace RemoveConstructionNodes
{

void Traverse(
	Ut::UtSet<Aftk::C_SceneNode*> & toDelete, 
	Aftk::C_SceneNode* pSceneNode)
{
	AftkUt::Query::C_Flags flags = AftkUt::Query::GetConventionFlags(pSceneNode);
	if (flags.asDynamic || //Is an actor
		flags.isConstructionNode)
		//We'll come and delete this later.
		toDelete.Insert(pSceneNode);

	//Recurse
	for (U32 i = 0; i < pSceneNode->GetNumChildren(); ++i)
		Traverse(toDelete, pSceneNode->GetChildPtr(i));
}

void BuildChildLightList(
	Ut::UtSet<Aftk::C_SceneNode*>& lights,
	Aftk::C_SceneNode* pChild)
{
	if (!pChild) return;

	Aftk::C_SceneNode* pSrc = pChild;
	if (AftkUt::Query::IsInstance(pSrc))
		pSrc = AftkUt::Query::GetTopLevelSourceNodePtr(pSrc);
	
	if (pSrc && pSrc->HasType(Aftk::E_ObjType_SceneLight))
		lights.Insert(pChild);

	for (U32 i = 0; i < pChild->GetNumChildren(); ++i)
		BuildChildLightList(lights, pChild->GetChildPtr(i));
}

void ReparentChildLights(Aftk::C_SceneNode* pTopLevelNode)
{
	Aftk::C_Scene* pScene = pTopLevelNode->GetScenePtr();

	Ut::UtSet<Aftk::C_SceneNode*> lights;
	BuildChildLightList(lights, pTopLevelNode);
	for (Ut::UtSet<Aftk::C_SceneNode*>::Iterator it = lights.Begin(); it != lights.End(); ++it)
	{
		Aftk::C_SceneNode* pLight = *it;

		Aftk::Matrix44 worldMatrix = pLight->GetWorldMatrix();

		//Orphan light.
		pScene->ExtractSceneNodeAndReparentChildren(pLight, true);
		//Reparent light.
		pTopLevelNode->GetParentPtr()->AddChildPtr(pLight);
		
		pLight->SetWorldMatrix(worldMatrix);
	}
}

void RemovePendingChildren(
	Ut::UtSet<Aftk::C_SceneNode*>& nodes,
	Aftk::C_SceneNode* pParent)
{
	for (U32 i = 0; i < pParent->GetNumChildren(); ++i)
	{
		nodes.Erase(pParent->GetChildPtr(i));
	}
}

U32 Do(Aftk::C_SceneNode* pSceneNode)
{
	Aftk::C_Scene* pScene = pSceneNode->GetScenePtr();
	assert(pScene);

	Ut::UtSet<Aftk::C_SceneNode*> toDelete;
	Traverse(toDelete, pSceneNode);

	while(toDelete.Size() != 0)
	{
		Ut::UtSet<Aftk::C_SceneNode*>::Iterator it = toDelete.Begin();
		Aftk::C_SceneNode* pNode = *it;
		if (!pNode) 
		{
			toDelete.Erase(pNode);
			continue;
		}

		ReparentChildLights(pNode);
		RemovePendingChildren(toDelete, pNode);

		pScene->DeleteSceneNodeAndChildren(pNode);
		toDelete.Erase(pNode);
	}

	return (U32)toDelete.Size();
}

}
}
