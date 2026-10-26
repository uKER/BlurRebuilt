#include "stdafx.h"
#include "renderingstatistics.h"
#include "system/debuglog.h"
#include "system/debug/log/logger.h"
#include "core/threads/atomic.h"
#include "system/debug/Variables.h"
#include "Fx/FxStatistics.h"
#include "Fx/Effect.h"
#include "Fx/Shared.h"

//#pragma optimize("", off)

namespace RenderingData {
namespace Target
{

static Debug::Logger &g_TraceLog = Debug::GetLogger("graphics.renderingdata.trace");
static Debug::Logger &g_StatsLog = Debug::GetLogger("graphics.renderingdata.stats");
static Debug::Logger &g_DataLog = Debug::GetLogger("graphics.renderingdata.data");

Debug::Bool g_bDebugTrace("Graphics.RenderingData.Trace", false);

RenderingStatistics RenderingStatistics::s_instance;

RenderingStatistics::RenderingStatistics()
{
	Reset();
}

#ifndef CFG_SPEED

// Main thread only.
void RenderingStatistics::BeginFrame()
{
	memset(&m_frame, 0, sizeof(m_frame));
	m_effectNames.Clear();
	m_effectPointers.Clear();
	m_parameters.Clear();
	m_vertexBuffers.Clear();
	m_indexBuffers.Clear();
	m_scenes.Clear();
	m_memoryHit.Clear();
	m_sceneCounters.Clear();
	m_nodeCounters.Clear();
	m_vertexDeclarations.Clear();
	m_materialTypes.Clear();

	Debug::Logger &log = Debug::GetLogger("graphics.renderingdata");
	log.SetLevel(g_bDebugTrace?::DebugLevel::Debug : ::DebugLevel::Info);

	FX::FxStatistics::Instance().BeginFrame();
}

// Main thread only.
void RenderingStatistics::EndFrame()
{
	g_bDebugTrace = false;

	FX::FxStatistics::Instance().EndFrame((float)m_frame.drawPrimitive);

	if (Debug::GetEffectiveLevel(g_StatsLog) < ::DebugLevel::Debug)
		return;

	m_frameCount++;
	m_total.drawPrimitive += m_frame.drawPrimitive;
	m_total.indicesRendered += m_frame.indicesRendered;
	m_total.setVertexBuffer += m_frame.setVertexBuffer;
	m_total.setIndexBuffer += m_frame.setIndexBuffer;
	m_total.setMaterial += m_frame.setMaterial;
	m_total.setParameter += m_frame.setParameter;
	m_total.setEffect += m_frame.setEffect;
	m_total.setTexture += m_frame.setTexture;
	m_total.setBlendState += m_frame.setBlendState;
	m_total.setVertexDeclaration += m_frame.setVertexDeclaration;
	m_total.beginScene += m_frame.beginScene;
	m_total.beginContext += m_frame.beginContext;
	Dump();
}

// Main thread only.
void RenderingStatistics::Reset()
{
	memset(&m_frame, 0, sizeof(m_frame));
	memset(&m_total, 0, sizeof(m_total));
	m_frameCount = 0;
}

// Main thread only.
void RenderingStatistics::Dump()
{
	LOGDEBUG( g_StatsLog, "Total calls:");
	LOGDEBUG( g_StatsLog, "DrawPrimitive: %i", m_frame.drawPrimitive);
	LOGDEBUG( g_StatsLog, "IndicesRendered: %i", m_frame.indicesRendered);
	LOGDEBUG( g_StatsLog, "SetVertexBuffer: %i, %i unique values", m_frame.setVertexBuffer, m_vertexBuffers.Size());
	LOGDEBUG( g_StatsLog, "SetIndexBuffer: %i, %i unique values", m_frame.setIndexBuffer, m_indexBuffers.Size());
	LOGDEBUG( g_StatsLog, "SetMaterial: %i, %i types", m_frame.setMaterial, m_materialTypes.Size());
	LOGDEBUG( g_StatsLog, "SetParameter: %i", m_frame.setParameter);
	LOGDEBUG( g_StatsLog, "SetEffect: %i, %i unique values", m_frame.setEffect, m_effectPointers.Size());
	LOGDEBUG( g_StatsLog, "SetTexture: %i", m_frame.setTexture);
	LOGDEBUG( g_StatsLog, "SetBlendState: %i", m_frame.setBlendState);
	LOGDEBUG( g_StatsLog, "SetVertexDeclaration: %i, %i unique values", m_frame.setVertexDeclaration, m_vertexDeclarations.Size());
	LOGDEBUG( g_StatsLog, "BeginContext: %i", m_frame.beginContext);
	LOGDEBUG( g_StatsLog, "BeginScene: %i", m_frame.beginScene);
	LOGDEBUG( g_StatsLog, "SetNode: %i", m_frame.setNode);
	//LOGDEBUG( g_StatsLog, "Bytes Copied: %i", m_frame.memCopy);

	LOGDEBUG( g_StatsLog, "Scenes:");

	for (NameMap::Iterator i = m_scenes.Begin(); i != m_scenes.End(); ++i)
		LOGDEBUG( g_StatsLog, "  %s: %i", i->GetKey().Chars(), i->GetValue().count.count);

	LOGDEBUG( g_StatsLog, "Effects:");

	for (NameMap::Iterator i = m_effectNames.Begin(); i != m_effectNames.End(); ++i)
		LOGDEBUG( g_StatsLog, "  %s: set %i times", i->GetKey().Chars(), i->GetValue().count.count);

	LOGDEBUG( g_StatsLog, "Parameters:");

	for (NameMap::Iterator i = m_parameters.Begin(); i != m_parameters.End(); ++i)
		LOGDEBUG( g_StatsLog, "  %s: set %i times, %i unique values", i->GetKey().Chars(), i->GetValue().count.count, i->GetValue().dataMap.Size());

	for (MaterialMap::Iterator i = m_materialTypes.Begin(); i != m_materialTypes.End(); ++i)
	{
		LOGDEBUG( g_StatsLog, "Material: %i", i->GetValue().count.count);
		for (int j = 0; j < i->GetKey().Size(); j++)
		{
			S32 unique = i->GetValue().parameters[i->GetKey()[j]].dataMap.Size();
			LOGDEBUG( g_StatsLog, "  %s, %i unique values", i->GetKey()[j].Chars(), unique);
		}
	}

	//LOGDEBUG( g_StatsLog, "Bytes Copied:");
	//for (SceneCountersMap::Iterator i = m_sceneCounters.Begin(); i != m_sceneCounters.End(); ++i)
	//	LOGDEBUG( g_StatsLog, "%s: %i", i->GetKey().Chars(), i->GetValue().memCopy);

	//for (NodeCountersMap::Iterator i = m_nodeCounters.Begin(); i != m_nodeCounters.End(); ++i)
	//	LOGDEBUG( g_StatsLog, "%i: %i times, %i bytes", i->GetKey(), i->GetValue().count.count, i->GetValue().memCopy);
}

void RenderingStatistics::MemoryAccess(const void* p)
{
	//LOGDEBUG(g_TestLog, "MemoryAccess: %i", p);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		lock.Enter();

		// note: only works single threaded
		Btl::Set<U32>::InsertionReturnType r = m_memoryHit.Insert(((U32)p) & 0xffffff80);

		if (r.inserted)
			m_sceneCounters[m_currentScene].cacheMiss++;

		lock.Exit();
	}
}

void RenderingStatistics::MemCopy(S32 bytes) 
{
	MT::AtomicAdd(&m_frame.memCopy, bytes);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		lock.Enter();

		// note: only works single threaded
		m_sceneCounters[m_currentScene].memCopy += bytes;
		m_nodeCounters[m_currentNode].memCopy += bytes;

		lock.Exit();
	}
}

void RenderingStatistics::BeginScene(const char* name)
{
	LOGDEBUG( g_DataLog, "BeginScene '%s'\n", name);

	MT::AtomicIncrement(&m_frame.beginScene);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		lock.Enter();
		m_currentScene = name;
		m_scenes[name].count++;
		lock.Exit();
	}
}

void RenderingStatistics::BeginContext()
{
	LOGDEBUG( g_DataLog, "BeginContext\n");
	MT::AtomicIncrement(&m_frame.beginContext);
}

void RenderingStatistics::SetNode(const void* pNode)
{
	MT::AtomicIncrement(&m_frame.setNode);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		lock.Enter();
		m_currentNode = pNode;
		m_nodeCounters[m_currentNode].count++;
		LOGDEBUG( g_DataLog, "SetNode\n");
		lock.Exit();
	}
}

void RenderingStatistics::BeginPrimitive(S32 i)
{
	LOGDEBUG( g_DataLog, "BeginPrimitive %i", i);
}

void RenderingStatistics::BeginInstance(S32 i)
{
	LOGDEBUG( g_DataLog, "BeginInstance %i", i);
}

void RenderingStatistics::BeginBaseInstance()
{
	LOGDEBUG( g_TraceLog, "BeginBaseInstance\n");
}

void RenderingStatistics::DrawPrimitive(U32 indexCount)
{
	LOGDEBUG( g_TraceLog, "DrawPrimitive\n");
	MT::AtomicIncrement(&m_frame.drawPrimitive);
	MT::AtomicAdd(&m_frame.indicesRendered, indexCount);
}

void RenderingStatistics::SetVertexBuffer(U32 data)
{
	LOGDEBUG( g_TraceLog, "SetVertexBuffer\n");
	MT::AtomicIncrement(&m_frame.setVertexBuffer);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		lock.Enter();
		m_vertexBuffers[data]++;
		lock.Exit();
	}
}

void RenderingStatistics::SetIndexBuffer(U32 data)
{
	LOGDEBUG( g_TraceLog, "SetIndexBuffer\n");
	MT::AtomicIncrement(&m_frame.setIndexBuffer);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		lock.Enter();
		m_indexBuffers[data]++;
		lock.Exit();
	}
}

void RenderingStatistics::BeginMaterial()
{
	LOGDEBUG( g_TraceLog, "SetMaterial\n");
	MT::AtomicIncrement(&m_frame.setMaterial);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		lock.Enter();
	}
}

void RenderingStatistics::EndMaterial()
{
//	LOGWARN(g_TraceLog,"EndMaterial 1");

 	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
 
		MaterialData& materialData = m_materialTypes[m_currentMaterial];
		materialData.count++;

		for (int i = 0; i < m_currentMaterial.Size(); i++)
		{
			CountUnique& total = materialData.parameters[m_currentMaterial[i]];
			CountUnique& current = m_currentMaterialData[m_currentMaterial[i]];
			
			total.count.count += current.count.count;

			for (DataMap::Iterator j = current.dataMap.Begin(); j != current.dataMap.End(); ++j)
				total.dataMap[j->GetKey()].count += j->GetValue().count;
		}

		m_currentMaterial.Clear();
		m_currentMaterialData.Clear();
 
 
		lock.Exit();

	}
//LOGWARN(g_TraceLog,"EndMaterial 5");
}

void RenderingStatistics::SetParameter(const char* name, const U8* pData, S32 size)
{
	LOGDEBUG( g_TraceLog, "Set %s to %x\n", name, (U32)*pData);
	MT::AtomicIncrement(&m_frame.setParameter);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		U32 data = 0;

		for (int i = 0; i < size; i++)
			data = Btl::Hash(pData[i], data);

		//lock.Enter();
		m_parameters[name].count++;
		m_parameters[name].dataMap[data]++;
		m_currentMaterialData[name].count++;
		m_currentMaterialData[name].dataMap[data]++;
		//lock.Exit();

		m_currentMaterial.PushBack(name);
	}
}

void RenderingStatistics::SetEffect(FX::Effect *pEffect)
{
	const char *name = ((FX::SharedEffect*)pEffect)->Name.Chars();
	LOGDEBUG( g_TraceLog, "SetEffect '%s'\n", name);
	MT::AtomicIncrement(&m_frame.setEffect);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		lock.Enter();
		m_effectNames[name].count++;
		m_effectPointers[(U32)pEffect].count++;
		lock.Exit();
	}
}

void RenderingStatistics::SetTexture()
{
	LOGDEBUG( g_TraceLog, "SetTexture\n");
	MT::AtomicIncrement(&m_frame.setTexture);
}

void RenderingStatistics::SetBlendState()
{
	LOGDEBUG( g_TraceLog, "SetBlendState\n");
	MT::AtomicIncrement(&m_frame.setBlendState);
}

void RenderingStatistics::SetVertexDeclaration(U32 data)
{
	LOGDEBUG( g_TraceLog, "SetVertexDeclaration\n");
	MT::AtomicIncrement(&m_frame.setVertexDeclaration);

	if (g_StatsLog.GetEffectiveLevel() == DebugLevel::Debug)
	{
		lock.Enter();
		m_vertexDeclarations[data].count++;
		lock.Exit();
	}
}

#endif

}}
