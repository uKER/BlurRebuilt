#ifndef NATIVE_SCENE_AREA_LIGHT_NODE_H
#define NATIVE_SCENE_AREA_LIGHT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeSceneAreaLightNode : public C_NativeSceneNonAmbientLightNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_SceneAreaLightNode* pAreaLightNode, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_SceneAreaLightNode* pAreaLightNode, 
			bool useOuterBlockHeaders);
	};
}
#endif
