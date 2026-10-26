#include "stdafx.h"

#include "../FbxIoUtility.h"
#include "FbxIoSceneNodeImport.h"

using namespace FbxIo;

Aftk::C_Status C_SceneNodeImport::Import(		
	C_SceneImport* pSceneImport,
	KFbxNode* pFbxNode, 
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;
	pAftkSceneNode->SetName(pFbxNode->GetName());
	
	KFbxVector4 t = pFbxNode->GetLocalTFromDefaultTake();
	KFbxVector4 r = pFbxNode->GetLocalRFromDefaultTake();
	KFbxVector4 s = pFbxNode->GetLocalSFromDefaultTake();

	//Set local transformation
	KFbxXMatrix fbxMtxLocal(t, r, s);

	Aftk::Matrix44 aftkMtxLocal;
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
			aftkMtxLocal[i][j] = static_cast<F32>(fbxMtxLocal[i][j]);
	
	pAftkSceneNode->SetLocalMatrix(aftkMtxLocal);
		
	//Set the base pose
	KFbxPose*   pPose = pFbxNode->GetScene()->GetPose(0);
	KFbxXMatrix fbxBasePoseMtx = GetBasePose(pFbxNode, pPose);
	Aftk::Matrix44 aftkBasePoseMtx;
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
			aftkBasePoseMtx[i][j] = static_cast<F32>(fbxBasePoseMtx[i][j]);
	pAftkSceneNode->SetBasePose(aftkBasePoseMtx);
			 
	return Aftk::C_Status::E_Status_Success;
}

KFbxXMatrix C_SceneNodeImport::GetBasePose(KFbxNode* pFbxNode, KFbxPose* pPose)
{
    KFbxXMatrix basePose;
	bool        poseMtxFound = false;

	if (pPose)
	{
		int lNodeIndex = pPose->Find(pFbxNode);
		
		if (lNodeIndex > -1)
		{
			if (pPose->IsBindPose() || 	!pPose->IsLocalMatrix(lNodeIndex))
			{
				basePose = GetPoseMatrix(pPose, lNodeIndex);
			}
			poseMtxFound = true;
		}
	}

	if (!poseMtxFound)
	{
		basePose = pFbxNode->GetGlobalFromDefaultTake();
	}

    return basePose;
}

KFbxXMatrix C_SceneNodeImport::GetPoseMatrix(KFbxPose* pPose, int pNodeIndex)
{
	KFbxXMatrix poseXMtx;
	KFbxMatrix poseMtx = pPose->GetMatrix(pNodeIndex);

	memcpy((double*)poseXMtx, (double*)poseMtx, sizeof(poseMtx.mData));

	return poseXMtx;
}