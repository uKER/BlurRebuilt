#ifndef NEXUS_RENDER_MESH_TEXTURE_EFFECT_WRAPPER_H
#define NEXUS_RENDER_MESH_TEXTURE_EFFECT_WRAPPER_H

namespace NexusRender
{
	class MeshTextureEffectConstants;

	//The convention used here is that _effects_ represent different materials.
	//Each effect should be able to render all object types.
	class MeshTextureEffectWrapper : public MeshBasicEffectWrapper
	{
	public:
		MeshTextureEffectWrapper();

		virtual Tu::Status Build(D3DShaderManager* pShaderManager);

		virtual Tu::Status SetTechnique(
			const SceneRendererSettings& settings,
			D3DManager::EVertexDecl vertexDecl,
			IRenderMeshNode::EEffectMode mode = 
				IRenderMeshNode::EFFECT_MODE_NOT_APPLICABLE);

		virtual void UpdateNodeConstants(const RenderNode& renderNode);

		virtual void UpdateMaterialConstants(
			SceneRenderer& sceneRenderer,
			const EffectConstants& constants);

		virtual EEffectType GetType() const;

		virtual const char* GetEffectName() const;

	protected:
		D3DXHANDLE			m_offsetWidthHeightCSHandle;
		D3DXHANDLE			m_clampCSHandle;
		D3DXHANDLE			m_rotatedCSHandle;

		D3DXHANDLE			m_colourTextureHandle;
		D3DXHANDLE			m_colourTextureCSHandle;
	};
};

#endif
