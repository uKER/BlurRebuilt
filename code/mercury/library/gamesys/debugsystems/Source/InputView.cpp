
#include "DebugSystemsPch.h"
#include <DebugSystems/InputView.h>
#include <framework/application/ApplicationBase.h>
#include <input/Input.h>
#include <input/Device.h>
#include <render/font/Font.h>
#include <system/debug/I_DebugRender.h>

#if defined(PLATFORM_PC)
#include <Input/Pad360.h>
#endif
#ifdef PLATFORM_PS3
#include <Input/PadPS3.h>
#endif

#pragma warning (push, 4)

namespace DebugSystems
{
using namespace Maths;

static Debug::Bool g_InputViewEnable("System.InputView.Enable", false);
static Debug::Float g_InputScreenX("System.InputView.ScreenX", 840.f, Debug::Range(0.0f, 1024.0f));
static Debug::Float g_InputScreenY("System.InputView.ScreenY", 680.f, Debug::Range(0.0f, 768.0f));

C_InputView::C_InputView()
{
	m_pFontRenderer = 0;
}

void C_InputView::Init()
{
}

C_InputView::~C_InputView()
{
	delete m_pFontRenderer;
	m_pFontRenderer = 0;
}

void C_InputView::Render()
{
	if (!g_InputViewEnable)
		return;

	if (!m_pFontRenderer)
	{
		Render::Font::C_FontSystem* fontSystem = Render::Font::FontSystem();
		if (fontSystem)
			m_pFontRenderer = fontSystem->GetFontRenderer("dontship/fonts/arialdebug.font");

		if (!m_pFontRenderer)
			return;
	}

	Input::C_Device* pDevice = Framework::C_ApplicationBase::GetInstance()->GetPad(0);
	if (!pDevice)
		return;

	F32 sx = g_InputScreenX;
	F32 sy = g_InputScreenY;

	DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Immediate();
	debugRender->SetBlendMode(DbUtil::E_BlendMode_Alpha);

	debugRender->Draw2dBar(Maths::HVec2(sx - 142.0f, sy - 94.0f), Maths::HVec2(304.0f, 162.0f), RGBA(0x60606060));
	debugRender->Draw2dBar(Maths::HVec2(sx - 140.0f, sy - 92.0f), Maths::HVec2(300.0f, 158.0f), RGBA(0x80000000));
	
	F32 fButtons[16] = {0.0f};
	S32 i = 0;
	for ( ; i < 16; i++)
		fButtons[i] = pDevice->GetButtonValue(s_ButtonId[i]);

	const F32 fontx = 24.0f, fonty = 32.0f;
	m_pFontRenderer->SetScaleX(fontx);
	m_pFontRenderer->SetScaleY(fonty);

	for (i = 0; i < 16; i++)
	{
		const C_ButtonPlace& place = s_ButtonPlace[i];
		if (!place.text)
			continue;

		F32 bright = fButtons[i];
		F32 alpha = 0.25f + bright * 0.75f;

		RGBA colour(place.r * bright, place.g * bright, place.b * bright, alpha);
		RGBA back  (place.r * bright, place.g * bright, place.b * bright, alpha * 0.4f);
		m_pFontRenderer->SetColourTop(colour);
		m_pFontRenderer->SetColourBottom(colour);
		
		F32 px = sx + place.x;
		F32 py = sy + place.y;
		Maths::HVec2 pos(sx + place.x, sy + place.y);
		
		F32 wide = place.width;

		debugRender->Draw2dBar
		(
			Maths::HVec2(px - wide * 0.5f - 1.f, py - 11.0f),
			Maths::HVec2(wide + 2.f, 22.0f),
			back
		);

		m_pFontRenderer->Print(Maths::HVec2(px, py - fonty * 0.5f), place.text, Render::Font::E_Justification_Centre);
	}

	F32 fAxis[4] = {0.0f};
	for (i = 0; i < 4; i++)
		fAxis[i] = pDevice->GetAxisSignedValue(s_AxisId[i]);

	// Draw sticks:
	DrawStick(sx - 100.0f, sy, fAxis[0], fAxis[1]);
	DrawStick(sx +  68.0f, sy, fAxis[2], fAxis[3]);
}

void C_InputView::DrawStick(F32 sx, F32 sy, F32 ax, F32 ay)
{
	// Draw background of stick:
	Display::BasicRender()->DrawColoured2DRect(sx - 27.0f, sy - 36.0f, 54.0f, 72.0f, RGBA(0x40000000));

	F32 tx = sx + ax * 24.0f;
	F32 ty = sy - ay * 32.0f;
	Display::BasicRender()->DrawColoured2DRect(sx - 3.0f, sy - 4.0f, 6.0f, 8.0f, RGBA(0x80ffffff));
	Display::BasicRender()->DrawColoured2DRect(tx - 3.0f, ty - 4.0f, 6.0f, 8.0f, RGBA(0xffffffff));
}

S32 C_InputView::s_ButtonId[16]=
{
#if defined(PLATFORM_PC)
	Input::C_Pad360::E_Button_A,
	Input::C_Pad360::E_Button_B,
	Input::C_Pad360::E_Button_X,
	Input::C_Pad360::E_Button_Y,
	Input::C_Pad360::E_Button_DPadUp,
	Input::C_Pad360::E_Button_DPadDown,
	Input::C_Pad360::E_Button_DPadLeft,
	Input::C_Pad360::E_Button_DPadRight,
	Input::C_Pad360::E_Button_Start,
	Input::C_Pad360::E_Button_Back,
	Input::C_Pad360::E_Button_LeftShoulder,
	Input::C_Pad360::E_Button_RightShoulder,
	Input::C_Pad360::E_Button_LeftTrigger,
	Input::C_Pad360::E_Button_RightTrigger,
	Input::C_Pad360::E_Button_LeftThumb,
	Input::C_Pad360::E_Button_RightThumb,
#endif
#ifdef PLATFORM_PS3
	Input::C_PadPS3::E_Button_Cross,
	Input::C_PadPS3::E_Button_Circle,
	Input::C_PadPS3::E_Button_Square,
	Input::C_PadPS3::E_Button_Triangle,
	Input::C_PadPS3::E_Button_DPadUp,
	Input::C_PadPS3::E_Button_DPadDown,
	Input::C_PadPS3::E_Button_DPadLeft,
	Input::C_PadPS3::E_Button_DPadRight,
	Input::C_PadPS3::E_Button_Start,
	Input::C_PadPS3::E_Button_Select,
	Input::C_PadPS3::E_Button_L1,
	Input::C_PadPS3::E_Button_R1,
	Input::C_PadPS3::E_Button_L2,
	Input::C_PadPS3::E_Button_R2,
	Input::C_PadPS3::E_Button_L3,
	Input::C_PadPS3::E_Button_R3,
#endif
};

// List of where to draw the buttons on the screen:
C_InputView::C_ButtonPlace C_InputView::s_ButtonPlace[16]=
{
	{ 128.0f,  20.0f,  0.0f, 1.0f, 0.0f, "A",  16.f},
	{ 148.0f,   0.0f,  1.0f, 0.0f, 0.0f, "B",  16.f},
	{ 108.0f,   0.0f,  0.2f, 0.6f, 1.0f, "X",  16.f},
	{ 128.0f, -20.0f,  1.0f, 1.0f, 0.0f, "Y",  16.f},
	{ -40.0f, -20.0f,  1.0f, 1.0f, 1.0f, "U",  16.f},
	{ -40.0f,  20.0f,  1.0f, 1.0f, 1.0f, "D",  16.f},
	{ -60.0f,   0.0f,  1.0f, 1.0f, 1.0f, "L",  16.f},
	{ -20.0f,   0.0f,  1.0f, 1.0f, 1.0f, "R",  16.f},

	{  24.0f,   0.0f,  1.0f, 1.0f, 1.0f, "S",  16.f},
	{   4.0f,   0.0f,  1.0f, 1.0f, 1.0f, "b",  16.f},
	{-100.0f, -50.0f,  1.0f, 1.0f, 1.0f, "LB", 54.f},
	{  68.0f, -50.0f,  1.0f, 1.0f, 1.0f, "RB", 54.f},
	{-100.0f, -74.0f,  1.0f, 1.0f, 1.0f, "LT", 54.f},
	{  68.0f, -74.0f,  1.0f, 1.0f, 1.0f, "RT", 54.f},
	{-100.0f,  50.0f,  1.0f, 1.0f, 1.0f, "LS", 32.f},
	{  68.0f,  50.0f,  1.0f, 1.0f, 1.0f, "RS", 32.f}
};

S32 C_InputView::s_AxisId[4]=
{
#if defined(PLATFORM_PC)
		Input::C_Pad360::E_Axis_LeftStick_X,
		Input::C_Pad360::E_Axis_LeftStick_Y,
		Input::C_Pad360::E_Axis_RightStick_X,
		Input::C_Pad360::E_Axis_RightStick_Y
#endif
#ifdef PLATFORM_PS3
		Input::C_PadPS3::E_Axis_LeftStick_X,
		Input::C_PadPS3::E_Axis_LeftStick_Y,
		Input::C_PadPS3::E_Axis_RightStick_X,
		Input::C_PadPS3::E_Axis_RightStick_Y
#endif
};

}

#pragma warning (pop)
