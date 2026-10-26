#include "stdafx.h"

#include <core/threads/Atomic.h>
#include "fx/FXStatistics.h"
#include "fx/Effect.h"
#include "fx/Manager.h"
#include <System/debuglog.h>
#include <system/debug/log/logger.h>
#include <System/Perf.h>

namespace FX
{

static Debug::Logger &g_TraceLog = Debug::GetLogger("graphics.renderingdata.trace");
static Debug::Logger &g_StatsLog = Debug::GetLogger("graphics.renderingdata.stats");

FxStatistics FxStatistics::s_instance;

FxStatistics::FxStatistics()
{
#if ENABLE_FX_STATISTICS
	memset(&m_frame, 0, sizeof(m_frame));
#endif
}

void FxStatistics::BeginFrame()
{
#if ENABLE_FX_STATISTICS
	memset(&m_frame, 0, sizeof(m_frame));
	m_bindings.Clear();
	m_copies.Clear();
#endif
}

void FxStatistics::EndFrame(float fnPrimitives)
{
#if ENABLE_FX_STATISTICS
	Perf::AddNamedCounter(fnPrimitives, "Primitives");
	Perf::AddNamedCounter((F32)m_frame.nConstantsSet/fnPrimitives, "ConstantsSet");
	Perf::AddNamedCounter((F32)m_frame.nBindingsApplied/fnPrimitives, "BindingsSet");
	Perf::AddNamedCounter((F32)m_frame.nTexturesSet/fnPrimitives, "TexturesSet");
	Perf::AddNamedCounter((F32)m_frame.nSamplerStateChanges/fnPrimitives, "SamplerStatesSet");
	Perf::AddNamedCounter((F32)m_frame.nShaderChanges/fnPrimitives, "ShadersSet");

	Dump();
#endif
}

void FxStatistics::Dump()
{
#if ENABLE_FX_STATISTICS
	LOGDEBUG( g_StatsLog, "ApplyBinding: %i", m_frame.nBindingsApplied);
	LOGDEBUG( g_StatsLog, "SetConstant: %i", m_frame.nConstantsSet);
	LOGDEBUG( g_StatsLog, "SetTexture: %i", m_frame.nTexturesSet);
	LOGDEBUG( g_StatsLog, "SamplerState Changes: %i", m_frame.nSamplerStateChanges);
	LOGDEBUG( g_StatsLog, "Shader Changes: %i", m_frame.nShaderChanges);
	LOGDEBUG( g_StatsLog, "Parameter Copies: %i", m_frame.nParametersCopied);

	// list bindings and how many times each one was set this frame
	LOGDEBUG( g_StatsLog, "Shared parameters copied:");

	for (NameMap::Iterator i = m_copies.Begin(); i != m_copies.End(); ++i)
	{
		int x = i->GetValue().count.count;
		const char *psz = i->GetKey().Chars();
		LOGDEBUG( g_StatsLog, "%s: %i", psz, x );
		g_Manager.DumpConflictingBindings(g_StatsLog, psz, 10);
	}
#endif
}

#if ENABLE_FX_STATISTICS
void FxStatistics::ApplyBinding(const char *pszName)
{
	LOGDEBUG( g_TraceLog, "Applied Binding %s\n", pszName);
	MT::AtomicIncrement(&m_frame.nBindingsApplied);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		lock.Enter();
		m_bindings[pszName].count++;
		lock.Exit();
	}
}

void FxStatistics::CopyParameter(const char *pszName)
{
	LOGDEBUG( g_TraceLog, "Copied Parameter %s\n", pszName);
	MT::AtomicIncrement(&m_frame.nParametersCopied);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		lock.Enter();
		m_copies[pszName].count++;
		lock.Exit();
	}
}

void FxStatistics::SetVSConstant(int iHandle)
{
	LOGDEBUG( g_TraceLog, "Set VS Constant %d\n", iHandle);
	MT::AtomicIncrement(&m_frame.nConstantsSet);
}

void FxStatistics::SetPSConstant(int iHandle)
{
	LOGDEBUG( g_TraceLog, "Set PS Constant %d\n", iHandle);
	MT::AtomicIncrement(&m_frame.nConstantsSet);

	if (iHandle >= 76 && iHandle < 80)
		int breakhere = 0;
}

void FxStatistics::SetTexture()
{
	LOGDEBUG( g_TraceLog, "Set Texture\n");
	MT::AtomicIncrement(&m_frame.nTexturesSet);
}

void FxStatistics::ChangeSamplerState()
{
	LOGDEBUG( g_TraceLog, "Sampler State Changed\n");
	MT::AtomicIncrement(&m_frame.nSamplerStateChanges);
}

void FxStatistics::ChangeShader()
{
	LOGDEBUG( g_TraceLog, "Shader Changed\n");
	MT::AtomicIncrement(&m_frame.nShaderChanges);
}
#endif

}
