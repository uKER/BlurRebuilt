#ifndef MAYA_IO_PGR3_GAME_IMPORT_H
#define MAYA_IO_PGR3_GAME_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_PGR3GameImport : public I_GameImport
	{
	public:
		virtual ~C_PGR3GameImport();

		Aftk::C_Status PreProcessScene(
			C_Importer* pImporter,
			const C_ImportOptions* pOptions);

		bool IsReferenceInstance(const MDagPath& path) const;

		bool PreserveChildJointTransforms(
			C_Importer* pImporter,
			const MDagPath& path) const;

		Aftk::C_Status PostProcessScene(
			C_Importer* pImporter,
			const C_ImportOptions* pOptions);

	private:
		Aftk::C_Status PostProcessDagNode(
			C_Importer* pImporter,
			const C_ImportOptions* pOptions,
			const MDagPath& transformDagPath,
			const MSelectionList& list);

		Aftk::C_Status MergeBonedInstanceLodMeshes(
			C_Importer* pImporter,
			const C_ImportOptions* pOptions);

		Aftk::C_Status ImportReferenceInstance(
			C_Importer* pImporter,
			const C_ImportOptions* pOptions,
			const MDagPath& path);

		Aftk::C_Status ImportBonedInstance(
			C_Importer* pImporter,
			const C_ImportOptions* pOptions,
			const MDagPath& path);

		Aftk::C_Status OverrideMaterials(	
			C_Importer* pImporter,
			const C_ImportOptions* pOptions,
			const MDagPath& fnInstanceParent,
			const MDagPath& instancePath,
			Aftk::C_SceneMeshRemappableNode* pRemappedInstance);

		Aftk::C_Status GenerateBonedInstanceLodMesh(
			C_Importer* pImporter, 
			const C_ImportOptions* pOptions,
			const MDagPath& instanceSourceDagPath,
			const MDagPath& instanceDagPath,
			Aftk::C_SceneMeshNode** ppAftkLodGroupMeshNode);

		Aftk::C_Status GenerateBonedInstanceLodGroup(
			C_Importer* pImporter, 
			const C_ImportOptions* pOptions,
			const MDagPath& instanceSourceDagPath,
			const MDagPath& bonedInstanceDagPath,
			Aftk::C_SceneLodGroupNode** ppAftkLodGroupNode);

		Aftk::C_Status FindBones(
			C_Importer* pImporter,
			bool findingInstanceParentBones,
			const MDagPath& bonedInstancePath,
			const MFnDagNode& fnBonedInstance, 
			Aftk::C_SceneNode** pBones);

		Aftk::C_Status ImportBones(
			C_Importer* pImporter,
			Aftk::C_SceneMeshBonedInstanceNode* pAftkBonedInstance,
			const MDagPath& bonedInstancePath,
			const MFnDagNode& fnBonedInstance,
			const Aftk::C_SceneNode* pAftkBonedInstanceParent,
			const MDagPath& bonedInstanceParentPath,
			const MFnDagNode& fnBonedInstanceParent);

		bool IsBonedInstanceSource(const MFnDagNode& fnTransform) const;

		Aftk::C_Status ImportLodLevel(
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode,
			const Aftk::C_SceneMeshBonedInstanceNode* pOriginalBonedInstance,
			Aftk::C_SceneMeshNode* pAftkLodLevelMesh);

		//The mesh and mesh instance data to delete.
		Ut::UtSet< Aftk::C_SceneNode*> m_lodLevelsToDelete;

		Ut::UtSet< Aftk::C_SceneMeshNode* > m_lodGroupsProcessed;

		Ut::UtSet< std::string > m_pathsProcessed;

		//Maps from our original instance parent (an arbitrary lod), to a lod group.
		Ut::UtMap< Aftk::C_SceneMeshNode*, Aftk::C_SceneMeshNode* > m_lodGroupMapping;
	};
}

#endif
