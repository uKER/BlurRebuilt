#ifndef NATIVE_SCENE_LOD_GROUP_NODE_H
#define NATIVE_SCENE_LOD_GROUP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeSceneLodGroupNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_SceneLodGroupNode* pSceneLodGroupNode, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_SceneLodGroupNode* pSceneLodGroupNode,
			bool useOuterBlockHeaders);

		static bool ResolveUnmanagedDgNodePointers(
			U32 version,
			C_NativeFtk* pNativeFtk,
			C_SceneLodGroupNode* pSceneNode);
	};
}
#endif
