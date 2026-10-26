#include <nexus_render/stdafx.h>

using namespace NexusRender;

#include <nexus_render/stdafx.h>
#include <nexus_render/camera.h>

using namespace NexusRender;

MeshSelectionEffectWrapper::MeshSelectionEffectWrapper() : MeshEffectWrapper()
{
}

Tu::Status MeshSelectionEffectWrapper::Build(D3DShaderManager* pShaderManager)
{
	return __super::Build(pShaderManager);
}

Tu::Status MeshSelectionEffectWrapper::SetTechnique(
	const SceneRendererSettings& settings, 
	D3DManager::EVertexDecl decl,
	IRenderMeshNode::EEffectMode mode)
{
	switch(mode)
	{
		case IRenderMeshNode::EFFECT_MODE_BONED_INSTANCE:
		{
			if (FAILED(m_pEffect->SetTechnique("SelectionBonedInstance")))
				return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TECHNIQUE);
			return Tu::Status::SUCCESS;
		}

		case IRenderMeshNode::EFFECT_MODE_SKINNED:
		{
			if (FAILED(m_pEffect->SetTechnique("SelectionSkinned")))
				return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TECHNIQUE);
			return Tu::Status::SUCCESS;
		}
		break;

		case IRenderMeshNode::EFFECT_MODE_STATIC:
		{
			if (FAILED(m_pEffect->SetTechnique("SelectionStatic")))
				return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TECHNIQUE);
			return Tu::Status::SUCCESS;
		}
		break;
	}

	return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_SET_TECHNIQUE);
}

