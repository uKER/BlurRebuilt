#ifndef NATIVE_MATERIAL_NODE_H
#define NATIVE_MATERIAL_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	///This class reads/writes an AFTK material node to/from disk in its native format.
	class C_NativeMaterialNode : public C_NativeNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			S32 mtlIndex,
			const C_MaterialNode* pAftkMaterialNode,
			bool useOuterBlockHeaders);

		static C_Status DeserialiseLegacy(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk,
			C_MaterialNode* pAftkMaterialNode);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader,
			C_NativeFtk* pNativeFtk, 
			C_MaterialNode* pAftkMaterialNode, 
			bool useOuterBlockHeaders);
	};
}

#endif

