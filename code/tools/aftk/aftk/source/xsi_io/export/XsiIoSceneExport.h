#ifndef XSI_IO_SCENE_EXPORT_H
#define XSI_IO_SCENE_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_SceneExport
	{
	public:
		C_SceneExport();
		
		Aftk::C_Status Export(
			const Aftk::C_Scene* pAftkScene,
			CSLScene* pXsiScene,
			Aftk::I_Progress* pProgress);

		F32 GetDotXsiVersion() const;

		void SetDotXsiVersion( F32 newDotXsiVersion );

		const Aftk::C_Scene* GetAftkScene() const;

		CSLScene* GetXsiScene() const;

		Aftk::C_Status GetXsiSceneNodeFromAftk(
			const Aftk::C_SceneNode* pSceneNode,
			CSLModel** ppXsiNode);

		Aftk::C_Status AddToAftkXsiSceneNodeMapping(
			const Aftk::C_SceneNode* pAftkSceneNode,
			CSLModel* pXsiNode);

		Aftk::C_Status GetXsiMaterialFromAftk(
			const Aftk::C_SharedResource<Aftk::C_MaterialNode>* pMaterialResource,
			CSLBaseMaterial** ppXsiMaterial);

		Aftk::C_Status AddToAftkXsiMaterialMapping(
			const Aftk::C_SharedResource<Aftk::C_MaterialNode>* pMaterialResource,
			CSLBaseMaterial* pXsiMaterial);

	private:
		void CleanUpAfterFailure();

		Aftk::C_Status ExportMaterials();

		Aftk::C_Status ExportSceneHierarchyTopLevel();

		Aftk::C_Status ExportSceneHierarchyChildren(
			const Aftk::C_SceneNode* pAftkSceneNode,
			CSLModel* pXsiParentNode);

		Aftk::C_Status ExportSkinningData();

		Aftk::C_Status ExportSkinCluster( const Aftk::C_SceneMeshNode* pAftkSceneNode );

		F32 m_DotXsiVersion;

		const Aftk::C_Scene* m_pAftkScene;

		CSLScene * m_pXsiScene;

		Ut::UtMap<const Aftk::C_SharedResource<Aftk::C_MaterialNode>*, CSLBaseMaterial*> m_aftkXsiMaterialMap;

		Ut::UtMap<const Aftk::C_SceneNode*, CSLModel*>	m_aftkXsiSceneNodeMap;

		//Callbacks
		Aftk::I_Progress* m_pProgressNotification;
		U32 m_nProcessingStages;
		U32 m_nProcessingStagesCompleted;
	};

}

#endif