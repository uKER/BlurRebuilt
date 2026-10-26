#ifndef MAYA_IO_SCENE_LIGHT_IMPORT_H
#define MAYA_IO_SCENE_LIGHT_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Importer;

	class C_SceneLightImport : public C_SceneNodeImport
	{
	public:
		static Aftk::C_Status Import(
			C_Importer* pImport,
			const MDagPath &dagPath,
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode,
			const C_ImportOptions* pOptions);
	};
}

#endif


