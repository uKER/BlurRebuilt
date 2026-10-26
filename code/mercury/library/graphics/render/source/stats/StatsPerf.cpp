#include "stdafx.h"
#include "Render/Stats/StatsPerf.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Render
{
namespace Stats
{

// C_LoadStat

C_LoadStat::C_LoadStat()
{
	Reset();
}

C_LoadStat::~C_LoadStat()
{
}

void C_LoadStat::Reset()
{
	m_VertexDeclarationCount	= 0;
	m_VertexBufferCount			= 0;
	m_VertexBufferSize			= 0;
	m_IndexBufferCount			= 0;
	m_IndexBufferSize			= 0;
	m_TextureCount				= 0;
	m_TextureSize				= 0;
	m_ShaderCount				= 0;
	m_MaterialCount				= 0;
	m_GeometryCount				= 0;
	m_ChunkCount				= 0;
	m_BatchCount				= 0;
	m_PrimitiveCount			= 0;
}

void C_LoadStat::RenderString(const Maths::C_Sphere& sphere, const Maths::RGBA colour, const Util::C_String& string)
{
	Display::FontRender()->Draw3D(string.Chars(), sphere.Centre(), 1.0f, colour, Display::E_FontRenderAlign_Ct);
}

void C_LoadStat::RenderBounds(const Maths::C_Sphere& sphere, const Maths::RGBA colour)
{
	Display::BasicRender()->DrawColoured3DSphere(sphere.Centre(), sphere.Radius(), colour);
}

void C_LoadStat::AddIndexToList(const S32 item, Ut::Vector<S32>& list)
{
	for(S32 i=0; i<list.Size(); i++)
	{
		if(item == list[i])
			return;
	}
	list.PushBack(item);
}

// C_DrawStat

C_DrawStat::C_DrawStat()
{
	Reset();
}

C_DrawStat::~C_DrawStat()
{
}

void C_DrawStat::Reset()
{
	m_ChunkCount				= 0;
	m_BatchCount				= 0;
	m_PrimitiveCount			= 0;
	m_UpdateTimeCPU				= 0.0f;
	m_RenderTimeCPU				= 0.0f;
	m_RenderTimeGPU				= 0.0f;
}

void C_DrawStat::operator+=(const C_DrawStat& other)
{
	m_ChunkCount		+= other.m_ChunkCount;
	m_BatchCount		+= other.m_BatchCount;
	m_PrimitiveCount	+= other.m_PrimitiveCount;
}

#if 0
void C_PerfStat::UpdateString(const U32 flags, Util::C_String& string)
{
	#define BytesToMB(_bytes) ((F32)_bytes)/(1024.0f*1024.0f)

	string = m_Name;

	if(flags & E_PerfStat_ShowLoadStats)
	{
		if(g_debugRenderPerfStatsShowAllLoadStats)
		{
			string.Printf("%s bat%i tri%i vd%i vb%.2f/%i ib%.2f/%i t2d%.2f/%i",
				string.Chars(),
				m_BatchCount,
				m_PrimitiveCount,
				m_VertexDeclarationCount,
				BytesToMB(m_VertexBufferSize),
				m_VertexBufferCount,
				BytesToMB(m_IndexBufferSize),
				m_TextureCount,
				BytesToMB(m_TextureSize),
				m_IndexBufferCount);
		}
		else
		{
			string.Printf("%s ( %i / %i )",
				string.Chars(),
				m_BatchCount,
				m_PrimitiveCount);
		}
	}

	if(flags & E_PerfStat_ShowDrawStats)
	{
		string.Printf("%s ( %i / %i )",
			string.Chars(),
			m_VisibleBatchCount,
			m_VisiblePrimitiveCount);
	}
}

void C_PerfStat::RenderString(const Maths::HVec3& position, const Maths::RGBA colour, const Util::C_String& string)
{
	Display::FontRender()->Draw3D(string.Chars(), position, 1.0f, colour, Display::E_FontRenderAlign_Ct);
}

void C_PerfStat::RenderString(const Maths::C_Sphere& sphere, const Maths::RGBA colour, const Util::C_String& string)
{
	Display::FontRender()->Draw3D(string.Chars(), sphere.Centre(), 1.0f, colour, Display::E_FontRenderAlign_Ct);
}

void C_PerfStat::RenderBounds(const Maths::C_Sphere& sphere, const Maths::RGBA colour)
{
	Display::BasicRender()->DrawColoured3DSphere(sphere.Centre(), sphere.Radius(), colour);
}
#endif

}; // namespace Stats
}; // namespace Render
