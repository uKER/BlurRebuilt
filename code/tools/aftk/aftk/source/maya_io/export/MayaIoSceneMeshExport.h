#ifndef MAYA_IO_SCENE_MESH_EXPORT_H
#define MAYA_IO_SCENE_MESH_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Exporter;

	class C_SceneMeshExport : public C_SceneNodeExport
	{
	public:
		///
		///@param pPlugin Pointer to the translator plugin.
		///@param pReader Pointer to the AFTK importer class.
		///@param parent The parent maya object for this node.
		///@param pMayaSceneNodeObject The transform of the node we're creating, output.
		///@param pAftkScene The AFTK scene to import into.
		///@param pAftkSceneNode The AFTK node to import.
		static Aftk::C_Status Export(
			bool create,
			C_Exporter* pExporter,
			MObject& parentTransformNodeObject,
			MObject& mayaNewTransformObject,
			MObject& mayaNewShapeObject,
			const Aftk::C_Scene* pAftkScene,
			const Aftk::C_SceneNode* pAftkSceneNode);

	protected:
		static Aftk::C_Status FindShadingGroup(MObject& material, MObject* pShadingGroup);

		static Aftk::C_Status AssignMaterials(
			C_Exporter* pExporter,
			MDagPath& mayaShapeDagPath,
			const Aftk::C_Scene* pAftkScene,
			const Aftk::C_SceneMeshNode* pAftkSceneMeshNode);

		static MObject m_initialShadingGroupObject;
	};
}

#endif

