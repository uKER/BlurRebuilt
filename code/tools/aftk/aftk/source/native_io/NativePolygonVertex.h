#ifndef NATIVE_POLYGON_VERTEX_H
#define NATIVE_POLYGON_VERTEX_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativePolygonVertex
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_PolygonVertex* pPolygonVertex,
			const C_SceneMeshNode* pSceneMeshNode,
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_PolygonVertex* pPolygonVertex, 
			const C_SceneMeshNode* pSceneMeshNode,
			bool useOuterBlockHeaders);
	};
}
#endif
