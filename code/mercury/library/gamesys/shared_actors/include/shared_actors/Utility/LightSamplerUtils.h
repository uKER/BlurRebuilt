#ifndef LIGHTSAMPLERUTILS_8AE1E110_AD63_49F6_8CD9_E5738887333F_H
#define LIGHTSAMPLERUTILS_8AE1E110_AD63_49F6_8CD9_E5738887333F_H

#ifdef _MSC_VER
#pragma once
#endif


namespace AnimatedModel
{
	class C_AnimatedModelInstance;
}

namespace GameResource
{
	class C_LevelSceneManager;
}

namespace WorldRender
{
	class C_LightSampler;
	class C_EnvironmentSampler;
	class C_ModelInstance;
	struct MaterialParameter;
}


namespace LightSampler
{

WorldRender::C_LightSampler* NewInitialisedLightSampler(GameResource::C_LevelSceneManager* pLevelSceneManager);
WorldRender::C_EnvironmentSampler* NewInitialisedEnvironmentSampler(GameResource::C_LevelSceneManager* pLevelSceneManager);

WorldRender::MaterialParameter GetSHNodeParamsHandle(const WorldRender::C_ModelInstance* pModelInstance);
WorldRender::MaterialParameter GetSHNodeParamsHandle(const AnimatedModel::C_AnimatedModelInstance* pModelInstance);

void SetSHNodeParams(WorldRender::C_ModelInstance* pModelInstance, const WorldRender::C_LightSampler* pLightSampler, const WorldRender::MaterialParameter* pHandle = NULL);
void SetSHNodeParams(AnimatedModel::C_AnimatedModelInstance* pModelInstance, const WorldRender::C_LightSampler* pLightSampler, const WorldRender::MaterialParameter* pHandle = NULL);

} // namespace LightSampler


#endif // LIGHTSAMPLERUTILS_8AE1E110_AD63_49F6_8CD9_E5738887333F_H
