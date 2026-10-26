#ifndef MAYA_IO_EXPORT_H
#define MAYA_IO_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Exporter
	{
	public:
		///
		///\brief Constructor.
		///
		C_Exporter();
		
		///
		///@param file A maya file object.
		///@param options A maya options string.
		///@param mode File access mode.
		///@param pluginName The name of this plugin, as provided by Maya.
		Aftk::C_Status Export(
			const Aftk::C_Scene* pAftkScene,
			Aftk::I_Progress* pProgress);

		MObject GetMayaMaterial(
			const Aftk::C_SharedResource<Aftk::C_MaterialNode>* pAftkMaterialResource) const;

		MObject GetMayaMaterial(U32 instance) const;

		MObject GetMayaMaterialShadingGroup(U32 instance) const;

		Aftk::C_Status AddToDefaultShadingGroup(MDagPath& dagPath) const;

		C_AftkMayaMapping* AftkMayaMappingPtr();

		const C_AftkMayaMapping* AftkMayaMappingPtr() const;

		void Clear();

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		void CleanUpAfterFailure();

		Aftk::C_Status ExportMaterials();

		Aftk::C_Status ExportSceneHierarchyTopLevel();

		Aftk::C_Status ExportSceneHierarchyChildren(
			const Aftk::C_SceneNode* pAftkSceneNode, 
			MObject& mayaParentObject);

		Aftk::C_Status ExportSkinClusters();

		Aftk::C_Status ExportSkinCluster(
			MDagPath& mayaDagPath, 
			const Aftk::I_SceneMeshNode* pAftkMeshNode);

		Aftk::C_Status ExportBindPoses();

		Aftk::C_Status ExportBindPose(
			MDagPath& mayaDagPath,
			const Aftk::C_SceneNode* pAftkSceneNode);

		Aftk::C_Status ExportDg();

		Aftk::C_Status AssignToShadingGroup(
			const MObject& shadingGroup,
			MDagPath& dagPath) const;

		Aftk::C_Status ResolveInstances();

		Aftk::C_Status CreateInfluencesMap();

		bool IsInfluencerNode(
			const Aftk::C_SceneNode *pAftkSceneNode);

		const Aftk::C_Scene* m_pAftkScene;

		C_AftkMayaMapping				m_aftkMayaMapping;

		Ut::UtMap<const Aftk::C_SharedResource<Aftk::C_MaterialNode>*, MObject>	
			m_aftkMayaMaterialMappingMap;
		Ut::UtVector< Ut::DynamicVector<MObject> >						
			m_aftkMayaMaterialMappingArray;
		Ut::UtVector< Ut::DynamicVector<MObject> >						
			m_aftkMayaMaterialShadingGroupMappingArray;

		//maps nodes to other nodes that it influences
		Ut::MultiMap<const Aftk::C_SceneNode*, const Aftk::C_SceneNode*>
			m_aftkSceneNodeInfluenceMap;

		//Callbacks
		Aftk::I_Progress* m_pProgressNotification;
	#endif
	};
}

#endif
