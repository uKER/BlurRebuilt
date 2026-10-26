#ifndef DEBUGHUDPERF_H
#define DEBUGHUDPERF_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/vector.h>
#include <Btl/Utility/string.h>
#include <System/perf/ProfilerBackend.h>
#include <System/perf/ProfilerUtil.h>

namespace Render { namespace Font { class C_FontRenderer;}}

namespace DebugSystems
{

class C_DebugHUDPerf
{
public:
	C_DebugHUDPerf();
	~C_DebugHUDPerf();

	void							Update( const Maths::C_TimeS& timeStep );
	void							Render( );
	void							ProcessProfilerData();
	void							SetEventTree(Perf::C_EventTree* pEventTree){SAFE_DELETE_PTR(m_pEventTree); m_pEventTree = pEventTree;}
private:
	

	void							RenderItem(U32& index, const U32 level, Perf::C_EventTreeItem* pItem);
	U32								RenderBar(Maths::HVec2 PCREF position, U64 bar[2], Maths::HVec2& endPosition, const U32 level);

	Render::Font::C_FontRenderer*	m_pFontRenderer;
	U64								m_FirstEventTime;
	U64								m_LastEventTime;
	bool							m_bCallbackRegistered;

	Perf::C_EventTree*				m_pEventTree;
};

} // namespace DebugSystems

#endif
