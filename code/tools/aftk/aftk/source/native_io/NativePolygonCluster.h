#ifndef NATIVE_POLYGON_CLUSTER_H
#define NATIVE_POLYGON_CLUSTER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativePolygonCluster : public C_NativeComponentCluster
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_PolygonCluster* pPolygonCluster, 
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_PolygonCluster* pPolygonCluster, 
			C_SceneMeshNode* pSceneMeshNode,
			bool useOuterBlockHeaders);
	};
}
#endif
