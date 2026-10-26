#ifndef NATIVE_INFLUENCE_CLUSTER_NODE_H
#define NATIVE_INFLUENCE_CLUSTER_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeInfluenceClusterNode
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_InfluenceClusterNode* pInfluenceCluster, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_InfluenceClusterNode* pInfluenceCluster, 
			bool useOuterBlockHeaders);

		static bool ResolveUnmanagedDgNodePointers(
			U32 version,
			C_NativeFtk* pNativeFtk, 
			C_InfluenceClusterNode* pInfluenceCluster);

		static bool ResolveUnmanagedDgNodePointers176(
			U32 version,
			C_NativeFtk* pNativeFtk, 
			C_InfluenceClusterNode* pInfluenceCluster);
	};
}
#endif
