#ifndef MAYA_IO_SCENE_NODE_IMPORT_H
#define MAYA_IO_SCENE_NODE_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Importer;

	class C_SceneNodeImport
	{
	public:
		///@param dagPath DAG path to the node to export.
		///@param	AFTK destination scene.
		///@param	pOptions Export options.
		static Aftk::C_Status Import(	
			C_Importer* pImporter,
			const MDagPath &dagPath,
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode,
			bool setBasePose,
			const C_ImportOptions* pOptions);

		static Aftk::C_Status ImportConnectivity(	
			C_Importer* pImporter,
			const MDagPath& transformPath,
			const MDagPath* pShapePath,
			Aftk::C_SceneNode* pAftkSceneNode,
			const C_ImportOptions* pOptions);

		static Aftk::I_Plug* FindPlug(
			Aftk::C_SceneNode* pAftkSceneNode,
			const char* pPlugName);

	private:
		static Aftk::C_Status AddDynamicPlugs(
			C_Importer* pImporter,
			const MFnDagNode& fnDagNode, 
			Aftk::C_SceneNode* pAftkSceneNode);
	};
}

#endif
