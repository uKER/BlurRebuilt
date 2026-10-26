#ifndef MAYA_IO_SCENE_LOD_GROUP_IMPORT_H
#define MAYA_IO_SCENE_LOD_GROUP_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_SceneLodGroupImport : public C_SceneNodeImport
	{
	public:
		static Aftk::C_Status Import(	
			C_Importer* pImporter,
			const MDagPath &dagPath,
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode,
			const C_ImportOptions* pOptions);

		static Aftk::C_Status ResolveChildrenSceneWide(	
			C_Importer* pImporter,
			Aftk::C_Scene* pAftkScene,
			const C_ImportOptions* pOptions);
	};
}

#endif
