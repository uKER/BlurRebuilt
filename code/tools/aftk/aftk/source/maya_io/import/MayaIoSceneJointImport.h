#ifndef MAYA_IO_SCENE_JOINT_IMPORT_H
#define MAYA_IO_SCENE_JOINT_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Importer;

	class C_SceneJointImport : public C_SceneNodeImport
	{
	public:
		///
		///Creates an Aftk::C_SceneMeshNode for a given Maya joint.
		///@return success
		static Aftk::C_Status Import(
			C_Importer* pImporter,
			const MDagPath &dagPath,
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode,
			const C_ImportOptions* pOptions);

		static Aftk::C_Status GetSSC(
			const MDagPath &dagPath,
			bool& ssc);

		static void SetSSC(
			Aftk::C_SceneNode* pAftkSceneNode,
			bool ssc);
	};
}

#endif

