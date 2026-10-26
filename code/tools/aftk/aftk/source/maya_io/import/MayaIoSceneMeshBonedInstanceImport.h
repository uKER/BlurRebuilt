#ifndef MAYA_IO_SCENE_MESH_BONED_INSTANCE_IMPORT_H
#define MAYA_IO_SCENE_MESH_BONED_INSTANCE_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Importer;

	class SceneMeshBonedInstanceImport : public C_SceneNodeImport
	{
	public:
		static Aftk::C_Status Import(	
			C_Importer* pImporter,
			const MDagPath &dagPath,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode,
			const C_ImportOptions* pOptions);

	private:
		static Aftk::C_Status ImportMeshMaterials(	
			C_Importer* pImporter,
			const MDagPath &mayaDagPath,
			Aftk::C_Scene* pAftkScene,	
			const C_ImportOptions* pOptions);
	};
}

#endif
