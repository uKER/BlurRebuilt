#ifndef NEXUS_RENDER_MESH_BASIC_EFFECT_WRAPPER_H
#define NEXUS_RENDER_MESH_BASIC_EFFECT_WRAPPER_H

namespace NexusRender
{
	//The convention used here is that _effects_ represent different materials.
	//Each effect should be able to render all object types.
	class MeshBasicEffectWrapper : public MeshEffectWrapper
	{
	public:
		MeshBasicEffectWrapper();

		virtual ~MeshBasicEffectWrapper();

		virtual Tu::Status Build(D3DShaderManager* pShaderManager);

		virtual Tu::Status SetTechnique(
			const SceneRendererSettings& settings,
			D3DManager::EVertexDecl vertexDecl,
			IRenderMeshNode::EEffectMode mode = 
				IRenderMeshNode::EFFECT_MODE_NOT_APPLICABLE);

		virtual void UpdateCameraConstants(const Camera& camera);

		virtual void UpdateNodeConstants(const RenderNode& renderNode);

		virtual void UpdateMaterialConstants(
			SceneRenderer& sceneRenderer,
			const EffectConstants& constants) {}

		virtual EEffectType GetType() const;

		virtual const char* GetEffectName() const;

	protected:
		Tu::Status SetTechniqueNormal(
			const SceneRendererSettings& settings,
			D3DManager::EVertexDecl vertexDecl,
			IRenderMeshNode::EEffectMode mode);

		Tu::Status SetTechniqueCropSheet(
			const SceneRendererSettings& settings,
			D3DManager::EVertexDecl vertexDecl,
			IRenderMeshNode::EEffectMode mode);

		D3DXHANDLE m_mViewHandle;
		D3DXHANDLE m_mPaletteTransposedInverseHandle;
		D3DXHANDLE m_mWorldTransposedInverseHandle;
	};
};

#endif
