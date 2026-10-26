#ifndef NATIVE_SCENE_AIM_CONSTRAINT_NODE_H
#define NATIVE_SCENE_AIM_CONSTRAINT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeSceneAimConstraintNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_SceneAimConstraintNode* pSceneAimConstraintNode);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_SceneAimConstraintNode* pSceneAimConstraintNode);
	};
}
#endif
