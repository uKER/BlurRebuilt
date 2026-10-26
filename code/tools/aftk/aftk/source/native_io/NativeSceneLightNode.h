#ifndef NATIVE_SCENE_LIGHT_NODE_H
#define NATIVE_SCENE_LIGHT_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeSceneLightNode : public C_NativeSceneNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_SceneLightNode* pLightNode, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_SceneLightNode* pLightNode, 
			bool useOuterBlockHeaders);
	};
}

#endif
