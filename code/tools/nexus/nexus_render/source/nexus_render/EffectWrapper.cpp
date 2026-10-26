#include <nexus_render/stdafx.h>
#include <nexus_render/camera.h>

using namespace NexusRender;

EffectWrapper::EffectWrapper()
{
	m_pEffect = 0;
}

EffectWrapper::~EffectWrapper()
{
}

Tu::Status EffectWrapper::Build(D3DShaderManager* pShaderManager)
{
	return LoadEffect(pShaderManager);
}

Tu::Status EffectWrapper::LoadEffect(D3DShaderManager* pShaderManager)
{
	Tu::Status status;
	m_pEffect = pShaderManager->Load(GetEffectName());
	if (!m_pEffect)
	{
		std::stringstream ss;
		ss << "Failed to load effect [" << GetEffectName() << "]\n";
		return Tu::Status(Tu::Status::FAILURE, ss.str());
	}

	return Tu::Status::SUCCESS;
}

Tu::Status EffectWrapper::Commit()
{
	if (!m_pEffect)
		return Tu::Status(Tu::Status::FAILURE, SRD_E_NO_EFFECT_LOADED);

	//Update skinning information
	if (FAILED(m_pEffect->CommitChanges()))
		return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_COMMIT_EFFECT);

	return Tu::Status::SUCCESS;
}
