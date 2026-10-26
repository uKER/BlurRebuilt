#include "stdafx.h"
#include "Graphics/Lighting/TaskManager.h"
#include "Graphics/Lighting/JobQueuePrepass.h"
#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

namespace Prepass
{
namespace JobQueue
{

const U32 c_FBTileWidth		= 64;
const U32 c_FBTileHeight	= 64;

C_Context::C_Context()
	: m_PointLightCount(0)
	, m_JobIndex(0)
	, m_Usage(0)
{
}

C_Context::~C_Context()
{
}

void C_Context::AddPointLight(const Display::C_Float3& vPosition, const Display::C_Float3& vColour, const F32 fRadius, const F32 fIntensity)
{
	if(m_PointLightCount < c_MaximumPointLights)
	{
		m_PointLights[m_PointLightCount].fX			= vPosition.x;
		m_PointLights[m_PointLightCount].fY			= vPosition.y;
		m_PointLights[m_PointLightCount].fZ			= vPosition.z;
		m_PointLights[m_PointLightCount].fIntensity	= fIntensity;
		m_PointLights[m_PointLightCount].fR			= vColour.x;
		m_PointLights[m_PointLightCount].fG			= vColour.y;
		m_PointLights[m_PointLightCount].fB			= vColour.z;
		m_PointLights[m_PointLightCount].fRadius	= fRadius;
		m_PointLights[m_PointLightCount].fInvRadiusSq[0] = 1.0f / (fRadius * fRadius);
		m_PointLights[m_PointLightCount].fInvRadiusSq[1] = m_PointLights[m_PointLightCount].fInvRadiusSq[0];
		m_PointLights[m_PointLightCount].fInvRadiusSq[2] = m_PointLights[m_PointLightCount].fInvRadiusSq[0];
		m_PointLights[m_PointLightCount].fInvRadiusSq[3] = m_PointLights[m_PointLightCount].fInvRadiusSq[0];
		++m_PointLightCount;
	}
}

void C_Context::AddView(const Display::C_View* pView, const F32 fFarClip)
{
	BIZ_ASSERT(pView);

	Maths::Mat44 mViewProj = Maths::Load<Maths::Mat44>((MathsStorage::MatrixRow44*)&pView->GetViewSettings3D().GetViewProjMatrix());
	Maths::Mat44 mView = Maths::Load<Maths::Mat44>((MathsStorage::MatrixRow44*)&pView->GetViewSettings3D().GetViewMatrix());
	Maths::Mat44 mProj = Maths::Load<Maths::Mat44>((MathsStorage::MatrixRow44*)&pView->GetViewSettings3D().GetProjMatrix());

	Maths::Store(mViewProj, (MathsStorage::MatrixRow44*)m_View.fViewProjMatrix);
	Maths::Store(mView, (MathsStorage::MatrixRow44*)m_View.fViewMatrix);
	Maths::Store(mProj, (MathsStorage::MatrixRow44*)m_View.fProjMatrix);

	Maths::Store(Maths::Inverse(mViewProj), (MathsStorage::MatrixRow44*)m_View.fViewProjInverseMatrix);
	Maths::Store(Maths::Inverse(mView), (MathsStorage::MatrixRow44*)m_View.fViewInverseMatrix);
	Maths::Store(Maths::Inverse(mProj), (MathsStorage::MatrixRow44*)m_View.fProjInverseMatrix);
	
	m_View.fNearClip	= pView->GetViewSettings3D().GetNearClip();
	m_View.fFarClip		= (fFarClip < 0.0f) ? pView->GetViewSettings3D().GetFarClip() : fFarClip;
}

void C_Context::AddFog(const F32 fDepthFogDensity, const F32 fDepthFogNearDistance, const F32 fHeightFogDensity, const F32 fHeightFogFalloff, const F32 fHeightFogPlane[4])
{
	m_Fog.fDepthFogDensity		= fDepthFogDensity;
	m_Fog.fDepthFogNearDistance	= fDepthFogNearDistance;
	m_Fog.fHeightFogDensity		= fHeightFogDensity;
	m_Fog.fHeightFogFalloff		= fHeightFogFalloff;
	m_Fog.fHeightFogPlane[0]	= fHeightFogPlane[0];
	m_Fog.fHeightFogPlane[1]	= fHeightFogPlane[1];
	m_Fog.fHeightFogPlane[2]	= fHeightFogPlane[2];
	m_Fog.fHeightFogPlane[3]	= fHeightFogPlane[3];
}

void C_Context::Reset()
{
	for(S32 i = 0; i < c_MaximumPointLights; ++i)
	{
		m_PointLights[i].fX			= 0.0f;
		m_PointLights[i].fY			= 0.0f;
		m_PointLights[i].fZ			= 0.0f;
		m_PointLights[i].fIntensity	= 0.0f;
		m_PointLights[i].fR			= 0.0f;
		m_PointLights[i].fG			= 0.0f;
		m_PointLights[i].fB			= 0.0f;
		m_PointLights[i].fRadius	= 0.0f;
		m_PointLights[i].fInvRadiusSq[0] = 0.0f;
		m_PointLights[i].fInvRadiusSq[1] = 0.0f;
		m_PointLights[i].fInvRadiusSq[2] = 0.0f;
		m_PointLights[i].fInvRadiusSq[3] = 0.0f;

	}
	m_PointLightCount = 0;
	m_JobIndex = 0;
}

bool C_Context::GetUsage()
{
	return (MT::AtomicGet(&m_Usage) != 0);
}

void C_Context::SetUsage(const bool bUsage)
{
	MT::AtomicSet(&m_Usage, bUsage ? c_WorkerTasks : 0);
}

C_Prepass::C_Prepass() : m_JobQueueWorkerGroup(Scheduler::JQ_GROUP_NONE)
{
}

C_Prepass::~C_Prepass()
{
}

bool C_Prepass::Create()
{
	BIZ_ASSERT(m_JobQueueWorkerGroup == Scheduler::JQ_GROUP_NONE);

	m_JobQueueWorkerGroup = Scheduler::CreateGroup();

#if PLATFORM_PS3
	Scheduler::AddGroupDependency(m_JobQueueWorkerGroup, (U32*)Display::Device()->GetWriteFenceAddress());
#endif

	return true;
}

void C_Prepass::Destroy()
{
	Scheduler::DestroyGroup(m_JobQueueWorkerGroup);

	m_JobQueueWorkerGroup = Scheduler::JQ_GROUP_NONE;
}

C_Context* C_Prepass::CreateContext()
{
	// allocate a context that's flagged as free
	for(S32 i = 0; i < m_Contexts.Size(); ++i)
	{
		if(!m_Contexts[i].GetUsage())
		{
			m_Contexts[i].SetUsage(true);
			m_Contexts[i].Reset();
			return &m_Contexts[i];
		}
	}
	BIZ_ASSERT_FAIL("Failed to allocate context!\n");
	return 0;
}

void C_Prepass::InsertFence()
{
#if defined(PLATFORM_PS3)
	m_FenceValue = Display::Device()->InsertFence();
	Display::Device()->WriteFence();
	cellGcmFlush();
#endif
}

void C_Prepass::Run(C_Context* pContext)
{
#if defined(PLATFORM_PS3) 
	const U32 iTextureWidth		= m_pNormalDepthTexture->GetWidth();
	const U32 iTextureHeight	= m_pNormalDepthTexture->GetHeight();

	const U32 iTilesX			= (iTextureWidth + c_FBTileWidth - 1) / c_FBTileWidth;
	const U32 iTilesY			= (iTextureHeight + c_FBTileHeight - 1) / c_FBTileHeight;

	pContext->m_JobIndex = 0;
	{
		PERF_EVENT("Prepass.SpuJumpToSelf");
		for (U32 i = 0; i < c_WorkerTasks; ++i)
		{
			pContext->m_WorkerInput[i].pJobIndex					= &pContext->m_JobIndex;
			pContext->m_WorkerInput[i].jobCount						= iTilesX * iTilesY;
			pContext->m_WorkerInput[i].gateAddress					= GCMCreateJumpToSelf128BytesAligned();
			pContext->m_WorkerInput[i].gateValue					= 0;
			pContext->m_WorkerInput[i].freeAddress					= (U32)&pContext->m_Usage;
			pContext->m_WorkerInput[i].freeValue					= 0;
			pContext->m_WorkerInput[i].pointLightsAddress			= (U32)pContext->m_PointLights;
			pContext->m_WorkerInput[i].pointLightsCount				= pContext->m_PointLightCount;
			pContext->m_WorkerInput[i].viewAddress					= (U32)&pContext->m_View;
			pContext->m_WorkerInput[i].fogAddress					= (U32)&pContext->m_Fog;
			pContext->m_WorkerInput[i].textureWidth					= iTextureWidth;
			pContext->m_WorkerInput[i].textureHeight				= iTextureHeight;
			pContext->m_WorkerInput[i].normalDepthBufferAddress		= (U32)m_pNormalDepthTexture->GetAddress();
			pContext->m_WorkerInput[i].lightingBufferAddress		= (U32)m_pLightingTexture->GetAddress();
			pContext->m_WorkerInput[i].normalDepthBufferBankOffset	= 0; // this is nothing more than intelligent guess work at the moment
			Scheduler::AddTask(JobQueuePrepassWorkerModule, &pContext->m_WorkerInput[i], (void*)m_FenceValue, m_JobQueueWorkerGroup, m_JobQueuePolicy, 0, 0);
		}
	}
#endif
}

void C_Prepass::SetJobQueuePolicy(Scheduler::Policy policyHandle)
{
	m_JobQueuePolicy = policyHandle;
}

}; // namespace JobQueue
}; // namespace Prepass
