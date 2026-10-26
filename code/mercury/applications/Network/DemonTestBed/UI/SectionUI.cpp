#include "StdAfx.h"
#include "SectionUI.h"

#include <System/Memory/MemoryTracker.h>
#include <network/SystemHeaders.h>
#include <systemprofile/SystemProfile.h>
#include <systemprofile/SystemProfileManager.h>
#include "Network/Core/NetServices.h"

#include "DemonwareTests.h"

#if defined (PLATFORM_PS3)
#include <cell\sysmodule.h>
#endif   

C_DemonwareTests gTests;

namespace Samples
{

C_SectionUI::C_SectionUI( const Util::C_String& name ) 
	: C_Section( name )
{
	//
}

C_SectionUI::~C_SectionUI()
{
	//
}

bool C_SectionUI::OnCreate()
{

#if defined (PLATFORM_PS3)
	// load prx modules..
	int b = cellSysmoduleLoadModule(CELL_SYSMODULE_NETCTL);	
	int c = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL);
	int d = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP2);
#endif

	RacerNet::C_NetServices::Instance().Initialise(true);

	gTests.Start();

	return true;
}


void C_SectionUI::OnDestroy()
{
	gTests.Stop();
	RacerNet::C_NetServices::Instance().Shutdown();
}

void C_SectionUI::OnUpdate( const C_Input& input, const C_Timer& timer )
{
#if defined (PLATFORM_PS3)
	// ps3 callback processing.
	cellSysutilCheckCallback();
#endif

	gTests.Poll(timer.GetFrameTime());

	RacerNet::C_NetServices::Instance().Poll();
}

void C_SectionUI::OnRenderHudView( const C_Input& input, const C_Timer& timer )
{
}

void C_SectionUI::OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer)
{
	// Note this is only here because nothing clears the tiles otherwise, if the game is to be rendered then remove me.
	Display::Device()->Clear(Display::E_Clear_Colour|Display::E_Clear_Depth|Display::E_Clear_Stencil);

	CPU_TIMING(0, Maths::RGBA(0, 255, 0, 255), "CPU.RenderUI");
	GPU_TIMING(0, Maths::RGBA(0, 255, 0, 255), "GPU.RenderUI");
}

void C_SectionUI::OnRender( const C_Input& input, const C_Timer& timer )
{
}

void C_SectionUI::OnPreRender( const C_Input& input, const C_Timer& timer )
{

}

void C_SectionUI::OnPostRender( const C_Input& input, const C_Timer& timer )
{

}

}; // namespace Samples
