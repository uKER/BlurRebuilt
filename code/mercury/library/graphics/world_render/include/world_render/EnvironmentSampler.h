#ifndef __ENVIRONMENT_SAMPLER_H__
#define __ENVIRONMENT_SAMPLER_H__

#include "world_render/LightSamplePointScene.h"
#include "world_render/RenderModel.h"

namespace WorldRender
{

class C_Scene;
class C_ModelInstance;
class C_Model;

/// this class is used to retrieve lighting information from the game world and apply it to a model instance
class C_EnvironmentSampler
{
public:
	/// constructor
	C_EnvironmentSampler();
	/// destructor
	~C_EnvironmentSampler();

	/// initialise this class using a WorldRender::C_Scene
	void Initialise(C_Scene *pScene);

	/// get the current position of the sampler
	const Maths::HVec3& GetPosition() const;

	/// change the current position of the sampler
	void SetPosition(const Maths::HVec3& position);

	/// update the lerping of the SH sampler
	void Update();

	/// set the current data in the sampler to the given model-instance
	void ApplyToModelInstance(C_ModelInstance *p);

	/// get the current light sample data
	const C_LightSampleData& GetCurrentLightSampleData() const {return m_CurrentLightSampleData;}

private:
	C_Scene *m_pScene;
	MaterialParameter m_hSHNode, m_hEnvMap;
	const C_Model *				m_pCachedModel;
	RenderingData::Target::SharedTextureData *m_pEnvMapTexture;
	const void					*m_pEnvMapGPUTexture;
	Maths::HVec3				m_vPosition;
	bool						m_bFirstUpdate;
	C_LightSampleData			m_NearestLightSampleData;
	C_LightSamplePointScene*	m_pLightSamplePointScene;
	C_LightSampleData			m_CurrentLightSampleData;
};

}

#endif
