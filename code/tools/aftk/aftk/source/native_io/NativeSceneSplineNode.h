#ifdef _MSC_VER_
#pragma once
#endif

#ifndef NATIVE_SCENE_SPLINE_NODE_H
#define NATIVE_SCENE_SPLINE_NODE_H

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeSceneSplineNode : public C_NativeSceneNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_SceneSplineNode* pSceneSplineNode, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_SceneSplineNode* pSceneSplineNode,
			bool useOuterBlockHeaders);
	};
}
#endif
