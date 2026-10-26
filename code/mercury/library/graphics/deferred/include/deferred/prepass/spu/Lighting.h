#ifndef DEFERRED_PREPASS_SPU_LIGHTING_H
#define DEFERRED_PREPASS_SPU_LIGHTING_H

#include "System/Scheduler/JobQueue.h"
#include "Deferred/Prepass/SPU/JobQueue.h"

namespace Deferred
{
namespace JobQueue
{

const U32	c_MaximumPointLights	= 128;
const U32	c_MaximumSpotLights		= 32;
const U32	c_WorkerTasks			= 5;

class C_Context
{
	friend class C_Prepass;
public:

	C_Context();
	~C_Context();

	void AddPointLight(const Display::C_Float3& vPosition, const Display::C_Float3& vColour, const F32 fRadius, const F32 fIntensity);
	void AddSpotLight(const Display::C_Matrix& mWorld, const Display::C_Float3& vColour, const F32 fRadius, const F32 fHorizontalFoV, const F32 fVerticalFoV, const U32 iTexture);
	void AddView(const Display::C_View* pView);

	void Reset();

	bool GetUsage();
	void SetUsage(const bool bUsage);

private:

	LightingData::MasterInput			m_MasterInput;
	LightingData::WorkerInput			m_WorkerInput[c_WorkerTasks];

	LightingData::PointLight ALIGN128	m_PointLights[c_MaximumPointLights];
	LightingData::SpotLight ALIGN128	m_SpotLights[c_MaximumSpotLights];
	
	LightingData::View ALIGN128			m_View;

	U32									m_PointLightCount;
	U32									m_SpotLightCount;

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

	void Run(C_Context* pContext);

	void SetJobQueuePolicy(Scheduler::PolicyHandle policyHandle);	

	void SetNormalTexture(const Display::C_Texture2D* pNormalTexture)	{ m_pNormalTexture = pNormalTexture; }
	void SetDepthTexture(const Display::C_Texture2D* pDepthTexture)		{ m_pDepthTexture = pDepthTexture; }
	void SetLightingTexture(Display::C_Texture2D* pLightingTexture)		{ m_pLightingTexture = pLightingTexture; }

private:

	Btl::Vector<C_Context, Btl::VectorParameters_Array<8> >	m_Contexts;

	Scheduler::GroupHandle		m_JobQueueMasterGroup;
	Scheduler::GroupHandle		m_JobQueueWorkerGroup;
	Scheduler::PolicyHandle		m_JobQueuePolicy;

	const Display::C_Texture2D*	m_pNormalTexture;
	const Display::C_Texture2D*	m_pDepthTexture;
	Display::C_Texture2D*		m_pLightingTexture;
	Scheduler::Label			m_Label;

};

}; // namespace JobQueue
}; // namespace Deferred

#endif // DEFERRED_PREPASS_SPU_LIGHTING_H
