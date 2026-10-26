#ifndef AFTK_RENDER_MESH_REMAPPABLE_NODE_INCLUDE
#define AFTK_RENDER_MESH_REMAPPABLE_NODE_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif


namespace Aftk
{
	class C_Status;
}

namespace AftkRender
{
	class C_RenderMeshRemappableNode : public I_RenderMeshNode
	{
	public:
		C_RenderMeshRemappableNode(
			Aftk::C_SceneNode *pAftkNode,
			Aftk::C_SceneMeshRemappableNode *pAftkMaterialMappingSource,
			C_SceneRenderer *pRenderer);

		virtual ~C_RenderMeshRemappableNode();

		Aftk::C_Status Rebuild();

		const F32* GetLightMapUVOffsets() const;

		virtual E_RenderNodeType GetType() const;

		virtual bool HasType(E_RenderNodeType type) const;

	protected:
		F32 m_LightMapUVOffsets[2];
		Aftk::C_SceneMeshRemappableNode *m_pAftkMaterialMappingSource;
	};
}

#endif