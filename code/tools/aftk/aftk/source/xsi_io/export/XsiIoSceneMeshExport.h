#ifndef	XSI_IO_SCENE_MESH_EXPORT_H
#define XSI_IO_SCENE_MESH_EXPORT_H

#include "XsiIoSceneNodeExport.h"

namespace XsiIo
{

	class C_SceneExport;

	class C_SceneMeshExport : public C_SceneNodeExport
	{
	public:
		static Aftk::C_Status Export(
			C_SceneExport* pExporter,
			CSLModel* pXsiNode,
			const Aftk::C_Scene* pAftkScene,
			const Aftk::C_SceneNode* pAftkSceneNode);

	protected:
		static Aftk::C_Status ExportTriangleCluster(
			C_SceneExport* pExporter,
			CSLMesh* pXsiMesh,
			const Aftk::C_PolygonCluster* pAftkPolygonCluster,
			const Aftk::C_SceneMeshNode* pAftkSceneMeshNode);

		static Aftk::C_Status ExportPolygonCluster(
			C_SceneExport* pExporter,
			CSLMesh* pXsiMesh,
			const Aftk::C_PolygonCluster* pAftkPolygonCluster,
			const Aftk::C_SceneMeshNode* pAftkSceneMeshNode);
	};

}

#endif