#ifndef NEXUS_RENDER_MESH_LIGHTMAP_EFFECT_WRAPPER_H
#define NEXUS_RENDER_MESH_LIGHTMAP_EFFECT_WRAPPER_H

namespace NexusRender
{
	class MeshLightmapEffectConstants;
	//The convention used here is that _effects_ represent different materials.
	//Each effect should be able to render all object types.
	class MeshLightmapEffectWrapper : public MeshTextureEffectWrapper
	{
	public:
		MeshLightmapEffectWrapper();

		Tu::Status Build(D3DShaderManager* pShaderManager);

		Tu::Status SetTechnique(
			const SceneRendererSettings& settings,
			D3DManager::EVertexDecl vertexDecl,
			IRenderMeshNode::EEffectMode mode = 
				IRenderMeshNode::EFFECT_MODE_NOT_APPLICABLE);

		virtual void UpdateNodeConstants(const RenderNode& renderNode);

		virtual void UpdateMaterialConstants(
			SceneRenderer& sceneRenderer,
			const EffectConstants& constants);

		__forceinline EffectWrapper::EEffectType GetType() const;

		__forceinline const char* GetEffectName() const;

	private:
		D3DXHANDLE	m_lightMapUVOffsetsHandle;
		D3DXHANDLE	m_lightmapTextureHandle;
	};

	EffectWrapper::EEffectType MeshLightmapEffectWrapper::GetType() const
	{
		return EffectWrapper::EFFECT_TYPE_MESH_LIGHTMAP;
	}

	const char* MeshLightmapEffectWrapper::GetEffectName() const
	{
		#ifdef LOAD_SHADERS_FROM_FILE
			static const char* pEffectName = "../nexus_render/Shaders/MeshLightmap.fx";
		#else
			static const char* pEffectName = "IDR_MESH_LIGHTMAP_EFFECT";
		#endif
		return pEffectName;
	}

};

#endif
