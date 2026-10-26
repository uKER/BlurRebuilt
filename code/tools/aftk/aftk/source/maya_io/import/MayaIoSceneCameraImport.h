#ifndef MAYA_IO_SCENE_CAMERA_IMPORT_H
#define MAYA_IO_SCENE_CAMERA_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Importer;

	class C_SceneCameraImport : public C_SceneNodeImport
	{
	public:
		static Aftk::C_Status Import(
			C_Importer* pImporter,
			const MDagPath &dagPath,
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode,
			const C_ImportOptions* pOptions);

		static Aftk::I_Plug* FindPlug(
			Aftk::C_SceneCameraNode* pAftkCamera,
			const char* pPlugName);

		static Aftk::C_Status PostProcess(	
			C_Importer* pImporter,
			Aftk::C_Scene* pAftkScene,
			const C_ImportOptions* pOptions);
	};
}

#endif
