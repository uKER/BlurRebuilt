#ifndef NATIVE_ANIMATION_CURVE_NODE_H
#define NATIVE_ANIMATION_CURVE_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeAnimationCurveNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_AnimationCurveNode* pCurve,
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_AnimationCurveNode* pCurve,
			bool useOuterBlockHeaders);

		static C_Status DeserialiseLegacy176(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_AnimationCurveNode* pCurve,
			Aftk::E_AnimAttribute& usage,
			bool useOuterBlockHeaders);
	};
}
#endif
