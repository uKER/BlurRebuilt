#ifdef _MSC_VER_
#pragma once
#endif

#ifndef NATIVE_SHADER_NODE_H
#define NATIVE_SHADER_NODE_H

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeShaderNode : public C_NativeNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_ShaderNode* pShaderNode,
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader,
			C_NativeFtk* pNativeFtk, 
			C_ShaderNode* pShaderNode, 
			bool useOuterBlockHeaders);
	};
}
#endif

