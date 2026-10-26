#include "stdafx.h"
#include "Deferred/Prepass/SPU/Lighting.h"
#include "Deferred/Prepass/SPU/JobQueue.h"
#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

namespace Deferred
{
namespace JobQueue
{

const U32 c_FBTileWidth		= 64;
const U32 c_FBTileHeight	= 64;

C_Context::C_Context()
	: m_PointLightCount(0)
	, m_SpotLightCount(0)
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
		m_PointLights[m_PointLightCount].fInvRadiusSq[0] = 1/ (fRadius * fRadius);
		m_PointLights[m_PointLightCount].fInvRadiusSq[1] = m_PointLights[m_PointLightCount].fInvRadiusSq[0];
		m_PointLights[m_PointLightCount].fInvRadiusSq[2] = m_PointLights[m_PointLightCount].fInvRadiusSq[0];
		m_PointLights[m_PointLightCount].fInvRadiusSq[3] = m_PointLights[m_PointLightCount].fInvRadiusSq[0];
		++m_PointLightCount;
	}
}

void C_Context::AddSpotLight(const Display::C_Matrix& mWorld, const Display::C_Float3& vColour, const F32 fRadius, const F32 fHorizontalFoV, const F32 fVerticalFoV, const U32 iTexture)
{

	if(m_SpotLightCount < c_MaximumSpotLights)
	{
		Maths::Mat44 mLightWorld = Maths::Load<Maths::Mat44>((MathsStorage::MatrixRow44*)&mWorld);
		Display::C_Float4 vLightPosition, vLightDirection;
		Maths::Store(Maths::MakeVec4F32(Maths::RowW(mLightWorld)), &vLightPosition);
		Maths::Store(Maths::MakeVec4F32(Maths::RowZ(mLightWorld)), &vLightDirection);
		F32 fFalloffX, fFalloffY;
		fFalloffX = 1.0f / (fRadius * fRadius);
		fFalloffY = cosf(fHorizontalFoV);
		
		m_SpotLights[m_SpotLightCount].fR				= vColour.x;
		m_SpotLights[m_SpotLightCount].fG				= vColour.y;
		m_SpotLights[m_SpotLightCount].fB				= vColour.z;
		m_SpotLights[m_SpotLightCount].fRadius			= fRadius;
		m_SpotLights[m_SpotLightCount].fPositionX[0]	= vLightPosition.x;
		m_SpotLights[m_SpotLightCount].fPositionX[1]	= vLightPosition.x;
		m_SpotLights[m_SpotLightCount].fPositionX[2]	= vLightPosition.x;
		m_SpotLights[m_SpotLightCount].fPositionX[3]	= vLightPosition.x;
		m_SpotLights[m_SpotLightCount].fPositionY[0]	= vLightPosition.y;
		m_SpotLights[m_SpotLightCount].fPositionY[1]	= vLightPosition.y;
		m_SpotLights[m_SpotLightCount].fPositionY[2]	= vLightPosition.y;
		m_SpotLights[m_SpotLightCount].fPositionY[3]	= vLightPosition.y;
		m_SpotLights[m_SpotLightCount].fPositionZ[0]	= vLightPosition.z;
		m_SpotLights[m_SpotLightCount].fPositionZ[1]	= vLightPosition.z;
		m_SpotLights[m_SpotLightCount].fPositionZ[2]	= vLightPosition.z;
		m_SpotLights[m_SpotLightCount].fPositionZ[3]	= vLightPosition.z;
		m_SpotLights[m_SpotLightCount].fDirectionX[0]	= vLightDirection.x;
		m_SpotLights[m_SpotLightCount].fDirectionX[1]	= vLightDirection.x;
		m_SpotLights[m_SpotLightCount].fDirectionX[2]	= vLightDirection.x;
		m_SpotLights[m_SpotLightCount].fDirectionX[3]	= vLightDirection.x;
		m_SpotLights[m_SpotLightCount].fDirectionY[0]	= vLightDirection.y;
		m_SpotLights[m_SpotLightCount].fDirectionY[1]	= vLightDirection.y;
		m_SpotLights[m_SpotLightCount].fDirectionY[2]	= vLightDirection.y;
		m_SpotLights[m_SpotLightCount].fDirectionY[3]	= vLightDirection.y;
		m_SpotLights[m_SpotLightCount].fDirectionZ[0]	= vLightDirection.z;
		m_SpotLights[m_SpotLightCount].fDirectionZ[1]	= vLightDirection.z;
		m_SpotLights[m_SpotLightCount].fDirectionZ[2]	= vLightDirection.z;
		m_SpotLights[m_SpotLightCount].fDirectionZ[3]	= vLightDirection.z;
		m_SpotLights[m_SpotLightCount].fFalloffX[0]		= fFalloffX;
		m_SpotLights[m_SpotLightCount].fFalloffX[1]		= fFalloffX;
		m_SpotLights[m_SpotLightCount].fFalloffX[2]		= fFalloffX;
		m_SpotLights[m_SpotLightCount].fFalloffX[3]		= fFalloffX;
		m_SpotLights[m_SpotLightCount].fFalloffY[0]		= fFalloffY;
		m_SpotLights[m_SpotLightCount].fFalloffY[1]		= fFalloffY;
		m_SpotLights[m_SpotLightCount].fFalloffY[2]		= fFalloffY;
		m_SpotLights[m_SpotLightCount].fFalloffY[3]		= fFalloffY;
	
		
		
//		m_SpotLights[m_SpotLightCount].fHorizontalFoV	= fHorizontalFoV;
// 		m_SpotLights[m_SpotLightCount].fVerticalFoV		= fVerticalFoV;
// 		m_SpotLights[m_SpotLightCount].iTexture			= iTexture;					//Think about texture support later.
		++m_SpotLightCount;
	}
}

void C_Context::AddView(const Display::C_View* pView)
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

	if(Display::Device()->GetDepthFlip())
	{
		m_View.fNearClip	= pView->GetViewSettings3D().GetFarClip();
		m_View.fFarClip		= pView->GetViewSettings3D().GetNearClip();
	}
	else
	{
		m_View.fNearClip	= pView->GetViewSettings3D().GetNearClip();
		m_View.fFarClip		= pView->GetViewSettings3D().GetFarClip();
	}
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

	for(S32 i = 0; i < c_MaximumSpotLights; ++i)
	{
		m_SpotLights[m_SpotLightCount].fR				= 0.0f;
		m_SpotLights[m_SpotLightCount].fG				= 0.0f;
		m_SpotLights[m_SpotLightCount].fB				= 0.0f;
		m_SpotLights[m_SpotLightCount].fRadius			= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionX[0]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionX[1]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionX[2]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionX[3]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionY[0]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionY[1]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionY[2]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionY[3]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionZ[0]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionZ[1]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionZ[2]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fPositionZ[3]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionX[0]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionX[1]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionX[2]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionX[3]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionY[0]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionY[1]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionY[2]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionY[3]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionZ[0]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionZ[1]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionZ[2]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fDirectionZ[3]	= 0.0f;
		m_SpotLights[m_SpotLightCount].fFalloffX[0]		= 0.0f;
		m_SpotLights[m_SpotLightCount].fFalloffX[1]		= 0.0f;
		m_SpotLights[m_SpotLightCount].fFalloffX[2]		= 0.0f;
		m_SpotLights[m_SpotLightCount].fFalloffX[3]		= 0.0f;
		m_SpotLights[m_SpotLightCount].fFalloffY[0]		= 0.0f;
		m_SpotLights[m_SpotLightCount].fFalloffY[1]		= 0.0f;
		m_SpotLights[m_SpotLightCount].fFalloffY[2]		= 0.0f;
		m_SpotLights[m_SpotLightCount].fFalloffY[3]		= 0.0f;
	}
	m_SpotLightCount = 0;

	m_JobIndex = 0;
}

bool C_Context::GetUsage()
{
	return (MT::AtomicGet(&m_Usage) != 0);
}

void C_Context::SetUsage(const bool bUsage)
{
	MT::AtomicSet(&m_Usage, bUsage ? 1 : 0);
}

C_Prepass::C_Prepass() : m_JobQueueMasterGroup(Scheduler::JQ_GROUP_NONE), m_JobQueueWorkerGroup(Scheduler::JQ_GROUP_NONE)
{
}

C_Prepass::~C_Prepass()
{
}

bool C_Prepass::Create()
{
	BIZ_ASSERT(m_JobQueueWorkerGroup == Scheduler::JQ_GROUP_NONE);

	m_JobQueueMasterGroup = Scheduler::CreateGroup();
	m_JobQueueWorkerGroup = Scheduler::CreateGroup();

#if PLATFORM_PS3
	Scheduler::AddGroupDependency(m_JobQueueWorkerGroup, (U32*)Display::Device()->GetWriteFenceAddress());
#endif

	return true;
}

void C_Prepass::Destroy()
{
	Scheduler::DestroyGroup(m_JobQueueWorkerGroup);
	Scheduler::DestroyGroup(m_JobQueueMasterGroup);

	m_JobQueueWorkerGroup = Scheduler::JQ_GROUP_NONE;
	m_JobQueueMasterGroup = Scheduler::JQ_GROUP_NONE;
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
	return 0;
}

void C_Prepass::Run(C_Context* pContext)
{
#if defined(PLATFORM_PS3)
	const U32 fenceValue = Display::Device()->InsertFence();
	Display::Device()->WriteFence();

	cellGcmFlush(); // must flush the command buffer before waiting on the fence (for instance, otherwise a GPAD capture will hang)

	{
		PERF_EVENT("SPU Lighting Flush");
//		Scheduler::Flush(m_JobQueueMasterGroup);
		Scheduler::Flush(m_JobQueueWorkerGroup);
	}
//	Scheduler::AddTask(LightingPrepassMasterModule, &pContext->m_MasterInput, 0, m_JobQueueMasterGroup, m_JobQueuePolicy, 0, 0);
	
	const U32 iTextureWidth		= m_pNormalTexture->GetWidth();
	const U32 iTextureHeight	= m_pNormalTexture->GetHeight();

	const U32 iTilesX			= (iTextureWidth + c_FBTileWidth - 1) / c_FBTileWidth;
	const U32 iTilesY			= (iTextureHeight + c_FBTileHeight - 1) / c_FBTileHeight;

	pContext->m_JobIndex = 0;
	{
		PERF_EVENT("Prepass.SpuJumpToSelf");
		for (U32 i = 0; i < c_WorkerTasks; ++i)
		{
			pContext->m_WorkerInput[i].pJobIndex				= &pContext->m_JobIndex;
			pContext->m_WorkerInput[i].jobCount					= iTilesX * iTilesY;
			pContext->m_WorkerInput[i].gateAddress				= GCMCreateJumpToSelf128BytesAligned();
			pContext->m_WorkerInput[i].gateValue				= 0;
			pContext->m_WorkerInput[i].freeAddress				= (U32)&pContext->m_Usage;
			pContext->m_WorkerInput[i].freeValue				= 0;
			pContext->m_WorkerInput[i].pointLightsAddress		= (U32)pContext->m_PointLights;
			pContext->m_WorkerInput[i].pointLightsCount			= pContext->m_PointLightCount;
			pContext->m_WorkerInput[i].spotLightsAddress		= (U32)pContext->m_SpotLights;
			pContext->m_WorkerInput[i].spotLightsCount			= pContext->m_SpotLightCount;
			pContext->m_WorkerInput[i].viewAddress				= (U32)&pContext->m_View;
			pContext->m_WorkerInput[i].textureWidth				= iTextureWidth;
			pContext->m_WorkerInput[i].textureHeight			= iTextureHeight;
			pContext->m_WorkerInput[i].normalBufferAddress		= (U32)m_pNormalTexture->GetAddress();
			pContext->m_WorkerInput[i].depthBufferAddress		= (U32)m_pDepthTexture->GetAddress();
			pContext->m_WorkerInput[i].lightingBufferAddress	= (U32)m_pLightingTexture->GetAddress();
			pContext->m_WorkerInput[i].normalBufferBankOffset	= 0; // these are nothing more than intelligent
			pContext->m_WorkerInput[i].depthBufferBankOffset	= 1; // guess work at the moment
			Scheduler::AddTask(LightingPrepassWorkerModule, &pContext->m_WorkerInput[i], (void*)fenceValue, m_JobQueueWorkerGroup, m_JobQueuePolicy, 0, 0);
		}
	}
#endif
}

void C_Prepass::SetJobQueuePolicy(Scheduler::Policy policyHandle)
{
	m_JobQueuePolicy = policyHandle;
}

}; // namespace JobQueue
}; // namespace Deferred
