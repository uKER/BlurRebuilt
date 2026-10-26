#ifndef NATIVE_WEIGHT_ENVELOPE_H
#define NATIVE_WEIGHT_ENVELOPE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeWeightEnvelope
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_WeightEnvelope* pWeightEnvelope, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_WeightEnvelope* pWeightEnvelope, 
			bool useOuterBlockHeaders);

		static bool ResolveUnmanagedDgNodePointers(
			U32 version,
			C_NativeFtk* pNativeFtk,
			C_WeightEnvelope* pWeightEnvelope);
	};
}
#endif
