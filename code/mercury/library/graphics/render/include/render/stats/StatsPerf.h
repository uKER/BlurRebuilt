#ifndef RENDER_STATS_STATS_PERF_H
#define RENDER_STATS_STATS_PERF_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef CFG_GOLD
#define TRACK_LOAD_STATS // [LloydW] Undef this to knock out tracking load stats.
#define TRACK_DRAW_STATS // [LloydW] Undef this to knock out tracking draw stats.
#endif

namespace Render
{
namespace Stats
{

//! C_LoadStat

class C_LoadStat
{
public:

	C_LoadStat();
	~C_LoadStat();

	void Reset();

	static void RenderString(const Maths::C_Sphere& sphere, const Maths::RGBA colour, const Util::C_String& string);
	static void RenderBounds(const Maths::C_Sphere& sphere, const Maths::RGBA colour);

	static void AddIndexToList(const S32 index, Ut::Vector<S32>& list);
	
public:

	S32					m_VertexDeclarationCount;
	S32 				m_VertexBufferCount;
	S32 				m_VertexBufferSize;
	S32 				m_IndexBufferCount;
	S32 				m_IndexBufferSize;
	S32					m_TextureCount;
	S32					m_TextureSize;
	S32					m_ShaderCount;
	S32					m_MaterialCount;
	S32					m_GeometryCount;
	S32					m_ChunkCount;
	S32 				m_BatchCount;
	S32					m_PrimitiveCount;
};

//! C_DrawStat

class C_DrawStat
{
public:

	C_DrawStat();
	~C_DrawStat();

	void Reset();

	void operator+=(const C_DrawStat& other);

public:

	S32					m_ChunkCount;
	S32 				m_BatchCount;
	S32					m_PrimitiveCount;
	F32					m_UpdateTimeCPU;
	F32					m_RenderTimeCPU;
	F32					m_RenderTimeGPU;
};

}; // namespace Stats
}; // namespace Render

#endif // RENDER_STATS_STATS_PERF_H
