#ifndef NEXUS_RENDER_AFTK_UTILITY_H
#define NEXUS_RENDER_AFTK_UTILITY_H

namespace NexusRender
{
	class AftkUtility
	{
	public:
		static Aftk::C_SceneNode* GetIndirectSourcePtr(Aftk::C_SceneNode* pSceneNode);

		static Aftk::I_SceneMeshNode* GetMeshInterfacePtr(Aftk::C_SceneNode* pSceneNode);
	};
}

#endif
