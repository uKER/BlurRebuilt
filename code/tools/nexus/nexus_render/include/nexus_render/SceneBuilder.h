#ifndef NEXUS_RENDER_SCENE_BUILDER_H
#define NEXUS_RENDER_SCENE_BUILDER_H

namespace NexusRender
{
	class SceneBuilder
	{
	public:
		static Tu::Status Build(
			RenderNode** ppRootRenderNode,
			SceneRenderer* pSceneRenderer,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderInstanceNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderDirectionalLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderPointLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSpotLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderAreaLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSplineNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderMeshNodes,
			Ut::OpenHashMap<
				const void*, 
				RenderNode*, 
				Ut::HashKeyOpPtr >& aftkNodeToRenderNodeMapping,
				Aftk::C_Scene* pAftkScene,
			Nexus::Progress* pProgress);

	private:
		static RenderNode* BuildNode(
			Aftk::C_SceneNode* pAftkThisNode, 
			SceneRenderer* pSceneRenderer,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderInstanceNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderDirectionalLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderPointLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSpotLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderAreaLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSplineNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderMeshNodes,
			Ut::OpenHashMap<
				const void*, 
				RenderNode*, 
				Ut::HashKeyOpPtr >& aftkNodeToRenderNodeMapping,
			Tu::Status& status, 
			Nexus::Progress* pProgress);

		static void BuildSpecificNodeLists(
			Aftk::C_Scene* pAftkScene,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderDirectionalLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderPointLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSpotLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderAreaLightNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderSplineNodes,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderMeshNodes,
			Ut::OpenHashMap<
				const void*, 
				RenderNode*, 
				Ut::HashKeyOpPtr >& aftkNodeToRenderNodeMapping);

		static Tu::Status BuildInstances(
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderNodes,
			Ut::OpenHashMap<
				const void*, 
				RenderNode*, 
				Ut::HashKeyOpPtr >& aftkNodeToRenderNodeMapping,
			Nexus::Progress* pProgress);
	};
}

#endif
