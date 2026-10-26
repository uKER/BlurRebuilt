#ifndef NEXUS_RENDER_MESH_EFFECT_WRAPPER_H
#define NEXUS_RENDER_MESH_EFFECT_WRAPPER_H

namespace NexusRender
{
	class MeshEffectWrapper : public EffectWrapper
	{
	public:
		MeshEffectWrapper();

		virtual ~MeshEffectWrapper();

		virtual Tu::Status Build(D3DShaderManager* pShaderManager);

		virtual void UpdateCameraConstants(const Camera& camera) {}

		virtual void UpdateNodeConstants(const RenderNode& renderNode);

		virtual void UpdateMaterialConstants(
			SceneRenderer& sceneRenderer,
			const EffectConstants& constants) {}

	protected:
		D3DXHANDLE			m_mWorldViewProjectionHandle;
		D3DXHANDLE			m_mPaletteHandle;
	};
};

#endif
