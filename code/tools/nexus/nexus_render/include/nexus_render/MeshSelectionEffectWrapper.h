#ifndef NEXUS_RENDER_MESH_SELECTION_EFFECT_WRAPPER_H
#define NEXUS_RENDER_MESH_SELECTION_EFFECT_WRAPPER_H

namespace NexusRender
{
	//The convention used here is that _effects_ represent different materials.
	//Each effect should be able to render all object types.
	class MeshSelectionEffectWrapper : public MeshEffectWrapper
	{
	public:
		MeshSelectionEffectWrapper();

		Tu::Status Build(D3DShaderManager* pShaderManager);

		Tu::Status SetTechnique(
			const SceneRendererSettings& settings,
			D3DManager::EVertexDecl vertexDecl,
				IRenderMeshNode::EEffectMode mode = 
					IRenderMeshNode::EFFECT_MODE_NOT_APPLICABLE);

		__forceinline EEffectType GetType() const;

		__forceinline const char* GetEffectName() const;

	private:
	};

	EffectWrapper::EEffectType MeshSelectionEffectWrapper::GetType() const
	{
		return EffectWrapper::EFFECT_TYPE_MESH_SELECTION;
	}

	const char* MeshSelectionEffectWrapper::GetEffectName() const
	{
		#ifdef LOAD_SHADERS_FROM_FILE
			static const char* pEffectName = "../nexus_render/Shaders/MeshSelection.fx";
		#else
			static const char* pEffectName = "IDR_MESH_SELECTION_EFFECT";
		#endif
		return pEffectName;
	}

};

#endif
