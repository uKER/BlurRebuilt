#include "SharedActorsPch.h"

#include <shared_actors/utility/LightSamplerUtils.h>

#include <animated_model/AnimatedModelInstance.h>
#include <game_resource/LevelSceneManager.h>
#include <world_render/RenderModelData.h>
#include <world_render/ModelInstance.h>
#include <world_render/MaterialParameter.h>
#include <world_render/EnvironmentSampler.h>


namespace LightSampler
{

/**
@brief allocates a light sampler and initialises it with the scene.
@param pLevelSceneManager scene to use.
@returns NULL if the light sampler could not be initialised.
*/
WorldRender::C_LightSampler* NewInitialisedLightSampler(GameResource::C_LevelSceneManager* pLevelSceneManager)
{
	BIZ_ASSERT(pLevelSceneManager != NULL);

	WorldRender::C_LightSampler* pLightSampler = NULL;

	if (pLevelSceneManager != NULL)
	{
		pLightSampler = new WorldRender::C_LightSampler;
		pLightSampler->Initialise(&pLevelSceneManager->GetWorldRenderScene()->GetLightSamplePointScene());
	}
	
	return pLightSampler;
}

/**
@brief allocates an environment sampler and initialises it with the scene.
@param pLevelSceneManager scene to use.
@returns NULL if the sampler could not be initialised.
*/
WorldRender::C_EnvironmentSampler* NewInitialisedEnvironmentSampler(GameResource::C_LevelSceneManager* pLevelSceneManager)
{
	BIZ_ASSERT(pLevelSceneManager != NULL);

	WorldRender::C_EnvironmentSampler* pEnvSampler = NULL;

	if (pLevelSceneManager != NULL)
	{
		pEnvSampler = new WorldRender::C_EnvironmentSampler;
		pEnvSampler->Initialise(pLevelSceneManager->GetWorldRenderScene());
	}
	
	return pEnvSampler;
}

/**
@param pModelInstance model to get handle for.
@returns material parameter handle for SHNode values on this model.
*/
WorldRender::MaterialParameter GetSHNodeParamsHandle(const WorldRender::C_ModelInstance* pModelInstance)
{
	WorldRender::MaterialParameter handle;

	if (pModelInstance != NULL)
		handle = pModelInstance->GetRenderModel()->GetParameter("g_SHNode");
	else
		handle.index = -1;

	return handle;
}

/**
@param pModelInstance model to get handle for.
@returns material parameter handle for SHNode values on this model.
*/
WorldRender::MaterialParameter GetSHNodeParamsHandle(const AnimatedModel::C_AnimatedModelInstance* pModelInstance)
{
	WorldRender::MaterialParameter handle;

	if (pModelInstance != NULL)
		handle = GetSHNodeParamsHandle(pModelInstance->GetModelInstance());
	else
		handle.index = -1;

	return handle;
}

/**
@param pModelInstance model to set parameters for.
@param pLightSampler sampler to get parameters from.
@param pHandle if non-NULL, cached result of GetSHNodeParamsHandle.
*/
void SetSHNodeParams(WorldRender::C_ModelInstance* pModelInstance, const WorldRender::C_LightSampler* pLightSampler, const WorldRender::MaterialParameter* pHandle)
{
	if ((pModelInstance != NULL) && (pLightSampler != NULL))
	{
		WorldRender::MaterialParameter handle;

		if (pHandle != NULL)
			handle = *pHandle;
		else
			handle = GetSHNodeParamsHandle(pModelInstance);

		if (handle.index != -1)
			pModelInstance->SetParameter(handle, &pLightSampler->GetLightSampleData().m_SampleData[0]);
	}
}

/**
@param pModelInstance model to set parameters for.
@param pLightSampler sampler to get parameters from.
@param pHandle if non-NULL, cached result of GetSHNodeParamsHandle.
*/
void SetSHNodeParams(AnimatedModel::C_AnimatedModelInstance* pModelInstance, const WorldRender::C_LightSampler* pLightSampler, const WorldRender::MaterialParameter* pHandle)
{
	if (pModelInstance != NULL)
		SetSHNodeParams(pModelInstance->GetModelInstance(), pLightSampler, pHandle);
}

} // namespace LightSampler
