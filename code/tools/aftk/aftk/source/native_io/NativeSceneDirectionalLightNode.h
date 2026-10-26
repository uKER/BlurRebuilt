#ifndef NATIVE_SCENE_DIRECTIONAL_LIGHT_NODE_H
#define NATIVE_SCENE_DIRECTIONAL_LIGHT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeSceneDirectionalLightNode : public C_NativeSceneNonAmbientLightNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_SceneDirectionalLightNode* pDirectionalLightNode, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_SceneDirectionalLightNode* pDirectionalLightNode, 
			bool useOuterBlockHeaders);
	};
}
#endif
