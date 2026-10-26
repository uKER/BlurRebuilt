#ifndef AFTK_MESH_SCENE_BUILDER_INCLUDE
#define AFTK_MESH_SCENE_BUILDER_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	class C_Scene;
	class C_SceneGraphNode;
	class C_GeometryNode;

	class C_MeshSceneBuilder
	{
	public:
		static bool BuildScene(
			Aftk::C_Scene *pAftkScene,
			C_Scene* pRenderScene,
			Ut::UtVector<Ut::DynamicVector<C_GeometryNode*> >& geometryNodes,
			Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>& nodeMap);


		static C_SceneGraphNode* BuildNode(
			Aftk::C_SceneNode* pSceneNode,
			C_Scene* pRenderScene,
			Ut::UtVector<Ut::DynamicVector<C_GeometryNode*> >& geometryNodes,
			Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>& nodeMap);

	private:
	};
}

#endif