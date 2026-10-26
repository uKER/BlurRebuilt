#include "stdafx.h"
#include "Samples/Section/SectionNetConnect.h"
#include "Render/Render/GridRender.h"
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"

#ifdef COMPILER_VC
	#include <crtdbg.h>
	#ifdef PLATFORM_PC
		#include <windows.h>
	#endif
#endif
#ifdef PLATFORM_PS3
	#include <libsn.h>
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace Samples
{
	extern bool SectionNetConnect_Step(float dt, const C_Input& input);
	extern void SectionNetConnect_Start();
	extern void SectionNetConnect_End();
	extern void SectionNetConnect_Render(float fps);

	Debug::Logger &g_Log = Debug::GetLogger("NetTestApp.Asserts");
};

bool dbg_breakOnAssert = true;

bool TheClubAssert(const char *pszFunction, const char *pszExpression, const char *pszFilename, int iLineNumber)
{
	LOGFATAL( Samples::g_Log, "Assert(%s) failed in %s(%d) [%s]\n",
		pszExpression,
		pszFilename,
		iLineNumber,
		pszFunction);

	#ifndef CFG_SPEED
		#if (defined(COMPILER_VC) && defined(PLATFORM_PC) && defined(BUILD_CONFIG_DEBUG))
			// on the PC, in DEBUG mode, bring up the assert dialog box
			// (in RELEASE mode CrtDbgReport doesn't exist)
			   if (_CrtDbgReport(_CRT_ASSERT, pszFilename, iLineNumber, NULL, pszExpression))
				   _CrtDbgBreak();
		#endif
		#ifdef PLATFORM_PC
		if (IsDebuggerPresent() && dbg_breakOnAssert)
				DebugBreak();
		#endif
		#if (defined(PLATFORM_PS3))
			if (dbg_breakOnAssert)
				snPause();
		#endif
	#endif
	
	return true;
}

namespace Samples
{

C_SectionNetConnect::C_SectionNetConnect(const Util::C_String& name) 
: C_Section(name)
{
	Samples::SectionNetConnect_Start();

	if(!Render::Debug::g_pDebugRenderSys)
	{
		Render::Debug::C_DebugRenderSys* m_pDebugRenderSys;
		m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
		DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	}

	g_pCoreAssertFunction = TheClubAssert;
}

C_SectionNetConnect::~C_SectionNetConnect()
{
	Samples::SectionNetConnect_End();
}

void C_SectionNetConnect::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	Samples::SectionNetConnect_Step(timer.GetFrameTime(), input);
}

void C_SectionNetConnect::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionNetConnect");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 0, 0, 0));

	Samples::SectionNetConnect_Render(timer.GetFrameRate());	
}

}; // namespace Samples

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

