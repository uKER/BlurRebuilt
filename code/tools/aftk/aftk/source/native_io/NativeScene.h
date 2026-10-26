#ifndef NATIVE_SCENE_H
#define NATIVE_SCENE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_NativeFtk;

	class C_NativeScene
	{
	public:
		static C_Status Serialise(
			C_NativeDiskWriter* pDiskWriter, 
			C_NativeFtk* pNativeFtk, 
			const C_Scene* pScene,
			I_Progress* pProgressCallback);

		static C_Status Deserialise(
			C_NativeDiskReader* pDiskReader, 
			C_NativeFtk* pNativeFtk, 
			C_Scene* pScene,
			I_Progress* pProgressCallback);

	private:
		///
		/// Writes a header block which specifies the number of nodes which exist in this scene
		static C_Status	WriteScenePrototype(C_NativeDiskWriter* pDiskWriter, C_NativeFtk* pNativeFtk, const C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	WriteScene(C_NativeDiskWriter* pDiskWriter, C_NativeFtk* pNativeFtk, const C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	WriteMaterials(C_NativeDiskWriter* pDiskWriter, C_NativeFtk* pNativeFtk, const C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	WriteBlindData(C_NativeDiskWriter* pDiskWriter, C_NativeFtk* pNativeFtk, const C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	WriteUnmanagedDgNodes(C_NativeDiskWriter* pDiskWriter, C_NativeFtk* pNativeFtk, const C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	WriteMemoryManagerCues(C_NativeDiskWriter* pDiskWriter, C_NativeFtk* pNativeFtk, const C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	WriteXRefs(C_NativeDiskWriter* pDiskWriter, C_NativeFtk* pNativeFtk, const C_Scene* pScene, I_Progress* pProgressCallback);

		static C_Status	ReadScenePrototype(C_NativeDiskReader* pDiskReader, C_NativeFtk* pNativeFtk, C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	ReadScenePrototype176(C_NativeDiskReader* pDiskReader, C_NativeFtk* pNativeFtk, C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	ReadScene(C_NativeDiskReader* pDiskReader, C_NativeFtk* pNativeFtk, C_Scene* pScene, Matrix44& sceneTransform, I_Progress* pProgressCallback);
		static C_Status	ReadMaterials(C_NativeDiskReader* pDiskReader, C_NativeFtk* pNativeFtk, C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	ReadBlindData(C_NativeDiskReader* pDiskReader, C_NativeFtk* pNativeFtk, C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	ReadUnmanagedDgNodes(C_NativeDiskReader* pDiskReader, C_NativeFtk* pNativeFtk, C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	ReadUnknownUnmanagedDgNode(C_NativeDiskReader* pDiskReader, C_NativeFtk* pNativeFtk, C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	ReadMemoryManagerCues(C_NativeDiskReader* pDiskReader, C_NativeFtk* pNativeFtk, C_Scene* pScene, I_Progress* pProgressCallback);
		static C_Status	ReadXRefs(C_NativeDiskReader* pDiskReader, C_NativeFtk* pNativeFtk, C_Scene* pScene, I_Progress* pProgressCallback);
		static bool ResolveObjectPointers(U32 version, C_NativeFtk* pNativeFtk, C_Scene* pScene);
		static bool ResolveObjectPointer(U32 version, C_NativeFtk* pNativeFtk, S32 id, I_Object* pObject);
		static bool EvaluateUnmanagedPlugs(U32 version, C_NativeFtk* pNativeFtk, C_Scene* pScene);

		static U32 m_numProcessingStagesCompleted;
		static U32 m_numProcessingStages;
	};
}

#endif
