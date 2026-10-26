#ifndef RD_RENDERING_STATISTICS_H
#define RD_RENDERING_STATISTICS_H
#ifdef _MSC_VER
#pragma once
#endif

#if !PLATFORM_SPU
#include "core/threads/mutex.h"
#include "btl/containers/map.h"
#include "btl/containers/set.h"
#endif

namespace FX
{
class Effect;
}

namespace RenderingData {
namespace Target
{

class RenderingStatistics
{
public:
#if PLATFORM_SPU
	static RenderingStatistics& Instance() {return *(RenderingStatistics*)NULL;}
#else
	static RenderingStatistics& Instance() {return s_instance;}
#endif

#if !CFG_SPEED && !PLATFORM_SPU

	void BeginFrame();
	void EndFrame();
	void Reset();
	void Dump();
	void BeginContext();
	void BeginScene(const char* name);
	void SetNode(const void* pNode);
	void BeginPrimitive(S32 i);
	void BeginInstance(S32 i);
	void BeginBaseInstance();
	void DrawPrimitive(U32 indices);
	void SetVertexBuffer(U32 data);
	void SetIndexBuffer(U32 data);
	void BeginMaterial();
	void EndMaterial();
	void SetParameter(const char* name, const U8* pData, S32 size);
	void SetEffect(FX::Effect *pFX);
	void SetTexture();
	void SetBlendState();
	void SetVertexDeclaration(U32 data);
	void MemoryAccess(const void* p);
	void MemCopy(S32 bytes);

#else

	void BeginFrame() {}
	void EndFrame() {}
	void Reset() {}
	void Dump() {}
	void BeginContext() {}
	void BeginScene(const char* name) {}
	void SetNode(const void* pNode) {}
	void BeginPrimitive(S32 i) {}
	void BeginInstance(S32 i) {}
	void BeginBaseInstance() {}
	void DrawPrimitive(U32 indices) {}
	void SetVertexBuffer(U32 data) {}
	void SetIndexBuffer(U32 data) {}
	void BeginMaterial() {}
	void EndMaterial() {}
	void SetParameter(const char* name, const U8* pData, S32 size) {}
	void SetEffect(FX::Effect *pFX) {}
	void SetTexture() {}
	void SetBlendState() {}
	void SetVertexDeclaration(U32 data) {}
	void MemoryAccess(const void* p) {}
	void MemCopy(S32 bytes) {}

#endif

private:
	RenderingStatistics();
	static RenderingStatistics s_instance;

#if !CFG_SPEED && !PLATFORM_SPU
	struct Counters
	{
		S32 drawPrimitive;
		U32 indicesRendered;
		S32 setVertexBuffer;
		S32 setIndexBuffer;
		S32 setMaterial;
		S32 setParameter;
		S32 setEffect;
		S32 setTexture;
		S32 setBlendState;
		S32 setVertexDeclaration;
		S32 beginContext;
		S32 beginScene;
		S32 setNode;
		U32 memCopy;
	};

	struct Count
	{
		Count() : count(0) {}
		void operator ++(int) {count++;}
		S32 count;
	};

	typedef Btl::Map<U32, Count> DataMap;

	struct CountUnique
	{
		Count count;
		DataMap dataMap;
	};

	typedef Btl::Map<Btl::String, CountUnique> NameMap;
	NameMap m_effectNames;
	NameMap m_parameters;
	NameMap m_scenes;

	DataMap m_effectPointers;
	DataMap m_vertexBuffers;
	DataMap m_indexBuffers;
	DataMap m_vertexDeclarations;

	S32 m_frameCount;
	Counters m_frame;
	Counters m_total;

	Btl::Set<U32> m_memoryHit;
	Btl::String m_currentScene;
	const void* m_currentNode;

	struct SceneCounters
	{
		SceneCounters() : memCopy(0) {}

		Count cacheMiss;
		U32 memCopy;
	};

	typedef Btl::Map<Btl::String, SceneCounters> SceneCountersMap;
	SceneCountersMap m_sceneCounters;

	struct NodeCounters
	{
		NodeCounters() : memCopy(0) {}

		Count count;
		U32 memCopy;
	};

	typedef Btl::Map<const void*, NodeCounters> NodeCountersMap;
	NodeCountersMap m_nodeCounters;

	struct MaterialData
	{
		NameMap parameters;
		Count count;
	};

	typedef Btl::Vector<Btl::String> Material;
	Material m_currentMaterial;
	NameMap m_currentMaterialData;
	typedef Btl::Map<Material, MaterialData> MaterialMap;
	MaterialMap m_materialTypes; 

	MT::Lock lock;
#endif
};

}}

#endif
