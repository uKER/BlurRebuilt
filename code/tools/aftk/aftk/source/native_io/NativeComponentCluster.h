#ifndef NATIVE_COMPONENT_CLUSTER_H
#define NATIVE_COMPONENT_CLUSTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeComponentCluster
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_ComponentCluster* pComponentCluster, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_ComponentCluster* pComponentCluster, 
			bool useOuterBlockHeaders);
	};
}
#endif
