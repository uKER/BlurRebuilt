#ifndef MAYA_IO_SCENE_POINT_LIGHT_EXPORT_H
#define MAYA_IO_SCENE_POINT_LIGHT_EXPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Exporter;

	class C_ScenePointLightExport : public C_SceneNonAmbientLightExport
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
	};
}

#endif

