#ifndef FBX_IO_ANIMATION_IMPORT_H
#define FBX_IO_ANIMATION_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace FbxIo
{
	class C_SceneImport;

	class AnimationImport
	{
	public:
		static Aftk::C_Status Import(
			C_SceneImport* pSceneImport,
			KFbxNode* pFbxNode, 
			Aftk::C_SceneNode* pAftkSceneNode);
	};
}

#endif
