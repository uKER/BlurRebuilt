#include "stdafx.h"

#include <fx/fx.h>
#include "world_render/Scene.h"
#include "world_render/ModelInstance.h"
#include "world_render/EnvironmentSampler.h"
#include "world_render/CubemapPoints.h"

namespace WorldRender
{

/// constructor
C_EnvironmentSampler::C_EnvironmentSampler()
: m_pCachedModel(NULL)
, m_bFirstUpdate(true)
, m_pEnvMapTexture(NULL)
{
	m_pEnvMapGPUTexture = FX::GetDefaultGPUTexture();
}

/// destructor
C_EnvironmentSampler::~C_EnvironmentSampler()
{
	// release associated texture
	if (m_pEnvMapTexture)
		WorldRender::C_LocalisedCubemapData::ReleaseGPUTexture(m_pEnvMapTexture);
}

/// initialise this class using a WorldRender::C_Scene
void C_EnvironmentSampler::Initialise(C_Scene *pScene)
{
	m_pScene = pScene;
	m_NearestLightSampleData = pScene->GetLightSamplePointScene().GetDefaultLightSampleData();
}

/// get the current position of the sampler
const Maths::HVec3& C_EnvironmentSampler::GetPosition() const
{
	return m_vPosition;
}

/// change the current position of the sampler
void C_EnvironmentSampler::SetPosition(const Maths::HVec3& position)
{
	const Maths::HF32 fDistSqrd = m_vPosition.DistanceSquared(position);
	const Maths::HF32 fAllowedMovementSqrd = (1.5f * 1.5f);

	if( (fDistSqrd > fAllowedMovementSqrd) || m_bFirstUpdate)
	{
		m_pScene->GetLightSamplePointScene().GetClosestSample( m_NearestLightSampleData, position );
		RenderingData::Target::SharedTextureData *pTex = m_pScene->GetLightSamplePointScene().GetClosestCubemap( position );
		if (pTex != m_pEnvMapTexture)
		{
			if (m_pEnvMapTexture)
				WorldRender::C_LocalisedCubemapData::ReleaseGPUTexture(m_pEnvMapTexture);
			if (pTex)
				m_pEnvMapGPUTexture = WorldRender::C_LocalisedCubemapData::GetGPUTexture(pTex);
			else
				m_pEnvMapGPUTexture = FX::GetDefaultGPUTexture();
			m_pEnvMapTexture = pTex;
		}
		m_vPosition = position;
	}
}

/// update the lerping of the SH sampler
void C_EnvironmentSampler::Update()
{
	if (m_bFirstUpdate)
	{
		m_CurrentLightSampleData = m_NearestLightSampleData;
		m_bFirstUpdate = false;
	}
	else
	{
		//Lerp
		m_CurrentLightSampleData.Lerp( m_NearestLightSampleData, 0.2f );
	}
}

/// set the current data in the sampler to the given model-instance
void C_EnvironmentSampler::ApplyToModelInstance(C_ModelInstance *pModelInstance)
{
	if (!pModelInstance)
		return;
	const C_Model *pModel = pModelInstance->GetRenderModel();
	if (m_pCachedModel != pModel)
	{
		m_pCachedModel = pModel;
		m_hSHNode = pModel->GetParameter("g_SHNode");
		m_hEnvMap = pModel->GetParameter("g_tEnvironment");
	}

	pModelInstance->SetParameter(m_hSHNode, &m_CurrentLightSampleData.m_SampleData[0]);
	pModelInstance->SetParameter(m_hEnvMap, &m_pEnvMapGPUTexture);	
}

}
