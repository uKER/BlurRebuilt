#ifndef NATIVE_POLYGON_H
#define NATIVE_POLYGON_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativePolygon
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_Polygon* pPolygon, 
			const C_SceneMeshNode* pSceneMeshNode,
			bool useOuterBlockHeaders);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader,
			C_NativeFtk* pNativeFtk,
			C_Polygon* pPolygon, 
			C_SceneMeshNode* pSceneMeshNode,
			bool useOuterBlockHeaders);
	};
}
#endif
