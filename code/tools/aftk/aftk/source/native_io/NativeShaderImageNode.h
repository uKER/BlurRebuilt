#ifndef NATIVE_SHADER_IMAGE_NODE_H
#define NATIVE_SHADER_IMAGE_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeShaderImageNode : public C_NativeShaderNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_ShaderImageNode* pShaderImageNode,
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader,
			C_NativeFtk* pNativeFtk, 
			C_ShaderImageNode* pShaderImageNode, 
			bool useOuterBlockHeaders);
	};
}
#endif

