#ifndef NEXUS_RENDER_GIZMO_EFFECT_WRAPPER_H
#define NEXUS_RENDER_GIZMO_EFFECT_WRAPPER_H

namespace NexusRender
{
	//The convention used here is that _effects_ represent different materials.
	//Each effect should be able to render all object types.
	class GizmoEffectWrapper : public EffectWrapper
	{
	public:
		GizmoEffectWrapper();

		Tu::Status Build(D3DShaderManager* pShaderManager);

		Tu::Status SetTechnique(
			const SceneRendererSettings& settings,
			D3DManager::EVertexDecl vertexDecl,
			IRenderMeshNode::EEffectMode mode = 
				IRenderMeshNode::EFFECT_MODE_NOT_APPLICABLE);

		void UpdateCameraConstants(const Camera& camera);

		void UpdateNodeConstants(const RenderNode& renderNode);

		void UpdateMaterialConstants(
			SceneRenderer& sceneRenderer,
			const EffectConstants& constants);

		__forceinline EffectWrapper::EEffectType GetType() const;

		__forceinline const char* GetEffectName() const;

	private:
		D3DXHANDLE	m_mViewProjectionHandle;
		D3DXHANDLE	m_mWorldHandle;
	};

	EffectWrapper::EEffectType GizmoEffectWrapper::GetType() const
	{
		return EffectWrapper::EFFECT_TYPE_GIZMO;
	}

	const char* GizmoEffectWrapper::GetEffectName() const
	{
		#ifdef LOAD_SHADERS_FROM_FILE
			static const char* pEffectName = "../nexus_render/Shaders/Gizmo.fx";
		#else
			static const char* pEffectName = "IDR_GIZMO_EFFECT";
		#endif
		return pEffectName;
	}
};

#endif
