#include "LodPch.h"

#include <Lod/LodManager.h>
#include <Display/Render/BasicRender.h>
#include <Display/Render/FontRender.h>

namespace Lod
{

#if PLATFORM_PC
// [Blur2PC] Retail PC ships with dynamic detail off (Blur.exe 1.2 0x00e32a30); the graphical detail
// preset sets the draw distance ranges and detail stays at 1.
Debug::Bool g_LodEnabled("GameSys.Lod.DynamicEnabled", false);
#elif defined(CFG_SPEED) && !defined(CFG_PROFILE)
Debug::Bool g_LodEnabled("GameSys.Lod.DynamicEnabled", true);
#else
Debug::Bool g_LodEnabled("GameSys.Lod.DynamicEnabled", false);
#endif

Debug::Bool g_LodDetailOverride("GameSys.Lod.OverrideDetail", false);
Debug::Float g_LodDetailCpu("GameSys.Lod.CpuDetail", 1.f, Debug::Range(0.f, 1.f));
Debug::Float g_LodDetailGpu("GameSys.Lod.GpuDetail", 1.f, Debug::Range(0.f, 1.f));
Debug::Bool g_LodTargetOverride("GameSys.Lod.OverrideTargetFPS", false);
Debug::Float g_LodTarget("GameSys.Lod.TargetFPS", 30.f, Debug::Range(0.f, 100.f));

Debug::Float g_LodRenderSize	("GameSys.Lod.RenderSize",		10.3f, Debug::Range(0, 50));
Debug::Float g_LodRenderX		("GameSys.Lod.RenderX",	132.0f, Debug::Range(0.f, 1024.f));
Debug::Float g_LodRenderY		("GameSys.Lod.RenderY",	760.f, Debug::Range(0.f, 768.f));

// C_LodManager

C_LodManager::C_LodManager() : m_TargetFrameTime(0.030f), m_AdaptSpeed(1.0f), m_CpuDetail(1.0f), m_GpuDetail(1.0f)
{
}

C_LodManager::~C_LodManager()
{
}

void C_LodManager::Initialise(const F32 frameRate)
{
	// aim for a framerate 10% better than requested to give us breathing room
	m_TargetFrameTime = 1.0f / (frameRate * 1.1f);
}

void C_LodManager::Update(const F32 frameTime, const bool updateCpu, const bool updateGpu)
{
	if(g_LodEnabled)
	{
		F32 adaptFrame = frameTime * m_AdaptSpeed;

		F32 targetFrameTime;

		if(g_LodTargetOverride)
			targetFrameTime = 1.0f / (g_LodTarget * 1.1f);
		else
			targetFrameTime = m_TargetFrameTime;

		if(updateCpu)
		{
			F32 cpuTime = Display::Device()->GetCpuTime() / 1000.0f;
			F32 cpuDiff = targetFrameTime - cpuTime;
			m_CpuDetail += Maths::Clamped(cpuDiff, -adaptFrame, adaptFrame);
			m_CpuDetail = Maths::Clamped(m_CpuDetail, 0.0f, 1.0f);
		}
		if(updateGpu)
		{
			F32 gpuTime = Display::Device()->GetGpuTime() / 1000.0f;
			F32 gpuDiff = targetFrameTime - gpuTime;
			m_GpuDetail += Maths::Clamped(gpuDiff, -adaptFrame, adaptFrame);
			m_GpuDetail = Maths::Clamped(m_GpuDetail, 0.0f, 1.0f);
		}
		if(g_LodDetailOverride)
		{
			m_CpuDetail = g_LodDetailCpu;
			m_GpuDetail = g_LodDetailGpu;
		}

		SetDynamicDetail(m_CpuDetail, m_GpuDetail, g_LodDetailOverride);
	}
	else
	{
		SetDynamicDetail(1.0f, 1.0f, true);
	}
}

void C_LodManager::SetStaticDetail(const F32 cpuDetail, const F32 gpuDetail)
{
	for(LodInterfaces::Iterator pIterator = m_LodInterfaces.Begin(); pIterator != m_LodInterfaces.End(); pIterator++)
	{
		I_LodInterface* pLodInterface = pIterator->GetValue();
		pLodInterface->SetStaticDetail(cpuDetail, gpuDetail);
	}
}

void C_LodManager::SetDynamicDetail(const F32 cpuDetail, const F32 gpuDetail, const bool immediate)
{
	for(LodInterfaces::Iterator pIterator = m_LodInterfaces.Begin(); pIterator != m_LodInterfaces.End(); pIterator++)
	{
		I_LodInterface* pLodInterface = pIterator->GetValue();
		pLodInterface->SetDynamicDetail(cpuDetail, gpuDetail, immediate);
	}
}

void C_LodManager::AddLodInterface(I_LodInterface* pLodInterface)
{
	m_LodInterfaces.Insert(pLodInterface);
}

void C_LodManager::RemoveLodInterface(I_LodInterface* pLodInterface)
{
	m_LodInterfaces.Erase(pLodInterface);
}

void C_LodManager::Render() const
{
	if(g_LodEnabled && (m_CpuDetail < 1.f || m_GpuDetail < 1.f))
	{
		char buffer[128];
		sprintf_s(buffer, sizeof(buffer), "Detail %.2f CPU %.2f GPU", m_CpuDetail, m_GpuDetail);
		Display::FontRender()->Draw(buffer, g_LodRenderX, g_LodRenderY - g_LodRenderSize, g_LodRenderSize, g_LodRenderSize, Maths::RGBA(255,255,255,255), Display::E_FontRenderAlign_Ct);
	}
}

} // Lod
