#ifndef XSI_IO_XSI_TO_AFTK_H
#define XSI_IO_XSI_TO_AFTK_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_ImportOptions;

	class C_SceneImport
	{
	public:
		Aftk::C_Status Import(
			CSLScene* pScene, 
			Aftk::C_Scene* pAftkScene,
			const C_ImportOptions* pOptions,
			Aftk::I_Progress* pProgressCallback);

		const Aftk::C_SharedResource<Aftk::C_MaterialNode>* GetAftkMaterialFromXsi(
			CSLBaseMaterial* pXsiMaterial,
			Aftk::C_Status& status);

		Aftk::C_Status AddToXsiAftkSceneNodeMapping(
			CSLModel* pXsiNode,
			Aftk::C_SceneNode* pAftkSceneNode);

		Aftk::C_Status AddToAftkXsiSceneNodeMapping(
			const Aftk::C_SceneNode* pAftkSceneNode,
			CSLModel* pXsiNode);

		CSLScene* GetXsiScenePtr();

		Aftk::C_Scene* GetAftkScenePtr();

		const C_ImportOptions* GetOptionsPtr() const;
		
	private:
		Aftk::C_Status ConvertMaterials(
			CSLScene* pXsiScene,
			Aftk::C_Scene* pAftkScene, 
			Aftk::I_Progress* pProgress);

		Aftk::C_SceneNode* ConvertNode(
			CSLModel* pXsiNode, 
			Aftk::I_Progress* pProgress,
			Aftk::C_Status& status);

		const Aftk::C_ShaderNode* GetShaderDestinationConnection(
			const Aftk::C_ShaderImageNode* pImage);

		U32 CountXsiSceneNodes(CSLModel* pXsiNode);

		Aftk::C_Status ImportSkinClusters(Aftk::C_Scene* pAftkScene);

		Aftk::C_Status ImportSkinCluster(Aftk::C_SceneNode* pAftkSceneNode);

		Aftk::C_Status GetAftkSceneNodeFromXsi(
			CSLModel* pXsiNode,
			Aftk::C_SceneNode** ppAftkSceneNode);

		Aftk::C_Status GetXsiFromAftkSceneNode(
			const Aftk::C_SceneNode* pSceneNode,
			CSLModel** ppXsiNode);

		void CleanUpAfterFailure();

		typedef Ut::UtMap<CSLBaseMaterial*, const Aftk::C_SharedResource<Aftk::C_MaterialNode>*> MaterialMap;
		MaterialMap m_materialMap;

		Ut::UtMap<CSLModel*, Aftk::C_SceneNode*>			m_xsiAftkSceneNodeMapping;
		Ut::UtMap<const Aftk::C_SceneNode*, CSLModel*>	m_aftkXsiSceneNodeMapping;

		CSLScene*				m_pXsiScene;
		Aftk::C_Scene*			m_pAftkScene;
		const C_ImportOptions*	m_pOptions;

		U32 m_numProcessingStagesCompleted;
		U32 m_numProcessingStages;
	};
}

#endif