
#ifndef NATIVE_SCENE_JOINT_NODE_H
#define NATIVE_SCENE_JOINT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeSceneJointNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_SceneJointNode* pSceneJointNode, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_SceneJointNode* pSceneJointNode,
			bool useOuterBlockHeaders);
	};
}
#endif
