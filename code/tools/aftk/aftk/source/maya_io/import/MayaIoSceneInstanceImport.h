#ifndef MAYA_IO_SCENE_INSTANCE_IMPORT_H
#define MAYA_IO_SCENE_INSTANCE_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Importer;

	class C_SceneInstanceImport : public C_SceneNodeImport
	{
	public:
		///
		///\brief Creates an Aftk::C_SceneMeshNode for a given Maya instance.
		///
		///@param dagPath DAG path to the instance to export.
		///@param	AFTK destination scene.
		///@param	pOptions Export options.
		///@param	The ATK source node targeted by this instance.
		///@return success
		static Aftk::C_Status Import(		
			C_Importer* pImporter,
			const MDagPath &dagPath,
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode,
			const C_ImportOptions* pOptions,
			Aftk::C_SceneNode* pAftkSourceNode);
	};
}

#endif

