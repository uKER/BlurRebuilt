#include "StdAfx.h"
#include "SectionUI.h"

#include "UiManager.h"

#include <System/Memory/MemoryTracker.h>

#include <SystemProfile/SystemProfile.h>
#include <SystemProfile/SystemProfileManager.h>

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

#if defined( USER_WBROWN ) || defined( USER_DGILL )
#define USE_MEMTRACKING
#endif

bool C_SectionUI::OnCreate()
{
#if (defined USE_MEMTRACKING)
	Memory::Tracker::Instance().DumpNodeStats();
#endif

	SysProfile::C_Manager::Instance().Initialise();

	m_pUiManager = new UI::C_UiManager( 0 );
	assert( m_pUiManager );

	m_pUiManager->CreateTextureBundle( false );

#if (defined USE_MEMTRACKING)
	Memory::Tracker::Instance().DumpNodeStats();
#endif

	return true;
}


void C_SectionUI::OnDestroy()
{
	m_pUiManager->DeleteTextureBundle( true );

	SAFE_DELETE_PTR( m_pUiManager );

	SysProfile::C_Manager::Instance().Shutdown();

#if (defined USE_MEMTRACKING)
	Memory::Tracker::Instance().DumpAllocatedBlocks();
	Memory::Tracker::Instance().DumpNodeStats();
#endif
}

void C_SectionUI::OnUpdate( const C_Input& input, const C_Timer& timer )
{
	XtNetworkUpdate();

	// ***
	// this would be in the game somewhere done by someone else
	SysProfile::Notifications notifications;
	SysProfile::GameInvite invite;

	SysProfile::C_Manager::Instance().Poll( invite, notifications );

	// TODO - profile/net/invite changes UI messages etc ...

	// ***

	m_pUiManager->Process( Maths::C_TimeS( timer.GetFrameTime() ), input );
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

	// Render the UI layer
	m_pUiManager->Render();
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
