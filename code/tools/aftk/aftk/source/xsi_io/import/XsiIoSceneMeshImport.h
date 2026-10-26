#ifndef XSI_IO_SCENE_MESH_IMPORT_H
#define XSI_IO_SCENE_MESH_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class C_SceneMeshNodeImport : public C_SceneNodeImport
	{
	public:
		static Aftk::C_Status Import(
			C_SceneImport* pSceneImport,
			CSLModel* pXsiNode, 
			Aftk::C_SceneNode* pAftkSceneNode);

	protected:
		static Aftk::C_Status ImportTriangleCluster(
			C_SceneImport* pSceneImport,
			CSLTriangleList* pTriangleList, 
			Aftk::C_SceneMeshNode* pAftkSceneNode);

		static Aftk::C_Status ImportPolygonCluster(
			C_SceneImport* pSceneImport,
			CSLPolygonList* pPolygonList,
			Aftk::C_SceneMeshNode* pAftkSceneNode);

		static Aftk::C_Status DeleteUnusedTextureCoordinates(
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode);
	};
}

#endif
