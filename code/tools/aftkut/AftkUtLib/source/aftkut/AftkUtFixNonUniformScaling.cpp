#include "stdafx.h"

using namespace AftkUt;

Aftk::C_Status C_FixNonUniformScaling::Fix(
	Aftk::C_Scene& aftkScene)
{
	for (Aftk::C_SceneNodeIterator it(&aftkScene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode = it;

		//We're only interested in meshes.
		if (pSceneNode->GetType() != Aftk::E_ObjType_SceneMesh)
			continue;

		Aftk::C_SceneMeshNode* pMeshNode = (Aftk::C_SceneMeshNode*)pSceneNode;

		if (HasNonUniformScaling(*pMeshNode))
		{
			if (pMeshNode->GetNumInstancesOfThisNode() > 0)
			{
				LOGDEBUG(g_Log, "Instanced mesh %s has non uniform scaling\n", pMeshNode->GetPath().c_str());
				continue;
			}
			if (C_NodeFlagParser::IsFlagged(*pMeshNode))
			{
				LOGDEBUG(g_Log, "Mesh %s with actor properties has non uniform scaling\n", pMeshNode->GetPath().c_str());
				continue;
			}

			LOGDEBUG(g_Log, "Found mesh %s with non-uniform scaling. Freezing transform...\n", pMeshNode->GetPath().c_str());

			Aftk::Matrix44 localMatrix = pMeshNode->GetLocalMatrix();
			pMeshNode->ApplyMatrix(localMatrix);

			UtMaths::SetIdentity(&localMatrix);
			pMeshNode->SetLocalMatrix(localMatrix, true);
		}
	}

	return Aftk::C_Status::E_Status_Success;

}


bool C_FixNonUniformScaling::HasNonUniformScaling(
	const Aftk::C_SceneNode& aftkSceneNode)
{
	Aftk::Vector3F32 scale;
	
	if (!aftkSceneNode.GetScale(scale, true))
	{
		LOGERROR(g_Log, "Failed to get scale factor for %s\n", aftkSceneNode.GetPath().c_str());
		return false;
	}

	F32 range =  Maths::Max(scale.x, Maths::Max(scale.y, scale.z)) - 
		Maths::Min(scale.x, Maths::Min(scale.y, scale.z));

	if (range > AFTK_EPSILON)
		return true;
	else
		return false;
}

