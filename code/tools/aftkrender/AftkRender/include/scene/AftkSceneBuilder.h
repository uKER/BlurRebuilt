#ifndef AFTK_RENDER_SCENE_BUILDER_H
#define AFTK_RENDER_SCENE_BUILDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	class C_Scene;
	class C_SceneGraphNode;
	class C_GeometryNode;

	class C_AftkSceneBuilder
	{
	public:
		static bool BuildScene(
			Aftk::C_Scene* pScene,
			C_Scene *pRenderScene,
			Ut::UtVector<Ut::DynamicVector<C_GeometryNode*> >& geometryNodes,
			Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>& nodeMap);


		static C_SceneGraphNode* BuildNode(
			Aftk::C_SceneNode* pSceneNode,
			C_Scene *pRenderScene,
			Ut::UtVector<Ut::DynamicVector<C_GeometryNode*> >& geometryNodes,
			Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>& nodeMap);

	private:
	};
}

#endif

