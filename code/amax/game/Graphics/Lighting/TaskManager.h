#ifndef AMAX_GRAPHICS_LIGHTING_TASK_MANAGER_H
#define AMAX_GRAPHICS_LIGHTING_TASK_MANAGER_H

#include "System/Scheduler/JobQueue.h"
#include "Graphics/Lighting/JobQueuePrepass.h"

namespace Prepass
{
namespace JobQueue
{

const U32	c_MaximumPointLights	= 128;
const U32	c_WorkerTasks			= 3;

class C_Context
{
	friend class C_Prepass;
public:

	C_Context();
	~C_Context();

	void AddPointLight(const Display::C_Float3& vPosition, const Display::C_Float3& vColour, const F32 fRadius, const F32 fIntensity);
	void AddView(const Display::C_View* pView, const F32 fFarClip = -1.0f);
	void AddFog(const F32 fDepthFogDensity, const F32 fDepthFogNearDistance, const F32 fHeightFogDensity, const F32 fHeightFogFalloff, const F32 fHeightFogPlane[4]);

	void Reset();

	bool GetUsage();
	void SetUsage(const bool bUsage);

private:

	LightingData::WorkerInput			m_WorkerInput[c_WorkerTasks];

	LightingData::PointLight ALIGN128	m_PointLights[c_MaximumPointLights];

	LightingData::View ALIGN128			m_View;
	LightingData::Fog ALIGN128			m_Fog;

	U32									m_PointLightCount;

	S32 ALIGN128						m_JobIndex;
	U32 ALIGN128						m_Usage;

};

class C_Prepass
{
public:

	C_Prepass();
	~C_Prepass();

	bool Create();
	void Destroy();

	C_Context* CreateContext();

	void InsertFence();
	void Run(C_Context* pContext);

	void SetJobQueuePolicy(Scheduler::PolicyHandle policyHandle);	

	void SetNormalDepthTexture(const Display::C_Texture2D* pNormalDepthTexture)	{ m_pNormalDepthTexture = pNormalDepthTexture; }
	void SetLightingTexture(Display::C_Texture2D* pLightingTexture)				{ m_pLightingTexture = pLightingTexture; }

private:

	Btl::Vector<C_Context, Btl::VectorParameters_Array<8> >	m_Contexts;

	Scheduler::GroupHandle		m_JobQueueWorkerGroup;
	Scheduler::PolicyHandle		m_JobQueuePolicy;

	const Display::C_Texture2D*	m_pNormalDepthTexture;
	Display::C_Texture2D*		m_pLightingTexture;

	U32							m_FenceValue;
};

}; // namespace JobQueue
}; // namespace Prepass

#endif // AMAX_GRAPHICS_LIGHTING_TASK_MANAGER_H
