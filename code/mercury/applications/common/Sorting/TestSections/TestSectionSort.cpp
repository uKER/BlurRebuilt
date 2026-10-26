#include "stdafx.h"
#include "TestSectionSort.h"
#include <Framework/Application/Application.h>
#include <Framework/Application/UserControls.h>
#include <Input/GameInput.h>

#if defined(PLATFORM_PS3)
static Scheduler::Policy g_DeflatePolicy[] = { 1, 1, 0, 0, 0, 0, 0, 0 };
static Scheduler::Policy g_InflatePolicy[] = { 0, 0, 1, 1, 1, 1, 1, 1 };
#else
static Scheduler::Policy g_DeflatePolicy[] = { 1, 1, 1, 1, 1, 1 };
static Scheduler::Policy g_InflatePolicy[] = { 1, 1, 1, 1, 1, 1 };
#endif

C_TestSectionSort::C_TestSectionSort(const Util::C_String& name) : Framework::C_TestSection(name)
{
	Scheduler::Init();
	Scheduler::Start();
	Scheduler::SetGroupPoolSize(2);
}

C_TestSectionSort::~C_TestSectionSort()
{
	Scheduler::Shutdown();
}

void C_TestSectionSort::OnUpdate()
{
	if (Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_MenuConfirm))
	{
	}
	else if (Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_MenuCancel))
	{
	}
}

void C_TestSectionSort::OnRender2DView()
{
	Display::FontRender()->Draw("Sort", 1024.f/2.f, 768.f/2.f, 50.f, 50.f, Maths::RGBA(255, 255, 255, 255), Display::E_FontRenderAlign_Ct);
}
