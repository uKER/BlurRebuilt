
#include "DebugSystemsPch.h"
#include <DebugSystems/ProfilerView.h>

#pragma warning (push, 3)
#include <system/perf/Profiler.h>
#include <system/perf/ProfilerUtil.h>
#include <Display.h>
#include <Display/Buffer/VertexDeclarations.h>
#include <btl/utility/hash.h>
#pragma warning (pop)

#ifdef PLATFORM_PS3
#include <sys/sys_time.h>
#include <sys/time_util.h>
#endif

namespace DebugSystems
{

using namespace Maths;

static Debug::Bool ProfEnable("System.ProfilerView.Enable", false);
static Debug::Int  BarDepth("System.ProfilerView.Depth", 3, Debug::Range(1, 10));
static Debug::Float BarScale("System.ProfilerView.Scale", 20.f, Debug::Range(0.f, 40.f));
static Debug::Bool BarKey("System.ProfilerView.ShowKey", false);
static Debug::Float Scroll("System.ProfilerView.Scroll",  0.f, Debug::Range(0.f, 1.f));
static Debug::Float MinimumTime("System.ProfilerView.MinimumTime", 0.2f, Debug::Range(0.f, 1.f));

C_ProTotal::C_ProTotal(const char* name, U32 colour, F32 total, S8 depth)
{
	m_Name = name;
	m_Colour = colour;
	m_Total = total;
	m_Depth = depth;
}

C_ProfilerView::C_ProfilerView()
{
	m_LastFrameEnd = 0;
	m_VertexBuffer = 0;
	m_VertexPos = m_VertexCount = 0;
	m_LastTime = 0.f;
	m_bCallbackRegistered = false;
}

void C_ProfilerView::Init()
{
}

void C_ProfilerView::ProfilerCallback(Perf::ProfilerCommon* profiler, void* that)
{
	((C_ProfilerView*)that)->Callback(profiler);
}

void C_ProfilerView::Callback(Perf::ProfilerCommon* profiler)
{
	m_ProMark.Clear();
	m_ProTotal.Clear();

	Perf::EventList list = profiler->GetEventList();

	S32 max = list.Size();
	if (max <= 0)
		return;

	// Base the time information relative to the end of the last frame:
	S32 start = (S32)( m_LastFrameEnd ? m_LastFrameEnd : list[0].time );

	S32 depth = 0;
	Ut::UtStackVector<const char*, 32> nameStack;
	const char *name = "???"; // Name of the section we are currently in

	U32 mainThread = 0;
#if defined( PLATFORM_PS3 )
	sys_ppu_thread_t tid;
	sys_ppu_thread_get_id(&tid);
	mainThread = tid;
#endif

	m_LastTime = 0.f;

	for (S32 i = 0; i < max; i++)
	{
		Perf::Event& evt = list[i];
		
		if (evt.name == (char*)-1)
			continue; // Thread switch event
		if (evt.thread != mainThread)
			continue;

		S32 bits = (S32)((evt.time >> 60) & 15);
		bool end = (bits & 8) != 0;
		S32 prevDepth = depth;

		if (!end) depth++;
		
		if (depth >= BarDepth)
		{
			if (end) depth--;
			continue;
		}

		AddMark(name, (S32)evt.time - start, (S8)prevDepth);

		// Update the name of the event we are going into now:
		if (!end)
		{
			// Beginning of event:
			nameStack.PushBack(name);
			name = evt.name;
		}
		else
		{
			// End of event:
			if (nameStack.Size() > 0)
			{
				name = nameStack.Back();
				nameStack.PopBack();
			}
			else
				name = "???";
		}

		if (end) depth--;
	}

	// Find the time at the end of the profiling info
	if (max > 0)
	{
#if defined( PLATFORM_PS3 )
		SYS_TIMEBASE_GET(m_LastFrameEnd);
#else
		m_LastFrameEnd = list[max - 1].time;
#endif
	}
	
	AddMark("End", (S32)m_LastFrameEnd - start, 0);
}

void C_ProfilerView::AddMark(const char* name, S32 timeVal, S8 depth)
{
	C_ProMark mark;
	mark.m_Name = name;
	
#ifdef PLATFORM_PS3
	const F32 scale = 1.f / 79800.f; // sys_time_get_timebase_frequency()
#else
	const F32 scale = 1.f / 50000.f;
#endif

	mark.m_Time = (F32)timeVal * scale;
	mark.m_Colour = 0xff404040 | Btl::Hash(name);

	m_ProMark.PushBack(mark);

	F32 taken = mark.m_Time - m_LastTime;
	AddTimeToCategory(name, mark.m_Colour, taken, (S8)depth);
	m_LastTime = mark.m_Time;
}

void C_ProfilerView::AddTimeToCategory(const char* name, U32 colour, F32 ms, S8 depth)
{
	S32 max = m_ProTotal.Size();
	S32 i = 0;

	for (; i < max; i++)
	{
		if (m_ProTotal[i].m_Name == name) // sic - if we are pointing at the same string
			break;
	}

	if (i >= max)
		m_ProTotal.PushBack(C_ProTotal(name, colour, ms, depth));
	else
		m_ProTotal[i].m_Total += ms;
}

void C_ProfilerView::Render()
{
	using namespace Maths;
	using namespace Util;

	if (m_bCallbackRegistered != ProfEnable)
	{
		if (ProfEnable)
			Perf::ProfilerCommon::Instance().RegisterCallback(ProfilerCallback, this);
		else
		{
			Perf::ProfilerCommon::Instance().UnregisterCallback(ProfilerCallback, this);
			m_ProMark.Clear();
			m_ProTotal.Clear();
		}
		m_bCallbackRegistered = ProfEnable;
	}

	if (m_ProMark.Size() <= 0)
		return;

	Display::Device()->SetDepthFunc(Display::E_CmpFunc_Always);

	m_VertexCount = 4; // Background
	m_VertexCount += m_ProMark.Size() << 2;
	m_VertexCount += 8; // 60 / 30 fps marks

	if (m_VertexCount >= 64000)
		m_VertexCount = 64000;

	Display::BasicRender()->BindColoured2D();
	m_VertexBuffer = (Display::C_BasicRenderColouredVertex2D*)Display::Device()->BeginVertices
	(
		Display::E_PrimitiveType_QuadList,
		m_VertexCount,
		sizeof(Display::C_BasicRenderColouredVertex2D),
		Display::BasicRender()->m_pVertexDeclaration[Display::E_BasicRenderVertexShader_Coloured2D],
		true
	);

	m_VertexPos = 0;

	if (m_VertexBuffer == 0)
	{
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);
		return;
	}

	F32 xoff = 64.f, yoff = 40.f;
	F32 prevx = 64.f;

	F32 point60 = xoff + (1000.f / 60.f) * BarScale;
	F32 point30 = xoff + (1000.f / 30.f) * BarScale;

	S32 keyRows = BarKey ? m_ProTotal.Size() : 0;
	F32 fontx = 14.f, fonty = 20.f;
	
	// Draw background:
	F32 height = 48.f + keyRows * fonty;
	if (height > 768.f)
		height = 768.f;
	AddRect(xoff - 4, yoff - 4, point30 + 4, yoff + height, 0xc0000000);

	for (S32 i = 0; i < m_ProMark.Size(); i++)
	{
		const C_ProMark& mark = m_ProMark[i];
		
		F32 x = xoff + mark.m_Time * BarScale;
		
		AddRect(prevx, yoff, x, yoff + 40.f, mark.m_Colour);

		prevx = x;
	}

	AddRect(point60 - 1.f, yoff - 4.f, point60 + 1.f, yoff + 48.f, 0x80ffffff);
	AddRect(point30 - 1.f, yoff - 4.f, point30 + 1.f, yoff + 48.f, 0x80ffffff);

	Display::Device()->EndVertices(m_VertexBuffer + m_VertexPos);

	yoff = 92.f;
	F32 range = keyRows * fonty - 640.f;
	if (range > 0.f)
		yoff -= range * Scroll;

	for (S32 i = 0; i < keyRows; i++)
	{
		C_ProTotal& t = m_ProTotal[i];
		
		F32 x = xoff;
		F32 y = yoff + i * fonty;
		if (y < 80.f || y >= 720.f)
			continue;

		RGBA col(t.m_Colour);
		F32 alpha = (t.m_Total - MinimumTime) * 10.f + 1.f;
		if (alpha < 0.f)
			continue;
		
		if (alpha > 1.f) alpha = 1.f;
		col.A((U8)(alpha * 255.f));

		char text[32] = {0};
		sprintf(text, "%6.2f", t.m_Total);
		Display::FontRender()->Draw(text,     x, y, fontx, fonty, col);
		
		x += (6 + (F32)t.m_Depth) * fontx;
		Display::FontRender()->Draw(t.m_Name, x, y, fontx, fonty, col);
	}
	
	Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);
}

void C_ProfilerView::AddRect(F32 x1, F32 y1, F32 x2, F32 y2, U32 colour)
{
	// Check we allocated enough space:
	if (m_VertexPos + 4 > m_VertexCount)
		return;

	Display::C_BasicRenderColouredVertex2D* pv = m_VertexBuffer + m_VertexPos;

	pv->x = x1; pv->y = y2; pv->c = colour; pv++;
	pv->x = x1; pv->y = y1; pv->c = colour; pv++;
	pv->x = x2; pv->y = y1; pv->c = colour; pv++;
	pv->x = x2; pv->y = y2; pv->c = colour; pv++;
	m_VertexPos += 4;
}

}
