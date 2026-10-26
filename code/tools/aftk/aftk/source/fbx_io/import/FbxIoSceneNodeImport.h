#ifndef FBX_IO_SCENE_NODE_IMPORT_H
#define FBX_IO_SCENE_NODE_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "FbxIoSceneImport.h"

namespace FbxIo
{
	class C_Main;

	class C_SceneNodeImport
	{
	public:
		static Aftk::C_Status Import(
			C_SceneImport* pSceneImport,
			KFbxNode* pFbxNode, 
			Aftk::C_SceneNode* pAftkSceneNode);
	private:
		static KFbxXMatrix GetBasePose(KFbxNode* pFbxNode, KFbxPose* pPose);
		static KFbxXMatrix GetPoseMatrix(KFbxPose* pPose, int pNodeIndex);
	};
}

#endif
