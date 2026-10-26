#ifndef NATIVE_BLIND_DATA_NODE_H
#define NATIVE_BLIND_DATA_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeBlindDataNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_BlindDataNode* pBlindDataNode, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader,
			C_NativeFtk* pNativeFtk, 
			C_BlindDataNode* pBlindDataNode,
			bool useOuterBlockHeaders);
	};
}
#endif
