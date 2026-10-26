#ifdef _MSC_VER_
#pragma once
#endif

#ifndef NATIVE_SCENE_POINT_LIGHT_NODE_H
#define NATIVE_SCENE_POINT_LIGHT_NODE_H

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeScenePointLightNode : public C_NativeSceneNonAmbientLightNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_ScenePointLightNode* pPointLightNode, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_ScenePointLightNode* pPointLightNode, 
			bool useOuterBlockHeaders);
	};
}
#endif
