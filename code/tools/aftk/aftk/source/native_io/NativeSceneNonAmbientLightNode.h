#ifndef NATIVE_SCENE_NON_AMBIENT_LIGHT_NODE_H
#define NATIVE_SCENE_NON_AMBIENT_LIGHT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeSceneNonAmbientLightNode : public C_NativeSceneLightNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_SceneNonAmbientLightNode* pNonAmbientLightNode, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_SceneNonAmbientLightNode* pNonAmbientLightNode, 
			bool useOuterBlockHeaders);
	};
}
#endif
