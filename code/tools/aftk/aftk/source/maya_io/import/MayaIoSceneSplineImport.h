#ifndef MAYA_IO_SCENE_SPLINE_IMPORT_H
#define MAYA_IO_SCENE_SPLINE_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Importer;

	class C_SceneSplineImport : public C_SceneNodeImport
	{
	public:
		///
		///Creates an Aftk::C_SceneMeshNode for a given Maya spline.
		///@param dagPath DAG path to the node to export.
		///@param	AFTK destination scene.
		///@param	pOptions Export options.
		///@return success.
		static Aftk::C_Status Import(		
			C_Importer* pImporter,
			const MDagPath &dagPath,
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode,
			const C_ImportOptions* pOptions);
	};
}

#endif

