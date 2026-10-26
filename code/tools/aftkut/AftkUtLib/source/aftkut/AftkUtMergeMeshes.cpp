#include "stdafx.h"

using namespace AftkUt;

Aftk::C_Status C_MergeMeshes::Merge(
	Aftk::C_Scene* pScene,
	const Ut::Vector<Aftk::C_SceneMeshNode*>& toMerge,
	Aftk::I_Progress* pProgress)
{
	if (pProgress)
	{
		pProgress->OnPrintProgress("Merging meshes...");
		pProgress->OnProgress(0);
	}

	if (toMerge.Size() > 0)
	{
		Aftk::C_SceneMeshNode* pTargetMesh = 0;
		Aftk::C_SceneNodeIterator it(pScene);
		Ut::UtVector<Ut::DynamicVector<Aftk::C_SceneMeshNode*> > toDelete;

		for (U32 iSceneNode = 0; iSceneNode < (U32)toMerge.Size(); iSceneNode++)
		{
			Aftk::C_SceneNode* pSceneNode = toMerge[iSceneNode];
			if (pSceneNode == pTargetMesh)
				continue;

			if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				//Ignore instanced nodes.
				if (pSceneNode->GetNumInstancesOfThisNode() != 0)
					continue;

				//Ignore nodes with blind data.
				if (pSceneNode->GetBlindDataResourcePtr() != 0)
					continue;

				//Ignore nodes that have influence clusters.
				if (pSceneNode->GetNumInfluenceClusters() != 0)
					continue;

				if (!pTargetMesh)
				{
					pTargetMesh = (Aftk::C_SceneMeshNode*)pSceneNode;
					LOGDEBUG(g_Log, "Merge target chosen was %s. Unique Id %i.",
						pTargetMesh->GetName().c_str(), pTargetMesh->GetUniqueId());
				}
				else
				{
					Aftk::C_SceneMeshNode* pMeshToAdd = (Aftk::C_SceneMeshNode*)pSceneNode;
					if (pMeshToAdd->GetNumVertexDataChannels() != pTargetMesh->GetNumVertexDataChannels())
						continue;

					bool dataTypesMatch = true;
					for (U32 i = 0; i < pTargetMesh->GetNumVertexDataChannels(); i++)
					{
						const Aftk::C_VertexDataChannelInfo& lhsInfo = pTargetMesh->GetVertexDataChannelInfo(i);
						const Aftk::C_VertexDataChannelInfo& rhsInfo = pMeshToAdd->GetVertexDataChannelInfo(i);

						if (lhsInfo.GetDataType() != rhsInfo.GetDataType())
						{
							dataTypesMatch = false;
							break;
						}
					}
					if (!dataTypesMatch)
					{
						LOGWARN(g_Log, "Skipped merging %s into %s due to vertex colour type mismatch.",
							pMeshToAdd->GetName().c_str(), pTargetMesh->GetName().c_str());
						continue;
					}

					LOGDEBUG(g_Log, "Merging %s into %s.",
						pTargetMesh->GetName().c_str(), pMeshToAdd->GetName().c_str());

					Aftk::Matrix44 destInv;
					UtMaths::Invert4x4(&destInv, pTargetMesh->GetWorldMatrix());
					Aftk::Matrix44 matrixToApply = pMeshToAdd->GetWorldMatrix() * destInv;

					pMeshToAdd->ApplyMatrix(matrixToApply);
					pTargetMesh->AddMesh(pMeshToAdd);
					toDelete.PushBack(pMeshToAdd);
				}
			}
		}
		for (U32 i = 0; i < (U32)toDelete.Size(); i++)
			pScene->DeleteSceneNodeAndReparentChildren(toDelete[i], true);
	}

	if (pProgress)
		pProgress->OnProgress(100.f);

	return Aftk::C_Status::E_Status_Success;
}