#pragma once

namespace NexusRender
{
	//The convention used here is that _effects_ represent different materials.
	//Each effect should be able to render all object types.
	class D3DShaderManager;
	class EffectConstants;

	class EffectWrapper
	{
	public:
		enum EEffectType
		{
			EFFECT_TYPE_GIZMO,

			EFFECT_TYPE_MESH_BASIC,
			EFFECT_TYPE_MESH_TEXTURE,
			EFFECT_TYPE_MESH_LIGHTMAP,
			EFFECT_TYPE_MESH_SELECTION,

			EFFECT_TYPE_COUNT
		};

		EffectWrapper();

		virtual ~EffectWrapper();

		__forceinline ID3DXEffect* EffectPtr();

		__forceinline const ID3DXEffect* EffectPtr() const;

		virtual Tu::Status Build(D3DShaderManager* pShaderManager);

		Tu::Status Commit();

		__forceinline void Begin(U32& numPasses, U32 flags);

		__forceinline void BeginPass(U32 pass);

		__forceinline void EndPass();

		__forceinline void End();

		virtual Tu::Status SetTechnique(
			const SceneRendererSettings& settings,
			D3DManager::EVertexDecl vertexDecl,
			IRenderMeshNode::EEffectMode mode = 
				IRenderMeshNode::EFFECT_MODE_NOT_APPLICABLE) = 0;

		virtual void UpdateCameraConstants(const Camera& camera) = 0;

		virtual void UpdateNodeConstants(const RenderNode& renderNode) = 0;

		virtual void UpdateMaterialConstants(
			SceneRenderer& sceneRenderer,
			const EffectConstants& constants) = 0;

		virtual EEffectType GetType() const = 0;

		virtual const char* GetEffectName() const = 0;

	protected:
		Tu::Status LoadEffect(D3DShaderManager* pShaderManager);

		ID3DXEffect*	m_pEffect;
	};

	ID3DXEffect* EffectWrapper::EffectPtr()
	{
		return m_pEffect;
	}

	const ID3DXEffect* EffectWrapper::EffectPtr() const
	{
		return m_pEffect;
	}

	void EffectWrapper::Begin(U32& numPasses, U32 flags)
	{
		m_pEffect->Begin(&numPasses, flags);
	}

	void EffectWrapper::BeginPass(U32 pass)
	{
		m_pEffect->BeginPass(pass);
	}

	void EffectWrapper::EndPass()
	{
		m_pEffect->EndPass();
	}

	void EffectWrapper::End()
	{
		m_pEffect->End();
	}
};